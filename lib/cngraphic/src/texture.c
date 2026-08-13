#include "libcngraphic.h"
#include <SDL_image.h>

CN_API Texture *new_texture(const Vector2 *size, cnbool alpha)
{
    if (!size) {
        RAISE(ERR_INVALID_POINTER, "can't create texture with no size.");
        return (NULL);
    }
    if (size->x <= 0 || size->y <= 0) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "can't create texture with invalid sizes (%f x %f).", size->x, size->y);
        return (NULL);
    }

    Texture *texture = (Texture *)malloc(sizeof(Texture));

    if (!texture) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new texture.");
        return (NULL);
    }

    texture->surface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, (int)size->x, (int)size->y, alpha ? 32 : 24,  alpha ? SDL_PIXELFORMAT_RGBA32 : SDL_PIXELFORMAT_RGB24);
    
    if (!texture->surface) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new sdl_surface of size (%f x %f).", size->x, size->y);
        (void)free((void *)texture);
        return (NULL);
    }
    
    texture->size.x = size->x;
    texture->size.y = size->y;
    texture->api = R_API_NONE;
    memset(&texture->gpu_handler, 0, sizeof(texture->gpu_handler));
    return (texture);
}

CN_API uint8_t resize_texture(Texture *texture, const Vector2 *new_size)
{
    if (!texture) {
        RAISE(ERR_INVALID_POINTER, "can't resize an empty texture.");
        return (1);
    }

    if (!new_size) {
        RAISE(ERR_INVALID_POINTER, "can't resize texture with no size.");
        return (1);
    }
    if (new_size->x <= 0 || new_size->y <= 0) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "can't resize texture with invalid sizes (%f x %f).", new_size->x, new_size->y);
        return (1);
    }

    cnbool alpha = texture->surface->format->BytesPerPixel == 4;

    SDL_Surface *new_surface = SDL_CreateRGBSurfaceWithFormat(
        SDL_SWSURFACE,
        (int)new_size->x, (int)new_size->y,
        alpha ? 32 : 24,
        alpha ? SDL_PIXELFORMAT_RGBA32 : SDL_PIXELFORMAT_RGB24);

    if (!new_surface) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to resize texture by allocating a new one (%f x %f -> %f x %f).", texture->size.x, texture->size.y, new_size->x, new_size->y);
        return (1);
    }

    INVALIDATE_GPU(texture);

    (void)SDL_FreeSurface(texture->surface);
    texture->surface = new_surface;
    texture->size.x  = new_size->x;
    texture->size.y  = new_size->y;
    return (0);
}

CN_API void draw_texture(Texture *__src_texture, SDL_Renderer *__dest_renderer, const Rect *__src_rect, const Vector2 *__dest_at, const Vector2 *__ratios, double __angle)
{
    if (!__src_texture || !__dest_renderer) {
        RAISE(ERR_INVALID_POINTER, "can't draw a texture with no source / destination.");
        return;
    }

    if (__src_texture->api != R_API_SDL) {
        RAISE(ERR_NOT_COMPATIBLE, "can't draw a texture not made for SDL renderer.");
        return;
    }

    if (!__src_texture->gpu_handler.sdl_texture.gpu_texture || __src_texture->gpu_handler.sdl_texture.renderer != __dest_renderer) {
        __src_texture->gpu_handler.sdl_texture.renderer = __dest_renderer;
        __src_texture->gpu_handler.sdl_texture.gpu_texture = SDL_CreateTextureFromSurface(__dest_renderer, __src_texture->surface);
        if (!__src_texture->gpu_handler.sdl_texture.gpu_texture) {
            RAISE_FMT(ERR_OS, "failed to create new sdl texture (%s).", SDL_GetError());
            return;
        }
    }

    Rect r = {0, 0, 0, 0};
    Vector2 dst_vec = {0, 0};
    Vector2 ratio_vec = {1, 1};

    if (__src_rect) {
        r.x = __src_rect->x;
        r.y = __src_rect->y;
        r.w = (__src_rect->w ? __src_rect->w : __src_texture->size.x);
        r.h = (__src_rect->h ? __src_rect->h : __src_texture->size.y);
    } else {
        r.w = __src_texture->size.x;
        r.h = __src_texture->size.y;
    }

    if (__dest_at)
        (void)memcpy(&dst_vec, __dest_at, sizeof(Vector2));
    if (__ratios)
        (void)memcpy(&ratio_vec, __ratios, sizeof(Vector2));

    (void)SDL_RenderCopyEx(__dest_renderer, __src_texture->gpu_handler.sdl_texture.gpu_texture,
        &(SDL_Rect){(int)r.x, (int)r.y, (int)r.w, (int)r.h},
        &(SDL_Rect){
            (int)dst_vec.x, (int)dst_vec.y,
            (int)(__src_texture->size.x * ratio_vec.x),
            (int)(__src_texture->size.y * ratio_vec.y)
        }, __angle, NULL, SDL_FLIP_NONE);
}

CN_API void draw_texture_gl(Texture *__src_texture, Quad *__dst_quad, const Vector2 *__at, const Vector2 *__size, cncolor __tint, const Vector2 *__view_port)
{
    if (!__src_texture || !__dst_quad) {
        RAISE(ERR_INVALID_POINTER, "can't draw a texture with no source / destination.");
        return;
    }

    if (__src_texture->api != R_API_GL || __dst_quad->shader.api != R_API_GL) {
        RAISE(ERR_NOT_COMPATIBLE, "can't draw a texture on opengl with src/dest not being made for opengl.");
        return;
    };

    if (!__src_texture->gpu_handler.gl_texture.gl_id || __src_texture->gpu_handler.gl_texture.gl_ctx != SDL_GL_GetCurrentContext()) {
        if (!texture_upload_gl(__src_texture)) {
            PROPAGATE_ERR();
            return;
        }
    }

    if (!__dst_quad->shader.gpu_handler.gl.gl_shader) {
        RAISE(ERR_INVALID_POINTER, "can't draw a texture using quad that has an invalid shader.");
        return;
    }

    if (__dst_quad->shader.gpu_handler.gl.gl_ctx != SDL_GL_GetCurrentContext()) {
        RAISE(ERR_INVALID_POINTER, "can't draw a texture that has a quad that has a shader made by ctx different than the actual one.");
        return;
    }


    Vector2 size = *__size;

    if (__size->x == 0)
        size.x = __src_texture->size.x;
    if (__size->y == 0)
        size.y = __src_texture->size.y;

    uint32_t depth_was_enabled = glIsEnabled(GL_DEPTH_TEST);
    uint32_t blend_was_enabled = glIsEnabled(GL_BLEND);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(__dst_quad->shader.gpu_handler.gl.gl_shader);

    glUniform2f(glGetUniformLocation(__dst_quad->shader.gpu_handler.gl.gl_shader, "u_position"),   __at->x, __at->y);
    glUniform2f(glGetUniformLocation(__dst_quad->shader.gpu_handler.gl.gl_shader, "u_size"),       size.x, size.y);
    glUniform2f(glGetUniformLocation(__dst_quad->shader.gpu_handler.gl.gl_shader, "u_resolution"), __view_port->x, __view_port->y);
    glUniform4f(glGetUniformLocation(__dst_quad->shader.gpu_handler.gl.gl_shader, "u_color"),
            ((__tint & 0xff000000) >> 24) / 255.0f,
            ((__tint & 0x00ff0000) >> 16) / 255.0f,
            ((__tint & 0x0000ff00) >> 8) / 255.0f,
            (__tint & 0x000000ff) / 255.0f);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, __src_texture->gpu_handler.gl_texture.gl_id);
    glUniform1i(glGetUniformLocation(__dst_quad->shader.gpu_handler.gl.gl_shader, "u_texture"), 0);
    glUniform1i(glGetUniformLocation(__dst_quad->shader.gpu_handler.gl.gl_shader, "u_has_texture"), 1);

    glBindVertexArray(__dst_quad->vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    if (depth_was_enabled)
        glEnable(GL_DEPTH_TEST);
    if (!blend_was_enabled)
        glDisable(GL_BLEND);
}

CN_API Texture *copy_texture(Texture *texture)
{
    if (!texture) {
        RAISE(ERR_INVALID_POINTER, "can't copy empty texture.");
        return (NULL);
    }

    SDL_Surface *surface = SDL_ConvertSurface(texture->surface, texture->surface->format, SDL_SWSURFACE);
    Texture *temp;

    if (!surface) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new surface.");
        return (NULL);
    }

    temp = new_texture_from_surface(surface);

    if (!temp) {
        PROPAGATE_ERR();
        (void)SDL_FreeSurface(surface);
        return (NULL);
    }

    return (temp);
}

CN_API Texture *new_texture_from_buffer(const void *buffer, size_t size)
{
    if (!buffer || size == 0) {
        RAISE(ERR_INVALID_POINTER, "can't create a texture from buffer if it is empty.");
        return (NULL);
    }

    Texture *texture = (Texture *)malloc(sizeof(Texture));

    if (!texture) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate texture.");
        return (NULL);
    }

    SDL_RWops *rw = SDL_RWFromConstMem(buffer, (int)size);

    if (!rw) {
        RAISE(ERR_OS, "failed to create RWops from buffer.");
        (void)free(texture);
        return (NULL);
    }

    texture->surface = IMG_Load_RW(rw, 1);

    if (!texture->surface) {
        RAISE(ERR_OS, "failed to load texture from buffer, missing texture is being created instead.");
        texture->surface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, 100, 100, 32, SDL_PIXELFORMAT_RGBA32);

        if (!texture->surface) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate missing texture.");
            (void)free(texture);
            return (NULL);
        }
        SDL_Rect rects[3] = {{0, 0, 100, 100}, {50, 0, 50, 50}, {0, 50, 50, 50}};

        (void)SDL_FillRect(texture->surface, &rects[0], SDL_MapRGB(texture->surface->format, 0, 0, 0));
        (void)SDL_FillRect(texture->surface, &rects[1], SDL_MapRGB(texture->surface->format, 106, 22, 171));
        (void)SDL_FillRect(texture->surface, &rects[2], SDL_MapRGB(texture->surface->format, 106, 22, 171));
    }

    texture->size.x = texture->surface->w;
    texture->size.y = texture->surface->h;
    texture->api = R_API_NONE;
    memset(&texture->gpu_handler, 0, sizeof(texture->gpu_handler));

    return (texture);
}

CN_API Texture *new_texture_from_file(const char *path)
{
    if (!path) {
        RAISE(ERR_INVALID_POINTER, "can't create a texture from file if the path is empty.");
        return (NULL);
    }

    Texture *texture = (Texture *)malloc(sizeof(Texture));

    if (!texture) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate texture.");
        return (NULL);
    }

    texture->surface = IMG_Load(path);

    if (!texture->surface) {
        RAISE_FMT(ERR_OS, "failed to load texture from image '%s', missing texture is being created instead.", path);
        texture->surface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, 100, 100, 32, SDL_PIXELFORMAT_RGBA32);

        if (!texture->surface) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate missing texture.");
            (void)free(texture);
            return (NULL);
        }
        SDL_Rect rects[3] = {{0, 0, 100, 100}, {50, 0, 50, 50}, {0, 50, 50, 50}};

        (void)SDL_FillRect(texture->surface, &rects[0], SDL_MapRGB(texture->surface->format, 0, 0, 0));
        (void)SDL_FillRect(texture->surface, &rects[1], SDL_MapRGB(texture->surface->format, 106, 22, 171));
        (void)SDL_FillRect(texture->surface, &rects[2], SDL_MapRGB(texture->surface->format, 106, 22, 171));
    }

    texture->size.x = texture->surface->w;
    texture->size.y = texture->surface->h;
    texture->api = R_API_NONE;
    memset(&texture->gpu_handler, 0, sizeof(texture->gpu_handler));

    return (texture);
}

CN_API Texture *new_texture_from_surface(SDL_Surface *surface)
{
    if (!surface) {
        RAISE(ERR_INVALID_POINTER, "can't create texture from empty surface.");
        return (NULL);
    }

    Texture *texture = (Texture *)malloc(sizeof(Texture));

    if (!texture) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate texture.");
        return (NULL);
    }

    texture->surface = surface;
    texture->size.x = texture->surface->w;
    texture->size.y = texture->surface->h;
    texture->api = R_API_NONE;
    memset(&texture->gpu_handler, 0, sizeof(texture->gpu_handler));
    return (texture);
}

CN_API void delete_texture(Texture *texture)
{
    if (!texture) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty texture.");
        return;
    }
    switch (texture->api) {
        case R_API_SDL:
            if (texture->gpu_handler.sdl_texture.gpu_texture)
                (void)SDL_DestroyTexture(texture->gpu_handler.sdl_texture.gpu_texture);
            break;
        case R_API_GL:
            if (texture->gpu_handler.gl_texture.gl_id)
                (void)glDeleteTextures(1, &texture->gpu_handler.gl_texture.gl_id);
        default:
            break;
    }

    if (texture->surface)
        (void)SDL_FreeSurface(texture->surface);
    texture->size.x = 0;
    texture->size.y = 0;
    texture->surface = NULL;
    memset(&texture->gpu_handler, 0, sizeof(texture->gpu_handler));
    (void)free((void *)texture);
}

CN_API void blit(const Texture *__src, Texture *__dst, const Rect *__src_rect, const Vector2 *__dest_at)
{
    if (!__src || !__dst) {
        RAISE(ERR_INVALID_POINTER, "can't blit empty src/dst texture.");
        return;
    }
    Rect r = {0, 0, 0, 0};
    Vector2 dst_vec = {0, 0};

    if (__src_rect) {
        r.x = __src_rect->x;
        r.y = __src_rect->y;
        r.w = (__src_rect->w ? __src_rect->w : __src->size.x);
        r.h = (__src_rect->h ? __src_rect->h : __src->size.y);
    } else {
        r.w = __src->size.x;
        r.h = __src->size.y;
    }

    if (__dest_at)
        (void)memcpy(&dst_vec, __dest_at, sizeof(Vector2));
    (void)SDL_BlitSurface(__src->surface,
        &(SDL_Rect){
            (int)r.x, (int)r.y, (int)r.w, (int)r.h
        }, __dst->surface,
        &(SDL_Rect){
            (int)dst_vec.x, (int)dst_vec.y, (int)r.w, (int)r.h
    });
    INVALIDATE_GPU(__dst);
}

CN_API void blit_ratio(const Texture *__src, Texture *__dst, const Rect *__src_rect, const Vector2 *__dest_at, const Vector2 *__ratios)
{
    if (!__src || !__dst) {
        RAISE(ERR_INVALID_POINTER, "can't blit empty src/dst texture.");
        return;
    }
    Rect r = {0, 0, 0, 0};
    Vector2 dst_vec = {0, 0};
    Vector2 ratio_vec = {1, 1};

    if (__src_rect) {
        r.x = __src_rect->x;
        r.y = __src_rect->y;
        r.w = (__src_rect->w ? __src_rect->w : __src->size.x);
        r.h = (__src_rect->h ? __src_rect->h : __src->size.y);
    } else {
        r.w = __src->size.x;
        r.h = __src->size.y;
    }

    if (__dest_at)
        (void)memcpy(&dst_vec, __dest_at, sizeof(Vector2));
    if (__ratios)
        (void)memcpy(&ratio_vec, __ratios, sizeof(Vector2));

    (void)SDL_BlitScaled(__src->surface,
        &(SDL_Rect){
            (int)r.x, (int)r.y, (int)r.w, (int)r.h
        }, __dst->surface,
        &(SDL_Rect){
            (int)dst_vec.x, (int)dst_vec.y, (int)(__src->size.x * ratio_vec.x), (int)(__src->size.y * ratio_vec.y)
    });
    INVALIDATE_GPU(__dst);
}

CN_API void set_opacity_texture(Texture *texture, uint8_t opacity)
{
    if (!texture) {
        RAISE(ERR_INVALID_POINTER, "can't set opacity of empty texture.");
        return;
    }
    (void)SDL_SetSurfaceAlphaMod(texture->surface, (uint8_t)fmax(0, fmin(255, opacity)));
    INVALIDATE_GPU(texture);
}

CN_API uint8_t get_opacity_texture(const Texture *texture)
{
    if (!texture) {
        RAISE(ERR_INVALID_POINTER, "can't get opacity of empty texture.");
        return (0);
    }

    uint8_t a;

    (void)SDL_GetSurfaceAlphaMod(texture->surface, &a);
    return (a);
}

CN_API void draw_rect(Texture *texture, const Rect *rect, cncolor color)
{
    if (!texture) {
        RAISE(ERR_INVALID_POINTER, "can't draw on empty texture.");
        return;
    }
    SDL_FillRect(texture->surface,
        &(SDL_Rect){(int)rect->x, (int)rect->y, (int)rect->w, (int)rect->h},
        SDL_MapRGBA(texture->surface->format,
            (color & 0xff000000) >> 24,
            (color & 0x00ff0000) >> 16,
            (color & 0x0000ff00) >> 8,
            (color & 0x000000ff)));
    INVALIDATE_GPU(texture);
}

CN_API void clear_texture(Texture *texture, cncolor color)
{
    if (!texture) {
        RAISE(ERR_INVALID_POINTER, "can't clear empty texture.");
        return;
    }
    (void)SDL_FillRect(texture->surface,
        NULL,
        SDL_MapRGBA(texture->surface->format,
            (color & 0xff000000) >> 24,
            (color & 0x00ff0000) >> 16,
            (color & 0x0000ff00) >> 8,
            (color & 0x000000ff)));
    INVALIDATE_GPU(texture);
}

CN_API cnbool texture_upload_gl(Texture *texture)
{
    if (!texture || !texture->surface) {
        RAISE(ERR_INVALID_POINTER, "can't upload empty texture.");
        return false;
    }

    if (texture->api != R_API_GL) {
        RAISE(ERR_INVALID_POINTER, "can't upload to opengl texture not made for opengl.");
        return (false);
    }

    if (texture->gpu_handler.gl_texture.gl_id)
        glDeleteTextures(1, &texture->gpu_handler.gl_texture.gl_id);

    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(texture->surface,
                            SDL_PIXELFORMAT_RGBA32, 0);
    if (!rgba) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new suface.");
        return false;
    }

    glGenTextures(1, &texture->gpu_handler.gl_texture.gl_id);
    glBindTexture(GL_TEXTURE_2D, texture->gpu_handler.gl_texture.gl_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 rgba->w, rgba->h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 rgba->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    SDL_FreeSurface(rgba);
    texture->gpu_handler.gl_texture.gl_ctx = SDL_GL_GetCurrentContext();
    return true;
}
