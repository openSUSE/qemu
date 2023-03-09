#ifndef QEMU_I386_TDX_VTPM_H
#define QEMU_I386_TDX_VTPM_H

#ifndef CONFIG_USER_ONLY
#include CONFIG_DEVICES /* CONFIG_TDX */
#endif

#include <linux/kvm.h>
#include "qemu/osdep.h"
#include "sysemu/sysemu.h"
#include "qemu/uuid.h"

typedef unsigned char TdUserId[16];
typedef struct TdxVtpm {
    TdxGuest *tdx;
    QIOChannelSocket *ioc;
} TdxVtpm;

QIOChannelSocket* tdx_vtpm_setup_communication(const char *local_addr);
int tdx_vtpm_init_base(TdxVtpm *base, TdxGuest *tdx,
                       const char* local_addr,
                       IOHandler *read, void *read_opaque);

typedef struct TdxVtpmServer {
    TdxVtpm parent;

} TdxVtpmServer;

int tdx_vtpm_init_server(TdxVtpm *base, TdxVmcallService *vms,
                         TdxGuest *tdx, TdxVmcallServiceType *type);

typedef struct TdxVtpmClient {
    TdxVtpm parent;

    TdUserId user_id;
    struct UnixSocketAddress server_addr;

} TdxVtpmClient;


int tdx_vtpm_init_client(TdxVtpm *base, TdxVmcallService *vms,
                         TdxGuest *tdx, TdxVmcallServiceType *type);

#endif
