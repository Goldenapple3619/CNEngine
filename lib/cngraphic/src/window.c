#include "libcngraphic.h"

#include <string.h>

CN_API Window *new_window(const char *name, const Texture *icon, const Videomode *video_mode)
{
    if (!name || !video_mode)
        return (NULL);

    Window *window = (Window *)malloc(sizeof(Window));
    cnflags renderer_flags = 0;

    if (!window)
        return (NULL);
    
    window->title = strdup(name);

    if (!window->title) {
        (void)free(window);
        return (NULL);
    }

    (void)memcpy(&window->video_mode, video_mode, sizeof(Videomode));
    
    window->window = SDL_CreateWindow(window->title,
        window->video_mode.position.x, window->video_mode.position.y,
        window->video_mode.size.x, window->video_mode.size.y,
        window->video_mode.native_flags);

    if (!window->window) {
        (void)free(window->title);
        (void)free(window);
        return (NULL);
    }

    if ((window->video_mode.flags & VDM_ACCELERATION) > 0)
        renderer_flags |= SDL_RENDERER_ACCELERATED;
    if ((window->video_mode.flags & VDM_VSYNC) > 0)
        renderer_flags |= SDL_RENDERER_PRESENTVSYNC;

    window->renderer = SDL_CreateRenderer(window->window, -1, renderer_flags);

    if (!window->renderer) {
        (void)free(window->title);
        (void)SDL_DestroyWindowSurface(window->window);
        (void)SDL_DestroyWindow(window->window);
        (void)free(window);
        return (NULL);
    }

    window->texture = new_texture_from_surface(SDL_GetWindowSurface(window->window));

    if (icon)
        SDL_SetWindowIcon(window->window, icon->surface);

    window->id = SDL_GetWindowID(window->window);
    window->event_map = NULL;

    return (window);
}

CN_API void set_vsync_window(Window *window, cnbool value)
{
    if (!window)
        return;
    if (value)
        window->video_mode.flags |= VDM_VSYNC;
    else
        window->video_mode.flags ^= VDM_VSYNC;
}

CN_API cnbool get_vsync_window(const Window *window)
{
    if (!window)
        return (false);
    return ((window->video_mode.flags & VDM_VSYNC) > 0);
}

CN_API void set_inactive_window(Window *window, cnbool value)
{
    if (!window)
        return;
    if (value)
        window->video_mode.flags |= VDM_INACTIVE;
    else
        window->video_mode.flags ^= VDM_INACTIVE;
}

CN_API cnbool get_inactive_window(const Window *window)
{
    if (!window)
        return (false);
    return ((window->video_mode.flags & VDM_INACTIVE) > 0);
}

CN_API void set_closable_window(Window *window, cnbool value)
{
    if (!window)
        return;
    if (value)
        window->video_mode.flags |= VDM_CLOSABLE;
    else
        window->video_mode.flags ^= VDM_CLOSABLE;
}

CN_API cnbool get_closable_window(const Window *window)
{
    if (!window)
        return (false);
    return ((window->video_mode.flags & VDM_CLOSABLE) > 0);
}

CN_API void set_hidden_window(Window *window, cnbool value)
{
    if (!window)
        return;
    if (value) {
        (void)SDL_HideWindow(window->window);
        window->video_mode.native_flags |= VDM_N_HDN;
        window->video_mode.native_flags ^= VDM_N_SHWN;
    } else {
        (void)SDL_ShowWindow(window->window);
        window->video_mode.native_flags ^= VDM_N_HDN;
        window->video_mode.native_flags |= VDM_N_SHWN;
    }
}

CN_API cnbool get_hidden_window(const Window *window)
{
    if (!window)
        return (false);
    return ((window->video_mode.native_flags & VDM_N_HDN) > 0);
}

static void _update_window_quit(Window *window)
{
    if (!get_closable_window(window))
        return;

    if (!has_event_window(window, SDL_QUIT) && !has_event_window(window, SDL_WINDOWEVENT_CLOSE)) // to implement
        return;

    (void)set_hidden_window(window, true);
    (void)set_inactive_window(window, true);
}

static void _update_window_resize(Window *window)
{
    (void)window; // to implement
}

static void _update_window_move(Window *window)
{
    (void)window; // to implement
}

CN_API void update_window(Window *window)
{
    if (!window)
        return;

    (void)_update_window_move(window);
    (void)_update_window_resize(window);
    (void)_update_window_quit(window);
}

CN_API void draw_window(Window *window)
{
    if (!window)
        return;
    (void)SDL_UpdateWindowSurface(window->window);
}

CN_API cnbool has_event_window(const Window *window, uint32_t type)
{
    if (!window)
        return (false);
    (void)type;
    return (false); // to implement
}

const struct event_map_entry_s *get_event_window(const Window *window, uint32_t type) {
    if (!window)
        return (NULL);
    (void)type;
    return (NULL); // to implement
}

CN_API void clear_window(Window *window, cncolor color)
{
    if (!window)
        return;
    (void)clear_texture(window->texture, color);
}

CN_API void delete_window(Window *window)
{
    if (!window)
        return;
    if (window->title) {
        (void)free(window->title);
        window->title = NULL;
    }
    if (window->renderer) {
        (void)SDL_DestroyRenderer(window->renderer);
        window->renderer = NULL;
    }
    if (window->texture) {
        (void)delete_texture(window->texture);
        window->texture = NULL;
    }
    if (window->window) {
        (void)SDL_DestroyWindow(window->window);
        window->window = NULL;
    }
    (void)free(window);
}
