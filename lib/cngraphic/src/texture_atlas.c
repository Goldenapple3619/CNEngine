#include "libcngraphic.h"

CN_API TextureAtlas *new_texture_atlas(void)
{
    TextureAtlas *atlas = (TextureAtlas *)malloc(sizeof(TextureAtlas));

    if (!atlas)
        return (NULL);

    atlas->capacity = 0;
    atlas->size = 0;
    atlas->keys = NULL;
    atlas->content = NULL;

    return (atlas);
}

CN_API uint8_t texture_atlas_resize(TextureAtlas *atlas, size_t new_capacity)
{
    if (!atlas)
        return (1);

    atlas->content = realloc(atlas->content, new_capacity * sizeof(Texture *));
    atlas->keys  = realloc(atlas->keys,  new_capacity * sizeof(uint64_t));

    if (!atlas->content || !atlas->keys) {
        atlas->capacity = 0;
        return (1);
    }

    atlas->capacity = new_capacity;
    return (0);
}

CN_API uint8_t add_texture_atlas(TextureAtlas *atlas, Texture *texture, const char *key)
{
    if (!atlas || !key || !texture)
        return (1);

    uint64_t k = _get_attrs_hash(key);

    for (size_t i = 0; i < atlas->size; ++i) {
        if (atlas->keys[i] == k) {
            (void)delete_texture(atlas->content[i]);
            atlas->content[i] = texture;
            return (0);
        }
    }

    if (atlas->size >= atlas->capacity) {
        size_t new_capacity = atlas->capacity == 0 ? 8 : atlas->capacity * 2;
        if (texture_atlas_resize(atlas, new_capacity))
            return (1);
    }

    atlas->keys[atlas->size]  = k;
    atlas->content[atlas->size] = texture;
    atlas->size++;
    return (0);
}

CN_API void remove_texture_atlas(TextureAtlas *atlas, const char *key)
{
    if (!atlas || !key || atlas->size == 0)
        return;

    uint64_t k = _get_attrs_hash(key);

    for (size_t i = 0; i < atlas->size; ++i) {
        if (atlas->keys[i] == k) {
            size_t last = atlas->size - 1;

            (void)delete_texture(atlas->content[i]);

            atlas->keys[i]  = atlas->keys[last];
            atlas->content[i] = atlas->content[last];

            atlas->size--;
            return;
        }
    }
}

CN_API const Texture *get_texture(TextureAtlas *atlas, const char *key, Texture *(*tex_from_key)(const char *))
{
    if (!atlas || !key)
        return (NULL);

    uint64_t k = _get_attrs_hash(key);
    
    for (size_t i = 0; i < atlas->size; ++i) {
        if (atlas->keys[i] == k)
            return (atlas->content[i]);
    }

    Texture *texture = ((!tex_from_key) ? new_texture_from_file : tex_from_key)(key);

    if (!texture)
        return (NULL);

    if (add_texture_atlas(atlas, texture, key)) {
        (void)delete_texture(texture);
        return (NULL);
    }

    return (texture);
}

CN_API void delete_texture_atlas(TextureAtlas *atlas)
{
    if (!atlas)
        return;

    for (size_t i = 0; i < atlas->size; ++i) {
        (void)delete_texture(atlas->content[i]);
    }
    if (atlas->keys) {
        (void)free(atlas->keys);
        atlas->keys = NULL;
    }
    if (atlas->content) {
        (void)free(atlas->content);
        atlas->content = NULL;
    }
    atlas->capacity = 0;
    atlas->keys = 0;
    (void)free(atlas);
}
