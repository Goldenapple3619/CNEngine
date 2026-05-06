#ifndef _BUILD_H_
    #define _BUILD_H_

    #include <libxml/parser.h>
    #include <libxml/tree.h>

    #include "../engine.h"

    #define ENGINE_OBJ_MAGIC 0x0875C4E3

    typedef enum {
        ENGINE_OBJ_UKN = 0x00,
        ENGINE_OBJ_GUI,
        ENGINE_OBJ_SCN,
        ENGINE_OBJ_OBJ
    } engine_obj_type;

    struct engine_object_file_section_s {
        char *section_name;
    
        struct {
            uint32_t type;
            uint32_t flags;
        } write_infos;

        char *content;
        uint64_t content_size;
    };

    struct engine_object_file_s {
        char *object_name;
    
        struct {
            uint8_t endian;
            uint32_t type;
            uint32_t flags;
        } write_infos;
    
        struct generic_map_s sections;
    };

    struct engine_obj_header_s {
        uint32_t magic;
        uint8_t endian;
        uint32_t flags;
        uint32_t type;
        uint64_t section_header_off;
        uint64_t strndx_off;
        uint32_t name;
    };

    struct engine_obj_section_header_entry_s {
        uint32_t section_name;
        uint32_t section_type;
        uint32_t section_flags;
        uint64_t section_size;

        uint64_t section_off;
    };

    struct engine_obj_section_header_s {
        uint64_t size;
        uint64_t section_count;

        struct engine_obj_section_header_entry_s *entries;
    };

    uint8_t write_object_file(const struct engine_object_file_s *object_file_write_ctx);

    int build_gui(size_t argc, char **argv);

    static const struct {
        const char *name;
        route_callback callback;
    } build_types[] = {
        {"gui", &build_gui},
        {"obj", NULL},
        {"scn", NULL},
        {"proj", NULL},
        {NULL, NULL}
    };

#endif
