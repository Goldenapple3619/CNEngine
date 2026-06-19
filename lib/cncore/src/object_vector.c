#include "libcncore.h"

CN_API ObjectVector *new_object_vector(void)
{
    ObjectVector *vec = (ObjectVector *)malloc(sizeof(ObjectVector));

    if (!vec) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate object vector.");
        return (NULL);
    }
    vec->capacity = 0;
    vec->size = 0;
    vec->objects = NULL;
    return (vec);
}

CN_API void delete_object_vector(ObjectVector *vec)
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty object vector.");
        return;
    }
    for (size_t i = 0; i < vec->size; ++i) {
        (void)collect_object(vec->objects[i]);
    }
    if (vec->objects) {
        (void)free(vec->objects);
        vec->objects = NULL;
    }
    vec->capacity = 0;
    vec->size = 0;
    (void)free(vec);
}

CN_API uint8_t resize_object_vector(ObjectVector *vec, size_t new_capacity)
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "can't resize empty object vector.");
        return (1);
    }

    vec->objects = realloc(vec->objects, new_capacity * sizeof(Object *));

    if (!vec->objects) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to resize object vector (%zu -> %zu).", vec->capacity, new_capacity);
        vec->size = 0;
        vec->capacity = 0;
        return (1);
    }

    vec->capacity = new_capacity;

    return (0);
}

CN_API uint8_t insert_object_vector(ObjectVector *vec, Object *obj)
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "can't insert in empty object vector.");
        return (1);
    }

    if (!obj) {
        RAISE(ERR_INVALID_POINTER, "can't insert empty object.");
        return (1);
    }

    if (vec->size >= vec->capacity) {
        size_t new_capacity = vec->capacity == 0 ? 8 : vec->capacity * 2;
        if (resize_object_vector(vec, new_capacity)) {
            PROPAGATE_ERR()
            return (1);
        }
    }

    vec->objects[vec->size] = obj;
    vec->size++;
    (void)share_object(obj);
    return (0);
}

CN_API void remove_object_vector(ObjectVector *vec, size_t i)
{
    if (!vec) {
        RAISE(ERR_INVALID_POINTER, "can't remove in empty object vector.");
        return;
    }

    if (vec->size == 0 || i >= vec->size) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "can't remove value at invalid position (%zu >= %zu).", i, vec->size);
        return;
    }

    size_t last = vec->size - 1;

    (void)collect_object(vec->objects[i]);

    vec->objects[i]  = vec->objects[last];
    vec->size--;
}

CN_API void remove_object_ordered_vector(ObjectVector *vec, size_t i)
{
    if (!vec || vec->size == 0) {
        RAISE(ERR_INVALID_POINTER, "can't remove in empty object vector.");
        return;
    }

    if (i >= vec->size) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "can't remove value at invalid position (%zu >= %zu).", i, vec->size);
        return;
    }

    (void)collect_object(vec->objects[i]);

    for (size_t j = i; j < vec->size - 1; ++j)
        vec->objects[j] = vec->objects[j + 1];

    vec->size--;
}
