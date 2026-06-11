#ifndef _LIBCNASSETS_H_
    #define _LIBCNASSETS_H_

    #define ENGINE_USE_LARGE_FILE true
    #define _FILE_OFFSET_BITS 64

    #include "libcncore.h"
    #include <stdlib.h>
    #include <string.h>

    #define ENGINE_OBJ_MAGIC 0x0875C4E3U
    
    #define ENGINE_OBJ_HDR_SZ 33U
    #define ENGINE_OBJ_SECHDR_PREFIX_SZ 16U
    #define ENGINE_OBJ_SECHDR_ENTRY_SZ 28U

    #define ENGINE_MAX_PAD 4096U
    #define ENGINE_PAD_CHAR 0xCD

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
        ENGINE_OBJ_OBJ,
        ENGINE_OBJ_ASSET_PACK,
        ENGINE_OBJ_RAW_RESSOURCES
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
        ENGINE_WRT_ALIGN64_FLAG = (1 << 4),
        ENGINE_WRT_ALIGN4096_FLAG = (1 << 5)
    } engine_wrt_flags;

    typedef enum {
        ENGINE_SEC_UKN = 0x00,
        ENGINE_SEC_GUI_NODES,
        ENGINE_SEC_GUI_STYLE,
        ENGINE_SEC_GUI_CONN
    } engine_section_type;

    typedef enum {
        ENGINE_SEC_NULL_FLAG = 0x00
    } engine_section_wrt_flags;

    struct engine_object_file_writer_ctx_s;

    struct engine_object_file_section_writer_ctx_s {
        char *section_name;
    
        struct {
            uint32_t type;
            uint32_t flags;
        } write_infos;

        char *(*content_generator)(struct engine_object_file_section_writer_ctx_s *self, const struct engine_object_file_writer_ctx_s *writer, struct generic_map_s *strndx);
        uint64_t (*content_size_generator)(struct engine_object_file_section_writer_ctx_s *self);

        void *_v;
    };

    struct engine_object_file_writer_ctx_s {
        char *object_name;
    
        struct {
            uint8_t endian;
            uint32_t type;
            uint32_t flags;
        } write_infos;
    
        struct generic_vector_s sections;
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

    struct strndx_entry_s {
        char *string;
        uint32_t addr;
    };

    typedef struct {
        struct {
            const void *mapped_area;
            size_t size;

            int fd;
            cnbool ready;
        } _content;
    } CNAssetReader;

    typedef uint8_t (*wr8_fn)(FILE *, uint8_t);
    typedef uint8_t (*wr16_fn)(FILE *, uint16_t);
    typedef uint8_t (*wr32_fn)(FILE *, uint32_t);
    typedef uint8_t (*wr64_fn)(FILE *, uint64_t);
    typedef struct {
        wr8_fn u8;
        wr16_fn u16;
        wr32_fn u32;
        wr64_fn u64;
    } fpio_handler_t;

    uint8_t fpwr_u8(FILE *fp, uint8_t v);
    uint8_t fpwr_u16_be(FILE *fp, uint16_t v);
    uint8_t fpwr_u16_le(FILE *fp, uint16_t v);
    uint8_t fpwr_u32_be(FILE *fp, uint32_t v);
    uint8_t fpwr_u32_le(FILE *fp, uint32_t v);
    uint8_t fpwr_u64_be(FILE *fp, uint64_t v);
    uint8_t fpwr_u64_le(FILE *fp, uint64_t v);

    void bufwr_u8(char *buf, uint8_t v);
    void bufwr_u16_be(char *buf, uint16_t v);
    ;void bufwr_u16_le(char *buf, uint16_t v);
    void bufwr_u32_be(char *buf, uint32_t v);
    void bufwr_u32_le(char *buf, uint32_t v);
    void bufwr_u64_be(char *buf, uint64_t v);
    void bufwr_u64_le(char *buf, uint64_t v);

    uint8_t bufrd_u8(const void *p);
    uint16_t bufrd_u16_be(const void *p);
    uint16_t bufrd_u16_le(const void *p);
    uint32_t bufrd_u32_be(const void *p);
    uint32_t bufrd_u32_le(const void *p);
    uint64_t bufrd_u64_be(const void *p);
    uint64_t bufrd_u64_le(const void *p);

    fpio_handler_t fpio_handler_from_writer(const struct engine_object_file_writer_ctx_s *object_file_write_ctx);

    struct engine_object_file_writer_ctx_s *new_writer_ctx(const char *name);
    uint8_t writer_ctx_set_object_name(struct engine_object_file_writer_ctx_s *wctx, const char *name);
    uint8_t write_object_file(FILE *fp, const struct engine_object_file_writer_ctx_s *object_file_write_ctx);
    uint8_t writer_ctx_add_section(struct engine_object_file_writer_ctx_s *wctx, struct engine_object_file_section_writer_ctx_s *section);
    void delete_writer_ctx(struct engine_object_file_writer_ctx_s *wctx);

    struct engine_object_file_section_writer_ctx_s *new_writer_section(const char *name, void *content_holder);
    uint8_t writer_section_set_name(struct engine_object_file_section_writer_ctx_s *section, const char *name);
    void delete_writer_section(struct engine_object_file_section_writer_ctx_s *section);

    uint32_t add_str_table(const char *str, struct generic_map_s *strndx);

    CNAssetReader *new_object_file_reader(void);
    uint8_t init_object_file_reader(CNAssetReader *reader, char *file_path);
    void uninit_object_file_reader(CNAssetReader *reader);
    void delete_object_file_reader(CNAssetReader *reader);

#endif
