cmake_minimum_required(VERSION 2.6)

set(api_tests test_libyang test_tree_schema test_xml test_dict test_tree_data test_xpath test_diff test_merge)
set(data_tests test_data_initialization test_leafref_remove test_instid_remove test_keys test_autodel test_when test_defaults test_emptycont test_unique test_mandatory test_json test_parse_print)
set(schema_yin_tests test_print_transform)
set(schema_tests test_ietf test_augment test_typedef test_import test_include)
set(conformance_tests test_sec6_1_1 test_sec6_2 test_sec5_1 test_sec5_5 test_sec6_1_3 test_sec6_2_1 test_sec7_1 test_sec7_2 test_sec7_3 test_sec7_3_1 test_sec7_3_4 test_sec7_5_2 test_sec7_5_4 test_sec7_5_5 test_sec7_6_2 test_sec7_6_3 test_sec7_6_5 test_sec7_7_2 test_sec7_7_3 test_sec7_7_4 test_sec7_7_5 test_sec7_8_1 test_sec7_8_2 test_sec7_8_3 test_sec7_9_1 test_sec7_9_2 test_sec7_9_3 test_sec7_9_4 test_sec7_10 test_sec7_11 test_sec7_12_1 test_sec7_13_1 test_sec7_15 test_sec7_16_1 test_sec7_16_2 test_sec7_18_1 test_sec7_18_2 test_sec7_18_3_1 test_sec7_18_3_2 test_sec7_19_1 test_sec7_19_5 test_sec9_2)

include_directories(SYSTEM ${CMOCKA_INCLUDE_DIR})

foreach(test_name IN LISTS api_tests)
    add_executable(${test_name} api/${test_name}.c)
    target_link_libraries(${test_name} ${CMOCKA_LIBRARIES} yang)
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
endforeach(test_name)

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

foreach(test_name IN LISTS schema_tests)
    add_executable(${test_name} schema/${test_name}.c)
    target_link_libraries(${test_name} ${CMOCKA_LIBRARIES} yang)
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
endforeach(test_name)

foreach(test_name IN LISTS conformance_tests)
    add_executable(${test_name} conformance/${test_name}.c)
    target_link_libraries(${test_name} ${CMOCKA_LIBRARIES} yang)
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
endforeach(test_name)

configure_file("${PROJECT_SOURCE_DIR}/tests/config.h.in" "${PROJECT_SOURCE_DIR}/tests/config.h" ESCAPE_QUOTES @ONLY)

if(ENABLE_VALGRIND_TESTS)
    find_program(valgrind_FOUND valgrind)
    if(valgrind_FOUND)
        foreach(test_name IN LISTS api_tests data_tests schema_yin_tests schema_tests conformance_tests)
            add_test(${test_name}_valgrind valgrind --leak-check=full --show-leak-kinds=all
                 ${CMAKE_BINARY_DIR}/tests/${test_name})
        endforeach(test_name)
    else(valgrind_FOUND)
        Message("-- valgrind executable not found! Disabling memory leaks tests")
    endif(valgrind_FOUND)
endif()
