cmake_minimum_required(VERSION 2.6)
project(libyang C)

# check the supported platform
if(NOT UNIX)
	message(FATAL_ERROR "Only *nix like systems are supported.")
endif()

if(NOT LIB_INSTALL_DIR)
	set(LIB_INSTALL_DIR lib)
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
	src/xml.c
	src/parser/yin.c
	src/parser/xml.c
	src/tree.c
	src/printer.c
	src/printer/yang.c
	src/printer/tree.c
	src/printer/info.c
	src/printer/json.c
	src/yang_types.c)

set(lintsrc
	tools/lint/main.c
	tools/lint/commands.c
	tools/lint/completion.c
	linenoise/linenoise.c)

add_library(yang SHARED ${libsrc})
set_target_properties(yang PROPERTIES VERSION 0.1.0 SOVERSION 0.1)

install(TARGETS yang DESTINATION ${LIB_INSTALL_DIR})

# generate doxygen documentation for libyang API
add_custom_target(doc
                  COMMAND doxygen Doxyfile
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# clean cmake cache
add_custom_target(cclean
                  COMMAND make clean
                  COMMAND find . -iname '*cmake*' -not -name CMakeLists.txt -exec rm -rf {} +
                  COMMAND rm -rf Makefile
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(yanglint ${lintsrc})
target_link_libraries(yanglint yang)

