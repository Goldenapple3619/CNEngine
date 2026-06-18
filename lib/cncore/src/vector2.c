#include "libcncore.h"

#include <stdlib.h>

CN_API Vector2 *new_vector2(cnnumber x, cnnumber y)
{
    Vector2 *v = (Vector2 *)malloc(sizeof(Vector2));

    if (!v) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate Vec2.");
        return (NULL);
    }
    v->x = x;
    v->y = y;
    return (v);
}

CN_API Vector2 add_vector2(const Vector2 *vec0, const Vector2 *vec1)
{
    return (Vector2){.x = vec1->x + vec0->x, .y = vec1->y + vec0->y};
}

CN_API void delete_vector2(Vector2 *v)
{
    if (!v) {
        RAISE(ERR_INVALID_POINTER, "can't remove empty Vec2.");
        return;
    }
    (void)free((void *)v);
}
