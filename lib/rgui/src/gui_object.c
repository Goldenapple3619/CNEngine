#include "librgui.h"

static cn_value _init(Object *__this, void **args)
{
    if (!args || !args[0])
        return (VALUE_ERR);

    PREP_INIT()

    INIT_VEC2(__this, *(Vector2 *)args[0], "position");
    INIT_INT(__this, GUI_ALIGN_LEFT, "align");
    INIT_INT(__this, 0, "z-index");

    INIT_OBJECT_STATIC(__this, new_list(), NULL, "childs");

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