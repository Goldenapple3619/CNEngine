#ifndef _CNSCENEOBJ_H_
    #define _CNSCENEOBJ_H_

    #include "libcnassets.h"

    struct scene_element_s {
        char *id;
        char *object_type;

        union {
            const struct scene_element_s *_o;
            const char *_n;
        } parent;

        struct generic_vector_s attributes;

        cnbool parent_loaded;
    };

    CN_API struct asset_registry *register_asset(void);
    CN_API void unregister_asset(struct asset_registry *registered_asset);

    void init_tree_section_registry(struct section_registry *reg);

#endif