#include "libcngraphic.h"

CN_API Event *new_event(cn_event type, cnnumber x, cnnumber y, int64_t v)
{
    Event *ev = (Event *)malloc(sizeof(Event));

    if (!ev)
        return (NULL);

    ev->type = type;
    ev->x = x;
    ev->y = y;
    ev->v = v;
    return (ev);
}

CN_API void delete_event(Event *ev)
{
    if (!ev)
        return;
    ev->type = EV_NULL;
    (void)free(ev);
}