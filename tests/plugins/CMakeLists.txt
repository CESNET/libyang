
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

function(ly_add_plugin)
    cmake_parse_arguments(ADDPLUGIN "" "NAME" "SOURCES" ${ARGN})
    set(PLUGIN_NAME plugin_${ADDPLUGIN_NAME})

    add_library(${PLUGIN_NAME} MODULE)
    set_target_properties(${PLUGIN_NAME} PROPERTIES PREFIX "")
	foreach(PLUGIN_SOURCE ${ADDPLUGIN_SOURCES})
        target_sources(${PLUGIN_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_SOURCE})
    endforeach()

	target_link_libraries(${PLUGIN_NAME} yang)	
endfunction(ly_add_plugin)

ly_add_plugin(NAME invalid SOURCES invalid.c)
ly_add_plugin(NAME simple SOURCES simple.c)