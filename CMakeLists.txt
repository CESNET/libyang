cmake_minimum_required(VERSION 2.8.9)
project(libyang C)

include(GNUInstallDirs)

set(LIBYANG_DESCRIPTION "libyang is YANG data modelling language parser and toolkit written (and providing API) in C.")

# set version
set(LIBYANG_MAJOR_VERSION 0)
set(LIBYANG_MINOR_VERSION 12)
set(LIBYANG_MICRO_VERSION 30)
set(LIBYANG_VERSION ${LIBYANG_MAJOR_VERSION}.${LIBYANG_MINOR_VERSION}.${LIBYANG_MICRO_VERSION})
set(LIBYANG_SOVERSION ${LIBYANG_MAJOR_VERSION}.${LIBYANG_MINOR_VERSION})
configure_file(${PROJECT_SOURCE_DIR}/src/libyang.h.in ${PROJECT_SOURCE_DIR}/src/libyang.h)

if(PLUGINS_DIR)
	set(LIBYANG_EXT_PLUGINS_DIR ${PLUGINS_DIR})
else()
	set(LIBYANG_EXT_PLUGINS_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/libyang)
endif()
configure_file(${PROJECT_SOURCE_DIR}/src/extensions_config.h.in ${PROJECT_SOURCE_DIR}/src/extensions_config.h)

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

find_package(FLEX)
find_package(BISON)
find_program (SED_TOOL NAMES sed)

if(NOT BISON_FOUND)
	message(WARNING "Missing Bison.\nTarget bison does not exist.")
elseif (NOT FLEX_FOUND)
	message(WARNING "Missing Flex.\nTarget bison does not exist.")
else()
	if (BISON_VERSION VERSION_LESS 3)
		set(EMPTYDIR "")
	else ()
		set(EMPTYDIR "%empty")
	endif()
	configure_file(${PROJECT_SOURCE_DIR}/src/yang.y.in ${PROJECT_SOURCE_DIR}/src/yang.y)
	add_custom_target(bison
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src
		COMMAND bison -Wno-other -l -o parser_yang_bis.c --defines=parser_yang_bis.h yang.y
		COMMAND flex -8 -L -o parser_yang_lex.c --header-file=parser_yang_lex.h yang.l
		COMMAND ${SED_TOOL} -i 's/int yychar\;/int yychar\;\\nchar *s = NULL, *tmp_s = NULL\;\\nstruct lys_include inc\;\\nstruct lys_module *trg = NULL\;\\nstruct lys_node *tpdf_parent = NULL, *data_node = NULL\;\\nvoid *actual = NULL\;\\nstruct lys_node_uses *refine_parent = NULL\;\\nint config_inherit = 0, actual_type = 0\;\\nint64_t cnt_val\;\\nint read_string = read_all\;\\nvoid *yang_type = NULL\;/' parser_yang_bis.c
	)
endif()

# check the supported platform
if(NOT UNIX)
	message(FATAL_ERROR "Only *nix like systems are supported.")
endif()

# set default build type if not specified by user
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE debug)
endif()

set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG   "-g -O0")

if(CMAKE_BUILD_TYPE STREQUAL debug)
	option(ENABLE_BUILD_TESTS "Build tests" ON)
	option(ENABLE_VALGRIND_TESTS "Build tests with valgrind" ON)
else()
	option(ENABLE_BUILD_TESTS "Build tests" OFF)
	option(ENABLE_VALGRIND_TESTS "Build tests with valgrind" OFF)
endif()

set(libsrc
	src/common.c
	src/context.c
	src/log.c
	src/dict.c
	src/resolve.c
	src/validation.c
	src/xml.c
	src/parser.c
	src/parser_yin.c
	src/parser_xml.c
	src/parser_json.c
	src/parser_yang_bis.c
	src/parser_yang_lex.c
	src/parser_yang.c
	src/tree_schema.c
	src/tree_data.c
	src/extensions.c
	src/printer.c
	src/xpath.c
	src/printer_yang.c
	src/printer_yin.c
	src/printer_xml.c
	src/printer_tree.c
	src/printer_info.c
	src/printer_json.c
	src/yang_types.c)

set(lintsrc
	tools/lint/main.c
	tools/lint/main_ni.c
	tools/lint/commands.c
	tools/lint/completion.c
	linenoise/linenoise.c)

set(yang2yinsrc
	tools/yang2yin/main.c)

set(headers
	src/libyang.h
	src/tree_schema.h
	src/tree_data.h
	src/extensions.h
	src/xml.h
	src/dict.h)

# create static libyang library
if(STATIC)
    add_library(yang_static STATIC ${libsrc})
    set_target_properties(yang_static PROPERTIES OUTPUT_NAME yang)
    set_target_properties(yang_static PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")
    target_link_libraries(yang_static m)
    target_link_libraries(yang_static ${CMAKE_DL_LIBS})
    target_link_libraries(yang_static ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(yang_static ${PCRE_LIBRARIES})
    install(TARGETS yang_static DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif(STATIC)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
add_library(yangobj OBJECT ${libsrc})
add_library(yang SHARED $<TARGET_OBJECTS:yangobj>)
set_target_properties(yang PROPERTIES VERSION ${LIBYANG_VERSION} SOVERSION ${LIBYANG_SOVERSION})
set_target_properties(yang PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")

# link math
target_link_libraries(yang m)

#link dl
target_link_libraries(yang ${CMAKE_DL_LIBS})

# find pthreads
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
target_link_libraries(yang ${CMAKE_THREAD_LIBS_INIT})

# find PCRE library
find_package(PCRE REQUIRED)
include_directories(${PCRE_INCLUDE_DIRS})
target_link_libraries(yang ${PCRE_LIBRARIES})

install(TARGETS yang DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libyang)

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
	# generate and install pkg-config file
	configure_file("libyang.pc.in" "libyang.pc" @ONLY)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libyang.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
	# check that pkg-config includes the used path
	execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable pc_path pkg-config RESULT_VARIABLE RETURN OUTPUT_VARIABLE PC_PATH ERROR_QUIET)
	if(RETURN EQUAL 0)
		string(REGEX MATCH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig" SUBSTR "${PC_PATH}")
		string(LENGTH "${SUBSTR}" SUBSTR_LEN)
		if(SUBSTR_LEN EQUAL 0)
			message(WARNING "pkg-config will not detect the new package after installation, adjust PKG_CONFIG_PATH using \"export PKG_CONFIG_PATH=\${PKG_CONFIG_PATH}:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig\".")
		endif()
	endif()
endif()

# generate doxygen documentation for libyang API
find_package(Doxygen)
if(DOXYGEN_FOUND)
	set(DOXYGEN_SKIP_DOT TRUE)
	add_custom_target(doc
			COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	configure_file(Doxyfile.in Doxyfile)
endif()

# clean cmake cache
add_custom_target(cclean
		COMMAND make clean
		COMMAND find . -iname '*cmake*' -not -name CMakeLists.txt -not -path './CMakeModules*' -exec rm -rf {} +
		COMMAND rm -rf Makefile Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# YANG extensions plugins
add_subdirectory(src/extensions)

# yanglint
add_executable(yanglint ${lintsrc})
target_link_libraries(yanglint yang)
install(TARGETS yanglint DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${PROJECT_SOURCE_DIR}/tools/lint/yanglint.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

# yang2yin
add_executable(yang2yin ${yang2yinsrc})

if(ENABLE_VALGRIND_TESTS)
	set(ENABLE_BUILD_TESTS ON)
endif()

if(ENABLE_BUILD_TESTS)
	find_package(CMocka 1.0.0)
	if(CMOCKA_FOUND)
		enable_testing()
		add_subdirectory(tests)
	endif(CMOCKA_FOUND)
endif(ENABLE_BUILD_TESTS)

if(JAVASCRIPT_BINDING)
	include(swig/javascript/CMakeLists.txt)
endif()
