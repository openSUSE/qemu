#include "tdx-vmcall-service.h"

static struct {
    int error;
    const char *error_str;
} _vsc_error[] = {
#define VSE_ENTRY(x) {.error = x, .error_str = #x}
    VSE_ENTRY(TDG_VP_VMCALL_SERVICE_SUCCESS),
    VSE_ENTRY(TDG_VP_VMCALL_SERVICE_DEVICE_ERROR),
    VSE_ENTRY(TDG_VP_VMCALL_SERVICE_TIME_OUT),
    VSE_ENTRY(TDG_VP_VMCALL_SERVICE_RSP_BUF_TOO_SMALL),
    VSE_ENTRY(TDG_VP_VMCALL_SERVICE_BAD_CMD_BUF_SIZE),
    VSE_ENTRY(TDG_VP_VMCALL_SERVICE_BAD_RSP_BUF_SIZE),
    VSE_ENTRY(TDG_VP_VMCALL_SERVICE_BUSY),
    VSE_ENTRY(TDG_VP_VMCALL_SERVICE_INVALID_OPERAND),
    VSE_ENTRY(TDG_VP_VMCALL_SERVICE_OUT_OF_RESOURCE),
#undef VSE_ENTRY
};

const char *vsc_error(int err) {
    return _vsc_error[err].error_str;
}

void vmcall_service_printf(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    vprintf(format, va);
    va_end(va);
}

void vmcall_service_dump_user_id(void *buf)
{
    unsigned char *p = buf;

    printf("User ID:\n");
    for (int i = 0; i <= 15; ++i) {
        printf("0x%02x ", p[i]);
        if (((i + 1) % 8) == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void vmcall_service_dump_data(void *buf, int size)
{
    unsigned char *p = buf;
    int _size = size;

    if (_size > 32) {
        _size = 32;
    }

    printf("Data (size:%d, only first 32 bytes):\n", size);
    for (int  i = 0; i < _size; ++i) {
        printf("0x%02x ", p[i]);
        if (((i + 1) % 8) == 0) {
            printf("\n");
        }
    }
    printf("\n");
}
