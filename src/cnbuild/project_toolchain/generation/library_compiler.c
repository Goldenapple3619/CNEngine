#include "../project_toolchain.h"
#include <string.h>

LibraryCompiler *new_library_compiler(const char *libname, const char *toolchain, const char *build_path)
{

    if (!libname || !toolchain) {
        RAISE(ERR_INVALID_POINTER, "can't allocate new library compiler from empty libname / toolchain.")
        return (NULL);
    }

    LibraryCompiler *compiler = (LibraryCompiler *)malloc(sizeof(LibraryCompiler));

    if (!compiler) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new library compiler.")
        return (NULL);
    }
    compiler->output_path = strdup(libname);
    compiler->compiler_path = strdup(toolchain);
    compiler->build_path = strdup(build_path);
    compiler->includes_path = NULL;
    compiler->library_path = NULL;
    compiler->objs.capacity = 0;
    compiler->objs.size = 0;
    compiler->objs.content = NULL;
    compiler->srcs.capacity = 0;
    compiler->srcs.size = 0;
    compiler->srcs.content = NULL;
    compiler->libs.capacity = 0;
    compiler->libs.size = 0;
    compiler->libs.content = NULL;
    compiler->preprocessor_definitions.capacity = 0;
    compiler->preprocessor_definitions.size = 0;
    compiler->preprocessor_definitions.content = NULL;

    return (compiler);
}

uint8_t library_compiler_add_src(LibraryCompiler *compiler, const char *srcname)
{
    if (!compiler) {
        RAISE(ERR_INVALID_POINTER, "can't add src to empty library compiler.");
        return (1);
    }
    if (!srcname) {
        RAISE(ERR_INVALID_POINTER, "can't add src to library compiler with empty srcname.");
        return (1);
    }

    char *temp = strdup(srcname);

    if (!temp) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new srcname.");
        return (1);
    }

    if (insert_generic_vector(&compiler->srcs, temp)) {
        PROPAGATE_ERR();
        (void)free(temp);
        return (1);
    }

    return (0);
}

uint8_t library_compiler_add_lib(LibraryCompiler *compiler, const char *libname)
{
    if (!compiler) {
        RAISE(ERR_INVALID_POINTER, "can't add lib to empty library compiler.");
        return (1);
    }
    if (!libname) {
        RAISE(ERR_INVALID_POINTER, "can't add lib to library compiler with empty libname.");
        return (1);
    }

    char *temp = strdup(libname);

    if (!temp) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new libname.");
        return (1);
    }

    if (insert_generic_vector(&compiler->libs, temp)) {
        PROPAGATE_ERR();
        (void)free(temp);
        return (1);
    }

    return (0);
}

uint8_t library_compiler_set_include_path(LibraryCompiler *compiler, const char *include_path)
{
    if (!compiler) {
        RAISE(ERR_INVALID_POINTER, "can't set include path to empty library compiler.");
        return (1);
    }
    if (!include_path) {
        RAISE(ERR_INVALID_POINTER, "can't set include path to library compiler with empty path.");
        return (1);
    }

    compiler->includes_path = strdup(include_path);

    if (!compiler->includes_path) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new include path.");
        return (1);
    }
    return (0);
}

uint8_t library_compiler_set_library_path(LibraryCompiler *compiler, const char *library_path)
{
    if (!compiler) {
        RAISE(ERR_INVALID_POINTER, "can't set library path to empty library compiler.");
        return (1);
    }
    if (!library_path) {
        RAISE(ERR_INVALID_POINTER, "can't set library path to library compiler with empty path.");
        return (1);
    }

    compiler->library_path = strdup(library_path);

    if (!compiler->library_path) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new library path.");
        return (1);
    }
    return (0);
}

uint8_t library_compiler_build_objects(LibraryCompiler *compiler)
{
    if (!compiler) {
        RAISE(ERR_INVALID_POINTER, "can't build object of empty library compiler.");
        return (1);
    }

    char *temp_path;
    char *flat_name;
    char *obj_path;
    char **argv = malloc(sizeof(char *) * (12 + 1 + (compiler->preprocessor_definitions.size)));
    size_t i;

    if (!argv) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new argv of size %zu.", (12 + 1 + (compiler->preprocessor_definitions.size)));
        return (1);
    }

    argv[0] = compiler->compiler_path;
    argv[1] = "-fPIC";
    argv[2] = "-c";
    argv[3] = NULL;
    argv[4] = "-o";
    argv[5] = NULL;
    argv[6] = "-I";
    argv[7] = compiler->includes_path ? compiler->includes_path : "./";
    argv[8] = "-Wall";
    argv[9] = "-Wextra";
    argv[10] = "-Wshadow";
    argv[11] = "-O2";

    for (i = 0; i < compiler->preprocessor_definitions.size; ++i) {
        argv[12 + i] = malloc(sizeof(char) * strlen((char *)compiler->preprocessor_definitions.content[i]) + 3);
        if (!argv[12 + i]) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new str for preprocessor definition.");
            (void)free(argv);
            return (1);
        }
        strcpy(argv[12 + i], "-D");
        strcpy(argv[12 + i] + strlen("-D"), (char *)compiler->preprocessor_definitions.content[i]);
    }

    argv[12 + i] = NULL;

    for (size_t o = 0; o < compiler->srcs.size; ++o) {
        argv[3] = (char *)compiler->srcs.content[o];

        flat_name = flatten_source_path(compiler->srcs.content[o]);

        if (!flat_name) {
            PROPAGATE_ERR();
            for (i = 0; i < compiler->preprocessor_definitions.size; ++i)
                (void)free(argv[12 + i]);
            (void)free(argv);
            return (1);
        }

        temp_path = join_path(compiler->build_path, flat_name);
        (void)free(flat_name);

        if (!temp_path) {
            PROPAGATE_ERR();
            for (i = 0; i < compiler->preprocessor_definitions.size; ++i)
                (void)free(argv[12 + i]);
            (void)free(argv);
            return (1);
        }

        obj_path = replace_extension(temp_path, "o");
        (void)free(temp_path);

        if (!obj_path) {
            PROPAGATE_ERR();
            for (i = 0; i < compiler->preprocessor_definitions.size; ++i)
                (void)free(argv[12 + i]);
            (void)free(argv);
            return (1);
        }

        if (insert_generic_vector(&compiler->objs, obj_path)) {
            PROPAGATE_ERR();
            (void)free(obj_path);
            for (i = 0; i < compiler->preprocessor_definitions.size; ++i)
                (void)free(argv[12 + i]);
            (void)free(argv);
            return (1);
        }

        argv[5] = (char *)compiler->objs.content[o];

        printf("building %s\n", argv[3]);
        // for (size_t v = 0; argv[v]; ++v)
        //     printf(argv[v + 1] ? "%s " : "%s\n", argv[v]);

        if (run_program(compiler->compiler_path, (const char * const*)argv)) {
            RAISE_FMT(ERR_OS, "compiler '%s' returned failure.", compiler->compiler_path);
            for (i = 0; i < compiler->preprocessor_definitions.size; ++i)
                (void)free(argv[12 + i]);
            (void)free(argv);
            return (1);
        }
    }
    for (i = 0; i < compiler->preprocessor_definitions.size; ++i)
        (void)free(argv[12 + i]);
    (void)free(argv);
    return (0);
}

uint8_t library_compiler_build_dynlib(LibraryCompiler *compiler)
{
    if (!compiler) {
        RAISE(ERR_INVALID_POINTER, "can't build dynlib of empty library compiler.");
        return (1);
    }

    char **argv = malloc(sizeof(char *) * (6 + compiler->objs.size + compiler->libs.size + 1));
    size_t i = 0;
    
    if (!argv) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new argv of size %zu.", (6 + compiler->objs.size + compiler->libs.size + 1));
        return (1);
    }

    argv[0] = compiler->compiler_path;
    argv[1] = "-shared";
    argv[2] = "-o";
    argv[3] = compiler->output_path;
    argv[4] = "-L";
    argv[5] = compiler->library_path ? compiler->library_path : "./";
    
    for (i = 0; i < compiler->libs.size; ++i) {
        argv[6 + i] = malloc(sizeof(char) * strlen((char *)compiler->libs.content[i]) + 3);
        if (!argv[6 + i]) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new str for library.");
            (void)free(argv);
            return (1);
        }
        strcpy(argv[6 + i], "-l");
        strcpy(argv[6 + i] + strlen("-l"), (char *)compiler->libs.content[i]);
    }

    for (i = 0; i < compiler->objs.size; ++i)
        argv[6 + compiler->libs.size + i] = (char *)compiler->objs.content[i];

    argv[6 + compiler->libs.size + i] = NULL;

    printf("linking %s\n", compiler->output_path);

    // for (size_t v = 0; argv[v]; ++v)
    //     printf(argv[v + 1] ? "%s " : "%s\n", argv[v]);

    if (run_program(compiler->compiler_path, (const char * const*)argv)) {
        RAISE_FMT(ERR_OS, "compiler '%s' returned failure.", compiler->compiler_path);
        for (i = 0; i < compiler->libs.size; ++i)
            (void)free(argv[6 + i]);
        (void)free(argv);
        return (1);
    }
    for (i = 0; i < compiler->libs.size; ++i)
        (void)free(argv[6 + i]);
    (void)free(argv);
    return (0);
}

void delete_library_compiler(LibraryCompiler *compiler)
{
    if (!compiler) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty library compiler.");
        return;
    }

    if (compiler->output_path)
        (void)free(compiler->output_path);
    if (compiler->build_path)
        (void)free(compiler->build_path);
    if (compiler->includes_path)
        (void)free(compiler->includes_path);
    if (compiler->library_path)
        (void)free(compiler->library_path);
    if (compiler->compiler_path)
        (void)free(compiler->compiler_path);
    if (compiler->objs.content)
        (void)empty_generic_vector(&compiler->objs, &free);
    if (compiler->srcs.content)
        (void)empty_generic_vector(&compiler->srcs, &free);
    if (compiler->libs.content)
        (void)empty_generic_vector(&compiler->libs, &free);
    if (compiler->preprocessor_definitions.content)
        (void)empty_generic_vector(&compiler->preprocessor_definitions, &free);
    (void)free(compiler);
}

uint8_t library_compiler_add_preprocessor_definition(LibraryCompiler *compiler, const char *definition_name, const char *definition_content)
{
    if (!compiler) {
        RAISE(ERR_INVALID_POINTER, "can't add definition to empty library compiler.");
        return (1);
    }
    if (!definition_name) {
        RAISE(ERR_INVALID_POINTER, "can't add empty definition to library compiler.");
        return (1);
    }

    size_t name_size = strlen(definition_name);
    char *temp = malloc(sizeof(char) * (name_size + 1 + (definition_content ? strlen(definition_content) : 0) + 1));

    if (!temp) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new preprocessor definition.");
        return (1);
    }

    (void)strcpy(temp, definition_name);
    temp[name_size] = '=';

    if (definition_content) {
        (void)strcpy(temp + name_size + 1, definition_content);
    }

    for (size_t i = 0; i < name_size; ++i)
        temp[i] = (char)toupper((int)temp[i]);


    if (insert_generic_vector(&compiler->preprocessor_definitions, temp)) {
        PROPAGATE_ERR();
        (void)free(temp);
        return (1);
    }

    return (0);
}

static char *get_compiler_for_build_from_config(const EngineConfig *config, const CNBuild *build_info)
{
    EngineRessourceSet *set;

    for (size_t i = 0; i < config->ressources.size; ++i) {
        set = config->ressources.content[i];

        if (set->machine == build_info->machine && set->architecture == build_info->arch) {
            return (set->toolchain);
        }
    }

    return ("gcc");
}

uint8_t compile_library(const EngineConfig *config, const CNProject *project, const CNBuild *build_info, const char *output_path, const char *build_path, const char *include_path, const char *lib_path)
{
    if (!project) {
        RAISE(ERR_INVALID_POINTER, "can't compile library for empty project.");
        return (1);
    }

    LibraryCompiler *compiler;
    CNAsset *temp_asset;
    SubModule *temp_module;
    char *temp_preproc;

    compiler = new_library_compiler(output_path, get_compiler_for_build_from_config(config, build_info), build_path);

    if (!compiler) {
        PROPAGATE_ERR();
        return (1);
    }

    compiler->machine = build_info->machine;
    compiler->architecture = build_info->arch;

    if (library_compiler_set_include_path(compiler, include_path)) {
        PROPAGATE_ERR();
        (void)delete_library_compiler(compiler);
        return (1);
    }

    if (library_compiler_set_library_path(compiler, lib_path)) {
        PROPAGATE_ERR();
        (void)delete_library_compiler(compiler);
        return (1);
    }

    for (size_t i = 0; i < project->content.size; ++i) {
        temp_asset = project->content.content[i];

        if (temp_asset->type == CNASSET_TP_SRC) {
            if (library_compiler_add_src(compiler, temp_asset->location)) {
                PROPAGATE_ERR();
                (void)delete_library_compiler(compiler);
                return (1);
            }
        }
    }

    for (size_t i = 0; i < build_info->dependencies.size; ++i) {
        temp_module = (SubModule *)build_info->dependencies.content[i];
        temp_preproc = malloc(sizeof(char) * (5 + strlen(temp_module->name) + 1));

        if (!temp_preproc) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new preproc definition.");
            (void)delete_library_compiler(compiler);
            return (1);
        }

        (void)strcpy(temp_preproc, "_HAS_");
        (void)strcpy(temp_preproc + 5, temp_module->name);

        if (library_compiler_add_preprocessor_definition(compiler, temp_preproc, "1")) {
            PROPAGATE_ERR()
            (void)free(temp_preproc);
            (void)delete_library_compiler(compiler);
            return (1);
        }

        (void)free(temp_preproc);

        for (size_t j = 0; j < temp_module->libs.size; ++j) {
            if (!((SubModuleLib *)temp_module->libs.content[j])->link)
                continue;
            if (!((SubModuleLib *)temp_module->libs.content[j])->name) {
                RAISE_FMT(WAR_IMPORTANT, "linking asked for '%s' but no name was provided to identify it.", ((SubModuleLib *)temp_module->libs.content[j])->path);
                continue;
            }
            if (library_compiler_add_lib(compiler, ((SubModuleLib *)temp_module->libs.content[j])->name)) {
                PROPAGATE_ERR();
                delete_library_compiler(compiler);
                return (1);
            }
        }
    }

    if (build_info->scene_entry_point) {
        if (library_compiler_add_preprocessor_definition(compiler, "_ENTRY_SCENE", build_info->scene_entry_point)) {
            PROPAGATE_ERR();
            (void)delete_library_compiler(compiler);
            return (1);
        }
    }

    if (library_compiler_build_objects(compiler)) {
        PROPAGATE_ERR();
        delete_library_compiler(compiler);
        return (1);
    }

    if (library_compiler_build_dynlib(compiler)) {
        PROPAGATE_ERR();
        delete_library_compiler(compiler);
        return (1);
    }

    (void)delete_library_compiler(compiler);

    return (0);
}
