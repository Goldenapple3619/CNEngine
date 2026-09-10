project(cngraphic C)

find_package(OpenGL REQUIRED)

add_library(cngraphic SHARED
    src/texture.c
    src/window.c
    src/common.c
    src/graphic_submodule_ctx.c
    src/window_universe.c
    src/event.c
    src/event_map.c
    src/interface.c
    src/material.c
    src/mesh.c
    src/shader.c
    src/quad.c
)

if(CN_BUILD_TESTS)
    target_compile_options(cngraphic
        PRIVATE
            --coverage -O0 -g
    )
    target_link_options(cngraphic PRIVATE --coverage)
else()
    target_compile_options(cngraphic
        PRIVATE
            -Wall -Wextra -Wshadow -Werror -O2
    )
endif()

target_include_directories(cngraphic
    PUBLIC
        include
        ../cncore/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cngraphic
    PUBLIC
        cncore
        SDL2::SDL2
        SDL2_image::SDL2_image
        OpenGL::GL
        glad_gl
)