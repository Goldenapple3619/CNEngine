#ifndef _LIBRGUI_H_
    #define _LIBRGUI_H_

    #include "libcncore.h"
    #include "libcngraphic.h"
    #include <SDL_ttf.h>

    struct text_mode_s {
        Vector2 position;
        cncolor color;
        int32_t size;
        const char *text;
        const char *font_location;
    };

    struct gui_board_mode_s {
        Vector2 position;
        Vector2 resolution;
        Vector2 upscale;
    };

    CN_API cnbool start_gui(void);
    CN_API void end_gui(void);

    CN_API Object *new_guiboard(void);

    CN_API Object *new_guiobject(void);
    CN_API Object *new_text(void);

#endif
