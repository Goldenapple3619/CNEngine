#include "../project_toolchain.h"

GameAssetCompiler *new_asset_compiler(const char *toolchain, const char *build_path, const char *dist_path)
{
    if (!toolchain || !build_path || !dist_path) {
        RAISE(ERR_INVALID_POINTER, "can't allocate new asset compiler from empty toolchain / build_path / dist_path.")
        return (NULL);
    }

    GameAssetCompiler *compiler = malloc(sizeof(GameAssetCompiler));

    if (!compiler) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new asset compiler.");
        return (NULL);
    }

    compiler->alignement = 8;
    compiler->endianness = true;
    compiler->max_bank_size = 2100000000;

    compiler->build_path = strdup(build_path);
    if (!compiler->build_path) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new build_path str.");
        (void)free(compiler);
        return (NULL);
    }

    compiler->dist_path = strdup(dist_path);
    if (!compiler->dist_path) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new dist_path str.");
        (void)free(compiler->build_path);
        (void)free(compiler);
        return (NULL);
    }

    compiler->compiler_path = strdup(toolchain);
    if (!compiler->compiler_path) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new compiler_path str.");
        (void)free(compiler->build_path);
        (void)free(compiler->dist_path);
        (void)free(compiler);
        return (NULL);
    }

    compiler->objs.content = NULL;
    compiler->objs.capacity = 0;
    compiler->objs.size = 0;
    compiler->srcs.content = NULL;
    compiler->srcs.capacity = 0;
    compiler->srcs.size = 0;
    return (compiler);
}

void delete_asset_compiler(GameAssetCompiler *compiler)
{
    if (!compiler) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty asset compiler.");
        return;
    }

    if (compiler->build_path)
        (void)free(compiler->build_path);
    if (compiler->dist_path)
        (void)free(compiler->dist_path);
    if (compiler->compiler_path)
        (void)free(compiler->compiler_path);
    if (compiler->objs.content)
        (void)empty_generic_vector(&compiler->objs, &free);
    if (compiler->srcs.content)
        (void)empty_generic_vector(&compiler->srcs, (expr_free)&delete_cnasset);
    (void)free(compiler);
}

uint8_t add_src_to_asset_compiler(GameAssetCompiler *compiler, const char *location, cnasset_type type)
{
    CNAsset *asset_src = new_cnasset(location, type);

    if (!asset_src) {
        PROPAGATE_ERR();
        return (1);
    }

    if (insert_generic_vector(&compiler->srcs, asset_src)) {
        PROPAGATE_ERR();
        (void)delete_cnasset(asset_src);
        return (1);
    }

    return (0);
}

uint8_t asset_compiler_build_objects(GameAssetCompiler *compiler)
{
    CNAsset *temp;
    size_t len = (size_t)snprintf(NULL, 0, "--align=%" PRIu16, compiler->alignement) + 1;
    char *temp_out;
    char *ext;
    char *argv[] = {
        compiler->compiler_path,
        "build",
        NULL,
        NULL,
        "-o",
        NULL,
        NULL,
        compiler->endianness ? "--endian=little" : "--endian=big",
        NULL
    };

    argv[6] = malloc(len);

    if (!argv[6]) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate alignement string.");
        return (1);
    }

    (void)snprintf(argv[6], len, "--align=%d", compiler->alignement);

    for (size_t i = 0; i < compiler->srcs.size; ++i) {
        temp = (CNAsset *)compiler->srcs.content[i];
        argv[3] = temp->location;

        switch (temp->type) {
            case CNASSET_TP_GUI:
                argv[2] = "gui";
                ext = "cgui";
                break;
            case CNASSET_TP_SCN:
                argv[2] = "scn";
                ext = "scn";
                break;
            case CNASSET_TP_OBJ:
                argv[2] = "obj";
                ext = "cobj";
                break;
            case CNASSET_TP_RAW:
                argv[2] = "asset";
                ext = "csst";
                break;
            case CNASSET_TP_PCA:
                (void)printf("registering %s.", temp->location);

                temp_out = strdup(temp->location);

                if (!temp_out) {
                    RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new temp_out string.");
                    (void)free(argv[6]);
                    return (1);
                }

                if (insert_generic_vector(&compiler->objs, temp_out)) {
                    PROPAGATE_ERR()
                    (void)free(temp_out);
                    (void)free(argv[6]);
                    return (1);
                }
                (void)free(temp_out);
                continue;
            default:
                RAISE_FMT(ERR_INVALID_TYPE, "asset %s can't be compiled.", temp->location);
                (void)free(argv[6]);
                return (1);
        }

        temp_out = join_path(compiler->build_path, path_basename(temp->location));

        if (!temp_out) {
            PROPAGATE_ERR();
            (void)free(argv[6]);
            return (1);
        }

        argv[5] = replace_extension(temp_out, ext);
        (void)free(temp_out);

        if (!argv[5]) {
            PROPAGATE_ERR();
            (void)free(argv[6]);
            return (1);
        }

        printf("building %s\n", argv[3]);
        // for (size_t v = 0; argv[v]; ++v)
        //     printf(argv[v + 1] ? "%s " : "%s\n", argv[v]);

        if (run_program(compiler->compiler_path, (const char * const*)argv)) {
            RAISE_FMT(ERR_OS, "compiler '%s' returned failure.", compiler->compiler_path);
            (void)free(argv[5]);
            (void)free(argv[6]);
            return (1);
        }

        if (insert_generic_vector(&compiler->objs, argv[5])) {
            PROPAGATE_ERR()
            (void)free(argv[5]);
            (void)free(argv[6]);
            return (1);
        }

        argv[5] = NULL;
    }

    (void)free(argv[6]);

    return (0);
}

uint8_t asset_compiler_build_banks(GameAssetCompiler *compiler)
{
    size_t bnk_cnt = 0;
    size_t item_start = 0;
    size_t item_end = 0;
    uint64_t size_registered = 0;
    char *temp_bank_name;
    size_t len = (size_t)snprintf(NULL, 0, "--align=%" PRIu16, compiler->alignement) + 1;
    char **argv;
    char *alignement_flag = malloc(len);

    if (!alignement_flag) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate alignement flag.");
        return (1);
    }

    (void)snprintf(alignement_flag, len, "--align=%d", compiler->alignement);

    // printf("/BANK#%zu - 0", bnk_cnt);

    // if (!compiler->objs.size)
    //     printf("\nempty?\n");

    for (size_t i = 0; i < compiler->objs.size; ++i) {
        size_registered += get_file_size((const char *)compiler->objs.content[i]);

        if (size_registered > compiler->max_bank_size || i + 1 >= compiler->objs.size) {
            // printf("\n");

            argv = malloc(sizeof(char *) * (7 + ((item_end - item_start) + 1) + 1));

            if (!argv) {
                RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new argv.");
                (void)free(alignement_flag);
                return (1);
            }
            len = (size_t)snprintf(NULL, 0, "pack%03zu.cpk", bnk_cnt) + 1;
            temp_bank_name = malloc(len);

            if (!temp_bank_name) {
                RAISE(ERR_OUT_OF_MEMORY, "failed to allocate bank name.");
                (void)free(alignement_flag);
                return (1);
            }

            (void)snprintf(temp_bank_name, len, "pack%03zu.cpk", bnk_cnt);

            argv[0] = compiler->compiler_path;
            argv[1] = "build";
            argv[2] = "lnk";
            argv[3] = compiler->endianness ? "--endian=little" : "--endian=big";
            argv[4] = alignement_flag;
            argv[5] = "-o";
            argv[6] = join_path(compiler->dist_path, temp_bank_name);

            (void)free(temp_bank_name);

            if (!argv[6]) {
                PROPAGATE_ERR();
                (void)free(alignement_flag);
                return (1);
            }

            for (size_t j = item_start; j <= item_end; ++j) {
                argv[7 + (j - item_start)] = (char *)compiler->objs.content[j];
            }

            argv[7 + (item_end - item_start) + 1] = NULL;

            printf("packing %s\n", argv[6]);
            // for (size_t v = 0; argv[v]; ++v)
            //     printf(argv[v + 1] ? "%s " : "%s\n", argv[v]);

            if (run_program(compiler->compiler_path, (const char * const*)argv)) {
                RAISE_FMT(ERR_OS, "compiler '%s' returned failure.", compiler->compiler_path);
                (void)free(argv[6]);
                (void)free(alignement_flag);
                return (1);
            }

            (void)free(argv[6]);
            (void)free(argv);

            item_start = i;
            item_end = i;
            ++bnk_cnt;

            // if (i + 1 < compiler->objs.size)
            //     printf("/BANK#%zu - 1", bnk_cnt);

            continue;
        }

        ++item_end;
        // printf("\r/BANK#%zu - %zu", bnk_cnt, (item_end - item_start) + 1);
    }

    (void)free(alignement_flag);
    return (0);
}

uint8_t compile_assets(const CNProject *project, const CNBuild *build_info, const char *dist_path, const char *build_path, const char *toolchain_path)
{
    (void)build_info;
    if (!project) {
        RAISE(ERR_INVALID_POINTER, "can't compile assets for empty project.");
        return (1);
    }

    GameAssetCompiler *compiler;
    CNAsset *temp_asset;

    compiler = new_asset_compiler(toolchain_path, build_path, dist_path);

    if (!compiler) {
        PROPAGATE_ERR();
        return (1);
    }

    compiler->endianness = build_info->assets_endian;
    compiler->alignement = build_info->assets_alignement;
    compiler->max_bank_size = build_info->assets_max_bank_size;

    for (size_t i = 0; i < project->content.size; ++i) {
        temp_asset = project->content.content[i];

        if (temp_asset->type != CNASSET_TP_SRC) {
            if (add_src_to_asset_compiler(compiler, temp_asset->location, temp_asset->type)) {
                PROPAGATE_ERR();
                (void)delete_asset_compiler(compiler);
                return (1);
            }
        }
    }

    if (asset_compiler_build_objects(compiler)) {
        PROPAGATE_ERR();
        delete_asset_compiler(compiler);
        return (1);
    }

    if (asset_compiler_build_banks(compiler)) {
        PROPAGATE_ERR();
        delete_asset_compiler(compiler);
        return (1);
    }

    (void)delete_asset_compiler(compiler);

    return (0);
}
