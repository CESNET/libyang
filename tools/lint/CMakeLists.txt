# yanglint

set(lintsrc
    main.c
    main_ni.c
    cmd.c
    cmd_add.c
    cmd_clear.c
    cmd_data.c
    cmd_list.c
    cmd_load.c
    cmd_print.c
    cmd_searchpath.c
    common.c
    completion.c
    configuration.c
    linenoise/linenoise.c)

set(format_sources
    ${format_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    PARENT_SCOPE)
   
add_executable(yanglint ${lintsrc} $<TARGET_OBJECTS:compat>)
target_link_libraries(yanglint yang)
install(TARGETS yanglint DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${PROJECT_SOURCE_DIR}/tools/lint/yanglint.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
target_include_directories(yanglint BEFORE PRIVATE ${PROJECT_BINARY_DIR})

