#include "libcngraphic.h"
#include <SDL_image.h>

CN_API Texture *new_texture(const Vector2 *size, cnbool alpha)
{
    if (!size || size->x <= 0 || size->y <= 0)
        return (NULL);

    Texture *texture = (Texture *)malloc(sizeof(Texture));

    if (!texture)
        return (NULL);

    texture->surface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, (int)size->x, (int)size->y, alpha ? 32 : 24,  alpha ? SDL_PIXELFORMAT_RGBA32 : SDL_PIXELFORMAT_RGB24);
    
    if (!texture->surface) {
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
    if (!texture || !new_size || new_size->x <= 0 || new_size->y <= 0)
        return (1);

    cnbool alpha = texture->surface->format->BytesPerPixel == 4;

    SDL_Surface *new_surface = SDL_CreateRGBSurfaceWithFormat(
        SDL_SWSURFACE,
        (int)new_size->x, (int)new_size->y,
        alpha ? 32 : 24,
        alpha ? SDL_PIXELFORMAT_RGBA32 : SDL_PIXELFORMAT_RGB24);

    if (!new_surface)
        return (1);

    INVALIDATE_GPU(texture);

    (void)SDL_FreeSurface(texture->surface);
    texture->surface = new_surface;
    texture->size.x  = new_size->x;
    texture->size.y  = new_size->y;
    return (0);
}

CN_API void draw_texture(Texture *__src_texture, SDL_Renderer *__dest_renderer, const Rect *__src_rect, const Vector2 *__dest_at, const Vector2 *__ratios, double __angle)
{
    if (!__src_texture || !__dest_renderer || __src_texture->api != R_API_SDL)
        return;

    if (!__src_texture->gpu_handler.sdl_texture.gpu_texture || __src_texture->gpu_handler.sdl_texture.renderer != __dest_renderer) {
        __src_texture->gpu_handler.sdl_texture.renderer = __dest_renderer;
        __src_texture->gpu_handler.sdl_texture.gpu_texture = SDL_CreateTextureFromSurface(__dest_renderer, __src_texture->surface);
        if (!__src_texture->gpu_handler.sdl_texture.gpu_texture)
            return;
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

CN_API Texture *copy_texture(Texture *texture)
{
    if (!texture)
        return (NULL);

    SDL_Surface *surface = SDL_ConvertSurface(texture->surface, texture->surface->format, SDL_SWSURFACE);
    Texture *temp;

    if (!surface)
        return (NULL);

    temp = new_texture_from_surface(surface);

    if (!temp) {
        (void)SDL_FreeSurface(surface);
        return (NULL);
    }

    return (temp);
}

CN_API Texture *new_texture_from_file(const char *path)
{
    if (!path)
        return (NULL);

    Texture *texture = (Texture *)malloc(sizeof(Texture));

    if (!texture)
        return (NULL);

    texture->surface = IMG_Load(path);

    if (!texture->surface) {
        texture->surface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, 100, 100, 32, SDL_PIXELFORMAT_RGBA32);

        if (!texture->surface) {
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
    if (!surface)
        return (NULL);

    Texture *texture = (Texture *)malloc(sizeof(Texture));

    if (!texture) {
        (void)SDL_FreeSurface(surface);
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
    if (!texture)
        return;
    switch (texture->api) {
        case R_API_SDL:
            if (texture->gpu_handler.sdl_texture.gpu_texture)
                (void)SDL_DestroyTexture(texture->gpu_handler.sdl_texture.gpu_texture);
            break;
        case R_API_GL:
            if (texture->gpu_handler.gl_id)
                (void)glDeleteTextures(1, &texture->gpu_handler.gl_id);
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
    if (!__src || !__dst)
        return;
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
    if (!__src || !__dst)
        return;
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
    if (!texture)
        return;
    (void)SDL_SetSurfaceAlphaMod(texture->surface, (uint8_t)fmax(0, fmin(255, opacity)));
    INVALIDATE_GPU(texture);
}

CN_API uint8_t get_opacity_texture(const Texture *texture)
{
    if (!texture)
        return (0);

    uint8_t a;

    (void)SDL_GetSurfaceAlphaMod(texture->surface, &a);
    return (a);
}

CN_API void draw_rect(Texture *texture, const Rect *rect, cncolor color)
{
    if (!texture)
        return;
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
    if (!texture)
        return;
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
    if (!texture || !texture->surface || texture->api != R_API_GL)
        return false;

    if (texture->gpu_handler.gl_id)
        glDeleteTextures(1, &texture->gpu_handler.gl_id);

    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(texture->surface,
                            SDL_PIXELFORMAT_RGBA32, 0);
    if (!rgba)
        return false;

    glGenTextures(1, &texture->gpu_handler.gl_id);
    glBindTexture(GL_TEXTURE_2D, texture->gpu_handler.gl_id);

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
    return true;
}
