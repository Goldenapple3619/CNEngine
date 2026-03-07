#include "libcngraphic.h"
#include <SDL2/SDL_image.h>

CN_API Texture *new_texture(const Vector2 *size, cnbool alpha)
{
    if (!size)
        return (NULL);

    Texture *texture = (Texture *)malloc(sizeof(Texture));

    if (!texture)
        return (NULL);

    texture->surface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, (int)round(size->x), (int)round(size->y), alpha ? 32 : 24,  alpha ? SDL_PIXELFORMAT_RGBA32 : SDL_PIXELFORMAT_RGB24);
    
    if (!texture->surface) {
        (void)free((void *)texture);
        return (NULL);
    }
    
    texture->size.x = size->x;
    texture->size.y = size->y;
    return (texture);
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

        SDL_FillRect(texture->surface, &rects[0], SDL_MapRGB(texture->surface->format, 0, 0, 0));
        SDL_FillRect(texture->surface, &rects[1], SDL_MapRGB(texture->surface->format, 106, 22, 171));
        SDL_FillRect(texture->surface, &rects[2], SDL_MapRGB(texture->surface->format, 106, 22, 171));
    }

    texture->size.x = texture->surface->w;
    texture->size.y = texture->surface->h;

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
    return (texture);
}

CN_API void delete_texture(Texture *texture)
{
    if (!texture)
        return;
    if (texture->surface)
        (void)SDL_FreeSurface(texture->surface);
    texture->size.x = 0;
    texture->size.y = 0;
    texture->surface = NULL;
    (void)free((void *)texture);
}

CN_API void blit(const Texture *__src, Texture *__dst, const Rect *__src_rect, const Vector2 *__dest_at)
{
    if (!__src || !__dst)
        return;
    (void)__src_rect;
    (void)__dest_at; // to implement
}

CN_API void blit_ratio(const Texture *__src, Texture *__dst, const Rect *__src_rect, const Vector2 *__dest_at, const Vector2 *__ratios)
{
    if (!__src || !__dst)
        return;
    (void)__src_rect;
    (void)__dest_at;
    (void)__ratios; // to implement
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
}
