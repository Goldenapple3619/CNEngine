#include "libcncore.h"
#include "libcngraphic.h"

static cn_value _draw(Object *__this, void **args)
{
    (void)args;

    ObjectVector *interfaces = get_attr(__this, "interfaces")->as.ptr;
    WindowUniverse *wu = get_attr(__this, "all_window")->as.ptr;

    (void)draw_all_window(wu);

    for (size_t i = 0; i < interfaces->size; ++i)
        (void)call_method(interfaces->objects[i], "_draw", NULL);

    return (null_value);
}

static cn_value _update(Object *__this, void **args)
{
    (void)args;

    ObjectVector *interfaces = get_attr(__this, "interfaces")->as.ptr;
    WindowUniverse *wu = get_attr(__this, "all_window")->as.ptr;
    Window *temp;

    if (are_all_window_closed(wu)) {
        (void)call_method(__this, "_stop", NULL);
    }

    (void)update_all_window(wu);


    for (size_t i = 0; i < interfaces->size; ++i) {
        temp = get_attr(interfaces->objects[i], "window")->as.ptr;

        if (is_window_closed_addr(wu, temp)) {
            (void)remove_object_vector(interfaces, i);
            --i;
            continue;
        }

        (void)call_method(interfaces->objects[i], "_update", args);
    }

    return (null_value);
}

static cn_value _events(Object *__this, void **args)
{
    (void)args;

    ObjectVector *interfaces = get_attr(__this, "interfaces")->as.ptr;
    WindowUniverse *wu = get_attr(__this, "all_window")->as.ptr;

    (void)clear_events_all_window(wu);
    (void)fetch_events_all_window(wu);

    for (size_t i = 0; i < interfaces->size; ++i)
        (void)call_method(interfaces->objects[i], "_events", NULL);

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
    INIT_CUSTOM_ALLOCATION(ctx, new_window_universe(), delete_window_universe, "all_window");
    INIT_CUSTOM_ALLOCATION(ctx, new_texture_atlas(), delete_texture_atlas, "texture_atlas");

    Object *temp = new_interface();

    if (!temp || call_method(temp, "_init", (cnany []){(cnany)"test", NULL, &v, NULL}).as.i == VALUE_ERR.as.i) {
        (void)delete_object(temp);
        return (VALUE_ERR);
    }
    insert_object_vector(get_attr(ctx, "interfaces")->as.ptr, temp);
    add_window_in_universe(get_attr(ctx, "all_window")->as.ptr, get_attr(temp, "window")->as.ptr);

    temp = new_interface();

    if (!temp || call_method(temp, "_init", (cnany []){(cnany)"test", NULL, &v, NULL}).as.i == VALUE_ERR.as.i) {
        (void)delete_object(temp);
        return (VALUE_ERR);
    }
    insert_object_vector(get_attr(ctx, "interfaces")->as.ptr, temp);
    add_window_in_universe(get_attr(ctx, "all_window")->as.ptr, get_attr(temp, "window")->as.ptr);

    if (!set_method(ctx, "draw", _draw))
        return (VALUE_ERR);
    if (!set_method(ctx, "update", _update))
        return (VALUE_ERR);
    if (!set_method(ctx, "events", _events))
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
    DEL_CUSTOM_ALLOCAION(ctx, delete_window_universe, "all_window");
    DEL_CUSTOM_ALLOCAION(ctx, delete_texture_atlas, "texture_atlas");

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
