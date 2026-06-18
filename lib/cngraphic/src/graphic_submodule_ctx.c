#include "libcncore.h"
#include "libcngraphic.h"

static cn_value _draw(Object *__this, void **args)
{
    (void)args;

    ObjectVector *interfaces = get_attr(__this, "interfaces")->as.ptr;
    // WindowUniverse *wu = get_attr(__this, "all_window")->as.ptr;

    // (void)draw_all_window(wu); this is now handled by the interfaces to respect draw order

    for (size_t i = 0; i < interfaces->size; ++i)
        (void)call_method(interfaces->objects[i], "_draw", NULL);

    return (null_value);
}

static cn_value _update(Object *__this, void **args)
{
    (void)args;

    ObjectVector *interfaces = get_attr(__this, "interfaces")->as.ptr;
    WindowUniverse *wu = get_attr(__this, "all_window")->as.ptr;
    int64_t main_window = get_attr(__this, "_main_window_id")->as.i;
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
            Texture *temp_texture;

            for (struct list_iterator_s it = list_get_iterator(elements); !list_iterator_isend(&it); list_iterator_next(&it)) {
                if (list_iterator_value_isnull(&it))
                    continue;

                if (!has_attr(it.val.as.ptr, "texture"))
                    continue;

                temp_texture = get_attr(it.val.as.ptr, "texture")->as.ptr;
                
                switch (temp_texture->api) {
                    case R_API_SDL:
                        if (temp_texture->gpu_handler.sdl_texture.renderer == temp->renderer) {
                            INVALIDATE_GPU(temp_texture);
                        }
                        break;
                    case R_API_GL:
                        if (temp_texture->gpu_handler.gl_texture.gl_ctx == temp->gl_ctx) {
                            INVALIDATE_GPU(temp_texture);
                        }
                        break;
                    default:
                        break;
                }
            }
            if (main_window != -1 && temp->id == main_window) {
                (void)call_method(__this, "_stop", NULL);
                set_attr(__this, "_main_window_id", CN_TYPE_INT, (int64_t []){-1});
            }
            (void)remove_object_vector(interfaces, i);
            --i;
            continue;
        }

        (void)call_method(interfaces->objects[i], "_update", PACK_ARG(&get_attr(__this, "dt")->as.f));
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

static cn_value _set_main_window(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't set main window without window id.");
        return (VALUE_ERR);
    }

    ObjectVector *interfaces = get_attr(__this, "interfaces")->as.ptr;
    cnbool found = false;

    for (size_t i = 0; i < interfaces->size; ++i) {
        if (((Window *)(get_attr(interfaces->objects[i], "window")->as.ptr))->id != (int64_t)*(int32_t *)args[0])
            continue;
        found = true;
        break;
    }

    if (!found) {
        RAISE(ERR_OUT_OF_BOUND, "can't set main window from an id that don't match any window in universe.");
        return (VALUE_ERR);
    }

    INIT_INT(__this, (int64_t)*(int32_t *)args[0], "_main_window_id");

    return (VALUE_OK);
}

static cn_value _spawn_interface(Object *__this, void **args)
{
    if (!args) {
        RAISE(ERR_INVALID_POINTER, "missing arguments required to create an interface.");
        return (null_value);
    }

    Object *interface = build_object(new_interface(), (void *[]){args[0], args[1], args[2], NULL});
    ObjectVector *vec = get_attr(__this, "interfaces")->as.ptr;

    if (!interface) {
        PROPAGATE_ERR();
        return (null_value);
    }

    if (insert_object_vector(vec, interface)) {
        PROPAGATE_ERR();
        (void)delete_object(interface);
        return (null_value);
    }

    if (add_window_in_universe(get_attr(__this, "all_window")->as.ptr, get_attr(interface, "window")->as.ptr)) {        
        PROPAGATE_ERR();
        (void)remove_object_vector(vec, vec->size - 1);
        return (null_value);
    }

    return ((cn_value){.type=CN_TYPE_WEAK_OBJECT, .as.ptr=interface});
}

static cn_value _init(Object *__this, void **args)
{
    (void)__this;

    PREP_INIT()

    if (!args || !(args[0])) {
        RAISE(ERR_INVALID_POINTER, "can't init graphic submodules without core ctx as arg.");
        return (VALUE_ERR);
    }
    
    Object *ctx = (Object *)(args[0]);
    
    if (!start_graphics()) {
        PROPAGATE_ERR()
        return (VALUE_ERR);
    }

    INIT_INT(ctx, -1, "_main_window_id");
    INIT_CUSTOM_ALLOCATION(ctx, new_object_vector(), delete_object_vector, "interfaces");
    INIT_CUSTOM_ALLOCATION(ctx, new_window_universe(), delete_window_universe, "all_window");
    INIT_OBJECT_STATIC(ctx, new_atlas(NULL, (expr_free)&delete_texture), NULL,  "texture_atlas");
    INIT_OBJECT_STATIC(ctx, new_atlas(NULL, (expr_free)&delete_mesh), NULL, "mesh_atlas");
    INIT_OBJECT_STATIC(ctx, new_atlas(NULL, (expr_free)&delete_material), NULL, "material_atlas");

    if (call_method(ctx, "register_draw", PACK_ARG(_draw)).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }
    if (call_method(ctx, "register_update", PACK_ARG(_update)).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }
    if (call_method(ctx, "register_event", PACK_ARG(_events)).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }

    INIT_METHOD(ctx, "spawn_interface", _spawn_interface)
    INIT_METHOD(ctx, "set_main_window", _set_main_window)
    
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

    for (struct list_iterator_s it = list_get_iterator(elements); !list_iterator_isend(&it); list_iterator_next(&it)) {
        if (list_iterator_value_isnull(&it))
            continue;

        if (!has_attr(it.val.as.ptr, "texture"))
            continue;

        INVALIDATE_GPU((Texture *)(get_attr(it.val.as.ptr, "texture")->as.ptr));
    }

    DEL_CUSTOM_ALLOCAION(ctx, delete_object_vector, "interfaces");
    DEL_CUSTOM_ALLOCAION(ctx, delete_window_universe, "all_window");

    end_graphics();
    
    return (VALUE_OK);
}

CN_API Object *new_graphic_submodule(void)
{
    Object *obj = new_object();

    if (!obj) {
        PROPAGATE_ERR();
        return (NULL);
    }

    SET_PARENT_CLASS_BUILD_STATIC(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}
