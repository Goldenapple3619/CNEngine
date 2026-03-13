#include "libcncore.h"

CN_API ObjectVector *new_object_vector(void)
{
    ObjectVector *vec = (ObjectVector *)malloc(sizeof(ObjectVector));

    if (!vec)
        return (NULL);
    vec->capacity = 0;
    vec->size = 0;
    vec->objects = NULL;
    return (vec);
}

CN_API void delete_object_vector(ObjectVector *vec)
{
    if (!vec)
        return;
    for (size_t i = 0; i < vec->size; ++i) {
        (void)release_object(vec->objects[i]);

        if (((Object *)vec->objects[i])->ref_count <= 0)
            (void)delete_object(vec->objects[i]);
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
    if (!vec)
        return (1);

    vec->objects = realloc(vec->objects, new_capacity * sizeof(Object *));

    if (!vec->objects) {
        vec->capacity = 0;
        return (1);
    }

    vec->capacity = new_capacity;

    return (0);
}

CN_API uint8_t insert_object_vector(ObjectVector *vec, Object *obj)
{
    if (!vec || !obj)
        return (1);

    if (vec->size >= vec->capacity) {
        size_t new_capacity = vec->capacity == 0 ? 8 : vec->capacity * 2;
        if (resize_object_vector(vec, new_capacity))
            return (1);
    }

    vec->objects[vec->size] = obj;
    vec->size++;
    (void)share_object(obj);
    return (0);
}

void remove_object_vector(ObjectVector *vec, size_t i)
{
    if (!vec || vec->size == 0 || i >= vec->size)
        return;

    size_t last = vec->size - 1;

    (void)release_object(vec->objects[i]);

    if (((Object *)vec->objects[i])->ref_count <= 0)
        (void)delete_object(vec->objects[i]);

    vec->objects[i]  = vec->objects[last];
    vec->size--;
}
