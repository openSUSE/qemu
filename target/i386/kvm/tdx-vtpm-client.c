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

typedef struct TdxVtpmClientDataEntry {
    void *buf;
    int buf_size;
    uint8_t state;
    QSIMPLEQ_ENTRY(TdxVtpmClientDataEntry) queue_entry;
} TdxVtpmClientDataEntry;

typedef struct TdxVtpmClientPendingRequest {
    TdxVmcallServiceItem *vsi;
    QLIST_ENTRY(TdxVtpmClientPendingRequest) list_entry;
} TdxVtpmClientPendingRequest;

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

static int tdx_vtpm_client_do_receive_message(TdxVmcallServiceItem *vsi,
                                              uint8_t recved_state,
                                              void *buf, int size)
{
    int ret;
    int rsp_size;
    TdxVtpmRspReceiveMessage *rsp;
    int total_size;
    int state;

    rsp = tdx_vmcall_service_rsp_buf(vsi);
    rsp_size = tdx_vmcall_service_rsp_size(vsi);

    total_size = sizeof(*rsp) + size;
    tdx_vmcall_service_set_rsp_size(vsi, total_size);
    if (total_size > rsp_size) {
        state = TDG_VP_VMCALL_SERVICE_RSP_BUF_TOO_SMALL;
        ret = -1;
    } else {
        rsp->head = tdx_vtpm_init_comm_head(TDX_VTPM_RECEIVE_MESSAGE);
        rsp->status = recved_state;
        rsp->reserved = 0;
        memcpy(rsp->data, buf, size);

        state = TDG_VP_VMCALL_SERVICE_SUCCESS;
        ret = 0;
    }

    tdx_vmcall_service_set_response_state(vsi, state);
    tdx_vmcall_service_complete_request(vsi);

    return ret;
}

static void tdx_vtpm_client_request_queue_remove(TdxVtpmClient *client,
                                                 TdxVtpmClientPendingRequest* entry)

{
    QLIST_REMOVE(entry, list_entry);

    tdx_vmcall_service_item_unref(entry->vsi);
    g_free(entry);
}

static int tdx_vtpm_client_request_queue_add(TdxVtpmClient *client,
                                             TdxVtpmClientPendingRequest *entry)
{
    TdxVtpmClientPendingRequest *new;

    new = g_try_malloc(sizeof(*new));
    if (!new)
        return -1;

    tdx_vmcall_service_item_ref(entry->vsi);
    new->vsi = entry->vsi;

    QLIST_INSERT_HEAD(&client->request_list, new, list_entry);
    return 0;
}

static TdxVtpmClientPendingRequest* tdx_vtpm_client_request_queue_get(TdxVtpmClient *client)
{
    if (QLIST_EMPTY(&client->request_list))
        return NULL;

    return QLIST_FIRST(&client->request_list);
}

static int tdx_vtpm_client_data_queue_add(TdxVtpmClient *client,
                                          TdxVtpmClientDataEntry *entry)
{
    TdxVtpmClientDataEntry *new;

    new = g_try_malloc(sizeof(*new));
    if (!new) {
        return -1;
    }

    new->buf = g_try_malloc(entry->buf_size);
    if (!new->buf) {
        g_free(new);
        return -1;
    }

    memcpy(new->buf, entry->buf, entry->buf_size);
    new->buf_size = entry->buf_size;
    QSIMPLEQ_INSERT_TAIL(&client->data_queue, new, queue_entry);
    return 0;
}

static void tdx_vtpm_client_data_queue_remove(TdxVtpmClient *client)
{
    TdxVtpmClientDataEntry* i = QSIMPLEQ_FIRST(&client->data_queue);

    QSIMPLEQ_REMOVE_HEAD(&client->data_queue, queue_entry);

    g_free(i->buf);
    g_free(i);
}

static TdxVtpmClientDataEntry*  tdx_vtpm_client_data_queue_get(TdxVtpmClient *client)
{
    if (QSIMPLEQ_EMPTY(&client->data_queue))
        return NULL;

    return QSIMPLEQ_FIRST(&client->data_queue);
}

static void tdx_vtpm_handle_receive_message_timeout_handler(TdxVmcallServiceItem *vsi,
                                                            void *opaque)
{
    TdxVtpmClient *client = opaque;
    TdxVtpmClientPendingRequest *found = NULL;
    TdxVtpmClientPendingRequest *i;

    qemu_mutex_lock(&client->lock);

    QLIST_FOREACH(i, &client->request_list, list_entry) {
        if (vsi != i->vsi)
            continue;
        found = i;
        break;
    }

    g_assert(found);
    tdx_vtpm_client_request_queue_remove(client, found);

    qemu_mutex_unlock(&client->lock);

}

static void tdx_vtpm_handle_receive_message(TdxVtpmClient *vtpm_client,
                                            TdxVmcallServiceItem *vsi)
{
    int ret;

    if (!QSIMPLEQ_EMPTY(&vtpm_client->data_queue)) {
        TdxVtpmClientDataEntry*  entry;

        entry = tdx_vtpm_client_data_queue_get(vtpm_client);
        ret = tdx_vtpm_client_do_receive_message(vsi,
                                                 entry->state,
                                                 entry->buf, entry->buf_size);
        if (!ret) {
            tdx_vtpm_client_data_queue_remove(vtpm_client);
        }
    } else {
        TdxVtpmClientPendingRequest entry;

        entry.vsi = vsi;
        ret = tdx_vtpm_client_request_queue_add(vtpm_client, &entry);
        if (ret) {
            error_report("Failed to add data queue, data dropped");
            return;
        }

        tdx_vmcall_service_set_timeout_handler(vsi,
                                               tdx_vtpm_handle_receive_message_timeout_handler,
                                               vtpm_client);
    }
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
    case TDX_VTPM_RECEIVE_MESSAGE:
        tdx_vtpm_handle_receive_message(vtpm_client, vsi);
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

static void tdx_vtpm_client_handle_trans_protocol_data(TdxVtpmClient *client,
                                                       void *buf, int size)
{
    TdxVtpmTransProtocolData *data = buf;
    int payload_size = trans_protocol_data_payload_size(data);
    TdxVtpmClientDataEntry data_entry;
    int ret;

    if (!QLIST_EMPTY(&client->request_list)) {
        TdxVtpmClientPendingRequest *request;

        request = tdx_vtpm_client_request_queue_get(client);
        ret = tdx_vtpm_client_do_receive_message(request->vsi,
                                                 data->state,
                                                 data->data, payload_size);
        tdx_vtpm_client_request_queue_remove(client, request);
        if (!ret) {
            return;
        }
    }

    data_entry.buf = data->data;
    data_entry.buf_size = payload_size;
    data_entry.state = data->state;
    ret = tdx_vtpm_client_data_queue_add(client, &data_entry);
    if (ret) {
        error_report("Failed to push queue data, dropped data");
        return;
    }

}

static void tdx_vtpm_client_handle_trans_protocol(TdxVtpmClient *client,
                                                  void *buf,
                                                  int size)
{
    TdxVtpmTransProtocolHead *head = buf;

    switch(head->type) {
    case TDX_VTPM_TRANS_PROTOCOL_TYPE_DATA:
        tdx_vtpm_client_handle_trans_protocol_data(client, buf, size);
        break;
    default:
        error_report("Not implemented trans protocol type: %d", head->type);
    }
}

static void tdx_vtpm_client_handle_recv_data(TdxVtpmClient *client)
{
    TdxVtpmTransProtocolHead *head;
    QIOChannelSocket *ioc = client->parent.ioc;
    void *data;
    int read_size;

    data = g_try_malloc(TDX_VTPM_TRANS_PROTOCOL_MAX_LEN);
    if (!data) {
        error_report("Out of memory");
        return;
    }

    read_size = qio_channel_read(QIO_CHANNEL(ioc),
                                 data, TDX_VTPM_TRANS_PROTOCOL_MAX_LEN,
                                 NULL);
    if (read_size <= 0) {
        error_report("Read trans protocol failed: %d", read_size);
        goto out;
    }

    head = data;
    if (head->length > TDX_VTPM_TRANS_PROTOCOL_MAX_LEN) {
        error_report("Exceed max len of trans protocol:%d", head->length);
        exit(-1);
    }

    tdx_vtpm_client_handle_trans_protocol(client, data, read_size);
 out:
    g_free(data);
}

static void tdx_vtpm_socket_client_recv(void *opaque)
{
    TdxVtpmClient *client = opaque;

    qemu_mutex_lock(&client->lock);

    tdx_vtpm_client_handle_recv_data(client);

    qemu_mutex_unlock(&client->lock);
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
    QSIMPLEQ_INIT(&client->data_queue);
    QLIST_INIT(&client->request_list);

    g_free(local_addr);
    return 0;

 free:
    g_free(local_addr);
 fail:
    return -1;
}
