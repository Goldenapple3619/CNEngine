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

    CN_API Object *new_2dboard(void);
    CN_API Object *new_tile(void);

#endif
