#include "libcncore.h"

CN_API ObjMethodPair *new_object_method_pair(Object *obj, cn_method *method)
{
    if (!method)
        return (NULL);

    ObjMethodPair *pair = (ObjMethodPair *)malloc(sizeof(ObjMethodPair));

    if (!pair)
        return (NULL);
    pair->obj = share_object(obj);
    pair->method = method;
    return (pair);
}

CN_API ObjMethodPair *new_weak_object_method_pair(Object *obj, cn_method *method)
{
    if (!method)
        return (NULL);

    ObjMethodPair *pair = (ObjMethodPair *)malloc(sizeof(ObjMethodPair));

    if (!pair)
        return (NULL);
    pair->obj = obj;
    pair->method = method;
    return (pair);
}

CN_API void delete_object_method_pair(ObjMethodPair *pair)
{
    if (!pair)
        return;
    if (pair->obj) {
        (void)release_object(pair->obj);

        if (pair->obj->ref_count <= 0)
            (void)delete_object(pair->obj);

        pair->obj = NULL;
    }
    pair->method = NULL;
    (void)free(pair);
}

CN_API void delete_weak_object_method_pair(ObjMethodPair *pair)
{
    if (!pair)
        return;
    pair->obj = NULL;
    pair->method = NULL;
    (void)free(pair);
}
