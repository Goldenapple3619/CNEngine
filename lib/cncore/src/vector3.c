#include "libcncore.h"

#include <stdlib.h>

CN_API Vector3 *new_vector3(cnnumber x, cnnumber y, cnnumber z)
{
    Vector3 *v = (Vector3 *)malloc(sizeof(Vector3));

    if (!v)
        return (NULL);
    v->x = x;
    v->y = y;
    v->z = z;
    return (v);
}

CN_API void delete_vector3(Vector3 *v)
{
    if (!v)
        return;
    (void)free((void *)v);
}
