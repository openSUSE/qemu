#ifndef QEMU_I386_TDX_VTPM_H
#define QEMU_I386_TDX_VTPM_H

#ifndef CONFIG_USER_ONLY
#include CONFIG_DEVICES /* CONFIG_TDX */
#endif

#include <linux/kvm.h>
#include "qemu/osdep.h"
#include "sysemu/sysemu.h"
#include "qemu/uuid.h"

typedef struct SocketRecvBuffer {
    void *buf;
    int size;
    int used_size;
    bool update_buf;
} SocketRecvBuffer;

int socket_recv_buffer_init(SocketRecvBuffer *srb, int init_size);
int socket_recv_buffer_next(SocketRecvBuffer *srb, void **data, int *size);
void* socket_recv_buffer_get_buf(SocketRecvBuffer *srb);
int socket_recv_buffer_get_free_size(SocketRecvBuffer *srb);
void socket_recv_buffer_update_used_size(SocketRecvBuffer *srb, int new_used_size);

typedef unsigned char TdUserId[16];
typedef struct TdxVtpm {
    TdxGuest *tdx;
    QIOChannelSocket *ioc;
} TdxVtpm;

QIOChannelSocket* tdx_vtpm_setup_communication(const char *local_addr);
int tdx_vtpm_init_base(TdxVtpm *base, TdxGuest *tdx,
                       const char* local_addr,
                       IOHandler *read, void *read_opaque);

struct TdxVtpmServerPendingRequest;
struct TdxVtpmServerPendingManageRequest;
struct TdxVtpmServerSessionRequest;
typedef struct TdxVtpmServer {
    TdxVtpm parent;

    QemuMutex lock;

    /*UserID -> client session */
    GHashTable *client_session;

    /*UserID -> recevied data cache */
    GHashTable *client_data;

    QLIST_HEAD(, TdxVtpmServerPendingRequest) request_list;

    QLIST_HEAD(, TdxVtpmServerPendingManageRequest) manage_request_list;

    QSIMPLEQ_HEAD(, TdxVtpmServerSessionDataIndex) session_data_index;

} TdxVtpmServer;

int tdx_vtpm_init_server(TdxVtpm *base, TdxVmcallService *vms,
                         TdxGuest *tdx, TdxVmcallServiceType *type);

struct TdxVtpmClientDataEntry;
struct TdxVtpmClientPendingRequest;
typedef struct TdxVtpmClient {
    TdxVtpm parent;

    TdUserId user_id;
    struct UnixSocketAddress server_addr;

    QemuMutex lock;

    QSIMPLEQ_HEAD(, TdxVtpmClientDataEntry) data_queue;
    QLIST_HEAD(, TdxVtpmClientPendingRequest) request_list;
} TdxVtpmClient;

int tdx_vtpm_init_client(TdxVtpm *base, TdxVmcallService *vms,
                         TdxGuest *tdx, TdxVmcallServiceType *type);

enum TdxVtpmCommand {
    TDX_VTPM_SEND_MESSAGE = 1,
    TDX_VTPM_RECEIVE_MESSAGE = 2,
    TDX_VTPM_WAIT_FOR_REQUEST = 1,
    TDX_VTPM_REPORT_STATUS = 2,
};

typedef struct TdxVtpmCommHead {
    uint8_t version;
    uint8_t command;
} QEMU_PACKED TdxVtpmCommHead;

TdxVtpmCommHead tdx_vtpm_init_comm_head(uint8_t type);

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

typedef struct TdxVtpmCmdWaitForRequest {
    TdxVtpmCommHead head;
    unsigned char reserved[2];
    TdUserId user_id; // all 0 for broadcast
} QEMU_PACKED TdxVtpmCmdWaitForRequest;

typedef struct TdxVtpmRspWaitForRequest {
    TdxVtpmCommHead head;
    unsigned char operation;
    unsigned char reserved;
    TdUserId user_id;
    unsigned char data[0];
} QEMU_PACKED TdxVtpmRspWaitForRequest;

typedef struct TdxVtpmCmdReportStatus {
    TdxVtpmCommHead head;
    unsigned char operation;
    unsigned char status;
    TdUserId user_id;
    unsigned char data[0];
} QEMU_PACKED TdxVtpmCmdReportStatus;
#define tdx_vtpm_cmd_report_status_payload_size(size) \
    (size) - sizeof(TdxVtpmCmdReportStatus)

typedef struct TdxVtpmRspReportStatus {
    TdxVtpmCommHead head;
    unsigned char reserved[2];
} QEMU_PACKED TdxVtpmRspReportStatus;

typedef struct TdxVtpmCmdReceiveMessage {
    TdxVtpmCommHead head;
    unsigned char reserved[2];
} QEMU_PACKED TdxVtpmCmdReceiveMessage;

typedef struct TdxVtpmRspReceiveMessage {
    TdxVtpmCommHead head;
    unsigned char status;
    unsigned char reserved;
    unsigned char data[0];
} QEMU_PACKED TdxVtpmRspReceiveMessage;

#define TDX_VTPM_TRANS_PROTOCOL_MAX_LEN (16 * 1024)

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
