#ifndef _LIBR3D_H_
    #define _LIBR3D_H_

    #include "libcncore.h"
    #include "libcngraphic.h"

    #include <GL/gl.h>

    struct threed_board_mode_s {
        Vector2 position;
        Vector2 resolution;
        Vector2 upscale;

        Object *scene;
    };

    CN_API Object *new_3dboard(void);

#endif
