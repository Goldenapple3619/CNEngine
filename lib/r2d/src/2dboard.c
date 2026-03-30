#include "libr2d.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

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
    Window *window = args[0];

    if (((window->video_mode.flags & VDM_CPU) > 0))
        clear_texture(texture, 0x000000ff);

    Vector2 *upscale = &get_attr(__this, "upscale")->as.vec2;
    Vector2 *resolution = &get_attr(__this, "resolution")->as.vec2;
    Vector2 *position = &get_attr(__this, "position")->as.vec2;
    Object *scene = get_attr(__this, "scene")->as.ptr;
    Object *elements = get_attr(scene, "objects")->as.ptr;
    size_t len = call_method(elements, "len", NULL).as.i;
    Vector2 computed_upscale = (Vector2){upscale->x / resolution->x, upscale->y / resolution->y};
    cn_value val;
    Object *temp;
    int64_t flags;
    Texture *temp_texture;
    Vector3 *temp_position;
    Vector3 *temp_scale;
    Rect *temp_rotation;
    Rect *temp_texture_bounding;

    for (size_t i = 0; i < len; ++i) {
        val = call_method(elements, "at", (cnany []){(size_t []){i}, NULL});

        if (val.type == CN_TYPE_NULL)
            continue;
        
        temp = val.as.ptr;
        flags = get_attr(temp, "_flags")->as.i;
        
        if (!((flags & CN_OBJ_DRAWABLE) > 0))
            continue;

        temp_position = &get_attr(temp, "position")->as.vec3;
        temp_scale = &get_attr(temp, "scale")->as.vec3;
        temp_rotation = &get_attr(temp, "rotation")->as.rect;
        temp_texture_bounding = has_attr(temp, "texture_bounding") ? &get_attr(temp, "texture_bounding")->as.rect : NULL;

        (void)temp_rotation;

        if (has_attr(temp, "texture")) {
            temp_texture = get_attr(temp, "texture")->as.ptr;

            if (((window->video_mode.flags & VDM_CPU) > 0))
                if ((temp_scale->x == 1.0) && (temp_scale->y == 1.0))
                    blit(temp_texture, texture, temp_texture_bounding, &(Vector2){.x = temp_position->x, .y = temp_position->y});
                else
                    blit_ratio(temp_texture, texture, temp_texture_bounding, &(Vector2){.x = temp_position->x, .y = temp_position->y}, &(Vector2){.x = temp_scale->x, .y = temp_scale->y});
            else if ((window->video_mode.flags & VDM_GPU) > 0)
                draw_texture(temp_texture, window->renderer, temp_texture_bounding, &(Vector2){.x = position->x + temp_position->x * computed_upscale.x, .y = position->y + temp_position->y * computed_upscale.y}, &(Vector2){.x = temp_scale->x * computed_upscale.x, .y = temp_scale->y * computed_upscale.y}, 2.0 * atan2((double)temp_rotation->w, (double)temp_rotation->h) * (180.0 / M_PI));
            else if ((window->video_mode.flags & VDM_OPENGL) > 0) {
                // not implemented
                continue;
            }
        }

        if (has_method(temp, "_draw"))
            (void)call_method(temp, "_draw", (cnany []){__this, window, NULL});
    }

    if (((window->video_mode.flags & VDM_CPU) > 0))
        blit_ratio(texture, window->texture, NULL, position, &computed_upscale);

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