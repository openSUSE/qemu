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
#include "qapi-types-tdx.h"
#include "qapi-commands-tdx.h"
#include "qapi-events-tdx.h"

#include "trace.h"

#define VTPM_SERVICE_TD_GUID "c3c87a08-3b4a-41ad-a52d-96f13cf89a66"

enum TdxVtpmOperation {
    TDX_VTPM_OPERATION_NOOP,
    TDX_VTPM_OPERATION_COMM,
    TDX_VTPM_OPERATION_CREATE,
    TDX_VTPM_OPERATION_DESTROY,
};

typedef struct TdxVtpmServerClientDataEntry {
    void *buf;
    int buf_size;
    enum TdxVtpmOperation operation;

    QSIMPLEQ_ENTRY(TdxVtpmServerClientDataEntry) queue_entry;
} TdxVtpmServerClientDataEntry;

typedef struct TdxVtpmServerClientSession {
    TdUserId user_id;
    struct UnixSocketAddress client_addr;
} TdxVtpmServerClientSession;

typedef struct TdxVtpmServerPendingRequest {
    TdxVmcallServiceItem *vsi;

    QLIST_ENTRY(TdxVtpmServerPendingRequest) list_entry;
} TdxVtpmServerPendingRequest;

typedef struct TdxVtpmServerClientSessionDataNode {
    TdUserId user_id;
    QSIMPLEQ_HEAD(, TdxVtpmServerClientDataEntry) data_queue;
} TdxVtpmServerClientSessionDataNode;

typedef struct TdxVtpmServerPendingManageRequest {
    TdUserId user_id;
    enum TdxVtpmOperation operation;
    char user_id_str[UUID_FMT_LEN + 1];

    QLIST_ENTRY(TdxVtpmServerPendingManageRequest) list_entry;
} TdxVtpmServerPendingManageRequest;

typedef struct TdxVtpmServerSessionDataIndex {
    TdUserId user_id;
    QSIMPLEQ_ENTRY(TdxVtpmServerSessionDataIndex) queue_entry;
} TdxVtpmServerSessionDataIndex;

struct g_hash_find_opaque {
    TdUserId *user_id;
    TdUserId result;
};

static TdUserId null_user_id;

static int tdx_vtpm_server_client_session_set_addr(TdxVtpmServerClientSession* session,
                                                    struct UnixSocketAddress *addr)
{
    if (!g_strcmp0(session->client_addr.path, addr->path))
        return 0;

    g_free(session->client_addr.path);
    session->client_addr = *addr;
    session->client_addr.path = g_strdup(addr->path);
    if (addr->path && !session->client_addr.path)
        return -1;
    return 0;
}

static int tdx_vtpm_client_session_data_queue_add(TdxVtpmServer *server,
                                                  TdUserId* user_id,
                                                  TdxVtpmServerClientDataEntry *entry);
static void tdx_vtpm_server_destroy_client_session(TdxVtpmServer *server,
                                                   TdxVtpmServerClientSession *session);
static TdxVtpmServerClientSession*
tdx_vtpm_server_create_client_session(TdxVtpmServer *server,
                                      TdUserId *user_id,
                                      struct UnixSocketAddress *addr)
{
    TdxVtpmServerClientSession* new;
    int ret;

    new = g_malloc0(sizeof(*new));
    if (!new)
        return NULL;

    memcpy(new->user_id, user_id, sizeof(new->user_id));
    if (tdx_vtpm_server_client_session_set_addr(new, addr)) {
        g_free(new);
        return NULL;
    }

    ret = g_hash_table_insert(server->client_session,
                              &new->user_id, new);
    if (!ret) {
        error_report("Failed to insert client session");
        goto fail_free;
    }

    return new;

 fail_free:
    tdx_vtpm_server_destroy_client_session(server, new);
    return NULL;
}

static void
tdx_vtpm_client_session_free_data_queue_entry(struct TdxVtpmServerClientDataEntry *entry)
{
    g_free(entry->buf);
    g_free(entry);
}

static void
tdx_vtpm_client_session_remove_data_node(TdxVtpmServer *server,
                                         TdUserId *user_id)
{
    TdxVtpmServerClientSessionDataNode *data_node;

    data_node = g_hash_table_lookup(server->client_data, user_id);
    if (!data_node)
        return;

    g_assert(QSIMPLEQ_EMPTY(&data_node->data_queue));

    g_hash_table_remove(server->client_data, user_id);
    g_free(data_node);
}

static void
tdx_vtpm_client_session_remove_data_node_entry(TdxVtpmServerClientSessionDataNode *data_node)
{
    TdxVtpmServerClientDataEntry *p;

    if (!data_node)
        return;

    while(!QSIMPLEQ_EMPTY(&data_node->data_queue)) {
        p = QSIMPLEQ_FIRST(&data_node->data_queue);
        QSIMPLEQ_REMOVE_HEAD(&data_node->data_queue, queue_entry);
        tdx_vtpm_client_session_free_data_queue_entry(p);
    }
}

static TdxVtpmServerClientSessionDataNode*
tdx_vtpm_client_session_get_data_node(TdxVtpmServer *server,
                                      TdUserId *user_id, bool create)
{
    TdxVtpmServerClientSessionDataNode *data_node;

    /*Data node already existed, then just return */
    data_node = g_hash_table_lookup(server->client_data, user_id);
    if (data_node)
        return data_node;

    if (!create)
        return NULL;

    data_node = g_try_malloc(sizeof(*data_node));
    if (!data_node)
        return NULL;

    memcpy(data_node->user_id, user_id, sizeof(data_node->user_id));
    QSIMPLEQ_INIT(&data_node->data_queue);

    if (!g_hash_table_insert(server->client_data,
                             &data_node->user_id, data_node)) {
        g_free(data_node);
        return NULL;
    }

    return data_node;
}

static void tdx_vtpm_server_destroy_client_session(TdxVtpmServer *server,
                                                   TdxVtpmServerClientSession *session)
{
    TdUserId user_id;
    TdxVtpmServerClientSessionDataNode *data_node;

    memcpy(&user_id, &session->user_id, sizeof(user_id));
    g_hash_table_remove(server->client_session, &user_id);
    data_node = tdx_vtpm_client_session_get_data_node(server,
                                                      &user_id, false);
    if (data_node)
        tdx_vtpm_client_session_remove_data_node_entry(data_node);
    tdx_vtpm_client_session_remove_data_node(server, &user_id);

    g_free(session->client_addr.path);
    g_free(session);
}

static int tdx_vtpm_client_session_data_queue_add(TdxVtpmServer *server,
                                                  TdUserId *user_id,
                                                  TdxVtpmServerClientDataEntry *entry)
{
    TdxVtpmServerClientSessionDataNode *data_node;
    struct TdxVtpmServerClientDataEntry *new;

    data_node = tdx_vtpm_client_session_get_data_node(server, user_id, true);
    if (!data_node)
        return -1;

    new = g_try_malloc0(sizeof(*new));
    if (!new) {
        return -1;
    }

    new->operation = entry->operation;
    if (entry->buf_size && entry->buf) {
        new->buf = g_try_malloc(entry->buf_size);
        if (!new->buf) {
            g_free(new);
            return -1;
        }
        memcpy(new->buf, entry->buf, entry->buf_size);
        new->buf_size = entry->buf_size;
    }

    QSIMPLEQ_INSERT_TAIL(&data_node->data_queue, new, queue_entry);

    return 0;
}

static TdxVtpmServerClientDataEntry*
tdx_vtpm_client_session_data_queue_get(TdxVtpmServer *server,
                                       TdUserId *user_id)
{
    TdxVtpmServerClientSessionDataNode *data_node;

    data_node = tdx_vtpm_client_session_get_data_node(server, user_id, false);
    if (!data_node)
        return NULL;

    if (QSIMPLEQ_EMPTY(&data_node->data_queue)) {
        return NULL;
    }

    return QSIMPLEQ_FIRST(&data_node->data_queue);
}

static void tdx_vtpm_client_session_data_queue_remove(TdxVtpmServer *server, TdUserId *user_id)
{
    TdxVtpmServerClientSessionDataNode *data_node;

    data_node = tdx_vtpm_client_session_get_data_node(server, user_id, false);
    if (!data_node)
        return;

    if (!QSIMPLEQ_EMPTY(&data_node->data_queue)) {
        struct TdxVtpmServerClientDataEntry *data_entry;

        data_entry = QSIMPLEQ_FIRST(&data_node->data_queue);
        QSIMPLEQ_REMOVE_HEAD(&data_node->data_queue, queue_entry);
        tdx_vtpm_client_session_free_data_queue_entry(data_entry);
    }

    if (QSIMPLEQ_EMPTY(&data_node->data_queue)) {
        tdx_vtpm_client_session_remove_data_node(server, user_id);
    }
}

static int tdx_vtpm_server_request_queue_add(TdxVtpmServer *server,
                                             TdxVtpmServerPendingRequest *entry)
{
    TdxVtpmServerPendingRequest *new;

    new = g_malloc0(sizeof(*new));
    if (!new)
        return -1;

    /*pair with unref in tdx_vtpm_client_session_request_queue_remove() */
    tdx_vmcall_service_item_ref(entry->vsi);

    *new = *entry;
    QLIST_INSERT_HEAD(&server->request_list, new, list_entry);

    return 0;
}

static void tdx_vtpm_server_request_queue_remove(TdxVtpmServer *server,
                                                 TdxVtpmServerPendingRequest *entry)
{
    QLIST_REMOVE(entry, list_entry);

    /*pair with ref in tdx_vtpm_client_session_request_queue_push() */
    tdx_vmcall_service_item_unref(entry->vsi);

    g_free(entry);
}

static bool td_user_id_equal(const TdUserId *a, const TdUserId *b)
{
    return !memcmp(a, b, sizeof(*a));
}

static int tdx_vtpm_server_handle_wait_for_request(TdxVtpmServer *server,
                                                   TdxVmcallServiceItem *vsi, bool add);
static void tdx_vtpm_server_check_pending_request(TdxVtpmServer *server)
{
    TdxVtpmServerPendingRequest *request;
    int ret;

    QLIST_FOREACH(request, &server->request_list, list_entry) {
        ret = tdx_vtpm_server_handle_wait_for_request(server, request->vsi, false);
        if (ret) {
            continue;
        }
        tdx_vtpm_server_request_queue_remove(server, request);
        break;
    }
}

static int
tdx_vtpm_client_session_data_index_add(TdxVtpmServer *server,
                                       TdUserId *user_id)
{
    TdxVtpmServerSessionDataIndex *new;

    new = g_malloc(sizeof(*new));
    if (!new)
        return -1;

    memcpy(&new->user_id, user_id, sizeof(new->user_id));
    QSIMPLEQ_INSERT_TAIL(&server->session_data_index, new, queue_entry);

    return 0;
}

static TdxVtpmServerSessionDataIndex *
tdx_vtpm_client_session_data_index_get(TdxVtpmServer *server)
{
    if (QSIMPLEQ_EMPTY(&server->session_data_index)) {
        return NULL;
    }
    return QSIMPLEQ_FIRST(&server->session_data_index);
}

static void
tdx_vtpm_client_session_data_index_remove(TdxVtpmServer *server,
                                          TdxVtpmServerSessionDataIndex *node)
{
    if (!node)
        return;

    if (QSIMPLEQ_EMPTY(&server->session_data_index))
        return;

    QSIMPLEQ_REMOVE(&server->session_data_index, node,
                    TdxVtpmServerSessionDataIndex, queue_entry);

    g_free(node);
}

static void tdx_vtpm_server_handle_trans_protocol_data(TdxVtpmServer *server,
                                                       void *buf, int size)
{
    TdxVtpmTransProtocolData *data = buf;
    TdUserId *user_id = (TdUserId*)data->user_id;
    TdxVtpmServerClientSession *session;
    TdxVtpmServerClientDataEntry entry;
    struct UnixSocketAddress addr;
    char path[PATH_MAX];
    int ret;

    addr.path = path;
    qio_channel_socket_get_dgram_recv_address(server->parent.ioc, &addr);

    session = g_hash_table_lookup(server->client_session, user_id);
    if (!session) {
        session = tdx_vtpm_server_create_client_session(server,
                                                        user_id, &addr);
        if (!session) {
            error_report("Failed to create client session, data dropped");
            goto fail;
        }
    } else {
        if (tdx_vtpm_server_client_session_set_addr(session, &addr)) {
            error_report("Failed to update session peer address, will lose later message to client");
        }
    }

    entry.operation = TDX_VTPM_OPERATION_COMM;
    entry.buf = data->data;
    entry.buf_size = trans_protocol_data_payload_size(data);
    ret = tdx_vtpm_client_session_data_queue_add(server, &session->user_id, &entry);
    if (ret) {
        error_report("Failed to push data entry");
        goto fail;
    }
    ret = tdx_vtpm_client_session_data_index_add(server, &session->user_id);
    if (ret) {
        error_report("Failed to push request session entry");
        goto fail_free;
    }

    VMCALL_DEBUG("Received Data:\n");
    VMCALL_DUMP_USER_ID(&session->user_id);
    VMCALL_DUMP_DATA(entry.buf, entry.buf_size);

    tdx_vtpm_server_check_pending_request(server);

 fail:
    qemu_mutex_unlock(&server->lock);
    return;
 fail_free:
    tdx_vtpm_client_session_data_queue_remove(server, &session->user_id);
    qemu_mutex_unlock(&server->lock);
    return;
}

static void tdx_vtpm_server_handle_trans_protocol(TdxVtpmServer *server,
                                                  void *buf, int size)
{
    TdxVtpmTransProtocolHead *head = buf;

    switch(head->type) {
    case TDX_VTPM_TRANS_PROTOCOL_TYPE_DATA:
        VMCALL_DEBUG("<Socket.WaitForRequest> BEGIN\n");
        tdx_vtpm_server_handle_trans_protocol_data(server, buf, size);
        VMCALL_DEBUG("<Socket.WaitForRequest> END\n");
        break;
    default:
        error_report("Not implemented trans protocol type: %d", head->type);
    }
}

static void tdx_vtpm_server_handle_recv_data(TdxVtpmServer *server)
{
    TdxVtpmTransProtocolHead *head;
    QIOChannelSocket *ioc = server->parent.ioc;
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
        error_report("Read trans protocol fail: %d", read_size);
        /*
         * TODO: tdx_vtpm_server_client_disconnect() when change to
         *       STREAM socket.
         */
        goto out;
    }

    head = data;
    if (head->length > TDX_VTPM_TRANS_PROTOCOL_MAX_LEN) {
        error_report("Exceed max len of trans protocol:%d", head->length);
        exit(-1);
    }

    /*handle the received trans protocol here*/
    tdx_vtpm_server_handle_trans_protocol(server, data, read_size);

 out:
    g_free(data);
    return;
}

static void tdx_vtpm_socket_server_recv(void *opaque)
{
    TdxVtpmServer *server = opaque;

    qemu_mutex_lock(&server->lock);

    tdx_vtpm_server_handle_recv_data(server);

    qemu_mutex_unlock(&server->lock);
}

static void tdx_vtpm_server_wait_for_request_timeout_handler(TdxVmcallServiceItem *vsi,
                                                             void *opaque)
{
    TdxVtpmServer *server = opaque;
    TdxVtpmServerPendingRequest *found = NULL;
    TdxVtpmServerPendingRequest *i;

    qemu_mutex_lock(&server->lock);

    QLIST_FOREACH(i, &server->request_list, list_entry) {
        if (vsi != i->vsi)
            continue;

        found = i;
        break;
    }

    g_assert(found);
    tdx_vtpm_server_request_queue_remove(server, found);

    qemu_mutex_unlock(&server->lock);
}

static void tdx_vtpm_prepare_wait_for_request_response(TdxVtpmRspWaitForRequest *rsp,
                                                       int operation, TdUserId *user_id,
                                                       void *data, int size)
{
    rsp->head = tdx_vtpm_init_comm_head(TDX_VTPM_WAIT_FOR_REQUEST);
    rsp->operation = operation;
    memcpy(&rsp->user_id, user_id, sizeof(rsp->user_id));
    if (data && size)
        memcpy(rsp->data, data, size);
}

static void tdx_vtpm_server_fire_wait_for_request(TdxVtpmServer *server,
                                                  TdUserId *user_id,
                                                  TdxVtpmServerClientDataEntry *entry,
                                                  TdxVtpmServerSessionDataIndex *session_data_index,
                                                  TdxVmcallServiceItem *vsi)
{
    TdxVtpmRspWaitForRequest *rsp;
    int state = TDG_VP_VMCALL_SERVICE_SUCCESS;
    int size;
    int total_size;

    size = tdx_vmcall_service_rsp_size(vsi);
    total_size = sizeof(TdxVtpmRspWaitForRequest) + entry->buf_size;
    tdx_vmcall_service_set_rsp_size(vsi, total_size);
    if (total_size > size) {
        state = TDG_VP_VMCALL_SERVICE_RSP_BUF_TOO_SMALL;
        VMCALL_DEBUG("Response buffer too small:%d should be at least %d\n", size, total_size);
        goto out;
    }

    VMCALL_DEBUG("Operation:%d Copied %d byte\n", entry->operation, entry->buf_size);
    VMCALL_DUMP_USER_ID(user_id);
    VMCALL_DUMP_DATA(entry->buf, entry->buf_size);

    rsp = tdx_vmcall_service_rsp_buf(vsi);
    tdx_vtpm_prepare_wait_for_request_response(rsp, entry->operation, user_id,
                                               entry->buf, entry->buf_size);
    tdx_vtpm_client_session_data_queue_remove(server, user_id);
    if (session_data_index) {
        tdx_vtpm_client_session_data_index_remove(server, session_data_index);
    }
 out:
    tdx_vmcall_service_set_response_state(vsi, state);
    tdx_vmcall_service_complete_request(vsi);
}

static void tdx_vtpm_server_add_wait_for_request(TdxVtpmServer *server, TdxVmcallServiceItem *vsi)
{
    int ret;
    TdxVtpmServerPendingRequest entry = {0};

    entry.vsi = vsi;
    ret = tdx_vtpm_server_request_queue_add(server, &entry);
    if (ret) {
        VMCALL_DEBUG("Failed to add WaitForRequest request, out of memory\n");
        tdx_vmcall_service_set_response_state(vsi, TDG_VP_VMCALL_SERVICE_OUT_OF_RESOURCE);
        tdx_vmcall_service_complete_request(vsi);
        return;
    }

    VMCALL_DEBUG("Added WaitForRequest request\n");
    tdx_vmcall_service_set_timeout_handler(vsi,
                                           tdx_vtpm_server_wait_for_request_timeout_handler,
                                           server);
}

static gboolean g_hash_find_pending_data(gpointer key, gpointer value, gpointer opaque)
{
    TdUserId *user_id = key;
    TdxVtpmServerClientSessionDataNode *data_node = value;
    struct g_hash_find_opaque *i = opaque;

    if (i->user_id) {
        if (!td_user_id_equal(user_id, i->user_id))
            return false;
    }

    if (QSIMPLEQ_EMPTY(&data_node->data_queue))
        return false;

    memcpy(&i->result, user_id, sizeof(*user_id));
    return true;
}

static int tdx_vtpm_server_handle_wait_for_request(TdxVtpmServer *server,
                                                   TdxVmcallServiceItem *vsi,
                                                   bool add)
{
    TdxVtpmCmdWaitForRequest *cmd;
    TdUserId *user_id = NULL;
    TdxVtpmServerClientDataEntry *entry = NULL;
    TdxVtpmServerSessionDataIndex *session_data_index = NULL;

    cmd = tdx_vmcall_service_cmd_buf(vsi);
    if (td_user_id_equal(&cmd->user_id, &null_user_id)) {
        do {
            tdx_vtpm_client_session_data_index_remove(server, session_data_index);
            session_data_index = tdx_vtpm_client_session_data_index_get(server);
            if (!session_data_index) {
                break;
            }
            entry = tdx_vtpm_client_session_data_queue_get(server,
                                                           &session_data_index->user_id);
        } while (!entry);
        if (session_data_index) {
            user_id = &session_data_index->user_id;
        }
    } else {
        struct g_hash_find_opaque opaque = {};
        gpointer find_ret;

        opaque.user_id = &cmd->user_id;
        find_ret = g_hash_table_find(server->client_data,
                                     g_hash_find_pending_data, &opaque);
        if (find_ret) {
            user_id = &opaque.result;
            entry = tdx_vtpm_client_session_data_queue_get(server, user_id);
        }
    }

    if (user_id) {
        tdx_vtpm_server_fire_wait_for_request(server, user_id,
                                              entry, session_data_index, vsi);
        return 0;
    } else if (add) {
        tdx_vtpm_server_add_wait_for_request(server, vsi);
    }

    return 1;
}

static int tdx_vtpm_server_sanity_check_report_status(TdxVmcallServiceItem *vsi)
{
    TdxVtpmCmdReportStatus *cmd = tdx_vmcall_service_cmd_buf(vsi);
    int cmd_size = tdx_vmcall_service_cmd_size(vsi);
    int rsp_size = tdx_vmcall_service_rsp_size(vsi);

    if (rsp_size < sizeof(TdxVtpmRspReportStatus)) {
        VMCALL_DEBUG("Response buffer size %d shuold > %d\n", rsp_size,
                     sizeof(TdxVtpmRspReportStatus));
        return TDG_VP_VMCALL_SERVICE_BAD_RSP_BUF_SIZE;
    }

    if (cmd->operation > TDX_VTPM_OPERATION_DESTROY) {
        VMCALL_DEBUG("Invalid operation value: %d\n", cmd->operation);
        return TDG_VP_VMCALL_SERVICE_INVALID_OPERAND;
    }

    if (cmd->operation == TDX_VTPM_OPERATION_COMM &&
        cmd_size < sizeof(*cmd)) {
        VMCALL_DEBUG("No payload for operation COMMUNICATION\n");
        return TDG_VP_VMCALL_SERVICE_BAD_CMD_BUF_SIZE;
    }

    if (cmd->operation == TDX_VTPM_OPERATION_CREATE &&
        cmd_size < sizeof(*cmd)) {
        VMCALL_DEBUG("incorrect Command size for operation CREATE:%d should be at least %d\n",
                     cmd_size, sizeof(*cmd));
        return TDG_VP_VMCALL_SERVICE_BAD_CMD_BUF_SIZE;
    }

    if (cmd->operation == TDX_VTPM_OPERATION_DESTROY &&
        cmd_size < sizeof(*cmd)) {
        VMCALL_DEBUG("incorrect Command size for operation DESTROY:%d should be at least %d\n",
                     cmd_size, sizeof(*cmd));
        return TDG_VP_VMCALL_SERVICE_BAD_CMD_BUF_SIZE;
    }

    return TDG_VP_VMCALL_SERVICE_SUCCESS;
}

static int tdx_vtpm_server_send_data_message(TdxVtpmServer *server,
                                             TdxVtpmServerClientSession *session,
                                             uint8_t state,
                                             void *data, int data_size)
{
    TdxVtpmTransProtocolData pack;
    struct iovec i[3];

    pack.head = tdx_vtpm_init_trans_protocol_head(TDX_VTPM_TRANS_PROTOCOL_TYPE_DATA);

    i[0].iov_base = &state;
    i[0].iov_len = sizeof(state);
    i[1].iov_base = &session->user_id;
    i[1].iov_len = sizeof(session->user_id);
    i[2].iov_base = data;
    i[2].iov_len = data_size;

    if (tdx_vtpm_trans_send(server->parent.ioc, &session->client_addr,
                            &pack.head, i, 3)) {
        VMCALL_DEBUG("Failed to send data, may peer disconnected\n");
        return -1;
    }

    VMCALL_DEBUG("Sent data:\n");
    VMCALL_DUMP_USER_ID(&session->user_id);
    VMCALL_DUMP_DATA(data, data_size);

    return 0;
}

static void tdx_vtpm_server_prepare_rsp_report_status(TdxVmcallServiceItem *vsi)
{
    TdxVtpmRspReportStatus *rsp;

    rsp = tdx_vmcall_service_rsp_buf(vsi);

    rsp->head.version = 0;
    rsp->head.command = TDX_VTPM_REPORT_STATUS;
    memset(rsp->reserved, 0, sizeof(rsp->reserved));
}

static void tdx_vtpm_server_client_disconnect(TdxVtpmServer *server,
                                              TdxVtpmServerClientSession *session)
{
    tdx_vtpm_server_destroy_client_session(server, session);
}

static void tdx_vtpm_server_manage_instance_complete(TdxVtpmServer *server,
                                                     TdUserId *user_id, int operation,
                                                     int64_t result);
static void tdx_vtpm_server_handle_report_status(TdxVtpmServer *server, TdxVmcallServiceItem *vsi)
{
    TdxVtpmCmdReportStatus *cmd = tdx_vmcall_service_cmd_buf(vsi);
    int size = tdx_vmcall_service_cmd_size(vsi);
    TdxVtpmServerClientSession *session;
    int ret = TDG_VP_VMCALL_SERVICE_SUCCESS;

    ret = tdx_vtpm_server_sanity_check_report_status(vsi);
    if (ret != TDG_VP_VMCALL_SERVICE_SUCCESS) {
        goto out;
    }

    if (cmd->operation == TDX_VTPM_OPERATION_CREATE ||
        cmd->operation == TDX_VTPM_OPERATION_DESTROY) {
        tdx_vtpm_server_manage_instance_complete(server,
                                                 &cmd->user_id,
                                                 cmd->operation,
                                                 cmd->status);
        goto out;
    }

    if (cmd->operation == TDX_VTPM_OPERATION_COMM) {
        session = g_hash_table_lookup(server->client_session, &cmd->user_id);
        g_assert(session);
        if (tdx_vtpm_server_send_data_message(server, session,
                                              cmd->status,
                                              cmd->data,
                                              tdx_vtpm_cmd_report_status_payload_size(size))) {
            tdx_vtpm_server_client_disconnect(server, session);
            ret = TDG_VP_VMCALL_SERVICE_DEVICE_ERROR;
        }
    }

 out:
    tdx_vtpm_server_prepare_rsp_report_status(vsi);
    tdx_vmcall_service_set_response_state(vsi, ret);
    tdx_vmcall_service_complete_request(vsi);
}

static void tdx_vtpm_server_handle_command(TdxVtpmServer *server, TdxVmcallServiceItem *vsi)
{
    TdxVtpmCommHead *head;
    int64_t size;

    head = tdx_vmcall_service_cmd_buf(vsi);
    size = tdx_vmcall_service_cmd_size(vsi);
    if (!head || !size || size < sizeof(*head)) {
        tdx_vmcall_service_set_response_state(vsi,
                                              TDG_VP_VMCALL_SERVICE_INVALID_OPERAND);
        tdx_vmcall_service_complete_request(vsi);
        error_report("cmd size < cmd's head size");
        return;
    }

    switch(head->command) {
    case TDX_VTPM_WAIT_FOR_REQUEST:
        VMCALL_DEBUG("<WaitForRequest> BEGIN\n");
        tdx_vtpm_server_handle_wait_for_request(server, vsi, true);
        VMCALL_DEBUG("<WaitForRequest> END\n");
        break;
    case TDX_VTPM_REPORT_STATUS:
        VMCALL_DEBUG("<ReportStatus> BEGIN\n");
        tdx_vtpm_server_handle_report_status(server, vsi);
        VMCALL_DEBUG("<ReportStatus> END\n");
        break;
    default:
        error_report("Not implemented cmd: %d", head->command);
        tdx_vmcall_service_set_response_state(vsi,
                                              TDG_VP_VMCALL_SERVICE_INVALID_OPERAND);
        tdx_vmcall_service_complete_request(vsi);
        return;
    }
}

static void tdx_vtpm_vmcall_service_server_handler(TdxVmcallServiceItem *vsi,
                                                   void* opaque)
{
    TdxVtpmServer *server = opaque;

    qemu_mutex_lock(&server->lock);

    tdx_vtpm_server_handle_command(server, vsi);

    qemu_mutex_unlock(&server->lock);

    return;
}


/*Same as g_str_hash() for TdUserId which may has no null byte at end */
static guint g_uuid_hash(gconstpointer key) {
    const signed char *p = key;
    const signed char *end = key + sizeof(TdUserId);
    guint ret = 5381;

    for(p = key; p < end; ++p) {
        ret = ((ret << 5) + ret) + *p;
    }

    return ret;
}

static gboolean g_hash_equal(gconstpointer a, gconstpointer b)
{
    return td_user_id_equal(a, b);
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

    server->client_session = g_hash_table_new_full(g_uuid_hash, g_hash_equal,
                                                   NULL, NULL);
    if (!server->client_session) {
        error_report("Failed to init client hash for vTPM server");
        return -1;
    }

    server->client_data = g_hash_table_new_full(g_uuid_hash, g_hash_equal,
                                                NULL, NULL);
    if (!server->client_data) {
        g_hash_table_destroy(server->client_session);
        error_report("Failed to init client data hash for vTPM server");
        return -1;
    }

    qemu_mutex_init(&server->lock);
    QLIST_INIT(&server->request_list);
    QLIST_INIT(&server->manage_request_list);
    QSIMPLEQ_INIT(&server->session_data_index);

    qemu_uuid_parse(VTPM_SERVICE_TD_GUID, &type->from);
    type->from = qemu_uuid_bswap(type->from);
    type->to = tdx_vtpm_vmcall_service_server_handler;
    type->vsi_size = sizeof(TdxVmcallServiceItem);

    return 0;
}

enum TdxVtpmInstanceManageResult {
    TDX_VTPM_INSTANCE_MANAGE_SUCCESS,
    TDX_VTPM_INSTANCE_MANAGE_INVALID_PARAMETER = 1001,
    TDX_VTPM_INSTANCE_MANAGE_COMMUNICATION_LAYER_ERROR = 1002,
    TDX_VTPM_INSTANCE_MANAGE_REQUEST_NOT_SUPPORT = 1003
};

static TdxVtpmServer *tdx_vtpm_get_server(void)
{
    QemuUUID uuid;
    TdxVmcallServiceType* type;
    MachineState *ms = MACHINE(qdev_get_machine());
    TdxGuest *tdx = TDX_GUEST(ms->cgs);

    if (!tdx)
        return NULL;

    qemu_uuid_parse(VTPM_SERVICE_TD_GUID, &uuid);
    uuid = qemu_uuid_bswap(uuid);

    type = tdx_vmcall_service_find_handler(&uuid,
                                           &tdx->vmcall_service);

    if (!type) {
        return NULL;
    }

    return type->opaque;
}

static int tdx_vtpm_server_parse_user_id(const char *user_id, char *to_user_id)
{
    QemuUUID uuid;

    if (!qemu_uuid_parse(user_id, &uuid)) {
        uuid = qemu_uuid_bswap(uuid);
        memcpy(to_user_id, uuid.data, sizeof(TdUserId));
        return 0;
    } else if (strlen(user_id) <= sizeof(TdUserId)) {
        memset(to_user_id, 0, sizeof(TdUserId));
        strcpy(to_user_id, user_id);
        return 0;
    }

    return -1;
}

static int tdx_vtpm_server_add_manage_pending_request(TdxVtpmServer *server,
                                                      const char *user_id_str,
                                                      TdUserId *user_id,
                                                      int operation)
{
    TdxVtpmServerPendingManageRequest *new;
    size_t n;

    new = g_try_malloc(sizeof(*new));
    if (!new) {
        return -1;
    }

    memcpy(&new->user_id, user_id, sizeof(TdUserId));
    new->operation = operation;
    n = sizeof(new->user_id_str);
    strncpy(new->user_id_str, user_id_str, n - 1);
    new->user_id_str[n - 1] = 0;

    QLIST_INSERT_HEAD(&server->manage_request_list,
                      new, list_entry);

    return 0;
}

static TdxVtpmServerPendingManageRequest*
tdx_vtpm_server_get_manage_pending_request(TdxVtpmServer *server,
                                           TdUserId *user_id,
                                           int operation)
{
    TdxVtpmServerPendingManageRequest *i;

    QLIST_FOREACH(i, &server->manage_request_list, list_entry) {
        if (!td_user_id_equal(&i->user_id, user_id))
            continue;

        if (i->operation != operation)
            continue;

        return i;
    }

    return NULL;
}
static void tdx_vtpm_server_remove_manage_pending_request(TdxVtpmServer *server,
                                                          TdxVtpmServerPendingManageRequest *entry)
{
    if (!entry) {
        return;
    }
    QLIST_REMOVE(entry, list_entry);
    g_free(entry);
}

static void tdx_vtpm_server_manage_instance_complete(TdxVtpmServer *server,
                                                     TdUserId *user_id, int operation,
                                                     int64_t result)
{
    TdxVtpmServerPendingManageRequest *entry;

    entry = tdx_vtpm_server_get_manage_pending_request(server, user_id, operation);
    if (!entry)
        return;

    qapi_event_send_tdx_vtpm_operation_result(entry->user_id_str, result);
    tdx_vtpm_server_remove_manage_pending_request(server, entry);
}

static void
tdx_vtpm_server_manage_instance(const char *user_id, bool create,
                                Error **errp)
{
    TdxVtpmServerPendingManageRequest *i;
    int64_t state = TDX_VTPM_INSTANCE_MANAGE_SUCCESS;
    TdxVtpmServerClientDataEntry entry;
    TdxVtpmServer *server;
    TdUserId td_user_id;
    int ret;

    server = tdx_vtpm_get_server();
    if (!server) {
        state = TDX_VTPM_INSTANCE_MANAGE_REQUEST_NOT_SUPPORT;
        goto out;
    }

    if (tdx_vtpm_server_parse_user_id(user_id, (char*)&td_user_id)) {
        state = TDX_VTPM_INSTANCE_MANAGE_INVALID_PARAMETER;
        goto out;
    }

    memset(&entry, 0, sizeof(entry));
    if (create) {
        entry.operation = TDX_VTPM_OPERATION_CREATE;
    } else {
        entry.operation = TDX_VTPM_OPERATION_DESTROY;
    }

    qemu_mutex_lock(&server->lock);

    ret = tdx_vtpm_server_add_manage_pending_request(server,
                                                     user_id,
                                                     &td_user_id, entry.operation);
    if (ret) {
        goto fail;
    }
    ret = tdx_vtpm_client_session_data_queue_add(server, &td_user_id, &entry);
    if (ret) {
        goto fail_manage_request;
    }
    ret = tdx_vtpm_client_session_data_index_add(server, &td_user_id);
    if (ret) {
        goto fail_session_data;
    }

    tdx_vtpm_server_check_pending_request(server);

    qemu_mutex_unlock(&server->lock);

    return;

 fail_session_data:
    tdx_vtpm_client_session_data_queue_remove(server, &td_user_id);
 fail_manage_request:
    i = tdx_vtpm_server_get_manage_pending_request(server, &td_user_id,
                                                   entry.operation);
    tdx_vtpm_server_remove_manage_pending_request(server, i);
 fail:
    state = TDX_VTPM_INSTANCE_MANAGE_COMMUNICATION_LAYER_ERROR;
    qemu_mutex_unlock(&server->lock);
 out:
    qapi_event_send_tdx_vtpm_operation_result(user_id, state);
    return;
}

void
qmp_tdx_vtpm_create_instance(const char *user_id, Error **errp)
{
    return tdx_vtpm_server_manage_instance(user_id, true, errp);
}

void
qmp_tdx_vtpm_destroy_instance(const char *user_id, Error **errp)
{
    return tdx_vtpm_server_manage_instance(user_id, false, errp);
}
