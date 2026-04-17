
#include "libcncore.h"

CN_API struct generic_map_s *new_generic_map(void)
{
    struct generic_map_s *gen_map = (struct generic_map_s *)malloc(sizeof(struct generic_map_s));

    if (!gen_map)
        return (NULL);

    gen_map->capacity = 0;
    gen_map->size = 0;
    gen_map->keys = NULL;
    gen_map->content = NULL;

    return (gen_map);
}

CN_API uint8_t generic_map_resize(struct generic_map_s *gen_map, size_t new_capacity)
{
    if (!gen_map)
        return (1);

    gen_map->content = realloc(gen_map->content, new_capacity * sizeof(void *));
    gen_map->keys  = realloc(gen_map->keys,  new_capacity * sizeof(uint64_t));

    if (!gen_map->content || !gen_map->keys) {
        gen_map->capacity = 0;
        return (1);
    }

    gen_map->capacity = new_capacity;
    return (0);
}

CN_API uint8_t add_generic_map(struct generic_map_s *gen_map, void *element, const char *key, void (*_delete_obj)(void *))
{
    if (!gen_map || !key || !element)
        return (1);

    uint64_t k = _get_attrs_hash(key);

    for (size_t i = 0; i < gen_map->size; ++i) {
        if (gen_map->keys[i] == k) {
            if (_delete_obj && gen_map->content[i])
                (void)_delete_obj(gen_map->content[i]);
            gen_map->content[i] = element;
            return (0);
        }
    }

    if (gen_map->size >= gen_map->capacity) {
        size_t new_capacity = gen_map->capacity == 0 ? 8 : gen_map->capacity * 2;
        if (generic_map_resize(gen_map, new_capacity))
            return (1);
    }

    gen_map->keys[gen_map->size]  = k;
    gen_map->content[gen_map->size] = element;
    gen_map->size++;
    return (0);
}

CN_API void remove_generic_map(struct generic_map_s *gen_map, const char *key, void (*_delete_obj)(void *))
{
    if (!gen_map || !key || gen_map->size == 0)
        return;

    uint64_t k = _get_attrs_hash(key);

    for (size_t i = 0; i < gen_map->size; ++i) {
        if (gen_map->keys[i] == k) {
            size_t last = gen_map->size - 1;

            if (_delete_obj && gen_map->content[i])
                (void)_delete_obj(gen_map->content[i]);

            gen_map->keys[i]  = gen_map->keys[last];
            gen_map->content[i] = gen_map->content[last];

            gen_map->size--;
            return;
        }
    }
}

CN_API const void *get_generic_map(struct generic_map_s *gen_map, const char *key, void *(*_obj_from_key_default)(const char *), void (*_delete_obj)(void *))
{
    if (!gen_map || !key)
        return (NULL);

    uint64_t k = _get_attrs_hash(key);
    void *obj = NULL;
    
    for (size_t i = 0; i < gen_map->size; ++i) {
        if (gen_map->keys[i] == k)
            return (gen_map->content[i]);
    }

    if (_obj_from_key_default)
        obj = _obj_from_key_default(key);
    else
        return (NULL);

    if (!obj)
        return (NULL);

    if (add_generic_map(gen_map, obj, key, _delete_obj)) {
        if (_delete_obj)
            (void)_delete_obj(obj);
        return (NULL);
    }

    return (obj);
}

CN_API void delete_generic_map(struct generic_map_s *gen_map, void (*_delete_obj)(void *))
{
    if (!gen_map)
        return;

    if (_delete_obj) {
        for (size_t i = 0; i < gen_map->size; ++i) {
            if (gen_map->content[i])
                (void)_delete_obj(gen_map->content[i]);
        }
    }
    if (gen_map->keys) {
        (void)free(gen_map->keys);
        gen_map->keys = NULL;
    }
    if (gen_map->content) {
        (void)free(gen_map->content);
        gen_map->content = NULL;
    }
    gen_map->capacity = 0;
    gen_map->keys = 0;
    (void)free(gen_map);
}
