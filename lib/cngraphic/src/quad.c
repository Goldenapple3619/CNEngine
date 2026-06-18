#include "libcngraphic.h"

CN_API Quad *new_quad(void)
{
    Quad *quad = malloc(sizeof(Quad));

    if (!quad) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to create quad.");
        return (NULL);
    }

    quad->shader.api = R_API_NONE;

    cnnumber quad_vert[] = {
        -1, -1,      0, 0,
         1, -1,      1, 0,
         1,  1,      1, 1,
        -1, -1,      0, 0,
         1,  1,      1, 1,
        -1,  1,      0, 1,
    };

    glGenVertexArrays(1, &quad->vao);
    glGenBuffers(1, &quad->vbo);
    glBindVertexArray(quad->vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vert), quad_vert, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    const char *vert =
        "#version 330 core\n"
        "layout(location=0) in vec2 a_pos;\n"
        "layout(location=1) in vec2 a_uv;\n"
        "out vec2 v_uv;\n"
        "void main() { gl_Position = vec4(a_pos, 0.0, 1.0); v_uv = a_uv; }\n";

    const char *frag =
        "#version 330 core\n"
        "in vec2 v_uv;\n"
        "uniform sampler2D u_screen;\n"
        "out vec4 frag_color;\n"
        "void main() { frag_color = texture(u_screen, v_uv); }\n";

    if (gl_shader_compile(&quad->shader, vert, frag)) {
        PROPAGATE_ERR();
        (void)delete_quad(quad);
        return (NULL);
    }

    return (quad);
}

CN_API Quad *new_quad2d(void)
{
    Quad *quad2d = malloc(sizeof(Quad));

    if (!quad2d) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to create quad2d.");
        return (NULL);
    }

    quad2d->shader.api = R_API_NONE;

    cnnumber quad_vert[] = {
        0, 0,       0, 0,
        1, 0,       1, 0,
        1, 1,       1, 1,
        0, 0,       0, 0,
        1, 1,       1, 1,
        0, 1,       0, 1,
    };

    glGenVertexArrays(1, &quad2d->vao);
    glGenBuffers(1, &quad2d->vbo);
    glBindVertexArray(quad2d->vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad2d->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vert), quad_vert, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    const char *vert =
        "#version 330 core\n"
        "layout(location = 0) in vec2 a_pos;\n"
        "layout(location = 1) in vec2 a_uv;\n"
        "uniform vec2 u_position;\n"
        "uniform vec2 u_size;\n"
        "uniform vec2 u_resolution;\n"
        "out vec2 v_uv;\n"
        "void main() {\n"
        "    vec2 pos = a_pos * u_size + u_position;\n"
        "    vec2 ndc = (pos / u_resolution) * 2.0 - 1.0;\n"
        "    ndc.y = -ndc.y;\n"
        "    gl_Position = vec4(ndc, 0.0, 1.0);\n"
        "    v_uv = a_uv;\n"
        "}\n";

    const char *frag =
        "#version 330 core\n"
        "in vec2 v_uv;\n"
        "uniform sampler2D u_texture;\n"
        "uniform vec4      u_color;\n"
        "uniform int       u_has_texture;\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "    vec4 base = u_has_texture == 1\n"
        "        ? texture(u_texture, v_uv) * u_color\n"
        "        : u_color;\n"
        "    frag_color = base;\n"
        "}\n";

    if (gl_shader_compile(&quad2d->shader, vert, frag)) {
        PROPAGATE_ERR();
        (void)delete_quad(quad2d);
        return (NULL);
    }

    return (quad2d);
}

CN_API void delete_quad(Quad *quad)
{
    if (!quad) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty quad.");
        return;
    }
    if (quad->vao)
        (void)glDeleteVertexArrays(1, &quad->vao);
    if (quad->vbo)
        (void)glDeleteBuffers(1, &quad->vbo);
    (void)delete_gpu_shader(&quad->shader);
    (void)free(quad);
}