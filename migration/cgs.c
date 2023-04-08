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
#include "sysemu/kvm.h"
#include "savevm.h"
#include "ram.h"
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
    uint32_t nr_channels = 1, nr_pages = 1;

    if (!cgs_mig.savevm_state_setup) {
        return 0;
    }

    if (migrate_use_multifd()) {
        nr_channels = migrate_multifd_channels();
	nr_pages = MULTIFD_PACKET_SIZE / TARGET_PAGE_SIZE;
    }

    ret = cgs_mig.savevm_state_setup(nr_channels, nr_pages);
    cgs_check_error(f, ret);

    return ret;
}

int cgs_mig_savevm_state_start(QEMUFile *f)
{
    int ret;

    if (!cgs_mig.savevm_state_start) {
        return 0;
    }

    qemu_put_byte(f, QEMU_VM_SECTION_CGS_START);
    ret = cgs_mig.savevm_state_start(f);
    cgs_check_error(f, ret);
    /*
     * Flush the initial message (i.e. QEMU_VM_SECTION_CGS_START + vendor
     * specific data if there is) immediately to have the destinatino side
     * kick off the process as soon as possible.
     */
    if (!ret) {
        qemu_fflush(f);
    }

    return ret;
}

/* Return number of bytes sent or the error value (< 0) */
long cgs_ram_save_start_epoch(QEMUFile *f)
{
    long ret;

    if (!cgs_mig.savevm_state_ram_start_epoch) {
        return 0;
    }

    ram_save_cgs_epoch_header(f);
    ret = cgs_mig.savevm_state_ram_start_epoch(f);
    cgs_check_error(f, ret);

    /* 8 bytes for the cgs header */
    return ret + 8;
}

/* Return number of bytes sent or the error value (< 0) */
long cgs_mig_savevm_state_ram(QEMUFile *f, RAMBlock *block, ram_addr_t offset,
                              hwaddr gpa)
{
    long hdr_bytes, ret;

    if (!cgs_mig.savevm_state_ram) {
        return 0;
    }

    hdr_bytes = ram_save_cgs_ram_header(f, block, offset);
    ret = cgs_mig.savevm_state_ram(f, gpa);
    /*
     * Returning 0 isn't expected. Either succeed with returning bytes of data
     * written to the file or error with a negative error code returned.
     */
    assert(ret);
    cgs_check_error(f, ret);

    return hdr_bytes + ret;
}

int cgs_mig_savevm_state_downtime(QEMUFile *f)
{
    int ret;

    if (!cgs_mig.savevm_state_downtime) {
        return 0;
    }

    ret = cgs_mig.savevm_state_downtime();
    cgs_check_error(f, ret);

    return ret;
}

int cgs_mig_savevm_state_end(QEMUFile *f)
{
    int ret;

    if (!cgs_mig.savevm_state_end) {
        return 0;
    }

    qemu_put_byte(f, QEMU_VM_SECTION_CGS_END);
    ret = cgs_mig.savevm_state_end(f);
    cgs_check_error(f, ret);

    return ret;
}

/* gfn_end indicates the last private page that has been migrated. */
int cgs_mig_savevm_state_ram_cancel(QEMUFile *f, hwaddr gfn_end)
{
    int ret;

    if (!cgs_mig.savevm_state_ram_cancel) {
        return 0;
    }

    ret = cgs_mig.savevm_state_ram_cancel(gfn_end);
    cgs_check_error(f, ret);

    return ret;
}

void cgs_mig_savevm_state_cleanup(void)
{
    uint32_t nr_channels = 1;

    if (!cgs_mig.savevm_state_cleanup) {
        return;
    }

    if (migrate_use_multifd()) {
        nr_channels = migrate_multifd_channels();
    }

    cgs_mig.savevm_state_cleanup(nr_channels);
}

int cgs_mig_loadvm_state_setup(QEMUFile *f)
{
    int ret;
    uint32_t nr_channels = 1, nr_pages = 1;

    if (migrate_use_multifd()) {
        nr_channels = migrate_multifd_channels();
        nr_pages = MULTIFD_PACKET_SIZE / TARGET_PAGE_SIZE;
    }

   if (!cgs_mig.loadvm_state_setup) {
        return 0;
    }

    ret = cgs_mig.loadvm_state_setup(nr_channels, nr_pages);
    cgs_check_error(f, ret);

    return ret;
}

int cgs_mig_loadvm_state(QEMUFile *f)
{
    int ret;

    if (!cgs_mig.loadvm_state) {
        return 0;
    }

    ret = cgs_mig.loadvm_state(f);
    cgs_check_error(f, ret);

    return ret;
}

void cgs_mig_loadvm_state_cleanup(void)
{
    uint32_t nr_channels = 1;

    if (!cgs_mig.loadvm_state_cleanup) {
        return;
    }

    if (migrate_use_multifd()) {
        nr_channels = migrate_multifd_channels();
    }

    cgs_mig.loadvm_state_cleanup(nr_channels);
}

int cgs_mig_multifd_send_prepare(MultiFDSendParams *p, Error **errp)
{

    if (!cgs_mig.multifd_send_prepare) {
        return 0;
    }

    return cgs_mig.multifd_send_prepare(p, errp);
}

int cgs_mig_multifd_recv_pages(MultiFDRecvParams *p, Error **errp)
{
    if (!cgs_mig.multifd_recv_pages) {
        return 0;
    }

    return cgs_mig.multifd_recv_pages(p, errp);
}

uint32_t cgs_mig_iov_num(uint32_t page_batch_num)
{
    if (!cgs_mig.iov_num) {
        return page_batch_num;
    }

    return cgs_mig.iov_num(page_batch_num);
}

void cgs_mig_init(void)
{
    switch (kvm_vm_type) {
    case KVM_X86_TDX_VM:
        tdx_mig_init(&cgs_mig);
        break;
    default:
        return;
    }
}
