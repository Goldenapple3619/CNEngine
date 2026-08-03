#include "build.h"

uint8_t build_get_args(size_t argc, char **argv, const char *toolchain, struct build_args_s *args)
{
    if (!args)
        return (0);

    if (!strcmp(toolchain, "project"))
        args->output_file = strdup("./");
    else
        args->output_file = strdup("output.cno");


    if (!args->output_file) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate string while parsing args.");
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
                RAISE(ERR_OUT_OF_MEMORY, "failed to allocate string while parsing args.");
                return (1);
            }

            continue;
        }

        if (insert_generic_vector(&args->input_files, argv[i])) {
            PROPAGATE_ERR();
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

Object *init_asset_ctx(void)
{
    Object *asset_ctx = new_asset_submodule();

    if (!asset_ctx) {
        PROPAGATE_ERR();
        (void)run_gc();
        return (NULL);
    }

    asset_ctx = build_object(asset_ctx, PACK_ARG(asset_ctx));

    if (!asset_ctx) {
        PROPAGATE_ERR();
        (void)run_gc();
        return (NULL);
    }

    #ifdef _WIN32
        if (call_method(asset_ctx, "register_fmt", PACK_ARG("./dist/windows-amd64/lib/libcnguiobj.dll")).as.i == VALUE_ERR.as.i) {
            PROPAGATE_ERR();
            DELOC(asset_ctx);
            return (NULL);
        }

        if (call_method(asset_ctx, "register_fmt", PACK_ARG("./dist/windows-amd64/lib/libcnsceneobj.dll")).as.i == VALUE_ERR.as.i) {
            PROPAGATE_ERR();
            DELOC(asset_ctx);
            return (NULL);
        }

        if (call_method(asset_ctx, "register_fmt", PACK_ARG("./dist/windows-amd64/lib/libcnobjectobj.dll")).as.i == VALUE_ERR.as.i) {
            PROPAGATE_ERR();
            DELOC(asset_ctx);
            return (NULL);
        }
    #else
        if (call_method(asset_ctx, "register_fmt", PACK_ARG("./dist/linux-amd64/lib/libcnguiobj.so")).as.i == VALUE_ERR.as.i) {
            PROPAGATE_ERR();
            DELOC(asset_ctx);
            return (NULL);
        }

        if (call_method(asset_ctx, "register_fmt", PACK_ARG("./dist/linux-amd64/lib/libcnsceneobj.so")).as.i == VALUE_ERR.as.i) {
            PROPAGATE_ERR();
            DELOC(asset_ctx);
            return (NULL);
        }

        if (call_method(asset_ctx, "register_fmt", PACK_ARG("./dist/linux-amd64/lib/libcnobjectobj.so")).as.i == VALUE_ERR.as.i) {
            PROPAGATE_ERR();
            DELOC(asset_ctx);
            return (NULL);
        }
    #endif

    return (asset_ctx);
}

int build(size_t argc, char **argv)
{
    Object *asset_ctx = init_asset_ctx();
    size_t i = 0;
    int ret;

    if (!asset_ctx) {
        PROPAGATE_ERR();
        return (1);
    }

    if (argc < 3) {
        fprintf(stderr, "%s: asset build toolchain missing.", argv[0]);
        DELOC(asset_ctx);
        return (1);
    }

    while ((*(build_types + i)).name) {
        if (!strcmp((*(build_types + i)).name, argv[2])) {
            ret = ((*(build_types + i)).callback(argc, argv, asset_ctx));
            DELOC(asset_ctx);
            (void)xmlCleanupParser();
            return (ret);
        }
        ++i;
    };

    fprintf(stderr, "%s: invalid build toolchain '%s'.", argv[0], argv[2]);
    DELOC(asset_ctx);

    return (1);
}
