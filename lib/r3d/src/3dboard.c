#include "libr3d.h"
#include "math.h"

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()

    if (!args || !args[0])
        return (VALUE_ERR);

    struct threed_board_mode_s *mode = args[0];

    Vector2 upscale = (Vector2){
        .x = (mode->upscale.x != -1 ? mode->upscale.x : mode->resolution.x),
        .y = (mode->upscale.y != -1 ? mode->upscale.y : mode->resolution.y),
    };

    INIT_VEC2(__this, mode->position, "position");
    INIT_VEC2(__this, mode->resolution, "resolution");
    INIT_VEC2(__this, upscale, "upscale");

    // teporary testing purpose
    INIT_OBJECT_STATIC(__this, new_camera3d(),
        ((cnany []){
            &(struct scene_object_mode_s){{0, 0, 5}, {1, 1, 1}, {0, 0, 0}, CN_OBJ_HOST | CN_OBJ_DRAWABLE},
            NULL
        })
    , "camera")
    INIT_OBJECT_SHR(__this, mode->scene, "scene");

    return (VALUE_OK);
}

static cn_value _draw(Object *__this, void **args)
{
    Window *window = args[0];

    cnnumber view[16], proj[16], model[16];
    Object *scene = get_attr(__this, "scene")->as.ptr;
    Object *elements = get_attr(scene, "objects")->as.ptr;
    size_t len = call_method(elements, "len", NULL).as.i;
    Vector2 upscale = get_attr(__this, "upscale")->as.vec2;
    Vector2 resolution = get_attr(__this, "resolution")->as.vec2;
    Vector2 position = get_attr(__this, "position")->as.vec2;
    cn_value val;
    Object *temp;
    int64_t flags;
    Vector3 *temp_position;
    Vector3 *temp_scale;
    Vector3 *temp_rotation;
    Mesh *temp_mesh;
    Material *temp_mat;
    cnbool have_camera = has_attr(__this, "camera");
    Object *camera = NULL;

    (void)upscale;

    glViewport(position.x, position.y, resolution.x, resolution.y);

    if (have_camera) {
        camera = get_attr(__this, "camera")->as.ptr;

        if (camera) {
            Vector3 *cam_pos = &get_attr(camera, "position")->as.vec3;
            Vector3    *cam_rot = &get_attr(camera, "rotation")->as.vec3;
            cnnumber    fov     =  get_attr(camera, "fov")->as.num;
            cnnumber    near    =  get_attr(camera, "near")->as.num;
            cnnumber    far     =  get_attr(camera, "far")->as.num;
            cnnumber    aspect  =  resolution.x / resolution.y;

            _make_view(view, cam_pos, cam_rot);
            _make_proj(proj, fov, aspect, near, far);
        }
    }

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
        temp_rotation = &get_attr(temp, "rotation")->as.vec3;

        if (!have_camera || !camera)
            continue;

        if (has_attr(temp, "mesh") && has_attr(temp, "material")) {
            temp_mesh = get_attr(temp, "mesh")->as.ptr;
            temp_mat = get_attr(temp, "material")->as.ptr;

            if (!temp_mesh || !temp_mat)
                continue;

            if (!temp_mesh->uploaded) {
                if (!mesh_upload_gl(temp_mesh))
                    continue;
            }

            _make_model(model, temp_position, temp_scale, temp_rotation);

            material_use_gl(temp_mat, model, view, proj);
            mesh_draw_gl(temp_mesh);
        }

        if (has_method(temp, "_draw"))
            (void)call_method(temp, "_draw", (cnany []){__this, window, NULL});
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

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_draw", &_draw);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}