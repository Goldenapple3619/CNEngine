#ifndef _LIBCNASSETS_H_
    #define _LIBCNASSETS_H_

    #define ENGINE_USE_LARGE_FILE true
    #define _FILE_OFFSET_BITS 64

    #include "libcncore.h"
    #include <stdlib.h>
    #include <string.h>
    #ifdef _WIN32
        #include <windows.h>
    #endif

    #define ENGINE_OBJ_MAGIC 0x0875C4E3U
    
    #define ENGINE_OBJ_HDR_SZ 31U
    #define ENGINE_OBJ_SECHDR_PREFIX_SZ 16U
    #define ENGINE_OBJ_SECHDR_ENTRY_SZ 26U

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
        ENGINE_OBJ_RAW_RESSOURCES,
        ENGINE_OBJ_ASSET_PACK,
        ENGINE_OBJ_GUI,
        ENGINE_OBJ_SCN,
        ENGINE_OBJ_OBJ,
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
        ENGINE_SEC_GUI_CONN,

        ENGINE_SEC_SCENE_TREE
    } engine_section_type;

    typedef enum {
        ENGINE_SEC_NULL_FLAG = 0x00
    } engine_section_wrt_flags;

    struct engine_object_file_writer_ctx_s;

    struct engine_object_file_section_writer_ctx_s {
        char *section_name;
    
        struct {
            uint16_t type;
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
            uint16_t type;
            uint32_t flags;
        } write_infos;
    
        struct generic_vector_s sections;
    };

    struct engine_obj_header_s {
        uint32_t magic;
        uint8_t endian;
        uint32_t flags;
        uint16_t type;
        uint64_t section_header_off;
        uint64_t strndx_off;
        uint32_t name;
    };

    struct engine_obj_section_header_entry_s {
        uint32_t section_name;
        uint16_t section_type;
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

    typedef uint8_t (*wr8_fn)(FILE *, uint8_t);
    typedef uint8_t (*wr16_fn)(FILE *, uint16_t);
    typedef uint8_t (*wr32_fn)(FILE *, uint32_t);
    typedef uint8_t (*wr64_fn)(FILE *, uint64_t);

    typedef uint8_t (*rd8_fn)(const void *p);
    typedef uint16_t (*rd16_fn)(const void *p);
    typedef uint32_t (*rd32_fn)(const void *p);
    typedef uint64_t (*rd64_fn)(const void *p);

    typedef struct {
        wr8_fn u8;
        wr16_fn u16;
        wr32_fn u32;
        wr64_fn u64;
    } fpio_handler_t;

    typedef struct {
        rd8_fn u8;
        rd16_fn u16;
        rd32_fn u32;
        rd64_fn u64;
    } mapio_reader_t;

    typedef struct {
        struct {
            const void *mapped_area;
            size_t size;

            #ifdef _WIN32
                HANDLE file;
                HANDLE mapping;
            #else
                int fd;
            #endif
            cnbool ready;
        } _content;

        struct engine_obj_header_s header;
        struct engine_obj_section_header_s section_header;

        mapio_reader_t read_handler;
    } CNAssetReader;

    struct section_blk {
        const uint8_t *section_blk_ptr;

        uint64_t blk_size;
    };

    struct section_registry {
        const char *name;

        uint16_t section_type;

        char *(*data_builder)(struct engine_object_file_section_writer_ctx_s *self, const struct engine_object_file_writer_ctx_s *wctx, struct generic_map_s *strndx);
        uint64_t (*size_compute)(struct engine_object_file_section_writer_ctx_s *self);
        char *(*strndx_reconstructor)(char *rw_content, uint64_t content_size, const CNAssetReader *reader, struct generic_map_s *new_strndx);
    };

    struct asset_registry {
        const char *name;

        uint16_t asset_type;

        struct generic_vector_s registered_sections;
    };

    struct asset_loader_s {
        void *dl_handle;
        struct asset_registry *registry;

        struct asset_registry *(*register_asset)(void);
        void (*unregister_asset)(struct asset_registry *);
    };

    CN_API uint8_t fpwr_u8(FILE *fp, uint8_t v);
    CN_API uint8_t fpwr_u16_be(FILE *fp, uint16_t v);
    CN_API uint8_t fpwr_u16_le(FILE *fp, uint16_t v);
    CN_API uint8_t fpwr_u32_be(FILE *fp, uint32_t v);
    CN_API uint8_t fpwr_u32_le(FILE *fp, uint32_t v);
    CN_API uint8_t fpwr_u64_be(FILE *fp, uint64_t v);
    CN_API uint8_t fpwr_u64_le(FILE *fp, uint64_t v);

    CN_API void bufwr_u8(char *buf, uint8_t v);
    CN_API void bufwr_u16_be(char *buf, uint16_t v);
    CN_API void bufwr_u16_le(char *buf, uint16_t v);
    CN_API void bufwr_u32_be(char *buf, uint32_t v);
    CN_API void bufwr_u32_le(char *buf, uint32_t v);
    CN_API void bufwr_u64_be(char *buf, uint64_t v);
    CN_API void bufwr_u64_le(char *buf, uint64_t v);

    CN_API uint8_t bufrd_u8(const void *p);
    CN_API uint16_t bufrd_u16_be(const void *p);
    CN_API uint16_t bufrd_u16_le(const void *p);
    CN_API uint32_t bufrd_u32_be(const void *p);
    CN_API uint32_t bufrd_u32_le(const void *p);
    CN_API uint64_t bufrd_u64_be(const void *p);
    CN_API uint64_t bufrd_u64_le(const void *p);

    CN_API fpio_handler_t fpio_handler_from_writer(const struct engine_object_file_writer_ctx_s *object_file_write_ctx);

    CN_API struct engine_object_file_writer_ctx_s *new_writer_ctx(const char *name);
    CN_API uint8_t writer_ctx_set_object_name(struct engine_object_file_writer_ctx_s *wctx, const char *name);
    CN_API uint8_t write_object_file(FILE *fp, const struct engine_object_file_writer_ctx_s *object_file_write_ctx);
    CN_API uint8_t writer_ctx_add_section(struct engine_object_file_writer_ctx_s *wctx, struct engine_object_file_section_writer_ctx_s *section);
    CN_API void delete_writer_ctx(struct engine_object_file_writer_ctx_s *wctx);

    CN_API struct engine_object_file_section_writer_ctx_s *new_writer_section(const char *name, void *content_holder);
    CN_API uint8_t writer_section_set_name(struct engine_object_file_section_writer_ctx_s *section, const char *name);
    CN_API void delete_writer_section(struct engine_object_file_section_writer_ctx_s *section);
    CN_API uint32_t flags_to_align(uint32_t flags);

    CN_API uint32_t add_str_table(const char *str, struct generic_map_s *strndx);

    CN_API CNAssetReader *new_object_file_reader(void);
    CN_API const char *object_file_reader_get_string(const CNAssetReader *reader, uint32_t off);
    CN_API void object_file_reader_get_section(const CNAssetReader *reader, struct section_blk *section_block, uint64_t section_index);
    CN_API uint8_t object_file_reader_read_header(CNAssetReader *reader);
    CN_API uint8_t object_file_reader_read_section_header(CNAssetReader *reader);
    CN_API uint8_t init_object_file_reader(CNAssetReader *reader, const char *file_path);
    CN_API void uninit_object_file_reader(CNAssetReader *reader);
    CN_API void delete_object_file_reader(CNAssetReader *reader);
    CN_API void print_object_file(CNAssetReader *reader);

    CN_API Object *new_asset_submodule(void);

    CN_API struct asset_loader_s *new_asset_loader(void);
    CN_API uint8_t asset_loader_init(struct asset_loader_s *loader, const char *path);
    CN_API void asset_loader_uninit(struct asset_loader_s *loader);
    CN_API void delete_asset_loader(struct asset_loader_s *loader);

    CN_API uint8_t section_registry_to_wctx_section(const char *section_name, void *content, struct section_registry *reg, struct engine_object_file_writer_ctx_s *wctx);

    CN_API cn_type typename_from_string(const char *str);
    CN_API uint8_t value_from_string(const char *str, cn_type type, cn_value *val);
#endif
