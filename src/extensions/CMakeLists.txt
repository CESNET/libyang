macro(EXTENSION_PLUGIN PLUGIN_NAME SRCS)
	add_library(${PLUGIN_NAME} SHARED ${SRCS})
	set_target_properties(${PLUGIN_NAME} PROPERTIES PREFIX "")
	target_link_libraries(${PLUGIN_NAME} yang)
	install(TARGETS ${PLUGIN_NAME} DESTINATION ${LIBYANG_EXT_PLUGINS_DIR})	
endmacro(EXTENSION_PLUGIN)

EXTENSION_PLUGIN(nacm "nacm.c")
EXTENSION_PLUGIN(metadata "metadata.c")

# special plugin for test_extensions - only built, but not installed
add_library(libyang_ext_test SHARED ext_test.c)
set_target_properties(libyang_ext_test PROPERTIES PREFIX "")
target_link_libraries(libyang_ext_test yang)
