#include "cnobjectobj.h"

static uint64_t build_info_section_size(struct engine_object_file_section_writer_ctx_s *self)
{
    (void)self;

    return (
        sizeof(uint32_t) + // object name
        sizeof(uint32_t) // base name
    );
}

static char *build_info_section(struct engine_object_file_section_writer_ctx_s *self, const struct engine_object_file_writer_ctx_s *wctx, struct generic_map_s *strndx)
{
    struct object_element_s *obj = self->_v;
    char *generated = malloc(sizeof(char) * build_info_section_size(self));
    void (*writter_u32)(char *, uint32_t) = wctx->write_infos.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u32_le : &bufwr_u32_be;
    uint64_t pos = 0;

    if (!generated)
        return (NULL);

    writter_u32(generated + pos, add_str_table(obj->id, strndx));
    pos += sizeof(uint32_t);

    writter_u32(generated + pos, add_str_table(obj->base, strndx));
    pos += sizeof(uint32_t);

    return (generated);
}

static char *reconstruct_info_section_strndx(char *rw_content, uint64_t content_size, const CNAssetReader *reader, struct generic_map_s *new_strndx)
{
    (void)content_size;

    uint64_t pos = 0;
    uint32_t val = 0;
    void (*writter_u32)(char *, uint32_t) = reader->header.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u32_le : &bufwr_u32_be;

    val = add_str_table(object_file_reader_get_string(reader, reader->read_handler.u32((const void *)(rw_content + pos))), new_strndx);
    if (val == 0xFFFFFFFFu)
        return (NULL);
    (void)writter_u32(rw_content + pos, val);
    pos += sizeof(uint32_t);

    val = add_str_table(object_file_reader_get_string(reader, reader->read_handler.u32(rw_content + pos)), new_strndx);
    if (val == 0xFFFFFFFFu)
        return (NULL);
    (void)writter_u32(rw_content + pos, val);
    pos += sizeof(uint32_t);

    return (rw_content);
}

static char *convert_info_section_endian(char *rw_content, uint64_t content_size, const CNAssetReader *reader, engine_wrt_endian endian)
{
    (void)content_size;

    uint64_t pos = 0;
    void (*writter_u32)(char *, uint32_t) = endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u32_le : &bufwr_u32_be;
    uint32_t (*reader_u32)(const void *) = reader->header.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufrd_u32_le: &bufrd_u32_be;

    (void)writter_u32(rw_content + pos, reader_u32((const void *)(rw_content + pos)));
    pos += sizeof(uint32_t);

    (void)writter_u32(rw_content + pos, reader_u32((const void *)(rw_content + pos)));
    pos += sizeof(uint32_t);

    return (rw_content);
}

void init_info_section_registry(struct section_registry *reg)
{
    if (!reg)
        return;

    reg->name = "object_info";

    reg->section_type = ENGINE_SEC_OBJ_INFO;

    reg->strndx_reconstructor = &reconstruct_info_section_strndx;
    reg->data_builder = &build_info_section;
    reg->size_compute = &build_info_section_size;
    reg->endian_converter = &convert_info_section_endian;
}
