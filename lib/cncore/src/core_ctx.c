#include "libcncore.h"

static cn_value _init(Object *__this, void *args)
{
    (void)args;

    set_attr(__this, "name", CN_TYPE_STRING, (cnany)"ctx");
    set_attr(__this, "is_running", CN_TYPE_INT, (cnany)((int64_t [1]){0}));

    set_attr(__this, "clock", CN_TYPE_GENERIC_UNIQ_PTR, (cnany)new_clock());

    return ((cn_value){CN_TYPE_INT, {0}});
}

static cn_value _run(Object *__this, void *args)
{
    (void)args;

    cnnumber dt;

    if (!has_attr(__this, "is_running") || !has_attr(__this, "clock"))
        return (null_value);

    Clock *c = (Clock *)get_attr(__this, "clock")->as.ptr;

    if (!c)
        return (null_value);

    set_attr(__this, "is_running", CN_TYPE_INT, (cnany)((int64_t [1]){1}));

    while (get_attr(__this, "is_running")->as.i) {
        print_object(__this);
        printf("\n");
        
        dt = clock_tick(c, 60);
        set_attr(__this, "dt", CN_TYPE_FLOAT, (cnany)&dt);
    };

    return (null_value);
}

static cn_value _del(Object *__this, void *args)
{
    (void)args;

    cn_value *c = get_attr(__this, "clock");

    if (!c)
        return (null_value);
    delete_clock((Clock *)c->as.ptr);
    return (null_value);
}

static cn_value _stop(Object *__this, void *args)
{
    (void)args;

    set_attr(__this, "is_running", CN_TYPE_INT, (cnany)((int64_t [1]){0}));
    return (null_value);
}

CN_API Object *new_ctx(cn_method callback_init, cn_method callback_run, cn_method callback_del)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);
    obj->base = create_default_object();
    if (!obj->base)
        return (NULL);
    set_method(obj, "_init", callback_init ? callback_init: &_init);
    set_method(obj, "_run", callback_run ? callback_run : &_run);
    set_method(obj, "_stop", &_stop);
    set_method(obj, "_del", callback_del ? callback_del : &_del);
    return (obj);
}
