cmake_minimum_required(VERSION 3.20)

project(CNEngine
    VERSION 0.1
    LANGUAGES C
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)


# ========= Options =========
option(CN_BUILD_GAME "Build game executable" ON)

# ========= Fetch SDL2 =========
include(FetchContent)

set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.10
)
FetchContent_MakeAvailable(SDL2)

set(SDL2IMAGE_VENDORED ON CACHE BOOL "" FORCE)
set(SDL2IMAGE_WEBP OFF CACHE BOOL "" FORCE)

if(WIN32)
    set(SDL2IMAGE_EXTERNAL_PROJECT_CMAKE_ARGS
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/mingw_toolchain.cmake
    )
endif()

FetchContent_Declare(
    SDL2_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-2.8.8
)
FetchContent_MakeAvailable(SDL2_image)

set(SDL2TTF_VENDORED ON CACHE BOOL "" FORCE)
set(SDL2TTF_HARFBUZZ ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL2_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-2.24.0
)
FetchContent_MakeAvailable(SDL2_ttf)

set(SDL2MIXER_WAVPACK OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_VENDORED ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL2_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG release-2.8.1
)

FetchContent_MakeAvailable(SDL2_mixer)


# ========= Modules =========
add_subdirectory(lib/cncore)
add_subdirectory(lib/cngraphic)
add_subdirectory(lib/cnaudio)
# add_subdirectory(lib/cninput)

# ========= Game executable =========
if(CN_BUILD_GAME)
    add_executable(cn_game
        main.c
    )

    target_compile_options(cn_game
        PRIVATE
            -Wall -Wextra -Wshadow -Werror -O2
    )

    target_link_libraries(cn_game
        PRIVATE
            cncore
            cngraphic
            cnaudio
            # cninput
            SDL2::SDL2
            SDL2_image::SDL2_image
            SDL2_mixer::SDL2_mixer
    )

    target_include_directories(cn_game
        PRIVATE
            lib/cncore/include
            lib/cngraphic/include
            lib/cnaudio/include
            # lib/cninput/include
    )
endif()