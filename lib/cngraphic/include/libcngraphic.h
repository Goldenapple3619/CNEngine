#ifndef _LIBCNGRAPGIC_H_
    #define _LIBCNGRAPGIC_H_

    #include <SDL2/SDL.h>
    #include "libcncore.h"

    enum VIDEOMODE_FLAGS {
        VDM_RESIZABLE = (1 << 0),
        VDM_VSYNC = (1 << 1),
        VDM_CLOSABLE = (1 << 2),
        VDM_ACCELERATION = (1 << 3),
        VDM_HIDDEN = (1 << 4)
    };

    struct texture_s {
        Vector2 size;

        SDL_Surface *surface;
    };

    struct videomode_s {
        Vector2 size;
        Vector2 position;

        cnflags flags;
    };

    struct window_s {
        char *title;
        struct texture_s *icon;
        struct videomode_s *video_mode;

        uint32_t id;

        SDL_Renderer *renderer;
        struct texture_s *texture;
    };

    struct window_universe_s {
        struct window_s **windows;

        size_t size;
    };

    struct interface_s {
        struct window_s *window;

        struct object_vector_s elements;

        void (*draw)(struct object_s *__this);
        void (*update)(struct object_s *__this, cntime delta_time);
        void (*event)(struct object_s *__this);
    };

    typedef struct texture_s Texture;

    CN_API void blit(Texture *__src, Texture *__dst, Rect *__src_rect, Rect *__dest_rect);
    CN_API void blit_ratio(Texture *__src, Texture *__dst, Rect *__src_rect, Vector2 *__dest_at, Vector2 *__ratios);

    CN_API Texture *new_texture(Vector2 *size, cnbool alpha);
    CN_API Texture *new_texture_from_file(const char *path);
    CN_API Texture *new_texture_from_surface(SDL_Surface *surface);
    CN_API void delete_texture(Texture *texture);

#endif
