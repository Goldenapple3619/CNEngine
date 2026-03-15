#include "libcncore.h"
#include "libcngraphic.h"

static cn_value _draw(Object *__this, void **args)
{
    (void)args;

    ObjectVector *interfaces = get_attr(__this, "interfaces")->as.ptr;

    for (size_t i = 0; i < interfaces->size; ++i)
        (void)call_method(interfaces->objects[i], "_draw", NULL);

    return (null_value);
}

static cn_value _init(Object *__this, void **args)
{
    (void)__this;

    PREP_INIT()

    Videomode v = {.size.x = 800, .size.y = 600, .position.x = 0, .position.y = 0, .flags = VDM_CLOSABLE, .native_flags = VDM_N_SHWN};

    if (!args || !(args[0]))
        return (VALUE_ERR);
    
    Object *ctx = (Object *)(args[0]);
    
    if (!start_graphics())
        return (VALUE_ERR);

    INIT_CUSTOM_ALLOCATION(ctx, new_object_vector(), delete_object_vector, "interfaces");


    Object *temp = new_interface();

    if (!temp || call_method(temp, "_init", (cnany []){(cnany)"test", NULL, &v, NULL}).as.i == VALUE_ERR.as.i) {
        (void)delete_object(temp);
        return (VALUE_ERR);
    }
    insert_object_vector(get_attr(ctx, "interfaces")->as.ptr, temp);

    if (!set_method(ctx, "draw", _draw))
        return (VALUE_ERR);
    
    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)__this;

    PREP_DEL()

    if (!args || !(args[0]))
        return (VALUE_ERR);
    
    Object *ctx = (Object *)(args[0]);

    DEL_CUSTOM_ALLOCAION(ctx, delete_object_vector, "interfaces");

    end_graphics();
    
    return (VALUE_OK);
}

CN_API Object *new_graphic_submodule(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}
