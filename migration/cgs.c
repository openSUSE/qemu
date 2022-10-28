/*
 * QEMU Migration for Confidential Guest Support
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

static CgsMig cgs_mig;

bool cgs_mig_is_ready(void)
{
    /*
     * For the legacy VM migration and some vendor specific implementations
     * that don't require the check, return true to have the migration flow
     * continue.
     */
    if (!cgs_mig.is_ready) {
        return true;
    }

    return cgs_mig.is_ready();
}
