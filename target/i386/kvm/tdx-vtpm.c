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

QIOChannelSocket* tdx_vtpm_setup_communication(const char *local_addr)
{
    QIOChannelSocket* ioc;
    SocketAddress *addr;
    Error *local_err;

    addr = socket_parse(local_addr, &local_err);
    if (!addr)
        return NULL;

    ioc = qio_channel_socket_new();
    if (!ioc) {
        goto free_addr;
    }

    addr->u.q_unix.abstract = true;
    if (qio_channel_socket_dgram_sync(ioc, addr, NULL, &local_err)) {
        goto free_ioc;
    }

    qio_channel_set_blocking(QIO_CHANNEL(ioc), false, NULL);

    g_free(addr);
    return ioc;

 free_ioc:
    object_unref(ioc);
 free_addr:
    g_free(addr);
    return NULL;
}

int tdx_vtpm_init_base(TdxVtpm *base, TdxGuest *tdx,
                       const char* local_addr,
                       IOHandler *read, void *read_opaque)
{
    QIOChannelSocket *ioc;

    ioc = tdx_vtpm_setup_communication(local_addr);
    if (!ioc) {
        error_report("Failed to setup communication:%s",
                     local_addr);
        return -1;
    }
    qemu_set_fd_handler(ioc->fd, read, NULL, read_opaque);

    base->ioc = ioc;
    base->tdx = tdx;

    return 0;
}

void tdx_guest_init_vtpm(TdxGuest *tdx)
{
    TdxVmcallService *vms = &tdx->vmcall_service;
    TdxVmcallServiceType vtpm;
    TdxVtpm *instance;
    bool is_server;
    int size;
    int ret;

    is_server = !g_strcmp0(vms->vtpm_type, "server");
    if (is_server) {
        size =  sizeof(TdxVtpmServer);
    } else {
        size = sizeof(TdxVtpmClient);
    }

    instance = g_try_malloc0(size);
    if (!instance) {
        error_report("Failed to create vtpm %s instance.",
                     vms->vtpm_type);
        return;
    }

    if (is_server) {
        ret = tdx_vtpm_init_server(instance, vms, tdx, &vtpm);
    } else {
        ret = tdx_vtpm_init_client(instance, vms, tdx, &vtpm);
    }

    if (ret) {
        error_report("Failed to init vtpm %s instance.",
                     vms->vtpm_type);
        goto free;
    }

    vtpm.opaque = instance;
    tdx_vmcall_service_register_type(tdx, &vtpm);

    return;
 free:
    g_free(instance);
}
