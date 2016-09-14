cmake_minimum_required(VERSION 2.6)

set(internal_tests
  test_lys_find_grouping_up
  test_lys_node_switch
  test_lyd_list_equal
  test_lyd_attr_parent)

include_directories(SYSTEM ${CMOCKA_INCLUDE_DIR})

foreach(src IN LISTS libsrc)
    list(APPEND src_internal "../${src}")
endforeach(src)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=default")
add_library(yang_internal ${src_internal})
target_link_libraries(yang_internal yang)

foreach(test_name IN LISTS internal_tests)
    add_executable(${test_name} tree_internal/${test_name}.c)
    target_link_libraries(${test_name} ${CMOCKA_LIBRARIES} yang_internal)
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
