#include "engine.h"

int argument_route(size_t argc, char **argv)
{
    size_t i = 0;

    if (argc < 2) {
        fprintf(stderr, "%s: no command specified, use 'cnengine help' to get information about the usage.\n", argv[0]);
        return (1);
    }
    
    while ((*(routes + i)).name) {
        if (!strcmp((*(routes + i)).name, argv[1]))
            return ((*(routes + i)).callback(argc, argv));
        ++i;
    };

    fprintf(stderr, "%s: invalid command '%s', use 'cnengine help' to get a list of valid commands.\n", argv[0], argv[1]);
    return (1);
}
