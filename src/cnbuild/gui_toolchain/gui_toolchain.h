#ifndef _GUI_TOOLCHAIN_H_
    #define _GUI_TOOLCHAIN_H_

    #include "../build.h"

    struct parsed_gui_element_data_s {
        char *id;
        char *object_type;
        char *text_content;

        const struct parsed_gui_element_data_s *parent;
        struct generic_map_s styles;
    };

    void delete_parsed_gui(struct parsed_gui_element_data_s *parsed_gui);
    uint8_t fill_gui_element(xmlNode *node, struct parsed_gui_element_data_s *element);
    uint64_t generate_gui_node_section_size(struct engine_object_file_section_writer_ctx_s *self);
    char *generate_gui_node_section_content(struct engine_object_file_section_writer_ctx_s *self, const struct engine_object_file_writer_ctx_s *wctx, struct generic_map_s *strndx);
    uint8_t element_gui_to_wctx(struct generic_vector_s *element, struct engine_object_file_writer_ctx_s *wctx);
    uint8_t build_gui_element(xmlNode *node, struct generic_vector_s *parsed_data, struct parsed_gui_element_data_s *parent);
    uint8_t parse_gui(const char *file_path, struct engine_object_file_writer_ctx_s *wctx);

#endif