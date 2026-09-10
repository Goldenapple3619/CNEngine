find_package(PkgConfig REQUIRED)
pkg_check_modules(CRITERION REQUIRED criterion)

add_executable(test_lib
    test_core.c
)
target_link_libraries(test_lib
    cncore
    ${CRITERION_LIBRARIES}
)
target_include_directories(test_lib PRIVATE ${CRITERION_INCLUDE_DIRS})

target_compile_options(test_lib PRIVATE --coverage -O0 -g)
target_link_options(test_lib PRIVATE --coverage)

add_test(NAME lib_tests COMMAND test_lib --verbose)
