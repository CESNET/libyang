# Correct RPATH usage on OS X
set(CMAKE_MACOSX_RPATH TRUE)

configure_file("${PROJECT_SOURCE_DIR}/tests/tests_config.h.in" "${PROJECT_BINARY_DIR}/tests/tests_config.h" ESCAPE_QUOTES @ONLY)
include_directories(SYSTEM ${CMOCKA_INCLUDE_DIR})
include_directories(${PROJECT_BINARY_DIR}/tests/)

if(WIN32 AND BUILD_SHARED_LIBS)
    # There is no rpath on Windows
    get_property(YANG_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(YANG_MULTI_CONFIG)
        set(YANG_DLL_TESTS_DIR "${CMAKE_BINARY_DIR}/tests/$<CONFIG>")
    else()
        set(YANG_DLL_TESTS_DIR "${CMAKE_BINARY_DIR}/tests")
    endif()
    add_custom_target(yang_dll_for_tests
        COMMAND ${CMAKE_COMMAND} -E make_directory "${YANG_DLL_TESTS_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:yang> "${YANG_DLL_TESTS_DIR}"
        DEPENDS yang)
endif()

function(ly_add_utest)
    cmake_parse_arguments(ADDTEST "" "NAME;WRAP" "SOURCES" ${ARGN})
    set(TEST_NAME utest_${ADDTEST_NAME})

    foreach(TEST_SOURCE ${ADDTEST_SOURCES})
        list(APPEND TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SOURCE})
    endforeach()

    add_executable(${TEST_NAME} ${TEST_SOURCES} ${compatsrc})
    target_link_libraries(${TEST_NAME} yang ${CMOCKA_LIBRARIES})
    if(TARGET yang_dll_for_tests)
        add_dependencies(${TEST_NAME} yang_dll_for_tests)
    endif()

    # Set common attributes of all tests
    set_target_properties(${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
    if (NOT APPLE AND ADDTEST_WRAP)
        set_target_properties(${TEST_NAME} PROPERTIES LINK_FLAGS "${ADDTEST_WRAP}")
    endif()

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT "MALLOC_CHECK_=3")

    if(ENABLE_VALGRIND_TESTS)
        add_test(${TEST_NAME}_valgrind valgrind --leak-check=full --show-leak-kinds=all --suppressions=${PROJECT_SOURCE_DIR}/tests/ld.supp --error-exitcode=1 ${CMAKE_BINARY_DIR}/tests/${TEST_NAME})
    endif()
endfunction()

add_subdirectory(plugins)
add_subdirectory(utests)
if(NOT WIN32)
    add_subdirectory(style)
    add_subdirectory(fuzz)
    add_subdirectory(perf)
endif()
add_subdirectory(yanglint)
add_subdirectory(yangre)
add_subdirectory(yangupdate)

set(format_sources ${format_sources} PARENT_SCOPE)
