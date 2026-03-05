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

CN_API void set_attr(Object *object, const char *name, cn_type type, cnany value)
{
    if (!object || !name || !value)
        return;

    uint64_t hash = _get_attrs_hash(name);
    OBJAttrib *attr = create_object_attribute(name, type, value);

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

CN_API void set_method(Object *object, const char *name, cn_method func)
{
    if (!object || !name || !func)
        return;

    uint64_t hash = _get_attrs_hash(name);

    cn_value temp;

    temp.type = CN_TYPE_FUNCTION;
    (void)_init_attribute_value(&temp, (cnany)func);

    OBJAttrib *attr = create_object_attribute_from_cnvalue(name, &temp);

    if (!attr)
        return;

    (void)_insert_object_attrs(&object->methods, hash, attr);
}

CN_API cn_method get_method(const Object *object, const char *name)
{
    if (!object || !name)
        return (NULL);
    
    uint64_t hash = _get_attrs_hash(name);
    const Object *temp = object;
    OBJAttrib *found;

    while (temp) {
        found = _find_object_attrs(&temp->methods, hash);

        if (found)
            return ((cn_method)found->value.as.ptr);

        temp = temp->base;
    }

    return (NULL);
}

CN_API cn_value call_method(Object *object, const char *name, void *args)
{
    if (!object || !name)
        return (null_value);
    if (!has_method(object, name))
        return  (null_value);
    return (get_method(object, name))(object, args);
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

CN_API void print_object(const Object *object)
{
    if (!object || !has_attr(object, "_str"))
        return;

    cn_value val = call_method((Object *)object, "_str", NULL);

    if (val.type != CN_TYPE_STRING || !val.as.str) {
        (void)_delete_object_attribute_value(&val);
        return;
    }

    (void)printf("%s", val.as.str);
    (void)_delete_object_attribute_value(&val);
}

static cn_value _init(Object *__this, void *args) { (void)args; (void)__this; return (null_value); }
static cn_value _del(Object *__this, void *args) { (void)args; (void)__this; return (null_value); }
static cn_value _str(Object *this, void *args) {
    (void)args;

    cn_value result;
    result.type = CN_TYPE_STRING;
    result.as.str = NULL;

    if (!this)
        return (null_value);

    cn_value *name_val = get_attr(this, "name");
    const char *name = "object";

    if (name_val && name_val->type == CN_TYPE_STRING && name_val->as.str)
        name = name_val->as.str;

    int needed = snprintf(NULL, 0, "<%s@%p>", name, (void *)this);
    char *str = malloc(needed + 1);

    if (!str)
        return (null_value);

    snprintf(str, needed + 1, "<%s@%p>", name, (void *)this);

    result.as.str = str;
    return (result);
}

CN_API Object *create_default_object(void)
{
    Object *obj = new_object();

    set_attr(obj, "name", CN_TYPE_STRING, "object");
    
    set_method(obj, "_str", _str);
    set_method(obj, "_del", _del);
    set_method(obj, "_init", _init);

    return (obj);
}
