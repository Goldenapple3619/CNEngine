#include "libcninput.h"

InputEntry *new_input_entry(const char *name)
{
    if (!name)
        return (NULL);

    InputEntry *entry = (InputEntry *)malloc(sizeof(InputEntry));

    if (!entry)
        return (NULL);

    entry->cbs = NULL;
    entry->cbs_capacity = 0;
    entry->cbs_size = 0;
    entry->controllers = NULL;
    entry->controllers_capacity = 0;
    entry->controllers_size = 0;
    entry->name = strdup(name);

    if (!entry->name) {
        (void)free(entry);
        return (NULL);
    }

    return (entry);
}

INPUT_STATE input_state_from_event(cn_event ev)
{
    switch (ev) {
        case EV_KEYDOWN:
            return (INPUT_STATE_ACTIVATE);

        case EV_KEYUP:
            return (INPUT_STATE_STOP);
        
        default:
            return (INPUT_STATE_STOP);
    }
}

void input_entry_activate(const InputEntry *ie, const Event *event)
{
    INPUT_STATE state = input_state_from_event(event->type);

    for (size_t i = 0; i < ie->cbs_size; ++i)
        ie->cbs[i]->method(ie->cbs[i]->obj, (cnany []){(cnany)(&state), (cnany)event});
}

cnbool input_entry_cmp(const InputEntry *ie, const Event *ev)
{
    for (size_t i = 0; i < ie->controllers_size; ++i) {
        if (input_controller_cmp(ie->controllers[i], ev))
            return (true);
    }
    return (false);
}

uint8_t input_entry_resize_callback(InputEntry *ie, size_t new_capacity)
{
    if (!ie)
        return (1);

    ie->cbs = realloc(ie->cbs, new_capacity * sizeof(ObjMethodPair *));

    if (!ie->cbs) {
        ie->cbs_capacity = 0;
        return (1);
    }

    ie->cbs_capacity = new_capacity;
    return (0);
}

uint8_t input_entry_resize_controller(InputEntry *ie, size_t new_capacity)
{
    if (!ie)
        return (1);

    ie->controllers = realloc(ie->controllers, new_capacity * sizeof(InputController *));

    if (!ie->controllers) {
        ie->controllers_capacity = 0;
        return (1);
    }

    ie->controllers_capacity = new_capacity;
    return (0);
}

uint8_t input_entry_add_callback(InputEntry *ie, cn_method method, Object *obj)
{
    if (!ie || !method)
        return (1);

    if (ie->cbs_size >= ie->cbs_capacity) {
        size_t new_capacity = ie->cbs_capacity == 0 ? 8 : ie->cbs_capacity * 2;
        if (input_entry_resize_callback(ie, new_capacity))
            return (1);
    }

    ie->cbs[ie->cbs_size] = new_object_method_pair(obj, method);

    if (!ie->cbs[ie->cbs_size])
        return (1);

    ie->cbs_size++;
    return (0);
}

uint8_t input_entry_add_controller(InputEntry *ie, input_type target_type, int64_t target_value, cnbool value_ignored)
{
    if (!ie)
        return (1);

    if (ie->controllers_size >= ie->controllers_capacity) {
        size_t new_capacity = ie->controllers_capacity == 0 ? 8 : ie->controllers_capacity * 2;
        if (input_entry_resize_controller(ie, new_capacity))
            return (1);
    }

    ie->controllers[ie->controllers_size] = new_input_controller(target_type, target_value, value_ignored);

    if (!ie->controllers[ie->controllers_size])
        return (1);

    ie->controllers_size++;
    return (0);
}

void input_entry_remove_callback(InputEntry *ie, size_t i)
{
    if (!ie || ie->cbs_size == 0 || i >= ie->cbs_size)
        return;

    size_t last = ie->cbs_size - 1;

    (void)delete_object_method_pair(ie->cbs[i]);

    ie->cbs[i]  = ie->cbs[last];
    ie->cbs_size--;
}

void input_entry_remove_controller(InputEntry *ie, size_t i)
{
    if (!ie || ie->controllers_size == 0 || i >= ie->controllers_size)
        return;

    size_t last = ie->controllers_size - 1;

    (void)delete_input_controller(ie->controllers[i]);

    ie->controllers[i]  = ie->controllers[last];
    ie->controllers_size--;
}


void delete_input_entry(InputEntry *ie)
{
    if (!ie)
        return;
    if (ie->controllers) {
        for (size_t i = 0; i < ie->controllers_size; ++i)
            (void)delete_input_controller(ie->controllers[i]);
        (void)free(ie->controllers);
    }
    if (ie->cbs) {
        for (size_t i = 0; i < ie->cbs_size; ++i)
            (void)delete_object_method_pair(ie->cbs[i]);
        (void)free(ie->cbs);
    }
    if (ie->name)
        (void)free(ie->name);
    (void)free(ie);
}
