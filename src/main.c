#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <signal.h>
#include "libcncore.h"
#include "libcngraphic.h"
#include "librgui.h"
#include "libr2d.h"

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
Object *add_home_window(Object *ctx)
{
    Videomode v = (Videomode){
        .size.x = 800, .size.y = 600,
        .position.x = (SDL_WINDOWPOS_CENTERED), .position.y = (SDL_WINDOWPOS_CENTERED),
        .flags = VDM_CLOSABLE,
        .native_flags = VDM_N_SHWN | VDM_N_RSZL
    };
    cn_value val = call_method(ctx, "spawn_interface", (cnany []){"test", NULL, &v});

    if (val.type == CN_TYPE_NULL) {
        return (NULL);
    }

    Object *interface = val.as.ptr;

    Object *gui_board = build_object(new_guiboard(), (cnany []){
        &(struct gui_board_mode_s){
            .position = (Vector2){.x = 0, .y = 0},
            .resolution = ((Window *)get_attr(interface, "window")->as.ptr)->video_mode.size,
            .upscale = (Vector2){.x = -1, .y = -1},
            .gpu_mode = false,
            .flags = FLAG_RGUI_DYNAMIC_RESOLUTION
        },
        NULL
    });

    if (!gui_board) {
        return (NULL);
    }

    if (call_method(interface, "add_element", (cnany []){gui_board, NULL}).as.i == VALUE_ERR.as.i) {
        delete_object(gui_board);
        return (NULL);
    }

    Object *fps_text = build_object(new_text(), (cnany []){
        &(struct text_mode_s){
            .position = (Vector2){.x = 5, .y = 5},
            .color = 0xffffffff,
            .font_location = "./assets/fonts/ConsolaMono-Book.ttf",
            .size = 12,
            .text = "fps: 0.00"
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

    return (interface);
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

    // if (call_method(get_attr(ctx, "scene")->as.ptr, "add_element",
    //         (cnany []){
    //             build_object(new_tile(), (cnany[]){&(struct scene_object_mode_s){
    //                 .coords = (Vector3){.x = 16, .y = 16, .z = 0},
    //                 .flags = CN_OBJ_DRAWABLE | CN_OBJ_HOST,
    //                 .rotation = (Rect){.x = 0, .y = 0, .w = 0, .h = 0},
    //                 .scale = (Vector3){.x = 1, .y = 1, .z = 1}
    //             }, NULL}),
    //             NULL
    //         }).as.i == VALUE_ERR.as.i) {
    //     delete_object(ctx);
    //     return (1);
    // }

    // Object *twod_board = build_object(new_2dboard(), (cnany []){
    //     &(struct twod_board_mode_s){
    //         .position = (Vector2){.x = 100, .y = 100},
    //         .resolution = (Vector2){.x = 50, .y = 50},
    //         .upscale = (Vector2){.x = 300, .y = 300},

    //         .scene = get_attr(ctx, "scene")->as.ptr,
    //         .gpu_mode = true
    //     },
    //     NULL
    // });

    // if (!twod_board) {
    //     delete_object(ctx);
    //     return (1);
    // }

    // if (call_method(val.as.ptr, "add_element", (cnany []){twod_board, NULL}).as.i == VALUE_ERR.as.i) {
    //     delete_object(twod_board);
    //     delete_object(ctx);
    //     return (1);
    // }


    call_method(ctx, "_run", NULL);
    delete_object(ctx);

    return (0);
}