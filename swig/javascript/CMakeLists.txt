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


set_target_properties(libyang_javascript PROPERTIES
	COMPILE_FLAGS " -DJAVASCRIPT_BINDING=ON"
	PREFIX ""
	OUTPUT_NAME libyang_javascript
	SUFFIX ".node"
)

function(PREPEND var prefix)
	set(listVar "")
	foreach(f ${ARGN})
		list(APPEND listVar "${prefix}/${f}")
	endforeach(f)
	set(${var} "${listVar}" PARENT_SCOPE)
endfunction(PREPEND)

message(INFO " - swig Version ${SWIG_VERSION}")
message(INFO " - CXX compiler Version ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

if(${V8_VERSION_MAJOR} GREATER 3)
	message(INFO " - Using V8 version > 3 so requiring C++11 compiler")
	if(CMAKE_VERSION VERSION_LESS "3.1")
		message(INFO " - **WARNING** Need to use CMAKE version 3.1+, but it is ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
		message(INFO " -             So a workaround will be used.")
		if(CMAKE_COMPILER_IS_GNUCXX)
			if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7")
				message(FATAL_ERROR " FATAL ERROR: GNU gcc compiler is also too old (need 4.7+, but ${CMAKE_CXX_COMPILER_VERSION}) and does not support C++11 standard.")
			endif()
			set(LIBYANG_CXX11_WORKAROUND_OPTION "-std=gnu++11")
		else()
			set(LIBYANG_CXX11_WORKAROUND_OPTION "-std=c++11")
		endif()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIBYANG_CXX11_WORKAROUND_OPTION} ")
	endif()
endif()

macro (libyang_CREATE_INSTALL_PACKAGE_JSON generated_file install_location)
	configure_file(${CMAKE_SOURCE_DIR}/swig/javascript/${generated_file}.cmake ${CMAKE_CURRENT_BINARY_DIR}/${generated_file} @ONLY)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${generated_file} DESTINATION ${install_location})
endmacro (libyang_CREATE_INSTALL_PACKAGE_JSON)
libyang_create_install_package_json(package.json lib/node_modules/libyang)

macro (libyang_CREATE_BINDING_GYP generated_file)
	set(libyang_LIB_SRCS_GYP "")
	#set(libyang_NPM_SRCS ${libsrc})
	PREPEND(libyang_NPM_SRCS ${PROJECT_SOURCE_DIR} ${libsrc})

	foreach(srcfile ${libyang_NPM_SRCS})
		file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${srcfile})
		set(libyang_LIB_SRCS_GYP "'${rel}',\n${libyang_LIB_SRCS_GYP}")
	endforeach(srcfile)

	PREPEND(libyang_NPM_LIBS ${PROJECT_SOURCE_DIR} "${PCRE_LIBRARIES}")
	foreach(srcfile ${libyang_NPM_LIBS})
			file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${srcfile})
			set(libyang_LIB_THIRD_PARTY_GYP "'-L/${rel}',\n${libyang_LIB_THIRD_PARTY_GYP}")
	endforeach(srcfile)

	PREPEND(libyang_LIB_INCLUDE_DIRS ${PROJECT_SOURCE_DIR} ${PCRE_LIBRARIES})
	PREPEND(${libyang_LIB_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR} ${PCRE_INCLUDE_DIRS})
	foreach(includedir ${libyang_LIB_INCLUDE_DIRS})
		file(RELATIVE_PATH rel ${CMAKE_SOURCE_DIR} ${includedir})
		set(libyang_LIB_INCLUDE_DIRS_GYP "'${rel}',\n${libyang_LIB_INCLUDE_DIRS_GYP}")
	endforeach(includedir)

	configure_file(${CMAKE_SOURCE_DIR}/swig/javascript/${generated_file}.cmake ${CMAKE_CURRENT_BINARY_DIR}/${generated_file} @ONLY)
endmacro (libyang_CREATE_BINDING_GYP)
libyang_create_binding_gyp(binding.gyp)

set(SWIG_DEST "javascript")
file(COPY "${CMAKE_SOURCE_DIR}/src" DESTINATION ${SWIG_DEST})
file(COPY "${CMAKE_SOURCE_DIR}/models" DESTINATION ${SWIG_DEST})
file(COPY "${CMAKE_SOURCE_DIR}/swig/javascript/test/files" DESTINATION "${SWIG_DEST}/test")
configure_file("${CMAKE_SOURCE_DIR}/swig/javascript/test/yang.js" "${SWIG_DEST}/test/yang.js" ESCAPE_QUOTES @ONLY)

add_custom_target(npmpkg)
add_custom_command(TARGET npmpkg POST_BUILD
	COMMENT "start building node package"
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libyang_javascriptJAVASCRIPT_wrap.cxx "${SWIG_DEST}/src/"
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/package.json ${SWIG_DEST}
	COMMAND sed -i "'s/libyang.node/build\\/Release\\/libyang.node/'" ${SWIG_DEST}/package.json
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/binding.gyp ${SWIG_DEST}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/libyang_javascript.node ${SWIG_DEST})
add_dependencies (npmpkg libyang_javascript)

