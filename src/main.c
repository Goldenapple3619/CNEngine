#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <signal.h>
#include "libcncore.h"
#include "libcngraphic.h"
#include "librgui.h"
#include "libr2d.h"
#include "libr3d.h"

static Object *global_ctx = NULL;

static void sigint_handler(int signum)
{
    (void)signum;

    if (global_ctx && has_method(global_ctx, "_stop"))
        call_method(global_ctx, "_stop", NULL);
}

cn_value fps_update(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    char temp[20];

    (void)memset(temp, 0, sizeof(temp) / sizeof(char));
    (void)snprintf(temp, (sizeof(temp) / sizeof(char)) - sizeof(char), "%.2f", 1000.0 / *(double *)args[0]); 
    (void)call_method(__this, "set_text", (cnany []){temp, NULL});

    return (null_value);
}

Object *add_test_opengl_window(Object *ctx)
{
    Videomode v = (Videomode){
        .size.x = 800, .size.y = 600,
        .position.x = (SDL_WINDOWPOS_CENTERED), .position.y = (SDL_WINDOWPOS_CENTERED),
        .flags = VDM_CLOSABLE | VDM_OPENGL,
        .native_flags = VDM_N_SHWN | VDM_N_RSZL | VDM_N_OPENGL
    };
    cn_value val = call_method(ctx, "spawn_interface", (cnany []){"test", NULL, &v});

    if (val.type == CN_TYPE_NULL) {
        return (NULL);
    }

    Object *window_interface = val.as.ptr;

    Object *threed_board = build_object(new_3dboard(), (cnany []){
        &(struct threed_board_mode_s){
            .position = (Vector2){.x = 0, .y = 0},
            .resolution = ((Window *)get_attr(window_interface, "window")->as.ptr)->video_mode.size,
            .upscale = (Vector2){.x = -1, .y = -1},
            .scene = get_attr(ctx, "scene")->as.ptr
        },
        NULL
    });

    if (!threed_board) {
        return (NULL);
    }

    if (call_method(window_interface, "add_element", (cnany []){threed_board, NULL}).as.i == VALUE_ERR.as.i) {
        delete_object(threed_board);
        return (NULL);
    }

    Object *obj = build_object(new_object3d(), (cnany []){
        &(struct scene_object_mode_s){
            .coords = {0, 0, 0},
            .flags = CN_OBJ_DRAWABLE | CN_OBJ_HOST,
            .rotation = {0, 0, 0},
            .scale = {1, 1, 1}
        }
    });

    if (!obj) {
        return (NULL);
    }

    if (call_method(get_attr(ctx, "scene")->as.ptr, "add_element", (cnany []){obj, NULL}).as.i == VALUE_ERR.as.i) {
        delete_object(obj);
        return (NULL);
    }

    return (window_interface);
}

Object *add_test_twod_window(Object *ctx)
{
    Videomode v = (Videomode){
        .size.x = 800, .size.y = 600,
        .position.x = (SDL_WINDOWPOS_CENTERED), .position.y = (SDL_WINDOWPOS_CENTERED),
        .flags = VDM_CLOSABLE | VDM_GPU,
        .native_flags = VDM_N_SHWN | VDM_N_RSZL
    };
    cn_value val = call_method(ctx, "spawn_interface", (cnany []){"test", NULL, &v});

    if (val.type == CN_TYPE_NULL) {
        return (NULL);
    }

    Object *window_interface = val.as.ptr;

    Object *twod_board = build_object(new_2dboard(), (cnany []){
        &(struct twod_board_mode_s){
            .position = (Vector2){.x = 0, .y = 0},
            .resolution = ((Window *)get_attr(window_interface, "window")->as.ptr)->video_mode.size,
            .upscale = (Vector2){.x = -1, .y = -1},
            .scene = get_attr(ctx, "scene")->as.ptr
        },
        NULL
    });

    if (!twod_board) {
        return (NULL);
    }

    if (call_method(window_interface, "add_element", (cnany []){twod_board, NULL}).as.i == VALUE_ERR.as.i) {
        delete_object(twod_board);
        return (NULL);
    }

    Object *tile = build_object(new_tile(), (cnany []){
        &(struct scene_object_mode_s){
            .coords = {15, 15, 0},
            .flags = CN_OBJ_DRAWABLE | CN_OBJ_HOST,
            .rotation = {0, 0, 0},
            .scale = {2, 2, 1}
        }
    });

    if (!tile)
        return (NULL);

    if (call_method(get_attr(ctx, "scene")->as.ptr, "add_element", (cnany []){tile, NULL}).as.i == VALUE_ERR.as.i) {
        delete_object(tile);
        return (NULL);
    }

    return (window_interface);
}

Object *add_home_window(Object *ctx)
{
    Videomode v = (Videomode){
        .size.x = 800, .size.y = 600,
        .position.x = (SDL_WINDOWPOS_CENTERED), .position.y = (SDL_WINDOWPOS_CENTERED),
        .flags = VDM_CLOSABLE | VDM_CPU,
        .native_flags = VDM_N_SHWN | VDM_N_RSZL
    };
    cn_value val = call_method(ctx, "spawn_interface", (cnany []){"test", NULL, &v});

    if (val.type == CN_TYPE_NULL) {
        return (NULL);
    }

    Object *window_interface = val.as.ptr;

    Object *gui_board = build_object(new_guiboard(), (cnany []){
        &(struct gui_board_mode_s){
            .position = (Vector2){.x = 0, .y = 0},
            .resolution = ((Window *)get_attr(window_interface, "window")->as.ptr)->video_mode.size,
            .upscale = (Vector2){.x = -1, .y = -1},
            .flags = FLAG_RGUI_DYNAMIC_RESOLUTION
        },
        NULL
    });

    if (!gui_board) {
        return (NULL);
    }

    if (call_method(window_interface, "add_element", (cnany []){gui_board, NULL}).as.i == VALUE_ERR.as.i) {
        delete_object(gui_board);
        return (NULL);
    }

    Object *fps_text = build_object(new_text(), (cnany []){
        &(struct text_mode_s){
            .position = (Vector2){.x = 5, .y = 5},
            .color = 0xffffffff,
            .font_location = "./assets/fonts/ConsolaMono-Book.ttf",
            .size = 12,
            .text = "fps: 0.00",
            .align = GUI_ALIGN_RIGHT
        },
        NULL
    });

    if (!fps_text) {
        return (NULL);
    }

    set_method(fps_text, "_update", fps_update);
    if (call_method(gui_board, "add_element", (cnany []){fps_text, NULL}).as.i == VALUE_ERR.as.i) {
        delete_object(fps_text);
        return (NULL);
    }

    return (window_interface);
}


Object *build_engine(void)
{
    Object *ctx = new_ctx();

    if (!ctx)
        return (NULL);

    global_ctx = ctx;

    if (!submodule_ctx(ctx, new_graphic_submodule())) {
        (void)delete_object(ctx);
        return (NULL);
    }

    if (!submodule_ctx(ctx, new_gui_submodule())) {
        (void)delete_object(ctx);
        return (NULL);
    }

    cn_value val = call_method(ctx, "_init", NULL);

    if (val.type == CN_TYPE_NULL || val.as.i == VALUE_ERR.as.i) {
        delete_object(ctx);
        return (NULL);
    }

    signal(SIGINT, &sigint_handler);

    return (ctx);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Object *ctx = build_engine();

    if (!ctx)
        return (1);

    if (!add_home_window(ctx)) {
        delete_object(ctx);
        return (1);
    }
    
    if (!add_test_twod_window(ctx)) {
        delete_object(ctx);
        return (1);
    }

    if (!add_test_opengl_window(ctx)) {
        delete_object(ctx);
        return (1);
    }


    call_method(ctx, "_run", NULL);
    delete_object(ctx);

    return (0);
}