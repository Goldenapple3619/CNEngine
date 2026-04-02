#include "libcncore.h"

static cn_value _init(Object *__this, void **args)
{

    if (!args || !args[0])
        return (VALUE_ERR);

    PREP_INIT()

    struct scene_object_mode_s *mode = args[0];

    INIT_VEC3(__this, mode->coords, "position");
    INIT_VEC3(__this, mode->rotation, "rotation");
    INIT_VEC3(__this, mode->scale, "scale");

    INIT_INT(__this, mode->flags, "_flags");

    INIT_OBJECT_STATIC(__this, new_list(), NULL, "childs");

    return (VALUE_OK);
}

static cn_value _add_child(Object *__this, void **args)
{
    if (!args || !args[0])
        return (VALUE_ERR);

    Object *childs = get_attr(__this, "childs")->as.ptr;

    call_method(childs, "push", args);

    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)__this;
    (void)args;

    return (null_value);
}


CN_API Object *new_scene_object(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());

    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "add_child", &_add_child);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    return (obj);
}
