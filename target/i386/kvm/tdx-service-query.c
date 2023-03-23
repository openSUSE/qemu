#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qom/object_interfaces.h"
#include "sysemu/tdx.h"
#include "hw/i386/x86.h"
#include "kvm_i386.h"
#include "tdx.h"
#include "tdx-vmcall-service.h"

#include "trace.h"

typedef struct TdxServiceQueryCmd {
    unsigned char version;
    unsigned char command;
    unsigned char reserved[2];
    QemuUUID guid;
} QEMU_PACKED TdxServiceQueryCmd;

typedef struct TdxServiceQueryRsp {
    unsigned char version;
    unsigned char command;
    unsigned char status;
    unsigned char reserved;
    QemuUUID guid;
} QEMU_PACKED TdxServiceQueryRsp;

enum TdxServiceQueryCommand {
    TDX_SERVICE_QUERY_CMD_QUERY = 0,
};

#define TDX_SERVICE_QUERY "FB6FC5E1-3378-4ACB-8964-FA5EE43B9C8A"

static int tdx_service_query_sanity_check(TdxVmcallServiceItem *vsi)
{
    int cmd_size = tdx_vmcall_service_cmd_size(vsi);
    int rsp_size = tdx_vmcall_service_rsp_size(vsi);
    TdxServiceQueryCmd *cmd = tdx_vmcall_service_cmd_buf(vsi);
    TdxServiceQueryRsp *rsp = tdx_vmcall_service_rsp_buf(vsi);

    if (cmd_size < sizeof(TdxServiceQueryCmd)) {
        return TDG_VP_VMCALL_SERVICE_BAD_CMD_BUF_SIZE;
    }

    if (rsp_size < sizeof(TdxServiceQueryRsp)) {
        return TDG_VP_VMCALL_SERVICE_BAD_RSP_BUF_SIZE;
    }

    if (cmd->command != TDX_SERVICE_QUERY_CMD_QUERY) {
        return TDG_VP_VMCALL_SERVICE_NOT_SUPPORT;
    }

    if (cmd->version != rsp->version) {
        return TDG_VP_VMCALL_SERVICE_NOT_SUPPORT;
    }

    return TDG_VP_VMCALL_SERVICE_SUCCESS;
}

static void tdx_service_query_handler(TdxVmcallServiceItem *vsi,
                                      void* opaque)
{
    int ret;
    TdxVmcallServiceType* found;
    TdxServiceQueryCmd *cmd;
    TdxServiceQueryRsp *rsp;
    TdxGuest *tdx = opaque;
    QemuUUID guid;

    ret = tdx_service_query_sanity_check(vsi);
    if (ret) {
        goto out;
    }

    cmd = tdx_vmcall_service_cmd_buf(vsi);
    rsp = tdx_vmcall_service_rsp_buf(vsi);

    guid = cmd->guid;
    found = tdx_vmcall_service_find_handler(&guid,
                                            &tdx->vmcall_service);
    rsp->version = 0;
    rsp->command = cmd->command;
    rsp->status = found ? 0 : 1;
    rsp->reserved = 0;
    rsp->guid = cmd->guid;

 out:
    tdx_vmcall_service_set_response_state(vsi, ret);
    tdx_vmcall_service_complete_request(vsi);
}

void tdx_guest_init_service_query(TdxGuest *tdx)
{
    TdxVmcallServiceType type;

    qemu_uuid_parse(TDX_SERVICE_QUERY, &type.from);
    type.from = qemu_uuid_bswap(type.from);
    type.to = tdx_service_query_handler;
    type.opaque = tdx;
    type.vsi_size = sizeof(TdxVmcallServiceItem);

    tdx_vmcall_service_register_type(tdx, &type);
}
