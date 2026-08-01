#ifndef _ENGINE_MAIN_H_
    #define _ENGINE_MAIN_H_

    #define $template(name) ;

    #include "libcncore.h"

    #if defined(_HAS_GRAPHICS) && (_HAS_GRAPHICS == 1)
        #include "libcngraphic.h"
    #endif

    #if defined(_HAS_GUI) && (_HAS_GUI == 1)
        #include "librgui.h"
    #endif

    #if defined(_HAS_AUDIO) && (_HAS_AUDIO == 1)
        #include "libcnaudio.h"
    #endif

    #if defined(_HAS_INPUT) && (_HAS_INPUT == 1)
        #include "libcninput.h"
    #endif

    #if defined(_HAS_ASSETS) && (_HAS_ASSETS == 1)
        #include "libcnassets.h"
    #endif

    #if defined(_WIN32)
        #include <windows.h>
    #endif

    #include <signal.h>

    typedef enum {
        FDS_NONE,
        FDS_INFO,
        FDS_WARNING,
        FDS_ERROR
    } FEEDBACK_STATUS_TYPE;

    uint8_t load_submodules(Object *ctx); // generated

    #if defined(_HAS_ASSETS) && (_HAS_ASSETS == 1)
        uint8_t register_engine_asset_api(Object *ctx);
        uint8_t load_assets_handler(Object *ctx);
        uint8_t load_entry_scene(Object *ctx);
    #endif

    #if defined(_HAS_GRAPHICS) && (_HAS_GRAPHICS == 1)
        uint8_t add_main_interface(Object *ctx);
    #endif
#endif