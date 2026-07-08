#include "libcnassets.h"

CN_API struct engine_object_file_writer_ctx_s *new_writer_ctx(const char *name)
{
    struct engine_object_file_writer_ctx_s *wctx = malloc(sizeof(struct engine_object_file_writer_ctx_s));

    if (!wctx) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new writer ctx.");
        return (NULL);
    }

    wctx->object_name = name ? strdup(name) : NULL;
    if (name && !wctx->object_name) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new writer ctx name '%s'.", name);
        (void)free(wctx);
        return (NULL);
    }
    wctx->write_infos.endian = ENGINE_WRT_BIG_ENDIAN;
    wctx->write_infos.type = ENGINE_OBJ_UKN;
    wctx->write_infos.flags = ENGINE_WRT_NULL_FLAG;

    wctx->sections.capacity = 0;
    wctx->sections.size = 0;
    wctx->sections.content = NULL;
    return (wctx);
}

CN_API uint8_t writer_ctx_set_object_name(struct engine_object_file_writer_ctx_s *wctx, const char *name)
{
    if (!wctx) {
        RAISE(ERR_INVALID_POINTER, "can't set object name on empty ctx.")
        return (1);
    }
    if (wctx->object_name)
        (void)free(wctx->object_name);
    wctx->object_name = name ? strdup(name) : NULL;
    if (name && !wctx->object_name) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new writer ctx name '%s'.", name);
        return (1);
    }
    return (0);
}

CN_API uint8_t writer_ctx_add_section(struct engine_object_file_writer_ctx_s *wctx, struct engine_object_file_section_writer_ctx_s *section)
{
    if (!wctx) {
        RAISE(ERR_INVALID_POINTER, "can't add section to empty ctx.")
        return (1);
    }

    if (!section) {
        RAISE(ERR_INVALID_POINTER, "can't add empty section to ctx.");
        return (1);
    }

    if (insert_generic_vector(&wctx->sections, section)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}

CN_API void delete_writer_ctx(struct engine_object_file_writer_ctx_s *wctx)
{
    if (!wctx) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty ctx.")
        return;
    }
    if (wctx->object_name)
        (void)free(wctx->object_name);
    if (wctx->sections.content) {
        for (size_t i = 0; i < wctx->sections.size; ++i) {
            (void)delete_writer_section(wctx->sections.content[i]);
        }

        (void)free(wctx->sections.content);
    }
    (void)free(wctx);
}

CN_API struct engine_object_file_section_writer_ctx_s *new_writer_section(const char *name, void *content_holder)
{
    struct engine_object_file_section_writer_ctx_s *section = malloc(sizeof(struct engine_object_file_section_writer_ctx_s));

    if (!section) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new writer section '%s'.", name ? name : "null");
        return (NULL);
    }

    section->section_name = name ? strdup(name) : NULL;
    if (name && !section->section_name) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new writer section name '%s'.", name);
        (void)free(section);
        return (NULL);
    }

    section->write_infos.type = ENGINE_SEC_UKN;
    section->write_infos.flags = ENGINE_SEC_NULL_FLAG;
    section->content_generator = NULL;
    section->content_size_generator = NULL;
    section->_v = content_holder;
    return (section);
}

CN_API uint8_t writer_section_set_name(struct engine_object_file_section_writer_ctx_s *section, const char *name)
{
    if (!section) {
        RAISE(ERR_INVALID_POINTER, "can't set name empty ctx section.")
        return (1);
    }
    if (section->section_name)
        (void)free(section->section_name);
    section->section_name = name ? strdup(name) : NULL;
    if (name && !section->section_name) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new writer section name '%s'.", name);
        return (1);
    }
    return (0);
}

CN_API void delete_writer_section(struct engine_object_file_section_writer_ctx_s *section)
{
    if (!section) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty ctx section.")
        return;
    }
    if (section->section_name)
        (void)free(section->section_name);
    section->content_generator = NULL;
    section->content_size_generator = NULL;
    section->_v = NULL;
    (void)free(section);
}

CN_API uint32_t flags_to_align(uint32_t flags)
{
    if ((flags & ENGINE_WRT_ALIGN64_FLAG) > 0)
        return (64);
    if ((flags & ENGINE_WRT_ALIGN16_FLAG) > 0)
        return (16);
    if ((flags & ENGINE_WRT_ALIGN8_FLAG) > 0)
        return (8);
    if ((flags & ENGINE_WRT_ALIGN4_FLAG) > 0)
        return (4);
    if ((flags & ENGINE_WRT_ALIGN4096_FLAG) > 0)
        return (4096);
    return (1);
}

static uint8_t write_pad(FILE *fp, uint64_t pos, uint32_t align)
{
    static uint8_t zeroes[ENGINE_MAX_PAD];
    uint64_t pad;
    size_t chunk;
    
    (void)memset((void *)zeroes, ENGINE_PAD_CHAR, sizeof(zeroes) * sizeof(uint8_t));

    if (align <= 1)
        return (0);

    pad = (align - (pos % align)) % align;
    while (pad > 0) {
        chunk = (pad > sizeof(zeroes)) ? sizeof(zeroes) : (size_t)pad;
        if (fwrite(zeroes, 1, chunk, fp) != chunk) {
            RAISE_FMT(ERR_OS, "failed to write padding of size %zu.", chunk);
            return (1);
        }
        pad -= chunk;
    }
    return (0);
}

static uint8_t write_object_file_header(FILE *fp, const fpio_handler_t *io, const struct engine_obj_header_s *header)
{
    if (fpwr_u32_be(fp, header->magic)) {
        RAISE(ERR_OS, "failed to write header magic.");
        return 1;
    }
    if (fpwr_u8(fp, header->endian)) {
        RAISE(ERR_OS, "failed to write header endian.");
        return 1;
    }
    if (io->u32(fp, header->flags)) {
        RAISE(ERR_OS, "failed to write header flags.");
        return 1;
    }
    if (io->u16(fp, header->type)) {
        RAISE(ERR_OS, "failed to write header type.");
        return 1;
    }
    if (io->u64(fp, header->section_header_off)) {
        RAISE(ERR_OS, "failed to write sheader off.");
        return 1;
    }
    if (io->u64(fp, header->strndx_off)) {
        RAISE(ERR_OS, "failed to write strndx off.");
        return 1;
    }
    if (io->u32(fp, header->name)) {
        RAISE(ERR_OS, "failed to write header name.");
        return 1;
    }
    return 0;
}

static uint8_t write_object_file_section_header(FILE *fp, const fpio_handler_t *io, const struct engine_obj_section_header_s *section_header, const struct generic_vector_s *section_header_entries)
{
    struct engine_obj_section_header_entry_s *temp_entry;

    if (io->u64(fp, section_header->size)) {
        RAISE(ERR_OS, "failed to write sheader size.");
        return 1;
    }
    if (io->u64(fp, section_header->section_count)) {
        RAISE(ERR_OS, "failed to write sheader section count.");
        return 1;
    }

    for (size_t i = 0; i < section_header_entries->size; ++i) {
        temp_entry = section_header_entries->content[i];

        if (io->u32(fp, temp_entry->section_name)) {
            RAISE_FMT(ERR_OS, "failed to write sheader section name #%zu.", i);
            return 1;
        }
        if (io->u16(fp, temp_entry->section_type)) {
            RAISE_FMT(ERR_OS, "failed to write sheader section type #%zu.", i);
            return 1;
        }
        if (io->u32(fp, temp_entry->section_flags)) {
            RAISE_FMT(ERR_OS, "failed to write sheader section flags #%zu.", i);
            return 1;
        }
        if (io->u64(fp, temp_entry->section_size)) {
            RAISE_FMT(ERR_OS, "failed to write sheader section size #%zu.", i);
            return 1;
        }
        if (io->u64(fp, temp_entry->section_off)) {
            RAISE_FMT(ERR_OS, "failed to write sheader section off #%zu.", i);
            return 1;
        }
    }

    return 0;
}

static uint8_t write_object_file_strndx(FILE *fp, const fpio_handler_t *io, const struct generic_map_s *strndx)
{
    (void)io;

    const struct strndx_entry_s *entry;

    for (size_t i = 0; i < strndx->size; ++i) {
        entry = strndx->content[i];

        if (fwrite(entry->string, 1, strlen(entry->string) + 1, fp) != strlen(entry->string) + 1) {
            RAISE_FMT(ERR_OS, "failed to write strndx entry #%zu.", i);
            return (1);
        }
    }

    return (0);
}

static struct strndx_entry_s *new_strndx_entry(const char *str)
{
    struct strndx_entry_s *entry = (struct strndx_entry_s *)malloc(sizeof(struct strndx_entry_s));

    if (!entry) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new strndx entry.");
        return (NULL);
    }

    entry->addr = 0xFFFFFFFFu;
    entry->string = str ? strdup(str) : strdup("");

    if (!entry->string) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new strndx entry's string.");
        (void)free(entry);
        return (NULL);
    }
    return (entry);
}

static void delete_strndx_entry(struct strndx_entry_s *entry)
{
    if (!entry) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty strndx entry.");
        return;
    }
    if (entry->string)
        (void)free(entry->string);
    (void)free(entry);
}

CN_API uint32_t add_str_table(const char *str, struct generic_map_s *strndx)
{
    if (!strndx) {
        RAISE(ERR_INVALID_POINTER, "can't add to empty strndx table.");
        return (0xFFFFFFFFu);
    }
    if (!str)
        str = "<null>";

    struct strndx_entry_s *found;

    if (!has_generic_map(strndx, str)) {
        found = new_strndx_entry(str);

        if (!found) {
            PROPAGATE_ERR();
            return (0xFFFFFFFFu);
        }

        if (add_generic_map(strndx, found, str, (expr_free)&delete_strndx_entry)) {
            PROPAGATE_ERR();
            (void)delete_strndx_entry(found);
            return (0xFFFFFFFFu);
        }

        if (strndx->size <= 1)
            found->addr = 0;
        else
            found->addr = ((struct strndx_entry_s *)strndx->content[strndx->size - 2])->addr + strlen(((struct strndx_entry_s *)strndx->content[strndx->size - 2])->string) + 1;
    } else {
        found = get_generic_map(strndx, str, NULL, NULL);
    }

    return (found->addr);
}

static uint8_t mv_pad(FILE *fp, int64_t *x, uint32_t align)
{
    *x = (ENGINE_FTELL(fp));

    if (*x < 0) {
        RAISE(ERR_OS, "failed to ftell current file position before padding.");
        return (1);
    }
    
    if (write_pad(fp, *x, align)) {
        PROPAGATE_ERR();
        return (1);
    }

    *x = ENGINE_FTELL(fp);

    if (*x < 0) {
        RAISE(ERR_OS, "failed to ftell current file position after padding.");
        return (1);
    }

    return (0);
}

CN_API uint8_t write_object_file(FILE *fp, const struct engine_object_file_writer_ctx_s *object_file_write_ctx)
{
    struct engine_obj_header_s header;
    struct engine_obj_section_header_s section_header;
    struct engine_obj_section_header_entry_s *temp_entry;
    struct generic_vector_s *section_header_entries;
    struct generic_map_s *strndx;
    struct engine_object_file_section_writer_ctx_s *temp_section;
    fpio_handler_t io_handler;
    int32_t align = flags_to_align(object_file_write_ctx->write_infos.flags);
    int64_t x;
    size_t content_size;
    char *content;

    if (!fp) {
        RAISE(ERR_INVALID_POINTER, "can't write to empty fp.")
        return (1);
    }

    if (!object_file_write_ctx) {
        RAISE(ERR_INVALID_POINTER, "can't write empty writer_ctx.")
        return (1);
    }

    if (ENGINE_FSEEK(fp, 0, SEEK_SET) != 0) {
        RAISE(ERR_OS, "failed to seek to start of file.");
        return (1);
    }

    io_handler = fpio_handler_from_writer(object_file_write_ctx);

    strndx = new_generic_map();

    if (!strndx) {
        PROPAGATE_ERR();
        return (1);
    }

    section_header_entries = new_generic_vector();

    if (!section_header_entries) {
        PROPAGATE_ERR();
        delete_generic_map(strndx, (expr_free)&delete_strndx_entry);
        return (1);
    }

    header.magic = ENGINE_OBJ_MAGIC;
    header.endian = object_file_write_ctx->write_infos.endian;
    header.flags = object_file_write_ctx->write_infos.flags;
    header.type = object_file_write_ctx->write_infos.type;
    header.section_header_off = 0;
    header.strndx_off = 0;
    header.name = add_str_table(object_file_write_ctx->object_name ? object_file_write_ctx->object_name : "unnamed_object", strndx);

    if (header.name == 0xFFFFFFFFu) {
        PROPAGATE_ERR();
        delete_generic_map(strndx, (expr_free)&delete_strndx_entry);
        delete_generic_vector(section_header_entries, &free);
        return (1);
    }

    section_header.size = ENGINE_OBJ_SECHDR_PREFIX_SZ + ENGINE_OBJ_SECHDR_ENTRY_SZ * object_file_write_ctx->sections.size;
    section_header.section_count = object_file_write_ctx->sections.size;

    for (size_t i = 0; i < object_file_write_ctx->sections.size; ++i) {
        temp_section = object_file_write_ctx->sections.content[i];
        temp_entry = malloc(sizeof(struct engine_obj_section_header_entry_s));

        if (!temp_entry) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new section header entry.");
            (void)delete_generic_map(strndx, (expr_free)&delete_strndx_entry);
            (void)delete_generic_vector(section_header_entries, &free);
            return (1);
        }

        if (insert_generic_vector(section_header_entries, temp_entry)) {
            PROPAGATE_ERR();
            (void)free(temp_entry);
            (void)delete_generic_map(strndx, (expr_free)&delete_strndx_entry);
            (void)delete_generic_vector(section_header_entries, &free);
            return (1);
        }
    
        temp_entry->section_flags = temp_section->write_infos.flags;
        temp_entry->section_type = temp_section->write_infos.type;
        temp_entry->section_size = temp_section->content_size_generator ? temp_section->content_size_generator(temp_section) : 0;
        temp_entry->section_name = add_str_table(temp_section->section_name, strndx);
        temp_entry->section_off = 0;

        if (temp_entry->section_name == 0xFFFFFFFFu) {
            PROPAGATE_ERR();
            delete_generic_map(strndx, (expr_free)&delete_strndx_entry);
            delete_generic_vector(section_header_entries, &free);
            return (1);
        }
    }

    if (write_object_file_header(fp, &io_handler, &header)) {
        PROPAGATE_ERR();
        delete_generic_map(strndx, (expr_free)&delete_strndx_entry);
        delete_generic_vector(section_header_entries, &free);
        return (1);
    }

    if (mv_pad(fp, &x, align)) {
        PROPAGATE_ERR();
        delete_generic_map(strndx, (expr_free)&delete_strndx_entry);
        delete_generic_vector(section_header_entries, &free);
        return (1);
    };
    header.section_header_off = (uint64_t)x;

    if (write_object_file_section_header(fp, &io_handler, &section_header, section_header_entries)) {
        PROPAGATE_ERR();
        delete_generic_map(strndx, (expr_free)&delete_strndx_entry);
        delete_generic_vector(section_header_entries, &free);
        return (1);
    }

    for (size_t i = 0; i < object_file_write_ctx->sections.size; ++i) {
        temp_section = object_file_write_ctx->sections.content[i];

        if (mv_pad(fp, &x, align)) {
            PROPAGATE_ERR();
            delete_generic_map(strndx, (expr_free)&delete_strndx_entry);
            delete_generic_vector(section_header_entries, &free);
            return (1);
        };
        ((struct engine_obj_section_header_entry_s *)section_header_entries->content[i])->section_off = (uint64_t)x;

        content_size = temp_section->content_size_generator ? (size_t)temp_section->content_size_generator(temp_section) : (size_t)0;
        content = temp_section->content_generator ? temp_section->content_generator(temp_section, object_file_write_ctx, strndx) : NULL;

        if (content_size && content) {
            if (fwrite(content, 1, content_size, fp) != content_size) {
                RAISE_FMT(ERR_OS, "failed to write section content of supposed size %zu.", content_size);
                delete_generic_map(strndx, (expr_free)&delete_strndx_entry);
                delete_generic_vector(section_header_entries, &free);
                free(content);
                return (1);
            }
        } else {
            RAISE_FMT(WAR_IMPORTANT, "written an empty section at %" PRIx64 "\n", ENGINE_FTELL(fp))
        }

        if (content)
            (void)free(content);
    }

    if (mv_pad(fp, &x, align)) {
        PROPAGATE_ERR();
        delete_generic_map(strndx, (expr_free)&delete_strndx_entry);
        delete_generic_vector(section_header_entries, &free);
        return (1);
    };
    header.strndx_off = (uint64_t)x;

    if (write_object_file_strndx(fp, &io_handler, strndx)) {
        PROPAGATE_ERR();
        delete_generic_map(strndx, (expr_free)&delete_strndx_entry);
        delete_generic_vector(section_header_entries, &free);
        return (1);
    }

    delete_generic_map(strndx, (expr_free)&delete_strndx_entry);

    if (ENGINE_FSEEK(fp, 0, SEEK_SET) != 0) {
        RAISE(ERR_OS, "failed to re seek to start of file for second round.");
        delete_generic_vector(section_header_entries, &free);
        return (1);
    };

    if (write_object_file_header(fp, &io_handler, &header)) {
        PROPAGATE_ERR();
        delete_generic_vector(section_header_entries, &free);
        return (1);
    }

    if (ENGINE_FSEEK(fp, header.section_header_off, SEEK_SET) != 0) {
        RAISE(ERR_OS, "failed to re seek to section header location for second round.");
        delete_generic_vector(section_header_entries, &free);
        return (1);
    }

    if (write_object_file_section_header(fp, &io_handler, &section_header, section_header_entries)) {
        PROPAGATE_ERR();
        delete_generic_vector(section_header_entries, &free);
        return (1);
    }

    delete_generic_vector(section_header_entries, &free);

    if (fflush(fp) != 0) {
        RAISE(ERR_OS, "failed to flush the file.");
        return (1);
    }

    return (0);
}
