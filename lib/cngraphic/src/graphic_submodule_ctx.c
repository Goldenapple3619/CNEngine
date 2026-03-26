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
            Object *scene = get_attr(__this, "scene")->as.ptr;
            Object *elements = get_attr(scene, "objects")->as.ptr;
            size_t len = call_method(elements, "len", NULL).as.i;
            cn_value val;

            for (size_t j = 0; j < len; ++j) {
                val = call_method(elements, "at", (cnany []){(size_t []){j}, NULL});

                if (val.type == CN_TYPE_NULL)
                    continue;

                if (!has_attr(val.as.ptr, "texture"))
                    continue;
                
                if (((Texture *)get_attr(val.as.ptr, "texture")->as.ptr)->renderer == ((Window *)(get_attr(interfaces->objects[i], "window")->as.ptr))->renderer) {
                    INVALIDATE_GPU((Texture *)(get_attr(val.as.ptr, "texture")->as.ptr));
                }
            }

            (void)remove_object_vector(interfaces, i);
            --i;
            continue;
        }

        (void)call_method(interfaces->objects[i], "_update", (cnany []){&get_attr(__this, "dt")->as.f, NULL});
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

static cn_value _spawn_interface(Object *__this, void **args)
{
    if (!args)
        return (null_value);

    Object *interface = build_object(new_interface(), (void *[]){args[0], args[1], args[2], NULL});
    ObjectVector *vec = get_attr(__this, "interfaces")->as.ptr;

    if (!interface)
        return (null_value);

    if (insert_object_vector(vec, interface)) {
        (void)delete_object(interface);
        return (null_value);
    }

    if (add_window_in_universe(get_attr(__this, "all_window")->as.ptr, get_attr(interface, "window")->as.ptr)) {        
        (void)remove_object_vector(vec, vec->size - 1);
        return (null_value);
    }

    return ((cn_value){.type=CN_TYPE_OBJECT, .as.ptr=interface});
}

static cn_value _init(Object *__this, void **args)
{
    (void)__this;

    PREP_INIT()

    if (!args || !(args[0]))
        return (VALUE_ERR);
    
    Object *ctx = (Object *)(args[0]);
    
    if (!start_graphics())
        return (VALUE_ERR);

    INIT_CUSTOM_ALLOCATION(ctx, new_object_vector(), delete_object_vector, "interfaces");
    INIT_CUSTOM_ALLOCATION(ctx, new_window_universe(), delete_window_universe, "all_window");
    INIT_CUSTOM_ALLOCATION(ctx, new_texture_atlas(), delete_texture_atlas, "texture_atlas");

    if (call_method(ctx, "register_draw", (cnany []){_draw, NULL}).as.i == VALUE_ERR.as.i)
        return (VALUE_ERR);
    if (call_method(ctx, "register_update", (cnany []){_update, NULL}).as.i == VALUE_ERR.as.i)
        return (VALUE_ERR);
    if (call_method(ctx, "register_event", (cnany []){_events, NULL}).as.i == VALUE_ERR.as.i)
        return (VALUE_ERR);

    INIT_METHOD(ctx, "spawn_interface", _spawn_interface)
    
    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)__this;

    PREP_DEL()

    if (!args || !(args[0]))
        return (VALUE_ERR);
    
    Object *ctx = (Object *)(args[0]);

    Object *scene = get_attr(ctx, "scene")->as.ptr;
    Object *elements = get_attr(scene, "objects")->as.ptr;
    size_t len = call_method(elements, "len", NULL).as.i;
    cn_value val;

    for (size_t i = 0; i < len; ++i) {
        val = call_method(elements, "at", (cnany []){(size_t []){i}, NULL});

        if (val.type == CN_TYPE_NULL)
            continue;

        if (!has_attr(val.as.ptr, "texture"))
            continue;

        INVALIDATE_GPU((Texture *)(get_attr(val.as.ptr, "texture")->as.ptr));
    }

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
