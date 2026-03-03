#include "libcncore.h"

void _init_object_attrs(struct attr_map_s *attribute_map)
{
    attribute_map->attrs = NULL;
    attribute_map->keys = NULL;
    attribute_map->size = 0;
    attribute_map->capacity = 0;
}

void _delete_object_attrs(struct attr_map_s *attribute_map)
{
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

void _attr_map_resize(struct attr_map_s *map, size_t new_capacity)
{
    map->attrs = realloc(map->attrs, new_capacity * sizeof(OBJAttrib *));
    map->keys  = realloc(map->keys,  new_capacity * sizeof(uint64_t));
    map->capacity = new_capacity;
}

void _insert_object_attrs(struct attr_map_s *attribute_map, uint64_t k, OBJAttrib *attr)
{
    if  (!attribute_map || !attr)
        return;

    for (size_t i = 0; i < attribute_map->size; ++i) {
        if (attribute_map->keys[i] == k) {
            (void)delete_object_attribute(attribute_map->attrs[i]);
            attribute_map->attrs[i] = attr;
            return;
        }
    }

    if (attribute_map->size >= attribute_map->capacity) {
        size_t new_capacity = attribute_map->capacity == 0 ? 8 : attribute_map->capacity * 2;
        (void)_attr_map_resize(attribute_map, new_capacity);
    }

    attribute_map->keys[attribute_map->size]  = k;
    attribute_map->attrs[attribute_map->size] = attr;
    attribute_map->size++;
}

void _remove_object_attrs(struct attr_map_s *attribute_map, uint64_t k)
{
    if (!attribute_map || attribute_map->size == 0)
        return;

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
}

OBJAttrib *_find_object_attrs(const struct attr_map_s *attribute_map, uint64_t k)
{
    if (!attribute_map)
        return (NULL);

    for (size_t i = 0; i < attribute_map->size; ++i) {
        if  (attribute_map->keys[i] != k)
            continue;
        return (attribute_map->attrs[i]);
    }

    return (NULL);
}

uint64_t _get_attrs_hash(const char *str)
{
    if (!str)
        return 0;

    uint64_t hash = 14695981039346656037ULL;
    const uint64_t prime = 1099511628211ULL;

    while (*str) {
        hash ^= (unsigned char)(*str);
        hash *= prime;
        str++;
    }

    return hash;
}
