#include "cnobjectobj.h"

CN_API struct asset_registry *register_asset(void)
{
    struct asset_registry *object_asset = (struct asset_registry *)malloc(sizeof(struct asset_registry));
    struct section_registry *temp_section_registry;

    if (!object_asset)
        return (NULL);

    object_asset->name = "object";
    object_asset->asset_type = ENGINE_OBJ_OBJ;
    object_asset->registered_sections.capacity = 0;
    object_asset->registered_sections.size = 0;
    object_asset->registered_sections.content = NULL;

    temp_section_registry = (struct section_registry *)malloc(sizeof(struct section_registry));

    if (!temp_section_registry) {
        (void)free(object_asset);
        return (NULL);
    }

    (void)init_info_section_registry(temp_section_registry);

    if (insert_generic_vector(&object_asset->registered_sections, temp_section_registry)) {
        (void)free(temp_section_registry);
        (void)free(object_asset);
        return (NULL);
    }

    temp_section_registry = (struct section_registry *)malloc(sizeof(struct section_registry));

    if (!temp_section_registry) {
        (void)empty_generic_vector(&object_asset->registered_sections, &free);
        (void)free(object_asset);
        return (NULL);
    }

    (void)init_symbols_section_registry(temp_section_registry);

    if (insert_generic_vector(&object_asset->registered_sections, temp_section_registry)) {
        (void)free(temp_section_registry);
        (void)empty_generic_vector(&object_asset->registered_sections, &free);
        (void)free(object_asset);
        return (NULL);
    }

    temp_section_registry = (struct section_registry *)malloc(sizeof(struct section_registry));

    if (!temp_section_registry) {
        (void)empty_generic_vector(&object_asset->registered_sections, &free);
        (void)free(object_asset);
        return (NULL);
    }

    (void)init_attrs_section_registry(temp_section_registry);

    if (insert_generic_vector(&object_asset->registered_sections, temp_section_registry)) {
        (void)free(temp_section_registry);
        (void)empty_generic_vector(&object_asset->registered_sections, &free);
        (void)free(object_asset);
        return (NULL);
    }

    return (object_asset);
}

CN_API void unregister_asset(struct asset_registry *registered_asset)
{
    if (!registered_asset)
        return;
    if (registered_asset->registered_sections.content)
        (void)empty_generic_vector(&registered_asset->registered_sections, &free);
    (void)free(registered_asset);
}
