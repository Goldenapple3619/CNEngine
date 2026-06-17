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

CNProject *parse_project(const char *file_path, Object *asset_ctx)
{
    CNProject *project;
    char *project_root = get_dirname(file_path);
    char *build_path;
    char *include_path = "dist/linux-amd64/include";
    
    (void)asset_ctx;

    if (!project_root)
        return (NULL);

    build_path = init_build_path(project_root);

    if (!build_path) {
        (void)free(project_root);
        return (NULL);
    }

    project = parse_project_xml(file_path, project_root);

    if (!project) {
        (void)free(build_path);
        (void)free(project_root);
        return (NULL);
    }

    if (compile_library(project, build_path, include_path)) {
        (void)free(build_path);
        (void)free(project_root);
        return (NULL) ;
    }
    
    (void)free(build_path);
    (void)free(project_root);

    return (project);
}

int build_project(size_t argc, char **argv, Object *asset_ctx)
{
    struct build_args_s build_args = {0};
    CNProject *left_overs;
    
    (void)asset_ctx;

    if (build_get_args(argc - 3, argv + 3, &build_args)) {
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

    left_overs = parse_project(build_args.input_files.content[0], asset_ctx);

    if (!left_overs) {
        (void)reset_args(&build_args);
        return (1);
    }

    (void)delete_cnproject(left_overs);
    (void)reset_args(&build_args);

    return (0);
}