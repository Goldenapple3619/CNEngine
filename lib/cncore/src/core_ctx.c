#include "libcncore.h"

#define VALUE_FALSE (cn_value){CN_TYPE_INT, {false}}
#define VALUE_TRUE (cn_value){CN_TYPE_INT, {true}}

static cn_value _init(Object *__this, void *args)
{
    (void)args;

    if (!set_attr(__this, "name", CN_TYPE_STRING, (cnany)"ctx"))
        return (VALUE_ERR);
    if (!set_attr(__this, "is_running", CN_TYPE_INT, (cnany)((int64_t [1]){0})))
        return (VALUE_ERR);

    Clock *temp_clock = new_clock();

    if (!temp_clock)
        return (VALUE_ERR);

    if (!set_attr(__this, "clock", CN_TYPE_GENERIC_UNIQ_PTR, (cnany)temp_clock)) {
        (void)delete_clock(temp_clock);
        return (VALUE_ERR);
    }

    cn_value *temp_vec_attr = get_attr(__this, "submodules");

    if (!temp_vec_attr)
        return (VALUE_ERR);

    ObjectVector *temp_vec = temp_vec_attr->as.ptr;
    cn_value ret;

    for (size_t i = 0; i < temp_vec->size; ++i) {
        ret = call_method(temp_vec->objects[i], "_init", (cnany)((cnany *){(cnany)__this}));

        if (ret.type == CN_TYPE_NULL || ret.as.i == VALUE_ERR.as.i)
            return (VALUE_ERR);
    }

    return (VALUE_OK);
}

static cn_value _run(Object *__this, void *args)
{
    (void)args;

    cnnumber dt;

    cn_value *ptr_is_running = get_attr(__this, "is_running");
    Clock *c = (Clock *)get_attr(__this, "clock")->as.ptr;

    set_attr(__this, "is_running", CN_TYPE_INT, (cnany)((int64_t [1]){1}));

    while (ptr_is_running->as.i) {
        if (has_method(__this, "event"))
            call_method(__this, "event", NULL);

        if (has_method(__this, "update"))
            call_method(__this, "update", NULL);

        if (has_method(__this, "draw"))
            call_method(__this, "draw", NULL);
        
        dt = clock_tick(c, 60);
        set_attr(__this, "dt", CN_TYPE_FLOAT, (cnany)&dt);
    };

    return (null_value);
}

static cn_value _del(Object *__this, void *args)
{
    (void)args;

    cn_value *c = get_attr(__this, "clock");
    cn_value *s = get_attr(__this, "submodules");

    if (c)
        delete_clock((Clock *)c->as.ptr);
    if (s) {
        ObjectVector *vec = s->as.ptr;

        for (size_t i = 0; i < vec->size; ++i) {
            call_method(vec->objects[i], "_del", (cnany)((cnany *){(cnany)__this}));
        }

        delete_object_vector(vec);
    }
    return (null_value);
}

static cn_value _stop(Object *__this, void *args)
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

CN_API Object *new_ctx()
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);
    obj->base = create_default_object();
    if (!obj->base)
        return (NULL);
    set_attr(obj, "submodules", CN_TYPE_GENERIC_UNIQ_PTR, (cnany)new_object_vector());
    set_method(obj, "_init", &_init);
    set_method(obj, "_run", &_run);
    set_method(obj, "_stop", &_stop);
    set_method(obj, "_del", &_del);
    return (obj);
}
