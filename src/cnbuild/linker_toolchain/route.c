#include "linker_toolchain.h"

#include <sys/mman.h>

static void delete_parsed_data(struct engine_object_file_writer_ctx_s *wctx)
{
    (void)wctx;
}

uint8_t parse_cnasset(char *file_path, struct engine_object_file_writer_ctx_s *wctx)
{
    void *data = NULL;

    (void)file_path;
    (void)wctx;
    (void)data;
    return (0);
}

int build_asset_pack(size_t argc, char **argv)
{
    struct engine_object_file_writer_ctx_s *wctx;
    struct build_args_s build_args = {0};
    FILE *fp;

    if (build_get_args(argc - 3, argv + 3, &build_args)) {
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
        fprintf(stderr, "writter ctx allocation failed.\n");
        (void)reset_args(&build_args);
        return (1);
    }

    wctx->write_infos.endian = build_args.endian;
    wctx->write_infos.flags = build_args.padding;

    for (size_t i = 0; i < build_args.input_files.size; ++i) {
        if (parse_cnasset(build_args.input_files.content[i], wctx)) {
            (void)delete_parsed_data(wctx);
            (void)delete_writer_ctx(wctx);
            (void)reset_args(&build_args);
            return (1);
        }
    }

    fp = fopen(build_args.output_file, "w");

    if (!fp) {
        fprintf(stderr, "%s: failed to open output file.\n", build_args.output_file);
        (void)delete_parsed_data(wctx);
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        return (1);
    }
    
    if (write_object_file(fp, wctx)) {
        fprintf(stderr, "%s: failed to write output file.\n", build_args.output_file);
        (void)delete_parsed_data(wctx);
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        (void)fclose(fp);
        return (1);
    }

    (void)fclose(fp);
    (void)delete_writer_ctx(wctx);
    (void)reset_args(&build_args);

    return (0);
}