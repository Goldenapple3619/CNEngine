project(cnaudio C)

add_library(cnaudio SHARED

)

target_compile_options(cnaudio
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -O2
)

target_include_directories(cnaudio
    PUBLIC
        include
)

target_link_libraries(cnaudio
    PUBLIC
        SDL2::SDL2
)