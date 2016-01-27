cmake_minimum_required(VERSION 2.6)

add_executable(test_data_initialization data/test_data_initialization.c)
target_link_libraries(test_data_initialization ${CMOCKA_LIBRARIES} yang)
add_test(NAME test_data_initialization COMMAND $<TARGET_FILE:test_data_initialization>)

add_executable(test_leafref_remove data/test_leafref_remove.c)
target_link_libraries(test_leafref_remove ${CMOCKA_LIBRARIES} yang)
add_test(NAME test_leafref_remove COMMAND $<TARGET_FILE:test_leafref_remove>)

add_executable(test_ietf schema/yin/test_ietf.c)
target_link_libraries(test_ietf ${CMOCKA_LIBRARIES} yang)
add_test(NAME test_ietf COMMAND $<TARGET_FILE:test_ietf>)

add_executable(test_augment schema/yin/test_augment.c)
target_link_libraries(test_augment ${CMOCKA_LIBRARIES} yang)
add_test(NAME test_augment COMMAND $<TARGET_FILE:test_augment>)

configure_file("${PROJECT_SOURCE_DIR}/tests/config.h.in" "${PROJECT_SOURCE_DIR}/tests/config.h" ESCAPE_QUOTES @ONLY)

if(ENABLE_VALGRIND_TESTS)
	find_program(valgrind_FOUND valgrind)
	if(valgrind_FOUND)
		add_test(test_data_initialization_valgrind valgrind --leak-check=full --show-leak-kinds=all
			 ${CMAKE_BINARY_DIR}/tests/test_data_initialization)
        add_test(test_augment_valgrind valgrind --leak-check=full --show-leak-kinds=all
             ${CMAKE_BINARY_DIR}/tests/test_augment)
	else(valgrind_FOUND)
		Message("-- valgrind executable not found! Disabling memory leaks tests")
	endif(valgrind_FOUND)
endif()
