#include "librgui.h"

static cn_value _init(Object *__this, void **args)
{
    (void)__this;

    if (!args || !(args[0]))
        return (VALUE_ERR);
    
    if (!start_gui())
        return (VALUE_ERR);
    
    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)__this;

    if (!args || !(args[0]))
        return (VALUE_ERR);

    end_gui();
    
    return (VALUE_OK);
}

CN_API Object *new_gui_submodule(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}
