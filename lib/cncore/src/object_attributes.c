#include "libcncore.h"

CN_API OBJAttrib *create_object_attribute(const char *name, cn_type type, cnany value)
{
    if (!name) {
        RAISE(ERR_INVALID_POINTER, "can't create attribute with no name.");
        return (NULL);
    }

    OBJAttrib *attribute = (OBJAttrib *)malloc(sizeof(OBJAttrib));

    if (!attribute) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate attribute.");
        return (NULL);
    }
    
    #ifdef STRING_INDIVIDUAL_ALLOCATION
        attribute->name = (char *)strdup(name);

        if (!attribute->name) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate attribute name.");
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

CN_API OBJAttrib *create_object_attribute_from_cnvalue(const char *name, const cn_value *value)
{
    if (!value || !name) {
        RAISE(ERR_INVALID_POINTER, "can't create attribute with no name / no value.");
        return (NULL);
    }

    OBJAttrib *attribute = (OBJAttrib *)malloc(sizeof(OBJAttrib));

    if (!attribute) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate attribute.");
        return (NULL);
    }
    
    #ifdef STRING_INDIVIDUAL_ALLOCATION
        attribute->name = (char *)strdup(name);

        if (!attribute->name) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate attribute name.");
            (void)free(attribute);
            return (NULL);
        }
    #else
        attribute->name = name;
    #endif

    attribute->value.type = value->type;

    (void)_init_attribute_value(&attribute->value, _attribute_value_extract(value));

    return (attribute);
}

CN_API void delete_object_attribute(OBJAttrib *attribute)
{
    if (!attribute) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty attribute.");
        return;
    }
    (void)_delete_object_attribute_value(&attribute->value);
    #ifdef STRING_INDIVIDUAL_ALLOCATION
        if (attribute->name)
            (void)free((void *)attribute->name);
        attribute->name = NULL;
    #endif
    (void)free((void *)attribute);
}
