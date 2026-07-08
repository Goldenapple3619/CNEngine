#include "libr3d.h"
#include "math.h"

static cn_value _init(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't init 3d board with no 3d board mode.")
        return (VALUE_ERR);
    }

    PREP_INIT()

    struct threed_board_mode_s *mode = args[0];

    Vector2 upscale = (Vector2){
        .x = (mode->upscale.x != -1 ? mode->upscale.x : mode->resolution.x),
        .y = (mode->upscale.y != -1 ? mode->upscale.y : mode->resolution.y),
    };

    INIT_VEC2(__this, mode->position, "position");
    INIT_VEC2(__this, mode->resolution, "resolution");
    INIT_VEC2(__this, upscale, "upscale");

    INIT_OBJECT_STATIC(__this, new_camera3d(),
        (PACK_ARG(
            &(struct scene_object_mode_s){{0, 0, 5}, {1, 1, 1}, {0, 0, 0}, CN_OBJ_HOST | CN_OBJ_DRAWABLE}
        ))
    , "camera")
    INIT_OBJECT_SHR(__this, mode->scene, "scene");

    return (VALUE_OK);
}
static void _cpu_rendering(void)
{
    // not implemented
}

static void _gpu_rendering(void)
{
    // not implemented
}

static void _opengl_rendering(Mesh *mesh, Material *material, const Vector3 *position, const Vector3 *scale, const Vector3 *rotation, const cnnumber view[16], const cnnumber proj[16])
{
    cnnumber model[16];

    if (!mesh->uploaded) {
        if (!mesh_upload_gl(mesh))
            return;
    }

    _make_model(model, position, scale, rotation);

    material_use_gl(material, model, view, proj);
    mesh_draw_gl(mesh);
}

static cn_value _render_object(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't draw with no object.");
        return (null_value);
    }

    threed_render_stack *render_stack = args[0]; 
    int64_t flags = get_attr(render_stack->obj, "_flags")->as.i;
    
    if (!((flags & CN_OBJ_DRAWABLE) > 0))
        return (null_value);

    Vector3 *object_position = &get_attr(render_stack->obj, "position")->as.vec3;
    Vector3 *object_scale = &get_attr(render_stack->obj, "scale")->as.vec3;
    Vector3 *object_rotation = &get_attr(render_stack->obj, "rotation")->as.vec3;

    if (has_attr(render_stack->obj, "mesh") && has_attr(render_stack->obj, "material")) {
        Mesh *object_mesh = get_attr(render_stack->obj, "mesh")->as.ptr;
        Material *object_mat = get_attr(render_stack->obj, "material")->as.ptr;

        if (!object_mesh || !object_mat)
            return (null_value);

        if (((render_stack->window->video_mode.flags & VDM_CPU) > 0)) {
            _cpu_rendering();
        } else if ((render_stack->window->video_mode.flags & VDM_GPU) > 0) {
            _gpu_rendering();
        } else if ((render_stack->window->video_mode.flags & VDM_OPENGL) > 0) {
            _opengl_rendering(
                object_mesh,
                object_mat,
                object_position,
                object_scale,
                object_rotation,
                render_stack->view,
                render_stack->proj);
        }
    }

    if (has_method(render_stack->obj, "_draw"))
        (void)call_method(render_stack->obj, "_draw", PACK_ARG(__this, render_stack->window));

    return (null_value);
}

static cn_value _draw(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't draw with no window.");
        return (null_value);
    }

    threed_render_stack render_stack;

    Object *scene = get_attr(__this, "scene")->as.ptr;
    Object *elements = get_attr(scene, "objects")->as.ptr;
    cnbool have_camera = has_attr(__this, "camera");
    Object *camera = NULL;

    render_stack.window = args[0];
    render_stack.canva_scale = get_attr(__this, "upscale")->as.vec2;
    render_stack.canva_size = get_attr(__this, "resolution")->as.vec2;
    render_stack.canva_position = get_attr(__this, "position")->as.vec2;

    if (((render_stack.window->video_mode.flags & VDM_OPENGL) > 0))
        glViewport(render_stack.canva_position.x, render_stack.window->video_mode.size.y - render_stack.canva_position.y - render_stack.canva_scale.y, render_stack.canva_scale.x, render_stack.canva_scale.y);

    if (have_camera) {
        camera = get_attr(__this, "camera")->as.ptr;

        if (camera) {
            _make_view(render_stack.view,
                &get_attr(camera, "position")->as.vec3, &get_attr(camera, "rotation")->as.vec3);
            _make_proj(render_stack.proj,
                get_attr(camera, "fov")->as.num,
                render_stack.canva_size.x / render_stack.canva_size.y,
                get_attr(camera, "near")->as.num, get_attr(camera, "far")->as.num);
        }
    }

    for (struct list_iterator_s it = list_get_iterator(elements); !list_iterator_isend(&it); list_iterator_next(&it)) {
        if (list_iterator_value_isnull(&it))
            continue;

        if (!have_camera || !camera)
            continue;
        
        render_stack.obj = it.val.as.ptr;

        _render_object(__this, (cnany[]){&render_stack, NULL});
    }

    return (null_value);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;
    (void)__this;

    return (null_value);
}

CN_API Object *new_3dboard(void)
{
    Object *obj = new_object();

    if (!obj) {
        PROPAGATE_ERR();
        return (NULL);
    }

    SET_PARENT_CLASS_BUILD_STATIC(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_draw", &_draw);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}