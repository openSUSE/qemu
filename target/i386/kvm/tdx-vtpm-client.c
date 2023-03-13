#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qom/object_interfaces.h"
#include "sysemu/tdx.h"
#include "io/channel-socket.h"
#include "hw/i386/x86.h"
#include "kvm_i386.h"
#include "tdx.h"
#include "tdx-vmcall-service.h"
#include "tdx-vtpm.h"

#include "trace.h"

#define VTPM_USER_GUID "64590793-7852-4E52-BE45-CDBB116F20F3"

static char* tdx_vtpm_client_generate_path(const char *base)
{
    char *path = g_malloc0(PATH_MAX);

    if (!path)
        return path;

    snprintf(path, PATH_MAX,
             "unix:/tmp/vtpm-client-%s-%d.sock", base, getpid());

    return path;
}

static int tdx_vtpm_client_save_server_addr(TdxVtpmClient *client,
                                             const char* addr)
{
    SocketAddress *s_addr;
    Error *local_err;

    s_addr = socket_parse(addr, &local_err);
    if (!s_addr) {
        return -1;
    }

    memset(&client->server_addr, 0, sizeof(client->server_addr));
    client->server_addr.path = g_strdup(s_addr->u.q_unix.path);
    client->server_addr.abstract = true;
    g_free(s_addr);
    return 0;
}

static void tdx_vtpm_socket_client_recv(void *opaque)
{
    printf("%s\n", __func__);
}

static void tdx_vtpm_set_send_message_rsp(TdxVmcallServiceItem *vsi,
                                          unsigned char status)
{
    TdxVtpmRspSendMessage *rsp;

    rsp = tdx_vmcall_service_rsp_buf(vsi);
    rsp->head.version = 0;
    rsp->head.command = TDX_VTPM_SEND_MESSAGE;
    rsp->status = status;
    rsp->reserved= 0;
}

static int tdx_vtpm_sanity_check_send_message(TdxVmcallServiceItem *vsi)
{
    int64_t size;

    size = tdx_vmcall_service_cmd_size(vsi);
    if (size <= sizeof(TdxVtpmCmdSendMessage)) {
        tdx_vmcall_service_set_response_state(vsi,
                                              TDG_VP_VMCALL_SERVICE_BAD_CMD_BUF_SIZE);
        return -1;
    }

    size = tdx_vmcall_service_rsp_size(vsi);
    if (size < sizeof(TdxVtpmRspSendMessage)) {
        tdx_vmcall_service_set_response_state(vsi,
                                              TDG_VP_VMCALL_SERVICE_BAD_RSP_BUF_SIZE);
        return -1;
    }

    return 0;
}

static int tdx_vtpm_send_data_message(TdxVtpmClient *vtpm_client,
                                      TdxVmcallServiceItem *vsi)
{
    TdxVtpmTransProtocolData pack;
    TdxVtpmCmdSendMessage *cmd;
    int64_t cmd_payload_size;
    int64_t size;
    struct iovec payload[3];
    uint8_t dummy = 0;

    cmd = tdx_vmcall_service_cmd_buf(vsi);
    size = tdx_vmcall_service_cmd_size(vsi);
    cmd_payload_size = size - sizeof(*cmd);

    pack.head = tdx_vtpm_init_trans_protocol_head(TDX_VTPM_TRANS_PROTOCOL_TYPE_DATA);

    payload[0].iov_base = &dummy;
    payload[0].iov_len = sizeof(dummy);
    payload[1].iov_base = &vtpm_client->user_id;
    payload[1].iov_len = sizeof(vtpm_client->user_id);
    payload[2].iov_base = cmd->data;
    payload[2].iov_len = cmd_payload_size;

    if (tdx_vtpm_trans_send(vtpm_client->parent.ioc, &vtpm_client->server_addr,
                            &pack.head, payload, 3)) {
        tdx_vmcall_service_set_response_state(vsi,
                                              TDG_VP_VMCALL_SERVICE_DEVICE_ERROR);
        return -1;
    }

    return 0;
}

static void tdx_vtpm_handle_send_message(TdxVtpmClient *vtpm_client,
                                         TdxVmcallServiceItem *vsi)
{
    if (tdx_vtpm_sanity_check_send_message(vsi)) {
        goto out;
    }

    if (tdx_vtpm_send_data_message(vtpm_client, vsi)) {
        goto out;
    }

    tdx_vmcall_service_set_response_state(vsi,
                                          TDG_VP_VMCALL_SERVICE_SUCCESS);
 out:
    /* status in response of Send message is not use now, always 0*/
    tdx_vtpm_set_send_message_rsp(vsi, 0);
    tdx_vmcall_service_complete_request(vsi);

    return;
}

static void tdx_vtpm_vmcall_service_handle_command(TdxVtpmClient *vtpm_client,
                                                   TdxVmcallServiceItem *vsi)
{
    TdxVtpmCommHead *cmd_head;
    int64_t size;

    cmd_head = tdx_vmcall_service_cmd_buf(vsi);
    size = tdx_vmcall_service_cmd_size(vsi);

    if (!cmd_head || !size || size < sizeof(*cmd_head)) {
        tdx_vmcall_service_set_response_state(vsi,
                                              TDG_VP_VMCALL_SERVICE_INVALID_OPERAND);
        tdx_vmcall_service_complete_request(vsi);
        return;
    }

    switch (cmd_head->command) {
    case TDX_VTPM_SEND_MESSAGE:
        tdx_vtpm_handle_send_message(vtpm_client, vsi);
        break;
    default:
        error_report("%d not implemented", cmd_head->command);
    }
}

static void tdx_vtpm_vmcall_service_client_handler(TdxVmcallServiceItem *vsi, void* opaque)
{
    TdxVtpmClient *vtpm_client = opaque;

    qemu_mutex_lock(&vtpm_client->lock);

    tdx_vtpm_vmcall_service_handle_command(vtpm_client, vsi);

    qemu_mutex_unlock(&vtpm_client->lock);
}

int tdx_vtpm_init_client(TdxVtpm *base, TdxVmcallService *vms,
                         TdxGuest *tdx, TdxVmcallServiceType *type)
{
    int ret;
    char *local_addr;
    TdxVtpmClient *client = container_of(base,
                                         TdxVtpmClient, parent);
    QemuUUID uuid;
    bool is_uuid = false;

    if (!qemu_uuid_parse(vms->vtpm_userid, &uuid)) {
        is_uuid = true;
    } else if (strlen(vms->vtpm_userid) <= sizeof(client->user_id)) {
        memcpy(client->user_id, vms->vtpm_userid, sizeof(client->user_id));
    } else {
        error_report("Invalid vtpm user id, should be UUID or text length < 17");
        goto fail;
    }

    if (tdx_vtpm_client_save_server_addr(client, vms->vtpm_path)) {
        error_report("Failed to save server addr %s for client.",
                     vms->vtpm_path);
        goto fail;
    }

    local_addr = tdx_vtpm_client_generate_path(vms->vtpm_userid);
    if (!local_addr) {
        error_report("Failed to generate addr for client");
        goto fail;
    }

    if (is_uuid) {
        uuid = qemu_uuid_bswap(uuid);
        memcpy(client->user_id, &uuid, sizeof(client->user_id));
    }
    ret = tdx_vtpm_init_base(base, tdx, local_addr,
                             tdx_vtpm_socket_client_recv,
                             client);
    if (ret)
        goto free;

    qemu_uuid_parse(VTPM_USER_GUID, &type->from);
    type->from = qemu_uuid_bswap(type->from);
    type->to = tdx_vtpm_vmcall_service_client_handler;
    type->vsi_size = sizeof(TdxVmcallServiceItem);

    qemu_mutex_init(&client->lock);

    g_free(local_addr);
    return 0;

 free:
    g_free(local_addr);
 fail:
    return -1;
}
