#include "libcncore.h"

CN_API Object *share_object(Object *object)
{
    if (!object)
        return (NULL);
    object->ref_count++;
    return (object);
}

CN_API Object *release_object(Object *object)
{
    if (!object)
        return (NULL);
    object->ref_count--;
    return (object);
}
