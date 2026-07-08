#ifndef _GUI_TOOLCHAIN_H_
    #define _GUI_TOOLCHAIN_H_

    #include "../build.h"

    #include "cnguiobj.h"

    void delete_parsed_gui(struct gui_element_s *parsed_gui);
    uint8_t fill_gui_element(xmlNode *node, struct gui_element_s *element);
    uint8_t build_gui_element(xmlNode *node, struct generic_vector_s *parsed_data, struct gui_element_s *parent);
    struct generic_vector_s *parse_xml_gui(const char *file_path, struct engine_object_file_writer_ctx_s *wctx);
    struct generic_vector_s *parse_gui(const char *file_path, struct engine_object_file_writer_ctx_s *wctx, Object *asset_ctx);

#endif