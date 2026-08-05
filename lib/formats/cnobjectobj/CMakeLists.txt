project(cnobjectobj C)

add_library(cnobjectobj SHARED
    src/interface.c
    src/info_section.c
    src/attributes_section.c
    src/symbols_section.c
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