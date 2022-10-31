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

#include "qemu/osdep.h"
#include "qemu/error-report.h"
#include "qemu-file.h"
#include "cgs.h"

static CgsMig cgs_mig;

#define cgs_check_error(f, ret)                                  \
do {                                                             \
    if (ret < 0) {                                               \
        error_report("%s: failed: %s", __func__, strerror(ret)); \
        qemu_file_set_error(f, ret);                             \
        return ret;                                              \
    }                                                            \
} while (0)

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

int cgs_mig_savevm_state_setup(QEMUFile *f)
{
    int ret;

    if (!cgs_mig.savevm_state_setup) {
        return 0;
    }

    ret = cgs_mig.savevm_state_setup();
    cgs_check_error(f, ret);

    return ret;
}
