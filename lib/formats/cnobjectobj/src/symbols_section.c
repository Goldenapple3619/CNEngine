#include "cnobjectobj.h"

static uint64_t build_symbol_section_size(struct engine_object_file_section_writer_ctx_s *self)
{
    return ((
        sizeof(uint32_t) + // symbol name
        sizeof(uint32_t)) // method name
        * ((struct object_element_s *)self->_v)->methods.size
    );
}

static char *build_symbol_section(struct engine_object_file_section_writer_ctx_s *self, const struct engine_object_file_writer_ctx_s *wctx, struct generic_map_s *strndx)
{
    struct object_element_s *obj = self->_v;
    OBJAttrib *temp;
    char *generated = malloc(sizeof(char) * build_symbol_section_size(self));
    void (*writter_u32)(char *, uint32_t) = wctx->write_infos.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u32_le : &bufwr_u32_be;
    uint64_t pos = 0;

    if (!generated)
        return (NULL);

    for (size_t i = 0; i < obj->methods.size; ++i) {
        temp = obj->methods.content[i];

        writter_u32(generated + pos, add_str_table(temp->value.as.str, strndx));
        pos += sizeof(uint32_t);

        writter_u32(generated + pos, add_str_table(temp->name, strndx));
        pos += sizeof(uint32_t);
    }
    return (generated);
}

static char *reconstruct_symbol_section_strndx(char *rw_content, uint64_t content_size, const CNAssetReader *reader, struct generic_map_s *new_strndx)
{
    uint64_t pos = 0;
    uint32_t val = 0;
    void (*writter_u32)(char *, uint32_t) = reader->header.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u32_le : &bufwr_u32_be;

    while (pos < content_size) {
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
    }

    return (rw_content);
}

static char *convert_symbol_section_endian(char *rw_content, uint64_t content_size, const CNAssetReader *reader, engine_wrt_endian endian)
{
    uint64_t pos = 0;
    void (*writter_u32)(char *, uint32_t) = endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u32_le : &bufwr_u32_be;
    uint32_t (*reader_u32)(const void *) = reader->header.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufrd_u32_le: &bufrd_u32_be;

    while (pos < content_size) {
        (void)writter_u32(rw_content + pos, reader_u32((const void *)(rw_content + pos)));
        pos += sizeof(uint32_t);

        (void)writter_u32(rw_content + pos, reader_u32((const void *)(rw_content + pos)));
        pos += sizeof(uint32_t);
    }

    return (rw_content);
}

static engine_data_extract_status extract_symbols_data_chunk(void **dest, uint64_t *offset, const struct section_blk *section, const CNAssetReader *reader)
{
    if (!dest)
        return (ENGINE_DATA_EXTRACT_ERR);

    if (!*dest) {
        *dest = malloc(sizeof(struct object_element_s));
        if (!*dest)
            return (ENGINE_DATA_EXTRACT_ERR);
        (void)memset(*dest, 0, sizeof(struct object_element_s));
    }

    const char *name;
    const char *symbol;
    OBJAttrib *attr = NULL;

    while (*offset < section->blk_size) {
        if ((*offset) + sizeof(uint32_t) > section->blk_size)
            return (ENGINE_DATA_EXTRACT_ERR);

        symbol = strdup(object_file_reader_get_string(reader, reader->read_handler.u32((const void *)(section->section_blk_ptr + (*offset)))));
        *offset += sizeof(uint32_t);

        if (!symbol) {
            return (ENGINE_DATA_EXTRACT_ERR);
        }

        if ((*offset) + sizeof(uint32_t) > section->blk_size)
            return (ENGINE_DATA_EXTRACT_ERR);

        name = object_file_reader_get_string(reader, reader->read_handler.u32((const void *)(section->section_blk_ptr + (*offset))));
        *offset += sizeof(uint32_t);

        if (!name) {
            return (ENGINE_DATA_EXTRACT_ERR);
        }

        attr = create_object_attribute(name, CN_TYPE_FUNCTION, strdup(symbol));

        if (!attr || !attr->value.as.str) {
            if (attr)
                (void)free(attr);
            return (ENGINE_DATA_EXTRACT_ERR);
        }

        if (insert_generic_vector(&((struct object_element_s *)*dest)->methods, attr)) {
            (void)free(attr->value.as.str);
            (void)free(attr);
            return (ENGINE_DATA_EXTRACT_ERR);
        }
    }

    return (ENGINE_DATA_EXTRACT_COMPLETE);
}

void init_symbols_section_registry(struct section_registry *reg)
{
    if (!reg)
        return;

    reg->name = "object_symbols";

    reg->section_type = ENGINE_SEC_OBJ_SYMBOLS;

    reg->strndx_reconstructor = &reconstruct_symbol_section_strndx;
    reg->data_builder = &build_symbol_section;
    reg->size_compute = &build_symbol_section_size;
    reg->endian_converter = &convert_symbol_section_endian;
    reg->extract_data_chunk = &extract_symbols_data_chunk;
}
