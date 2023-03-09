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

static void tdx_vtpm_vmcall_service_client_handler(TdxVmcallServiceItem *vsi,
                                                   void* opaque)
{
    tdx_vmcall_service_set_response_state(vsi,
                                          TDG_VP_VMCALL_SERVICE_DEVICE_ERROR);
    tdx_vmcall_service_complete_request(vsi);

    return;
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

    g_free(local_addr);
    return 0;

 free:
    g_free(local_addr);
 fail:
    return -1;
}
