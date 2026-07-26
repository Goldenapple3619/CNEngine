#include "project_toolchain.h"

EngineGeneratorItem *new_generator_item(void)
{
    EngineGeneratorItem *item = malloc(sizeof(EngineGeneratorItem));

    if (!item) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new generator item");
        return (NULL);
    }

    item->location = NULL;
    item->forsubmodule = NULL;
    item->type = GENT_SRC;
    return (item);
}

void delete_generator_item(EngineGeneratorItem *item)
{
    if (!item) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty item.");
        return;
    }

    if (item->location) {
        (void)free(item->location);
        item->location = NULL;
    }
    if (item->forsubmodule) {
        (void)free(item->forsubmodule);
        item->forsubmodule = NULL;
    }
    (void)free(item);
}

EngineRessourceSet *new_ressource_set(void)
{
    EngineRessourceSet *set = malloc(sizeof(EngineRessourceSet));

    if (!set) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new engine ressource set.");
        return (NULL);
    }

    set->architecture = CNBUILD_ARCH_HOST;
    set->machine = CNBUILD_SYS_HOST;
    set->include_path = NULL;
    set->lib_path = NULL;
    set->toolchain = NULL;
    return (set);
}

void delete_ressource_set(EngineRessourceSet *set)
{
    if (!set) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty ressource set.");
        return;
    }

    if (set->toolchain)
        (void)free(set->toolchain);
    if (set->lib_path)
        (void)free(set->lib_path);
    if (set->include_path)
        (void)free(set->include_path);
    (void)free(set);
}

EngineConfig *new_engine_config(void)
{
    EngineConfig *config = malloc(sizeof(EngineConfig));

    if (!config) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new engine config.");
        return (NULL);
    }

    config->submodules_location = NULL;
    config->generator.capacity = 0;
    config->generator.size = 0;
    config->generator.content = NULL;
    config->ressources.capacity = 0;
    config->ressources.size = 0;
    config->ressources.content = NULL;
    return (config);
}

cnbool has_ressource_set(const EngineConfig *config, cnbuild_architectures arch, cnbuild_system machine)
{
    if (!config) {
        RAISE(ERR_INVALID_POINTER, "can't check if empty config has ressource.");
        return (false);
    }

    EngineRessourceSet *temp;

    for (size_t i = 0; i < config->ressources.size; ++i) {
        temp = config->ressources.content[i];

        if ((temp->architecture == arch || temp->architecture == CNBUILD_ARCH_HOST) && (temp->machine == machine || temp->machine == CNBUILD_SYS_HOST))
            return (true);
    }

    return (false);
}

const EngineRessourceSet *find_ressource_set(const EngineConfig *config, cnbuild_architectures arch, cnbuild_system machine)
{
    if (!config) {
        RAISE(ERR_INVALID_POINTER, "can't get ressource from empty config.");
        return (NULL);
    }

    EngineRessourceSet *temp;

    for (size_t i = 0; i < config->ressources.size; ++i) {
        temp = config->ressources.content[i];

        if ((temp->architecture == arch || temp->architecture == CNBUILD_ARCH_HOST) && (temp->machine == machine || temp->machine == CNBUILD_SYS_HOST))
            return (temp);
    }

    RAISE(ERR_OUT_OF_BOUND, "no ressource found for arch/sys set.");
    return (NULL);
}

void delete_engine_config(EngineConfig *config)
{
    if (!config) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty config.");
        return;
    }

    if (config->submodules_location)
        (void)free(config->submodules_location);
    if (config->ressources.content)
        (void)empty_generic_vector(&config->ressources, (expr_free)&delete_ressource_set);
    if (config->generator.content)
        (void)empty_generic_vector(&config->generator, (expr_free)&delete_generator_item);
    (void)free(config);
}
