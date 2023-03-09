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

#define VTPM_SERVICE_TD_GUID "c3c87a08-3b4a-41ad-a52d-96f13cf89a66"

static void tdx_vtpm_socket_server_recv(void *opaque)
{
    printf("%s\n", __func__);
}

static void tdx_vtpm_vmcall_service_server_handler(TdxVmcallServiceItem *vsi,
                                                   void* opaque)
{
    tdx_vmcall_service_set_response_state(vsi,
                                          TDG_VP_VMCALL_SERVICE_DEVICE_ERROR);
    tdx_vmcall_service_complete_request(vsi);

    return;
}

int tdx_vtpm_init_server(TdxVtpm *base, TdxVmcallService *vms,
                         TdxGuest *tdx, TdxVmcallServiceType *type)
{
    int ret;
    TdxVtpmServer *server = container_of(base,
                                         TdxVtpmServer, parent);

    ret = tdx_vtpm_init_base(base, tdx, vms->vtpm_path,
                             tdx_vtpm_socket_server_recv, server);
    if (ret)
        return ret;

    qemu_uuid_parse(VTPM_SERVICE_TD_GUID, &type->from);
    type->from = qemu_uuid_bswap(type->from);
    type->to = tdx_vtpm_vmcall_service_server_handler;
    type->vsi_size = sizeof(TdxVmcallServiceItem);

    return 0;
}
