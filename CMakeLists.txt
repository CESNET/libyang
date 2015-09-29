cmake_minimum_required(VERSION 2.6)
project(libyang C)

# set version
set(LIBYANG_MAJOR_VERSION 0)
set(LIBYANG_MINOR_VERSION 2)
set(LIBYANG_MICRO_VERSION 29)
set(LIBYANG_VERSION ${LIBYANG_MAJOR_VERSION}.${LIBYANG_MINOR_VERSION}.${LIBYANG_MICRO_VERSION})
set(LIBYANG_SOVERSION ${LIBYANG_MAJOR_VERSION}.${LIBYANG_MINOR_VERSION})

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

# check the supported platform
if(NOT UNIX)
	message(FATAL_ERROR "Only *nix like systems are supported.")
endif()

if(NOT LIB_INSTALL_DIR)
	set(LIB_INSTALL_DIR lib)
endif()

if(NOT INCLUDE_INSTALL_DIR)
	set(INCLUDE_INSTALL_DIR include/libyang)
endif()

# set default build type if not specified by user
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE debug)
endif()

set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -Wall -Wextra -fvisibility=hidden")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG   "-g -O0")

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
	src/tree_schema.c
	src/tree_data.c
	src/printer.c
	src/yang_library.c
	src/printer_yang.c
	src/printer_xml.c
	src/printer_tree.c
	src/printer_info.c
	src/printer_json.c
	src/yang_types.c)

set(lintsrc
	tools/lint/main.c
	tools/lint/commands.c
	tools/lint/completion.c
	linenoise/linenoise.c)

set(headers
	src/libyang.h
	src/tree_schema.h
	src/tree_data.h)

add_library(yang SHARED ${libsrc})
set_target_properties(yang PROPERTIES VERSION ${LIBYANG_VERSION} SOVERSION ${LIBYANG_SOVERSION})
set(LIBS ${LIBS} yang)

# find PCRE library
find_package(PCRE REQUIRED)
include_directories(${PCRE_INCLUDE_DIRS})
set(LIBS ${LIBS} ${PCRE_LIBRARIES})

install(TARGETS yang DESTINATION ${LIB_INSTALL_DIR})
install(FILES ${headers} DESTINATION ${INCLUDE_INSTALL_DIR})

# generate doxygen documentation for libyang API
add_custom_target(doc
                  COMMAND doxygen  ${CMAKE_BINARY_DIR}/Doxyfile
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
configure_file(Doxyfile.in Doxyfile)

# clean cmake cache
add_custom_target(cclean
                  COMMAND make clean
                  COMMAND find . -iname '*cmake*' -not -name CMakeLists.txt -not -path './CMakeModules*' -exec rm -rf {} +
                  COMMAND rm -rf Makefile Doxyfile
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(yanglint ${lintsrc})
target_link_libraries(yanglint ${LIBS})

