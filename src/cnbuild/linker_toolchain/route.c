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
    (void)writer;
    (void)strndx;
    uint64_t size = generate_section_size(self);
    char *section_content = malloc(sizeof(char) * size);

    (void)memcpy(section_content, ((struct section_blk *)self->_v)->section_blk_ptr, size);
    return (section_content);
}


uint8_t parse_cnasset(CNAssetReader *reader, const char *file_path, struct engine_object_file_writer_ctx_s *wctx)
{
    struct engine_object_file_section_writer_ctx_s *temp_section;
    struct section_blk *temp_blk;
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
    if (object_file_reader_read_section_header(reader)) {
        PROPAGATE_ERR();
        return (1);
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

        new_section_name = malloc(sizeof(char) * (strlen(base_obj_name) + strlen(base_section_name) + 1 + 1));

        if (!new_section_name)
            return (1);

        (void)snprintf(new_section_name, strlen(base_obj_name) + strlen(base_section_name) + 1 + 1, "%s.%s", base_obj_name, base_section_name);

        temp_blk = malloc(sizeof(struct section_blk));

        if (!temp_blk) {
            (void)free(new_section_name);
            return (1);
        }
    
        (void)object_file_reader_get_section(reader, temp_blk, i);

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

    (void)asset_ctx;

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

        if (parse_cnasset(reader, build_args.input_files.content[i],  wctx)) {
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