#include "libcnaudio.h"

static cn_value _init(Object *__this, void **args)
{
    (void)args;
    (void)__this;

    PREP_INIT()

    INIT_OBJECT_STATIC(__this, new_atlas(NULL, (void (*)(void *))Mix_FreeChunk), NULL, "audio_atlas");
    INIT_CUSTOM_ALLOCATION(__this, new_audio_vector(), delete_audio_vector, "audio_queue");

    return (VALUE_OK);
}

static cn_value _get_free_channels(Object *__this, void **args)
{
    (void)__this;
    (void)args;

    int32_t max_channels = Mix_AllocateChannels(-1);
    struct free_channels_arr_s *free_channels = (struct free_channels_arr_s *)malloc(sizeof(struct free_channels_arr_s));

    if (!free_channels)
        return (null_value);

    free_channels->free_channels_arr = (int32_t *)malloc(sizeof(int32_t) * max_channels);
    free_channels->size = 0;

    if (!free_channels->free_channels_arr) {
        (void)free(free_channels);
        return (null_value);
    }

    for (int32_t channel = 0; channel < max_channels; ++channel) {
        if (Mix_Playing(channel) != 0)
            continue;
        free_channels->free_channels_arr[free_channels->size] = channel;
        free_channels->size += 1;
    }
    
    return ((cn_value){.type=CN_TYPE_GENERIC_UNIQ_PTR, .as.ptr = free_channels});
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;

    PREP_DEL()

    DEL_CUSTOM_ALLOCAION(__this, delete_audio_vector, "audio_queue")

    return (null_value);
}

CN_API Object *new_audio_ctx()
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "get_free_channels", &_get_free_channels);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    return (obj);
}
