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

    struct text_mode_s {
        Vector2 position;
        cncolor color;
        int32_t size;
        cnrgui_alignement align;
        const char *text;
        const char *font_location;
    };

    struct gui_board_mode_s {
        Vector2 position;
        Vector2 resolution;
        Vector2 upscale;

        cnrgui_flags flags;
    };

    CN_API cnbool start_gui(void);
    CN_API void end_gui(void);

    CN_API Object *new_guiboard(void);

    CN_API Object *new_guiobject(void);
    CN_API Object *new_text(void);

    CN_API Object *new_gui_submodule(void);
#endif
