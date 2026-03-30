#include "libr3d.h"

CN_API Mesh *new_mesh(void)
{
    Mesh *mesh = malloc(sizeof(Mesh));

    if (!mesh)
        return (NULL);
    mesh->vertex_count = 0;
    mesh->vao = 0;
    return (mesh);
}

CN_API void delete_mesh(Mesh *mesh)
{
    if (!mesh)
        return;
    (void)free(mesh);
}
