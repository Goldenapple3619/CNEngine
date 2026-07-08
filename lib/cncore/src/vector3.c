#include "libcncore.h"

#include <stdlib.h>

CN_API Vector3 *new_vector3(cnnumber x, cnnumber y, cnnumber z)
{
    Vector3 *v = (Vector3 *)malloc(sizeof(Vector3));

    if (!v) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate Vec3.");
        return (NULL);
    }
    v->x = x;
    v->y = y;
    v->z = z;
    return (v);
}

CN_API void delete_vector3(Vector3 *v)
{
    if (!v) {
        RAISE(ERR_INVALID_POINTER, "can't remove empty Vec3.");
        return;
    }
    (void)free((void *)v);
}
