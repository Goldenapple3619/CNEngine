#include "libcncore.h"

CN_API struct generic_vector_s *new_generic_vector(void)
{
    struct generic_vector_s *vec = (struct generic_vector_s *)malloc(sizeof(struct generic_vector_s));

    if (!vec) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate generic vector.")
        return (NULL);
    }

    vec->capacity = 0;
    vec->size = 0;
    vec->content = NULL;
    return (vec);
}

CN_API void delete_generic_vector(struct generic_vector_s *vec, void (*_delete_obj)(void *))
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "delete of an invalid vector.");
        return;
    }

    if (_delete_obj) {
        for (size_t i = 0; i < vec->size; ++i)
            (void)_delete_obj(vec->content[i]);
    }

    if (vec->content) {
        (void)free(vec->content);
        vec->content = NULL;
    }

    vec->capacity = 0;
    vec->size = 0;
    (void)free(vec);
}

CN_API void empty_generic_vector(struct generic_vector_s *vec, void (*_delete_obj)(void *))
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "empty of an invalid vector.");
        return;
    }

    if (_delete_obj) {
        for (size_t i = 0; i < vec->size; ++i)
            (void)_delete_obj(vec->content[i]);
    }

    if (vec->content) {
        (void)free(vec->content);
        vec->content = NULL;
    }

    vec->capacity = 0;
    vec->size = 0;
}

CN_API uint8_t resize_generic_vector(struct generic_vector_s *vec, size_t new_capacity)
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "invalid resize on empty generic vector.");
        return (1);
    }

    vec->content = realloc(vec->content, new_capacity * sizeof(void *));

    if (!vec->content) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to resize generic vector (%zu -> %zu).", vec->capacity, new_capacity);
        vec->capacity = 0;
        vec->size = 0;
        return (1);
    }

    vec->capacity = new_capacity;

    return (0);
}

CN_API uint8_t insert_generic_vector(struct generic_vector_s *vec, void *obj)
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "invalid add on empty generic vector.");
        return (1);
    }

    if (!obj) {
        RAISE(ERR_INVALID_POINTER, "element is empty.");
        return (1);
    }

    if (vec->size >= vec->capacity) {
        size_t new_capacity = vec->capacity == 0 ? 8 : vec->capacity * 2;
        if (resize_generic_vector(vec, new_capacity)) {
            PROPAGATE_ERR();
            return (1);
        }
    }

    vec->content[vec->size] = obj;
    vec->size++;
    return (0);
}

CN_API void remove_generic_vector(struct generic_vector_s *vec, size_t i, void (*_delete_obj)(void *))
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "can't remove on invalid vec.");
        return;
    }

    if (vec->size == 0 || i >= vec->size) {
        RAISE_FMT(ERR_INVALID_POINTER, "can't remove at invalid position (%zu >= %zu).", i, vec->size);
        return;
    }

    size_t last = vec->size - 1;

    if (_delete_obj)
        (void)_delete_obj(vec->content[i]);

    vec->content[i]  = vec->content[last];
    vec->size--;
}

CN_API void remove_generic_ordered_vector(struct generic_vector_s *vec, size_t i, void (*_delete_obj)(void *))
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "can't remove on invalid vec.");
        return;
    }

    if (vec->size == 0 || i >= vec->size) {
        RAISE_FMT(ERR_INVALID_POINTER, "can't remove at invalid position (%zu >= %zu).", i, vec->size);
        return;
    }

    if (_delete_obj)
        (void)_delete_obj(vec->content[i]);

    for (size_t j = i; j < vec->size - 1; ++j)
        vec->content[j] = vec->content[j + 1];

    vec->size--;
}
