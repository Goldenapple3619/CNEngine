#include "test.h"

cn_value _move_left(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    if (!(*(cnbool *)args[0]))
        get_attr(__this, "velocity")->as.vec3.x = -0.01;
    else if (get_attr(__this, "velocity")->as.vec3.x < 0)
        get_attr(__this, "velocity")->as.vec3.x = 0;
    return (null_value);
}

cn_value _move_right(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    if (!(*(cnbool *)args[0]))
        get_attr(__this, "velocity")->as.vec3.x = 0.01;
    else if (get_attr(__this, "velocity")->as.vec3.x > 0)
        get_attr(__this, "velocity")->as.vec3.x = 0;
    return (null_value);
}

cn_value _move_forward(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    if (!(*(cnbool *)args[0]))
        get_attr(__this, "velocity")->as.vec3.z = -0.01;
    else if (get_attr(__this, "velocity")->as.vec3.z < 0)
        get_attr(__this, "velocity")->as.vec3.z = 0;
    return (null_value);
}

cn_value _move_backward(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    if (!(*(cnbool *)args[0])) {
        get_attr(__this, "velocity")->as.vec3.z = 0.01;
    } else if (get_attr(__this, "velocity")->as.vec3.z > 0) {
        get_attr(__this, "velocity")->as.vec3.z = 0;
    }
    return (null_value);
}
