#include "libcncore.h"

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()

    INIT_CUSTOM_ALLOCATION(__this, new_object_vector(), delete_object_vector, "_vec");

    if (args)
        (void)call_method(__this, "push", args);

    return (VALUE_OK);
}

static cn_value push(Object *__this, void **args)
{
    if (!args)
        return (VALUE_ERR);
    
    ObjectVector *vec = get_attr(__this, "_vec")->as.ptr;

    for (size_t i = 0; args[i]; ++i) {
        if (insert_object_vector(vec, (Object *)args[i]))
            return (VALUE_ERR);
    }
    return (VALUE_OK);
}

static cn_value len(Object *__this, void **args)
{
    (void)args;

    ObjectVector *vec = get_attr(__this, "_vec")->as.ptr;

    return ((cn_value){.type=CN_TYPE_INT, .as.i=vec->size});
}

static cn_value remove(Object *__this, void **args)
{
    if (!args || !(args[0]))
        return (VALUE_ERR);
    
    ObjectVector *vec = get_attr(__this, "_vec")->as.ptr;
    
    (void)remove_object_ordered_vector(vec, *((size_t *)args[0]));
    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;

    PREP_DEL()

    DEL_CUSTOM_ALLOCAION(__this, delete_object_vector, "_vec");

    return (null_value);
}

CN_API Object *new_list()
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "push", &push);
    CREATE_METHOD_CLASS_BUILD(obj, "len", &len);
    CREATE_METHOD_CLASS_BUILD(obj, "remove", &remove);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    return (obj);
}

