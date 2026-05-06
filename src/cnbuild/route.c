#include "build.h"

int build(size_t argc, char **argv)
{
    size_t i = 0;

    if (argc < 3) {
        fprintf(stderr, "%s: asset build toolchain missing.", argv[0]);
        return (1);
    }

    while ((*(build_types + i)).name) {
        if (!strcmp((*(build_types + i)).name, argv[2]))
            return ((*(build_types + i)).callback(argc, argv));
        ++i;
    };

    fprintf(stderr, "%s: invalid build toolchain '%s'.", argv[0], argv[2]);

    return (1);
}
