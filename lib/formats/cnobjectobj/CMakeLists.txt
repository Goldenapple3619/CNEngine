project(cnobjectobj C)

add_library(cnobjectobj SHARED
    src/interface.c
)

target_compile_options(cnobjectobj
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -O2
)

target_include_directories(cnobjectobj
    PUBLIC
        include
)

target_link_libraries(cnobjectobj
    PUBLIC
        cnassets
)