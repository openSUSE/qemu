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

#ifndef QEMU_MIGRATION_CGS_H
#define QEMU_MIGRATION_CGS_H
#include "qemu/osdep.h"
#include "migration.h"
#include "multifd.h"

#define CGS_PRIVATE_GPA_INVALID (~0UL)

typedef struct CgsMig {
    bool (*is_ready)(void);
    int (*savevm_state_setup)(uint32_t nr_channels, uint32_t nr_pages);
    int (*savevm_state_start)(QEMUFile *f);
    long (*savevm_state_ram_start_epoch)(QEMUFile *f);
    long (*savevm_state_ram)(QEMUFile *f, uint32_t channel_id, hwaddr gpa);
    int (*savevm_state_pause)(void);
    int (*savevm_state_end)(QEMUFile *f);
    int (*savevm_state_ram_abort)(hwaddr gfn_end);
    long (*savevm_state_ram_cancel)(QEMUFile *f, hwaddr gpa);
    void (*savevm_state_cleanup)(uint32_t nr_channels);
    int (*loadvm_state_setup)(uint32_t nr_channels, uint32_t nr_pages);
    int (*loadvm_state)(QEMUFile *f, uint32_t channel_id);
    void (*loadvm_state_cleanup)(uint32_t nr_channels);
    /* Multifd support */
    uint32_t (*iov_num)(uint32_t page_batch_num);
    int (*multifd_send_prepare)(MultiFDSendParams *p, Error **errp);
    int (*multifd_recv_pages)(MultiFDRecvParams *p, Error **errp);
} CgsMig;

bool cgs_mig_is_ready(void);
int cgs_mig_savevm_state_setup(QEMUFile *f);
int cgs_mig_savevm_state_start(QEMUFile *f);
long cgs_ram_save_start_epoch(QEMUFile *f);
long cgs_mig_savevm_state_ram(QEMUFile *f, uint32_t channel_id,
                              RAMBlock *block, ram_addr_t offset, hwaddr gpa);
bool cgs_mig_savevm_state_need_ram_cancel(void);
long cgs_mig_savevm_state_ram_cancel(QEMUFile *f, RAMBlock *block,
                                     ram_addr_t offset, hwaddr gpa);
int cgs_mig_savevm_state_pause(QEMUFile *f);
int cgs_mig_savevm_state_end(QEMUFile *f);
int cgs_mig_savevm_state_ram_abort(QEMUFile *f, hwaddr gfn_end);
void cgs_mig_savevm_state_cleanup(void);
int cgs_mig_loadvm_state_setup(QEMUFile *f);
int cgs_mig_loadvm_state(QEMUFile *f, uint32_t channel_id);
void cgs_mig_loadvm_state_cleanup(void);
int cgs_mig_multifd_send_prepare(MultiFDSendParams *p, Error **errp);
int cgs_mig_multifd_recv_pages(MultiFDRecvParams *p, Error **errp);
uint32_t cgs_mig_iov_num(uint32_t page_batch_num);
void cgs_mig_init(void);

void tdx_mig_init(CgsMig *cgs_mig);

#endif
