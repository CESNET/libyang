
set(format_sources
    ${format_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
    PARENT_SCOPE)

function(lyu_add_test)
    set(YANGUPDATE_PLUGINS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ARGV0}/plugins)
    set(YANGUPDATE_TEST_LIB yutest_${ARGV0}_lib)
    add_subdirectory(${PROJECT_SOURCE_DIR}/tools/update ${CMAKE_BINARY_DIR}/tests/yangupdate/${ARGV0})

    set(TEST_NAME yangupdate_${ARGV0})
    add_executable(${TEST_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${ARGV0}/test.c)
    target_include_directories(${TEST_NAME} PRIVATE ${PROJECT_BINARY_DIR}/libyang ${PROJECT_SOURCE_DIR}/tools/update)
    target_link_libraries(${TEST_NAME} ${YANGUPDATE_TEST_LIB} ${CMOCKA_LIBRARIES})
    set_target_properties(${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT "MALLOC_CHECK_=3")
    if(ENABLE_VALGRIND_TESTS)
        add_test(${TEST_NAME}_valgrind valgrind --leak-check=full --show-leak-kinds=all --suppressions=${PROJECT_SOURCE_DIR}/tests/ld.supp --error-exitcode=1 ${CMAKE_BINARY_DIR}/tests/${TEST_NAME})
    endif()
endfunction()

lyu_add_test(find_mod1)
lyu_add_test(complex_update)
