if(NOT DEFINED INCLUDE_DIR)
    message(FATAL_ERROR "INCLUDE_DIR must be defined")
endif()

set(SDL2_SUBDIR "${INCLUDE_DIR}/SDL2")

if(NOT EXISTS "${SDL2_SUBDIR}")
    message(WARNING "${SDL2_SUBDIR} does not exist, nothing to patch")
    return()
endif()

file(GLOB SDL2_HEADERS RELATIVE "${SDL2_SUBDIR}" "${SDL2_SUBDIR}/*.h")

if(NOT SDL2_HEADERS)
    message(WARNING "No headers found in ${SDL2_SUBDIR}, nothing to patch")
    return()
endif()

set(FILES_TO_PATCH
    "${INCLUDE_DIR}/SDL_image.h"
    "${INCLUDE_DIR}/SDL_ttf.h"
    "${INCLUDE_DIR}/SDL_mixer.h"
)

foreach(target_file ${FILES_TO_PATCH})
    if(NOT EXISTS "${target_file}")
        message(WARNING "${target_file} not found, skipping")
        continue()
    endif()

    get_filename_component(target_name "${target_file}" NAME)

    file(READ "${target_file}" file_contents)
    set(original_contents "${file_contents}")

    foreach(hdr ${SDL2_HEADERS})
        if(NOT hdr STREQUAL target_name)
            string(REPLACE "." "\\." hdr_escaped "${hdr}")

            string(REGEX REPLACE
                "#include([ \t]*)\"${hdr_escaped}\""
                "#include\\1\"SDL2/${hdr}\""
                file_contents "${file_contents}")

            string(REGEX REPLACE
                "#include([ \t]*)<${hdr_escaped}>"
                "#include\\1<SDL2/${hdr}>"
                file_contents "${file_contents}")
        endif()
    endforeach()

    if(NOT file_contents STREQUAL original_contents)
        file(WRITE "${target_file}" "${file_contents}")
    endif()
endforeach()
