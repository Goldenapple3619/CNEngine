#include "libr2d.h"

static cn_value _init(Object *__this, void **args)
{
    (void)args;

    call_method(__this->base, "_init", args);

    INIT_VEC2(__this, ((Vector2){.x = 0, .y = 0}), "velocity");

    return (VALUE_OK);
}

static cn_value _update(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    double delta_time = *(double *)args[0];
    Vector2 *velocity = &get_attr(__this, "velocity")->as.vec2;

    if (velocity->x == 0 && velocity->y == 0)
        return (null_value);

    Vector3 *position = &get_attr(__this, "position")->as.vec3;

    position->x = position->x + (velocity->x * delta_time); 
    position->y = position->y + (velocity->y * delta_time); 

    return (null_value);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;
    (void)__this;

    return (null_value);
}

CN_API Object *new_camera2d(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD_STATIC(obj, new_scene_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_update", &_update);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}