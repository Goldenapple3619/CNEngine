#include "libcncore.h"

#include <string.h>
#include <stdlib.h>

CN_API Object *new_object(void)
{
    Object *obj = (Object *)malloc(sizeof(Object));

    if (!obj) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new obj.")
        return (NULL);
    }

    obj->base = NULL;
    obj->ref_count = 0;

    (void)_init_object_attrs(&obj->attrs);
    (void)_init_object_attrs(&obj->methods);

    return (obj);
}

CN_API void delete_object(Object *object)
{
    if (!object) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty object.");
        return;
    }
    if (has_method(object, "_del"))
        (void)call_method(object, "_del", NULL);
    if (object->base) {
        (void)delete_object(object->base);
        object->base = NULL;
    }
    (void)_delete_object_attrs(&object->attrs);
    (void)_delete_object_attrs(&object->methods);
    (void)free(object);
}

CN_API cnbool set_attr(Object *object, const char *name, cn_type type, cnany value)
{
    if (!object) {
        RAISE(ERR_INVALID_POINTER, "can't set_attr with empty object.");
        return (false);
    }

    if (!name) {
        RAISE(ERR_INVALID_POINTER, "can't set_attr with empty name.");
        return (false);
    }

    cn_value *temp_exist = get_attr(object, name);

    if (temp_exist) {
        if (temp_exist->type == CN_TYPE_OBJECT && temp_exist->as.ptr == value)
            return (true);
        if (temp_exist->type == CN_TYPE_STRING && temp_exist->as.ptr == value)
            return (true);
        (void)_delete_object_attribute_value(temp_exist);
        temp_exist->type = type;
        (void)_init_attribute_value(temp_exist, value);
        return (true);
    }

    uint64_t hash = _get_attrs_hash(name);
    OBJAttrib *attr = create_object_attribute(name, type, value);

    if (!attr) {
        PROPAGATE_ERR();
        return (false);
    }

    if (_insert_object_attrs(&object->attrs, hash, attr)) {
        PROPAGATE_ERR();
        return (false);
    }
    return (true);
}

CN_API cn_value *get_attr(const Object *object, const char *name)
{
    if (!object) {
        RAISE(ERR_INVALID_POINTER, "can't get_attr with empty object.");
        return (NULL);
    }

    if (!name) {
        RAISE(ERR_INVALID_POINTER, "can't get_attr with empty name.");
        return (NULL);
    }
    
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
    if (!object) {
        RAISE(ERR_INVALID_POINTER, "can't has_attr with empty object.");
        return (false);
    }

    if (!name) {
        RAISE(ERR_INVALID_POINTER, "can't has_attr with empty name.");
        return (false);
    }
    
    uint64_t hash = _get_attrs_hash(name);
    const Object *temp = object;

    while (temp) {
        if (_find_object_attrs(&temp->attrs, hash))
            return (true);

        temp = temp->base;
    }

    return (false);
}

CN_API cnbool set_method(Object *object, const char *name, cn_method func)
{
    if (!object) {
        RAISE(ERR_INVALID_POINTER, "can't set_method with empty object.");
        return (false);
    }

    if (!name) {
        RAISE(ERR_INVALID_POINTER, "can't set_method with empty name.");
        return (false);
    }

    if (!func) {
        RAISE(ERR_INVALID_POINTER, "can't set_method with empty name.");
        return (false);
    }

    cn_value *temp_exist = get_method_holder(object, name);

    if (temp_exist) {
        (void)_delete_object_attribute_value(temp_exist);
        temp_exist->type = CN_TYPE_FUNCTION;
        (void)_init_attribute_value(temp_exist, (cnany)func);
        return (true);
    }

    uint64_t hash = _get_attrs_hash(name);

    cn_value temp;

    temp.type = CN_TYPE_FUNCTION;
    (void)_init_attribute_value(&temp, (cnany)func);

    OBJAttrib *attr = create_object_attribute_from_cnvalue(name, &temp);

    if (!attr) {
        PROPAGATE_ERR();
        return (false);
    }

    if (_insert_object_attrs(&object->methods, hash, attr)) {
        PROPAGATE_ERR();
        return (false);
    }
    return (true);
}

CN_API cn_value *get_method_holder(const Object *object, const char *name)
{
    if (!object) {
        RAISE(ERR_INVALID_POINTER, "can't get_method_holder with empty object.");
        return (NULL);
    }

    if (!name) {
        RAISE(ERR_INVALID_POINTER, "can't get_method_holder with empty name.");
        return (NULL);
    }
    
    uint64_t hash = _get_attrs_hash(name);
    OBJAttrib *found;

    found = _find_object_attrs(&object->methods, hash);

    if (found)
        return (&found->value);

    return (NULL);
}

CN_API cn_method get_method(const Object *object, const char *name)
{
    if (!object) {
        RAISE(ERR_INVALID_POINTER, "can't get_method with empty object.");
        return (NULL);
    }

    if (!name) {
        RAISE(ERR_INVALID_POINTER, "can't get_method with empty name.");
        return (NULL);
    }
    
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

CN_API cn_value call_method(Object *object, const char *name, void **args)
{
    if (!object) {
        RAISE(ERR_INVALID_POINTER, "can't get_method with empty object.");
        return (VALUE_NULL);
    }

    if (!name) {
        RAISE(ERR_INVALID_POINTER, "can't get_method with empty name.");
        return (VALUE_NULL);
    }

    if (!has_method(object, name)) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "can't call non existent method '%s'.", name);
        return  (VALUE_NULL);
    }
    return (get_method(object, name))(object, args);
}

CN_API cnbool has_method(const Object *object, const char *name)
{
    if (!object) {
        RAISE(ERR_INVALID_POINTER, "can't has_method with empty object.");
        return (false);
    }

    if (!name) {
        RAISE(ERR_INVALID_POINTER, "can't has_method with empty name.");
        return (false);
    }
    
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
    if (!object) {
        RAISE(ERR_INVALID_POINTER, "can't print_object with empty object.");
        return;
    }

    if (!has_method(object, "_str")) {
        RAISE(ERR_NOT_COMPATIBLE, "can't print_object with object that as no _str method.");
        return;
    }

    cn_value val = call_method((Object *)object, "_str", NULL);

    if (val.type != CN_TYPE_STRING || !val.as.str) {
        RAISE(ERR_INVALID_POINTER, "can't print invalid _str return value, check previous trace for potential errors.");
        (void)_delete_object_attribute_value(&val);
        return;
    }

    (void)printf("%s", val.as.str);
    (void)_delete_object_attribute_value(&val);
}

CN_API Object *build_object(Object *obj, void **args)
{
    if (!obj) {
        RAISE(ERR_INVALID_POINTER, "can't build empty object.");
        return (NULL);
    }
    cn_value val = call_method(obj, "_init", args);

    if (val.type == CN_TYPE_NULL) {
        RAISE(ERR_INVALID_TYPE, "object constructor returned null, should be OK or ERR.");
        (void)delete_object(obj);
        return (NULL);
    }
    if (val.as.i == VALUE_OK.as.i)
        return (obj);
    PROPAGATE_ERR();
    (void)delete_object(obj);
    return (NULL);
};

static cn_value _init(Object *__this, void **args) { (void)args; (void)__this; return (VALUE_NULL); }
static cn_value _del(Object *__this, void **args) { (void)args; (void)__this; return (VALUE_NULL); }
static cn_value _str(Object *this, void **args) {
    (void)args;

    cn_value result;
    result.type = CN_TYPE_STRING;
    result.as.str = NULL;

    if (!this) {
        RAISE(ERR_OUT_OF_MEMORY, "I am not real.")
        return (VALUE_NULL);
    }

    cn_value *name_val = get_attr(this, "name");
    const char *name = "object";

    if (name_val && name_val->type == CN_TYPE_STRING && name_val->as.str)
        name = name_val->as.str;

    int needed = snprintf(NULL, 0, "<%s@%p>", name, (void *)this);
    char *str = malloc(needed + 1);

    if (!str) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate string.")
        return (VALUE_NULL);
    }

    snprintf(str, needed + 1, "<%s@%p>", name, (void *)this);

    result.as.str = str;
    return (result);
}

CN_API Object *create_default_object(void)
{
    Object *obj = new_object();

    if (!obj) {
        PROPAGATE_ERR();
        return (NULL);
    }

    if (!set_attr(obj, "name", CN_TYPE_STRING, "object")) {
        PROPAGATE_ERR();
        (void)delete_object(obj);
        return (NULL);
    }
    
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_str", &_str);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    return (obj);
}
