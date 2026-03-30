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

    typedef struct {
        Texture *texture;
        GLuint shader;
        cncolor color;
    } Material;

    typedef struct {
        GLuint vao;
        size_t vertex_count;
    } Mesh;

    struct threed_board_mode_s {
        Vector2 position;
        Vector2 resolution;
        Vector2 upscale;

        Object *scene;
    };

    CN_API Object *new_3dboard(void);

    CN_API Mesh *new_mesh(void);
    CN_API void delete_mesh(Mesh *mesh);

    CN_API Material *new_material(void);
    CN_API void delete_material(Material *material);

#endif
