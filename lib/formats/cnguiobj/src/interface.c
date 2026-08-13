#include "cnguiobj.h"

CN_API struct asset_registry *register_asset(void)
{
    struct asset_registry *gui_asset = (struct asset_registry *)malloc(sizeof(struct asset_registry));
    struct section_registry *temp_section_registry;

    if (!gui_asset)
        return (NULL);

    gui_asset->name = "gui";
    gui_asset->asset_type = ENGINE_OBJ_GUI;
    gui_asset->registered_sections.capacity = 0;
    gui_asset->registered_sections.size = 0;
    gui_asset->registered_sections.content = NULL;

    temp_section_registry = (struct section_registry *)malloc(sizeof(struct section_registry));

    if (!temp_section_registry) {
        (void)free(gui_asset);
        return (NULL);
    }

    (void)init_node_section_registry(temp_section_registry);
    
    if (insert_generic_vector(&gui_asset->registered_sections, temp_section_registry)) {
        (void)free(temp_section_registry);
        (void)free(gui_asset);
        return (NULL);
    }

    return (gui_asset);
}

CN_API void unregister_asset(struct asset_registry *registered_asset)
{
    if (!registered_asset)
        return;
    if (registered_asset->registered_sections.content)
        (void)empty_generic_vector(&registered_asset->registered_sections, &free);
    (void)free(registered_asset);
}
