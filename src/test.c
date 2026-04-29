#include "libcncore.h"
#include "libcngraphic.h"
#include "libr3d.h"
#include "librgui.h"
#include "libcninput.h"

static cn_value _move_left(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    if (!(*(cnbool *)args[0]))
        get_attr(__this, "velocity")->as.vec3.x = -0.01;
    else if (get_attr(__this, "velocity")->as.vec3.x < 0)
        get_attr(__this, "velocity")->as.vec3.x = 0;
    return (null_value);
}

static cn_value _move_right(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    if (!(*(cnbool *)args[0]))
        get_attr(__this, "velocity")->as.vec3.x = 0.01;
    else if (get_attr(__this, "velocity")->as.vec3.x > 0)
        get_attr(__this, "velocity")->as.vec3.x = 0;
    return (null_value);
}

static cn_value _move_forward(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    if (!(*(cnbool *)args[0]))
        get_attr(__this, "velocity")->as.vec3.z = -0.01;
    else if (get_attr(__this, "velocity")->as.vec3.z < 0)
        get_attr(__this, "velocity")->as.vec3.z = 0;
    return (null_value);
}

static cn_value _move_backward(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    if (!(*(cnbool *)args[0])) {
        get_attr(__this, "velocity")->as.vec3.z = 0.01;
    } else if (get_attr(__this, "velocity")->as.vec3.z > 0) {
        get_attr(__this, "velocity")->as.vec3.z = 0;
    }
    return (null_value);
}

Object *create_threed_view(Object *ctx, Vector2 position, Vector2 size)
{
    Object *threed_board = build_object(new_3dboard(), PACK_ARG(
        &(struct threed_board_mode_s){
            .position = position,
            .resolution = size,
            .upscale = (Vector2){.x = -1, .y = -1},
            .scene = get_attr(ctx, "scene")->as.ptr
        },
        NULL
    ));

    if (!threed_board) {
        return (NULL);
    }

    return (threed_board);
}

Object *set_up_camera(Object *ctx, Object *board)
{
    Object *camera = get_attr(board, "camera")->as.ptr;

    if (
        call_method(ctx, "register_input_controller", PACK_ARG(
            "left", &(InputController){.ignore_value = false, .target_type = INPUT_KEY_PRESS, .target_value = SDLK_q}
        )).as.i == VALUE_ERR.as.i ||
        call_method(ctx, "register_input_controller", PACK_ARG(
            "right", &(InputController){.ignore_value = false, .target_type = INPUT_KEY_PRESS, .target_value = SDLK_d}
        )).as.i == VALUE_ERR.as.i ||
        call_method(ctx, "register_input_controller", PACK_ARG(
            "forward", &(InputController){.ignore_value = false, .target_type = INPUT_KEY_PRESS, .target_value = SDLK_z}
        )).as.i == VALUE_ERR.as.i ||
        call_method(ctx, "register_input_controller", PACK_ARG(
            "backward", &(InputController){.ignore_value = false, .target_type = INPUT_KEY_PRESS, .target_value = SDLK_s}
        )).as.i == VALUE_ERR.as.i
        )
        return (NULL);

    if (
        call_method(ctx, "register_input_callback", PACK_ARG(
            "left", &(ObjMethodPair){.obj = camera, .method = &_move_left}
        )).as.i == VALUE_ERR.as.i ||
        call_method(ctx, "register_input_callback", PACK_ARG(
            "right", &(ObjMethodPair){.obj = camera, .method = &_move_right}
        )).as.i == VALUE_ERR.as.i ||
        call_method(ctx, "register_input_callback", PACK_ARG(
            "forward", &(ObjMethodPair){.obj = camera, .method = &_move_forward}
        )).as.i == VALUE_ERR.as.i ||
        call_method(ctx, "register_input_callback", PACK_ARG(
            "backward", &(ObjMethodPair){.obj = camera, .method = &_move_backward}
        )).as.i == VALUE_ERR.as.i
        )
        return (NULL);

    return (camera);
}

cn_value show_fps(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    double delta_time = *(double *)args[0];
    char fps_text[23];

    if (delta_time <= 0)
        return (null_value);

    snprintf(fps_text, sizeof(fps_text), "%.2f fps", 1000.0 / (double)delta_time);
    call_method(__this, "set_text", PACK_ARG(fps_text));
    return (null_value);
}

Object *build_hud_test(Vector2 position, Vector2 size)
{
    Object *gui_board = build_object(new_guiboard(), PACK_ARG(&(struct gui_board_mode_s){.position = position, .resolution = size, .upscale = (Vector2){-1, -1}, .flags = FLAG_RGUI_DYNAMIC_RESOLUTION}));
    Object *fps_text = build_object(
        new_text(),
        PACK_ARG(
            &(struct text_mode_s){.color = 0xffffffff, .font_location = "assets/fonts/ConsolaMono-Book.ttf", .font_size = 16, .text = "hello world",
                .parent_mode = {.align = GUI_ALIGN_RIGHT, .position = {.x = 15, .y = 0}, .rotation = 0, .scale = {.x = 1, .y = 1}, .zindex = 0}
            }
        )
    );

    CREATE_METHOD_CLASS_BUILD(fps_text, "_update", &show_fps);

    if (call_method(gui_board, "add_element", PACK_ARG(
        build_object(
            new_text(),
            PACK_ARG(
                &(struct text_mode_s){.color = 0xff0000ff, .font_location = "assets/fonts/ConsolaMono-Book.ttf", .font_size = 16, .text = "hello world",
                    .parent_mode = {.align = GUI_ALIGN_MIDDLE, .position = {.x = 0, .y = 0}, .rotation = 0, .scale = {.x = 1, .y = 1}, .zindex = 0}
                }
            )
        ), NULL
    )).as.i == VALUE_ERR.as.i)
        return (NULL);


    if (call_method(gui_board, "add_element", PACK_ARG(
        fps_text, NULL
    )).as.i == VALUE_ERR.as.i)
        return (NULL);

    return (gui_board);
}

Object *add_test_window(Object *ctx)
{
    Videomode v = (Videomode){
        .size.x = 800, .size.y = 600,
        .position.x = (SDL_WINDOWPOS_CENTERED), .position.y = (SDL_WINDOWPOS_CENTERED),
        .flags = VDM_CLOSABLE | VDM_OPENGL | VDM_ACCELERATION | VDM_VSYNC,
        .native_flags = VDM_N_SHWN | VDM_N_RSZL | VDM_N_OPENGL
    };
    Texture *tex = new_texture_from_file("./assets/images/logo_XL.png");
    cn_value val = call_method(ctx, "spawn_interface", PACK_ARG("test window", tex, &v));

    delete_texture(tex); // don't worry, it won't cause any use after free, trust

    if (val.type == CN_TYPE_NULL) {
        return (NULL);
    }

    Object *window_interface = val.as.ptr;
    Object *view_threed = create_threed_view(ctx, (Vector2){.x = 0, .y = 0}, (Vector2){.x = v.size.x, .y = v.size.y});
    Object *hud_test = build_hud_test((Vector2){.x = 0, .y = 0}, (Vector2){.x = v.size.x, .y = v.size.y});

    if (call_method(window_interface, "add_element", PACK_ARG(view_threed, NULL)).as.i == VALUE_ERR.as.i) {
        delete_object(view_threed);
        return (NULL);
    }

    if (call_method(window_interface, "add_element", PACK_ARG(hud_test, NULL)).as.i == VALUE_ERR.as.i) {
        delete_object(hud_test);
        return (NULL);
    }

    if (call_method(get_attr(ctx, "scene")->as.ptr, "add_element", PACK_ARG(set_up_camera(ctx, view_threed), NULL)).as.i == VALUE_ERR.as.i) {
        return (NULL);
    }

    Object *obj = build_object(new_object3d(), PACK_ARG(
        &(struct scene_object_mode_s){
            .coords = {0, 0, 0},
            .flags = CN_OBJ_DRAWABLE | CN_OBJ_HOST,
            .rotation = {0, 0, 0},
            .scale = {1, 1, 1}
        }
    ));

    if (!obj) {
        return (NULL);
    }

    if (call_method(get_attr(ctx, "scene")->as.ptr, "add_element", PACK_ARG(obj, NULL)).as.i == VALUE_ERR.as.i) {
        delete_object(obj);
        return (NULL);
    }

    call_method(ctx, "set_main_window", PACK_ARG(INLNE_PRIM_T_ARG(((Window *)get_attr(window_interface, "window")->as.ptr)->id)));

    return (window_interface);
}
