project(cngraphic C)

add_library(cngraphic SHARED
    src/texture.c
)

target_compile_options(cngraphic
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -pedantic -pedantic-errors -O2
)

target_include_directories(cngraphic
    PUBLIC
        include
        ../cncore/include
)

target_link_libraries(cngraphic
    PUBLIC
        SDL2::SDL2
)