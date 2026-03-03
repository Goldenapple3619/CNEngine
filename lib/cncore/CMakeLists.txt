project(cncore C)

add_library(cncore SHARED
    src/rect.c
    src/vector2.c
    src/vector3.c
    src/clock.c
    src/object.c
)

target_compile_options(cncore
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -pedantic -pedantic-errors -O2
)

target_include_directories(cncore
    PUBLIC
        include
)

target_link_libraries(cncore
    PUBLIC
        SDL2::SDL2
)