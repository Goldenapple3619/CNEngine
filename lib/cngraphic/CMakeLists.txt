project(cngraphic C)

add_library(cngraphic SHARED
    src/texture.c
    src/window.c
    src/common.c
    src/graphic_submodule_ctx.c
    src/window_universe.c
    src/event.c
    src/event_map.c
    src/interface.c
    src/texture_atlas.c
)

target_compile_options(cngraphic
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -O2
)

target_include_directories(cngraphic
    PUBLIC
        include
        ../cncore/include
)

target_link_libraries(cngraphic
    PUBLIC
        SDL2::SDL2
        SDL2_image::SDL2_image
)