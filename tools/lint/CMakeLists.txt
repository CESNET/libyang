# yanglint

if(WIN32)
    set(YANGLINT_INTERACTIVE OFF)
else()
    set(YANGLINT_INTERACTIVE ON)
endif()

set(lintsrc
    main_ni.c
    cmd.c
    cmd_add.c
    cmd_clear.c
    cmd_data.c
    cmd_list.c
    cmd_feature.c
    cmd_load.c
    cmd_print.c
    cmd_searchpath.c
    cmd_extdata.c
    common.c
)
if(YANGLINT_INTERACTIVE)
    set(lintsrc ${lintsrc}
        main.c
        completion.c
        configuration.c
        linenoise/linenoise.c)
else()
    set(lintsrc ${lintsrc}
        main_ni_only.c)
endif()

set(format_sources
    ${format_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    PARENT_SCOPE)

add_executable(yanglint ${lintsrc} ${compatsrc})
target_link_libraries(yanglint yang)
install(TARGETS yanglint DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${PROJECT_SOURCE_DIR}/tools/lint/yanglint.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
target_include_directories(yanglint BEFORE PRIVATE ${PROJECT_BINARY_DIR})

if(WIN32)
    target_include_directories(yanglint PRIVATE ${GETOPT_INCLUDE_DIR})
    target_link_libraries(yanglint ${GETOPT_LIBRARY})
endif()

#
# tests
#
function(add_yanglint_test)
    cmake_parse_arguments(ADDTEST "" "NAME;VIA;SCRIPT" "" ${ARGN})
    set(TEST_NAME yanglint_${ADDTEST_NAME})

    if(${ADDTEST_VIA} STREQUAL "tclsh")
        set(WRAPPER ${PATH_TCLSH})
    else()
        message(FATAL_ERROR "build: unexpected wrapper '${ADDTEST_VIA}'")
    endif()

    add_test(NAME ${TEST_NAME} COMMAND ${WRAPPER} ${CMAKE_CURRENT_SOURCE_DIR}/tests/${ADDTEST_SCRIPT})
    set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT "TESTS_DIR=${CMAKE_CURRENT_SOURCE_DIR}/tests")
    set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT "YANG_MODULES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/tests/modules")
    set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT "YANGLINT=${PROJECT_BINARY_DIR}")
endfunction(add_yanglint_test)

if(ENABLE_TESTS)
    # tests of interactive mode using tclsh
    find_program(PATH_TCLSH NAMES tclsh)
    if(NOT PATH_TCLSH)
        message(WARNING "'tclsh' not found! The yanglint(1) interactive tests will not be available.")
    else()
        if(YANGLINT_INTERACTIVE)
            add_yanglint_test(NAME interactive VIA tclsh SCRIPT interactive/all.tcl)
            add_yanglint_test(NAME non-interactive VIA tclsh SCRIPT non-interactive/all.tcl)
        else()
            add_yanglint_test(NAME non-interactive VIA tclsh SCRIPT non-interactive/all.tcl)
        endif()
    endif()
endif()
