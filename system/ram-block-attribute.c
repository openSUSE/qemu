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
#include "system/ramblock.h"

OBJECT_DEFINE_SIMPLE_TYPE_WITH_INTERFACES(RamBlockAttribute,
                                          ram_block_attribute,
                                          RAM_BLOCK_ATTRIBUTE,
                                          OBJECT,
                                          { TYPE_RAM_DISCARD_MANAGER },
                                          { })

static size_t ram_block_attribute_get_block_size(const RamBlockAttribute *attr)
{
    /*
     * Because page conversion could be manipulated in the size of at least 4K
     * or 4K aligned, Use the host page size as the granularity to track the
     * memory attribute.
     */
    g_assert(attr && attr->mr && attr->mr->ram_block);
    g_assert(attr->mr->ram_block->page_size == qemu_real_host_page_size());
    return attr->mr->ram_block->page_size;
}


static bool
ram_block_attribute_rdm_is_populated(const RamDiscardManager *rdm,
                                     const MemoryRegionSection *section)
{
    const RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(rdm);
    const int block_size = ram_block_attribute_get_block_size(attr);
    uint64_t first_bit = section->offset_within_region / block_size;
    uint64_t last_bit = first_bit + int128_get64(section->size) / block_size - 1;
    unsigned long first_discard_bit;

    first_discard_bit = find_next_zero_bit(attr->bitmap, last_bit + 1,
                                           first_bit);
    return first_discard_bit > last_bit;
}

typedef int (*ram_block_attribute_section_cb)(MemoryRegionSection *s,
                                              void *arg);

static int ram_block_attribute_notify_populate_cb(MemoryRegionSection *section,
                                                   void *arg)
{
    RamDiscardListener *rdl = arg;

    return rdl->notify_populate(rdl, section);
}

static int ram_block_attribute_notify_discard_cb(MemoryRegionSection *section,
                                                 void *arg)
{
    RamDiscardListener *rdl = arg;

    rdl->notify_discard(rdl, section);
    return 0;
}

static int
ram_block_attribute_for_each_populated_section(const RamBlockAttribute *attr,
                                               MemoryRegionSection *section,
                                               void *arg,
                                               ram_block_attribute_section_cb cb)
{
    unsigned long first_bit, last_bit;
    uint64_t offset, size;
    const int block_size = ram_block_attribute_get_block_size(attr);
    int ret = 0;

    first_bit = section->offset_within_region / block_size;
    first_bit = find_next_bit(attr->bitmap, attr->bitmap_size,
                              first_bit);

    while (first_bit < attr->bitmap_size) {
        MemoryRegionSection tmp = *section;

        offset = first_bit * block_size;
        last_bit = find_next_zero_bit(attr->bitmap, attr->bitmap_size,
                                      first_bit + 1) - 1;
        size = (last_bit - first_bit + 1) * block_size;

        if (!memory_region_section_intersect_range(&tmp, offset, size)) {
            break;
        }

        ret = cb(&tmp, arg);
        if (ret) {
            error_report("%s: Failed to notify RAM discard listener: %s",
                         __func__, strerror(-ret));
            break;
        }

        first_bit = find_next_bit(attr->bitmap, attr->bitmap_size,
                                  last_bit + 2);
    }

    return ret;
}

static int
ram_block_attribute_for_each_discard_section(const RamBlockAttribute *attr,
                                             MemoryRegionSection *section,
                                             void *arg,
                                             ram_block_attribute_section_cb cb)
{
    unsigned long first_bit, last_bit;
    uint64_t offset, size;
    const int block_size = ram_block_attribute_get_block_size(attr);
    int ret = 0;

    first_bit = section->offset_within_region / block_size;
    first_bit = find_next_zero_bit(attr->bitmap, attr->bitmap_size,
                                   first_bit);

    while (first_bit < attr->bitmap_size) {
        MemoryRegionSection tmp = *section;

        offset = first_bit * block_size;
        last_bit = find_next_bit(attr->bitmap, attr->bitmap_size,
                                 first_bit + 1) - 1;
        size = (last_bit - first_bit + 1) * block_size;

        if (!memory_region_section_intersect_range(&tmp, offset, size)) {
            break;
        }

        ret = cb(&tmp, arg);
        if (ret) {
            error_report("%s: Failed to notify RAM discard listener: %s",
                         __func__, strerror(-ret));
            break;
        }

        first_bit = find_next_zero_bit(attr->bitmap,
                                       attr->bitmap_size,
                                       last_bit + 2);
    }

    return ret;
}

static uint64_t
ram_block_attribute_rdm_get_min_granularity(const RamDiscardManager *rdm,
                                            const MemoryRegion *mr)
{
    const RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(rdm);

    g_assert(mr == attr->mr);
    return ram_block_attribute_get_block_size(attr);
}

static void
ram_block_attribute_rdm_register_listener(RamDiscardManager *rdm,
                                          RamDiscardListener *rdl,
                                          MemoryRegionSection *section)
{
    RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(rdm);
    int ret;

    g_assert(section->mr == attr->mr);
    rdl->section = memory_region_section_new_copy(section);

    QLIST_INSERT_HEAD(&attr->rdl_list, rdl, next);

    ret = ram_block_attribute_for_each_populated_section(attr, section, rdl,
                                    ram_block_attribute_notify_populate_cb);
    if (ret) {
        error_report("%s: Failed to register RAM discard listener: %s",
                     __func__, strerror(-ret));
        exit(1);
    }
}

static void
ram_block_attribute_rdm_unregister_listener(RamDiscardManager *rdm,
                                            RamDiscardListener *rdl)
{
    RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(rdm);
    int ret;

    g_assert(rdl->section);
    g_assert(rdl->section->mr == attr->mr);

    if (rdl->double_discard_supported) {
        rdl->notify_discard(rdl, rdl->section);
    } else {
        ret = ram_block_attribute_for_each_populated_section(attr,
                rdl->section, rdl, ram_block_attribute_notify_discard_cb);
        if (ret) {
            error_report("%s: Failed to unregister RAM discard listener: %s",
                         __func__, strerror(-ret));
            exit(1);
        }
    }

    memory_region_section_free_copy(rdl->section);
    rdl->section = NULL;
    QLIST_REMOVE(rdl, next);
}

typedef struct RamBlockAttributeReplayData {
    ReplayRamDiscardState fn;
    void *opaque;
} RamBlockAttributeReplayData;

static int ram_block_attribute_rdm_replay_cb(MemoryRegionSection *section,
                                             void *arg)
{
    RamBlockAttributeReplayData *data = arg;

    return data->fn(section, data->opaque);
}

static int
ram_block_attribute_rdm_replay_populated(const RamDiscardManager *rdm,
                                         MemoryRegionSection *section,
                                         ReplayRamDiscardState replay_fn,
                                         void *opaque)
{
    RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(rdm);
    RamBlockAttributeReplayData data = { .fn = replay_fn, .opaque = opaque };

    g_assert(section->mr == attr->mr);
    return ram_block_attribute_for_each_populated_section(attr, section, &data,
                                            ram_block_attribute_rdm_replay_cb);
}

static int
ram_block_attribute_rdm_replay_discard(const RamDiscardManager *rdm,
                                       MemoryRegionSection *section,
                                       ReplayRamDiscardState replay_fn,
                                       void *opaque)
{
    RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(rdm);
    RamBlockAttributeReplayData data = { .fn = replay_fn, .opaque = opaque };

    g_assert(section->mr == attr->mr);
    return ram_block_attribute_for_each_discard_section(attr, section, &data,
                                            ram_block_attribute_rdm_replay_cb);
}

RamBlockAttribute *ram_block_attribute_create(MemoryRegion *mr)
{
    uint64_t bitmap_size;
    const int block_size  = qemu_real_host_page_size();
    RamBlockAttribute *attr;
    int ret;

    attr = RAM_BLOCK_ATTRIBUTE(object_new(TYPE_RAM_BLOCK_ATTRIBUTE));

    attr->mr = mr;
    ret = memory_region_set_ram_discard_manager(mr, RAM_DISCARD_MANAGER(attr));
    if (ret) {
        object_unref(OBJECT(attr));
        return NULL;
    }
    bitmap_size = ROUND_UP(mr->size, block_size) / block_size;
    attr->bitmap_size = bitmap_size;
    attr->bitmap = bitmap_new(bitmap_size);

    return attr;
}

void ram_block_attribute_destroy(RamBlockAttribute *attr)
{
    if (!attr) {
        return;
    }

    g_free(attr->bitmap);
    memory_region_set_ram_discard_manager(attr->mr, NULL);
    object_unref(OBJECT(attr));
}

static void ram_block_attribute_init(Object *obj)
{
    RamBlockAttribute *attr = RAM_BLOCK_ATTRIBUTE(obj);

    QLIST_INIT(&attr->rdl_list);
}

static void ram_block_attribute_finalize(Object *obj)
{
}

static void ram_block_attribute_class_init(ObjectClass *klass,
                                           const void *data)
{
    RamDiscardManagerClass *rdmc = RAM_DISCARD_MANAGER_CLASS(klass);

    rdmc->get_min_granularity = ram_block_attribute_rdm_get_min_granularity;
    rdmc->register_listener = ram_block_attribute_rdm_register_listener;
    rdmc->unregister_listener = ram_block_attribute_rdm_unregister_listener;
    rdmc->is_populated = ram_block_attribute_rdm_is_populated;
    rdmc->replay_populated = ram_block_attribute_rdm_replay_populated;
    rdmc->replay_discarded = ram_block_attribute_rdm_replay_discard;
}
