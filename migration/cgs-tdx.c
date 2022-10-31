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

static bool tdx_mig_is_ready(void)
{
    return tdx_premig_is_done();
}

void tdx_mig_init(CgsMig *cgs_mig)
{
    cgs_mig->is_ready = tdx_mig_is_ready;
}
