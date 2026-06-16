#ifndef _PROJECT_TOOLCHAIN_H_
    #define _PROJECT_TOOLCHAIN_H_

    #include "../build.h"

    #ifdef _WIN32
        #include <windows.h>
        #define PATH_SEP '\\'
    #else
        #include <sys/stat.h>
        #include <unistd.h>
        #include <sys/wait.h>
        #define PATH_SEP '/'
    #endif


    typedef enum {
        CNBUILD_ARCH_HOST = 0x00,
        CNBUILD_ARCH_AMD64,
        CNBUILD_ARCH_I386,
        CNBUILD_ARCH_ARM64
    } cnbuild_architectures;

    typedef enum {
        CNBUILD_SYS_HOST = 0x00,
        CNBUILD_SYS_WIN,
        CNBUILD_SYS_DARWIN,
        CNBUILD_SYS_GEN_LINUX
    } cnbuild_system;

    typedef enum {
        CNASSET_TP_RAW = 0x00,
        CNASSET_TP_GUI,
        CNASSET_TP_OBJ,
        CNASSET_TP_SCN,
        CNASSET_TP_SRC,
        CNASSET_TP_PCA
    } cnasset_type;

    typedef struct {
        char *location;

        cnasset_type type;
    } CNAsset;

    typedef struct {
        char *name;
        char *scene_entry_point;

        cnbuild_architectures arch;
        cnbuild_system machine;

        struct generic_vector_s dependencies;
    } CNBuild;

    typedef struct {
        char *name;
        char *version_name;

        struct generic_vector_s builds;
        struct generic_vector_s content;
    } CNProject;

#endif