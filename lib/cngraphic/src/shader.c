#include "libcngraphic.h"
#include <stdio.h>
#include <stdlib.h>

static GLuint _compile_stage_gl(GLenum type, const char *src)
{
    GLuint s = glCreateShader(type);
    GLint ok;

    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);

    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);

    if (!ok) {
        char log[ERR_MSG_SIZE];

        glGetShaderInfoLog(s, sizeof(log), NULL, log);
        RAISE_FMT(ERR_OS, "failed to compile glShader (%s).", log);
        glDeleteShader(s);
        return (0);
    }

    return (s);
}

static char *_read_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    long len;
    char *buf;
    size_t _;

    if (!f) {
        RAISE_FMT(ERR_OS, "failed to open shader file '%s'.", path);
        return (NULL);
    }
    (void)fseek(f, 0, SEEK_END);
    len = ftell(f);
    (void)rewind(f);

    buf = malloc(len + 1);

    if (!buf) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate buffer of size %zu for '%s'.", len + 1, path);
        (void)fclose(f);
        return (NULL);
    }

    _ = fread(buf, 1, len, f);

    (void)_;

    buf[len] = '\0';
    (void)fclose(f);

    return (buf);
}

CN_API uint8_t gl_shader_compile(Shader *shader, const char *vert_src, const char *frag_src)
{
    GLuint vert = _compile_stage_gl(GL_VERTEX_SHADER,   vert_src);
    GLuint frag;
    GLint ok;
    GLuint program;

    if (!vert) {
        PROPAGATE_ERR();
        return (1);
    }

    frag = _compile_stage_gl(GL_FRAGMENT_SHADER, frag_src);

    if (!frag) {
        PROPAGATE_ERR();
        glDeleteShader(vert);
        return (1);
    }

    program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &ok);

    if (!ok) {
        char log[ERR_MSG_SIZE];

        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        RAISE_FMT(ERR_OS, "failed to create glShader program (%s).", log);
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    if (!program)
        return (1);

    shader->api = R_API_GL;
    shader->gpu_handler.gl.gl_shader = program;
    shader->gpu_handler.gl.gl_ctx = SDL_GL_GetCurrentContext();
    return (0);
}

CN_API uint8_t gl_shader_load(Shader *shader, const char *vert_path, const char *frag_path)
{
    char *vert_src = _read_file(vert_path);
    char *frag_src;
    uint8_t ret;

    if (!vert_src) {
        PROPAGATE_ERR()
        return (1);
    }

    frag_src = _read_file(frag_path);

    if (!frag_src) {
        PROPAGATE_ERR()
        (void)free(vert_src);
        return (1);
    }

    ret = gl_shader_compile(shader, vert_src, frag_src);

    if (ret) {
        PROPAGATE_ERR();
    }

    (void)free(vert_src);
    (void)free(frag_src);

    return (ret);
}

CN_API Shader *new_shader(void)
{
    Shader *shader = (Shader *)malloc(sizeof(Shader));

    if (!shader) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new shader.");
        return (NULL);
    }
    shader->api = R_API_NONE;
    (void)memset(&shader->gpu_handler, 0, sizeof(shader->gpu_handler));
    return (shader);
}

CN_API void delete_gpu_shader(Shader *shader)
{
    if (!shader) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty shader gpu's data.");
        return;
    }

    switch (shader->api) {
        case R_API_GL:
            (void)glDeleteProgram(shader->gpu_handler.gl.gl_shader);
            shader->gpu_handler.gl.gl_shader = 0;
            shader->gpu_handler.gl.gl_ctx = 0;
            break;
        default:
            break;
    }
    shader->api = R_API_NONE;
}

CN_API void delete_shader(Shader *shader)
{
    if (!shader) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty shader.");
        return;
    }

    (void)delete_gpu_shader(shader);
    (void)free(shader);
}
