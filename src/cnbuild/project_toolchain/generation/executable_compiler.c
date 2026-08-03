#include "../project_toolchain.h"

uint8_t executable_compiler_build_objects(LibraryCompiler *compiler)
{
    if (!compiler) {
        RAISE(ERR_INVALID_POINTER, "can't build object of empty library compiler.");
        return (1);
    }

    char *temp_path;
    char *obj_path;
    char *flat_name;
    char **argv = malloc(sizeof(char *) * (7 + 1 + (compiler->preprocessor_definitions.size)));
    size_t i;

    if (!argv) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new argv of size %zu.", (7 + 1 + (compiler->preprocessor_definitions.size)));
        return (1);
    }

    argv[0] = compiler->compiler_path;
    argv[1] = "-c";
    argv[2] = NULL;
    argv[3] = "-o";
    argv[4] = NULL;
    argv[5] = "-I";
    argv[6] = compiler->includes_path ? compiler->includes_path : "./";

    for (i = 0; i < compiler->preprocessor_definitions.size; ++i) {
        argv[7 + i] = malloc(sizeof(char) * strlen((char *)compiler->preprocessor_definitions.content[i]) + 3);
        if (!argv[7 + i]) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new str for preprocessor definition.");
            (void)free(argv);
            return (1);
        }
        strcpy(argv[7 + i], "-D");
        strcpy(argv[7 + i] + strlen("-D"), (char *)compiler->preprocessor_definitions.content[i]);
    }

    argv[7 + i] = NULL;

    for (size_t o = 0; o < compiler->srcs.size; ++o) {
        argv[2] = (char *)compiler->srcs.content[o];

        flat_name = flatten_source_path(compiler->srcs.content[o]);

        if (!flat_name) {
            PROPAGATE_ERR();
            for (i = 0; i < compiler->preprocessor_definitions.size; ++i)
                (void)free(argv[8 + i]);
            (void)free(argv);
            return (1);
        }

        temp_path = join_path(compiler->build_path, flat_name);
        (void)free(flat_name);

        if (!temp_path) {
            PROPAGATE_ERR();
            for (i = 0; i < compiler->preprocessor_definitions.size; ++i)
                (void)free(argv[7 + i]);
            (void)free(argv);
            return (1);
        }

        obj_path = replace_extension(temp_path, "o");
        (void)free(temp_path);

        if (!obj_path) {
            PROPAGATE_ERR();
            for (i = 0; i < compiler->preprocessor_definitions.size; ++i)
                (void)free(argv[7 + i]);
            (void)free(argv);
            return (1);
        }

        if (insert_generic_vector(&compiler->objs, obj_path)) {
            PROPAGATE_ERR();
            free(obj_path);
            for (i = 0; i < compiler->preprocessor_definitions.size; ++i)
                (void)free(argv[7 + i]);
            (void)free(argv);
            return (1);
        }

        argv[4] = (char *)compiler->objs.content[o];

        printf("building %s\n", argv[2]);
        // for (size_t v = 0; argv[v]; ++v)
        //     printf(argv[v + 1] ? "%s " : "%s\n", argv[v]);

        if (run_program(compiler->compiler_path, (const char * const*)argv)) {
            RAISE_FMT(ERR_OS, "compiler '%s' returned failure.", compiler->compiler_path);
            for (i = 0; i < compiler->preprocessor_definitions.size; ++i)
                (void)free(argv[7 + i]);
            (void)free(argv);
            return (1);
        }
    }

    for (i = 0; i < compiler->preprocessor_definitions.size; ++i)
        (void)free(argv[7 + i]);
    (void)free(argv);
    return (0);
}

uint8_t executable_compiler_build_binary(LibraryCompiler *compiler)
{
    if (!compiler) {
        RAISE(ERR_INVALID_POINTER, "can't build dynlib of empty library compiler.");
        return (1);
    }

    const char *rpath_flag = NULL;

    if (compiler->machine == CNBUILD_SYS_GEN_LINUX) {
        rpath_flag = "-Wl,-rpath,$ORIGIN";
    } else if (compiler->machine == CNBUILD_SYS_DARWIN) {
        rpath_flag = "-Wl,-rpath,@executable_path";
    }

    char **argv = malloc(sizeof(char *) * (5 + compiler->objs.size + compiler->libs.size + (rpath_flag ? 1 : 0) + 1));
    size_t i = 0;

    if (!argv) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new argv of size %zu.", (5 + compiler->objs.size + compiler->libs.size + (rpath_flag ? 1 : 0) + 1));
        return (1);
    }

    argv[0] = compiler->compiler_path;
    argv[1] = "-o";
    argv[2] = compiler->output_path;
    argv[3] = "-L";
    argv[4] = compiler->library_path ? compiler->library_path : "./";

    for (i = 0; i < compiler->libs.size; ++i) {
        argv[5 + i] = malloc(sizeof(char) * strlen((char *)compiler->libs.content[i]) + 3);
        if (!argv[5 + i]) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new str for library.");
            (void)free(argv);
            return (1);
        }
        strcpy(argv[5 + i], "-l");
        strcpy(argv[5 + i] + strlen("-l"), (char *)compiler->libs.content[i]);
    }

    for (i = 0; i < compiler->objs.size; ++i)
        argv[5 + compiler->libs.size + i] = (char *)compiler->objs.content[i];

    if (rpath_flag)
        argv[5 + compiler->libs.size + i] = (char *)rpath_flag;

    argv[5 + compiler->libs.size + i + (rpath_flag ? 1 : 0)] = NULL;

    printf("linking %s\n", compiler->output_path);
    // for (size_t v = 0; argv[v]; ++v)
    //     printf(argv[v + 1] ? "%s " : "%s\n", argv[v]);

    if (run_program(compiler->compiler_path, (const char * const*)argv)) {
        RAISE_FMT(ERR_OS, "compiler '%s' returned failure.", compiler->compiler_path);
        for (i = 0; i < compiler->libs.size; ++i)
            (void)free(argv[5 + i]);
        (void)free(argv);
        return (1);
    }
    for (i = 0; i < compiler->libs.size; ++i)
        (void)free(argv[5 + i]);
    (void)free(argv);
    return (0);
}

cnbool _is_forsubmodule_src(const char *submodule, const CNBuild *build)
{
    SubModule *submod;

    for (size_t i = 0; i < build->dependencies.size; ++i) {
        submod = build->dependencies.content[i];

        if (!strcmp(submod->name, submodule))
            return (true);
    }
    return (false);
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

uint8_t compile_executable(const EngineConfig *config, const CNProject *project, const CNBuild *build_info, const char *output_path, const char *build_path, const char *include_path, const char *lib_path)
{
    if (!project) {
        RAISE(ERR_INVALID_POINTER, "can't compile library for empty project.");
        return (1);
    }

    ExecutableCompiler *compiler;
    EngineGeneratorItem *temp_asset;
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

    for (size_t i = 0; i < config->generator.size; ++i) {
        temp_asset = config->generator.content[i];

        if (temp_asset->type == GENT_SRC) {
            if (temp_asset->forsubmodule && !_is_forsubmodule_src(temp_asset->forsubmodule, build_info))
                continue;
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

    if (executable_compiler_build_objects(compiler)) {
        PROPAGATE_ERR();
        delete_library_compiler(compiler);
        return (1);
    }

    if (executable_compiler_build_binary(compiler)) {
        PROPAGATE_ERR();
        delete_library_compiler(compiler);
        return (1);
    }

    (void)delete_library_compiler(compiler);

    return (0);
}
