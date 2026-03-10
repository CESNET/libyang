if(NOT YANGUPDATE_PLUGINS_DIR)
    message(STATUS "yangupdate disabled - requires YANGUPDATE_PLUGINS_DIR to be set")
    return()
endif()

# get all the plugin files
file(GLOB_RECURSE plg_files LIST_DIRECTORIES false CONFIGURE_DEPENDS "${YANGUPDATE_PLUGINS_DIR}/*.c")

# collect all the plugin structures
foreach(plg_file IN LISTS plg_files)
    file(READ ${plg_file} file_content)
    string(REGEX MATCHALL "struct lyu_plg [^ ]*" struct_lines ${file_content})
    foreach(struct_line IN LISTS struct_lines)
        string(SUBSTRING "${struct_line}" 15 -1 struct_name)

        # append the structure
        string(APPEND plugins_h_extern "extern struct lyu_plg ${struct_name};\n")
        string(APPEND plugins_h_structs "    &${struct_name},\n")
    endforeach()
endforeach()

# generate the plugins header
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/yang_update_plugins.h "#ifndef LY_YANG_UPDATE_PLUGINS_H_\n#define LY_YANG_UPDATE_PLUGINS_H_\n\n#include \"yang_update_plugins.h\"\n\n${plugins_h_extern}\nstruct lyu_plg *lyu_plugins[] = {\n${plugins_h_structs}};\n\n#endif\n")

# sources
set(lib_src
    yang_update.c
    ${plg_files})

set(format_sources
    ${format_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/yang_update.c
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    PARENT_SCOPE)

# yang update library
add_library(yang_update STATIC ${lib_src})
target_include_directories(yang_update PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_BINARY_DIR})
get_target_property(yang_linked_libraries yang LINK_LIBRARIES)
target_link_libraries(yang_update $<TARGET_OBJECTS:yangobj> ${yang_linked_libraries})

# yangupdate
add_executable(yangupdate main.c)
target_include_directories(yangupdate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
target_link_libraries(yangupdate yang_update)

install(TARGETS yangupdate DESTINATION ${CMAKE_INSTALL_BINDIR})
