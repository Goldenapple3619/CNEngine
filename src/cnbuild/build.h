#ifndef _BUILD_H_
    #define _BUILD_H_

    #include <libxml/parser.h>
    #include <libxml/tree.h>

    #include "../engine.h"


    struct build_args_s {
        char *output_file;
        struct generic_vector_s input_files;

        engine_wrt_flags padding;
        engine_wrt_endian endian;
    };

    struct parsed_gui_element_data_s {
        char *id;
        char *object_type;
        char *text_content;

        const struct parsed_gui_element_data_s *parent;
        struct generic_map_s styles;
    };

    int build_gui(size_t argc, char **argv);

    static const struct {
        const char *name;
        route_callback callback;
    } build_types[] = {
        {"gui", &build_gui},
        {"obj", NULL},
        {"scn", NULL},
        {"proj", NULL},
        {NULL, NULL}
    };

#endif
