#include "libcncore.h"

CN_API ObjMethodPair *new_object_method_pair(Object *obj, cn_method method)
{
    if (!method) {
        RAISE(ERR_INVALID_POINTER, "can't create obj method pair with no method.");
        return (NULL);
    }

    ObjMethodPair *pair = (ObjMethodPair *)malloc(sizeof(ObjMethodPair));

    if (!pair) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate ObjMethodPair.");
        return (NULL);
    }
    pair->obj = share_object(obj);
    pair->method = method;
    return (pair);
}

CN_API ObjMethodPair *new_weak_object_method_pair(Object *obj, cn_method method)
{
    if (!method) {
        RAISE(ERR_INVALID_POINTER, "can't create weak obj method pair with no method.");
        return (NULL);
    }

    ObjMethodPair *pair = (ObjMethodPair *)malloc(sizeof(ObjMethodPair));

    if (!pair) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate ObjMethodPair.");
        return (NULL);
    }
    pair->obj = obj;
    pair->method = method;
    return (pair);
}

CN_API void delete_object_method_pair(ObjMethodPair *pair)
{
    if (!pair) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty ObjMethodPair.");
        return;
    }
    if (pair->obj) {
        (void)collect_object(pair->obj);

        pair->obj = NULL;
    }
    pair->method = NULL;
    (void)free(pair);
}

CN_API void delete_weak_object_method_pair(ObjMethodPair *pair)
{
    if (!pair) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty weak ObjMethodPair.");
        return;
    }
    pair->obj = NULL;
    pair->method = NULL;
    (void)free(pair);
}
