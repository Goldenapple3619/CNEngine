#include "libcnassets.h"

uint8_t fpwr_u8(FILE *fp, uint8_t v)
{
    return (fwrite(&v, 1, 1, fp) == 1) ? 0 : 1;
}

uint8_t fpwr_u16_be(FILE *fp, uint16_t v)
{
    uint8_t b[2] = {
        (uint8_t)(v >>  8), (uint8_t)(v)
    };
    return (fwrite(b, 1, 2, fp) == 4) ? 0 : 1;
}

uint8_t fpwr_u16_le(FILE *fp, uint16_t v)
{
    uint8_t b[2] = {
        (uint8_t)(v), (uint8_t)(v >>  8),
    };
    return (fwrite(b, 1, 2, fp) == 4) ? 0 : 1;
}

uint8_t fpwr_u32_be(FILE *fp, uint32_t v)
{
    uint8_t b[4] = {
        (uint8_t)(v >> 24), (uint8_t)(v >> 16),
        (uint8_t)(v >>  8), (uint8_t)(v)
    };
    return (fwrite(b, 1, 4, fp) == 4) ? 0 : 1;
}

uint8_t fpwr_u32_le(FILE *fp, uint32_t v)
{
    uint8_t b[4] = {
        (uint8_t)(v), (uint8_t)(v >>  8),
        (uint8_t)(v >> 16), (uint8_t)(v >> 24)
    };
    return (fwrite(b, 1, 4, fp) == 4) ? 0 : 1;
}

uint8_t fpwr_u64_be(FILE *fp, uint64_t v)
{
    uint8_t b[8] = {
        (uint8_t)(v >> 56), (uint8_t)(v >> 48),
        (uint8_t)(v >> 40), (uint8_t)(v >> 32),
        (uint8_t)(v >> 24), (uint8_t)(v >> 16),
        (uint8_t)(v >>  8), (uint8_t)(v)
    };
    return (fwrite(b, 1, 8, fp) == 8) ? 0 : 1;
}

uint8_t fpwr_u64_le(FILE *fp, uint64_t v)
{
    uint8_t b[8] = {
        (uint8_t)(v), (uint8_t)(v >>  8),
        (uint8_t)(v >> 16), (uint8_t)(v >> 24),
        (uint8_t)(v >> 32), (uint8_t)(v >> 40),
        (uint8_t)(v >> 48), (uint8_t)(v >> 56)
    };
    return (fwrite(b, 1, 8, fp) == 8) ? 0 : 1;
}

fpio_handler_t fpio_handler_from_writer(const struct engine_object_file_writer_ctx_s *object_file_write_ctx)
{
    fpio_handler_t io_handler;
    
    io_handler.u8 = &fpwr_u8;

    if (object_file_write_ctx->write_infos.endian == ENGINE_WRT_BIG_ENDIAN) {
        io_handler.u16 = &fpwr_u16_be;
        io_handler.u32 = &fpwr_u32_be;
        io_handler.u64 = &fpwr_u64_be;
    } else {
        io_handler.u16 = &fpwr_u16_le;
        io_handler.u32 = &fpwr_u32_le;
        io_handler.u64 = &fpwr_u64_le;
    }

    return (io_handler);
}

void bufwr_u8(char *buf, uint8_t v)
{
    buf[0] = v;
}

void bufwr_u16_be(char *buf, uint16_t v)
{
    uint8_t b[2] = {
        (uint8_t)(v >>  8), (uint8_t)(v)
    };
    (void)memcpy(buf, b, sizeof(b));
}

void bufwr_u16_le(char *buf, uint16_t v)
{
    uint8_t b[2] = {
        (uint8_t)(v), (uint8_t)(v >>  8),
    };
    (void)memcpy(buf, b, sizeof(b));
}

void bufwr_u32_be(char *buf, uint32_t v)
{
    uint8_t b[4] = {
        (uint8_t)(v >> 24), (uint8_t)(v >> 16),
        (uint8_t)(v >>  8), (uint8_t)(v)
    };
    (void)memcpy(buf, b, sizeof(b));
}

void bufwr_u32_le(char *buf, uint32_t v)
{
    uint8_t b[4] = {
        (uint8_t)(v), (uint8_t)(v >>  8),
        (uint8_t)(v >> 16), (uint8_t)(v >> 24)
    };
    (void)memcpy(buf, b, sizeof(b));
}

void bufwr_u64_be(char *buf, uint64_t v)
{
    uint8_t b[8] = {
        (uint8_t)(v >> 56), (uint8_t)(v >> 48),
        (uint8_t)(v >> 40), (uint8_t)(v >> 32),
        (uint8_t)(v >> 24), (uint8_t)(v >> 16),
        (uint8_t)(v >>  8), (uint8_t)(v)
    };
    (void)memcpy(buf, b, sizeof(b));
}

void bufwr_u64_le(char *buf, uint64_t v)
{
    uint8_t b[8] = {
        (uint8_t)(v), (uint8_t)(v >>  8),
        (uint8_t)(v >> 16), (uint8_t)(v >> 24),
        (uint8_t)(v >> 32), (uint8_t)(v >> 40),
        (uint8_t)(v >> 48), (uint8_t)(v >> 56)
    };
    (void)memcpy(buf, b, sizeof(b));
}
