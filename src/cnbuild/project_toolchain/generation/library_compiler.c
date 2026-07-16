#include "../project_toolchain.h"

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
    char *obj_path;
    char *argv[] = {
        compiler->compiler_path,
        "-fPIC",
        "-c",
        NULL,
        "-o",
        NULL,
        "-I",
        compiler->includes_path ? compiler->includes_path : "./",
        NULL
    };

    for (size_t i = 0; i < compiler->srcs.size; ++i) {
        argv[3] = (char *)compiler->srcs.content[i];

        temp_path = join_path(compiler->build_path, path_basename(compiler->srcs.content[i]));

        if (!temp_path) {
            PROPAGATE_ERR();
            return (1);
        }

        obj_path = replace_extension(temp_path, "o");
        (void)free(temp_path);

        if (!obj_path) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&compiler->objs, obj_path)) {
            PROPAGATE_ERR();
            free(obj_path);
            return (1);
        }

        argv[5] = (char *)compiler->objs.content[i];

        for (size_t v = 0; argv[v]; ++v)
            printf(argv[v + 1] ? "%s " : "%s\n", argv[v]);

        if (run_program(compiler->compiler_path, (const char * const*)argv)) {
            RAISE_FMT(ERR_OS, "compiler '%s' returned failure.", compiler->compiler_path);
            return (1);
        }
    }

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

    for (; i < compiler->objs.size; ++i)
        argv[6 + i] = (char *)compiler->objs.content[i];

    argv[6 + i] = NULL;

    for (size_t v = 0; argv[v]; ++v)
        printf(argv[v + 1] ? "%s " : "%s\n", argv[v]);

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
    (void)free(compiler);
}

uint8_t compile_library(const CNProject *project, const CNBuild *build_info, const char *output_path, const char *build_path, const char *include_path, const char *lib_path)
{
    if (!project) {
        RAISE(ERR_INVALID_POINTER, "can't compile library for empty project.");
        return (1);
    }
    
    LibraryCompiler *compiler;
    CNAsset *temp_asset;
    SubModule *temp_module;

    compiler = new_library_compiler(output_path, "gcc", build_path);

    if (!compiler) {
        PROPAGATE_ERR();
        return (1);
    }

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
