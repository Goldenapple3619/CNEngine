#include "libcncore.h"

void _delete_object_attribute_value(cn_value *val)
{
    if (!val)
        return;
    switch (val->type) {
        case (CN_TYPE_INT):
        case (CN_TYPE_FLOAT):
        case (CN_TYPE_FUNCTION):
        case (CN_TYPE_GENERIC_UNIQ_PTR):
            break;

        case (CN_TYPE_STRING):
            if (val->as.str)
                (void)free(val->as.str);
            val->as.str = NULL;
            break;

        case (CN_TYPE_OBJECT):
            if (val->as.ptr) {
                (void)release_object(val->as.ptr);

                if (((Object *)val->as.ptr)->ref_count <= 0)
                    (void)delete_object(val->as.ptr);
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
        case (CN_TYPE_INT):
            dest->as.i = value ? *((typeof(dest->as.i) *)value) : 0;
            break;
        case (CN_TYPE_FLOAT):
            dest->as.f = value ? *((typeof(dest->as.f) *)value) : 0;
            break;
        case (CN_TYPE_FUNCTION):
            dest->as.ptr = value;
            break;
        case (CN_TYPE_OBJECT):
            dest->as.ptr = share_object((Object *)value);
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
