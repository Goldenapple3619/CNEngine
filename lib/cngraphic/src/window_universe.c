#include "libcngraphic.h"

CN_API WindowUniverse *new_window_universe(void)
{
    WindowUniverse *wu = (WindowUniverse *)malloc(sizeof(WindowUniverse));

    if (!wu)
        return (NULL);
    wu->capacity = 0;
    wu->size = 0;
    wu->windows = NULL;
    return (wu);
}

CN_API cnbool are_all_window_closed(const WindowUniverse *universe)
{
    if (!universe)
        return (true);
    return ((!universe->size) ? true : false);
}

CN_API cnbool is_window_closed(const WindowUniverse *universe, uint32_t window_id)
{
    if (!universe)
        return (true);

    for (size_t i = 0; i < universe->size; ++i) {
        if (universe->windows[i]->id == window_id)
            return (false);
    }
    return (true);
}

CN_API Window *get_window_in_universe(const WindowUniverse *universe, uint32_t window_id)
{
    if (!universe)
        return (NULL);

    for (size_t i = 0; i < universe->size; ++i) {
        if (universe->windows[i]->id == window_id)
            return (universe->windows[i]);
    }
    return (NULL);
}

CN_API cnbool is_window_closed_addr(const WindowUniverse *universe, void *p)
{
    if (!universe)
        return (true);

    for (size_t i = 0; i < universe->size; ++i) {
        if (universe->windows[i] == p)
            return (false);
    }
    return (true);
}

CN_API void clear_events_all_window(WindowUniverse *universe)
{
    if (!universe)
        return;

    // clear all the events in the window
}

CN_API void fetch_events_all_window(WindowUniverse *universe)
{
    if (!universe)
        return;

    SDL_Event ev;

    while (SDL_PollEvent(&ev)) {
        Window *window = get_window_in_universe(universe, ev.window.windowID);

        if (window) {
            // put the events in the window
        } else {
            for (size_t i = 0; i < universe->size; ++i) {
                window = universe->windows[i];

                // put the events in the window
            }
        }
    }
}

CN_API uint8_t resize_window_universe(WindowUniverse *universe, size_t new_capacity)
{
    if (!universe)
        return (1);

    universe->windows = realloc(universe->windows, new_capacity * sizeof(Window *));

    if (!universe->windows) {
        universe->capacity = 0;
        return (1);
    }

    universe->capacity = new_capacity;

    return (0);
}

CN_API void remove_window_from_universe(WindowUniverse *universe, uint32_t id)
{
    if (!universe || universe->size == 0)
        return;

    size_t last = universe->size - 1;
    size_t i = 0;

    for (; i < universe->size && (universe->windows[i])->id != id; ++i);

    if (i >= universe->size)
        return;

    universe->windows[i]  = universe->windows[last];
    universe->size--;
}

CN_API uint8_t add_window_in_universe(WindowUniverse *universe, Window *window)
{
    if (!universe || !window)
        return (1);

    if (universe->size >= universe->capacity) {
        size_t new_capacity = universe->capacity == 0 ? 8 : universe->capacity * 2;
        if (resize_window_universe(universe, new_capacity))
            return (1);
    }

    universe->windows[universe->size] = window;
    universe->size++;
    return (0);
}

CN_API void update_all_window(WindowUniverse *universe)
{
    if (!universe)
        return;
    for (size_t i = 0; i < universe->size; ++i) {
        (void)update_window(universe->windows[i]);

        if (get_inactive_window(universe->windows[i])) {
            (void)remove_window_from_universe(universe, universe->windows[i]->id);
            --i;
        }
    }
}

CN_API void draw_all_window(WindowUniverse *universe)
{
    if (!universe)
        return;
    for (size_t i = 0; i < universe->size; ++i) {
        (void)draw_window(universe->windows[i]);
    }
}

CN_API void delete_window_universe(WindowUniverse *universe)
{
    if (!universe)
        return;
    if (universe->windows)
        (void)free(universe->windows);
    universe->size = 0;
    universe->capacity = 0;
    universe->windows = NULL;
    (void)free(universe);
}
