add_test(NAME headers
	COMMAND ${CMAKE_SOURCE_DIR}/tests/style/check_includes.sh ${CMAKE_SOURCE_DIR}/src/ ${CMAKE_SOURCE_DIR}/tools/lint/ ${CMAKE_SOURCE_DIR}/tools/re/)

if (${SOURCE_FORMAT_ENABLED})
	add_test(NAME format WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND make format-check)
endif()
