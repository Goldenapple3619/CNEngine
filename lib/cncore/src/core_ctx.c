#include "libcncore.h"

static cn_value _update_scene(Object *__this, void **args)
{
    (void)args;

    Object *scene = get_attr(__this, "scene")->as.ptr;

    (void)call_method(scene, "_update", PACK_ARG(&get_attr(__this, "dt")->as.f));
    return (VALUE_NULL);
}

static cn_value _init(Object *__this, void **args)
{
    (void)args;
    PREP_INIT()

    if (!start_core()) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }

    INIT_STRING(__this, "ctx", "name");
    INIT_INT(__this, 0, "is_running");
    INIT_INT(__this, -1, "tps");
    INIT_FLOAT(__this, 0.0, "dt");
    INIT_NUMBER(__this, 1.0, "time_scale");
    INIT_CUSTOM_ALLOCATION(__this, new_clock(), delete_clock, "clock");
    INIT_CUSTOM_ALLOCATION(__this, new_value_vector(), delete_value_vector, "event_pool");
    INIT_CUSTOM_ALLOCATION(__this, new_value_vector(), delete_value_vector, "update_pool");
    INIT_CUSTOM_ALLOCATION(__this, new_value_vector(), delete_value_vector, "draw_pool");
    INIT_OBJECT_STATIC(__this, new_scene(), NULL, "scene");

    if (call_method(__this, "register_update", PACK_ARG(&_update_scene)).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }

    cn_value *temp_vec_attr = get_attr(__this, "submodules");

    if (!temp_vec_attr || !temp_vec_attr->as.ptr) {
        RAISE(ERR_INVALID_POINTER, "can't init core ctx because submodules is somehow not set.");
        return (VALUE_ERR);
    }

    ObjectVector *temp_vec = temp_vec_attr->as.ptr;
    cn_value ret;

    for (size_t i = 0; i < temp_vec->size; ++i) {
        ret = call_method(temp_vec->objects[i], "_init", PACK_ARG((cnany)__this));

        if (ret.type == CN_TYPE_NULL || ret.as.i == VALUE_ERR.as.i) {
            if (has_error()) {
                PROPAGATE_ERR();
            } else {
                RAISE(ERR_RUNTIME, "failed to init a submodule, no more infos.");
            }
            return (VALUE_ERR);
        }
    }

    return (VALUE_OK);
}

static cn_value _run(Object *__this, void **args)
{
    (void)args;

    double dt;
    cn_value *ptr_is_running = get_attr(__this, "is_running");
    cn_value *ptr_tps = get_attr(__this, "tps");
    cn_value *ptr_time_scale = get_attr(__this, "time_scale");
    Clock *c = (Clock *)get_attr(__this, "clock")->as.ptr;
    struct cn_value_vector_s *methods_pools[3] = {
        get_attr(__this, "event_pool")->as.ptr,
        get_attr(__this, "update_pool")->as.ptr,
        get_attr(__this, "draw_pool")->as.ptr
    };
    size_t i = 0;
    size_t j = 0;

    set_attr(__this, "is_running", CN_TYPE_INT, (cnany)((int64_t [1]){1})); // check not required since its set at init

    while (ptr_is_running->as.i) {
        for (j = 0; j < sizeof(methods_pools) / sizeof(struct cn_value_vector_s *); ++j) {
            for (i = 0; i < methods_pools[j]->size; ++i) {
                if (methods_pools[j]->values[i]->type != CN_TYPE_FUNCTION)
                    continue;
                ((cn_method)(methods_pools[j]->values[i]->as.ptr))(__this, NULL);
            }
        }

        dt = clock_tick(c, ptr_tps->as.i) * (double)ptr_time_scale->as.num;
        set_attr(__this, "dt", CN_TYPE_FLOAT, (cnany)&dt); // check not required since its set at init
    };

    return (VALUE_NULL);
}

static cn_value _register_draw(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't register empty draw function.");
        return (VALUE_ERR);
    }

    if (insert_value_vector(get_attr(__this, "draw_pool")->as.ptr, (cn_value){.type = CN_TYPE_FUNCTION, .as.ptr = args[0]}))
        return (VALUE_ERR);
    return (VALUE_OK);
}

static cn_value _register_update(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't register empty update function.");
        return (VALUE_ERR);
    }

    if (insert_value_vector(get_attr(__this, "update_pool")->as.ptr, (cn_value){.type = CN_TYPE_FUNCTION, .as.ptr = args[0]}))
        return (VALUE_ERR);
    return (VALUE_OK);
}

static cn_value _register_event(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't register empty event function.");
        return (VALUE_ERR);
    }

    if (insert_value_vector(get_attr(__this, "event_pool")->as.ptr, (cn_value){.type = CN_TYPE_FUNCTION, .as.ptr = args[0]})) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }
    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;
    PREP_DEL()

    cn_value *s = get_attr(__this, "submodules");
    
    if (s) {
        ObjectVector *vec = s->as.ptr;

        for (size_t i = 0; i < vec->size; ++i) {
            call_method(vec->objects[i], "_del", PACK_ARG((cnany)__this));
        }

        delete_object_vector(vec);
    }

    DEL_CUSTOM_ALLOCAION(__this, delete_clock, "clock");
    DEL_CUSTOM_ALLOCAION(__this, delete_value_vector, "event_pool");
    DEL_CUSTOM_ALLOCAION(__this, delete_value_vector, "update_pool");
    DEL_CUSTOM_ALLOCAION(__this, delete_value_vector, "draw_pool");

    end_core();

    return (VALUE_NULL);
}

static cn_value _stop(Object *__this, void **args)
{
    (void)args;

    if (!set_attr(__this, "is_running", CN_TYPE_INT, (cnany)((int64_t [1]){0}))) {
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }
    return (VALUE_NULL);
}

CN_API cnbool submodule_ctx(Object *ctx, Object *module)
{
    if (!ctx) {
        RAISE(ERR_INVALID_POINTER, "can't add submodules to empty ctx.");
        return (false);
    }

    if (!module) {
        RAISE(ERR_INVALID_POINTER, "can't add empty submodule to ctx.");
        return (false);
    }
    
    cn_value *s = get_attr(ctx, "submodules");

    if (!s) {
        RAISE(ERR_INVALID_POINTER, "submodules vector not found in ctx.");
        return (false);
    }
    
    ObjectVector *modules = s->as.ptr;

    if (!modules) {
        RAISE(ERR_INVALID_POINTER, "submodules vector is empty in ctx.");
        return (false);
    }

    if (insert_object_vector(modules, module)) {
        PROPAGATE_ERR();
        return (false);
    }

    return (true);
}

CN_API Object *new_ctx()
{
    PREP_CLASS_BUILD()

    Object *obj = new_object();

    if (!obj) {
        PROPAGATE_ERR();
        return (NULL);
    }

    SET_PARENT_CLASS_BUILD_STATIC(obj, create_default_object());
    CREATE_CUSTOM_ALLOCATION_CLASS_BUILD(obj, new_object_vector(), delete_object_vector, "submodules");

    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_run", &_run);
    CREATE_METHOD_CLASS_BUILD(obj, "_stop", &_stop);
    CREATE_METHOD_CLASS_BUILD(obj, "register_draw", &_register_draw);
    CREATE_METHOD_CLASS_BUILD(obj, "register_update", &_register_update);
    CREATE_METHOD_CLASS_BUILD(obj, "register_event", &_register_event);
    CREATE_METHOD_CLASS_BUILD(obj, "_stop", &_stop);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    return (obj);
}
