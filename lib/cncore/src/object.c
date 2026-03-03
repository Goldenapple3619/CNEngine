#include "libcncore.h"

#include <string.h>
#include <stdlib.h>

CN_API Object *new_object(void)
{
    Object *obj = (Object *)malloc(sizeof(Object));

    if (!obj)
        return (NULL);
    obj->base = NULL;
    obj->ref_count = 0;

    (void)_init_object_attrs(&obj->attrs);
    (void)_init_object_attrs(&obj->methods);

    return (obj);
}

CN_API Object *share_object(Object *object)
{
    if (!object)
        return (NULL);
    object->ref_count++;
    return (object);
}

CN_API Object *release_object(Object *object)
{
    if (!object)
        return (NULL);
    object->ref_count--;
    return (object);
}

CN_API void delete_object(Object *object)
{
    if (!object)
        return;
    if (object->base) {
        (void)delete_object(object->base);
        object->base = NULL;
    }
    (void)_delete_object_attrs(&object->attrs);
    (void)_delete_object_attrs(&object->methods);
    (void)free(object);
}

void _delete_object_attribute_value(cn_value *val)
{
    switch (val->type) {
        case (CN_TYPE_INT):
        case (CN_TYPE_FLOAT):
        case (CN_TYPE_FUNCTION):
        case (CN_TYPE_GENERIC_UNIQ_PTR):
            break;

        case (CN_TYPE_STRING):
            if (val->as.str)
                (void)free(val->as.str);
            break;

        case (CN_TYPE_OBJECT):
            if (val->as.ptr)
                (void)delete_object(val->as.ptr);
            break;
    
        default:
            break;
    }
}

void _init_attribute_value(cn_value *dest, cnany value)
{
    switch (dest->type) {
        case (CN_TYPE_INT):
            dest->as.i = value ? *(typeof(dest->as.i) *)value : 0;
            break;
        case (CN_TYPE_FLOAT):
            dest->as.f = value ? *(typeof(dest->as.f) *)value : 0;
            break;
        case (CN_TYPE_FUNCTION):
            dest->as.ptr = value;
            break;
        case (CN_TYPE_OBJECT):
            dest->as.ptr = value;
            break;
        case (CN_TYPE_GENERIC_UNIQ_PTR):
            dest->as.ptr = value;
            break;
        case (CN_TYPE_STRING):
            dest->as.str = (char *)value;
            break;
        default:
            break;
    }
}

CN_API OBJAttrib *create_object_attribute(const char *name, cn_type type, cnany value)
{
    OBJAttrib *attribute = (OBJAttrib *)malloc(sizeof(OBJAttrib));

    if (!attribute)
        return (NULL);
    
    #ifdef STRING_INDIVIDUAL_ALLOCATION
        attribute->name = (char *)strdup(name);

        if (!attribute->name) {
            (void)free(attribute);
            return (NULL);
        }
    #else
        attribute->name = name;
    #endif

    attribute->value.type = type;
    attribute->value.as.f = 0;
    attribute->value.as.i = 0;
    attribute->value.as.str = NULL;
    attribute->value.as.ptr = NULL;

    (void)_init_attribute_value(&attribute->value, value);

    return (attribute);
}

CN_API void delete_object_attribute(OBJAttrib *attribute)
{
    if (!attribute)
        return;
    (void)_delete_object_attribute_value(&attribute->value);
    #ifdef STRING_INDIVIDUAL_ALLOCATION
        if (attribute->name)
            (void)free((void *)attribute->name);
        attribute->name = NULL;
    #endif
    (void)free((void *)attribute);
}

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
