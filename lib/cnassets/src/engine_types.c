#include "libcnassets.h"

CN_API uint8_t value_from_string(const char *str, cn_type type, cn_value *val)
{
    cn_value temp;

    val->type = type;

    switch (type) {
        case CN_TYPE_VEC2:
            temp.as.vec2.x = (cnnumber)atof(str);
            temp.as.vec2.y = (cnnumber)atof(str);
            _init_attribute_value(val, (cnany)&temp.as.vec2);
            break;

        case CN_TYPE_VEC3:
            temp.as.vec3.x = (cnnumber)atof(str);
            temp.as.vec3.y = (cnnumber)atof(str);
            _init_attribute_value(val, (cnany)&temp.as.vec3);
            break;

        case CN_TYPE_RECT:
            temp.as.rect.x = (cnnumber)atof(str);
            temp.as.rect.y = (cnnumber)atof(str);
            temp.as.rect.w = (cnnumber)atof(str);
            temp.as.rect.h = (cnnumber)atof(str);
            _init_attribute_value(val, (cnany)&temp.as.rect);
            break;

        case CN_TYPE_NULL:
            _init_attribute_value(val, NULL);
            break;

        case CN_TYPE_STRING:
            _init_attribute_value(val, (cnany)str);
            break;

        case CN_TYPE_BOOL:
            if (!strcmp(str, "true")) {
                _init_attribute_value(val, INLNE_PRIM_T_ARG(true));
            } else if (!strcmp(str, "false")) {
                _init_attribute_value(val, INLNE_PRIM_T_ARG(false));
            } else {
                RAISE_FMT(ERR_INVALID_TYPE, "can't parse bool, expected 'true' or 'false', got: '%s'.", str);
                return (1);
            }

            break;
        case CN_TYPE_INT:
            temp.as.i = (int64_t)atoll(str); 
            _init_attribute_value(val, (cnany)&temp.as.i);
            break;

        case CN_TYPE_FLOAT:
            temp.as.f = (double)atof(str);
            _init_attribute_value(val, (cnany)&temp.as.f);
            break;

        case CN_TYPE_NUMBER:
            temp.as.num = (cnnumber)atof(str);
            _init_attribute_value(val, (cnany)&temp.as.num);
            break;

        default:
            RAISE(ERR_INVALID_TYPE, "can't parse from string unsupported type.")
            return (1);
    }

    return (0);
}

CN_API cn_type typename_from_string(const char *str)
{
    if (!strcmp(str, "object"))
        return (CN_TYPE_OBJECT);
    if (!strcmp(str, "wobject"))
        return (CN_TYPE_WEAK_OBJECT);
    if (!strcmp(str, "vec3"))
        return (CN_TYPE_VEC3);
    if (!strcmp(str, "vec2"))
        return (CN_TYPE_VEC2);
    if (!strcmp(str, "str"))
        return (CN_TYPE_STRING);
    if (!strcmp(str, "bool"))
        return (CN_TYPE_BOOL);
    if (!strcmp(str, "function"))
        return (CN_TYPE_FUNCTION);
    if (!strcmp(str, "int"))
        return (CN_TYPE_INT);
    if (!strcmp(str, "float"))
        return (CN_TYPE_FLOAT);
    if (!strcmp(str, "number"))
        return (CN_TYPE_NUMBER);
    if (!strcmp(str, "rect"))
        return (CN_TYPE_RECT);
    if (!strcmp(str, "null"))
        return (CN_TYPE_NULL);

    return (CN_TYPE_GENERIC_UNIQ_PTR);
}