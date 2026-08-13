#include "help.h"

#ifdef WIN32
    #define EL "\r\n"
#else
    #define EL "\n"
#endif

static const char help_help[] = ""

"%s help <command>"                                     EL
;

static const char help_init[] = ""

"%s init"                                               EL
                                                        EL
"Arguments:"                                            EL
"   -D <path>              set init directory"          EL

;

static const char help_dump[] = ""

"%s dump <object file>"                                 EL

;

static const char help_build[] = ""

"%s build <toolchain> <project file> ..."               EL
                                                        EL
"Toolchains:"                                           EL
"    gui, obj, scn, proj, asset, lnk"                   EL
                                                        EL
"Arguments:"                                            EL
                                                        EL
"    --align=n             set file alignement"         EL
"    --endian=(big/little) set file endianness"         EL
"    -o <name>             set output name"             EL

;

int help(size_t argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (argc > 2) {
        if (!strcmp(argv[2], "help")) {
            printf(help_help, argv[0]);
            return (0);
        }
        if (!strcmp(argv[2], "init")) {
            printf(help_init, argv[0]);
            return (0);
        }
        if (!strcmp(argv[2], "build")) {
            printf(help_build, argv[0]);
            return (0);
        }
        if (!strcmp(argv[2], "dump")) {
            printf(help_dump, argv[0]);
            return (0);
        }
        printf("help not found for command '%s'.", argv[2]);
        return (1);
    }

    printf(""

"%s <command>"                                          EL
                                                        EL
"Usage:"                                                EL
"    cnengine help  <command>"                          EL
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
                                                        EL
"Init:"                                                 EL
"   -D <path>              set init directory"          EL
                                                        EL
"Dump:"                                                 EL
"   path/to/the/object_file"                            EL
    , argv[0]);

    return (0);
}
