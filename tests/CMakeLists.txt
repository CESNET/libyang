cmake_minimum_required(VERSION 2.6)

set(TEST_SOURCES
	test_data_initialization.c
)

add_executable(test_data_initialization ${TEST_SOURCES})
target_link_libraries(test_data_initialization ${CMOCKA_LIBRARIES} yang)
add_test(NAME test_data_initialization COMMAND $<TARGET_FILE:test_data_initialization>)
configure_file(config.h.in config.h)

if(ENABLE_VALGRIND_TESTS)
	find_program(valgrind_FOUND valgrind)
	if(valgrind_FOUND)
		add_test(test_data_initialization_valgrind valgrind --leak-check=full --show-leak-kinds=all
			 ${CMAKE_BINARY_DIR}/tests/test_data_initialization)
	else(valgrind_FOUND)
		Message("-- valgrind executable not found! Disabling memory leaks tests")
	endif(valgrind_FOUND)
endif()
