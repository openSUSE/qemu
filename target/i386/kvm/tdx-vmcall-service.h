#ifndef QEMU_I386_TDX_VMCALL_SERVICE_H
#define QEMU_I386_TDX_VMCALL_SERVICE_H

#include <stdio.h>
#include <stdarg.h>

#define TDG_VP_VMCALL_SERVICE_SUCCESS                   0
#define TDG_VP_VMCALL_SERVICE_DEVICE_ERROR              1
#define TDG_VP_VMCALL_SERVICE_TIME_OUT                  2
#define TDG_VP_VMCALL_SERVICE_RSP_BUF_TOO_SMALL         3
#define TDG_VP_VMCALL_SERVICE_BAD_CMD_BUF_SIZE          4
#define TDG_VP_VMCALL_SERVICE_BAD_RSP_BUF_SIZE          5
#define TDG_VP_VMCALL_SERVICE_BUSY                      6
#define TDG_VP_VMCALL_SERVICE_INVALID_OPERAND           7
#define TDG_VP_VMCALL_SERVICE_OUT_OF_RESOURCE           8
#define TDG_VP_VMCALL_SERVICE_NOT_SUPPORT               0xFFFFFFFE

const char *vsc_error(int err);

void vmcall_service_printf(const char *format, ...);
void vmcall_service_dump_user_id(void *buf);
void vmcall_service_dump_data(void *buf, int size);

#ifdef VMCALL_SERVICE_DEBUG

#define VMCALL_DEBUG(x, ...) \
    vmcall_service_printf("%s: "x, __func__, ## __VA_ARGS__)
#define VMCALL_DUMP_USER_ID(b) \
    vmcall_service_dump_user_id(b)
#define VMCALL_DUMP_DATA(b, s) \
    vmcall_service_dump_data(b, s)

#else

#define VMCALL_DEBUG(x, ...)
#define VMCALL_DUMP_USER_ID(b)
#define VMCALL_DUMP_DATA(b, s)

#endif

#endif
