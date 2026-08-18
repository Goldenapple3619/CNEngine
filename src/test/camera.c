#include "test.h"
#include <SDL2/SDL.h>

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
        ) {
        PROPAGATE_ERR();
        return (NULL);
    }

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
        ) {
        PROPAGATE_ERR();
        return (NULL);
    }

    return (camera);
}
