#include "../project_toolchain.h"

LibraryCompiler *new_library_compiler(const char *libname, const char *toolchain, const char *build_path)
{

    if (!libname || !toolchain)
        return (NULL);

    LibraryCompiler *compiler = (LibraryCompiler *)malloc(sizeof(LibraryCompiler));

    if (!compiler)
        return (NULL);
    compiler->output_path = strdup(libname);
    compiler->compiler_path = strdup(toolchain);
    compiler->build_path = strdup(build_path);
    compiler->includes_path = NULL;
    compiler->objs.capacity = 0;
    compiler->objs.size = 0;
    compiler->objs.content = NULL;
    compiler->srcs.capacity = 0;
    compiler->srcs.size = 0;
    compiler->srcs.content = NULL;

    return (compiler);
}

uint8_t library_compiler_add_src(LibraryCompiler *compiler, const char *srcname)
{
    if (!compiler || !srcname)
        return (1);

    char *temp = strdup(srcname);

    if (!temp)
        return (1);

    if (insert_generic_vector(&compiler->srcs, temp)) {
        (void)free(temp);
        return (1);
    }

    return (0);
}

uint8_t library_compiler_set_include_path(LibraryCompiler *compiler, const char *include_path)
{
    if (!compiler || !include_path)
        return (1);

    compiler->includes_path = strdup(include_path);

    if (!compiler->includes_path)
        return (1);
    return (0);
}

uint8_t library_compiler_build_objects(LibraryCompiler *compiler)
{
    if (!compiler)
        return (1);

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

        if (!temp_path)
            return (1);

        obj_path = replace_extension(temp_path, "o");
        (void)free(temp_path);

        if (!obj_path)
            return (1);

        if (insert_generic_vector(&compiler->objs, obj_path)) {
            free(obj_path);
            return (1);
        }

        argv[5] = (char *)compiler->objs.content[i];

        for (size_t v = 0; argv[v]; ++v)
            printf(argv[v + 1] ? "%s " : "%s\n", argv[v]);

        if (run_program(compiler->compiler_path, (const char * const*)argv))
            return (1);
    }

    return (0);
}

uint8_t library_compiler_build_dynlib(LibraryCompiler *compiler)
{
    if (!compiler)
        return (1);

    char **argv = malloc(sizeof(char *) * (4 + compiler->objs.size + 1));
    size_t i = 0;
    
    if (!argv)
        return (1);

    argv[0] = compiler->compiler_path;
    argv[1] = "-shared";
    argv[2] = "-o";
    argv[3] = compiler->output_path;

    for (i = 0; i < compiler->objs.size; ++i)
        argv[4 + i] = (char *)compiler->objs.content[i];

    argv[4 + i] = NULL;

    for (size_t v = 0; argv[v]; ++v)
        printf(argv[v + 1] ? "%s " : "%s\n", argv[v]);

    if (run_program(compiler->compiler_path, (const char * const*)argv)) {
        (void)free(argv);
        return (1);
    }
    (void)free(argv);
    return (0);
}

void delete_library_compiler(LibraryCompiler *compiler)
{
    if (!compiler)
        return;

    if (compiler->output_path)
        (void)free(compiler->output_path);
    if (compiler->build_path)
        (void)free(compiler->build_path);
    if (compiler->includes_path)
        (void)free(compiler->includes_path);
    if (compiler->compiler_path)
        (void)free(compiler->compiler_path);
    if (compiler->objs.content)
        (void)empty_generic_vector(&compiler->objs, &free);
    if (compiler->srcs.content)
        (void)empty_generic_vector(&compiler->srcs, &free);
    (void)free(compiler);
}

uint8_t compile_library(const CNProject *project, const char *build_path, const char *include_path)
{
    if (!project)
        return (1);
    
    LibraryCompiler *compiler;
    CNAsset *temp_asset;

    compiler = new_library_compiler("./game.so", "gcc", build_path);

    if (!compiler) {
        return (1);
    }

    if (library_compiler_set_include_path(compiler, include_path)) {
        (void)delete_library_compiler(compiler);
        return (1);
    }

    for (size_t i = 0; i < project->content.size; ++i) {
        temp_asset = project->content.content[i];

        if (temp_asset->type == CNASSET_TP_SRC) {
            library_compiler_add_src(compiler, temp_asset->location);
        }
    }

    if (library_compiler_build_objects(compiler)) {
        delete_library_compiler(compiler);
        return (1);
    }

    if (library_compiler_build_dynlib(compiler)) {
        delete_library_compiler(compiler);
        return (1);
    }

    return (0);
}
