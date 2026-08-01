#include "project_toolchain.h"

char *extension_from_system(cnbuild_system system)
{
    switch (system) {
        case CNBUILD_SYS_WIN:
            return ("dll");

        case CNBUILD_SYS_GEN_LINUX:
            return ("so");

        case CNBUILD_SYS_DARWIN:
            return ("dylib");

        default:
            return ("so");
    }
}

char *extension_executable_from_system(cnbuild_system system)
{
    switch (system) {
        case CNBUILD_SYS_WIN:
            return ("exe");

        case CNBUILD_SYS_GEN_LINUX:
            return ("out");

        case CNBUILD_SYS_DARWIN:
            return ("out");

        default:
            return ("out");
    }
}

char *sysname_from_system(cnbuild_system system)
{
    switch (system) {
        case CNBUILD_SYS_WIN:
            return ("win");

        case CNBUILD_SYS_GEN_LINUX:
            return ("linux");

        case CNBUILD_SYS_DARWIN:
            return ("macos");

        default:
            return ("uknown");
    }
}

char *archname_from_arch(cnbuild_architectures arch)
{
    switch (arch) {
        case CNBUILD_ARCH_I386:
            return ("i386");

        case CNBUILD_ARCH_AMD64:
            return ("amd64");

        case CNBUILD_ARCH_ARM64:
            return ("arm64");

        default:
            return ("uknown");
    }
}

void render_step(const char *label, size_t step, size_t max_step)
{
    static const char charset[] = " .:!|+*#";
    static const size_t charset_len = sizeof(charset) - 1;
    const double percentage = max_step ? (double)step / (double)max_step : 0.0;
    size_t index = (size_t)(percentage * (charset_len - 1));

    if (index >= charset_len)
        index = charset_len - 1;

    printf("\r%s - %3.0f%% | [%c]", label ? label : "???", percentage * 100.0, charset[index]);

    if (step == max_step)
        printf("\n");
    fflush(stdout);
}

char *init_build_path(const char *project_root)
{
    char *build_path = join_path(project_root, "build");

    if (!build_path) {
        PROPAGATE_ERR();
        return (NULL);
    }

    if (!is_dir(build_path)) {
        if (make_dir(build_path)) {
            PROPAGATE_ERR();
            (void)free(build_path);
            return (NULL);
        }
    }

    return (build_path);
}

char *init_dist_path(const char *output_path)
{
    char *dist_path = join_path(output_path, "dist");

    if (!dist_path) {
        PROPAGATE_ERR();
        return (NULL);
    }

    if (!is_dir(dist_path)) {
        if (make_dir(dist_path)) {
            PROPAGATE_ERR();
            (void)free(dist_path);
            return (NULL);
        }
    }

    return (dist_path);
}

char *init_subdist_path(const char *dist_path, const char *project_name)
{
    char *subdist_path = join_path(dist_path, project_name);

    if (!subdist_path) {
        PROPAGATE_ERR();
        return (NULL);
    }

    if (!is_dir(subdist_path)) {
        if (make_dir(subdist_path)) {
            PROPAGATE_ERR();
            (void)free(subdist_path);
            return (NULL);
        }
    }

    return (subdist_path);
}

char *init_subbuild_path(const char *build_path, const char *project_name)
{
    char *subbuild_path = join_path(build_path, project_name);

    if (!subbuild_path) {
        PROPAGATE_ERR();
        return (NULL);
    }

    if (!is_dir(subbuild_path)) {
        if (make_dir(subbuild_path)) {
            PROPAGATE_ERR();
            (void)free(subbuild_path);
            return (NULL);
        }
    }

    return (subbuild_path);
}

char *init_subinclude_path(const char *build_path, const char *base_include_path, const CNBuild *build)
{
    char *subinclude_path = join_path(build_path, "include");
    char *temp_dest;
    SubModule *submodule;
    SubModuleInclude include;

    if (!subinclude_path) {
        PROPAGATE_ERR();
        return (NULL);
    }

    if (!is_dir(subinclude_path)) {
        if (make_dir(subinclude_path)) {
            PROPAGATE_ERR();
            (void)free(subinclude_path);
            return (NULL);
        }
    }

    render_step("HEADERS", 0, build->dependencies.size);

    for (size_t i = 0; i < build->dependencies.size; ++i) {
        render_step("HEADERS", i + 1, build->dependencies.size);

        submodule = build->dependencies.content[i];

        for (size_t j = 0; j < submodule->includes.size; ++j) {
            memcpy(&include, submodule->includes.content[j], sizeof(SubModuleInclude));

            include.path = strdup(include.path);

            if (!include.path) {
                printf(" | FAILURE\n");
                RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new include path str.");
                (void)free(subinclude_path);
                return (NULL);
            }

            include.path = resolve_path(include.path, base_include_path, "${INCLUDES}");

            if (!include.path) {
                printf(" | FAILURE\n");
                PROPAGATE_ERR()
                (void)free(subinclude_path);
                return (NULL);
            }

            temp_dest = join_path(subinclude_path, path_basename(include.path));

            if (!temp_dest) {
                printf(" | FAILURE\n");
                PROPAGATE_ERR()
                (void)free(include.path);
                (void)free(subinclude_path);
                return (NULL);
            }

            if (include.isdir) {
                if (copytree(include.path, temp_dest, include.overwrite) && !include.skip_error) {
                    printf(" | FAILURE\n");
                    RAISE_FMT(ERR_OS, "failed to copy tree '%s' to '%s'.", include.path, temp_dest);
                    (void)free(temp_dest);
                    (void)free(include.path);
                    (void)free(subinclude_path);
                    return (NULL);
                }
            } else {
                if (is_file(temp_dest) && !include.overwrite) {
                    (void)free(temp_dest);
                    (void)free(include.path);
                    continue;
                }
                if (copy_file(include.path, temp_dest) && !include.skip_error) {
                    printf(" | FAILURE\n");
                    RAISE_FMT(ERR_OS, "failed to copy file '%s' to '%s'.", include.path, subinclude_path);
                    (void)free(temp_dest);
                    (void)free(include.path);
                    (void)free(subinclude_path);
                    return (NULL);
                }
            }
            (void)free(include.path);
            (void)free(temp_dest);
        }
    }

    return (subinclude_path);
}

uint8_t prepare_executable_subinclude_path(const char *subinclude_path, const EngineConfig *config)
{
    char *temp_dest;
    EngineGeneratorItem *res;

    if (!is_dir(subinclude_path)) {
        if (make_dir(subinclude_path)) {
            PROPAGATE_ERR();
            return (1);
        }
    }

    render_step("HEADERS", 0, config->generator.size);

    for (size_t i = 0; i < config->generator.size; ++i) {
        render_step("HEADERS", i + 1, config->generator.size);

        res = config->generator.content[i];

        if (res->type != GENT_INCLUDE)
            continue;

        temp_dest = join_path(subinclude_path, path_basename(res->location));

        if (!temp_dest) {
            printf(" | FAILURE\n");
            PROPAGATE_ERR()
            return (1);
        }

        if (copy_file(res->location, temp_dest)) {
            printf(" | FAILURE\n");
            RAISE_FMT(ERR_OS, "failed to copy file '%s' to '%s'.", res->location, subinclude_path);
            (void)free(temp_dest);
            return (1);
        }

        (void)free(temp_dest);
    }

    return (0);
}

char *init_sublib_path(const char *dist_path, const char *base_lib_path, const CNBuild *build)
{
    char *sublib_path = join_path(dist_path, "bin");
    char *temp_dest;
    SubModule *submodule;
    SubModuleLib lib;

    if (!sublib_path) {
        PROPAGATE_ERR();
        return (NULL);
    }

    if (!is_dir(sublib_path)) {
        if (make_dir(sublib_path)) {
            PROPAGATE_ERR();
            (void)free(sublib_path);
            return (NULL);
        }
    }

    render_step("LIBS   ", 0, build->dependencies.size);

    for (size_t i = 0; i < build->dependencies.size; ++i) {
        render_step("LIBS   ", i + 1, build->dependencies.size);

        submodule = build->dependencies.content[i];

        for (size_t j = 0; j < submodule->libs.size; ++j) {
            memcpy(&lib, submodule->libs.content[j], sizeof(SubModuleLib));

            lib.path = strdup(lib.path);

            if (!lib.path) {
                printf(" | FAILURE\n");
                RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new include path str.");
                (void)free(sublib_path);
                return (NULL);
            }

            lib.path = resolve_path(lib.path, base_lib_path, "${LIBS}");

            if (!lib.path) {
                printf(" | FAILURE\n");
                PROPAGATE_ERR()
                (void)free(sublib_path);
                return (NULL);
            }

            lib.path = resolve_path(lib.path, extension_from_system(build->machine), "${EXT}");

            if (!lib.path) {
                printf(" | FAILURE\n");
                PROPAGATE_ERR()
                (void)free(sublib_path);
                return (NULL);
            }

            temp_dest = join_path(sublib_path, path_basename(lib.path));

            if (!temp_dest) {
                printf(" | FAILURE\n");
                PROPAGATE_ERR()
                (void)free(lib.path);
                (void)free(sublib_path);
                return (NULL);
            }

            if (lib.isdir) {
                if (copytree(lib.path, temp_dest, lib.overwrite) && !lib.skip_error) {
                    printf(" | FAILURE\n");
                    RAISE_FMT(ERR_OS, "failed to copy tree '%s' to '%s'.", lib.path, temp_dest);
                    (void)free(temp_dest);
                    (void)free(sublib_path);
                    (void)free(lib.path);
                    return (NULL);
                }
            } else {
                if (is_file(temp_dest) && !lib.overwrite) {
                    (void)free(lib.path);
                    (void)free(temp_dest);
                    continue;
                }
                if (copy_file(lib.path, temp_dest) && !lib.skip_error) {
                    printf(" | FAILURE\n");
                    RAISE_FMT(ERR_OS, "failed to copy file '%s' to '%s'.", lib.path, sublib_path);
                    (void)free(temp_dest);
                    (void)free(sublib_path);
                    (void)free(lib.path);
                    return (NULL);
                }
            }
            (void)free(lib.path);
            (void)free(temp_dest);
        }
    }

    return (sublib_path);
}

char *build_name_from_cnbuild(const CNBuild *build)
{
    size_t len = strlen(build->name) + 1 + strlen(sysname_from_system(build->machine)) + 1 + strlen(archname_from_arch(build->arch));
    char *name = malloc(sizeof(char) * (len + 1));

    if (!name) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new string of size %zu.", len + 1);
        return (NULL);
    }

    (void)snprintf(name, len, "%s_%s_%s", build->name, sysname_from_system(build->machine), archname_from_arch(build->arch));
    return (name);
}

uint8_t construct_build(const EngineConfig *config, const CNProject *project, const CNBuild *build, const char *build_path, const char *dist_path, const char *self_path)
{
    printf("* [BUILD#%s]\n", build->name ? build->name : "???");

    printf("-======- Preparation -======-\n");

    const EngineRessourceSet *temp_ressource_set = find_ressource_set(config, build->arch, build->machine);
    char *temp_build_path = init_subbuild_path(build_path, build->name);

    if (!temp_build_path) {
        PROPAGATE_ERR();
        return (1);
    }

    char *temp_dist_path = init_subdist_path(dist_path, build->name);

    if (!temp_dist_path) {
        PROPAGATE_ERR();
        (void)free(temp_build_path);
        return (1);
    }

    char *include_path = init_subinclude_path(temp_build_path, temp_ressource_set->include_path, build);

    if (!include_path) {
        PROPAGATE_ERR();
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        return (1);
    }

    char *lib_path = init_sublib_path(temp_dist_path, temp_ressource_set->lib_path, build);

    if (!lib_path) {
        PROPAGATE_ERR();
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        (void)free(include_path);
        return (1);
    }

    String *library_output_name = new_str(join_path(lib_path, "game"));

    if (!library_output_name || str_is_null(library_output_name)) {
        PROPAGATE_ERR();
        if (library_output_name)
            (void)delete_str(library_output_name);
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        (void)free(include_path);
        (void)free(lib_path);
        return (1);
    }

    str_override(library_output_name, replace_extension(library_output_name->c_str, extension_from_system(build->machine)));

    if (str_is_null(library_output_name)) {
        PROPAGATE_ERR();
        (void)delete_str(library_output_name);
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        (void)free(include_path);
        (void)free(lib_path);
        return (1);
    }

    printf("-======- Source Compilation -======-\n");

    if (compile_library(project, build, library_output_name->c_str, temp_build_path, include_path, lib_path)) {
        PROPAGATE_ERR();
        (void)delete_str(library_output_name);
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        (void)free(include_path);
        (void)free(lib_path);
        return (1);
    }

    printf("-======- Executable Preparation -======-\n");

    str_override(library_output_name, join_path(lib_path, project->name ? project->name : "game"));

    if (str_is_null(library_output_name)) {
        PROPAGATE_ERR();
        (void)delete_str(library_output_name);
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        (void)free(include_path);
        (void)free(lib_path);
        return (1);
    }

    str_override(library_output_name, replace_extension(library_output_name->c_str, extension_executable_from_system(build->machine)));

    if (str_is_null(library_output_name)) {
        PROPAGATE_ERR();
        (void)delete_str(library_output_name);
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        (void)free(include_path);
        (void)free(lib_path);
        return (1);
    }

    if (prepare_executable_subinclude_path(include_path, config)) {
        PROPAGATE_ERR();
        (void)delete_str(library_output_name);
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        (void)free(include_path);
        (void)free(lib_path);
        return (1);
    }

    printf("-======- Executable Compilation -======-\n");

    if (compile_executable(config, project, build, library_output_name->c_str, temp_build_path, include_path, lib_path)) {
        PROPAGATE_ERR();
        (void)delete_str(library_output_name);
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        (void)free(include_path);
        (void)free(lib_path);
        return (1);
    }

    (void)delete_str(library_output_name);
    (void)free(include_path);
    (void)free(lib_path);

    printf("-======- Game Assets Compilation -======-\n");

    if (compile_assets(project, build, temp_dist_path, temp_build_path, self_path)) {
        PROPAGATE_ERR();
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        return (1);
    }

    (void)free(temp_build_path);
    (void)free(temp_dist_path);
    return (0);
}

CNProject *parse_project(const EngineConfig *config, const char *output_path, const char *file_path, Object *asset_ctx, const char *self_path)
{
    CNProject *project;
    CNBuild *temp_build;
    char *project_root = get_dirname(file_path);
    char *dist_path;
    char *build_path;

    (void)asset_ctx;

    if (!project_root) {
        PROPAGATE_ERR();
        return (NULL);
    }

    build_path = init_build_path(project_root);

    if (!build_path) {
        PROPAGATE_ERR();
        (void)free(project_root);
        return (NULL);
    }

    dist_path = init_dist_path(output_path);

    if (!dist_path) {
        PROPAGATE_ERR();
        (void)free(project_root);
        (void)free(build_path);
        return (NULL);
    }

    project = parse_project_xml(config, file_path, project_root);

    if (!project) {
        PROPAGATE_ERR();
        (void)free(dist_path);
        (void)free(build_path);
        (void)free(project_root);
        return (NULL);
    }

    for (size_t i = 0; i < project->builds.size; ++i) {
        temp_build = project->builds.content[i];
        if (!has_ressource_set(config, temp_build->arch, temp_build->machine)) {
            RAISE_FMT(ERR_NOT_COMPATIBLE, "no ressource set matching arch/os found for build %s.", temp_build->name)
            (void)free(dist_path);
            (void)free(build_path);
            (void)free(project_root);
            return (NULL);
        }
        if (construct_build(config, project, project->builds.content[i], build_path, dist_path, self_path)) {
            PROPAGATE_ERR();
            (void)free(dist_path);
            (void)free(build_path);
            (void)free(project_root);
            return (NULL);
        }
    }

    (void)free(dist_path);
    (void)free(build_path);
    (void)free(project_root);

    return (project);
}

int build_project(size_t argc, char **argv, Object *asset_ctx)
{
    struct build_args_s build_args = {0};
    CNProject *left_overs;
    EngineConfig *config;

    if (build_get_args(argc - 3, argv + 3, "project", &build_args)) {
        PROPAGATE_ERR();
        return (1);
    }

    if (build_args.input_files.size == 0) {
        fprintf(stderr, "missing input file.\n");
        (void)reset_args(&build_args);
        return (1);
    }

    if (build_args.input_files.size > 1) {
        fprintf(stderr, "too much input files.\n");
        (void)reset_args(&build_args);
        return (1);
    }

    config = parse_config_xml("./assets/config.xml", "./");

    if (!config) {
        PROPAGATE_ERR();
        (void)reset_args(&build_args);
        return (1);
    }

    left_overs = parse_project(config, build_args.output_file, build_args.input_files.content[0], asset_ctx, argv[0]);

    delete_engine_config(config);

    if (!left_overs) {
        PROPAGATE_ERR();
        (void)reset_args(&build_args);
        return (1);
    }

    (void)delete_cnproject(left_overs);
    (void)reset_args(&build_args);

    return (0);
}