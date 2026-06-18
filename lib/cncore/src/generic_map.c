
#include "libcncore.h"

CN_API struct generic_map_s *new_generic_map(void)
{
    struct generic_map_s *gen_map = (struct generic_map_s *)malloc(sizeof(struct generic_map_s));

    if (!gen_map) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocated generic map");
        return (NULL);
    }

    gen_map->capacity = 0;
    gen_map->size = 0;
    gen_map->keys = NULL;
    gen_map->content = NULL;

    return (gen_map);
}

CN_API uint8_t generic_map_resize(struct generic_map_s *gen_map, size_t new_capacity)
{
    if (!gen_map) {
        RAISE(ERR_INVALID_POINTER, "invalid resize on empty generic map.");
        return (1);
    }

    gen_map->content = realloc(gen_map->content, new_capacity * sizeof(void *));
    gen_map->keys = realloc(gen_map->keys,  new_capacity * sizeof(uint64_t));

    if (!gen_map->content || !gen_map->keys) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to resize generic map.");
        gen_map->size = 0;
        gen_map->capacity = 0;
        return (1);
    }

    gen_map->capacity = new_capacity;
    return (0);
}

CN_API uint8_t add_generic_map(struct generic_map_s *gen_map, void *element, const char *key, void (*_delete_obj)(void *))
{
    if (!gen_map) {
        RAISE(ERR_INVALID_POINTER, "invalid add on empty generic map.");
        return (1);
    }

    if (!key || !element) {
        RAISE(ERR_INVALID_POINTER, "key or element are empty.");
        return (1);
    }

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
        if (generic_map_resize(gen_map, new_capacity)) {
            PROPAGATE_ERR();
            return (1);
        }
    }

    gen_map->keys[gen_map->size]  = k;
    gen_map->content[gen_map->size] = element;
    gen_map->size++;
    return (0);
}

CN_API void remove_generic_map(struct generic_map_s *gen_map, const char *key, void (*_delete_obj)(void *))
{
    if (!gen_map) {
        RAISE(ERR_INVALID_POINTER, "invalid remove on empty generic map.");
        return;
    }

    if (!key || gen_map->size == 0) {
        RAISE(ERR_OUT_OF_BOUND, "remove at invalid key.");
        return;
    }

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

    RAISE(ERR_OUT_OF_BOUND, "remove at invalid key.");
}

CN_API cnbool has_generic_map(struct generic_map_s *gen_map, const char *key)
{
    if (!gen_map) {
        RAISE(ERR_INVALID_POINTER, "can't has empty map.");
        return (false);
    }

    if (!key) {
        RAISE(ERR_INVALID_POINTER, "can't has with empty key.");
        return (false);
    }

    uint64_t k = _get_attrs_hash(key);
    
    for (size_t i = 0; i < gen_map->size; ++i) {
        if (gen_map->keys[i] == k)
            return (true);
    }
    return (false);
}

CN_API void *get_generic_map(struct generic_map_s *gen_map, const char *key, void *(*_obj_from_key_default)(const char *), void (*_delete_obj)(void *))
{
    if (!gen_map) {
        RAISE(ERR_INVALID_POINTER, "can't get empty map.");
        return (NULL);
    }

    if (!key) {
        RAISE(ERR_INVALID_POINTER, "can't get with empty key.");
        return (NULL);
    }


    uint64_t k = _get_attrs_hash(key);
    void *obj = NULL;
    
    for (size_t i = 0; i < gen_map->size; ++i) {
        if (gen_map->keys[i] == k)
            return (gen_map->content[i]);
    }

    if (_obj_from_key_default)
        obj = _obj_from_key_default(key);
    else {
        RAISE_FMT(ERR_OUT_OF_BOUND, "get map on invalid key '%s' with no default.", key);
        return (NULL);
    }

    if (!obj) {
        RAISE(ERR_OUT_OF_BOUND, "default object builder returned empty object.")
        return (NULL);
    }

    if (add_generic_map(gen_map, obj, key, _delete_obj)) {
        PROPAGATE_ERR();
        if (_delete_obj)
            (void)_delete_obj(obj);
        return (NULL);
    }

    return (obj);
}

CN_API void empty_generic_map(struct generic_map_s *gen_map, void (*_delete_obj)(void *))
{
    if (!gen_map) {
        RAISE(ERR_INVALID_POINTER, "empty of an invalid map.");
        return;
    }

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
}

CN_API void delete_generic_map(struct generic_map_s *gen_map, void (*_delete_obj)(void *))
{
    if (!gen_map) {
        RAISE(ERR_INVALID_POINTER, "delete of an invalid map.");
        return;
    }

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
