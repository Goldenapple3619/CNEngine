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
    if (object_texture->api == R_API_NONE)
        object_texture->api = R_API_SDL;
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

    Vector3 camera_position_center = {render_stack->camera_position.x - render_stack->canva_size.x / 2,
        render_stack->camera_position.y - render_stack->canva_size.y / 2,
        render_stack->camera_position.z};
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
                &(Vector2){object_position->x - camera_position_center.x, object_position->y - camera_position_center.y},
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
                &(Vector2){object_position->x - camera_position_center.x, object_position->y - camera_position_center.y},
                &(Vector2){object_scale->x,object_scale->y},
                object_texture_bounding,
                &(Vector3){object_rotation->x, object_rotation->y, object_rotation->z + render_stack->camera_rotation.z}, temp_texture, &gpu_data
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

    twod_render_stack render_stack;
    Object *camera = NULL;
    Object *scene = get_attr(__this, "scene")->as.ptr;
    Object *elements = get_attr(scene, "objects")->as.ptr;
    size_t len = call_method(elements, "len", NULL).as.i;
    cnbool have_camera = has_attr(__this, "camera");
    cn_value val;

    render_stack.window = args[0];
    render_stack.canva_scale = get_attr(__this, "upscale")->as.vec2;
    render_stack.canva_size = get_attr(__this, "resolution")->as.vec2;
    render_stack.canva_position = get_attr(__this, "position")->as.vec2;
    render_stack.canva_ratio = (Vector2){render_stack.canva_scale.x / render_stack.canva_size.x, render_stack.canva_scale.y / render_stack.canva_size.y};
    render_stack.cpu_texture = get_attr(__this, "texture")->as.ptr;

    if (((render_stack.window->video_mode.flags & VDM_CPU) > 0))
        clear_texture(render_stack.cpu_texture, 0x000000ff);

    if (have_camera) {
        camera = get_attr(__this, "camera")->as.ptr;

        if (camera) {
            render_stack.camera_position = get_attr(camera, "position")->as.vec3;
            render_stack.camera_rotation = get_attr(camera, "rotation")->as.vec3;
        }
    }

    for (size_t i = 0; i < len; ++i) {
        val = call_method(elements, "at", (cnany []){(size_t []){i}, NULL});

        if (val.type == CN_TYPE_NULL)
            continue;

        if (!have_camera || !camera)
            continue;

        render_stack.obj = val.as.ptr;
        
        _render_object(__this, (cnany []){&render_stack, NULL});
    }

    if (((render_stack.window->video_mode.flags & VDM_CPU) > 0))
        blit_ratio(render_stack.cpu_texture, render_stack.window->texture, NULL, &render_stack.canva_position, &render_stack.canva_ratio);

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