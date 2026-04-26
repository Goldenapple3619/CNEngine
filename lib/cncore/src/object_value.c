#include "libcncore.h"

void _delete_object_attribute_value(cn_value *val)
{
    if (!val)
        return;
    switch (val->type) {
        case (CN_TYPE_INT):
        case (CN_TYPE_NUMBER):
        case (CN_TYPE_BOOL):
        case (CN_TYPE_NULL):
        case (CN_TYPE_VEC2):
        case (CN_TYPE_VEC3):
        case (CN_TYPE_RECT):
        case (CN_TYPE_FLOAT):
        case (CN_TYPE_FUNCTION):
        case (CN_TYPE_WEAK_OBJECT):
        case (CN_TYPE_GENERIC_UNIQ_PTR):
            break;

        case (CN_TYPE_STRING):
            if (val->as.str)
                (void)free(val->as.str);
            val->as.str = NULL;
            break;

        case (CN_TYPE_OBJECT):
            if (val->as.ptr) {
                // (void)release_object(val->as.ptr);

                // if (((Object *)val->as.ptr)->ref_count <= 0)
                //     (void)delete_object(val->as.ptr);
                (void)collect_object(val->as.ptr);
            }
            val->as.ptr = NULL;
            break;

        default:
            break;
    }
}

void _init_attribute_value(cn_value *dest, cnany value)
{
    if (!dest)
        return;
    switch (dest->type) {
        case (CN_TYPE_NULL):
            dest->as.i = 0;
            break;
        case (CN_TYPE_BOOL):
            dest->as.b = value ? *((typeof(dest->as.b) *)value) : 0;
            break;
        case (CN_TYPE_INT):
            dest->as.i = value ? *((typeof(dest->as.i) *)value) : 0;
            break;
        case (CN_TYPE_FLOAT):
            dest->as.f = value ? *((typeof(dest->as.f) *)value) : 0;
            break;
        case (CN_TYPE_NUMBER):
            dest->as.num = value ? *((typeof(dest->as.num) *)value) : 0;
            break;
        case (CN_TYPE_VEC2):
            dest->as.vec2 = value ? *((typeof(dest->as.vec2) *)value) : (Vector2){.x = 0, .y = 0};
            break;
        case (CN_TYPE_VEC3):
            dest->as.vec3 = value ? *((typeof(dest->as.vec3) *)value) : (Vector3){.x = 0, .y = 0, .z = 0};
            break;
        case (CN_TYPE_RECT):
            dest->as.rect = value ? *((typeof(dest->as.rect) *)value) : (Rect){.x = 0, .y = 0, .w = 0, .h = 0};
            break;
        case (CN_TYPE_FUNCTION):
            dest->as.ptr = value;
            break;
        case (CN_TYPE_OBJECT):
            dest->as.ptr = share_object((Object *)value);
            break;
        case (CN_TYPE_WEAK_OBJECT):
            dest->as.ptr = value;
            break;
        case (CN_TYPE_GENERIC_UNIQ_PTR):
            dest->as.ptr = value;
            break;
        case (CN_TYPE_STRING):
            dest->as.str = value ? strdup((char *)value) : NULL;;
            break;
        default:
            break;
    }
}

void *_attribute_value_extract(const cn_value *src)
{
    if (!src)
        return (NULL);
    switch (src->type) {
        case (CN_TYPE_NULL):
            return (void *)&src->as.i;
            break;
        case (CN_TYPE_BOOL):
            return (void *)&src->as.b;
            break;
        case (CN_TYPE_VEC2):
            return (void *)&src->as.vec2;
            break;
        case (CN_TYPE_VEC3):
            return (void *)&src->as.vec3;
            break;
        case (CN_TYPE_RECT):
            return (void *)&src->as.rect;
            break;
        case (CN_TYPE_NUMBER):
            return (void *)&src->as.num;
            break;
        case (CN_TYPE_INT):
            return (void *)&src->as.i;
            break;
        case (CN_TYPE_FLOAT):
            return (void *)&src->as.f;
            break;
        case (CN_TYPE_FUNCTION):
            return src->as.ptr;
            break;
        case (CN_TYPE_OBJECT):
            return src->as.ptr;
            break;
        case (CN_TYPE_WEAK_OBJECT):
            return src->as.ptr;
            break;
        case (CN_TYPE_GENERIC_UNIQ_PTR):
            return src->as.ptr;
            break;
        case (CN_TYPE_STRING):
            return src->as.str;
            break;
        default:
            break;
    }
    return (NULL);
}
