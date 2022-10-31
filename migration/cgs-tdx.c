/*
 * QEMU Migration for Intel TDX Guests
 *
 * Copyright (C) 2022 Intel Corp.
 *
 * Authors:
 *      Wei Wang <wei.w.wang@intel.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 */

#include "cgs.h"
#include "target/i386/kvm/tdx.h"

typedef struct TdxMigStream {
    int fd;
    void *mbmd;
    void *buf_list;
    void *mac_list;
    void *gpa_list;
} TdxMigStream;

typedef struct TdxMigState {
    uint32_t nr_streams;
    TdxMigStream *streams;
} TdxMigState;

TdxMigState tdx_mig;

static bool tdx_mig_is_ready(void)
{
    return tdx_premig_is_done();
}

void tdx_mig_init(CgsMig *cgs_mig, uint32_t nr_channels)
{
    /* Only support 1 migration channel currently */
    assert(nr_channels == 1);
    tdx_mig.nr_streams = nr_channels;
    tdx_mig.streams = g_malloc0(sizeof(struct TdxMigStream) * nr_channels);

    cgs_mig->is_ready = tdx_mig_is_ready;
}
