#include "libcncore.h"

void _init_object_attrs(struct attr_map_s *attribute_map)
{
    if (!attribute_map) {
        RAISE(ERR_INVALID_POINTER, "can't init empty attribute map.");
        return;
    }
    attribute_map->attrs = NULL;
    attribute_map->keys = NULL;
    attribute_map->size = 0;
    attribute_map->capacity = 0;
}

void _delete_object_attrs(struct attr_map_s *attribute_map)
{
    if (!attribute_map) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty attribute map.");
        return;
    }
    for (size_t i = 0; i < attribute_map->size; ++i) {
        (void)delete_object_attribute(attribute_map->attrs[i]);
        attribute_map->attrs[i] = NULL;
    }
    if (attribute_map->attrs) {
        (void)free((void *)attribute_map->attrs);
        attribute_map->attrs = NULL;
    }
    if (attribute_map->keys) {
        (void)free((void *)attribute_map->keys);
        attribute_map->keys = NULL;
    }
    attribute_map->size = 0;
    attribute_map->capacity = 0;
}

uint8_t _attr_map_resize(struct attr_map_s *map, size_t new_capacity)
{
    if (!map) {
        RAISE(ERR_INVALID_POINTER, "can't resize empty attribute map.");
        return (1);
    }

    map->attrs = realloc(map->attrs, new_capacity * sizeof(OBJAttrib *));
    map->keys  = realloc(map->keys,  new_capacity * sizeof(uint64_t));

    if (!map->attrs || !map->keys) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to resize attribute map.");
        map->size = 0;
        map->capacity = 0;
        return (1);
    }

    map->capacity = new_capacity;
    return (0);
}

uint8_t _insert_object_attrs(struct attr_map_s *attribute_map, uint64_t k, OBJAttrib *attr)
{
    if  (!attribute_map) {
        RAISE(ERR_INVALID_POINTER, "can't insert on empty map.");
        return (1);
    }

    if (!attr) {
        RAISE(ERR_INVALID_POINTER, "can't insert empty attr.");
        return (1);
    }

    for (size_t i = 0; i < attribute_map->size; ++i) {
        if (attribute_map->keys[i] == k) {
            (void)delete_object_attribute(attribute_map->attrs[i]);
            attribute_map->attrs[i] = attr;
            return (0);
        }
    }

    if (attribute_map->size >= attribute_map->capacity) {
        size_t new_capacity = attribute_map->capacity == 0 ? 8 : attribute_map->capacity * 2;
        if (_attr_map_resize(attribute_map, new_capacity)) {
            PROPAGATE_ERR()
            return (1);
        }
    }

    attribute_map->keys[attribute_map->size]  = k;
    attribute_map->attrs[attribute_map->size] = attr;
    attribute_map->size++;
    return (0);
}

void _remove_object_attrs(struct attr_map_s *attribute_map, uint64_t k)
{
    if (!attribute_map) {
        RAISE(ERR_INVALID_POINTER, "can't remove on empty map.");
        return;
    }

    if (attribute_map->size == 0) {
        RAISE(ERR_OUT_OF_BOUND, "can't remove at invalid position.");
        return;
    }

    for (size_t i = 0; i < attribute_map->size; ++i) {
        if (attribute_map->keys[i] == k) {
            size_t last = attribute_map->size - 1;

            (void)delete_object_attribute(attribute_map->attrs[i]);

            attribute_map->keys[i]  = attribute_map->keys[last];
            attribute_map->attrs[i] = attribute_map->attrs[last];

            attribute_map->size--;
            return;
        }
    }

    RAISE(ERR_OUT_OF_BOUND, "can't remove at invalid position.");
}

OBJAttrib *_find_object_attrs(const struct attr_map_s *attribute_map, uint64_t k)
{
    if (!attribute_map) {
        RAISE(ERR_INVALID_POINTER, "can't find on empty map.");
        return (NULL);
    }

    for (size_t i = 0; i < attribute_map->size; ++i) {
        if  (attribute_map->keys[i] != k)
            continue;
        return (attribute_map->attrs[i]);
    }

    return (NULL);
}

CN_API uint64_t _get_attrs_hash(const char *str)
{
    if (!str) {
        RAISE(ERR_INVALID_POINTER, "can't hash on empty str.");
        return 0;
    }

    uint64_t hash = 14695981039346656037ULL;
    const uint64_t prime = 1099511628211ULL;

    while (*str) {
        hash ^= (unsigned char)(*str);
        hash *= prime;
        str++;
    }

    return hash;
}
