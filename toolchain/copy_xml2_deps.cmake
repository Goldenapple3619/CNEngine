file(GET_RUNTIME_DEPENDENCIES
    LIBRARIES "${XML2_LIB}"
    RESOLVED_DEPENDENCIES_VAR  xml2_deps
    UNRESOLVED_DEPENDENCIES_VAR xml2_unresolved
    # Don't copy glibc, libm, libpthread — those are guaranteed on any Linux
    EXCLUDE_REGEXES
        ".*libm\\.so.*"
        ".*libc\\.so.*"
        ".*libpthread\\.so.*"
        ".*libdl\\.so.*"
        ".*ld-linux.*"
        ".*linux-vdso.*"
)

file(MAKE_DIRECTORY "${DIST_DIR}/lib")

foreach(dep ${xml2_deps})
    get_filename_component(dep_name "${dep}" NAME)
    message(STATUS "Packing xml2 dep: ${dep_name}")
    file(COPY "${dep}" DESTINATION "${DIST_DIR}/lib")
endforeach()

foreach(dep ${xml2_unresolved})
    message(WARNING "Unresolved xml2 dep (not packed): ${dep}")
endforeach()
