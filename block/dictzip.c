/*
 * DictZip Block driver for dictzip enabled gzip files
 *
 * Use the "dictzip" tool from the "dictd" package to create gzip files that
 * contain the extra DictZip headers.
 *
 * dictzip(1) is a compression program which creates compressed files in the
 * gzip format (see RFC 1952). However, unlike gzip(1), dictzip(1) compresses
 * the file in pieces and stores an index to the pieces in the gzip header.
 * This allows random access to the file at the granularity of the compressed
 * pieces (currently about 64kB) while maintaining good compression ratios
 * (within 5% of the expected ratio for dictionary data).
 * dictd(8) uses files stored in this format.
 *
 * For details on DictZip see http://dict.org/.
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
#include <zlib.h>

// #define DEBUG

#ifdef DEBUG
#define dprintf(fmt, ...) do { printf("dzip: " fmt, ## __VA_ARGS__); } while (0)
#else
#define dprintf(fmt, ...) do { } while (0)
#endif

#define SECTOR_SIZE 512
#define Z_STREAM_COUNT 4
#define CACHE_COUNT 20

/* magic values */

#define GZ_MAGIC1     0x1f
#define GZ_MAGIC2     0x8b
#define DZ_MAGIC1      'R'
#define DZ_MAGIC2      'A'

#define GZ_FEXTRA     0x04      /* Optional field (random access index)    */
#define GZ_FNAME      0x08      /* Original name                           */
#define GZ_COMMENT    0x10      /* Zero-terminated, human-readable comment */
#define GZ_FHCRC      0x02      /* Header CRC16                            */

/* offsets */

#define GZ_ID            0      /* GZ_MAGIC (16bit)                        */
#define GZ_FLG           3      /* FLaGs (see above)                       */
#define GZ_XLEN         10      /* eXtra LENgth (16bit)                    */
#define GZ_SI           12      /* Subfield ID (16bit)                     */
#define GZ_VERSION      16      /* Version for subfield format             */
#define GZ_CHUNKSIZE    18      /* Chunk size (16bit)                      */
#define GZ_CHUNKCNT     20      /* Number of chunks (16bit)                */
#define GZ_RNDDATA      22      /* Random access data (16bit)              */

#define GZ_99_CHUNKSIZE 18      /* Chunk size (32bit)                      */
#define GZ_99_CHUNKCNT  22      /* Number of chunks (32bit)                */
#define GZ_99_FILESIZE  26      /* Size of unpacked file (64bit)           */
#define GZ_99_RNDDATA   34      /* Random access data (32bit)              */

struct BDRVDictZipState;

typedef struct DictZipAIOCB {
    BlockDriverAIOCB common;
    struct BDRVDictZipState *s;
    QEMUIOVector *qiov;          /* QIOV of the original request */
    QEMUIOVector *qiov_gz;       /* QIOV of the gz subrequest */
    QEMUBH *bh;                  /* BH for cache */
    z_stream *zStream;           /* stream to use for decoding */
    int zStream_id;              /* stream id of the above pointer */
    size_t start;                /* offset into the uncompressed file */
    size_t len;                  /* uncompressed bytes to read */
    uint8_t *gzipped;            /* the gzipped data */
    uint8_t *buf;                /* cached result */
    size_t gz_len;               /* amount of gzip data */
    size_t gz_start;             /* uncompressed starting point of gzip data */
    uint64_t offset;             /* offset for "start" into the uncompressed chunk */
    int chunks_len;              /* amount of uncompressed data in all gzip data */
} DictZipAIOCB;

typedef struct dict_cache {
    size_t start;
    size_t len;
    uint8_t *buf;
} DictCache;

typedef struct BDRVDictZipState {
    BlockDriverState *hd;
    z_stream zStream[Z_STREAM_COUNT];
    DictCache cache[CACHE_COUNT];
    int cache_index;
    uint8_t  stream_in_use;
    uint64_t chunk_len;
    uint32_t chunk_cnt;
    uint16_t *chunks;
    uint32_t *chunks32;
    uint64_t *offsets;
    int64_t file_len;
} BDRVDictZipState;

static int dictzip_probe(const uint8_t *buf, int buf_size, const char *filename)
{
    if (buf_size < 2)
        return 0;

    /* We match on every gzip file */
    if ((buf[0] == GZ_MAGIC1) && (buf[1] == GZ_MAGIC2))
        return 100;

    return 0;
}

static int start_zStream(z_stream *zStream)
{
    zStream->zalloc    = NULL;
    zStream->zfree     = NULL;
    zStream->opaque    = NULL;
    zStream->next_in   = 0;
    zStream->avail_in  = 0;
    zStream->next_out  = NULL;
    zStream->avail_out = 0;

    return inflateInit2( zStream, -15 );
}

static int dictzip_open(BlockDriverState *bs, const char *filename, int flags)
{
    BDRVDictZipState *s = bs->opaque;
    const char *err = "Unknown (read error?)";
    uint8_t magic[2];
    char buf[100];
    uint8_t header_flags;
    uint16_t chunk_len16;
    uint16_t chunk_cnt16;
    uint32_t chunk_len32;
    uint16_t header_ver;
    uint16_t tmp_short;
    uint64_t offset;
    int chunks_len;
    int headerLength = GZ_XLEN - 1;
    int rnd_offs;
    int ret;
    int i;
    const char *fname = filename;

    if (!strncmp(filename, "dzip://", 7))
        fname += 7;
    else if (!strncmp(filename, "dzip:", 5))
        fname += 5;

    ret = bdrv_file_open(&s->hd, fname, flags);
    if (ret < 0)
        return ret;

    /* initialize zlib streams */
    for (i = 0; i < Z_STREAM_COUNT; i++) {
        if (start_zStream( &s->zStream[i] ) != Z_OK) {
            err = s->zStream[i].msg;
            goto fail;
        }
    }

    /* gzip header */
    if (bdrv_pread(s->hd, GZ_ID, &magic, sizeof(magic)) != sizeof(magic))
        goto fail;

    if (!((magic[0] == GZ_MAGIC1) && (magic[1] == GZ_MAGIC2))) {
        err = "No gzip file";
        goto fail;
    }

    /* dzip header */
    if (bdrv_pread(s->hd, GZ_FLG, &header_flags, 1) != 1)
        goto fail;

    if (!(header_flags & GZ_FEXTRA)) {
        err = "Not a dictzip file (wrong flags)";
        goto fail;
    }

    /* extra length */
    if (bdrv_pread(s->hd, GZ_XLEN, &tmp_short, 2) != 2)
        goto fail;

    headerLength += le16_to_cpu(tmp_short) + 2;

    /* DictZip magic */
    if (bdrv_pread(s->hd, GZ_SI, &magic, 2) != 2)
        goto fail;

    if (magic[0] != DZ_MAGIC1 || magic[1] != DZ_MAGIC2) {
        err = "Not a dictzip file (missing extra magic)";
        goto fail;
    }

    /* DictZip version */
    if (bdrv_pread(s->hd, GZ_VERSION, &header_ver, 2) != 2)
        goto fail;

    header_ver = le16_to_cpu(header_ver);

    switch (header_ver) {
        case 1: /* Normal DictZip */
            /* number of chunks */
            if (bdrv_pread(s->hd, GZ_CHUNKSIZE, &chunk_len16, 2) != 2)
                goto fail;

            s->chunk_len = le16_to_cpu(chunk_len16);

            /* chunk count */
            if (bdrv_pread(s->hd, GZ_CHUNKCNT, &chunk_cnt16, 2) != 2)
                goto fail;

            s->chunk_cnt = le16_to_cpu(chunk_cnt16);
            chunks_len = sizeof(short) * s->chunk_cnt;
            rnd_offs = GZ_RNDDATA;
            break;
        case 99: /* Special Alex pigz version */
            /* number of chunks */
            if (bdrv_pread(s->hd, GZ_99_CHUNKSIZE, &chunk_len32, 4) != 4)
                goto fail;

            dprintf("chunk len [%#x] = %d\n", GZ_99_CHUNKSIZE, chunk_len32);
            s->chunk_len = le32_to_cpu(chunk_len32);

            /* chunk count */
            if (bdrv_pread(s->hd, GZ_99_CHUNKCNT, &s->chunk_cnt, 4) != 4)
                goto fail;

            s->chunk_cnt = le32_to_cpu(s->chunk_cnt);

            dprintf("chunk len | count = %d | %d\n", s->chunk_len, s->chunk_cnt);

            /* file size */
            if (bdrv_pread(s->hd, GZ_99_FILESIZE, &s->file_len, 8) != 8)
                goto fail;

            s->file_len = le64_to_cpu(s->file_len);
            chunks_len = sizeof(int) * s->chunk_cnt;
            rnd_offs = GZ_99_RNDDATA;
            break;
        default:
            err = "Invalid DictZip version";
            goto fail;
    }

    /* random access data */
    s->chunks = g_malloc(chunks_len);
    if (header_ver == 99)
        s->chunks32 = (uint32_t *)s->chunks;

    if (bdrv_pread(s->hd, rnd_offs, s->chunks, chunks_len) != chunks_len)
        goto fail;

    /* orig filename */
    if (header_flags & GZ_FNAME) {
        if (bdrv_pread(s->hd, headerLength + 1, buf, sizeof(buf)) != sizeof(buf))
            goto fail;

        buf[sizeof(buf) - 1] = '\0';
        headerLength += strlen(buf) + 1;

        if (strlen(buf) == sizeof(buf))
            goto fail;

        dprintf("filename: %s\n", buf);
    }

    /* comment field */
    if (header_flags & GZ_COMMENT) {
        if (bdrv_pread(s->hd, headerLength, buf, sizeof(buf)) != sizeof(buf))
            goto fail;

        buf[sizeof(buf) - 1] = '\0';
        headerLength += strlen(buf) + 1;

        if (strlen(buf) == sizeof(buf))
            goto fail;

        dprintf("comment: %s\n", buf);
    }

    if (header_flags & GZ_FHCRC)
        headerLength += 2;

    /* uncompressed file length*/
    if (!s->file_len) {
        uint32_t file_len;

        if (bdrv_pread(s->hd, bdrv_getlength(s->hd) - 4, &file_len, 4) != 4)
            goto fail;

        s->file_len = le32_to_cpu(file_len);
    }

    /* compute offsets */
    s->offsets = g_malloc(sizeof( *s->offsets ) * s->chunk_cnt);

    for (offset = headerLength + 1, i = 0; i < s->chunk_cnt; i++) {
        s->offsets[i] = offset;
        switch (header_ver) {
        case 1:
            offset += le16_to_cpu(s->chunks[i]);
            break;
        case 99:
            offset += le32_to_cpu(s->chunks32[i]);
            break;
        }

        dprintf("chunk %#x - %#x = offset %#x -> %#x\n", i * s->chunk_len, (i+1) * s->chunk_len, s->offsets[i], offset);
    }

    return 0;

fail:
    fprintf(stderr, "DictZip: Error opening file: %s\n", err);
    bdrv_delete(s->hd);
    if (s->chunks)
        g_free(s->chunks);
    return -EINVAL;
}

/* This callback gets invoked when we have the result in cache already */
static void dictzip_cache_cb(void *opaque)
{
    DictZipAIOCB *acb = (DictZipAIOCB *)opaque;

    qemu_iovec_from_buf(acb->qiov, 0, acb->buf, acb->len);
    acb->common.cb(acb->common.opaque, 0);
    qemu_bh_delete(acb->bh);
    qemu_aio_release(acb);
}

/* This callback gets invoked by the underlying block reader when we have
 * all compressed data. We uncompress in here. */
static void dictzip_read_cb(void *opaque, int ret)
{
    DictZipAIOCB *acb = (DictZipAIOCB *)opaque;
    struct BDRVDictZipState *s = acb->s;
    uint8_t *buf;
    DictCache *cache;
    int r, i;

    buf = g_malloc(acb->chunks_len);

    /* try to find zlib stream for decoding */
    do {
        for (i = 0; i < Z_STREAM_COUNT; i++) {
            if (!(s->stream_in_use & (1 << i))) {
                s->stream_in_use |= (1 << i);
                acb->zStream_id = i;
                acb->zStream = &s->zStream[i];
                break;
            }
        }
    } while(!acb->zStream);

    /* sure, we could handle more streams, but this callback should be single
       threaded and when it's not, we really want to know! */
    assert(i == 0);

    /* uncompress the chunk */
    acb->zStream->next_in   = acb->gzipped;
    acb->zStream->avail_in  = acb->gz_len;
    acb->zStream->next_out  = buf;
    acb->zStream->avail_out = acb->chunks_len;

    r = inflate( acb->zStream,  Z_PARTIAL_FLUSH );
    if ( (r != Z_OK) && (r != Z_STREAM_END) )
        fprintf(stderr, "Error inflating: [%d] %s\n", r, acb->zStream->msg);

    if ( r == Z_STREAM_END )
        inflateReset(acb->zStream);

    dprintf("inflating [%d] left: %d | %d bytes\n", r, acb->zStream->avail_in, acb->zStream->avail_out);
    s->stream_in_use &= ~(1 << acb->zStream_id);

    /* nofity the caller */
    qemu_iovec_from_buf(acb->qiov, 0, buf + acb->offset, acb->len);
    acb->common.cb(acb->common.opaque, 0);

    /* fill the cache */
    cache = &s->cache[s->cache_index];
    s->cache_index++;
    if (s->cache_index == CACHE_COUNT)
        s->cache_index = 0;

    cache->len = 0;
    if (cache->buf)
        g_free(cache->buf);
    cache->start = acb->gz_start;
    cache->buf = buf;
    cache->len = acb->chunks_len;

    /* free occupied ressources */
    g_free(acb->qiov_gz);
    qemu_aio_release(acb);
}

static void dictzip_aio_cancel(BlockDriverAIOCB *blockacb)
{
}

static const AIOCBInfo dictzip_aiocb_info = {
    .aiocb_size         = sizeof(DictZipAIOCB),
    .cancel             = dictzip_aio_cancel,
};

/* This is where we get a request from a caller to read something */
static BlockDriverAIOCB *dictzip_aio_readv(BlockDriverState *bs,
        int64_t sector_num, QEMUIOVector *qiov, int nb_sectors,
        BlockDriverCompletionFunc *cb, void *opaque)
{
    BDRVDictZipState *s = bs->opaque;
    DictZipAIOCB *acb;
    QEMUIOVector *qiov_gz;
    struct iovec *iov;
    uint8_t *buf;
    size_t  start = sector_num * SECTOR_SIZE;
    size_t  len = nb_sectors * SECTOR_SIZE;
    size_t  end = start + len;
    size_t  gz_start;
    size_t  gz_len;
    int64_t gz_sector_num;
    int     gz_nb_sectors;
    int     first_chunk, last_chunk;
    int     first_offset;
    int     i;

    acb = qemu_aio_get(&dictzip_aiocb_info, bs, cb, opaque);
    if (!acb)
        return NULL;

    /* Search Cache */
    for (i = 0; i < CACHE_COUNT; i++) {
        if (!s->cache[i].len)
            continue;

        if ((start >= s->cache[i].start) &&
            (end <= (s->cache[i].start + s->cache[i].len))) {
            acb->buf = s->cache[i].buf + (start - s->cache[i].start);
            acb->len = len;
            acb->qiov = qiov;
            acb->bh = qemu_bh_new(dictzip_cache_cb, acb);
            qemu_bh_schedule(acb->bh);

            return &acb->common;
        }
    }

    /* No cache, so let's decode */
    /* We need to read these chunks */
    first_chunk  = start / s->chunk_len;
    first_offset = start - first_chunk * s->chunk_len;
    last_chunk   = end / s->chunk_len;

    gz_start = s->offsets[first_chunk];
    gz_len = 0;
    for (i = first_chunk; i <= last_chunk; i++) {
        if (s->chunks32)
            gz_len += le32_to_cpu(s->chunks32[i]);
        else
            gz_len += le16_to_cpu(s->chunks[i]);
    }

    gz_sector_num = gz_start / SECTOR_SIZE;
    gz_nb_sectors = (gz_len / SECTOR_SIZE);

    /* account for tail and heads */
    while ((gz_start + gz_len) > ((gz_sector_num + gz_nb_sectors) * SECTOR_SIZE))
        gz_nb_sectors++;

    /* Allocate qiov, iov and buf in one chunk so we only need to free qiov */
    qiov_gz = g_malloc0(sizeof(QEMUIOVector) + sizeof(struct iovec) +
                           (gz_nb_sectors * SECTOR_SIZE));
    iov = (struct iovec *)(((char *)qiov_gz) + sizeof(QEMUIOVector));
    buf = ((uint8_t *)iov) + sizeof(struct iovec *);

    /* Kick off the read by the backing file, so we can start decompressing */
    iov->iov_base = (void *)buf;
    iov->iov_len = gz_nb_sectors * 512;
    qemu_iovec_init_external(qiov_gz, iov, 1);

    dprintf("read %d - %d => %d - %d\n", start, end, gz_start, gz_start + gz_len);

    acb->s = s;
    acb->qiov = qiov;
    acb->qiov_gz = qiov_gz;
    acb->start = start;
    acb->len = len;
    acb->gzipped = buf + (gz_start % SECTOR_SIZE);
    acb->gz_len = gz_len;
    acb->gz_start = first_chunk * s->chunk_len;
    acb->offset = first_offset;
    acb->chunks_len = (last_chunk - first_chunk + 1) * s->chunk_len;

    return bdrv_aio_readv(s->hd, gz_sector_num, qiov_gz, gz_nb_sectors,
                          dictzip_read_cb, acb);
}

static void dictzip_close(BlockDriverState *bs)
{
    BDRVDictZipState *s = bs->opaque;
    int i;

    for (i = 0; i < CACHE_COUNT; i++) {
        if (!s->cache[i].len)
            continue;

        g_free(s->cache[i].buf);
    }

    for (i = 0; i < Z_STREAM_COUNT; i++) {
        inflateEnd(&s->zStream[i]);
    }

    if (s->chunks)
        g_free(s->chunks);

    if (s->offsets)
        g_free(s->offsets);

    dprintf("Close\n");
}

static int64_t dictzip_getlength(BlockDriverState *bs)
{
    BDRVDictZipState *s = bs->opaque;
    dprintf("getlength -> %ld\n", s->file_len);
    return s->file_len;
}

static BlockDriver bdrv_dictzip = {
    .format_name     = "dzip",
    .protocol_name   = "dzip",

    .instance_size   = sizeof(BDRVDictZipState),
    .bdrv_file_open  = dictzip_open,
    .bdrv_close      = dictzip_close,
    .bdrv_getlength  = dictzip_getlength,
    .bdrv_probe      = dictzip_probe,

    .bdrv_aio_readv  = dictzip_aio_readv,
};

static void dictzip_block_init(void)
{
    bdrv_register(&bdrv_dictzip);
}

block_init(dictzip_block_init);
