#include "libcnaudio.h"
#include <SDL_mixer.h>

static cn_value _get_free_channels(Object *__this, void **args)
{
    (void)__this;
    (void)args;

    int32_t max_channels = Mix_AllocateChannels(-1);
    struct free_channels_arr_s *free_channels = (struct free_channels_arr_s *)malloc(sizeof(struct free_channels_arr_s));

    if (!free_channels)
        return (VALUE_NULL);

    free_channels->free_channels_arr = (int32_t *)malloc(sizeof(int32_t) * max_channels);
    free_channels->size = 0;

    if (!free_channels->free_channels_arr) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate array of audio channels of size %" PRIi32 ".", max_channels);
        (void)free(free_channels);
        return (VALUE_NULL);
    }

    for (int32_t channel = 0; channel < max_channels; ++channel) {
        if (Mix_Playing(channel) != 0)
            continue;
        free_channels->free_channels_arr[free_channels->size] = channel;
        free_channels->size += 1;
    }
    
    return ((cn_value){.type=CN_TYPE_GENERIC_UNIQ_PTR, .as.ptr = free_channels});
}

static cn_value _update(Object *__this, void **args)
{
    (void)__this;
    (void)args;

    return (VALUE_NULL);
}

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()

    if (!args || !(args[0])) {
        RAISE(ERR_INVALID_POINTER, "can't init audio submodule with no ctx.");
        return (VALUE_ERR);
    }
    
    Object *ctx = (Object *)(args[0]);
    
    if (!start_audio()) {
        PROPAGATE_ERR();
        if (get_attr(__this, "init_errors")->as.b)
            return (VALUE_ERR);
    }

    if (call_method(ctx, "register_update", PACK_ARG(_update)).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }

    INIT_OBJECT_STATIC(ctx, new_atlas(NULL, (expr_free)&Mix_FreeChunk), NULL, "audio_atlas");
    INIT_CUSTOM_ALLOCATION(ctx, new_audio_vector(), delete_audio_vector, "audio_queue");

    INIT_METHOD(ctx, "get_free_audio_channels", _get_free_channels)

    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    PREP_DEL()

    (void)__this;

    if (!args || !(args[0]))
        return (VALUE_NULL);

    Object *ctx = (Object *)(args[0]);

    DEL_CUSTOM_ALLOCAION(ctx, delete_audio_vector, "audio_queue")

    end_audio();

    return (VALUE_NULL);
}

CN_API Object *new_audio_ctx(cnbool skip_init_error)
{
    Object *obj = new_object();
    cnbool init_error = !skip_init_error;

    if (!obj) {
        PROPAGATE_ERR();
        return (NULL);
    }

    SET_PARENT_CLASS_BUILD_STATIC(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    if (!set_attr(obj, "init_errors", CN_TYPE_BOOL, &init_error)) {
        PROPAGATE_ERR();
        delete_object(obj);
        return (NULL);
    }

    return (obj);
}
