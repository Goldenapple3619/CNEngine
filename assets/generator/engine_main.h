#ifndef _ENGINE_MAIN_H_
    #define _ENGINE_MAIN_H_

    #ifndef _ENGINE_HAS_GRAPHICS
        #define _ENGINE_HAS_GRAPHICS 1
    #endif

    #ifndef _ENGINE_HAS_GUI
        #define _ENGINE_HAS_GUI 1
    #endif

    #if defined(_ENGINE_HAS_GRAPHICS) && (_ENGINE_HAS_GRAPHICS == 1)
        #include "libcngraphic.h"

        #if defined(_ENGINE_HAS_GUI) && (_ENGINE_HAS_GUI == 1)
            #include "librgui.h"
        #endif
    #endif

    #if defined(_WIN32)
        #include <windows.h>
    #endif

    #include "libcncore.h"
    #include <signal.h>

    typedef enum {
        FDS_NONE,
        FDS_INFO,
        FDS_WARNING,
        FDS_ERROR
    } FEEDBACK_STATUS_TYPE;

    uint8_t load_submodules(Object *ctx);
#endif