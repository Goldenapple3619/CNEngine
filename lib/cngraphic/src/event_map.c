#include "libcngraphic.h"

CN_API struct event_map_entry_s *new_event_map(cn_event type)
{
    struct event_map_entry_s *event_map = (struct event_map_entry_s *)malloc(sizeof(struct event_map_entry_s));

    if (!event_map) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate event map.");
        return (NULL);
    }

    event_map->capacity = 0;
    event_map->size = 0;
    event_map->events = NULL;
    event_map->type = type;
    return (event_map);
}

CN_API uint8_t resize_event_map(struct event_map_entry_s *event_map, size_t new_capacity)
{
    if (!event_map) {
        RAISE(ERR_INVALID_POINTER, "can't resize empty event map.");
        return (1);
    }

    event_map->events = realloc(event_map->events, new_capacity * sizeof(Event *));

    if (!event_map->events) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to resize event map.");
        event_map->capacity = 0;
        event_map->size = 0;
        return (1);
    }

    event_map->capacity = new_capacity;

    return (0);
}

CN_API uint8_t push_event_in_map(struct event_map_entry_s *event_map, cnnumber x, cnnumber y, int64_t v)
{
    if (!event_map) {
        RAISE(ERR_INVALID_POINTER, "can't push in empty event map.");
        return (1);
    }

    if (event_map->size >= event_map->capacity) {
        size_t new_capacity = event_map->capacity == 0 ? 8 : event_map->capacity * 2;
        if (resize_event_map(event_map, new_capacity)) {
            PROPAGATE_ERR();
            return (1);
        }
    }

    event_map->events[event_map->size] = new_event(event_map->type, x, y, v);
    event_map->size++;
    return (0);
}

CN_API void clear_events_in_map(struct event_map_entry_s *event_map)
{
    if (!event_map) {
        RAISE(ERR_INVALID_POINTER, "can't clear empty event map.");
        return;
    }
    
    if (event_map->size <= 0)
        return;

    for (size_t i = 0; i < event_map->size; ++i) {
        (void)delete_event(event_map->events[i]);
        event_map->events[i] = NULL;
    }
    event_map->size = 0;
}

CN_API void delete_event_map(struct event_map_entry_s *event_map)
{
    if (!event_map) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty event map.");
        return;
    }
    for (size_t i = 0; i < event_map->size; ++i) {
        (void)delete_event(event_map->events[i]);
    }
    if (event_map->events) {
        (void)free((void *)event_map->events);
        event_map->events = NULL;
    }
    event_map->capacity = 0;
    event_map->size = 0;
    (void)free((void *)event_map);
}
