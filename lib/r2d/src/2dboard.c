#include "libr2d.h"

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()

    if (!args || !args[0])
        return (VALUE_ERR);

    struct twod_board_mode_s *mode = args[0];

    Vector2 upscale = (Vector2){
        .x = (mode->upscale.x != -1 ? mode->upscale.x : mode->resolution.x),
        .y = (mode->upscale.y != -1 ? mode->upscale.y : mode->resolution.y),
    };

    INIT_VEC2(__this, mode->position, "position");
    INIT_VEC2(__this, mode->resolution, "resolution");
    INIT_VEC2(__this, upscale, "upscale");

    INIT_CUSTOM_ALLOCATION(__this, new_texture(&mode->resolution, true), delete_texture, "texture");
    INIT_OBJECT_SHR(__this, mode->scene, "scene");

    return (VALUE_OK);
}

static cn_value _draw(Object *__this, void **args)
{
    (void)__this;
    (void)args;

    Texture *texture = get_attr(__this, "texture")->as.ptr;

    clear_texture(texture, 0x000000ff);

    // to implement

    return (null_value);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;

    PREP_DEL()

    DEL_CUSTOM_ALLOCAION(__this, delete_texture, "texture")

    return (null_value);
}

CN_API Object *new_2dboard(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_draw", &_draw);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}