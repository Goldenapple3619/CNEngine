#include "libcncore.h"

static volatile Object *gc[GC_MAX_SIZE] = {0};
static volatile size_t gc_size = 0;

CN_API Object *share_object(Object *object)
{
    if (!object)
        return (NULL);
    object->ref_count++;
    return (object);
}

CN_API Object *release_object(Object *object)
{
    if (!object)
        return (NULL);
    object->ref_count--;
    return (object);
}

CN_API void run_gc(void)
{
    for (size_t i = 0; i < gc_size; ++i) {
        (void)delete_object((Object *)gc[i]);
    }
    gc_size = 0;
}

CN_API void collect_object(Object *object)
{
    if (!object)
        return;

    (void)release_object(object);

    if (object->ref_count > 0)
        return; // your death is set to be later, skipping for now.
    
    if (GC_MAX_SIZE <= gc_size) // gc full, its time to end all objects on our list, and prepare for next gen of objects.
        (void)run_gc();

    gc[gc_size] = object; // we put it on our list, we will end him later, but its finnaly time for him.
    gc_size++;
}
