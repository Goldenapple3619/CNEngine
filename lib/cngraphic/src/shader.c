#include "libcngraphic.h"
#include <stdio.h>
#include <stdlib.h>

static GLuint _compile_stage(GLenum type, const char *src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);

    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), NULL, log);
        fprintf(stderr, "shader compile error:\n%s\n", log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

static char *_read_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "shader: cannot open %s\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char *buf = malloc(len + 1);
    size_t _ = fread(buf, 1, len, f);
    (void)_;
    buf[len] = '\0';
    fclose(f);
    return buf;
}

GLuint gl_shader_compile(const char *vert_src, const char *frag_src)
{
    GLuint vert = _compile_stage(GL_VERTEX_SHADER,   vert_src);
    GLuint frag = _compile_stage(GL_FRAGMENT_SHADER, frag_src);

    if (!vert || !frag) {
        glDeleteShader(vert);
        glDeleteShader(frag);
        return 0;
    }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof(log), NULL, log);
        fprintf(stderr, "shader link error:\n%s\n", log);
        glDeleteProgram(prog);
        prog = 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return prog;
}

GLuint gl_shader_load(const char *vert_path, const char *frag_path)
{
    char *vert_src = _read_file(vert_path);
    char *frag_src = _read_file(frag_path);

    if (!vert_src || !frag_src) {
        free(vert_src);
        free(frag_src);
        return 0;
    }

    GLuint prog = gl_shader_compile(vert_src, frag_src);
    free(vert_src);
    free(frag_src);
    return prog;
}