#ifndef _BUILD_H_
    #define _BUILD_H_

    #include <libxml/parser.h>
    #include <libxml/tree.h>

    #include "uuid_compat.h"

    #include "../engine.h"

    struct build_args_s {
        char *output_file;
        struct generic_vector_s input_files;

        engine_wrt_flags padding;
        engine_wrt_endian endian;
    };

    char *strip_whitespace(const char *str);

    uint8_t build_get_args(size_t argc, char **argv, struct build_args_s *args);
    void reset_args(struct build_args_s *args);

    int build_gui(size_t argc, char **argv);
    int build_assets(size_t argc, char **argv);
    int build_obj(size_t argc, char **argv);
    int build_project(size_t argc, char **argv);
    int build_scene(size_t argc, char **argv);
    int build_asset_pack(size_t argc, char **argv);

    static const struct {
        const char *name;
        route_callback callback;
    } build_types[] = {
        {"gui", &build_gui},
        {"obj", &build_obj},
        {"scn", &build_scene},
        {"proj", &build_project},
        {"asset", &build_assets},
        {"lnk", &build_asset_pack},
        {NULL, NULL}
    };

#endif
