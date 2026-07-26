#include "engine_main.h"

uint8_t load_submodules(Object *ctx)
{
    #if defined(_HAS_GRAPHICS) && (_HAS_GRAPHICS == 1)
        if (!submodule_ctx(ctx, new_graphic_submodule())) {
            PROPAGATE_ERR();
            return (1);
        }
    #endif

    #if defined(_HAS_GUI) && (_HAS_GUI == 1)
        if (!submodule_ctx(ctx, new_gui_submodule())) {
            PROPAGATE_ERR();
            return (1);
        }
    #endif

    #if defined(_HAS_INPUT) && (_HAS_INPUT == 1)
        if (!submodule_ctx(ctx, new_input_submodule())) {
            PROPAGATE_ERR();
            return (1);
        }
    #endif

    #if defined(_HAS_AUDIO) && (_HAS_AUDIO == 1)
        if (!submodule_ctx(ctx, new_audio_ctx(true))) {
            PROPAGATE_ERR();
            return (1);
        }
    #endif

    #if defined(_HAS_ASSETS) && (_HAS_ASSETS == 1)
        if (!submodule_ctx(ctx, new_asset_submodule())) {
            PROPAGATE_ERR();
            return (1);
        }
    #endif

    return (0);
}