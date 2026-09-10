#include "libcngraphic.h"

#include <SDL2/SDL.h>
// #ifdef __APPLE__
//     #define GL_SILENCE_DEPRECATION
//     #include <OpenGL/gl.h>
// #else
//     #include <GL/gl.h>
// #endif
#include <glad/gl.h>
#include <string.h>

CN_API Window *new_window(const char *name, const Texture *icon, const Videomode *video_mode)
{
    if (!name || !video_mode) {
        RAISE(ERR_INVALID_POINTER, "can't create window with no name/video_mode.");
        return (NULL);
    }

    Window *window = (Window *)malloc(sizeof(Window));
    cnflags renderer_flags = 0;

    if (!window) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate window.");
        return (NULL);
    }
    
    window->title = strdup(name);

    if (!window->title) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate string.");
        (void)free(window);
        return (NULL);
    }

    (void)memcpy(&window->video_mode, video_mode, sizeof(Videomode));
    
    window->window = SDL_CreateWindow(window->title,
        window->video_mode.position.x, window->video_mode.position.y,
        window->video_mode.size.x, window->video_mode.size.y,
        window->video_mode.native_flags);

    if (!window->window) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate system window.");
        (void)free(window->title);
        (void)free(window);
        return (NULL);
    }

    if ((window->video_mode.flags & VDM_ACCELERATION) > 0)
        renderer_flags |= SDL_RENDERER_ACCELERATED;
    if ((window->video_mode.flags & VDM_VSYNC) > 0)
        renderer_flags |= SDL_RENDERER_PRESENTVSYNC;

    if ((video_mode->flags & VDM_GPU) > 0) {
        window->renderer = SDL_CreateRenderer(window->window, -1, renderer_flags);

        if (!window->renderer) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new renderer.");
            (void)free(window->title);
            (void)SDL_DestroyWindowSurface(window->window);
            (void)SDL_DestroyWindow(window->window);
            (void)free(window);
            return (NULL);
        }
    } else {
        window->renderer = NULL;
    }

    if ((video_mode->flags & VDM_OPENGL) > 0) {
        window->gl_ctx = SDL_GL_CreateContext(window->window);

        if (!window->gl_ctx) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new glContext.");
            if (window->renderer)
                (void)SDL_DestroyRenderer(window->renderer);
            (void)free(window->title);
            (void)SDL_DestroyWindowSurface(window->window);
            (void)SDL_DestroyWindow(window->window);
            (void)free(window);
            return (NULL);
        }

        if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to load glad into glContext.");
            (void)SDL_GL_DeleteContext(window->gl_ctx);
            if (window->renderer)
                (void)SDL_DestroyRenderer(window->renderer);
            (void)free(window->title);
            (void)SDL_DestroyWindowSurface(window->window);
            (void)SDL_DestroyWindow(window->window);
            (void)free(window);
            return (NULL);
        }

        if ((video_mode->flags & VDM_VSYNC) > 0) {
            SDL_GL_SetSwapInterval(1);
        }
    } else {
        window->gl_ctx = NULL;
    }

    if (((video_mode->flags & VDM_CPU) > 0)) {
        window->texture = new_texture_from_surface(SDL_GetWindowSurface(window->window));

        if (!window->texture) {
            PROPAGATE_ERR();
            if (window->renderer)
                (void)SDL_DestroyRenderer(window->renderer);
            if (window->gl_ctx)
                (void)SDL_GL_DeleteContext(window->gl_ctx);
            (void)free(window->title);
            (void)SDL_DestroyWindowSurface(window->window);
            (void)SDL_DestroyWindow(window->window);
            (void)free(window);
            return (NULL);
        }
    } else {
        window->texture = NULL;
    }

    if (icon)
        SDL_SetWindowIcon(window->window, icon->surface);

    window->id = SDL_GetWindowID(window->window);
    window->event_map = NULL;

    return (window);
}

CN_API uint8_t allow_event(Window *window, cn_event ev)
{
    size_t base_size;

    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't allow event on empty window.");
        return (1);
    }
    if (!window->event_map) {
        base_size = 0;
    } else {
        for (base_size = 0; window->event_map[base_size]; ++base_size) {
            if (window->event_map[base_size]->type == ev)
                return (0);
        }
    }

    window->event_map = realloc(window->event_map, base_size == 0 ? (2 * sizeof(struct event_map_entry_s *)) : ((base_size + 2) * sizeof(struct event_map_entry_s *)));
    
    if (!window->event_map) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to resize window event_map (%zu -> %zu).", base_size, (base_size + 2));
        return (1);
    }

    window->event_map[base_size] = new_event_map(ev);

    if (!window->event_map[base_size]) {
        PROPAGATE_ERR();
        return (1);
    }
    window->event_map[base_size + 1] = NULL;

    return (0);
}

CN_API void set_vsync_window(Window *window, cnbool value)
{
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't set vsync on empty window.");
        return;
    }
    if (value)
        window->video_mode.flags |= VDM_VSYNC;
    else
        window->video_mode.flags ^= VDM_VSYNC;
}

CN_API cnbool get_vsync_window(const Window *window)
{
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't get vsync on empty window.");
        return (false);
    }
    return ((window->video_mode.flags & VDM_VSYNC) > 0);
}

CN_API void set_inactive_window(Window *window, cnbool value)
{
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't set inactive on empty window.");
        return;
    }
    if (value)
        window->video_mode.flags |= VDM_INACTIVE;
    else
        window->video_mode.flags ^= VDM_INACTIVE;
}

CN_API cnbool get_inactive_window(const Window *window)
{
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't get inactive on empty window.");
        return (false);
    }
    return ((window->video_mode.flags & VDM_INACTIVE) > 0);
}

CN_API void set_closable_window(Window *window, cnbool value)
{
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't set closable on empty window.");
        return;
    }
    if (value)
        window->video_mode.flags |= VDM_CLOSABLE;
    else
        window->video_mode.flags ^= VDM_CLOSABLE;
}

CN_API cnbool get_closable_window(const Window *window)
{
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't get closable on empty window.");
        return (false);
    }
    return ((window->video_mode.flags & VDM_CLOSABLE) > 0);
}

CN_API void set_hidden_window(Window *window, cnbool value)
{
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't set hidden on empty window.");
        return;
    }
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
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't get hidden on empty window.");
        return (false);
    }
    return ((window->video_mode.native_flags & VDM_N_HDN) > 0);
}

static void _update_window_quit(Window *window)
{
    if (!get_closable_window(window))
        return;

    if (!has_event_window(window, EV_CLOSE))
        return;

    (void)set_hidden_window(window, true);
    (void)set_inactive_window(window, true);
}

static void _update_window_resize(Window *window)
{
    if (!has_event_window(window, EV_RESIZE))
        return;

    const struct event_map_entry_s *events = get_event_window(window, EV_RESIZE);

    window->video_mode.size.x = events->events[events->size - 1]->x;
    window->video_mode.size.y = events->events[events->size - 1]->y;

    if ((window->video_mode.flags & VDM_CPU) > 0) {
        SDL_UpdateWindowSurface(window->window);

        window->texture->surface = SDL_GetWindowSurface(window->window);
        window->texture->size.x = window->texture->surface->w;
        window->texture->size.y = window->texture->surface->h;
    }
}

static void _update_window_move(Window *window)
{
    if (!has_event_window(window, EV_MOVE))
        return;
    const struct event_map_entry_s *events = get_event_window(window, EV_MOVE);

    window->video_mode.position.x = events->events[events->size - 1]->x;
    window->video_mode.position.y = events->events[events->size - 1]->y;
}

CN_API void update_window(Window *window)
{
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't update on empty window.");
        return;
    }

    (void)_update_window_move(window);
    (void)_update_window_resize(window);
    (void)_update_window_quit(window);
}

CN_API uint8_t push_event_window(Window *window, cn_event type, cnnumber x, cnnumber y, int64_t v)
{
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't push event on empty window.");
        return (1);
    }

    if (!window->event_map)
        return (0);

    for (size_t i = 0; window->event_map[i]; ++i) {
        if (window->event_map[i]->type != type)
            continue;
        return (push_event_in_map(window->event_map[i], x, y, v));
    }
    return (0);
}

CN_API void draw_window(Window *window)
{
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't draw an empty window.");
        return;
    }
    if ((window->video_mode.flags & VDM_GPU) > 0)
        (void)SDL_RenderPresent(window->renderer);
    else if ((window->video_mode.flags & VDM_CPU) > 0)
        (void)SDL_UpdateWindowSurface(window->window);
    else if ((window->video_mode.flags & VDM_OPENGL) > 0) {
        (void)SDL_GL_SwapWindow(window->window);
    } 
}

CN_API cnbool has_event_window(const Window *window, cn_event type)
{
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't check events on empty window.");
        return (false);
    }
    if (!window->event_map)
        return (false);
    for (size_t i = 0; window->event_map[i]; ++i) {
        if (window->event_map[i]->type != type)
            continue;
        if (window->event_map[i]->size > 0)
            return (true);
    }
    return (false);
}

const struct event_map_entry_s *get_event_window(const Window *window, cn_event type) {
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't get events on empty window.");
        return (NULL);
    }
    if (!window->event_map)
        return (NULL);
    for (size_t i = 0; window->event_map[i]; ++i) {
        if (window->event_map[i]->type != type)
            continue;
        if (window->event_map[i]->size > 0)
            return (window->event_map[i]);
    }
    return (NULL);
}

CN_API void clear_window(Window *window, cncolor color)
{
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't clear an empty window.");
        return;
    }
    if ((window->video_mode.flags & VDM_GPU) > 0) {
        (void)SDL_SetRenderDrawColor(window->renderer, (color & 0xff000000) >> 24,
            (color & 0x00ff0000) >> 16,
            (color & 0x0000ff00) >> 8,
            (color & 0x000000ff));
        (void)SDL_RenderClear(window->renderer);
    } else if ((window->video_mode.flags & VDM_CPU) > 0) {
        (void)clear_texture(window->texture, color);
    } else if  ((window->video_mode.flags & VDM_OPENGL) > 0) {
        (void)SDL_GL_MakeCurrent(window->window, window->gl_ctx);

        (void)glClearColor(((color & 0xff000000) >> 24) / 255.0f,
            ((color & 0x00ff0000) >> 16) / 255.0f,
            ((color & 0x0000ff00) >> 8) / 255.0f,
            (color & 0x000000ff) / 255.0f);
        (void)glEnable(GL_DEPTH_TEST);
        // (void)glDisable(GL_CULL_FACE);
        (void)glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

CN_API void delete_window(Window *window)
{
    if (!window) {
        RAISE(ERR_INVALID_POINTER, "can't delete an empty window.");
        return;
    }

    if (window->title) {
        (void)free(window->title);
        window->title = NULL;
    }
    if (window->gl_ctx) {
        (void)SDL_GL_DeleteContext(window->gl_ctx);
        window->gl_ctx = NULL;
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
    if (window->event_map) {
        for (size_t i = 0; window->event_map[i]; ++i)
            (void)delete_event_map(window->event_map[i]);
        (void)free(window->event_map);
        window->event_map = NULL;
    }
    (void)free(window);
}
