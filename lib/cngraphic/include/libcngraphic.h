#ifndef _LIBCNGRAPGIC_H_
    #define _LIBCNGRAPGIC_H_

    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
    #include "libcncore.h"

    enum VIDEOMODE_FLAGS {
        VDM_VSYNC = (1 << 0),
        VDM_CLOSABLE = (1 << 1),
        VDM_ACCELERATION = (1 << 2),
        VDM_INACTIVE = (1 << 3)
    };

    enum VIDEOMODE_NATIVE_FLAGS {
        VDM_N_FSCRN = SDL_WINDOW_FULLSCREEN,
        VDM_N_OPENGL = SDL_WINDOW_OPENGL,
        VDM_N_SHWN = SDL_WINDOW_SHOWN,
        VDM_N_HDN = SDL_WINDOW_HIDDEN,
        VDM_N_BDLS = SDL_WINDOW_BORDERLESS,
        VDM_N_RSZL = SDL_WINDOW_RESIZABLE,
        VDM_N_MNZ = SDL_WINDOW_MINIMIZED,
        VDM_N_MXZ = SDL_WINDOW_MAXIMIZED,
        VDM_N_MSGBD = SDL_WINDOW_MOUSE_GRABBED,
        VMD_N_INPFOC = SDL_WINDOW_INPUT_FOCUS,
        VMD_N_MSFOC = SDL_WINDOW_MOUSE_FOCUS,
        VDM_N_FSCRND = SDL_WINDOW_FULLSCREEN_DESKTOP,
        VDM_N_MSCAPT = SDL_WINDOW_MOUSE_CAPTURE,
        VDM_N_ALSONTOP = SDL_WINDOW_ALWAYS_ON_TOP,
        VDM_N_SKPTB = SDL_WINDOW_SKIP_TASKBAR,
        VDM_N_WUTIL = SDL_WINDOW_UTILITY,
        VDM_N_WTLTP = SDL_WINDOW_TOOLTIP,
        VDM_N_WPOPMEN = SDL_WINDOW_POPUP_MENU,
        VDM_N_KBGD = SDL_WINDOW_KEYBOARD_GRABBED,
        VDM_N_VULKAN = SDL_WINDOW_VULKAN,
        VDM_N_METAL = SDL_WINDOW_METAL
    };

    struct texture_s {
        Vector2 size;

        SDL_Surface *surface;
    };

    struct videomode_s {
        Vector2 size;
        Vector2 position;

        cnflags flags;
        cnflags native_flags;
    };

    struct event_s {
        cnnumber x;
        cnnumber y;
        uint32_t type;
        uint64_t v;
    };

    struct event_map_entry_s {
        struct event_s **events;
        uint32_t type;

        size_t size;
        size_t capacity;
    };

    struct window_s {
        char *title;
        struct videomode_s video_mode;

        uint32_t id;

        SDL_Window *window;
        SDL_Renderer *renderer;
        struct texture_s *texture;

        const struct event_map_entry_s **event_map; // null terminated
    };

    struct window_universe_s {
        struct window_s **windows;

        size_t size;
        size_t capacity;
    };

    struct interface_s {
        struct window_s *window;

        struct object_vector_s elements;

        void (*draw)(struct object_s *__this);
        void (*update)(struct object_s *__this, cntime delta_time);
        void (*event)(struct object_s *__this);
    };

    typedef struct texture_s Texture;
    typedef struct window_s Window;
    typedef struct window_universe_s WindowUniverse;
    typedef struct videomode_s Videomode;
    typedef struct event_s Event;
    typedef struct interface_s Interface;
    typedef uint32_t cncolor;

    CN_API void blit(const Texture *__src, Texture *__dst, const Rect *__src_rect, const Vector2 *__dest_at);
    CN_API void blit_ratio(const Texture *__src, Texture *__dst, const Rect *__src_rect, const Vector2 *__dest_at, const Vector2 *__ratios);

    CN_API Texture *new_texture(const Vector2 *size, cnbool alpha);
    CN_API Texture *copy_texture(Texture *texture);
    CN_API Texture *new_texture_from_file(const char *path);
    CN_API Texture *new_texture_from_surface(SDL_Surface *surface);
    CN_API void clear_texture(Texture *texture, cncolor color);
    CN_API void set_opacity_texture(Texture *texture, uint8_t opacity);
    CN_API uint8_t get_opacity_texture(const Texture *texture);
    CN_API void draw_rect(Texture *texture, const Rect *rect, cncolor color);
    // CN_API void draw_ellipse(Texture *texture, const Rect *rect, cncolor color); // to implement
    CN_API void delete_texture(Texture *texture);

    CN_API Window *new_window(const char *name, const Texture *icon, const Videomode *video_mode);
    CN_API void set_vsync_window(Window *window, cnbool value);
    CN_API cnbool get_vsync_window(const Window *window);
    CN_API void set_closable_window(Window *window, cnbool value);
    CN_API cnbool get_closable_window(const Window *window);
    CN_API void set_hidden_window(Window *window, cnbool value);
    CN_API cnbool get_hidden_window(const Window *window);
    CN_API void set_inactive_window(Window *window, cnbool value);
    CN_API cnbool get_inactive_window(const Window *window);
    CN_API void update_window(Window *window);
    CN_API void draw_window(Window *window);
    CN_API cnbool has_event_window(const Window *window, uint32_t type);
    CN_API const struct event_map_entry_s *get_event_window(const Window *window, uint32_t type);
    CN_API void clear_window(Window *window, cncolor color);
    CN_API void delete_window(Window *window);

    CN_API WindowUniverse *new_window_universe(void);
    CN_API cnbool are_all_window_closed(const WindowUniverse *universe);
    CN_API cnbool is_window_closed(const WindowUniverse *universe, uint32_t window_id);
    CN_API cnbool is_window_closed_addr(const WindowUniverse *universe, void *p);
    CN_API Window *get_window_in_universe(const WindowUniverse *universe, uint32_t window_id);
    CN_API void clear_events_all_window(WindowUniverse *universe);
    CN_API void fetch_events_all_window(WindowUniverse *universe);
    CN_API void update_all_window(WindowUniverse *universe);
    CN_API void draw_all_window(WindowUniverse *universe);
    CN_API uint8_t add_window_in_universe(WindowUniverse *universe, Window *window);
    CN_API void remove_window_from_universe(WindowUniverse *universe, uint32_t id);
    CN_API uint8_t resize_window_universe(WindowUniverse *universe, size_t new_capacity);
    CN_API void delete_window_universe(WindowUniverse *universe);

    CN_API cnbool start_graphics(void);
    CN_API void end_graphics(void);

    CN_API Object *new_graphic_submodule(void);

#endif
