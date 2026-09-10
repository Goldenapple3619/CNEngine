#include "libr2d.h"

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()

    if (call_method(__this->base, "_init", args).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }

    INIT_CUSTOM_ALLOCATION(__this, new_texture_from_file("./assets/dirt.png"), delete_texture, "texture");

    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;

    PREP_DEL()

    DEL_CUSTOM_ALLOCAION(__this, delete_texture, "texture")

    return (VALUE_NULL);
}

CN_API Object *new_object2d(void)
{
    Object *obj = new_object();

    if (!obj) {
        PROPAGATE_ERR();
        return (NULL);
    }

    SET_PARENT_CLASS_BUILD_STATIC(obj, new_scene_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}