#include "libcngraphic.h"

CN_API WindowUniverse *new_window_universe(void)
{
    WindowUniverse *wu = (WindowUniverse *)malloc(sizeof(WindowUniverse));

    if (!wu) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate window universe.");
        return (NULL);
    }
    wu->capacity = 0;
    wu->size = 0;
    wu->windows = NULL;
    return (wu);
}

CN_API cnbool are_all_window_closed(const WindowUniverse *universe)
{
    if (!universe) {
        RAISE(ERR_INVALID_POINTER, "can't check if all window close on empty universe.");
        return (true);
    }
    return (universe->size ? false : true);
}

CN_API cnbool is_window_closed(const WindowUniverse *universe, uint32_t window_id)
{
    if (!universe) {
        RAISE(ERR_INVALID_POINTER, "can't check if window is close on empty universe.");
        return (true);
    }

    for (size_t i = 0; i < universe->size; ++i) {
        if (universe->windows[i]->id == window_id)
            return (false);
    }
    return (true);
}

CN_API Window *get_window_in_universe(const WindowUniverse *universe, uint32_t window_id)
{
    if (!universe) {
        RAISE(ERR_INVALID_POINTER, "can't get window in empty universe.");
        return (NULL);
    }

    for (size_t i = 0; i < universe->size; ++i) {
        if (universe->windows[i]->id == window_id)
            return (universe->windows[i]);
    }

    RAISE_FMT(ERR_OUT_OF_BOUND, "can't get non existent window in universe '%" PRIu32 "'.", window_id);
    return (NULL);
}

CN_API cnbool is_window_closed_addr(const WindowUniverse *universe, void *p)
{
    if (!universe) {
        RAISE(ERR_INVALID_POINTER, "can't check if window is close on empty universe.");
        return (true);
    }

    for (size_t i = 0; i < universe->size; ++i) {
        if (universe->windows[i] == p)
            return (false);
    }
    return (true);
}

CN_API void clear_events_all_window(WindowUniverse *universe)
{
    struct event_map_entry_s **evm;

    if (!universe) {
        RAISE(ERR_INVALID_POINTER, "can't clear events on empty universe.");
        return;
    }

    for (size_t i = 0; i < universe->size; ++i) {
        evm = universe->windows[i]->event_map;

        if (!evm)
            continue;

        while (*evm) {
            (void)clear_events_in_map(*evm);
            evm++;
        }
    }
}

static void sdl_window_ev_to_cnev(const SDL_Event *ev, Event *cnev)
{
    if (!ev || !cnev) {
        RAISE(ERR_INVALID_POINTER, "can't do window sdl event to engine event conversion if src/dest is empty.");
        return;
    }

    switch (ev->window.event) {
        case SDL_WINDOWEVENT_CLOSE:
            *cnev = (Event){0, 0, EV_CLOSE, 0};
            break;

        case SDL_WINDOWEVENT_SIZE_CHANGED:
            *cnev = (Event){ev->window.data1, ev->window.data2, EV_RESIZE, 0};
            break;

        case SDL_WINDOWEVENT_MOVED:
            *cnev = (Event){ev->window.data1, ev->window.data2, EV_MOVE, 0};
            break;

        default:
            *cnev = (Event){0, 0, EV_NULL, 0};
            break;
    }
}

static void sdl_ev_to_cnev(const SDL_Event *ev, Event *cnev)
{
    if (!ev || !cnev) {
        RAISE(ERR_INVALID_POINTER, "can't do sdl event to engine event conversion if src/dest is empty.");
        return;
    }
    switch (ev->type) {
        case SDL_WINDOWEVENT:
            (void)sdl_window_ev_to_cnev(ev, cnev);
            break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
            *cnev = (Event){0, 0, ev->key.type == SDL_KEYUP ? EV_KEYUP : EV_KEYDOWN, ev->key.keysym.sym};
            break;

        case SDL_MOUSEMOTION:
            *cnev = (Event){ev->motion.x, ev->motion.y, EV_MOUSEMOVE, 0};
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            *cnev = (Event){ev->button.x, ev->button.y, ev->button.type == SDL_MOUSEBUTTONUP ? EV_MOUSEUP : EV_MOUSEDOWN, ev->button.button};
            break;

        case SDL_MOUSEWHEEL:
            *cnev = (Event){ev->wheel.x, ev->wheel.y, EV_MOUSEWHEEL, ev->wheel.direction};
            break;

        case SDL_QUIT:
            *cnev = (Event){0, 0, EV_CLOSE, 0};
            break;
        
        default:
            *cnev = (Event){0, 0, EV_NULL, 0};
            break;
    }
}

static uint32_t get_video_id_from_event(const SDL_Event *ev)
{
    switch (ev->type) {
        case SDL_WINDOWEVENT:
            return (ev->window.windowID);
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            return (ev->key.windowID);
        case SDL_MOUSEMOTION:
            return (ev->motion.windowID);
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            return (ev->button.windowID);
        case SDL_MOUSEWHEEL:
            return (ev->wheel.windowID);
        case SDL_TEXTINPUT:
            return (ev->text.windowID);
        default:
            return (0);
    }
}

static cnbool filter_is_repeat_event(const SDL_Event *ev)
{
    switch (ev->type) {
        case SDL_KEYDOWN:
            return (ev->key.repeat == 1);

        default:
            return (false);
    }
}

CN_API void fetch_events_all_window(WindowUniverse *universe)
{
    if (!universe) {
        RAISE(ERR_INVALID_POINTER, "can't fetch events on empty universe.");
        return;
    }

    SDL_Event ev;
    Event cnev;
    uint32_t window_id;
    Window *window;

    while (SDL_PollEvent(&ev)) {
        if (filter_is_repeat_event(&ev))
            continue;

        window_id = get_video_id_from_event(&ev);

        if (window_id) {
            if (is_window_closed(universe, window_id))
                continue;

            window = get_window_in_universe(universe, window_id);

            (void)sdl_ev_to_cnev(&ev, &cnev);
            if (push_event_window(window, cnev.type, cnev.x, cnev.y, cnev.v))
                PROPAGATE_ERR()
        } else {
            for (size_t i = 0; i < universe->size; ++i) {
                window = universe->windows[i];

                (void)sdl_ev_to_cnev(&ev, &cnev);
                if (push_event_window(window, cnev.type, cnev.x, cnev.y, cnev.v))
                    PROPAGATE_ERR()
            }
        }
    }
}

CN_API uint8_t resize_window_universe(WindowUniverse *universe, size_t new_capacity)
{
    if (!universe) {
        RAISE(ERR_INVALID_POINTER, "can't resize empty universe.");
        return (1);
    }

    universe->windows = realloc(universe->windows, new_capacity * sizeof(Window *));

    if (!universe->windows) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to resize universe (%zu -> %zu).", universe->capacity, new_capacity);
        universe->size = 0;
        universe->capacity = 0;
        return (1);
    }

    universe->capacity = new_capacity;

    return (0);
}

CN_API void remove_window_from_universe(WindowUniverse *universe, uint32_t id)
{
    if (!universe) {
        RAISE(ERR_INVALID_POINTER, "can't remove window in empty universe.");
        return;
    }

    if (universe->size == 0)
        return;

    size_t last = universe->size - 1;
    size_t i = 0;

    for (; i < universe->size && (universe->windows[i])->id != id; ++i);

    if (i >= universe->size) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "can't remove non existent window in universe '%" PRIu32 "'.", id);
        return;
    }

    universe->windows[i]  = universe->windows[last];
    universe->size--;
}

CN_API uint8_t add_window_in_universe(WindowUniverse *universe, Window *window)
{
    if (!universe) {
        RAISE(ERR_INVALID_POINTER, "can't add window to empty universe.");
        return (1);
    }

    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't add empty window to universe.");
        return (1);
    }

    if (universe->size >= universe->capacity) {
        size_t new_capacity = universe->capacity == 0 ? 8 : universe->capacity * 2;
        if (resize_window_universe(universe, new_capacity)) {
            PROPAGATE_ERR();
            return (1);
        }
    }

    universe->windows[universe->size] = window;
    universe->size++;
    return (0);
}

CN_API void update_all_window(WindowUniverse *universe)
{
    if (!universe) {
        RAISE(ERR_INVALID_POINTER, "can't update empty universe.");
        return;
    }
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
    // this is now legacy and shouldn't be used

    if (!universe) {
        RAISE(ERR_INVALID_POINTER, "can't draw empty universe.");
        return;
    }
    for (size_t i = 0; i < universe->size; ++i) {
        (void)draw_window(universe->windows[i]);
    }
}

CN_API void delete_window_universe(WindowUniverse *universe)
{
    if (!universe) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty universe.");
        return;
    }
    if (universe->windows)
        (void)free(universe->windows);
    universe->size = 0;
    universe->capacity = 0;
    universe->windows = NULL;
    (void)free(universe);
}
