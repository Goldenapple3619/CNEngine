#include "libcnassets.h"

#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/mman.h>
#endif

CN_API CNAssetReader *new_object_file_reader(void)
{
    CNAssetReader *reader = (CNAssetReader *)malloc(sizeof(CNAssetReader));

    if (!reader) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new reader.");
        return (NULL);
    }

    reader->_content.ready = false;

    #ifdef _WIN32
        reader->_content.mapping = NULL;
        reader->_content.file = NULL;
    #else
        reader->_content.fd = -1;
    #endif
    reader->_content.mapped_area = NULL;
    reader->_content.size = 0;

    (void)memset(&reader->header, 0, sizeof(reader->header));
    (void)memset(&reader->section_header, 0, sizeof(reader->section_header));
    (void)memset(&reader->read_handler, 0, sizeof(reader->read_handler));

    return (reader);
}

CN_API uint8_t object_file_reader_read_header(CNAssetReader *reader)
{
    if (!reader || !reader->_content.ready) {
        RAISE(ERR_INVALID_POINTER, "can't read header of empty/notready reader.");
        return (1);
    }
    uint8_t *res = (uint8_t *)reader->_content.mapped_area;

    if (reader->_content.size < ENGINE_OBJ_HDR_SZ) {
        RAISE_FMT(ERR_CORRUPT_OR_INVALID, "can't read header (%zu < %zu).", reader->_content.size, (size_t)ENGINE_OBJ_HDR_SZ);
        return (1);
    }

    reader->header.magic = bufrd_u32_be(res);
    res += 4;

    if (reader->header.magic != ENGINE_OBJ_MAGIC) {
        RAISE_FMT(ERR_CORRUPT_OR_INVALID, "can't read header (%" PRIx32 " != %" PRIx32 ").", reader->header.magic, (uint32_t)ENGINE_OBJ_MAGIC);
        return (1);
    }

    reader->header.endian = bufrd_u8(res);
    res += 1;

    reader->read_handler.u8 = &bufrd_u8;
    if (reader->header.endian == ENGINE_WRT_BIG_ENDIAN) {
        reader->read_handler.u16 = &bufrd_u16_be;
        reader->read_handler.u32 = &bufrd_u32_be;
        reader->read_handler.u64 = &bufrd_u64_be;
    } else {
        reader->read_handler.u16 = &bufrd_u16_le;
        reader->read_handler.u32 = &bufrd_u32_le;
        reader->read_handler.u64 = &bufrd_u64_le;
    }

    reader->header.flags = reader->read_handler.u32(res);
    res += 4;

    reader->header.type = reader->read_handler.u16(res);
    res += 2;

    reader->header.section_header_off = reader->read_handler.u64(res);
    res += 8;

    reader->header.strndx_off = reader->read_handler.u64(res);
    res += 8;

    reader->header.name = reader->read_handler.u32(res);
    res += 4;

    return (0);
}

CN_API const char *object_file_reader_get_string(const CNAssetReader *reader, uint32_t off)
{
    if (!reader || !reader->_content.ready) {
        RAISE(ERR_INVALID_POINTER, "can't get string from empty/notready reader.");
        return (NULL);
    }
    if (reader->_content.size < reader->header.strndx_off + off) {
        RAISE_FMT(ERR_CORRUPT_OR_INVALID, "can't get string at invalid location (%zu < %" PRIu64 ").", reader->_content.size, reader->header.strndx_off + off);
        return (NULL);
    }

    uint8_t *res = (uint8_t *)reader->_content.mapped_area;

    return ((char *)(res + reader->header.strndx_off + off));
}

CN_API void object_file_reader_get_section(const CNAssetReader *reader, struct section_blk *section_block, uint64_t section_index)
{
    if (!reader || !reader->_content.ready) {
        RAISE(ERR_INVALID_POINTER, "can't get section of empty/notready reader.");
        section_block->section_blk_ptr = NULL;
        section_block->blk_size = 0;
        return;
    }
    if (section_index >= reader->section_header.section_count) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "can't get section at invalid position (%" PRIu64 " >= %" PRIu64 ").", section_index, reader->section_header.section_count);
        section_block->section_blk_ptr = NULL;
        section_block->blk_size = 0;
        return;
    }

    uint8_t *res = (uint8_t *)reader->_content.mapped_area;

    if (reader->_content.size < reader->section_header.entries[section_index].section_off) {
        RAISE_FMT(ERR_CORRUPT_OR_INVALID, "can't get section at invalid location (%zu < %" PRIu64 ").", reader->_content.size, reader->section_header.entries[section_index].section_off);
        section_block->section_blk_ptr = NULL;
        section_block->blk_size = 0;
        return;
    }

    section_block->section_blk_ptr = (const uint8_t *)(res + reader->section_header.entries[section_index].section_off);
    section_block->blk_size = reader->section_header.entries[section_index].section_size;
}

CN_API uint8_t object_file_reader_read_section_header(CNAssetReader *reader)
{
    if (!reader || !reader->_content.ready) {
        RAISE(ERR_INVALID_POINTER, "can't read sheader for empty/notready reader.");
        return (1);
    }
    
    if (reader->header.magic != ENGINE_OBJ_MAGIC) {
        RAISE_FMT(ERR_CORRUPT_OR_INVALID, "can't read sheader (%" PRIx32 " != %" PRIx32 ").", reader->header.magic, (uint32_t)ENGINE_OBJ_MAGIC);
        return (1);
    }

    if (reader->section_header.entries) {
        (void)free(reader->section_header.entries);
        reader->section_header.entries = NULL;
    }

    uint8_t *res = (uint8_t *)reader->_content.mapped_area;

    if (reader->_content.size < reader->header.section_header_off || ENGINE_OBJ_HDR_SZ > reader->header.section_header_off) {
        RAISE(ERR_CORRUPT_OR_INVALID, "can't read sheader, base dosen't match expected size / position.");
        return (1);
    }

    res += reader->header.section_header_off;

    if (!reader->read_handler.u16 || !reader->read_handler.u32 || !reader->read_handler.u64 || !reader->read_handler.u8) {
        RAISE(ERR_NOT_COMPATIBLE, "reader somehow don't have any ready read_handler to read b/l endian.");
        return (1);
    }

    reader->section_header.size = reader->read_handler.u64(res);
    res += 8;

    reader->section_header.section_count = reader->read_handler.u64(res);
    res += 8;

    if (reader->_content.size < reader->header.section_header_off + ENGINE_OBJ_SECHDR_PREFIX_SZ + ENGINE_OBJ_SECHDR_ENTRY_SZ * reader->section_header.section_count) {
        RAISE(ERR_CORRUPT_OR_INVALID, "can't read sheader, content dosen't match expected size / position.");
        reader->section_header.section_count = 0;
        return (1);
    }

    reader->section_header.entries = malloc(sizeof(struct engine_obj_section_header_entry_s) * reader->section_header.section_count);

    if (!reader->section_header.entries) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate sheader entries of size %zu.", reader->section_header.section_count)
        reader->section_header.section_count = 0;
        return (1);
    }

    for (uint64_t i = 0; i < reader->section_header.section_count; ++i) {
        reader->section_header.entries[i].section_name = reader->read_handler.u32(res);
        res += 4;

        reader->section_header.entries[i].section_type = reader->read_handler.u16(res);
        res += 2;

        reader->section_header.entries[i].section_flags = reader->read_handler.u32(res);
        res += 4;

        reader->section_header.entries[i].section_size = reader->read_handler.u64(res);
        res += 8;

        reader->section_header.entries[i].section_off = reader->read_handler.u64(res);
        res += 8;
    }

    return (0);
}

CN_API uint8_t init_object_file_reader(CNAssetReader *reader, const char *file_path)
{
    if (!reader) {
        RAISE(ERR_INVALID_POINTER, "can't init empty reader.");
        return (1);
    }

    if (!file_path) {
        RAISE(ERR_INVALID_POINTER, "can't init reader from empty file path.");
        return (1);
    }

    if (reader->_content.ready)
        (void)uninit_object_file_reader(reader);

    reader->_content.ready = false;
    reader->_content.size = 0;
    reader->_content.mapped_area = NULL;

    #ifdef _WIN32
        LARGE_INTEGER sz;

        reader->_content.file = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (reader->_content.file == INVALID_HANDLE_VALUE) {
            RAISE_FMT(ERR_OS, "failed to CreateFileA file '%s' for reader.", file_path);
            return (1);
        }

        if (!GetFileSizeEx(reader->_content.file, &sz)) {
            RAISE_FMT(ERR_OS, "failed to GetFileSizeEx file '%s' for reader.", file_path);
            CloseHandle(reader->_content.file);
            return (1);
        }

        reader->_content.size = (size_t)sz.QuadPart;
        reader->_content.mapping = CreateFileMappingA(reader->_content.file, NULL, PAGE_READONLY, 0, 0, NULL);

        if (!reader->_content.mapping) {
            RAISE_FMT(ERR_OS, "failed to CreateFileMappingA file '%s' for reader.", file_path);
            CloseHandle(reader->_content.file);
            return (1);
        }

        reader->_content.mapped_area = MapViewOfFile(reader->_content.mapping, FILE_MAP_READ, 0, 0, 0);

        if (!reader->_content.mapped_area) {
            RAISE_FMT(ERR_OS, "failed to MapViewOfFile from mapping of file '%s' for reader.", file_path);
            CloseHandle(reader->_content.mapping);
            CloseHandle(reader->_content.file);
            return (1);
        }
    #else
        struct stat st;

        reader->_content.fd = open(file_path, O_RDONLY);
        
        if (reader->_content.fd < 0) {
            RAISE_FMT(ERR_OS, "failed to open file '%s' for reader.", file_path);
            return (1);
        }

        if (fstat(reader->_content.fd, &st) != 0) {
            RAISE_FMT(ERR_OS, "failed to fstat file '%s' for reader.", file_path);
            (void)close(reader->_content.fd);
            return (1);
        }

        reader->_content.size = st.st_size;
        reader->_content.mapped_area = mmap(NULL, reader->_content.size, PROT_READ, MAP_PRIVATE, reader->_content.fd, 0);

        if (reader->_content.mapped_area == MAP_FAILED) {
            RAISE_FMT(ERR_OS, "failed to mmap file '%s' for reader.", file_path);
            (void)close(reader->_content.fd);
            return (1);
        }
    #endif

    reader->_content.ready = true;
    return (0);
}

CN_API void uninit_object_file_reader(CNAssetReader *reader)
{
    if (!reader || !reader->_content.ready) {
        RAISE(ERR_INVALID_POINTER, "can't uninit empty/notready reader.");
        return;
    }
    #ifdef _WIN32
        if (reader->_content.mapped_area)
            (void)UnmapViewOfFile(reader->_content.mapped_area);
        if (reader->_content.mapping)
            (void)CloseHandle(reader->_content.mapping);
        if (reader->_content.file)
            (void)CloseHandle(reader->_content.file);
    #else
        if (reader->_content.mapped_area)
            (void)munmap((void *)reader->_content.mapped_area, reader->_content.size);
        if (reader->_content.fd > -1)
            (void)close(reader->_content.fd);
    #endif
    reader->_content.ready = false;
}

CN_API void delete_object_file_reader(CNAssetReader *reader)
{
    if (!reader) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty reader.");
        return;
    }
    (void)uninit_object_file_reader(reader);
    if (reader->section_header.entries) {
        (void)free(reader->section_header.entries);
        reader->section_header.entries = NULL;
    }
    (void)free(reader);
}

CN_API void print_object_file(CNAssetReader *reader)
{
    if (!reader) {
        RAISE(ERR_INVALID_POINTER, "can't print empty reader.");
        return;
    }
    printf("<asset[%s]:%d\n", object_file_reader_get_string(reader, reader->header.name), reader->header.type);
    for (uint64_t i = 0; i < reader->section_header.section_count; ++i)
        printf("  <section[%s]:%d@(%" PRIx64 "-%" PRIx64 ")>\n", object_file_reader_get_string(reader, reader->section_header.entries[i].section_name), reader->section_header.entries[i].section_type, reader->section_header.entries[i].section_off, reader->section_header.entries[i].section_off + reader->section_header.entries[i].section_size);
    printf(">\n");
}
