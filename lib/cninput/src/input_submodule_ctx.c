#include "libcninput.h"

#include "libcncore.h"
#include "libcngraphic.h"

static cn_value _events(Object *__this, void **args)
{
    (void)args;

    WindowUniverse *wu = get_attr(__this, "all_window")->as.ptr;
    int64_t main_window = get_attr(__this, "_main_window_id")->as.i;
    Window *temp = get_window_in_universe(wu, main_window);
    Object *atlas = get_attr(__this, "inputs")->as.ptr;
    const struct event_map_entry_s *ev_map;
    const InputEntry *entry;

    if (!temp)
        return (null_value);

    cn_event allowed_events[] = {
        EV_KEYDOWN,
        EV_KEYUP
    };

    for (size_t i = 0; i < sizeof(allowed_events) / sizeof(cn_event); ++i) {
        ev_map = get_event_window(temp, allowed_events[i]);

        if (!ev_map || !ev_map->size)
            continue;

        for (size_t j = 0; j < ev_map->size; ++j) {
            for (struct list_iterator_s it = atlas_get_iterator(atlas, true); !atlas_iterator_isend(&it); atlas_iterator_next(&it)) {
                entry = it.val.as.ptr;

                if (!input_entry_cmp(entry, ev_map->events[j]))
                    continue;

                (void)input_entry_activate(entry, ev_map->events[j]);
            }
        }
    }

    return (null_value);
}

static cn_value _register_input_callback(Object *__this, void **args)
{
    if (!args || !args[0] || !args[1])
        return (VALUE_ERR);

    Object *atlas = get_attr(__this, "inputs")->as.ptr;
    InputEntry *ie;
    cn_value temp = call_method(atlas, "at", (cnany []){args[0]});

    if (temp.type == CN_TYPE_NULL) {
        ie = new_input_entry(args[0]);

        if (!ie)
            return (VALUE_ERR);

        if (call_method(atlas, "push", (cnany []){ie, args[0]}).as.i == VALUE_ERR.as.i)
            return (VALUE_ERR);
    } else {
        ie = temp.as.ptr;
    }

    if (input_entry_add_callback(ie, ((ObjMethodPair *)args[1])->method, ((ObjMethodPair *)args[1])->obj))
        return (VALUE_ERR);

    return (VALUE_OK);
}

static cn_value _register_input_controller(Object *__this, void **args)
{
    if (!args || !args[0] || !args[1])
        return (VALUE_ERR);

    Object *atlas = get_attr(__this, "inputs")->as.ptr;
    InputEntry *ie;
    cn_value temp = call_method(atlas, "at", (cnany []){args[0]});

    if (temp.type == CN_TYPE_NULL) {
        ie = new_input_entry(args[0]);

        if (!ie)
            return (VALUE_ERR);

        if (call_method(atlas, "push", (cnany []){ie, args[0]}).as.i == VALUE_ERR.as.i)
            return (VALUE_ERR);
    } else {
        ie = temp.as.ptr;
    }

    if (input_entry_add_controller(ie, ((InputController *)args[1])->target_type, ((InputController *)args[1])->target_value, ((InputController *)args[1])->ignore_value))
        return (VALUE_ERR);

    return (VALUE_OK);
}

static cn_value _unregister_input_callback(Object *__this, void **args)
{
    if (!args || !args[0] || !args[1])
        return (null_value);

    Object *atlas = get_attr(__this, "inputs")->as.ptr;
    InputEntry *ie;
    cn_value temp = call_method(atlas, "at", (cnany []){args[0]});

    if (temp.type == CN_TYPE_NULL) {
        return (null_value);
    } else {
        ie = temp.as.ptr;
    }

    for (size_t i = 0; i < ie->cbs_size; ++i) {
        if (ie->cbs[i]->method == ((ObjMethodPair *)args[1])->method && ie->cbs[i]->obj == ((ObjMethodPair *)args[1])->obj)
            input_entry_remove_callback(ie, i);
    }

    return (null_value);
}

static cn_value _init(Object *__this, void **args)
{
    (void)__this;

    PREP_INIT()

    if (!args || !(args[0]))
        return (VALUE_ERR);
    
    Object *ctx = (Object *)(args[0]);
    
    if (!start_input())
        return (VALUE_ERR);

    if (!has_attr(ctx, "_main_window_id") || !has_attr(ctx, "all_window"))
        return (VALUE_ERR);

    INIT_OBJECT_STATIC(ctx, new_atlas(NULL, (void (*)(void *))(&delete_input_entry)), NULL, "inputs");

    if (call_method(ctx, "register_event", (cnany []){&_events, NULL}).as.i == VALUE_ERR.as.i)
        return (VALUE_ERR);

    INIT_METHOD(ctx, "register_input_controller", _register_input_controller);
    INIT_METHOD(ctx, "register_input_callback", _register_input_callback);
    INIT_METHOD(ctx, "unregister_input_callback", _unregister_input_callback);
    
    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)__this;

    if (!args || !(args[0]))
        return (VALUE_ERR);
    
    Object *ctx = (Object *)(args[0]);

    (void)ctx;

    end_input();
    
    return (VALUE_OK);
}

CN_API Object *new_input_submodule(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}
