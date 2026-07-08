#ifndef _CNGUIOBJ_H_
    #define _CNGUIOBJ_H_

    #include "libcnassets.h"

    static const char asset_name[] = "gui";

    struct gui_element_s {
        char *id;
        char *object_type;
        char *text_content;

        union {
            const struct gui_element_s *_o;
            const char *_n;
        } parent;

        struct generic_map_s styles;
        struct generic_vector_s connectors;

        cnbool parent_loaded;
    };

    CN_API struct asset_registry *register_asset(void);
    CN_API void unregister_asset(struct asset_registry *registered_asset);

    void init_node_section_registry(struct section_registry *reg);

#endif