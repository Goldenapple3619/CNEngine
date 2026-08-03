#include "assets_toolchain.h"

static void fclose_wrapper(void *ptr)
{
    fclose((FILE *)ptr);
}

static uint64_t raw_content_size_generator(struct engine_object_file_section_writer_ctx_s *self)
{
    FILE *fp = (FILE *)self->_v;
    int64_t len;
    
    (void)ENGINE_FSEEK(fp, 0, SEEK_END);
    len = ENGINE_FTELL(fp);
    (void)rewind(fp);

    return (len);
}

static char *raw_content_generator(struct engine_object_file_section_writer_ctx_s *self, const struct engine_object_file_writer_ctx_s *writer, struct generic_map_s *strndx)
{
    (void)strndx;
    (void)writer;

    FILE *fp = (FILE *)self->_v;
    size_t len = raw_content_size_generator(self);
    char *buf;
    size_t _;

    buf = malloc(len + 1);

    if (!buf) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate buffer of size %zu for '%s'.", len + 1, self->section_name ? self->section_name : "null");
        return (NULL);
    }

    _ = fread(buf, 1, len, fp);
    (void)_;
    buf[len] = '\0';

    return (buf);
}

static uint8_t create_new_section_asset(struct engine_object_file_writer_ctx_s *wctx, const char *asset_path, FILE *fp)
{
    struct engine_object_file_section_writer_ctx_s *section;

    section = new_writer_section(asset_path, (void *)fp);

    if (!section) {
        PROPAGATE_ERR();
        return (1);
    }

    section->write_infos.type = ENGINE_SEC_RAW;

    if (writer_ctx_add_section(wctx, section)) {
        PROPAGATE_ERR();
        delete_writer_section(section);
        return (1);
    }

    section->content_generator = &raw_content_generator;
    section->content_size_generator = &raw_content_size_generator;

    return (0);
}

struct generic_vector_s *parse_asset(struct generic_vector_s *inputs, struct engine_object_file_writer_ctx_s *wctx, Object *asset_ctx)
{
    (void)asset_ctx;

    struct generic_vector_s *parsed_data;
    FILE *fp;

    if (writer_ctx_set_object_name(wctx, "rwa")) {
        PROPAGATE_ERR();
        return (NULL);
    }

    parsed_data = new_generic_vector();

    if (!parsed_data) {
        PROPAGATE_ERR();
        return (NULL);
    }

    for (size_t i = 0; i < inputs->size; ++i) {
        fp = fopen((const char *)inputs->content[i], "rb");

        if (!fp) {
            RAISE_FMT(ERR_OS, "failed to open '%s'.", (const char *)inputs->content[i]);
            (void)empty_generic_vector(parsed_data, (expr_free)&fclose_wrapper);
            return (NULL);
        }

        if (create_new_section_asset(wctx, inputs->content[i], fp)) {
            PROPAGATE_ERR();
            (void)empty_generic_vector(parsed_data, (expr_free)&fclose_wrapper);
            return (NULL);
        }
    }

    return (parsed_data);

}

int build_assets(size_t argc, char **argv, Object *asset_ctx)
{
    struct engine_object_file_writer_ctx_s *wctx;
    struct build_args_s build_args = {0};
    struct generic_vector_s *left_overs;
    FILE *fp;

    if (build_get_args(argc - 3, argv + 3, "asset", &build_args)) {
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
    wctx->write_infos.type = ENGINE_OBJ_RAW_RESSOURCES;

    left_overs = parse_asset(&build_args.input_files, wctx, asset_ctx);

    if (!left_overs) {
        PROPAGATE_ERR();
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        return (1);
    }

    fp = fopen(build_args.output_file, "wb");

    if (!fp) {
        RAISE_FMT(ERR_OS, "failed to open output file '%s'.", build_args.output_file);
        (void)empty_generic_vector(left_overs, (expr_free)&fclose_wrapper);
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        return (1);
    }
    
    if (write_object_file(fp, wctx)) {
        PROPAGATE_ERR();
        (void)empty_generic_vector(left_overs, (expr_free)&fclose_wrapper);
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        (void)fclose(fp);
        return (1);
    }

    (void)empty_generic_vector(left_overs, (expr_free)&fclose_wrapper);
    (void)fclose(fp);
    (void)delete_writer_ctx(wctx);
    (void)reset_args(&build_args);

    return (0);
}