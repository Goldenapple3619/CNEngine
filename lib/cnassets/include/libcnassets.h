#ifndef _LIBCNASSETS_H_
    #define _LIBCNASSETS_H_

    #include "libcncore.h"
    #include <stdlib.h>

    #define ENGINE_OBJ_MAGIC 0x0875C4E3U
    
    #define ENGINE_OBJ_HDR_SZ           33U
    #define ENGINE_OBJ_SECHDR_PREFIX_SZ 16U
    #define ENGINE_OBJ_SECHDR_ENTRY_SZ  28U
    
    #ifdef ENGINE_USE_LARGE_FILE
        #define ENGINE_FTELL(fp)        ftello(fp)
        #define ENGINE_FSEEK(fp, o, w)  fseeko((fp), (o), (w))
        typedef off_t file_off_t;
    #else
        #define ENGINE_FTELL(fp)        ftell(fp)
        #define ENGINE_FSEEK(fp, o, w)  fseek((fp), (long)(o), (w))
        typedef long file_off_t;
    #endif
    typedef enum {
        ENGINE_OBJ_UKN = 0x00,
        ENGINE_OBJ_GUI,
        ENGINE_OBJ_SCN,
        ENGINE_OBJ_OBJ
    } engine_obj_type;

    typedef enum {
        ENGINE_WRT_BIG_ENDIAN = 0x00,
        ENGINE_WRT_LITTLE_ENDIAN
    } engine_wrt_endian;

    typedef enum {
        ENGINE_WRT_NULL_FLAG = 0x00,
        ENGINE_WRT_ALIGN1_FLAG = (1 << 0),
        ENGINE_WRT_ALIGN4_FLAG = (1 << 1),
        ENGINE_WRT_ALIGN8_FLAG = (1 << 2),
        ENGINE_WRT_ALIGN16_FLAG = (1 << 3),
        ENGINE_WRT_ALIGN64_FLAG = (1 << 4)
    } engine_wrt_type;

    struct engine_object_file_section_writer_ctx_s {
        char *section_name;
    
        struct {
            uint32_t type;
            uint32_t flags;
        } write_infos;

        char *content;
        uint64_t content_size;
    };

    struct engine_object_file_writer_ctx_s {
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

    typedef uint8_t (*wr32_fn)(FILE *, uint32_t);
    typedef uint8_t (*wr64_fn)(FILE *, uint64_t);
    typedef struct { wr32_fn u32; wr64_fn u64; } io_vtbl_t;

    struct engine_object_file_writer_ctx_s *new_writer_ctx(const char *name);
    uint8_t writer_ctx_set_object_name(struct engine_object_file_writer_ctx_s *wctx, const char *name);
    void delete_writer_ctx(struct engine_object_file_writer_ctx_s *wctx);
    uint8_t write_object_file(FILE *fp, const struct engine_object_file_writer_ctx_s *object_file_write_ctx);

#endif
