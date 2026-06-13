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

"%s <command>"                                          EL
                                                        EL
"Usage:"                                                EL
"    cnengine help <command>"                           EL
"    cnengine init"                                     EL
"    cnengine build <toolchain> <project file> ..."     EL
"    cnengine dump  <object file>"                      EL
                                                        EL
"Commands:"                                             EL
"    help, init, build, dump"                           EL
                                                        EL
"Build:"                                                EL
"    gui, obj, scn, proj, asset, lnk"                   EL
                                                        EL
"    --align=n             set file alignement"         EL
"    --endian=(big/little) set file endianness"         EL
"    -o <name>             set output name"             EL
    , argv[0]);

    return (0);
}
