#ifndef _LIBRGUI_H_
    #define _LIBRGUI_H_

    #include "libcncore.h"
    #include "libcngraphic.h"
    #include <SDL_ttf.h>

    typedef enum {
        FLAG_RGUI_NONE = 0x00,
        FLAG_RGUI_DYNAMIC_RESOLUTION = (1 << 0)
    } cnrgui_flags;

    typedef enum {
        GUI_ALIGN_LEFT = 0x00,
        GUI_ALIGN_MIDDLE,
        GUI_ALIGN_RIGHT
    } cnrgui_alignement;

    typedef enum  {
        GUI_BG_NONE = 0x00,
        GUI_BG_COLOR,
        GUI_BG_IMAGE
    } cnrgui_background_type;

    struct gui_object_mode_s {
        Vector2 position;
        Vector2 scale;
        cnnumber rotation;

        int64_t zindex;

        cnrgui_alignement align;
    };

    struct text_mode_s {
        struct gui_object_mode_s parent_mode;
    
        cncolor color;

        int32_t font_size;
        const char *font_location;
    
        const char *text;
    };

    struct container_mode_s {
        struct gui_object_mode_s parent_mode;
    
        union {
            cncolor background_color;
            const Texture *background_image;
        } background;
        cnrgui_background_type background_type;

        Vector2 size;
        cnbool overflow;
    };

    struct gui_board_mode_s {
        Vector2 position;
        Vector2 resolution;
        Vector2 upscale;

        cnrgui_flags flags;
    };

    typedef struct {
        Object *obj;

        Window *window;
        Texture *cpu_texture;
        Quad *gl_quad;

        Vector2 canva_position;
        Vector2 canva_size;
        Vector2 canva_scale;
        Vector2 canva_ratio;
    } gui_render_stack;

    CN_API cnbool start_gui(void);
    CN_API void end_gui(void);

    CN_API Object *new_guiboard(void);

    CN_API Object *new_guiobject(void);
    CN_API Object *new_text(void);
    CN_API Object *new_container(void);

    CN_API Object *new_gui_submodule(void);
#endif
