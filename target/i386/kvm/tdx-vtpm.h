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

    QemuMutex lock;
} TdxVtpmClient;


int tdx_vtpm_init_client(TdxVtpm *base, TdxVmcallService *vms,
                         TdxGuest *tdx, TdxVmcallServiceType *type);

enum TdxVtpmCommand {
    TDX_VTPM_SEND_MESSAGE = 1,
};

typedef struct TdxVtpmCommHead {
    uint8_t version;
    uint8_t command;
} QEMU_PACKED TdxVtpmCommHead;

typedef struct TdxVtpmCmdSendMessage {
    TdxVtpmCommHead head;
    uint8_t reserved[2];
    uint8_t data[0];
} QEMU_PACKED TdxVtpmCmdSendMessage;

typedef struct TdxVtpmRspSendMessage {
    TdxVtpmCommHead head;
    uint8_t status;
    uint8_t reserved;
} QEMU_PACKED TdxVtpmRspSendMessage;


enum TdxVtpmTransProtocolType {
    TDX_VTPM_TRANS_PROTOCOL_TYPE_DATA = 1,
};

typedef struct TdxVtpmTransProtocolHead {
    uint8_t version;
    uint8_t type;
    uint8_t reserved[2];
    uint32_t length;
} QEMU_PACKED TdxVtpmTransProtocolHead;

TdxVtpmTransProtocolHead tdx_vtpm_init_trans_protocol_head(uint8_t type);

typedef struct TdxVtpmTransProtocolData {
    TdxVtpmTransProtocolHead head;

    /*payload*/
    uint8_t state;
    uint8_t user_id[16];
    uint8_t data[0];
} QEMU_PACKED TdxVtpmTransProtocolData;
#define trans_protocol_data_payload_size(item) \
    ((item)->head.length - sizeof(TdxVtpmTransProtocolData))

int tdx_vtpm_trans_send(QIOChannelSocket *socket_ioc,
                        struct UnixSocketAddress *addr,
                        TdxVtpmTransProtocolHead *head,
                        struct iovec *iovec, int iovec_count);

#endif
