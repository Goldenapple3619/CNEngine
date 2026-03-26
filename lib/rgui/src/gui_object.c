#include "librgui.h"

static cn_value _init(Object *__this, void **args)
{
    if (!args || !args[0])
        return (VALUE_ERR);

    INIT_VEC2(__this, *(Vector2 *)args[0], "position");

    return (VALUE_OK);
}


static cn_value _del(Object *__this, void **args)
{
    (void)args;
    (void)__this;


    return (null_value);
}

CN_API Object *new_guiobject(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}