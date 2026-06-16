#include "project_toolchain.h"

uint8_t parse_project(const char *file_path, struct engine_object_file_writer_ctx_s *wctx, Object *asset_ctx)
{
    struct generic_vector_s *parsed_data; 
    xmlDoc *doc;
    xmlNode *root;
    // xmlChar *temp_s;

    (void)asset_ctx;
    (void)wctx;

    doc = xmlReadFile(file_path, NULL, 0);

    if (!doc) {
        fprintf(stderr, "%s: failed to open and parse file.\n", file_path);
        return (1);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "project")) {
        fprintf(stderr, "%s: invalid root element '%s', expecting 'gui'.\n", file_path, root->name);
        return (1);
    }

    parsed_data = new_generic_vector();

    if (!parsed_data)
        return (1);

    (void)xmlFreeDoc(doc);
    (void)xmlCleanupParser();

    return (0);
}

int build_project(size_t argc, char **argv, Object *asset_ctx)
{
    struct engine_object_file_writer_ctx_s *wctx;
    struct build_args_s build_args = {0};
    FILE *fp;
    
    (void)asset_ctx;

    if (build_get_args(argc - 3, argv + 3, &build_args)) {
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
        fprintf(stderr, "writter ctx allocation failed.\n");
        (void)reset_args(&build_args);
        return (1);
    }

    wctx->write_infos.endian = build_args.endian;
    wctx->write_infos.flags = build_args.padding;

    fp = fopen(build_args.output_file, "w");

    if (!fp) {
        fprintf(stderr, "%s: failed to open output file.\n", build_args.output_file);
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        return (1);
    }
    
    if (write_object_file(fp, wctx)) {
        fprintf(stderr, "%s: failed to write output file.\n", build_args.output_file);
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