#include "project_toolchain.h"

char *init_build_path(const char *project_root)
{
    char *build_path = join_path(project_root, "build");

    if (!build_path)
        return (NULL);

    if (!is_dir(build_path)) {
        if (MKDIR(build_path)) {
            (void)free(build_path);
            return (NULL);
        }
    }

    return (build_path);
}

char *init_dist_path(const char *output_path)
{
    char *dist_path = join_path(output_path, "dist");

    if (!dist_path)
        return (NULL);

    if (!is_dir(dist_path)) {
        if (MKDIR(dist_path)) {
            (void)free(dist_path);
            return (NULL);
        }
    }

    return (dist_path);
}

char *init_subdist_path(const char *dist_path, const char *project_name)
{
    char *subdist_path = join_path(dist_path, project_name);

    if (!subdist_path)
        return (NULL);

    if (!is_dir(subdist_path)) {
        if (MKDIR(subdist_path)) {
            (void)free(subdist_path);
            return (NULL);
        }
    }

    return (subdist_path);
}

char *init_subbuild_path(const char *build_path, const char *project_name)
{
    char *subbuild_path = join_path(build_path, project_name);

    if (!subbuild_path)
        return (NULL);

    if (!is_dir(subbuild_path)) {
        if (MKDIR(subbuild_path)) {
            (void)free(subbuild_path);
            return (NULL);
        }
    }

    return (subbuild_path);
}

char *init_subinclude_path(const char *build_path, const char *base_include_path, const CNBuild *build)
{
    char *subinclude_path = join_path(build_path, "include");
    char *temp;
    char *temp_d;

    if (!subinclude_path)
        return (NULL);

    if (!is_dir(subinclude_path)) {
        if (MKDIR(subinclude_path)) {
            (void)free(subinclude_path);
            return (NULL);
        }
    }

    for (size_t i = 0; i < build->dependencies.size; ++i) {
        if (!strcmp(build->dependencies.content[i], "core")) {
            temp = join_path(base_include_path, "libcncore.h");
            if (copy_file(temp, subinclude_path)) {
                (void)free(subinclude_path);
                (void)free(temp);
                return (NULL);
            }
            (void)free(temp);
            temp = join_path(base_include_path, "SDL2");
            temp_d = join_path(subinclude_path, "SDL2");
            if (copytree(temp, temp_d)) {
                (void)free(temp_d);
                (void)free(subinclude_path);
                (void)free(temp);
                return (NULL);
            }
            (void)free(temp);
            (void)free(temp_d);
        } else if (!strcmp(build->dependencies.content[i], "graphic")) {
            temp = join_path(base_include_path, "libcngraphic.h");
            if (copy_file(temp, subinclude_path)) {
                (void)free(subinclude_path);
                (void)free(temp);
                return (NULL);
            }
            (void)free(temp);
            temp = join_path(base_include_path, "SDL_image.h");
            if (copy_file(temp, subinclude_path)) {
                (void)free(subinclude_path);
                (void)free(temp);
                return (NULL);
            }
            (void)free(temp);
            temp = join_path(base_include_path, "glad");
            temp_d = join_path(subinclude_path, "glad");
            if (copytree(temp, temp_d)) {
                (void)free(temp_d);
                (void)free(subinclude_path);
                (void)free(temp);
                return (NULL);
            }
            (void)free(temp);
            (void)free(temp_d);
            temp = join_path(base_include_path, "KHR");
            temp_d = join_path(subinclude_path, "KHR");
            if (copytree(temp, temp_d)) {
                (void)free(temp_d);
                (void)free(subinclude_path);
                (void)free(temp);
                return (NULL);
            }
            (void)free(temp_d);
            (void)free(temp);
        } else if (!strcmp(build->dependencies.content[i], "assets")) {
            temp = join_path(base_include_path, "libcnassets.h");
            if (copy_file(temp, subinclude_path)) {
                (void)free(subinclude_path);
                (void)free(temp);
                return (NULL);
            }
            (void)free(temp);
        } else if (!strcmp(build->dependencies.content[i], "audio")) {
            temp = join_path(base_include_path, "libcnaudio.h");
            if (copy_file(temp, subinclude_path)) {
                (void)free(subinclude_path);
                (void)free(temp);
                return (NULL);
            }
            (void)free(temp);
            temp = join_path(base_include_path, "SDL_mixer.h");
            if (copy_file(temp, subinclude_path)) {
                (void)free(subinclude_path);
                (void)free(temp);
                return (NULL);
            }
            (void)free(temp);
        } else if (!strcmp(build->dependencies.content[i], "rgui")) {
            temp = join_path(base_include_path, "librgui.h");
            if (copy_file(temp, subinclude_path)) {
                (void)free(subinclude_path);
                (void)free(temp);
                return (NULL);
            }
            (void)free(temp);
            temp = join_path(base_include_path, "SDL_ttf.h");
            if (copy_file(temp, subinclude_path)) {
                (void)free(subinclude_path);
                (void)free(temp);
                return (NULL);
            }
            (void)free(temp);
        } else if (!strcmp(build->dependencies.content[i], "r2d")) {
            temp = join_path(base_include_path, "libr2d.h");
            if (copy_file(temp, subinclude_path)) {
                (void)free(subinclude_path);
                (void)free(temp);
                return (NULL);
            }
            (void)free(temp);
        } else if (!strcmp(build->dependencies.content[i], "r3d")) {
            temp = join_path(base_include_path, "libr3d.h");
            if (copy_file(temp, subinclude_path)) {
                (void)free(subinclude_path);
                (void)free(temp);
                return (NULL);
            }
            (void)free(temp);
        }
    }

    return (subinclude_path);
}

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


char *build_name_from_cnbuild(const CNBuild *build)
{
    size_t len = strlen(build->name) + 1 + strlen(sysname_from_system(build->machine)) + 1 + strlen(archname_from_arch(build->arch));
    char *name = malloc(sizeof(char) * (len + 1));

    if (!name)
        return (NULL);

    (void)snprintf(name, len, "%s_%s_%s", build->name, sysname_from_system(build->machine), archname_from_arch(build->arch));
    return (name);
}

uint8_t construct_build(const CNProject *project, const CNBuild *build, const char *build_path, const char *dist_path)
{
    printf("preparing build directory.\n");
    char *temp_build_path = init_subbuild_path(build_path, build->name);

    if (!temp_build_path) {
        return (1);
    }

    printf("preparing dist directory.\n");
    char *temp_dist_path = init_subdist_path(dist_path, build->name);

    if (!temp_dist_path) {
        (void)free(temp_build_path);
        return (1);
    }

    printf("preparing includes.\n");
    char *include_path = init_subinclude_path(temp_build_path, "dist/linux-amd64/include", build);

    if (!include_path) {
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        return (1);
    }

    String *library_output_name = new_str(join_path(temp_dist_path, "game"));

    if (!library_output_name || str_is_null(library_output_name)) {
        if (library_output_name)
            (void)delete_str(library_output_name);
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        (void)free(include_path);
        return (1);
    }

    str_override(library_output_name, replace_extension(library_output_name->c_str, extension_from_system(build->machine)));

    if (str_is_null(library_output_name)) {
        (void)delete_str(library_output_name);
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        (void)free(include_path);
        return (1);
    }

    if (compile_library(project, build, library_output_name->c_str, temp_build_path, include_path)) {
        (void)delete_str(library_output_name);
        (void)free(temp_build_path);
        (void)free(temp_dist_path);
        (void)free(include_path);
        return (1);
    }

    (void)delete_str(library_output_name);
    (void)free(temp_build_path);
    (void)free(temp_dist_path);
    (void)free(include_path);
    return (0);
}

CNProject *parse_project(const char *output_path, const char *file_path, Object *asset_ctx)
{
    CNProject *project;
    char *project_root = get_dirname(file_path);
    char *dist_path;
    char *build_path;
    
    (void)asset_ctx;

    if (!project_root)
        return (NULL);

    build_path = init_build_path(project_root);

    if (!build_path) {
        (void)free(project_root);
        return (NULL);
    }

    dist_path = init_dist_path(output_path);

    if (!dist_path) {
        (void)free(project_root);
        (void)free(build_path);
        return (NULL);
    }

    project = parse_project_xml(file_path, project_root);

    if (!project) {
        (void)free(dist_path);
        (void)free(build_path);
        (void)free(project_root);
        return (NULL);
    }

    for (size_t i = 0; i < project->builds.size; ++i) {
        if (construct_build(project, project->builds.content[i], build_path, dist_path)) {
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
    
    (void)asset_ctx;

    if (build_get_args(argc - 3, argv + 3, "project", &build_args)) {
        (void)reset_args(&build_args);
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

    left_overs = parse_project(build_args.output_file, build_args.input_files.content[0], asset_ctx);

    if (!left_overs) {
        (void)reset_args(&build_args);
        return (1);
    }

    (void)delete_cnproject(left_overs);
    (void)reset_args(&build_args);

    return (0);
}