#include "librgui.h"

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()

    if (!args || !args[0])
        return (VALUE_ERR);

    struct container_mode_s *container_mode = args[0];

    if (call_method(__this->base, "_init", PACK_ARG(&container_mode->parent_mode)).as.i == VALUE_ERR.as.i)
        return (VALUE_ERR);

    switch (container_mode->background_type) {
        case GUI_BG_COLOR:
            INIT_CUSTOM_ALLOCATION(__this, new_texture(&container_mode->size, true), delete_texture, "texture");
            clear_texture(get_attr(__this, "texture")->as.ptr, container_mode->background.background_color);
            break;
        case GUI_BG_IMAGE:
            INIT_CUSTOM_ALLOCATION(__this, copy_texture((Texture *)container_mode->background.background_image), delete_texture, "texture");
            resize_texture(get_attr(__this, "texture")->as.ptr, &container_mode->size);
            break;
        default:
            break;
    } 

    INIT_VEC2(__this, container_mode->size, "size");

    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;

    PREP_DEL()

    DEL_CUSTOM_ALLOCAION(__this, delete_texture, "texture");

    return (null_value);
}

CN_API Object *new_container(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, new_guiobject());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}
