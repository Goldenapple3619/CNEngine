#include "project_toolchain.h"

CNBuild *new_build(void)
{
    CNBuild *build = malloc(sizeof(CNBuild));

    if (!build) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new build.");
        return (NULL);
    }

    build->arch = CNBUILD_ARCH_HOST;
    build->dependencies.capacity = 0;
    build->dependencies.size = 0;
    build->dependencies.content = NULL;
    build->machine = CNBUILD_SYS_HOST;
    build->scene_entry_point = NULL;
    build->name = NULL;
    return (build);
}

uint8_t build_set_entry_point(CNBuild *build, const char *entry_point)
{
    if (!build) {
        RAISE(ERR_INVALID_POINTER, "can't set entry point to empty build.");
        return (1);
    }

    if (!entry_point) {
        RAISE(ERR_INVALID_POINTER, "can't set empty entry point to build.");
        return (1);
    }

    if (build->scene_entry_point) {
        (void)free(build->scene_entry_point);
        build->scene_entry_point = NULL;
    }

    build->scene_entry_point = strdup(entry_point);

    if (!build->scene_entry_point) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new scene entry point string.");
        return (1);
    }

    return (0);
}

uint8_t build_set_name(CNBuild *build, const char *name)
{
    if (!build) {
        RAISE(ERR_INVALID_POINTER, "can't set name to empty build.");
        return (1);
    }

    if (!name) {
        RAISE(ERR_INVALID_POINTER, "can't set empty name to build.");
        return (1);
    }

    if (build->name) {
        (void)free(build->name);
        build->name = NULL;
    }

    build->name = strdup(name);

    if (!build->name) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate build name.");
        return (1);
    }

    return (0);
}


void delete_build(CNBuild *build)
{
    if (!build) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty build.");
        return;
    }

    if (build->dependencies.content)
        (void)empty_generic_vector(&build->dependencies, (expr_free)&delete_submodule);
    if (build->name)
        (void)free(build->name);
    if (build->scene_entry_point)
        (void)free(build->scene_entry_point);
    (void)free(build);
}
