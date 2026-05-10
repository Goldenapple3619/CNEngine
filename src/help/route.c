#include "help.h"

#ifdef WIN32
    #define EL "\r\n"
#else
    #define EL "\n"
#endif

int help(size_t argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf(""

"%s <command>"                          EL
                                        EL
"Usage:"                                EL
"    cnengine help <command>"           EL
"    cnengine init"                     EL
"    cnengine build <project file>"     EL
                                        EL
"Commands:"                             EL
"    help, init, build"                 EL

    , argv[0]);

    return (0);
}
