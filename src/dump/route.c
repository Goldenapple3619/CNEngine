#include "dump.h"
#include "libcnassets.h"

int dump(size_t argc, char **argv)
{
     if (argc < 3) {
        fprintf(stderr, "%s: file to dump missing.", argv[0]);
        return (1);
    }

    CNAssetReader *reader = new_object_file_reader();

    if (!reader) {
        fprintf(stderr, "%s: failed to allocate reader.", argv[0]);
        return (1);
    }

    if (init_object_file_reader(reader, argv[2])) {
        fprintf(stderr, "%s: %s: failed open and map file.", argv[0], argv[2]);
        delete_object_file_reader(reader);
        return (1);
    }
    if (object_file_reader_read_header(reader)) {
        fprintf(stderr, "%s: %s: failed to parse header.", argv[0], argv[2]);
        delete_object_file_reader(reader);
        return (1);
    }
    if (object_file_reader_read_section_header(reader)) {
        fprintf(stderr, "%s: %s: failed to parse section header.", argv[0], argv[2]);
        delete_object_file_reader(reader);
        return (1);
    }

    print_object_file(reader);
    delete_object_file_reader(reader);

    return (0);
}
