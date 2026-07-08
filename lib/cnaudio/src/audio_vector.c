#include "libcnaudio.h"

#include "libcncore.h"

CN_API struct audio_vector_s *new_audio_vector(void)
{
    struct audio_vector_s *vec = (struct audio_vector_s *)malloc(sizeof(struct audio_vector_s));

    if (!vec) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new audio vector.");
        return (NULL);
    }
    vec->size = 0;
    vec->capacity = 0;
    vec->audios = NULL;
    return (vec);
}

CN_API void delete_audio_vector(struct audio_vector_s *vec)
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty audio vector.");
        return;
    }
    for (size_t i = 0; i < vec->size; ++i) {
        (void)delete_audio(vec->audios[i]);
    }
    if (vec->audios) {
        (void)free(vec->audios);
        vec->audios = NULL;
    }
    vec->size = 0;
    vec->capacity = 0;
    (void)free(vec);
}

CN_API uint8_t resize_audio_vector(struct audio_vector_s *vec, size_t new_capacity)
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "can't resize empty audio vector.");
        return (1);
    }

    vec->audios = realloc(vec->audios, new_capacity * sizeof(Audio *));

    if (!vec->audios) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to resize audio vector (%zu -> %zu).", vec->capacity, new_capacity);
        vec->capacity = 0;
        vec->size = 0;
        return (1);
    }

    vec->capacity = new_capacity;

    return (0);
}

CN_API uint8_t insert_audio_vector(struct audio_vector_s *vec, Audio *value)
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "can't insert in empty audio vector.");
        return (1);
    }

    if (vec->size >= vec->capacity) {
        size_t new_capacity = vec->capacity == 0 ? 8 : vec->capacity * 2;
        if (resize_audio_vector(vec, new_capacity)) {
            PROPAGATE_ERR();
            return (1);
        }
    }

    vec->audios[vec->size] = value;
    vec->size++;
    return (0);
}

CN_API void remove_audio_vector(struct audio_vector_s *vec, size_t i)
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "can't remove in empty audio vector.");
        return;
    }

    if (vec->size == 0 || i >= vec->size) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "can't remove at invalid position (%zu >= %zu).", i, vec->size);
        return;
    }

    size_t last = vec->size - 1;

    (void)delete_audio(vec->audios[i]);

    vec->audios[i]  = vec->audios[last];
    vec->size--;
}

CN_API void remove_audio_ordered_vector(struct audio_vector_s *vec, size_t i)
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "can't remove in audio vector.");
        return;
    }

    if (vec->size == 0 || i >= vec->size) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "can't remove at invalid position (%zu >= %zu).", i, vec->size);
        return;
    }

    (void)delete_audio(vec->audios[i]);

    for (size_t j = i; j < vec->size - 1; ++j)
        vec->audios[j] = vec->audios[j + 1];

    vec->size--;
}
