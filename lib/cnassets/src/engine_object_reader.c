#include "libcnassets.h"

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

CNAssetReader *new_object_file_reader(void)
{
    CNAssetReader *reader = (CNAssetReader *)malloc(sizeof(CNAssetReader));

    if (!reader)
        return (NULL);

    reader->_content.ready = false;
    reader->_content.fd = -1,
    reader->_content.mapped_area = NULL;
    reader->_content.size = 0;
    return (reader);
}

uint8_t init_object_file_reader(CNAssetReader *reader, char *file_path)
{
    struct stat st;

    if (!reader || !file_path)
        return (1);

    if (reader->_content.ready)
        (void)uninit_object_file_reader(reader);

    reader->_content.ready = false;
    reader->_content.size = 0;
    reader->_content.mapped_area = NULL;
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

    reader->_content.ready = true;
    return (0);
}

void uninit_object_file_reader(CNAssetReader *reader)
{
    if (!reader || !reader->_content.ready)
        return;
    if (reader->_content.mapped_area)
        (void)munmap((void *)reader->_content.mapped_area, reader->_content.size);
    if (reader->_content.fd > -1)
        (void)close(reader->_content.fd);
    reader->_content.ready = false;
}

void delete_object_file_reader(CNAssetReader *reader)
{
    if (!reader)
        return;
    (void)uninit_object_file_reader(reader);
    (void)free(reader);
}