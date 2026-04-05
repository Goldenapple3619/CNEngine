#ifndef _LIBR3D_H_
    #define _LIBR3D_H_

    #include "libcncore.h"
    #include "libcngraphic.h"

    #ifdef __APPLE__
        #define GL_SILENCE_DEPRECATION
        #include <OpenGL/gl.h>
    #else
        #include <GL/gl.h>
    #endif

    struct threed_board_mode_s {
        Vector2 position;
        Vector2 resolution;
        Vector2 upscale;

        Object *scene;
    };

    typedef struct {
        Object *obj;

        Window *window;
        Texture *cpu_texture;

        Vector2 canva_position;
        Vector2 canva_size;
        Vector2 canva_scale;

        cnnumber proj[16];
        cnnumber view[16];
    } threed_render_stack;

    CN_API Object *new_3dboard(void);
    CN_API Object *new_camera3d(void);
    CN_API Object *new_object3d(void);

    CN_API void _make_proj(cnnumber out[16],
                       cnnumber fov_deg, cnnumber aspect,
                       cnnumber near, cnnumber far);
    CN_API void _make_view(cnnumber out[16],
                       const Vector3 *pos, const Vector3 *rot);
    CN_API void _make_model(cnnumber out[16],
                        const Vector3 *pos,
                        const Vector3 *scl,
                        const Vector3 *rot);

#endif
