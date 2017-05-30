cmake_minimum_required(VERSION 2.8.12)

find_package(Nodejs REQUIRED)
find_package(SWIG REQUIRED)

STRING(TOLOWER ${CMAKE_BUILD_TYPE} lower_type)
if(${lower_type} STREQUAL "release")
	set(JAVASCRIPT_BUILD_TYPE yang)
else()
	set(JAVASCRIPT_BUILD_TYPE yang-dev)
endif()

set(SWIG_DEST "${CMAKE_CURRENT_BINARY_DIR}/javascript")
file(MAKE_DIRECTORY ${SWIG_DEST})

set(V8_VERSION_HEX 0x0${V8_VERSION_MAJOR}${V8_VERSION_MINOR}${V8_VERSION_PATCH})
string(LENGTH "${V8_VERSION_HEX}" V8_VERSION_HEX_length)
while(V8_VERSION_HEX_length LESS 8)
	set(V8_VERSION_HEX "${V8_VERSION_HEX}0")
	string(LENGTH "${V8_VERSION_HEX}" V8_VERSION_HEX_length)
endwhile()

execute_process(COMMAND swig -c++ -javascript -node -DV8_VERSION=${V8_VERSION_HEX} -outcurrentdir -I${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/swig/javascript/libyang_javascript.i WORKING_DIRECTORY ${SWIG_DEST} RESULT_VARIABLE RET OUTPUT_VARIABLE OUT ERROR_VARIABLE ERROR)
if(NOT RET)
	function(PREPEND var prefix)
		set(listVar "")
		foreach(f ${ARGN})
			list(APPEND listVar "${prefix}/${f}")
		endforeach(f)
		set(${var} "${listVar}" PARENT_SCOPE)
	endfunction(PREPEND)

	macro(LIBYANG_CREATE_BINDING_GYP generated_file)
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

		configure_file(${CMAKE_SOURCE_DIR}/swig/javascript/${generated_file}.in ${SWIG_DEST}/${generated_file} @ONLY)
	endmacro(LIBYANG_CREATE_BINDING_GYP)
	LIBYANG_CREATE_BINDING_GYP(binding.gyp)

	file(COPY "${CMAKE_SOURCE_DIR}/src" DESTINATION ${SWIG_DEST})
	file(COPY "${SWIG_DEST}/libyang_javascript_wrap.cxx" DESTINATION "${SWIG_DEST}/src")
	file(REMOVE "${SWIG_DEST}/libyang_javascript_wrap.cxx")
	file(COPY "${CMAKE_SOURCE_DIR}/models" DESTINATION ${SWIG_DEST})
	file(COPY "${CMAKE_SOURCE_DIR}/swig/javascript/files" DESTINATION "${SWIG_DEST}")
	configure_file("${CMAKE_SOURCE_DIR}/swig/javascript/test.js.in" "${SWIG_DEST}/test.js" ESCAPE_QUOTES @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/swig/javascript/index.js.in" "${SWIG_DEST}/index.js" ESCAPE_QUOTES @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/swig/javascript/package.json.in" "${SWIG_DEST}/package.json" ESCAPE_QUOTES @ONLY)

else()
	message(${ERROR})
endif()

# remove javascript directory with "make clean" command.
set_property(DIRECTORY ${EXECUTABLE_OUTPUT_PATH} PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${SWIG_DEST}")
