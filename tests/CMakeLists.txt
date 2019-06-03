cmake_minimum_required(VERSION 2.8.12)

# Correct RPATH usage on OS X
set(CMAKE_MACOSX_RPATH TRUE)

if(ENABLE_STATIC AND ENABLE_VALGRIND_TESTS)
    message(WARNING "Can't run C valgrind tests on a static build")
else()
    find_program(VALGRIND_FOUND valgrind)
endif()

configure_file("${PROJECT_SOURCE_DIR}/tests/config.h.in" "${PROJECT_BINARY_DIR}/tests/config.h" ESCAPE_QUOTES @ONLY)
include_directories(SYSTEM ${CMOCKA_INCLUDE_DIR})
include_directories(${PROJECT_BINARY_DIR})

set(tests)
set(tests_wraps)
add_subdirectory(src)
add_subdirectory(features)

foreach(test_name IN LISTS tests)
    message(STATUS ${test_name})
    string(REGEX REPLACE "[a-z]*_(.*)" "\\1" name "${test_name}")
    string(REGEX REPLACE "([a-z]*)_.*" "\\1" prefix "${test_name}")
    add_executable(${test_name} ${prefix}/test_${name}.c $<TARGET_OBJECTS:yangobj>)
endforeach(test_name)

# Set common attributes of all tests
foreach(test_name IN LISTS tests)
    target_link_libraries(${test_name} ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${PCRE2_LIBRARIES} m)
    if (NOT APPLE)
        list(GET tests_wraps 0 test_wrap)
        set_target_properties(${test_name} PROPERTIES LINK_FLAGS "${test_wrap}")
        list(REMOVE_AT tests_wraps 0)
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name})
#    set_property(TEST ${test_name} PROPERTY ENVIRONMENT "LIBYANG_EXTENSIONS_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/extensions")
#    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT "LIBYANG_USER_TYPES_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/user_types")
    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT "MALLOC_CHECK_=3")
endforeach(test_name)

if(ENABLE_VALGRIND_TESTS)
    if(VALGRIND_FOUND)
        foreach(test_name IN LISTS tests)
            add_test(${test_name}_valgrind valgrind --leak-check=full --show-leak-kinds=all --suppressions=${PROJECT_SOURCE_DIR}/tests/ld.supp --error-exitcode=1
                 ${CMAKE_BINARY_DIR}/tests/${test_name})
#            set_property(TEST ${test_name}_valgrind PROPERTY ENVIRONMENT "LIBYANG_EXTENSIONS_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/extensions")
#            set_property(TEST ${test_name}_valgrind APPEND PROPERTY ENVIRONMENT "LIBYANG_USER_TYPES_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/user_types")
        endforeach(test_name)
    else(VALGRIND_FOUND)
        message(WARNING "valgrind executable not found! Disabling memory leaks tests.")
    endif(VALGRIND_FOUND)
endif()

if(ENABLE_COVERAGE)
    # Destination
    set(COVERAGE_DIR        "${CMAKE_BINARY_DIR}/tests/code_coverage/")
    set(COVERAGE_FILE_RAW   "${CMAKE_BINARY_DIR}/tests/coverage_raw.info")
    set(COVERAGE_FILE_CLEAN "${CMAKE_BINARY_DIR}/tests/coverage_clean.info")

    # Add coverage target
    add_custom_target(coverage
        COMMENT "Generating code coverage..."
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        # Cleanup code counters
        COMMAND "${PATH_LCOV}" --directory . --zerocounters --quiet

        # Run tests
        COMMAND "${CMAKE_CTEST_COMMAND}" --quiet

        # Capture the counters
        COMMAND "${PATH_LCOV}"
            --directory .
            --rc lcov_branch_coverage=1
            --rc 'lcov_excl_line=assert'
            --capture --quiet
            --output-file "${COVERAGE_FILE_RAW}"
        # Remove coverage of tests, system headers, etc.
        COMMAND "${PATH_LCOV}"
            --remove "${COVERAGE_FILE_RAW}" '${CMAKE_SOURCE_DIR}/tests/*'
            --rc lcov_branch_coverage=1
            --quiet --output-file "${COVERAGE_FILE_CLEAN}"
        # Generate HTML report
        COMMAND "${PATH_GENHTML}"
            --branch-coverage --function-coverage --quiet --title "libyang"
            --legend --show-details --output-directory "${COVERAGE_DIR}"
            "${COVERAGE_FILE_CLEAN}"
        # Delete the counters
        COMMAND "${CMAKE_COMMAND}" -E remove
            ${COVERAGE_FILE_RAW} ${COVERAGE_FILE_CLEAN}
        )

    add_custom_command(TARGET coverage POST_BUILD
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
        COMMENT "To see the code coverage report, open ${COVERAGE_DIR}index.html"
        COMMAND ;
        )
endif()
