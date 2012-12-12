/*
 * Tar block driver
 *
 * Copyright (c) 2009 Alexander Graf <agraf@suse.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "qemu-common.h"
#include "block/block_int.h"

// #define DEBUG

#ifdef DEBUG
#define dprintf(fmt, ...) do { printf("tar: " fmt, ## __VA_ARGS__); } while (0)
#else
#define dprintf(fmt, ...) do { } while (0)
#endif

#define SECTOR_SIZE      512

#define POSIX_TAR_MAGIC  "ustar"
#define OFFS_LENGTH      0x7c
#define OFFS_TYPE        0x9c
#define OFFS_MAGIC       0x101

#define OFFS_S_SP        0x182
#define OFFS_S_EXT       0x1e2
#define OFFS_S_LENGTH    0x1e3
#define OFFS_SX_EXT      0x1f8

typedef struct SparseCache {
    uint64_t start;
    uint64_t end;
} SparseCache;

typedef struct BDRVTarState {
    BlockDriverState *hd;
    size_t file_sec;
    uint64_t file_len;
    SparseCache *sparse;
    int sparse_num;
    uint64_t last_end;
    char longfile[2048];
} BDRVTarState;

static int tar_probe(const uint8_t *buf, int buf_size, const char *filename)
{
    if (buf_size < OFFS_MAGIC + 5)
        return 0;

    /* we only support newer tar */
    if (!strncmp((char*)buf + OFFS_MAGIC, POSIX_TAR_MAGIC, 5))
        return 100;

    return 0;
}

static int str_ends(char *str, const char *end)
{
    int end_len = strlen(end);
    int str_len = strlen(str);

    if (str_len < end_len)
        return 0;

    return !strncmp(str + str_len - end_len, end, end_len);
}

static int is_target_file(BlockDriverState *bs, char *filename,
                          char *header)
{
    int retval = 0;

    if (str_ends(filename, ".raw"))
        retval = 1;

    if (str_ends(filename, ".qcow"))
        retval = 1;

    if (str_ends(filename, ".qcow2"))
        retval = 1;

    if (str_ends(filename, ".vmdk"))
        retval = 1;

    if (retval &&
        (header[OFFS_TYPE] != '0') &&
        (header[OFFS_TYPE] != 'S')) {
        retval = 0;
    }

    dprintf("does filename %s match? %s\n", filename, retval ? "yes" : "no");

    /* make sure we're not using this name again */
    filename[0] = '\0';

    return retval;
}

static uint64_t tar2u64(char *ptr)
{
    uint64_t retval;
    char oldend = ptr[12];

    ptr[12] = '\0';
    if (*ptr & 0x80) {
        /* XXX we only support files up to 64 bit length */
        retval = be64_to_cpu(*(uint64_t *)(ptr+4));
        dprintf("Convert %lx -> %#lx\n", *(uint64_t*)(ptr+4), retval);
    } else {
        retval = strtol(ptr, NULL, 8);
        dprintf("Convert %s -> %#lx\n", ptr, retval);
    }

    ptr[12] = oldend;

    return retval;
}

static void tar_sparse(BDRVTarState *s, uint64_t offs, uint64_t len)
{
    SparseCache *sparse;

    if (!len)
        return;
    if (!(offs - s->last_end)) {
        s->last_end += len;
        return;
    }
    if (s->last_end > offs)
        return;

    dprintf("Last chunk until %lx new chunk at %lx\n", s->last_end, offs);

    s->sparse = g_realloc(s->sparse, (s->sparse_num + 1) * sizeof(SparseCache));
    sparse = &s->sparse[s->sparse_num];
    sparse->start = s->last_end;
    sparse->end = offs;
    s->last_end = offs + len;
    s->sparse_num++;
    dprintf("Sparse at %lx end=%lx\n", sparse->start,
                                       sparse->end);
}

static int tar_open(BlockDriverState *bs, const char *filename, int flags)
{
    BDRVTarState *s = bs->opaque;
    char header[SECTOR_SIZE];
    char *real_file = header;
    char *magic;
    const char *fname = filename;
    size_t header_offs = 0;
    int ret;

    if (!strncmp(filename, "tar://", 6))
        fname += 6;
    else if (!strncmp(filename, "tar:", 4))
        fname += 4;

    ret = bdrv_file_open(&s->hd, fname, flags);
    if (ret < 0)
        return ret;

    /* Search the file for an image */

    do {
        /* tar header */
        if (bdrv_pread(s->hd, header_offs, header, SECTOR_SIZE) != SECTOR_SIZE)
            goto fail;

        if ((header_offs > 1) && !header[0]) {
            fprintf(stderr, "Tar: No image file found in archive\n");
            goto fail;
        }

        magic = &header[OFFS_MAGIC];
        if (strncmp(magic, POSIX_TAR_MAGIC, 5)) {
            fprintf(stderr, "Tar: Invalid magic: %s\n", magic);
            goto fail;
        }

        dprintf("file type: %c\n", header[OFFS_TYPE]);

        /* file length*/
        s->file_len = (tar2u64(&header[OFFS_LENGTH]) + (SECTOR_SIZE - 1)) &
                      ~(SECTOR_SIZE - 1);
        s->file_sec = (header_offs / SECTOR_SIZE) + 1;

        header_offs += s->file_len + SECTOR_SIZE;

        if (header[OFFS_TYPE] == 'L') {
            bdrv_pread(s->hd, header_offs - s->file_len, s->longfile,
                       sizeof(s->longfile));
            s->longfile[sizeof(s->longfile)-1] = '\0';
            real_file = header;
        } else if (s->longfile[0]) {
            real_file = s->longfile;
        } else {
            real_file = header;
        }
    } while(!is_target_file(bs, real_file, header));

    /* We found an image! */

    if (header[OFFS_TYPE] == 'S') {
        uint8_t isextended;
        int i;

        for (i = OFFS_S_SP; i < (OFFS_S_SP + (4 * 24)); i += 24)
            tar_sparse(s, tar2u64(&header[i]), tar2u64(&header[i+12]));

        s->file_len = tar2u64(&header[OFFS_S_LENGTH]);
        isextended = header[OFFS_S_EXT];

        while (isextended) {
            if (bdrv_pread(s->hd, s->file_sec * SECTOR_SIZE, header,
                           SECTOR_SIZE) != SECTOR_SIZE)
                goto fail;

            for (i = 0; i < (21 * 24); i += 24)
                tar_sparse(s, tar2u64(&header[i]), tar2u64(&header[i+12]));
            isextended = header[OFFS_SX_EXT];
            s->file_sec++;
        }
        tar_sparse(s, s->file_len, 1);
    }

    return 0;

fail:
    fprintf(stderr, "Tar: Error opening file\n");
    bdrv_delete(s->hd);
    return -EINVAL;
}

typedef struct TarAIOCB {
    BlockDriverAIOCB common;
    QEMUBH *bh;
} TarAIOCB;

/* This callback gets invoked when we have pure sparseness */
static void tar_sparse_cb(void *opaque)
{
    TarAIOCB *acb = (TarAIOCB *)opaque;

    acb->common.cb(acb->common.opaque, 0);
    qemu_bh_delete(acb->bh);
    qemu_aio_release(acb);
}

static void tar_aio_cancel(BlockDriverAIOCB *blockacb)
{
}

static AIOCBInfo tar_aiocb_info = {
    .aiocb_size         = sizeof(TarAIOCB),
    .cancel             = tar_aio_cancel,
};

/* This is where we get a request from a caller to read something */
static BlockDriverAIOCB *tar_aio_readv(BlockDriverState *bs,
        int64_t sector_num, QEMUIOVector *qiov, int nb_sectors,
        BlockDriverCompletionFunc *cb, void *opaque)
{
    BDRVTarState *s = bs->opaque;
    SparseCache *sparse;
    int64_t sec_file = sector_num + s->file_sec;
    int64_t start = sector_num * SECTOR_SIZE;
    int64_t end = start + (nb_sectors * SECTOR_SIZE);
    int i;
    TarAIOCB *acb;

    for (i = 0; i < s->sparse_num; i++) {
        sparse = &s->sparse[i];
        if (sparse->start > end) {
            /* We expect the cache to be start increasing */
            break;
        } else if ((sparse->start < start) && (sparse->end <= start)) {
            /* sparse before our offset */
            sec_file -= (sparse->end - sparse->start) / SECTOR_SIZE;
        } else if ((sparse->start <= start) && (sparse->end >= end)) {
            /* all our sectors are sparse */
            char *buf = g_malloc0(nb_sectors * SECTOR_SIZE);

            acb = qemu_aio_get(&tar_aiocb_info, bs, cb, opaque);
            qemu_iovec_from_buf(qiov, 0, buf, nb_sectors * SECTOR_SIZE);
            g_free(buf);
            acb->bh = qemu_bh_new(tar_sparse_cb, acb);
            qemu_bh_schedule(acb->bh);

            return &acb->common;
        } else if (((sparse->start >= start) && (sparse->start < end)) ||
                   ((sparse->end >= start) && (sparse->end < end))) {
            /* we're semi-sparse (worst case) */
            /* let's go synchronous and read all sectors individually */
            char *buf = g_malloc(nb_sectors * SECTOR_SIZE);
            uint64_t offs;

            for (offs = 0; offs < (nb_sectors * SECTOR_SIZE);
                 offs += SECTOR_SIZE) {
                bdrv_pread(bs, (sector_num * SECTOR_SIZE) + offs,
                           buf + offs, SECTOR_SIZE);
            }

            qemu_iovec_from_buf(qiov, 0, buf, nb_sectors * SECTOR_SIZE);
            acb = qemu_aio_get(&tar_aiocb_info, bs, cb, opaque);
            acb->bh = qemu_bh_new(tar_sparse_cb, acb);
            qemu_bh_schedule(acb->bh);

            return &acb->common;
        }
    }

    return bdrv_aio_readv(s->hd, sec_file, qiov, nb_sectors,
                          cb, opaque);
}

static void tar_close(BlockDriverState *bs)
{
    dprintf("Close\n");
}

static int64_t tar_getlength(BlockDriverState *bs)
{
    BDRVTarState *s = bs->opaque;
    dprintf("getlength -> %ld\n", s->file_len);
    return s->file_len;
}

static BlockDriver bdrv_tar = {
    .format_name     = "tar",
    .protocol_name   = "tar",

    .instance_size   = sizeof(BDRVTarState),
    .bdrv_file_open  = tar_open,
    .bdrv_close      = tar_close,
    .bdrv_getlength  = tar_getlength,
    .bdrv_probe      = tar_probe,

    .bdrv_aio_readv  = tar_aio_readv,
};

static void tar_block_init(void)
{
    bdrv_register(&bdrv_tar);
}

block_init(tar_block_init);
