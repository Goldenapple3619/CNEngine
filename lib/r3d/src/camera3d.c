#include "libr3d.h"

static cn_value _init(Object *__this, void **args)
{
    (void)args;

    if (call_method(__this->base, "_init", args).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }

    INIT_VEC3(__this, ((Vector3){.x = 0, .y = 0, .z = 0}), "velocity");
    INIT_NUMBER(__this, 60.0f, "fov");
    INIT_NUMBER(__this, 0.1f, "near");
    INIT_NUMBER(__this, 1000.0f, "far");

    return (VALUE_OK);
}

static cn_value _update(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't update camera with no set delta time.")
        return (VALUE_NULL);
    }

    double delta_time = *(double *)args[0];
    Vector3 *velocity = &get_attr(__this, "velocity")->as.vec3;

    if (velocity->x == 0 && velocity->y == 0 && velocity->z == 0)
        return (VALUE_NULL);

    Vector3 *position = &get_attr(__this, "position")->as.vec3;

    position->x = position->x + (velocity->x * delta_time); 
    position->y = position->y + (velocity->y * delta_time); 
    position->z = position->z + (velocity->z * delta_time); 

    return (VALUE_NULL);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;
    (void)__this;

    return (VALUE_NULL);
}

CN_API Object *new_camera3d(void)
{
    Object *obj = new_object();

    if (!obj) {
        PROPAGATE_ERR();
        return (NULL);
    }

    SET_PARENT_CLASS_BUILD_STATIC(obj, new_scene_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_update", &_update);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}