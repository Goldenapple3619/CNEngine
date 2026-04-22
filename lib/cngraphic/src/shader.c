#include "libcngraphic.h"
#include <stdio.h>
#include <stdlib.h>

static GLuint _compile_stage(GLenum type, const char *src)
{
    GLuint s = glCreateShader(type);
    GLint ok;

    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);

    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);

    if (!ok) {
        char log[1024];

        glGetShaderInfoLog(s, sizeof(log), NULL, log);
        fprintf(stderr, "shader compile error:\n%s\n", log);
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
        fprintf(stderr, "shader: cannot open %s\n", path);
        return (NULL);
    }
    (void)fseek(f, 0, SEEK_END);
    len = ftell(f);
    (void)rewind(f);

    buf = malloc(len + 1);
    _ = fread(buf, 1, len, f);

    (void)_;

    buf[len] = '\0';
    (void)fclose(f);

    return (buf);
}

CN_API uint8_t gl_shader_compile(Shader *shader, const char *vert_src, const char *frag_src)
{
    GLuint vert = _compile_stage(GL_VERTEX_SHADER,   vert_src);
    GLuint frag = _compile_stage(GL_FRAGMENT_SHADER, frag_src);
    GLint ok;
    GLuint program;

    if (!vert || !frag) {
        glDeleteShader(vert);
        glDeleteShader(frag);
        return (1);
    }

    program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &ok);

    if (!ok) {
        char log[1024];

        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        fprintf(stderr, "shader link error:\n%s\n", log);
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    if (!program)
        return (1);

    shader->api = R_API_GL;
    shader->gpu_handler.gl_shader = program;
    return (0);
}

CN_API uint8_t gl_shader_load(Shader *shader, const char *vert_path, const char *frag_path)
{
    char *vert_src = _read_file(vert_path);
    char *frag_src = _read_file(frag_path);
    uint8_t ret;

    if (!vert_src || !frag_src) {
        (void)free(vert_src);
        (void)free(frag_src);
        return (1);
    }

    ret = gl_shader_compile(shader, vert_src, frag_src);

    (void)free(vert_src);
    (void)free(frag_src);

    return (ret);
}

CN_API Shader *new_shader(void)
{
    Shader *shader = (Shader *)malloc(sizeof(Shader));

    if (!shader)
        return (NULL);
    shader->api = R_API_NONE;
    (void)memset(&shader->gpu_handler, 0, sizeof(shader->gpu_handler));
    return (shader);
}

CN_API void delete_gpu_shader(Shader *shader)
{
    if (!shader)
        return;

    switch (shader->api) {
        case R_API_GL:
            (void)glDeleteProgram(shader->gpu_handler.gl_shader);
            shader->gpu_handler.gl_shader = 0;
            break;
        default:
            break;
    }
    shader->api = R_API_NONE;
}

CN_API void delete_shader(Shader *shader)
{
    if (!shader)
        return;

    (void)delete_gpu_shader(shader);
    (void)free(shader);
}
