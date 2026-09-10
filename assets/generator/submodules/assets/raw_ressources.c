#include "libcnassets.h"

cn_value _load_ressource(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't load unspecified ressource.");
        return (VALUE_NULL);
    }

    String str = {.c_str = NULL, .size = 0};

    if (str_override_cp(&str, args[0])) {
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    if (str_lcadd_cp(&str, "raw.rwa.")) {
        empty_str(&str);
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    cn_value val = call_method(__this, "get_all_asset_pack", PACK_ARG("../game"));
    cn_value container_val;

    if (val.type == CN_TYPE_NULL) {
        empty_str(&str);
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    for (struct list_iterator_s it = list_get_iterator(val.as.ptr); !list_iterator_isend(&it); list_iterator_next(&it)) {
        if (list_iterator_value_isnull(&it))
            continue;

        container_val = call_method(__this, "search_symbol_container", PACK_ARG(it.val.as.str, str.c_str));

        if (container_val.type == CN_TYPE_NULL) {
            PROPAGATE_ERR();
            continue;
        }

        collect_object(val.as.ptr);
        empty_str(&str);

        return (container_val);
    }

    RAISE_FMT(ERR_OUT_OF_BOUND, "can't find '%s' in '%s'.", (const char *)args[0], "../game");
    collect_object(val.as.ptr);
    empty_str(&str);
    return (VALUE_NULL);
}