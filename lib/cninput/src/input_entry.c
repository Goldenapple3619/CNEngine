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
        for (size_t i = 0; i < ie->cbs_size; ++i) {
            (void)release_object(ie->cbs[i]->obj);
            (void)free(ie->cbs[i]);
        }
        (void)free(ie->cbs);
    }
    if (ie->name)
        (void)free(ie->name);
    (void)free(ie);
}
