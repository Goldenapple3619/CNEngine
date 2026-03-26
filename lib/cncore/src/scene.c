#include "libcncore.h"

static cn_value _init(Object *__this, void **args)
{
    (void)args;

    PREP_INIT()

    INIT_OBJECT_STATIC(__this, new_list(), NULL, "objects");

    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)__this;
    (void)args;

    return (null_value);
}

CN_API Object *new_scene(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());

    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    return (obj);
}
