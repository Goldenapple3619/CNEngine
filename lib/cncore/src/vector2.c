#include "libcncore.h"

#include <stdlib.h>

CN_API Vector2 *new_vector2(cnnumber x, cnnumber y)
{
    Vector2 *v = (Vector2 *)malloc(sizeof(Vector2));

    if (!v)
        return (NULL);
    v->x = x;
    v->y = y;
    return (v);
}

CN_API void delete_vector2(Vector2 *v)
{
    if (!v)
        return;
    (void)free((void *)v);
}
