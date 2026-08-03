#ifndef _CNOBJECTOBJ_H_
    #define _CNOBJECTOBJ_H_

    #include "libcnassets.h"

    struct object_element_s {
        char *id;
        char *base;

        struct generic_vector_s methods;
        struct generic_vector_s attributes;
    };

    CN_API struct asset_registry *register_asset(void);
    CN_API void unregister_asset(struct asset_registry *registered_asset);

#endif