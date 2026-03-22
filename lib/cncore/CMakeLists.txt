project(cncore C)

add_library(cncore SHARED
    src/rect.c
    src/vector2.c
    src/vector3.c
    src/clock.c
    src/object.c
    src/object_vector.c
    src/object_value.c
    src/object_value_vector.c
    src/object_ownership.c
    src/object_attributes.c
    src/object_map.c
    src/core_ctx.c
    src/common.c
)

target_compile_options(cncore
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -O2
)

target_include_directories(cncore
    PUBLIC
        include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cncore
    PUBLIC
        SDL2::SDL2
)