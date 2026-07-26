#ifndef _PROJECT_TOOLCHAIN_H_
    #define _PROJECT_TOOLCHAIN_H_

    #include "../build.h"

    typedef enum {
        GENT_INCLUDE = 0x00,
        GENT_SRC,
    } generator_type;

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

        cnbuild_system machine;
        cnbuild_architectures architecture;

        char *includes_path;
        char *build_path;
        char *library_path;
        struct generic_vector_s srcs;
        struct generic_vector_s objs;
        struct generic_vector_s libs;
        struct generic_vector_s preprocessor_definitions;
    } LibraryCompiler;

    typedef LibraryCompiler ExecutableCompiler;

    typedef struct {
        char *name;
        char *path;
        cnbool isdir;
        cnbool link;
        cnbool overwrite;
        cnbool skip_error;
    } SubModuleLib;

    typedef struct {
        char *path;
        cnbool isdir;
        cnbool overwrite;
        cnbool skip_error;
    } SubModuleInclude;

    typedef struct {
        char *name;

        struct generic_vector_s libs;
        struct generic_vector_s includes;
        struct generic_vector_s need;
    } SubModule;

    typedef struct {
        cnbuild_architectures architecture;
        cnbuild_system machine;

        char *lib_path;
        char *include_path;
        char *toolchain;
    } EngineRessourceSet;

    typedef struct {
        char *location;
        char *forsubmodule;

        generator_type type;
    } EngineGeneratorItem;

    typedef struct {
        char *submodules_location;

        struct generic_vector_s ressources;
        struct generic_vector_s generator;
    } EngineConfig;


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

    char *resolve_path(char *base_path, const char *replace_with, const char *placeholder);

    CNProject *parse_project_xml(const EngineConfig *config, const char *file_path, const char *project_root);
    uint8_t parse_cnressources_xml(CNProject *project, xmlNode *node, const char *project_root);
    uint8_t parse_cnbuilds_xml(const EngineConfig *config, CNProject *project, xmlNode *node);
    uint8_t parse_submodules_xml(SubModule *submodule, const char *xml_path);

    uint8_t compile_library(const CNProject *project, const CNBuild *build_info, const char *output_path, const char *build_path, const char *include_path, const char *lib_path);
    uint8_t library_compiler_set_library_path(LibraryCompiler *compiler, const char *library_path);
    uint8_t library_compiler_set_include_path(LibraryCompiler *compiler, const char *include_path);
    uint8_t library_compiler_add_lib(LibraryCompiler *compiler, const char *libname);
    uint8_t library_compiler_add_src(LibraryCompiler *compiler, const char *srcname);
    LibraryCompiler *new_library_compiler(const char *libname, const char *toolchain, const char *build_path);
    void delete_library_compiler(LibraryCompiler *compiler);
    void clear_submodule_datas(SubModule *submodule);
    SubModule *new_submodule(void);
    void delete_submodule(SubModule *submodule);

    EngineConfig *parse_config_xml(const char *file_path, const char *engine_root);

    EngineRessourceSet *new_ressource_set(void);
    void delete_ressource_set(EngineRessourceSet *set);

    EngineConfig *new_engine_config(void);
    void delete_engine_config(EngineConfig *config);

    char *sysname_from_system(cnbuild_system system);

    cnbool has_ressource_set(const EngineConfig *config, cnbuild_architectures arch, cnbuild_system machine);
    const EngineRessourceSet *find_ressource_set(const EngineConfig *config, cnbuild_architectures arch, cnbuild_system machine);

    cnbuild_system os_from_string(const char *str);
    cnbuild_architectures arch_from_string(const char *str);

    EngineGeneratorItem *new_generator_item(void);
    void delete_generator_item(EngineGeneratorItem *item);

    uint8_t compile_executable(const EngineConfig *config, const CNProject *project, const CNBuild *build_info, const char *output_path, const char *build_path, const char *include_path, const char *lib_path);
    uint8_t library_compiler_add_preprocessor_definition(LibraryCompiler *compiler, const char *definition_name, const char *definition_content);
#endif
