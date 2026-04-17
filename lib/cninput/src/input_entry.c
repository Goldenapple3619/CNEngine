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
    (void)ie;
    (void)new_capacity;
    return (0);
}

uint8_t input_entry_resize_controller(InputEntry *ie, size_t new_capacity)
{
    (void)ie;
    (void)new_capacity;
    return (0);
}

uint8_t input_entry_add_callback(InputEntry *ie, cn_method method, Object *obj)
{
    (void)ie;
    (void)method;
    (void)obj;
    return (0);
}

uint8_t input_entry_add_controller(InputEntry *ie, input_type target_type, int64_t target_value, cnbool value_ignored)
{
    (void)ie;
    (void)target_type;
    (void)target_value;
    (void)value_ignored;
    return (0);
}

void input_entry_remove_callback(InputEntry *ie, size_t i)
{
    (void)ie;
    (void)i;
}

void input_entry_remove_controller(InputEntry *ie, size_t i)
{
    (void)ie;
    (void)i;
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
