#include "libcnassets.h"

#include <string.h>

CN_API struct asset_loader_s *new_asset_loader(void)
{
    struct asset_loader_s *loader = malloc(sizeof(struct asset_loader_s));
    
    if (!loader)
        return (NULL);

    memset(loader, 0, sizeof(*loader));

    return (loader);
}

CN_API uint8_t asset_loader_init(struct asset_loader_s *loader, const char *path)
{
    if (!loader || !path)
        return (1);

    memset(loader, 0, sizeof(*loader));

    loader->dl_handle = cnopen_library(path);

    if (!loader->dl_handle)
        return (1);

    loader->register_asset = (struct asset_registry *(*)(void))cnget_symbol(loader->dl_handle, "register_asset");
    loader->unregister_asset = (void (*)(struct asset_registry *))cnget_symbol(loader->dl_handle, "unregister_asset");

    if (!loader->register_asset || !loader->unregister_asset) {
        asset_loader_uninit(loader);
        return (1);
    }

    loader->registry = loader->register_asset();

    if (!loader->registry) {
        asset_loader_uninit(loader);
        return (1);
    }

    return (0);
}

CN_API void asset_loader_uninit(struct asset_loader_s *loader)
{
    if (!loader)
        return;

    if (loader->registry && loader->unregister_asset)
    {
        loader->unregister_asset(loader->registry);
    }

    if (loader->dl_handle)
    {
        cnclose_library(loader->dl_handle);
    }

    memset(loader, 0, sizeof(*loader));
}

CN_API void delete_asset_loader(struct asset_loader_s *loader)
{
    if (!loader)
        return;

    (void)asset_loader_uninit(loader);
    (void)free(loader);
}

CN_API uint8_t section_registry_to_wctx_section(const char *section_name, void *content, struct section_registry *reg, struct engine_object_file_writer_ctx_s *wctx)
{
    struct engine_object_file_section_writer_ctx_s *section;

    if (!reg || !wctx)
        return (1);

    section = new_writer_section(section_name, content);
    
    if (!section)
        return (1);

    section->write_infos.type = reg->section_type;

    if (writer_ctx_add_section(wctx, section)) {
        delete_writer_section(section);
        return (1);
    }

    section->content_generator = reg->data_builder;
    section->content_size_generator = reg->size_compute;

    return (0);
}