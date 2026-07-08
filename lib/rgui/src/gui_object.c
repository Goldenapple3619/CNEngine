#include "librgui.h"

static cn_value _init(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't init gui object with no gui object mode.");
        return (VALUE_ERR);
    }

    PREP_INIT()

    struct gui_object_mode_s *mode = (struct gui_object_mode_s *)(args[0]);

    INIT_VEC2(__this, mode->position, "position");
    INIT_VEC2(__this, mode->scale, "scale");
    INIT_NUMBER(__this, mode->rotation, "rotation");

    INIT_VEC2(__this, ((Vector2){0, 0}), "size");

    INIT_INT(__this, mode->align, "align");
    INIT_INT(__this, mode->justify, "justify");
    INIT_INT(__this, mode->positionning, "positionning");
    INIT_INT(__this, mode->zindex, "z-index");

    INIT_OBJECT_STATIC(__this, new_list((expr_free)&collect_object), NULL, "childs");

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

    SET_PARENT_CLASS_BUILD_STATIC(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}