#include "project_toolchain.h"

CNProject *new_cnproject(void)
{
    CNProject *proj = malloc(sizeof(CNProject));

    if (!proj)
        return (NULL);

    proj->builds.capacity = 0;
    proj->builds.size = 0;
    proj->builds.content = NULL;

    proj->name = NULL;
    proj->version_name = NULL;

    proj->content.capacity = 0;
    proj->content.size = 0;
    proj->content.content = NULL;

    return (proj);
}

void delete_cnproject(CNProject *ptr)
{
    if (!ptr)
        return;
    if (ptr->builds.content)
        (void)empty_generic_vector(&ptr->builds, (expr_free)&delete_build);
    if (ptr->content.content)
        (void)empty_generic_vector(&ptr->content, (expr_free)&delete_cnasset);
    if (ptr->name)
        (void)free(ptr->name);
    if (ptr->version_name)
        (void)free(ptr->version_name);
    (void)free(ptr);
}
