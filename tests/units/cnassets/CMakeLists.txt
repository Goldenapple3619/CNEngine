find_package(PkgConfig REQUIRED)
pkg_check_modules(CRITERION REQUIRED criterion)

add_executable(test_assets
    test_io.c
)
target_link_libraries(test_assets
    cncore
    cnassets
    ${CRITERION_LIBRARIES}
)
target_include_directories(test_assets PRIVATE ${CRITERION_INCLUDE_DIRS})

target_compile_options(test_assets PRIVATE --coverage -O0 -g)
target_link_options(test_assets PRIVATE --coverage)

add_test(NAME cnassets_tests COMMAND test_assets --verbose)
