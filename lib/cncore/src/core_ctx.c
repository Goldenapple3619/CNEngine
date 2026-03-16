#include "libcncore.h"

#define VALUE_FALSE (cn_value){CN_TYPE_INT, {false}}
#define VALUE_TRUE (cn_value){CN_TYPE_INT, {true}}

static cn_value _init(Object *__this, void **args)
{
    (void)args;
    PREP_INIT()

    if (!start_core())
        return (VALUE_ERR);

    INIT_STRING(__this, "ctx", "name");
    INIT_INT(__this, 0, "is_running");
    INIT_FLOAT(__this, 0.0, "dt");
    INIT_CUSTOM_ALLOCATION(__this, new_clock(), delete_clock, "clock");

    cn_value *temp_vec_attr = get_attr(__this, "submodules");

    if (!temp_vec_attr)
        return (VALUE_ERR);

    ObjectVector *temp_vec = temp_vec_attr->as.ptr;
    cn_value ret;

    for (size_t i = 0; i < temp_vec->size; ++i) {
        ret = call_method(temp_vec->objects[i], "_init", ((cnany []){(cnany)__this, NULL}));

        if (ret.type == CN_TYPE_NULL || ret.as.i == VALUE_ERR.as.i)
            return (VALUE_ERR);
    }

    return (VALUE_OK);
}

static cn_value _run(Object *__this, void **args)
{
    (void)args;

    cnnumber dt;

    cn_value *ptr_is_running = get_attr(__this, "is_running");
    Clock *c = (Clock *)get_attr(__this, "clock")->as.ptr;

    set_attr(__this, "is_running", CN_TYPE_INT, (cnany)((int64_t [1]){1}));

    while (ptr_is_running->as.i) {
        if (has_method(__this, "events"))
            call_method(__this, "events", NULL);

        if (has_method(__this, "update"))
            call_method(__this, "update", NULL);

        if (has_method(__this, "draw"))
            call_method(__this, "draw", NULL);
        
        dt = clock_tick(c, 60);
        set_attr(__this, "dt", CN_TYPE_FLOAT, (cnany)&dt);
    };

    return (null_value);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;
    PREP_DEL()
    DEL_CUSTOM_ALLOCAION(__this, delete_clock, "clock");

    cn_value *s = get_attr(__this, "submodules");
    
    if (s) {
        ObjectVector *vec = s->as.ptr;

        for (size_t i = 0; i < vec->size; ++i) {
            call_method(vec->objects[i], "_del", ((cnany []){(cnany)__this, NULL}));
        }

        delete_object_vector(vec);
    }

    end_core();

    return (null_value);
}

static cn_value _stop(Object *__this, void **args)
{
    (void)args;

    set_attr(__this, "is_running", CN_TYPE_INT, (cnany)((int64_t [1]){0}));
    return (null_value);
}

CN_API cnbool submodule_ctx(Object *ctx, Object *module)
{
    if (!ctx || !module)
        return (false);
    
    cn_value *s = get_attr(ctx, "submodules");

    if (!s)
        return (false);
    
    ObjectVector *modules = s->as.ptr;

    if (!modules)
        return (false);

    if (insert_object_vector(modules, module))
        return (false);

    return (true);
}

static Object *fill_methods(Object *obj)
{
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_run", &_run);
    CREATE_METHOD_CLASS_BUILD(obj, "_stop", &_stop);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    return (obj);
}

CN_API Object *new_ctx()
{
    Object *obj = new_object();
    ObjectVector *temp_vec;

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());

    temp_vec = new_object_vector();
    if (!temp_vec) {
        (void)delete_object(obj);
        return (NULL);
    }
    if (!set_attr(obj, "submodules", CN_TYPE_GENERIC_UNIQ_PTR, (cnany)temp_vec)) {
        (void)delete_object_vector(temp_vec);
        (void)delete_object(obj);
        return (NULL);
    }

    return (fill_methods(obj));
}
