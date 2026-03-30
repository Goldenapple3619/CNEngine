#ifndef _LIBR2D_H_
    #define _LIBR2D_H_

    #include "libcncore.h"
    #include "libcngraphic.h"

    struct twod_board_mode_s {
        Vector2 position;
        Vector2 resolution;
        Vector2 upscale;

        Object *scene;
    };

    typedef struct {
        Vector2 canva_off;
        Vector2 canva_scale;
        Vector2 canva_ratio;
        SDL_Renderer *renderer;
    } gpu_rendering_data;

    typedef struct {
        Object *obj;

        Window *window;
        Texture *cpu_texture;

        Vector2 canva_position;
        Vector2 canva_size;
        Vector2 canva_scale;
        Vector2 canva_ratio;

        Vector2 offset;
    } twod_render_stack;

    CN_API Object *new_2dboard(void);
    CN_API Object *new_tile(void);

#endif
