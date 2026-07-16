#include "cnsceneobj.h"

static uint64_t build_tree_section_size(struct engine_object_file_section_writer_ctx_s *self)
{
    return ((
        sizeof(uint32_t) + // parent
        sizeof(uint32_t) + // id
        sizeof(uint32_t)) * // type
            ((struct generic_vector_s *)self->_v)->size // count
    );
}

static char *build_tree_section(struct engine_object_file_section_writer_ctx_s *self, const struct engine_object_file_writer_ctx_s *wctx, struct generic_map_s *strndx)
{
    struct generic_vector_s *vec = self->_v;
    struct scene_element_s *temp;
    char *generated = malloc(sizeof(char) * build_tree_section_size(self));
    void (*writter_u32)(char *, uint32_t) = wctx->write_infos.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u32_le : &bufwr_u32_be;
    uint64_t pos = 0;

    if (!generated)
        return (NULL);

    for (size_t i = 0; i < vec->size; ++i) {
        temp = vec->content[i];

        if ((temp->parent_loaded && !temp->parent._o) || (!temp->parent_loaded && !temp->parent._n))
            writter_u32(generated + pos, add_str_table("__main_element", strndx));
        else
            if (temp->parent_loaded)
                writter_u32(generated + pos, add_str_table(temp->parent._o->id, strndx));
            else
                writter_u32(generated + pos, add_str_table(temp->parent._n, strndx));
        pos += sizeof(uint32_t);
        writter_u32(generated + pos, add_str_table(temp->id, strndx));
        pos += sizeof(uint32_t);
        writter_u32(generated + pos, add_str_table(temp->object_type, strndx));
        pos += sizeof(uint32_t);
    }
    return (generated);
}

static char *reconstruct_tree_section_strndx(char *rw_content, uint64_t content_size, const CNAssetReader *reader, struct generic_map_s *new_strndx)
{
    uint64_t pos = 0;
    uint32_t val = 0;
    void (*writter_u32)(char *, uint32_t) = reader->header.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u32_le : &bufwr_u32_be;

    while (pos < content_size) {
        val = add_str_table(object_file_reader_get_string(reader, reader->read_handler.u32(rw_content + pos)), new_strndx); // parent
        if (val == 0xFFFFFFFFu)
            return (NULL);
        (void)writter_u32(rw_content + pos, val);
        pos += sizeof(uint32_t);
    
        val = add_str_table(object_file_reader_get_string(reader, reader->read_handler.u32(rw_content + pos)), new_strndx); // id
        if (val == 0xFFFFFFFFu)
            return (NULL);
        (void)writter_u32(rw_content + pos, val);
        pos += sizeof(uint32_t);
    
        val = add_str_table(object_file_reader_get_string(reader, reader->read_handler.u32(rw_content + pos)), new_strndx); // type
        if (val == 0xFFFFFFFFu)
            return (NULL);
        (void)writter_u32(rw_content + pos, val);
        pos += sizeof(uint32_t);
    }

    return (rw_content);
}

void init_tree_section_registry(struct section_registry *reg)
{
    if (!reg)
        return;

    reg->name = "scene_tree";

    reg->section_type = ENGINE_SEC_SCENE_TREE;

    reg->strndx_reconstructor = &reconstruct_tree_section_strndx;
    reg->data_builder = &build_tree_section;
    reg->size_compute = &build_tree_section_size;
    reg->endian_converter = NULL;
}
