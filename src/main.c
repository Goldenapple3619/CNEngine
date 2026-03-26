#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <signal.h>
#include "libcncore.h"
#include "libcngraphic.h"
#include "librgui.h"
#include "libr2d.h"

static Object *global_ctx = NULL;

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

void sigint_handler(int signum)
{
    (void)signum;

    if (global_ctx && has_method(global_ctx, "_stop"))
        call_method(global_ctx, "_stop", NULL);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Videomode v = {.size.x = 800, .size.y = 600, .position.x = 0, .position.y = 0, .flags = VDM_CLOSABLE, .native_flags = VDM_N_SHWN};
    Object *ctx = new_ctx();

    if (!ctx)
        return (1);

    global_ctx = ctx;

    if (!submodule_ctx(ctx, new_graphic_submodule()))
        return (1);

    cn_value val = call_method(ctx, "_init", NULL);

    if (val.type == CN_TYPE_NULL || val.as.i == VALUE_ERR.as.i) {
        delete_object(ctx);
        return (1);
    }

    start_gui();
    
    val = call_method(ctx, "spawn_interface", (cnany []){"test", NULL, &v});

    if (val.type == CN_TYPE_NULL) {
        delete_object(ctx);
        return (1);
    }

    if (call_method(get_attr(ctx, "scene")->as.ptr, "add_element",
            (cnany []){
                build_object(new_tile(), (cnany[]){&(struct scene_object_mode_s){
                    .coords = (Vector3){.x = 16, .y = 16, .z = 0},
                    .flags = CN_OBJ_DRAWABLE | CN_OBJ_HOST,
                    .rotation = (Rect){.x = 0, .y = 0, .w = 0, .h = 0},
                    .scale = (Vector3){.x = 1, .y = 1, .z = 1}
                }, NULL}),
                NULL
            }).as.i == VALUE_ERR.as.i) {
        delete_object(ctx);
        return (1);
    }

    Object *twod_board = build_object(new_2dboard(), (cnany []){
        &(struct twod_board_mode_s){
            .position = (Vector2){.x = 0, .y = 0},
            .resolution = v.size,
            .upscale = (Vector2){.x = -1, .y = -1},

            .scene = get_attr(ctx, "scene")->as.ptr
        },
        NULL
    });

    if (!twod_board) {
        delete_object(ctx);
        return (1);
    }

    if (call_method(val.as.ptr, "add_element", (cnany []){twod_board, NULL}).as.i == VALUE_ERR.as.i) {
        delete_object(twod_board);
        delete_object(ctx);
        return (1);
    }

    Object *gui_board = build_object(new_guiboard(), (cnany []){
        &(struct gui_board_mode_s){
            .position = (Vector2){.x = 0, .y = 0},
            .resolution = v.size,
            .upscale = (Vector2){.x = -1, .y = -1}
        },
        NULL
    });

    if (!gui_board) {
        delete_object(ctx);
        return (1);
    }

    if (call_method(val.as.ptr, "add_element", (cnany []){gui_board, NULL}).as.i == VALUE_ERR.as.i) {
        delete_object(gui_board);
        delete_object(ctx);
        return (1);
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
        delete_object(ctx);
        return (1);
    }

    if (call_method(gui_board, "add_element", (cnany []){fps_text, NULL}).as.i == VALUE_ERR.as.i) {
        delete_object(fps_text);
        delete_object(ctx);
        return (1);
    }

    set_method(fps_text, "_update", fps_update);

    signal(SIGINT, &sigint_handler);
    call_method(ctx, "_run", NULL);
    delete_object(ctx);
    end_gui();
    return (0);
}