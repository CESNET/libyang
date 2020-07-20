add_test(NAME Headers
	COMMAND ${CMAKE_SOURCE_DIR}/tests/style/check_includes.sh ${CMAKE_SOURCE_DIR}/src/)
