#include "libcncore.h"

#include <stdlib.h>

CN_API Rect *new_rect(cnnumber x, cnnumber y, cnnumber w, cnnumber h)
{
    Rect *v = (Rect *)malloc(sizeof(Rect));

    if (!v) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate Rect.");
        return (NULL);
    }
    v->x = x;
    v->y = y;
    v->w = w;
    v->h = h;
    return (v);
}

CN_API void delete_rect(Rect *v)
{
    if (!v) {
        RAISE(ERR_INVALID_POINTER, "can't remove empty Rect.");
        return;
    }
    (void)free((void *)v);
}
