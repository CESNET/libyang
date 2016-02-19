cmake_minimum_required(VERSION 2.6)

set(data_tests test_data_initialization test_leafref_remove test_instid_remove)
set(schema_yin_tests test_ietf test_augment test_print_transform)

foreach(test_name IN LISTS data_tests)
    add_executable(${test_name} data/${test_name}.c)
    target_link_libraries(${test_name} ${CMOCKA_LIBRARIES} yang)
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
endforeach(test_name)

foreach(test_name IN LISTS schema_yin_tests)
    add_executable(${test_name} schema/yin/${test_name}.c)
    target_link_libraries(${test_name} ${CMOCKA_LIBRARIES} yang)
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
endforeach(test_name)

configure_file("${PROJECT_SOURCE_DIR}/tests/config.h.in" "${PROJECT_SOURCE_DIR}/tests/config.h" ESCAPE_QUOTES @ONLY)

if(ENABLE_VALGRIND_TESTS)
    find_program(valgrind_FOUND valgrind)
    if(valgrind_FOUND)
        foreach(test_name IN LISTS data_tests schema_yin_tests)
            add_test(${test_name}_valgrind valgrind --leak-check=full --show-leak-kinds=all
                 ${CMAKE_BINARY_DIR}/tests/${test_name})
        endforeach(test_name)
    else(valgrind_FOUND)
        Message("-- valgrind executable not found! Disabling memory leaks tests")
    endif(valgrind_FOUND)
endif()
