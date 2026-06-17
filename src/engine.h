#ifndef _ENGINE_H_
    #define _ENGINE_H_

    #include <stdio.h>
    #include <signal.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <string.h>

    #include "libcncore.h"
    #include "libcnaudio.h"
    #include "libcninput.h"
    #include "libcngraphic.h"
    #include "librgui.h"
    #include "libr2d.h"
    #include "libr3d.h"
    #include "libcnassets.h"

    #define COPY_BUFFER_SIZE 8192

    #ifdef _WIN32
        #include <windows.h>
        #include <direct.h>
        #define MKDIR(path) _mkdir(path)
        #define PATH_SEP '\\'
        #define WIN_MAX_COMMAND_SIZE 4096
    #else
        #include <dirent.h>
        #include <sys/stat.h>
        #include <unistd.h>
        #include <sys/wait.h>
        #include <sys/types.h>
        #define MKDIR(path) mkdir(path, 0755)
        #define PATH_SEP '/'
    #endif

    char *get_dirname(const char *path);
    char *join_path(const char *a, const char *b);
    cnbool is_dir(const char *path);
    int run_program(const char *program, const char *const argv[]);
    const char *get_extension(const char *path);
    char *replace_extension(const char *path, const char *ext);
    const char *path_basename(const char *path);
    uint8_t copy_file(const char *src, const char *dst);
    
    int argument_route(size_t argc, char **argv);

    int init(size_t argc, char **argv);
    int test(size_t argc, char **argv);
    int help(size_t argc, char **argv);
    int build(size_t argc, char **argv);
    int dump(size_t argc, char **argv);

    typedef int (*route_callback)(size_t, char **);

    static const struct {
        const char *name;
        route_callback callback;
    } routes[] = {
        {"init", &init},
        {"help", &help},
        {"test", &test},
        {"build", &build},
        {"dump", &dump},
        {NULL, NULL}
    };

#endif
