#include "libcnassets.h"

static uint8_t wr_u8(FILE *fp, uint8_t v)
{
    return (fwrite(&v, 1, 1, fp) == 1) ? 0 : 1;
}

static uint8_t wr_u16_be(FILE *fp, uint16_t v)
{
    uint8_t b[2] = {
        (uint8_t)(v >>  8), (uint8_t)(v)
    };
    return (fwrite(b, 1, 2, fp) == 4) ? 0 : 1;
}

static uint8_t wr_u16_le(FILE *fp, uint16_t v)
{
    uint8_t b[2] = {
        (uint8_t)(v), (uint8_t)(v >>  8),
    };
    return (fwrite(b, 1, 2, fp) == 4) ? 0 : 1;
}

static uint8_t wr_u32_be(FILE *fp, uint32_t v)
{
    uint8_t b[4] = {
        (uint8_t)(v >> 24), (uint8_t)(v >> 16),
        (uint8_t)(v >>  8), (uint8_t)(v)
    };
    return (fwrite(b, 1, 4, fp) == 4) ? 0 : 1;
}

static uint8_t wr_u32_le(FILE *fp, uint32_t v)
{
    uint8_t b[4] = {
        (uint8_t)(v), (uint8_t)(v >>  8),
        (uint8_t)(v >> 16), (uint8_t)(v >> 24)
    };
    return (fwrite(b, 1, 4, fp) == 4) ? 0 : 1;
}

static uint8_t wr_u64_be(FILE *fp, uint64_t v)
{
    uint8_t b[8] = {
        (uint8_t)(v >> 56), (uint8_t)(v >> 48),
        (uint8_t)(v >> 40), (uint8_t)(v >> 32),
        (uint8_t)(v >> 24), (uint8_t)(v >> 16),
        (uint8_t)(v >>  8), (uint8_t)(v)
    };
    return (fwrite(b, 1, 8, fp) == 8) ? 0 : 1;
}

static uint8_t wr_u64_le(FILE *fp, uint64_t v)
{
    uint8_t b[8] = {
        (uint8_t)(v), (uint8_t)(v >>  8),
        (uint8_t)(v >> 16), (uint8_t)(v >> 24),
        (uint8_t)(v >> 32), (uint8_t)(v >> 40),
        (uint8_t)(v >> 48), (uint8_t)(v >> 56)
    };
    return (fwrite(b, 1, 8, fp) == 8) ? 0 : 1;
}

struct engine_object_file_writer_ctx_s *new_writer_ctx(const char *name)
{
    struct engine_object_file_writer_ctx_s *wctx = malloc(sizeof(struct engine_object_file_writer_ctx_s));

    if (!wctx)
        return (NULL);

    wctx->object_name = name ? strdup(name) : NULL;
    if (name && !wctx->object_name) {
        (void)free(wctx);
        return (NULL);
    }
    wctx->write_infos.endian = ENGINE_WRT_BIG_ENDIAN;
    wctx->write_infos.type = ENGINE_OBJ_UKN;
    wctx->write_infos.flags = ENGINE_WRT_NULL_FLAG;

    wctx->sections.capacity = 0;
    wctx->sections.size = 0;
    wctx->sections.keys = NULL;
    wctx->sections.content = NULL;
    return (wctx);
}

uint8_t writer_ctx_set_object_name(struct engine_object_file_writer_ctx_s *wctx, const char *name)
{
    if (!wctx)
        return (1);
    if (wctx->object_name)
        (void)free(wctx->object_name);
    wctx->object_name = name ? strdup(name) : NULL;
    if (name && !wctx->object_name)
        return (1);
    return (0);
}

void delete_writer_ctx(struct engine_object_file_writer_ctx_s *wctx)
{
    if (!wctx)
        return;
    if (wctx->object_name)
        (void)free(wctx->object_name);
    (void)free(wctx);
}

static uint32_t flags_to_align(uint32_t flags)
{
    if ((flags & ENGINE_WRT_ALIGN64_FLAG) > 0)
        return (64);
    if ((flags & ENGINE_WRT_ALIGN16_FLAG) > 0)
        return (16);
    if ((flags & ENGINE_WRT_ALIGN8_FLAG) > 0)
        return (8);
    if ((flags & ENGINE_WRT_ALIGN4_FLAG) > 0)
        return (4);
    return (1);
}

static uint8_t write_pad(FILE *fp, uint64_t pos, uint32_t align)
{
    static const uint8_t zeroes[4096];
    uint64_t pad;
    size_t chunk;

    if (align <= 1)
        return (0);

    pad = (align - (pos % align)) % align;
    while (pad > 0) {
        chunk = (pad > sizeof(zeroes)) ? sizeof(zeroes) : (size_t)pad;
        if (fwrite(zeroes, 1, chunk, fp) != chunk)
            return (1);
        pad -= chunk;
    }
    return (0);
}

static uint8_t write_object_file_header(FILE *fp, const io_vtbl_t *io,
                     uint8_t endian, uint32_t flags, uint32_t type,
                     uint64_t sec_hdr_off, uint64_t strndx_off,
                     uint32_t name)
{
    if (wr_u32_be(fp, ENGINE_OBJ_MAGIC))
        return -1;
    if (wr_u8(fp,    endian))
        return -1;
    if (io->u32(fp,  flags))
        return -1;
    if (io->u32(fp,  type))
        return -1;
    if (io->u64(fp,  sec_hdr_off))
        return -1;
    if (io->u64(fp,  strndx_off))
        return -1;
    if (io->u32(fp,  name))
        return -1;
    return 0;
}

uint8_t write_object_file(FILE *fp, const struct engine_object_file_writer_ctx_s *object_file_write_ctx)
{
    (void)object_file_write_ctx;

    (void)write_object_file_header;
    (void)write_pad;
    (void)flags_to_align;
    (void)wr_u64_le;
    (void)wr_u64_be;
    (void)wr_u32_le;
    (void)wr_u32_be;
    (void)wr_u8;
    (void)wr_u16_le;
    (void)wr_u16_be;
    (void)fp;

    return (0);
}