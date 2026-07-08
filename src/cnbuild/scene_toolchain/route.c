#include "scene_toolchain.h"

int build_scene(size_t argc, char **argv, Object *asset_ctx)
{
    struct engine_object_file_writer_ctx_s *wctx;
    struct build_args_s build_args = {0};
    struct generic_vector_s *left_overs;
    FILE *fp;

    if (build_get_args(argc - 3, argv + 3, "scene", &build_args)) {
        PROPAGATE_ERR();
        (void)reset_args(&build_args);
        return (1);
    }

    if (build_args.input_files.size == 0) {
        fprintf(stderr, "missing input file.\n");
        (void)reset_args(&build_args);
        return (1);
    }

    if (build_args.input_files.size > 1) {
        fprintf(stderr, "too much input files.\n");
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
    wctx->write_infos.type = ENGINE_OBJ_SCN;

    left_overs = parse_scene(build_args.input_files.content[0], wctx, asset_ctx);

    if (!left_overs) {
        PROPAGATE_ERR();
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        return (1);
    }

    fp = fopen(build_args.output_file, "wb");

    if (!fp) {
        RAISE_FMT(ERR_OS, "failed to open output file '%s'.", build_args.output_file);
        (void)delete_generic_vector(left_overs, (expr_free)&delete_parsed_scene);
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        return (1);
    }
    
    if (write_object_file(fp, wctx)) {
        PROPAGATE_ERR();
        (void)delete_generic_vector(left_overs, (expr_free)&delete_parsed_scene);
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        (void)fclose(fp);
        return (1);
    }

    (void)fclose(fp);
    (void)delete_generic_vector(left_overs, (expr_free)&delete_parsed_scene);
    (void)delete_writer_ctx(wctx);
    (void)reset_args(&build_args);

    return (0);
}