#include "libcnassets.h"

#include <unistd.h>
#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/mman.h>
#endif

CNAssetReader *new_object_file_reader(void)
{
    CNAssetReader *reader = (CNAssetReader *)malloc(sizeof(CNAssetReader));

    if (!reader)
        return (NULL);

    reader->_content.ready = false;

    #ifdef _WIN32
        reader->_content.mapping = NULL;
        reader->_content.file = NULL;
    #else
        reader->_content.fd = -1;
    #endif
    reader->_content.mapped_area = NULL;
    reader->_content.size = 0;
    return (reader);
}

uint8_t init_object_file_reader(CNAssetReader *reader, char *file_path)
{
    if (!reader || !file_path)
        return (1);

    if (reader->_content.ready)
        (void)uninit_object_file_reader(reader);

    reader->_content.ready = false;
    reader->_content.size = 0;
    reader->_content.mapped_area = NULL;

    #ifdef _WIN32
        LARGE_INTEGER sz;

        reader->_content.file = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (reader->_content.file == INVALID_HANDLE_VALUE)
            return (1);

        if (!GetFileSizeEx(reader->_content.file, &sz)) {
            CloseHandle(reader->_content.file);
            return (1);
        }

        reader->_content.size = (size_t)sz.QuadPart;
        reader->_content.mapping = CreateFileMappingA(reader->_content.file, NULL, PAGE_READONLY, 0, 0, NULL);

        if (!reader->_content.mapping) {
            CloseHandle(reader->_content.file);
            return (1);
        }

        reader->_content.mapped_area = MapViewOfFile(reader->_content.mapping, FILE_MAP_READ, 0, 0, 0);

        if (!reader->_content.mapped_area) {
            CloseHandle(reader->_content.mapping);
            CloseHandle(reader->_content.file);
            return (1);
        }
    #else
        struct stat st;

        reader->_content.fd = open(file_path, O_RDONLY);
        
        if (reader->_content.fd < 0)
            return (1);

        if (fstat(reader->_content.fd, &st) != 0) {
            (void)close(reader->_content.fd);
            return (1);
        }

        reader->_content.size = st.st_size;
        reader->_content.mapped_area = mmap(NULL, reader->_content.size, PROT_READ, MAP_PRIVATE, reader->_content.fd, 0);

        if (reader->_content.mapped_area == MAP_FAILED) {
            (void)close(reader->_content.fd);
            return (1);
        }
    #endif

    reader->_content.ready = true;
    return (0);
}

void uninit_object_file_reader(CNAssetReader *reader)
{
    if (!reader || !reader->_content.ready)
        return;
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

void delete_object_file_reader(CNAssetReader *reader)
{
    if (!reader)
        return;
    (void)uninit_object_file_reader(reader);
    (void)free(reader);
}