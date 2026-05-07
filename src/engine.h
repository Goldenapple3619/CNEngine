#ifndef _ENGINE_H_
    #define _ENGINE_H_

    #include <stdio.h>
    #include <signal.h>
    #include "libcncore.h"
    #include "libcnaudio.h"
    #include "libcninput.h"
    #include "libcngraphic.h"
    #include "librgui.h"
    #include "libr2d.h"
    #include "libr3d.h"
    #include "libcnassets.h"

    int argument_route(size_t argc, char **argv);

    int init(size_t argc, char **argv);
    int test(size_t argc, char **argv);
    int help(size_t argc, char **argv);
    int build(size_t argc, char **argv);

    typedef int (*route_callback)(size_t, char **);

    static const struct {
        const char *name;
        route_callback callback;
    } routes[] = {
        {"init", &init},
        {"help", &help},
        {"test", &test},
        {"build", &build},
        {NULL, NULL}
    };

#endif
