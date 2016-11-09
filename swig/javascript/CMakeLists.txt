cmake_minimum_required(VERSION 3.1)

find_package(Nodejs REQUIRED)
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

include_directories(
	${NODEJS_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_SOURCE_DIR}/src
)

set(V8_VERSION_HEX 0x0${V8_VERSION_MAJOR}${V8_VERSION_MINOR}${V8_VERSION_PATCH})
string(LENGTH "${V8_VERSION_HEX}" V8_VERSION_HEX_length)
while(V8_VERSION_HEX_length LESS 8)
	set(V8_VERSION_HEX "${V8_VERSION_HEX}0")
	message(DEBUG " - Padded V8 version to match SWIG format")
	string(LENGTH "${V8_VERSION_HEX}" V8_VERSION_HEX_length)
endwhile()

set_property(SOURCE ${CMAKE_SOURCE_DIR}/swig/javascript/libyang_javascript.i PROPERTY SWIG_FLAGS "-node" "-I${CMAKE_BINARY_DIR}/src" "-DV8_VERSION=${V8_VERSION_HEX}")

set_source_files_properties(${CMAKE_SOURCE_DIR}/swig/javascript/libyang_javascript.i PROPERTIES CPLUSPLUS ON)

swig_add_module(libyang_javascript javascript ${CMAKE_SOURCE_DIR}/swig/javascript/libyang_javascript.i ${libsrc} ${PCRE_INCLUDE_DIRS} )

# link math
swig_link_libraries(libyang_javascript m)

# find pthreads
swig_link_libraries(libyang_javascript ${CMAKE_THREAD_LIBS_INIT})

# find PCRE library
swig_link_libraries(libyang_javascript ${PCRE_LIBRARIES})


function(PREPEND var prefix)
	set(listVar "")
	foreach(f ${ARGN})
		list(APPEND listVar "${prefix}/${f}")
	endforeach(f)
	set(${var} "${listVar}" PARENT_SCOPE)
endfunction(PREPEND)

macro (libyang_CREATE_INSTALL_PACKAGE_JSON generated_file install_location)
	configure_file(${CMAKE_SOURCE_DIR}/swig/javascript/${generated_file}.cmake ${CMAKE_CURRENT_BINARY_DIR}/${generated_file} @ONLY)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${generated_file} DESTINATION ${install_location})
endmacro (libyang_CREATE_INSTALL_PACKAGE_JSON)
libyang_create_install_package_json(package.json lib/node_modules/libyang)

macro (libyang_CREATE_BINDING_GYP generated_file)
	set(libyang_LIB_SRCS_GYP "")
	PREPEND(libyang_NPM_SRCS ${PROJECT_SOURCE_DIR} ${libsrc})

	foreach(srcfile ${libyang_NPM_SRCS})
		file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${srcfile})
		set(libyang_LIB_SRCS_GYP "'${rel}',\n\t\t\t${libyang_LIB_SRCS_GYP}")
	endforeach(srcfile)

	PREPEND(libyang_NPM_LIBS ${PROJECT_SOURCE_DIR} "${PCRE_LIBRARIES}")
	foreach(srcfile ${libyang_NPM_LIBS})
			file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${srcfile})
			set(libyang_LIB_THIRD_PARTY_GYP "'-L/${rel}',\n\t\t\t${libyang_LIB_THIRD_PARTY_GYP}")
	endforeach(srcfile)

	PREPEND(libyang_LIB_INCLUDE_DIRS ${PROJECT_SOURCE_DIR} ${PCRE_LIBRARIES})
	PREPEND(${libyang_LIB_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR} ${PCRE_INCLUDE_DIRS})
	foreach(includedir ${libyang_LIB_INCLUDE_DIRS})
		file(RELATIVE_PATH rel ${CMAKE_SOURCE_DIR} ${includedir})
		set(libyang_LIB_INCLUDE_DIRS_GYP "'${rel}',\n\t\t\t${libyang_LIB_INCLUDE_DIRS_GYP}")
	endforeach(includedir)

	configure_file(${CMAKE_SOURCE_DIR}/swig/javascript/${generated_file}.cmake ${CMAKE_CURRENT_BINARY_DIR}/${generated_file} @ONLY)
endmacro (libyang_CREATE_BINDING_GYP)
libyang_create_binding_gyp(binding.gyp)

set(SWIG_DEST "javascript")
file(COPY "${CMAKE_SOURCE_DIR}/src" DESTINATION ${SWIG_DEST})
file(COPY "${CMAKE_SOURCE_DIR}/models" DESTINATION ${SWIG_DEST})
file(COPY "${CMAKE_SOURCE_DIR}/swig/javascript/files" DESTINATION "${SWIG_DEST}")
configure_file("${CMAKE_SOURCE_DIR}/swig/javascript/test.js" "${SWIG_DEST}/test.js" ESCAPE_QUOTES @ONLY)
configure_file("${CMAKE_SOURCE_DIR}/swig/javascript/index.js" "${SWIG_DEST}/index.js" ESCAPE_QUOTES @ONLY)

add_custom_target(npmpkg ALL
	COMMENT "start building node package"
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/package.json ${SWIG_DEST}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libyang_javascriptJAVASCRIPT_wrap.cxx "${SWIG_DEST}/src/"
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/binding.gyp ${SWIG_DEST})
add_dependencies (npmpkg libyang_javascript)
