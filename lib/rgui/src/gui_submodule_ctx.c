#include "librgui.h"

static cn_value _init(Object *__this, void **args)
{
    (void)__this;

    if (!args || !(args[0])) {
        RAISE(ERR_INVALID_POINTER, "can't init gui submodule with not ctx.");
        return (VALUE_ERR);
    }
    
    if (!start_gui()) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }
    
    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)__this;

    if (!args || !(args[0]))
        return (VALUE_ERR);

    cn_value *interfaces_ref = get_attr(__this, "interfaces");
    ObjectVector *interfaces;
    ObjectVector *temp;

    if (interfaces_ref && interfaces_ref->as.ptr) {
        interfaces = interfaces_ref->as.ptr;

        for (size_t i = 0; i < interfaces->size; ++i) {
            temp = get_attr(interfaces->objects[i], "elements")->as.ptr;

            for (size_t j = 0; j < temp->size; ++j) {
                if (strcmp(get_attr(temp->objects[j], "name")->as.str, "guiboard"))
                    continue;

                remove_object_ordered_vector(temp, j);
                --j;
            }
        }
    }

    run_gc();
    end_gui();
    
    return (VALUE_OK);
}

CN_API Object *new_gui_submodule(void)
{
    Object *obj = new_object();

    if (!obj) {
        PROPAGATE_ERR();
        return (NULL);
    }

    SET_PARENT_CLASS_BUILD_STATIC(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}
