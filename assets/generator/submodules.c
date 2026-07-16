#include "engine_main.h"

uint8_t load_submodules(Object *ctx)
{
    #if defined(_ENGINE_HAS_GRAPHICS) && (_ENGINE_HAS_GRAPHICS == 1)
        if (!submodule_ctx(ctx, new_graphic_submodule())) {
            PROPAGATE_ERR();
            return (1);
        }
    #endif

    #if defined(_ENGINE_HAS_GUI) && (_ENGINE_HAS_GUI == 1)
        if (!submodule_ctx(ctx, new_gui_submodule())) {
            PROPAGATE_ERR();
            return (1);
        }
    #endif

    #if defined(_ENGINE_HAS_INPUT) && (_ENGINE_HAS_INPUT == 1)
        if (!submodule_ctx(ctx, new_input_submodule())) {
            PROPAGATE_ERR();
            return (1);
        }
    #endif

    #if defined(_ENGINE_HAS_AUDIO) && (_ENGINE_HAS_AUDIO == 1)
        if (!submodule_ctx(ctx, new_audio_ctx(true))) {
            PROPAGATE_ERR();
            return (1);
        }
    #endif

    #if defined(_ENGINE_HAS_ASSETS) && (_ENGINE_HAS_ASSETS == 1)
        if (!submodule_ctx(ctx, new_asset_submodule())) {
            PROPAGATE_ERR();
            return (1);
        }
    #endif
    

    return (0);
}