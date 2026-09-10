#ifndef _OBJECT_TOOLCHAIN_H_
    #define _OBJECT_TOOLCHAIN_H_

    #include "../build.h"

    #include "cnobjectobj.h"

    void delete_object_element_data(struct object_element_s *obj_data);
    uint8_t parse_object_element_methods_xml(xmlNode *node, struct object_element_s *obj_data);
    uint8_t parse_object_element_attributes_xml(xmlNode *node, struct object_element_s *obj_data);
    struct object_element_s *parse_object_element_data_xml(const char *object_path);
    struct object_element_s *parse_object(const char *object_path, struct engine_object_file_writer_ctx_s *wctx, Object *asset_ctx);
#endif