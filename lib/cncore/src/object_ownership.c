#include "libcncore.h"

static volatile Object *gc[GC_MAX_SIZE] = {0};
static volatile size_t gc_size = 0;
static volatile cnbool gc_running = false;

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
    if (gc_running)
        return;

    size_t i = 0;

    gc_running = true;

    while (i < gc_size) {
        (void)delete_object((Object *)gc[i]);
        i++;
    }

    gc_size = 0;
    gc_running = false;
}

CN_API void collect_object(Object *object)
{
    if (!object)
        return;

    (void)release_object(object);

    if (object->ref_count > 0)
        return; // your death is set to be later, skipping for now.
    
    if (GC_MAX_SIZE <= gc_size) { // gc full, its time to end all objects on our list, and prepare for next gen of objects.
        if (gc_running) {
            (void)delete_object(object); // gc is about to explode, handle it right away
            return;
        }
        
        (void)run_gc();
    }

    gc[gc_size] = object; // we put it on our list, we will end him later, but its finnaly time for him.
    gc_size++;
}
