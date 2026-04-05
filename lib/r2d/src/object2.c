#include "libr2d.h"

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()

    (void)args;

    call_method(__this->base, "_init", args);

    INIT_CUSTOM_ALLOCATION(__this, new_texture_from_file("./assets/dirt.png"), delete_texture, "texture");

    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;

    PREP_DEL()

    DEL_CUSTOM_ALLOCAION(__this, delete_texture, "texture")

    return (null_value);
}

CN_API Object *new_object2(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, new_scene_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}