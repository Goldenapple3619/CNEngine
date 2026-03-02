project(cninput C)

add_library(cninput SHARED

)

target_compile_options(cninput
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -pedantic -pedantic-errors -O2
)

target_include_directories(cninput
    PUBLIC
        include
)

target_link_libraries(cninput
    PUBLIC
        SDL2::SDL2
)