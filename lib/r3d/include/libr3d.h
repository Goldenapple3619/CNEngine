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

    typedef enum {
        R_API_UKN = 0x00,
        R_API_GL,
        R_API_VULKAN,
        R_API_METAL,
        R_API_DX11
    } rendering_api;

    typedef struct {
        cnnumber x, y, z;
        cnnumber nx, ny, nz;
        cnnumber u, v;
    } Vertex;

    typedef struct {
        Texture *texture;
        cncolor color;

        union {
            uint32_t  gl_shader;
        } gpu_handler;

        cnnumber ambient;
        cnnumber diffuse;
        cnnumber specular;
        cnnumber shininess;

        rendering_api api;
    } Material;

    typedef struct {
        Vertex *vertices;
        uint32_t *indices;
        size_t vertex_count;
        size_t index_count;

        rendering_api api;
        cnbool uploaded;
        union {
            struct {
                uint32_t vao;
                uint32_t vbo;
                uint32_t ebo;
            } gl;
        } gpu_handler;
    } Mesh;

    struct threed_board_mode_s {
        Vector2 position;
        Vector2 resolution;
        Vector2 upscale;

        Object *scene;
    };

    GLuint shader_compile(const char *vert_src, const char *frag_src);
    GLuint shader_load(const char *vert_path, const char *frag_path);

    CN_API Object *new_3dboard(void);
    CN_API Object *new_camera3d(void);
    CN_API Object *new_object3d(void);

    CN_API Mesh *new_mesh(void);
    void mesh_draw_gl(const Mesh *m);
    cnbool mesh_upload_gl(Mesh *m);
    CN_API void delete_mesh(Mesh *mesh);

    CN_API Material *new_material(void);
    void material_use_gl(const Material *mat,
                  const float model[16],
                  const float view[16],
                  const float proj[16]);
    CN_API void delete_material(Material *material);

#endif
