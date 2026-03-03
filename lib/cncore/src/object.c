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

CN_API void set_attr(Object *object, const char *name, const cn_value *value)
{
    if (object || !name || !value)
        return;

    uint64_t hash = _get_attrs_hash(name);
    OBJAttrib *attr = create_object_attribute_from_cnvalue(name, value);

    if (!attr)
        return;

    (void)_insert_object_attrs(&object->attrs, hash, attr);
}

CN_API cn_value *get_attr(const Object *object, const char *name)
{
    if (!object || !name)
        return (NULL);
    
    uint64_t hash = _get_attrs_hash(name);
    const Object *temp = object;
    OBJAttrib *found;

    while (temp) {
        found = _find_object_attrs(&temp->attrs, hash);

        if (found)
            return (&found->value);

        temp = temp->base;
    }

    return (NULL);
}

CN_API cnbool has_attr(const Object *object, const char *name)
{
    if (!object || !name)
        return (false);
    
    uint64_t hash = _get_attrs_hash(name);
    const Object *temp = object;

    while (temp) {
        if (_find_object_attrs(&temp->attrs, hash))
            return (true);

        temp = temp->base;
    }

    return (false);
}

CN_API void set_method(Object *object, const char *name, cnany func)
{
    if (object || !name || !func)
        return;

    uint64_t hash = _get_attrs_hash(name);

    cn_value temp;

    temp.type = CN_TYPE_FUNCTION;
    _init_attribute_value(&temp, func);

    OBJAttrib *attr = create_object_attribute_from_cnvalue(name, &temp);

    if (!attr)
        return;

    (void)_insert_object_attrs(&object->methods, hash, attr);
}

CN_API cnany get_method(const Object *object, const char *name)
{
    if (!object || !name)
        return (NULL);
    
    uint64_t hash = _get_attrs_hash(name);
    const Object *temp = object;
    OBJAttrib *found;

    while (temp) {
        found = _find_object_attrs(&temp->methods, hash);

        if (found)
            return (&found->value.as.ptr);

        temp = temp->base;
    }

    return (NULL);
}

CN_API cnbool has_method(const Object *object, const char *name)
{
    if (!object || !name)
        return (false);
    
    uint64_t hash = _get_attrs_hash(name);
    const Object *temp = object;

    while (temp) {
        if (_find_object_attrs(&temp->methods, hash))
            return (true);

        temp = temp->base;
    }

    return (false);
}
