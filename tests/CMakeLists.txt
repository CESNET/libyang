cmake_minimum_required(VERSION 2.8.12)

# Correct RPATH usage on OS X
set(CMAKE_MACOSX_RPATH TRUE)

if(ENABLE_STATIC AND ENABLE_VALGRIND_TESTS)
    message(WARNING "Can't run C valgrind tests on a static build")
else()
    find_program(VALGRIND_FOUND valgrind)
endif()

configure_file("${PROJECT_SOURCE_DIR}/tests/config.h.in" "${PROJECT_BINARY_DIR}/tests/config.h" ESCAPE_QUOTES @ONLY)
include_directories(SYSTEM ${CMOCKA_INCLUDE_DIR})
include_directories(${PROJECT_BINARY_DIR})

set(tests)
set(tests_wraps)
add_subdirectory(src)

foreach(test_name IN LISTS tests)
    message(STATUS ${test_name})
    string(REGEX REPLACE "[a-z]*_(.*)" "\\1" name "${test_name}")
    string(REGEX REPLACE "([a-z]*)_.*" "\\1" prefix "${test_name}")
    add_executable(${test_name} ${prefix}/test_${name}.c)
endforeach(test_name)

# Set common attributes of all tests
foreach(test_name IN LISTS tests)
    target_link_libraries(${test_name} ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${PCRE_LIBRARIES} m)
    list(GET tests_wraps 0 test_wrap)
    set_target_properties(${test_name} PROPERTIES LINK_FLAGS "${test_wrap}")
    list(REMOVE_AT tests_wraps 0)
    add_test(NAME ${test_name} COMMAND ${test_name})
#    set_property(TEST ${test_name} PROPERTY ENVIRONMENT "LIBYANG_EXTENSIONS_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/extensions")
#    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT "LIBYANG_USER_TYPES_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/user_types")
    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT "MALLOC_CHECK_=3")
endforeach(test_name)

if(ENABLE_VALGRIND_TESTS)
    if(VALGRIND_FOUND)
        foreach(test_name IN LISTS tests)
            add_test(${test_name}_valgrind valgrind --leak-check=full --show-leak-kinds=all --suppressions=${PROJECT_SOURCE_DIR}/tests/ld.supp --error-exitcode=1
                 ${CMAKE_BINARY_DIR}/tests/${test_name})
#            set_property(TEST ${test_name}_valgrind PROPERTY ENVIRONMENT "LIBYANG_EXTENSIONS_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/extensions")
#            set_property(TEST ${test_name}_valgrind APPEND PROPERTY ENVIRONMENT "LIBYANG_USER_TYPES_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/user_types")
        endforeach(test_name)
    else(VALGRIND_FOUND)
        message(WARNING "valgrind executable not found! Disabling memory leaks tests.")
    endif(VALGRIND_FOUND)
endif()