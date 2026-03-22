#include "libcngraphic.h"

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()
    INIT_CUSTOM_ALLOCATION(__this, new_window(args[0], args[1], args[2]), delete_window, "window");
    INIT_CUSTOM_ALLOCATION(__this, new_object_vector(), delete_object_vector, "elements");

    if (allow_event(get_attr(__this, "window")->as.ptr, EV_CLOSE))
        return (VALUE_ERR);

    return (VALUE_OK);
}

static cn_value _add_element(Object *__this, void **args)
{
    if (!args || !args[0])
        return (VALUE_ERR);

    ObjectVector *vec = get_attr(__this, "elements")->as.ptr;

    if (insert_object_vector(vec, args[0]))
        return (VALUE_ERR);
    return (VALUE_OK);
}

static cn_value _events(Object *__this, void **args)
{
    (void)args;

    Window *w = get_attr(__this, "window")->as.ptr;
    ObjectVector *elements = get_attr(__this, "elements")->as.ptr;

    for (size_t i = 0; i < elements->size; ++i)
        (void)call_method(elements->objects[i], "event", ((cnany []){(cnany)w, NULL}));

    return (null_value);
}

static cn_value _update(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    ObjectVector *elements = get_attr(__this, "elements")->as.ptr;

    for (size_t i = 0; i < elements->size; ++i)
        (void)call_method(elements->objects[i], "update", ((cnany []){(cnany)args[0], NULL}));

    return (null_value);
}

static cn_value _draw(Object *__this, void **args)
{
    (void)args;

    Window *w = get_attr(__this, "window")->as.ptr;
    ObjectVector *elements = get_attr(__this, "elements")->as.ptr;

    (void)clear_window(w, 0x0000ffff);

    for (size_t i = 0; i < elements->size; ++i)
        (void)call_method(elements->objects[i], "draw", ((cnany []){(cnany)w, NULL}));

    return (null_value);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;

    PREP_DEL()
    DEL_CUSTOM_ALLOCAION(__this, delete_window, "window");
    DEL_CUSTOM_ALLOCAION(__this, delete_object_vector, "elements");

    return (null_value);
}

CN_API Object *new_interface(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_draw", &_draw);
    CREATE_METHOD_CLASS_BUILD(obj, "_update", &_update);
    CREATE_METHOD_CLASS_BUILD(obj, "_events", &_events);
    CREATE_METHOD_CLASS_BUILD(obj, "add_element", &_add_element);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    return (obj);
}