#ifndef _BUILD_H_
    #define _BUILD_H_

    #include <libxml/parser.h>
    #include <libxml/tree.h>

    #include "../engine.h"


    

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
