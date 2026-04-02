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

static void _cpu_rendering(const Vector2 *position,
    const Vector2 *scale, const Rect *bounding, const Texture *object_texture, Texture *cpu_texture)
{
    if ((scale->x == 1.0) && (scale->y == 1.0))
        blit(object_texture, cpu_texture, bounding, &(Vector2){.x = position->x, .y = position->y});
    else
        blit_ratio(object_texture, cpu_texture, bounding, &(Vector2){.x = position->x, .y = position->y}, &(Vector2){.x = scale->x, .y = scale->y});
}

static void _gpu_rendering(const Vector2 *position,
    const Vector2 *scale, const Rect *bounding, const Vector3 *rotation, Texture *object_texture, gpu_rendering_data *rendering_data)
{
    draw_texture(
        object_texture, rendering_data->renderer,
        bounding,
        &(Vector2){
            .x = rendering_data->canva_off.x + position->x * rendering_data->canva_ratio.x,
            .y = rendering_data->canva_off.y + position->y * rendering_data->canva_ratio.y
        },
        &(Vector2){
            .x = scale->x * rendering_data->canva_ratio.x,
            .y = scale->y * rendering_data->canva_ratio.y
        },
        rotation->z
    );
}

static void _opengl_rendering(void)
{
    // not implemented for now
    return;
}

static cn_value _render_object(Object *__this, void **args)
{
    if (!args && !args[0])
        return (null_value);

    twod_render_stack *render_stack = args[0]; 
    int64_t flags = get_attr(render_stack->obj, "_flags")->as.i;
    
    if (!((flags & CN_OBJ_DRAWABLE) > 0))
        return (null_value);

    Vector3 *object_position = &get_attr(render_stack->obj, "position")->as.vec3;
    Vector3 *object_scale = &get_attr(render_stack->obj, "scale")->as.vec3;
    Vector3 *object_rotation = &get_attr(render_stack->obj, "rotation")->as.vec3;
    Rect *object_texture_bounding = has_attr(render_stack->obj, "texture_bounding") ?
        &get_attr(render_stack->obj, "texture_bounding")->as.rect : NULL;

    if (has_attr(render_stack->obj, "texture")) {
        Texture *temp_texture = get_attr(render_stack->obj, "texture")->as.ptr;

        if (((render_stack->window->video_mode.flags & VDM_CPU) > 0)) {
            Texture *cpu_texture = render_stack->cpu_texture ? render_stack->cpu_texture : get_attr(__this, "texture")->as.ptr;

            _cpu_rendering(
                &(Vector2){object_position->x, object_position->y},
                &(Vector2){object_scale->x, object_scale->y},
                object_texture_bounding,
                temp_texture, cpu_texture
            );
        } else if ((render_stack->window->video_mode.flags & VDM_GPU) > 0) {
            gpu_rendering_data gpu_data = {
                .canva_off = render_stack->canva_position,
                .canva_ratio = render_stack->canva_ratio,
                .canva_scale = render_stack->canva_scale,
                .renderer = render_stack->window->renderer
            };

            _gpu_rendering(
                &(Vector2){object_position->x, object_position->y},
                &(Vector2){object_scale->x,object_scale->y},
                object_texture_bounding,
                object_rotation, temp_texture, &gpu_data
            );
        } else if ((render_stack->window->video_mode.flags & VDM_OPENGL) > 0) {
            _opengl_rendering();
        }
    }

    if (has_method(render_stack->obj, "_draw"))
        (void)call_method(render_stack->obj, "_draw", (cnany []){__this, render_stack->window, NULL});

    return (null_value);
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
    Vector2 computed_upscale = (Vector2){upscale->x / resolution->x, upscale->y / resolution->y};
    Object *scene = get_attr(__this, "scene")->as.ptr;
    Object *elements = get_attr(scene, "objects")->as.ptr;
    size_t len = call_method(elements, "len", NULL).as.i;
    cn_value val;

    for (size_t i = 0; i < len; ++i) {
        val = call_method(elements, "at", (cnany []){(size_t []){i}, NULL});

        if (val.type == CN_TYPE_NULL)
            continue;
        
        _render_object(__this, (cnany []){&(twod_render_stack){
            .obj = val.as.ptr,
            .window = window,
            .cpu_texture = texture,

            .canva_position.x = position->x,
            .canva_position.y = position->y,

            .canva_scale.x = upscale->x,
            .canva_scale.y = upscale->y,

            .canva_size.x = resolution->x,
            .canva_size.y = resolution->y,

            .canva_ratio.x = computed_upscale.x,
            .canva_ratio.y = computed_upscale.y
        }, NULL});
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
    CREATE_METHOD_CLASS_BUILD(obj, "render_object", &_render_object);
    CREATE_METHOD_CLASS_BUILD(obj, "_draw", &_draw);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}