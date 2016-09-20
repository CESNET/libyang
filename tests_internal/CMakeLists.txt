cmake_minimum_required(VERSION 2.8.9)

set(internal_tests
  test_lys_find_grouping_up
  test_lys_node_switch
  test_lyd_list_equal
  test_lyd_attr_parent
  test_lyd_check_mandatory_tree)

include_directories(SYSTEM ${CMOCKA_INCLUDE_DIR})

configure_file("${PROJECT_SOURCE_DIR}/tests_internal/config.h.in" "${PROJECT_SOURCE_DIR}/tests_internal/config.h" ESCAPE_QUOTES @ONLY)

foreach(test_name IN LISTS internal_tests)
    add_executable(${test_name} $<TARGET_OBJECTS:yangobj> tree_internal/${test_name}.c)
    target_link_libraries(${test_name} ${CMOCKA_LIBRARIES} yang)
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
endforeach(test_name)

if(ENABLE_VALGRIND_TESTS)
    find_program(valgrind_FOUND valgrind)
    if(valgrind_FOUND)
        foreach(test_name IN LISTS internal_tests)
            add_test(${test_name}_valgrind valgrind --leak-check=full --show-leak-kinds=all
                 ${CMAKE_BINARY_DIR}/tests_internal/${test_name})
        endforeach(test_name)
    else(valgrind_FOUND)
        Message("-- valgrind executable not found! Disabling memory leaks tests")
    endif(valgrind_FOUND)
endif()
