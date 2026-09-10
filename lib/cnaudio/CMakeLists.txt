project(cnaudio C)

add_library(cnaudio SHARED
    src/common.c
    src/audio.c
    src/audio_sequence.c
    src/audio_ctx.c
    src/audio_vector.c
)

if(CN_BUILD_TESTS)
    target_compile_options(cnaudio
        PRIVATE
            --coverage -O0 -g
    )
    target_link_options(cnaudio PRIVATE --coverage)
else()
    target_compile_options(cnaudio
        PRIVATE
            -Wall -Wextra -Wshadow -Werror -O2
    )
endif()

target_include_directories(cnaudio
    PUBLIC
        include
        ../cncore/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cnaudio
    PUBLIC
        cncore
        SDL2::SDL2
        SDL2_mixer::SDL2_mixer
        m
)