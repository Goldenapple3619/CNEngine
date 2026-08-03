#include "linker_toolchain.h"

static void delete_parsed_data(struct engine_object_file_writer_ctx_s *wctx)
{
    for (size_t i = 0; i < wctx->sections.size; ++i) {
        if (!((struct engine_object_file_section_writer_ctx_s *)wctx->sections.content[i])->_v)
            continue;

        free(((struct engine_object_file_section_writer_ctx_s *)wctx->sections.content[i])->_v);
    }
}

static uint64_t generate_section_size(struct engine_object_file_section_writer_ctx_s *self)
{
    return (((struct section_blk *)self->_v)->blk_size);
}

static char *generate_section_content(struct engine_object_file_section_writer_ctx_s *self, const struct engine_object_file_writer_ctx_s *writer, struct generic_map_s *strndx)
{
    uint64_t size = generate_section_size(self);
    char *section_content = malloc(sizeof(char) * size);
    struct section_registry *temp_reg;

    if (!section_content) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new section content of size %" PRIu64 ".", size);
        return (NULL);
    }

    (void)memcpy(section_content, ((BLKAssetStorage *)self->_v)->_s.section_blk_ptr, size);

    if (!((BLKAssetStorage *)self->_v)->asset_reg || ((BLKAssetStorage *)self->_v)->_s.section_type == ENGINE_SEC_RAW) {
        return (section_content);
    }

    for (size_t i = 0; i < ((BLKAssetStorage *)self->_v)->asset_reg->registered_sections.size; ++i) {
        temp_reg = ((BLKAssetStorage *)self->_v)->asset_reg->registered_sections.content[i];

        if (temp_reg->section_type == ((BLKAssetStorage *)self->_v)->_s.section_type) {
            if (!temp_reg->strndx_reconstructor) {
                RAISE_FMT(WAR_IMPORTANT, "section registry for %" PRIu16 " as no strndx reconstruction callback, skipping.", ((BLKAssetStorage *)self->_v)->_s.section_type);
            } else {
                printf("reconstructing %s.\n", self->section_name);
                temp_reg->strndx_reconstructor(section_content, size, ((BLKAssetStorage *)self->_v)->reader, strndx);
            }

            if (((BLKAssetStorage *)self->_v)->reader->header.endian != writer->write_infos.endian) {
                if (!temp_reg->endian_converter) {
                    RAISE_FMT(WAR_IMPORTANT, "section registry for %" PRIu16 " as no endian converter callback, skipping.", ((BLKAssetStorage *)self->_v)->_s.section_type);
                } else {
                    printf("converting %s.\n", self->section_name);
                    temp_reg->endian_converter(section_content, size, ((BLKAssetStorage *)self->_v)->reader, writer->write_infos.endian);
                }
            }
            return (section_content);
        }
    }

    RAISE_FMT(WAR_IMPORTANT, "no section registry found for %" PRIu16 " no strndx reconstruction/endian conversion will be applied.", ((BLKAssetStorage *)self->_v)->_s.section_type);

    return (section_content);
}


uint8_t parse_cnasset(CNAssetReader *reader, const char *file_path, struct engine_object_file_writer_ctx_s *wctx, Object *asset_ctx)
{
    struct engine_object_file_section_writer_ctx_s *temp_section;
    BLKAssetStorage *temp_blk;
    char *new_section_name;
    const char *base_section_name;
    const char *base_obj_name;

    if (init_object_file_reader(reader, file_path)) {
        PROPAGATE_ERR();
        return (1);
    }

    if (object_file_reader_read_header(reader)) {
        PROPAGATE_ERR();
        return (1);
    }

    if (reader->header.type == ENGINE_OBJ_ASSET_PACK) {
        RAISE(ERR_NOT_COMPATIBLE, "asset pack merge not supported yet.");
        return (1);
    }

    if (object_file_reader_read_section_header(reader)) {
        PROPAGATE_ERR();
        return (1);
    }

    struct asset_registry *reg = call_method(asset_ctx, "find_asset_by_type", PACK_ARG(&reader->header.type)).as.ptr;

    if (!reg && reader->header.type != ENGINE_OBJ_RAW_RESSOURCES) {
        RAISE_FMT(WAR_IMPORTANT, "no asset registry found for %" PRIu16 " no strndx reconstruction/endian conversion will be applied & type treated as raw.", reader->header.type);
    }

    if (!reader->section_header.section_count) {
        RAISE_FMT(WAR_IMPORTANT, "no sections in file '%s'.", file_path);
    }

    for (uint64_t i = 0; i < reader->section_header.section_count; ++i) {
        base_obj_name = object_file_reader_get_string(reader, reader->header.name);

        if (!base_obj_name) {
            PROPAGATE_ERR();
            return (1);
        }

        base_section_name = object_file_reader_get_string(reader, reader->section_header.entries[i].section_name);

        if (!base_section_name) {
            PROPAGATE_ERR();
            return (1);
        }

        new_section_name = malloc(sizeof(char) * ((reg ? strlen(reg->name) : strlen("raw")) + strlen(base_obj_name) + strlen(base_section_name) + 2 + 1));

        if (!new_section_name) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new section name.");
            return (1);
        }

        (void)snprintf(new_section_name, (reg ? strlen(reg->name) : strlen("raw")) + strlen(base_obj_name) + strlen(base_section_name) + 2 + 1, "%s.%s.%s", (reg ? reg->name : "raw"), base_obj_name, base_section_name);

        temp_blk = malloc(sizeof(BLKAssetStorage));

        if (!temp_blk) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new section_blk.");
            (void)free(new_section_name);
            return (1);
        }

        temp_blk->asset_reg = reg;
        temp_blk->reader = reader;
    
        if (object_file_reader_get_section(reader, &temp_blk->_s, i)) {
            PROPAGATE_ERR();
            (void)free(new_section_name);
            (void)free(temp_blk);
            return (1);
        }

        temp_section = new_writer_section(new_section_name, temp_blk);
        (void)free(new_section_name);

        if (!temp_section) {
            PROPAGATE_ERR();
            (void)free(temp_blk);
            return (1);
        }

        temp_section->write_infos.type = reader->section_header.entries[i].section_type;
        temp_section->write_infos.flags = reader->section_header.entries[i].section_flags;
        temp_section->content_generator = &generate_section_content;
        temp_section->content_size_generator = &generate_section_size;

        if (writer_ctx_add_section(wctx, temp_section)) {
            PROPAGATE_ERR();
            delete_writer_section(temp_section);
            return (1);
        }
    }

    return (0);
}

int build_asset_pack(size_t argc, char **argv, Object *asset_ctx)
{
    struct engine_object_file_writer_ctx_s *wctx;
    struct build_args_s build_args = {0};
    struct generic_vector_s vec = {.capacity = 0, .size = 0, .content = NULL};
    CNAssetReader *reader;
    FILE *fp;

    if (build_get_args(argc - 3, argv + 3, "pack", &build_args)) {
        PROPAGATE_ERR();
        (void)reset_args(&build_args);
        return (1);
    }

    if (build_args.input_files.size == 0) {
        fprintf(stderr, "missing input file.\n");
        (void)reset_args(&build_args);
        return (1);
    }

    wctx = new_writer_ctx(NULL);

    if (!wctx) {
        PROPAGATE_ERR();
        (void)reset_args(&build_args);
        return (1);
    }

    wctx->write_infos.endian = build_args.endian;
    wctx->write_infos.flags = build_args.padding;
    wctx->write_infos.type = ENGINE_OBJ_ASSET_PACK;

    for (size_t i = 0; i < build_args.input_files.size; ++i) {
        reader = new_object_file_reader();

        if (!reader) {
            PROPAGATE_ERR();
            (void)delete_parsed_data(wctx);
            (void)delete_writer_ctx(wctx);
            (void)empty_generic_vector(&vec, (expr_free)&delete_object_file_reader);
            (void)reset_args(&build_args);
            return (1);
        }

        if (parse_cnasset(reader, build_args.input_files.content[i], wctx, asset_ctx)) {
            PROPAGATE_ERR();
            (void)delete_object_file_reader(reader);
            (void)delete_parsed_data(wctx);
            (void)delete_writer_ctx(wctx);
            (void)empty_generic_vector(&vec, (expr_free)&delete_object_file_reader);
            (void)reset_args(&build_args);
            return (1);
        }

        if (insert_generic_vector(&vec, reader)) {
            PROPAGATE_ERR();
            (void)delete_object_file_reader(reader);
            (void)delete_parsed_data(wctx);
            (void)delete_writer_ctx(wctx);
            (void)empty_generic_vector(&vec, (expr_free)&delete_object_file_reader);
            (void)reset_args(&build_args);
            return (1);
        }
    }

    fp = fopen(build_args.output_file, "wb");

    if (!fp) {
        RAISE_FMT(ERR_OS, "failed to open output file '%s'.", build_args.output_file);
        (void)delete_parsed_data(wctx);
        (void)delete_writer_ctx(wctx);
        (void)empty_generic_vector(&vec, (expr_free)&delete_object_file_reader);
        (void)reset_args(&build_args);
        return (1);
    }
    
    if (write_object_file(fp, wctx)) {
        PROPAGATE_ERR();
        (void)delete_parsed_data(wctx);
        (void)delete_writer_ctx(wctx);
        (void)empty_generic_vector(&vec, (expr_free)&delete_object_file_reader);
        (void)fclose(fp);
        (void)reset_args(&build_args);
        return (1);
    }

    (void)fclose(fp);
    (void)delete_parsed_data(wctx);
    (void)delete_writer_ctx(wctx);
    (void)empty_generic_vector(&vec, (expr_free)&delete_object_file_reader);
    (void)reset_args(&build_args);

    return (0);
}