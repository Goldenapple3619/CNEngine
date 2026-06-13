#include "build.h"

uint8_t build_get_args(size_t argc, char **argv, struct build_args_s *args)
{
    if (!args)
        return (0);
    args->output_file = strdup("output.cno");
    if (!args->output_file) {
        fprintf(stderr, "failed to allocate string while parsing args.\n");
        return (1);
    }
    args->input_files.capacity = 0;
    args->input_files.size = 0;
    args->padding = ENGINE_WRT_ALIGN8_FLAG;
    args->endian = ENGINE_WRT_BIG_ENDIAN;

    for (size_t i = 0; i < argc; ++i) {
        if (strstr(argv[i], "--align=") == argv[i]) {
            switch (atoi(argv[i] + strlen("--align="))) {
                case 0:
                case 1:
                    args->padding = ENGINE_WRT_ALIGN1_FLAG;
                    break;
                case 4:
                    args->padding = ENGINE_WRT_ALIGN4_FLAG;
                    break;
                case 8:
                    args->padding = ENGINE_WRT_ALIGN8_FLAG;
                    break;
                case 16:
                    args->padding = ENGINE_WRT_ALIGN16_FLAG;
                    break;
                case 64:
                    args->padding = ENGINE_WRT_ALIGN64_FLAG;
                    break;
                case 4096:
                    args->padding = ENGINE_WRT_ALIGN4096_FLAG;
                    break;
                default:
                    fprintf(stderr, "invalid alignement specified '%s'.\n", argv[i] + strlen("--align="));
                    return (1);
            }

            continue;
        }

        if (strstr(argv[i], "--endian=") == argv[i]) {
            if (!strcmp(argv[i] + strlen("--endian="), "big"))
                args->endian = ENGINE_WRT_BIG_ENDIAN;
            else if (!strcmp(argv[i] + strlen("--endian="), "little"))
                args->endian = ENGINE_WRT_LITTLE_ENDIAN;
            else {
                fprintf(stderr, "invalid endian provided '%s'.\n", argv[i] + strlen("--endian="));
                return (1);
            }

            continue;
        }

        if (!strcmp(argv[i], "-o")) {
            ++i;
            if (args->output_file)
                (void)free(args->output_file);
            args->output_file = strdup(argv[i]);

            if (!args->output_file) {
                fprintf(stderr, "failed to allocate string while parsing args.\n");
                return (1);
            }

            continue;
        }

        if (insert_generic_vector(&args->input_files, argv[i])) {
            fprintf(stderr, "failed to allocate vector while parsing args.\n");
            return (1);
        }
    }

    return (0);
}

void reset_args(struct build_args_s *args)
{
    if (args->output_file)
        (void)free(args->output_file);
    if (args->input_files.content)
        (void)free(args->input_files.content);
    args->endian = ENGINE_WRT_BIG_ENDIAN;
    args->padding = ENGINE_WRT_ALIGN8_FLAG;
    args->output_file = NULL;
    args->input_files.content = NULL;
    args->input_files.capacity = 0;
    args->input_files.size = 0;
}

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
