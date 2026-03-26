#include "libcncore.h"

static cn_value _init(Object *__this, void **args)
{
    (void)args;

    PREP_INIT()

    INIT_OBJECT_STATIC(__this, new_list(), NULL, "objects");

    return (VALUE_OK);
}

static cn_value _update(Object *__this, void **args)
{
    Object *elements = get_attr(__this, "objects")->as.ptr;
    size_t len = call_method(elements, "len", NULL).as.i;
    cn_value val;
    Object *temp;
    int64_t flags;

    for (size_t i = 0; i < len; ++i) {
        val = call_method(elements, "at", (cnany []){(size_t []){i}, NULL});

        if (val.type == CN_TYPE_NULL)
            continue;
        
        temp = val.as.ptr;
        flags = get_attr(temp, "_flags")->as.i;
        
        if (((flags & CN_OBJ_REPLICATE) > 0) && !((flags & CN_OBJ_HOST) > 0))
            continue;

        if (has_method(temp, "_update"))
            (void)call_method(temp, "_update", args);
    }

    return (null_value);
}

static cn_value _add_element(Object *__this, void **args)
{
    return (call_method(get_attr(__this, "objects")->as.ptr, "push", args));
}

static cn_value _del(Object *__this, void **args)
{
    (void)__this;
    (void)args;

    return (null_value);
}

CN_API Object *new_scene(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());

    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_update", &_update);
    CREATE_METHOD_CLASS_BUILD(obj, "add_element", &_add_element);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    return (obj);
}
