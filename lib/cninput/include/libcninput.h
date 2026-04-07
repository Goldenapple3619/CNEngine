#ifndef _LIBCNINPUT_H_
    #define _LIBCNINPUT_H_

    #include "libcncore.h"
    #include "libcngraphic.h"

    CN_API cnbool start_input(void);
    CN_API void end_input(void);

    CN_API Object *new_input_submodule(void);

#endif
