#ifndef _PROJECT_TOOLCHAIN_H_
    #define _PROJECT_TOOLCHAIN_H_

    #include "../build.h"

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

    typedef struct {
        char *output_path;
        
        char *compiler_path;
        
        char *includes_path;
        char *build_path;
        struct generic_vector_s srcs;
        struct generic_vector_s objs;
    } LibraryCompiler;


    CNProject *new_cnproject(void);
    void delete_cnproject(CNProject *ptr);

    CNAsset *new_cnasset(const char *asset_location, cnasset_type tp);
    void delete_cnasset(CNAsset *ptr);

    CNBuild *new_build(void);
    uint8_t build_set_entry_point(CNBuild *build, const char *entry_point);
    uint8_t build_set_name(CNBuild *build, const char *name);
    void delete_build(CNBuild *build);

    cnasset_type tp_from_string(const char *str);
    uint8_t cnressources_walk_path(CNProject *project, char *path, cnasset_type tp);

    char *resolve_path(char *base_path, const char *project_root);

    CNProject *parse_project_xml(const char *file_path, const char *project_root);
    uint8_t parse_cnressources_xml(CNProject *project, xmlNode *node, const char *project_root);
    uint8_t parse_cnbuilds_xml(CNProject *project, xmlNode *node);

    uint8_t compile_library(const CNProject *project, const char *build_path, const char *include_path);
#endif