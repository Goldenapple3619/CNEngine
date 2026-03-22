#include "libcncore.h"

CN_API struct cn_value_vector_s *new_value_vector(void)
{
    struct cn_value_vector_s *vec = (struct cn_value_vector_s *)malloc(sizeof(struct cn_value_vector_s));

    if (!vec)
        return (NULL);
    vec->size = 0;
    vec->capacity = 0;
    vec->values = NULL;
    return (vec);
}



CN_API void delete_value_vector(struct cn_value_vector_s *vec)
{
    if (!vec)
        return;
    for (size_t i = 0; i < vec->size; ++i) {
        (void)_delete_object_attribute_value(vec->values[i]);
        (void)free(vec->values[i]);
    }
    if (vec->values) {
        (void)free(vec->values);
        vec->values = NULL;
    }
    vec->size = 0;
    vec->capacity = 0;
    (void)free(vec);
}

CN_API uint8_t resize_value_vector(struct cn_value_vector_s *vec, size_t new_capacity)
{
    if (!vec)
        return (1);

    vec->values = realloc(vec->values, new_capacity * sizeof(cn_value *));

    if (!vec->values) {
        vec->capacity = 0;
        return (1);
    }

    vec->capacity = new_capacity;

    return (0);
}

CN_API uint8_t insert_value_vector(struct cn_value_vector_s *vec, cn_value value)
{
    if (!vec)
        return (1);

    cn_value *temp = malloc(sizeof(cn_value));

    if (!temp)
        return (1);

    temp->type = value.type;

    (void)_init_attribute_value(temp, _attribute_value_extract(&value));

    if (vec->size >= vec->capacity) {
        size_t new_capacity = vec->capacity == 0 ? 8 : vec->capacity * 2;
        if (resize_value_vector(vec, new_capacity)) {
            (void)_delete_object_attribute_value(temp);
            (void)free(temp);
            return (1);
        }
    }

    vec->values[vec->size] = temp;
    vec->size++;
    return (0);
}

CN_API void remove_value_vector(struct cn_value_vector_s *vec, size_t i)
{
    if (!vec || vec->size == 0 || i >= vec->size)
        return;

    size_t last = vec->size - 1;

    (void)_delete_object_attribute_value(vec->values[i]);
    (void)free(vec->values[i]);

    vec->values[i]  = vec->values[last];
    vec->size--;
}

CN_API void remove_value_ordered_vector(struct cn_value_vector_s *vec, size_t i)
{
    if (!vec || vec->size == 0 || i >= vec->size)
        return;

    (void)_delete_object_attribute_value(vec->values[i]);
    (void)free(vec->values[i]);

    for (size_t j = i; j < vec->size - 1; ++j)
        vec->values[j] = vec->values[j + 1];

    vec->size--;
}
