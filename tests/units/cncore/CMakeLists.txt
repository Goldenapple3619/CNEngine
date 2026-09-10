find_package(PkgConfig REQUIRED)
pkg_check_modules(CRITERION REQUIRED criterion)

add_executable(test_core
    test_objects.c
)
target_link_libraries(test_core
    cncore
    ${CRITERION_LIBRARIES}
)
target_include_directories(test_core PRIVATE ${CRITERION_INCLUDE_DIRS})

target_compile_options(test_core PRIVATE --coverage -O0 -g)
target_link_options(test_core PRIVATE --coverage)

add_test(NAME cncore_tests COMMAND test_core --verbose)
