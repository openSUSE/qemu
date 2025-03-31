/*
 * QEMU ram block attribute
 *
 * Copyright Intel
 *
 * Author:
 *      Chenyi Qiang <chenyi.qiang@intel.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory
 *
 */

#include "qemu/osdep.h"
#include "qemu/error-report.h"
#include "exec/ramblock.h"

OBJECT_DEFINE_TYPE_WITH_INTERFACES(RamBlockAttribute,
                                   ram_block_attribute,
                                   RAM_BLOCK_ATTRIBUTE,
                                   OBJECT,
                                   { TYPE_PRIVATE_SHARED_MANAGER },
                                   { })

static size_t ram_block_attribute_get_block_size(const RamBlockAttribute *attr)
{
    /*
     * Because page conversion could be manipulated in the size of at least 4K or 4K aligned,
     * Use the host page size as the granularity to track the memory attribute.
     */
    g_assert(attr && attr->mr && attr->mr->ram_block);
    g_assert(attr->mr->ram_block->page_size == qemu_real_host_page_size());
    return attr->mr->ram_block->page_size;
}


static bool ram_block_attribute_psm_is_shared(const GenericStateManager *gsm,
                                              const MemoryRegionSection *section)
{
    const RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(gsm);
    const int block_size = ram_block_attribute_get_block_size(attr);
    uint64_t first_bit = section->offset_within_region / block_size;
    uint64_t last_bit = first_bit + int128_get64(section->size) / block_size - 1;
    unsigned long first_discard_bit;

    first_discard_bit = find_next_zero_bit(attr->shared_bitmap, last_bit + 1, first_bit);
    return first_discard_bit > last_bit;
}

typedef int (*ram_block_attribute_section_cb)(MemoryRegionSection *s, void *arg);

static int ram_block_attribute_notify_shared_cb(MemoryRegionSection *section, void *arg)
{
    StateChangeListener *scl = arg;

    return scl->notify_to_state_set(scl, section);
}

static int ram_block_attribute_notify_private_cb(MemoryRegionSection *section, void *arg)
{
    StateChangeListener *scl = arg;

    scl->notify_to_state_clear(scl, section);
    return 0;
}

static int ram_block_attribute_for_each_shared_section(const RamBlockAttribute *attr,
                                                       MemoryRegionSection *section,
                                                       void *arg,
                                                       ram_block_attribute_section_cb cb)
{
    unsigned long first_bit, last_bit;
    uint64_t offset, size;
    const int block_size = ram_block_attribute_get_block_size(attr);
    int ret = 0;

    first_bit = section->offset_within_region / block_size;
    first_bit = find_next_bit(attr->shared_bitmap, attr->shared_bitmap_size, first_bit);

    while (first_bit < attr->shared_bitmap_size) {
        MemoryRegionSection tmp = *section;

        offset = first_bit * block_size;
        last_bit = find_next_zero_bit(attr->shared_bitmap, attr->shared_bitmap_size,
                                      first_bit + 1) - 1;
        size = (last_bit - first_bit + 1) * block_size;

        if (!memory_region_section_intersect_range(&tmp, offset, size)) {
            break;
        }

        ret = cb(&tmp, arg);
        if (ret) {
            error_report("%s: Failed to notify RAM discard listener: %s", __func__,
                         strerror(-ret));
            break;
        }

        first_bit = find_next_bit(attr->shared_bitmap, attr->shared_bitmap_size,
                                  last_bit + 2);
    }

    return ret;
}

static int ram_block_attribute_for_each_private_section(const RamBlockAttribute *attr,
                                                        MemoryRegionSection *section,
                                                        void *arg,
                                                        ram_block_attribute_section_cb cb)
{
    unsigned long first_bit, last_bit;
    uint64_t offset, size;
    const int block_size = ram_block_attribute_get_block_size(attr);
    int ret = 0;

    first_bit = section->offset_within_region / block_size;
    first_bit = find_next_zero_bit(attr->shared_bitmap, attr->shared_bitmap_size,
                                   first_bit);

    while (first_bit < attr->shared_bitmap_size) {
        MemoryRegionSection tmp = *section;

        offset = first_bit * block_size;
        last_bit = find_next_bit(attr->shared_bitmap, attr->shared_bitmap_size,
                                      first_bit + 1) - 1;
        size = (last_bit - first_bit + 1) * block_size;

        if (!memory_region_section_intersect_range(&tmp, offset, size)) {
            break;
        }

        ret = cb(&tmp, arg);
        if (ret) {
            error_report("%s: Failed to notify RAM discard listener: %s", __func__,
                         strerror(-ret));
            break;
        }

        first_bit = find_next_zero_bit(attr->shared_bitmap, attr->shared_bitmap_size,
                                       last_bit + 2);
    }

    return ret;
}

static uint64_t ram_block_attribute_psm_get_min_granularity(const GenericStateManager *gsm,
                                                            const MemoryRegion *mr)
{
    const RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(gsm);

    g_assert(mr == attr->mr);
    return ram_block_attribute_get_block_size(attr);
}

static void ram_block_attribute_psm_register_listener(GenericStateManager *gsm,
                                                      StateChangeListener *scl,
                                                      MemoryRegionSection *section)
{
    RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(gsm);
    PrivateSharedListener *psl = container_of(scl, PrivateSharedListener, scl);
    int ret;

    g_assert(section->mr == attr->mr);
    scl->section = memory_region_section_new_copy(section);

    QLIST_INSERT_HEAD(&attr->psl_list, psl, next);

    ret = ram_block_attribute_for_each_shared_section(attr, section, scl,
                                                      ram_block_attribute_notify_shared_cb);
    if (ret) {
        error_report("%s: Failed to register RAM discard listener: %s", __func__,
                     strerror(-ret));
    }
}

static void ram_block_attribute_psm_unregister_listener(GenericStateManager *gsm,
                                                        StateChangeListener *scl)
{
    RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(gsm);
    PrivateSharedListener *psl = container_of(scl, PrivateSharedListener, scl);
    int ret;

    g_assert(scl->section);
    g_assert(scl->section->mr == attr->mr);

    ret = ram_block_attribute_for_each_shared_section(attr, scl->section, scl,
                                                      ram_block_attribute_notify_private_cb);
    if (ret) {
        error_report("%s: Failed to unregister RAM discard listener: %s", __func__,
                     strerror(-ret));
    }

    memory_region_section_free_copy(scl->section);
    scl->section = NULL;
    QLIST_REMOVE(psl, next);
}

typedef struct RamBlockAttributeReplayData {
    ReplayStateChange fn;
    void *opaque;
} RamBlockAttributeReplayData;

static int ram_block_attribute_psm_replay_cb(MemoryRegionSection *section, void *arg)
{
    RamBlockAttributeReplayData *data = arg;

    return data->fn(section, data->opaque);
}

static int ram_block_attribute_psm_replay_on_shared(const GenericStateManager *gsm,
                                                    MemoryRegionSection *section,
                                                    ReplayStateChange replay_fn,
                                                    void *opaque)
{
    RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(gsm);
    RamBlockAttributeReplayData data = { .fn = replay_fn, .opaque = opaque };

    g_assert(section->mr == attr->mr);
    return ram_block_attribute_for_each_shared_section(attr, section, &data,
                                                       ram_block_attribute_psm_replay_cb);
}

static int ram_block_attribute_psm_replay_on_private(const GenericStateManager *gsm,
                                                     MemoryRegionSection *section,
                                                     ReplayStateChange replay_fn,
                                                     void *opaque)
{
    RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(gsm);
    RamBlockAttributeReplayData data = { .fn = replay_fn, .opaque = opaque };

    g_assert(section->mr == attr->mr);
    return ram_block_attribute_for_each_private_section(attr, section, &data,
                                                        ram_block_attribute_psm_replay_cb);
}

static bool ram_block_attribute_is_valid_range(RamBlockAttribute *attr,
                                               uint64_t offset, uint64_t size)
{
    MemoryRegion *mr = attr->mr;

    g_assert(mr);

    uint64_t region_size = memory_region_size(mr);
    int block_size = ram_block_attribute_get_block_size(attr);

    if (!QEMU_IS_ALIGNED(offset, block_size)) {
        return false;
    }
    if (offset + size < offset || !size) {
        return false;
    }
    if (offset >= region_size || offset + size > region_size) {
        return false;
    }
    return true;
}

static void ram_block_attribute_notify_to_private(RamBlockAttribute *attr,
                                                  uint64_t offset, uint64_t size)
{
    PrivateSharedListener *psl;
    int ret;

    QLIST_FOREACH(psl, &attr->psl_list, next) {
        StateChangeListener *scl = &psl->scl;
        MemoryRegionSection tmp = *scl->section;

        if (!memory_region_section_intersect_range(&tmp, offset, size)) {
            continue;
        }
        /*
         * No undo operation for the state_clear() callback failure at present.
         * Expect the state_clear() callback always succeed.
         */
        ret = scl->notify_to_state_clear(scl, &tmp);
        g_assert(!ret);
    }
}

static int ram_block_attribute_notify_to_shared(RamBlockAttribute *attr,
                                                uint64_t offset, uint64_t size)
{
    PrivateSharedListener *psl, *psl2;
    int ret = 0, ret2 = 0;

    QLIST_FOREACH(psl, &attr->psl_list, next) {
        StateChangeListener *scl = &psl->scl;
        MemoryRegionSection tmp = *scl->section;

        if (!memory_region_section_intersect_range(&tmp, offset, size)) {
            continue;
        }
        ret = scl->notify_to_state_set(scl, &tmp);
        if (ret) {
            break;
        }
    }

    if (ret) {
        /* Notify all already-notified listeners. */
        QLIST_FOREACH(psl2, &attr->psl_list, next) {
            StateChangeListener *scl2 = &psl2->scl;
            MemoryRegionSection tmp = *scl2->section;

            if (psl == psl2) {
                break;
            }
            if (!memory_region_section_intersect_range(&tmp, offset, size)) {
                continue;
            }
            /*
             * No undo operation for the state_clear() callback failure at present.
             * Expect the state_clear() callback always succeed.
             */
            ret2 = scl2->notify_to_state_clear(scl2, &tmp);
            g_assert(!ret2);
        }
    }
    return ret;
}

static bool ram_block_attribute_is_range_shared(RamBlockAttribute *attr,
                                                uint64_t offset, uint64_t size)
{
    const int block_size = ram_block_attribute_get_block_size(attr);
    const unsigned long first_bit = offset / block_size;
    const unsigned long last_bit = first_bit + (size / block_size) - 1;
    unsigned long found_bit;

    /* We fake a shorter bitmap to avoid searching too far. */
    found_bit = find_next_zero_bit(attr->shared_bitmap, last_bit + 1, first_bit);
    return found_bit > last_bit;
}

static bool ram_block_attribute_is_range_private(RamBlockAttribute *attr,
                                                uint64_t offset, uint64_t size)
{
    const int block_size = ram_block_attribute_get_block_size(attr);
    const unsigned long first_bit = offset / block_size;
    const unsigned long last_bit = first_bit + (size / block_size) - 1;
    unsigned long found_bit;

    /* We fake a shorter bitmap to avoid searching too far. */
    found_bit = find_next_bit(attr->shared_bitmap, last_bit + 1, first_bit);
    return found_bit > last_bit;
}

static int ram_block_attribute_psm_state_change(PrivateSharedManager *mgr, uint64_t offset,
                                                uint64_t size, bool to_private)
{
    RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(mgr);
    const int block_size = ram_block_attribute_get_block_size(attr);
    const unsigned long first_bit = offset / block_size;
    const unsigned long nbits = size / block_size;
    const uint64_t end = offset + size;
    unsigned long bit;
    uint64_t cur;
    int ret = 0;

    if (!ram_block_attribute_is_valid_range(attr, offset, size)) {
        error_report("%s, invalid range: offset 0x%lx, size 0x%lx",
                     __func__, offset, size);
        return -1;
    }

    if (to_private) {
        if (ram_block_attribute_is_range_private(attr, offset, size)) {
            /* Already private */
        } else if (!ram_block_attribute_is_range_shared(attr, offset, size)) {
            /* Unexpected mixture: process individual blocks */
            for (cur = offset; cur < end; cur += block_size) {
                bit = cur / block_size;
                if (!test_bit(bit, attr->shared_bitmap)) {
                    continue;
                }
                clear_bit(bit, attr->shared_bitmap);
                ram_block_attribute_notify_to_private(attr, cur, block_size);
            }
        } else {
            /* Completely shared */
            bitmap_clear(attr->shared_bitmap, first_bit, nbits);
            ram_block_attribute_notify_to_private(attr, offset, size);
        }
    } else {
        if (ram_block_attribute_is_range_shared(attr, offset, size)) {
            /* Already shared */
        } else if (!ram_block_attribute_is_range_private(attr, offset, size)) {
            /* Unexpected mixture: process individual blocks */
            unsigned long *modified_bitmap = bitmap_new(nbits);

            for (cur = offset; cur < end; cur += block_size) {
                bit = cur / block_size;
                if (test_bit(bit, attr->shared_bitmap)) {
                    continue;
                }
                set_bit(bit, attr->shared_bitmap);
                ret = ram_block_attribute_notify_to_shared(attr, cur, block_size);
                if (!ret) {
                    set_bit(bit - first_bit, modified_bitmap);
                    continue;
                }
                clear_bit(bit, attr->shared_bitmap);
                break;
            }

            if (ret) {
                /*
                 * Very unexpected: something went wrong. Revert to the old
                 * state, marking only the blocks as private that we converted
                 * to shared.
                 */
                for (cur = offset; cur < end; cur += block_size) {
                    bit = cur / block_size;
                    if (!test_bit(bit - first_bit, modified_bitmap)) {
                        continue;
                    }
                    assert(test_bit(bit, attr->shared_bitmap));
                    clear_bit(bit, attr->shared_bitmap);
                    ram_block_attribute_notify_to_private(attr, cur, block_size);
                }
            }
            g_free(modified_bitmap);
        } else {
            /* Complete private */
            bitmap_set(attr->shared_bitmap, first_bit, nbits);
            ret = ram_block_attribute_notify_to_shared(attr, offset, size);
            if (ret) {
                bitmap_clear(attr->shared_bitmap, first_bit, nbits);
            }
        }
    }

    return ret;
}

int ram_block_attribute_realize(RamBlockAttribute *attr, MemoryRegion *mr)
{
    uint64_t shared_bitmap_size;
    const int block_size  = qemu_real_host_page_size();
    int ret;

    shared_bitmap_size = ROUND_UP(mr->size, block_size) / block_size;

    attr->mr = mr;
    ret = memory_region_set_generic_state_manager(mr, GENERIC_STATE_MANAGER(attr));
    if (ret) {
        return ret;
    }
    attr->shared_bitmap_size = shared_bitmap_size;
    attr->shared_bitmap = bitmap_new(shared_bitmap_size);

    return ret;
}

void ram_block_attribute_unrealize(RamBlockAttribute *attr)
{
    g_free(attr->shared_bitmap);
    memory_region_set_generic_state_manager(attr->mr, NULL);
}

static void ram_block_attribute_init(Object *obj)
{
    RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(obj);

    QLIST_INIT(&attr->psl_list);
}

static void ram_block_attribute_finalize(Object *obj)
{
}

static void ram_block_attribute_class_init(ObjectClass *oc, void *data)
{
    GenericStateManagerClass *gsmc = GENERIC_STATE_MANAGER_CLASS(oc);
    PrivateSharedManagerClass *psmc = PRIVATE_SHARED_MANAGER_CLASS(oc);

    gsmc->get_min_granularity = ram_block_attribute_psm_get_min_granularity;
    gsmc->register_listener = ram_block_attribute_psm_register_listener;
    gsmc->unregister_listener = ram_block_attribute_psm_unregister_listener;
    gsmc->is_state_set = ram_block_attribute_psm_is_shared;
    gsmc->replay_on_state_set = ram_block_attribute_psm_replay_on_shared;
    gsmc->replay_on_state_clear = ram_block_attribute_psm_replay_on_private;
    psmc->state_change = ram_block_attribute_psm_state_change;
}
