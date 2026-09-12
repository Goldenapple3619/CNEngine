#include "libcnassets.h"
#include "libcncore.h"
#include "cnobjectobj.h"

cn_value _load_object(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't load unspecified object.");
        return (VALUE_NULL);
    }

    CNAssetReader *reader;
    size_t off;
    String str_info = {.c_str = NULL, .size = 0};
    String str_symbols = {.c_str = NULL, .size = 0};
    String str_attributes = {.c_str = NULL, .size = 0};
    struct object_element_s *new_object = NULL;
    struct section_registry *section_handler_info = call_method(__this, "find_section_by_type", PACK_ARG(INLNE_PRIM_T_ARG(ENGINE_SEC_OBJ_INFO))).as.ptr;

    if (!section_handler_info) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }

    struct section_registry *section_handler_symbols = call_method(__this, "find_section_by_type", PACK_ARG(INLNE_PRIM_T_ARG(ENGINE_SEC_OBJ_SYMBOLS))).as.ptr;

    if (!section_handler_symbols) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }

    if (str_override_cp(&str_info, args[0])) {
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    if (str_lcadd_cp(&str_info, "object.")) {
        empty_str(&str_info);
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    if (str_override_cp(&str_symbols, str_info.c_str)) {
        empty_str(&str_info);
        empty_str(&str_attributes);
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    if (str_override_cp(&str_attributes, str_info.c_str)) {
        empty_str(&str_info);
        empty_str(&str_symbols);
        empty_str(&str_attributes);
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    if (str_rcadd_cp(&str_info, ".object_info")) {
        empty_str(&str_info);
        empty_str(&str_symbols);
        empty_str(&str_attributes);
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    if (str_rcadd_cp(&str_symbols, ".object_symbols")) {
        empty_str(&str_info);
        empty_str(&str_symbols);
        empty_str(&str_attributes);
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    if (str_rcadd_cp(&str_attributes, ".object_attributes")) {
        empty_str(&str_info);
        empty_str(&str_symbols);
        empty_str(&str_attributes);
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    cn_value val = call_method(__this, "get_all_asset_pack", PACK_ARG("../game"));
    cn_value container_val;

    if (val.type == CN_TYPE_NULL) {
        empty_str(&str_info);
        empty_str(&str_symbols);
        empty_str(&str_attributes);
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    for (struct list_iterator_s it = list_get_iterator(val.as.ptr); !list_iterator_isend(&it); list_iterator_next(&it)) {
        if (list_iterator_value_isnull(&it))
            continue;

        reader = call_method(__this, "build_reader", PACK_ARG(it.val.as.str)).as.ptr;

        if (!reader) {
            PROPAGATE_ERR();
            collect_object(val.as.ptr);
            return (VALUE_NULL);
        }

        if (!str_is_null(&str_info)) {
            off = 0;
            container_val = call_method(__this, "search_symbol_reader", PACK_ARG(reader, str_info.c_str));

            if (container_val.type != CN_TYPE_NULL) {
                empty_str(&str_info);
                if (section_handler_info->extract_data_chunk) {
                    if (section_handler_info->extract_data_chunk((void **)&new_object, &off, container_val.as.ptr, reader) != ENGINE_DATA_EXTRACT_COMPLETE) {
                        RAISE_FMT(ERR_CORRUPT_OR_INVALID, "failed to extract data from section '%s'.", section_handler_info->name);
                        empty_str(&str_symbols);
                        empty_str(&str_attributes);
                        return (VALUE_NULL);
                    }
                }
            }
        }

        if (!str_is_null(&str_symbols)) {
            off = 0;
            container_val = call_method(__this, "search_symbol_reader", PACK_ARG(reader, str_symbols.c_str));

            if (container_val.type != CN_TYPE_NULL) {
                empty_str(&str_symbols);
                if (section_handler_symbols->extract_data_chunk((void **)&new_object, &off, container_val.as.ptr, reader) != ENGINE_DATA_EXTRACT_COMPLETE) {
                    RAISE_FMT(ERR_CORRUPT_OR_INVALID, "failed to extract data from section '%s'.", section_handler_symbols->name);
                    empty_str(&str_info);
                    empty_str(&str_attributes);
                    return (VALUE_NULL);
                }
            }
        }

        collect_object(val.as.ptr);

        if (str_is_null(&str_info) && str_is_null(&str_symbols)) {
            empty_str(&str_attributes);
            return ((cn_value){.as.ptr = new_object, .type = CN_TYPE_GENERIC_UNIQ_PTR});
        }
    }

    collect_object(val.as.ptr);
    empty_str(&str_info);
    empty_str(&str_symbols);
    empty_str(&str_attributes);
    if (new_object) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "only partial of '%s' found in '%s'.", (const char *)args[0], "../game");
        (void)free(new_object);
    } else {
        RAISE_FMT(ERR_OUT_OF_BOUND, "can't find '%s' in '%s'.", (const char *)args[0], "../game");
    }
    return (VALUE_NULL);
}
