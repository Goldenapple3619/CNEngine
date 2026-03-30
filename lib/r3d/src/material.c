#include "libr3d.h"

CN_API Material *new_material(void)
{
    Material *material = malloc(sizeof(Material));

    if (!material)
        return (NULL);
    material->color = 0;
    material->shader = 0;
    material->texture = NULL;
    return (material);
}

CN_API void delete_material(Material *material)
{
    if (material)
        return;
    (void)free(material);
}
