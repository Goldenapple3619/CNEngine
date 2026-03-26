#include "librgui.h"

CN_API cnbool start_gui(void)
{
    if (TTF_Init() != 0)
        return (false);
    return (true);
}

CN_API void end_gui(void)
{
    (void)TTF_Quit();
}
