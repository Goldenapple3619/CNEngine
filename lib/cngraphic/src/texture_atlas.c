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
