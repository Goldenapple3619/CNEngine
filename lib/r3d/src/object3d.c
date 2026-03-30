#include "libr3d.h"

static cn_value _init(Object *__this, void **args)
{
    (void)args;

    PREP_INIT()

    call_method(__this->base, "_init", args);

    INIT_CUSTOM_ALLOCATION(__this, new_material(), delete_material, "material");
    INIT_CUSTOM_ALLOCATION(__this, new_mesh(), delete_mesh, "mesh");

    return (VALUE_OK);
}


static cn_value _del(Object *__this, void **args)
{
    (void)args;

    PREP_DEL()

    DEL_CUSTOM_ALLOCAION(__this, delete_material, "material");
    DEL_CUSTOM_ALLOCAION(__this, delete_mesh, "mesh");

    return (null_value);
}

CN_API Object *new_object3d(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, new_scene_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}