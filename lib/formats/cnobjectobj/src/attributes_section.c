#include "cnobjectobj.h"

static const char *type_name_from_value(cn_type type)
{
    switch (type) {
        case CN_TYPE_BOOL:
            return ("bool");
        case CN_TYPE_STRING:
            return ("str");
        case CN_TYPE_FLOAT:
            return ("float");
        case CN_TYPE_FUNCTION:
            return ("function");
        case CN_TYPE_GENERIC_UNIQ_PTR:
            return ("data");
        case CN_TYPE_INT:
            return ("int");
        case CN_TYPE_NUMBER:
            return ("number");
        case CN_TYPE_OBJECT:
            return ("object");
        case CN_TYPE_RECT:
            return ("rect");
        case CN_TYPE_VEC2:
            return ("vec2");
        case CN_TYPE_VEC3:
            return ("vec3");
        case CN_TYPE_WEAK_OBJECT:
            return ("weak_object");
        case CN_TYPE_NULL:
            return ("null");
        default:
            return ("ukn");
    }
}

static cn_type value_from_type_name(const char *value)
{
    if (!strcmp(value, "bool"))
        return (CN_TYPE_BOOL);
    if (!strcmp(value, "str"))
        return (CN_TYPE_STRING);
    if (!strcmp(value, "float"))
        return (CN_TYPE_FLOAT);
    if (!strcmp(value, "function"))
        return (CN_TYPE_FUNCTION);
    if (!strcmp(value, "data"))
        return (CN_TYPE_GENERIC_UNIQ_PTR);
    if (!strcmp(value, "int"))
        return (CN_TYPE_INT);
    if (!strcmp(value, "number"))
        return (CN_TYPE_NUMBER);
    if (!strcmp(value, "object"))
        return (CN_TYPE_OBJECT);
    if (!strcmp(value, "rect"))
        return (CN_TYPE_RECT);
    if (!strcmp(value, "vec2"))
        return (CN_TYPE_VEC2);
    if (!strcmp(value, "vec3"))
        return (CN_TYPE_VEC3);
    if (!strcmp(value, "weak_object"))
        return (CN_TYPE_WEAK_OBJECT);
    if (!strcmp(value, "null"))
        return (CN_TYPE_NULL);
    if (!strcmp(value, "ukn"))
        return (CN_TYPE_NULL);
    return (CN_TYPE_NULL);
}

static size_t get_type_size(cn_value *val)
{
    switch(val->type) {
        case CN_TYPE_BOOL:
            return sizeof(uint8_t);
        case CN_TYPE_STRING:
            return sizeof(uint32_t);
        case CN_TYPE_FLOAT:
            return sizeof(uint64_t);
        case CN_TYPE_FUNCTION:
            return sizeof(uint32_t);
        case CN_TYPE_GENERIC_UNIQ_PTR:
            return sizeof(uint32_t);
        case CN_TYPE_INT:
            return sizeof(uint64_t);
        case CN_TYPE_NUMBER:
            return sizeof(uint32_t);
        case CN_TYPE_OBJECT:
            return sizeof(uint32_t);
        case CN_TYPE_RECT:
            return sizeof(uint32_t) * 4;
        case CN_TYPE_VEC2:
            return sizeof(uint32_t) * 2;
        case CN_TYPE_VEC3:
            return sizeof(uint32_t) * 3;
        case CN_TYPE_WEAK_OBJECT:
            return sizeof(uint32_t);
        case CN_TYPE_NULL:
            return 0;
        default:
            return 0;
    }
}

static uint8_t rebuild_strndx_type_value(char *rw_content, uint64_t *pos, const CNAssetReader *reader, struct generic_map_s *new_strndx, cn_type type)
{
    uint32_t val = 0;
    void (*writter_u32)(char *, uint32_t) = reader->header.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u32_le : &bufwr_u32_be;

    switch (type) {
        case CN_TYPE_BOOL:
            return (0);
        case CN_TYPE_STRING:
            val = add_str_table(object_file_reader_get_string(reader, reader->read_handler.u32(rw_content + *pos)), new_strndx);
            if (val == 0xFFFFFFFFu)
                return (1);
            (void)writter_u32(rw_content + *pos, val);
            *pos += sizeof(uint32_t);
            return (0);
        case CN_TYPE_FLOAT:
            return (0);
        case CN_TYPE_FUNCTION:
            val = add_str_table(object_file_reader_get_string(reader, reader->read_handler.u32(rw_content + *pos)), new_strndx);
            if (val == 0xFFFFFFFFu)
                return (1);
            (void)writter_u32(rw_content + *pos, val);
            *pos += sizeof(uint32_t);
            return (0);
        case CN_TYPE_GENERIC_UNIQ_PTR:
            val = add_str_table(object_file_reader_get_string(reader, reader->read_handler.u32(rw_content + *pos)), new_strndx);
            if (val == 0xFFFFFFFFu)
                return (1);
            (void)writter_u32(rw_content + *pos, val);
            *pos += sizeof(uint32_t);
            return (0);
        case CN_TYPE_INT:
            return (0);
        case CN_TYPE_NUMBER:
            return (0);
        case CN_TYPE_OBJECT:
            val = add_str_table(object_file_reader_get_string(reader, reader->read_handler.u32(rw_content + *pos)), new_strndx);
            if (val == 0xFFFFFFFFu)
                return (1);
            (void)writter_u32(rw_content + *pos, val);
            *pos += sizeof(uint32_t);
            return (0);
        case CN_TYPE_RECT:
            return (0);
        case CN_TYPE_VEC2:
            return (0);
        case CN_TYPE_VEC3:
            return (0);
        case CN_TYPE_WEAK_OBJECT:
            val = add_str_table(object_file_reader_get_string(reader, reader->read_handler.u32(rw_content + *pos)), new_strndx);
            if (val == 0xFFFFFFFFu)
                return (1);
            (void)writter_u32(rw_content + *pos, val);
            *pos += sizeof(uint32_t);
            return (0);
        case CN_TYPE_NULL:
            return (0);
        default:
            return (0);
    }
}

static void write_type_value(const struct engine_object_file_writer_ctx_s *wctx, char *generated, uint64_t *pos, struct generic_map_s *strndx, cn_value *type)
{
    void (*writter_u8)(char *, uint8_t) = &bufwr_u8;
    // void (*writter_u16)(char *, uint16_t) = wctx->write_infos.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u16_le : &bufwr_u16_be;
    void (*writter_u32)(char *, uint32_t) = wctx->write_infos.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u32_le : &bufwr_u32_be;
    void (*writter_u64)(char *, uint64_t) = wctx->write_infos.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u64_le : &bufwr_u64_be;

    switch (type->type) {
        case CN_TYPE_BOOL:
            writter_u8(generated + *pos, type->as.b);
            *pos += sizeof(uint8_t);
            return;
        case CN_TYPE_STRING:
            writter_u32(generated + *pos, add_str_table(type->as.str, strndx));
            *pos += sizeof(uint32_t);
            return;
        case CN_TYPE_FLOAT:
            writter_u64(generated + *pos, type->as.f);
            *pos += sizeof(uint64_t);
            return;
        case CN_TYPE_FUNCTION:
            writter_u32(generated + *pos, add_str_table(type->as.str, strndx));
            *pos += sizeof(uint32_t);
            return;
        case CN_TYPE_GENERIC_UNIQ_PTR:
            writter_u32(generated + *pos, add_str_table(type->as.str, strndx));
            *pos += sizeof(uint32_t);
            return;
        case CN_TYPE_INT:
            writter_u64(generated + *pos, type->as.i);
            *pos += sizeof(uint64_t);
            return;
        case CN_TYPE_NUMBER:
            writter_u32(generated + *pos, type->as.num);
            *pos += sizeof(uint32_t);
            return;
        case CN_TYPE_OBJECT:
            writter_u32(generated + *pos, add_str_table(type->as.str, strndx));
            *pos += sizeof(uint32_t);
            return;
        case CN_TYPE_RECT:
            writter_u32(generated + *pos, type->as.rect.x);
            *pos += sizeof(uint32_t);
            writter_u32(generated + *pos, type->as.rect.y);
            *pos += sizeof(uint32_t);
            writter_u32(generated + *pos, type->as.rect.w);
            *pos += sizeof(uint32_t);
            writter_u32(generated + *pos, type->as.rect.h);
            *pos += sizeof(uint32_t);
            return;
        case CN_TYPE_VEC2:
            writter_u32(generated + *pos, type->as.vec2.x);
            *pos += sizeof(uint32_t);
            writter_u32(generated + *pos, type->as.vec2.y);
            *pos += sizeof(uint32_t);
            return;
        case CN_TYPE_VEC3:
            writter_u32(generated + *pos, type->as.vec3.x);
            *pos += sizeof(uint32_t);
            writter_u32(generated + *pos, type->as.vec3.y);
            *pos += sizeof(uint32_t);
            writter_u32(generated + *pos, type->as.vec3.z);
            *pos += sizeof(uint32_t);
            return;
        case CN_TYPE_WEAK_OBJECT:
            writter_u32(generated + *pos, add_str_table(type->as.str, strndx));
            *pos += sizeof(uint32_t);
            return;
        case CN_TYPE_NULL:
            return;
        default:
            return;
    }
}

static uint64_t build_attrs_section_size(struct engine_object_file_section_writer_ctx_s *self)
{
    uint64_t size = 0;

    for (size_t i = 0; i < ((struct object_element_s *)self->_v)->attributes.size; ++i) {
        size += sizeof(uint32_t) + // attribute type
            sizeof(uint32_t) + // attribute name
                get_type_size(&((OBJAttrib *)((struct object_element_s *)self->_v)->attributes.content[i])->value); // value
    }
    return (size);
}

static char *build_attrs_section(struct engine_object_file_section_writer_ctx_s *self, const struct engine_object_file_writer_ctx_s *wctx, struct generic_map_s *strndx)
{
    struct object_element_s *obj = self->_v;
    OBJAttrib *temp;
    char *generated = malloc(sizeof(char) * build_attrs_section_size(self));
    void (*writter_u32)(char *, uint32_t) = wctx->write_infos.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u32_le : &bufwr_u32_be;
    uint64_t pos = 0;

    if (!generated)
        return (NULL);

    for (size_t i = 0; i < obj->attributes.size; ++i) {
        temp = obj->attributes.content[i];

        writter_u32(generated + pos, add_str_table(type_name_from_value(temp->value.type), strndx));
        pos += sizeof(uint32_t);

        writter_u32(generated + pos, add_str_table(temp->name, strndx));
        pos += sizeof(uint32_t);

        (void)write_type_value(wctx, generated, &pos, strndx, &temp->value);
    }
    return (generated);
}

static char *reconstruct_attrs_section_strndx(char *rw_content, uint64_t content_size, const CNAssetReader *reader, struct generic_map_s *new_strndx)
{
    uint64_t pos = 0;
    uint32_t val = 0;
    const char *type;
    void (*writter_u32)(char *, uint32_t) = reader->header.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u32_le : &bufwr_u32_be;

    while (pos < content_size) {
        type = object_file_reader_get_string(reader, reader->read_handler.u32((const void *)(rw_content + pos)));
        val = add_str_table(type, new_strndx);
        if (val == 0xFFFFFFFFu)
            return (NULL);
        (void)writter_u32(rw_content + pos, val);
        pos += sizeof(uint32_t);

        val = add_str_table(object_file_reader_get_string(reader, reader->read_handler.u32(rw_content + pos)), new_strndx);
        if (val == 0xFFFFFFFFu)
            return (NULL);
        (void)writter_u32(rw_content + pos, val);
        pos += sizeof(uint32_t);

        if (rebuild_strndx_type_value(rw_content, &pos, reader, new_strndx, value_from_type_name(type)))
            return (NULL);
    }

    return (rw_content);
}

void init_attrs_section_registry(struct section_registry *reg)
{
    if (!reg)
        return;

    reg->name = "object_attributes";

    reg->section_type = ENGINE_SEC_OBJ_ATTRS;

    reg->strndx_reconstructor = &reconstruct_attrs_section_strndx;
    reg->data_builder = &build_attrs_section;
    reg->size_compute = &build_attrs_section_size;
    reg->endian_converter = NULL;
    reg->extract_data_chunk = NULL;
}
