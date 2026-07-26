#include "engine_main.h"

uint8_t add_main_interface(Object *ctx)
{
    Videomode v = (Videomode){
        .size.x = 800, .size.y = 600,
        .position.x = (SDL_WINDOWPOS_CENTERED), .position.y = (SDL_WINDOWPOS_CENTERED),
        .flags = VDM_CLOSABLE | VDM_OPENGL | VDM_ACCELERATION,
        .native_flags = VDM_N_SHWN | VDM_N_RSZL | VDM_N_OPENGL
    };
    cn_value val = call_method(ctx, "spawn_interface", PACK_ARG("test window", NULL, &v));

    if (val.type == CN_TYPE_NULL) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}
