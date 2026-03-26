#include "libcncore.h"

static cn_value _init(Object *__this, void **args)
{

    if (!args || !args[0])
        return (VALUE_ERR);

    struct scene_object_mode_s *mode = args[0];

    INIT_VEC3(__this, mode->coords, "position");
    INIT_RECT(__this, mode->rotation, "rotation");
    INIT_VEC3(__this, mode->scale, "scale");

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
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    return (obj);
}
