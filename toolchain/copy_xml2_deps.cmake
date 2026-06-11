file(GET_RUNTIME_DEPENDENCIES
    LIBRARIES "${XML2_LIB}"
    RESOLVED_DEPENDENCIES_VAR   xml2_deps
    UNRESOLVED_DEPENDENCIES_VAR xml2_unresolved
)

file(MAKE_DIRECTORY "${DIST_DIR}/lib")

set(SYSTEM_LIB_PATTERNS
    ".*libm\\.so.*"
    ".*libc\\.so.*"
    ".*libpthread\\.so.*"
    ".*libdl\\.so.*"
    ".*ld-linux.*"
    ".*linux-vdso.*"
    ".*libgcc_s.*"
    ".*libstdc\\+\\+.*"
    ".*librt\\.so.*"
)

foreach(dep ${xml2_deps})
    set(_skip FALSE)
    foreach(pattern ${SYSTEM_LIB_PATTERNS})
        if(dep MATCHES "${pattern}")
            set(_skip TRUE)
            break()
        endif()
    endforeach()

    if(NOT _skip)
        get_filename_component(dep_name "${dep}" NAME)
        file(COPY "${dep}" DESTINATION "${DIST_DIR}/lib")
    endif()
endforeach()

foreach(dep ${xml2_unresolved})
    message(WARNING "Unresolved xml2 dep (not packed): ${dep}")
endforeach()