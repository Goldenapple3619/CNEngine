project(cnguiobj C)

add_library(cnguiobj SHARED
    src/interface.c
    src/node_section.c
    src/style_section.c
    src/connector_section.c
)

target_compile_options(cnguiobj
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -O2
)

target_include_directories(cnguiobj
    PUBLIC
        include
)

target_link_libraries(cnguiobj
    PUBLIC
        cnassets
)