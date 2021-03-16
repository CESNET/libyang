cmake_minimum_required(VERSION 2.8.12)

# force out-of-source build
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source build is not allowed. Please make a standalone build directory and run CMake from there. You may need to remove CMakeCache.txt.")
endif()

project(libyang C)

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

include(GNUInstallDirs)
include(CheckSymbolExists)
include(UseCompat)
include(ABICheck)
include(SourceFormat)
include(Doc)

# set default build type if not specified by user
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
# normalize build type string
string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
if ("${BUILD_TYPE_UPPER}" STREQUAL "RELEASE")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Type" FORCE)
elseif ("${BUILD_TYPE_UPPER}" STREQUAL "DEBUG")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build Type" FORCE)
elseif ("${BUILD_TYPE_UPPER}" STREQUAL "RELWITHDEBINFO")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build Type" FORCE)
elseif ("${BUILD_TYPE_UPPER}" STREQUAL "RELWITHDEBUG")
  set(CMAKE_BUILD_TYPE "RelWithDebug" CACHE STRING "Build Type" FORCE)
elseif ("${BUILD_TYPE_UPPER}" STREQUAL "ABICHECK")
  set(CMAKE_BUILD_TYPE "ABICheck" CACHE STRING "Build Type" FORCE)
elseif ("${BUILD_TYPE_UPPER}" STREQUAL "DOCONLY")
  set(CMAKE_BUILD_TYPE "DocOnly" CACHE STRING "Build Type" FORCE)
else ()
  message(FATAL_ERROR "Unknown CMAKE_BUILD_TYPE \"${CMAKE_BUILD_TYPE}\".")
endif ()

# check the supported platform
if(NOT UNIX)
    message(FATAL_ERROR "Only *nix like systems are supported.")
endif()

#
# variables
#

set(LIBYANG_DESCRIPTION "libyang is YANG data modelling language parser and toolkit written (and providing API) in C.")

# Correct RPATH usage on OS X
set(CMAKE_MACOSX_RPATH TRUE)

# keep all binaries in the build directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# set version of the project
set(LIBYANG_MAJOR_VERSION 2)
set(LIBYANG_MINOR_VERSION 0)
set(LIBYANG_MICRO_VERSION 0)
set(LIBYANG_VERSION ${LIBYANG_MAJOR_VERSION}.${LIBYANG_MINOR_VERSION}.${LIBYANG_MICRO_VERSION})
# set version of the library
set(LIBYANG_MAJOR_SOVERSION 2)
set(LIBYANG_MINOR_SOVERSION 0)
set(LIBYANG_MICRO_SOVERSION 0)
set(LIBYANG_SOVERSION_FULL ${LIBYANG_MAJOR_SOVERSION}.${LIBYANG_MINOR_SOVERSION}.${LIBYANG_MICRO_SOVERSION})
set(LIBYANG_SOVERSION ${LIBYANG_MAJOR_SOVERSION})

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_UNUSED_ATTR "UNUSED_ ## x __attribute__((__unused__))")
    set(COMPILER_PACKED_ATTR "__attribute__((__packed__))")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(COMPILER_UNUSED_ATTR "UNUSED_ ## x __attribute__((__unused__))")
    set(COMPILER_PACKED_ATTR "__attribute__((__packed__))")
else()
    set(COMPILER_UNUSED_ATTR "UNUSED_ ## x")
    set(COMPILER_PACKED_ATTR "")
endif()

set(CMAKE_C_FLAGS                "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-missing-field-initializers -std=c99")
set(CMAKE_C_FLAGS_DEBUG          "-g3 -O0")
set(CMAKE_C_FLAGS_ABICHECK       "-g -Og")

include_directories(${PROJECT_BINARY_DIR}/src ${PROJECT_SOURCE_DIR}/src)
configure_file(${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_BINARY_DIR}/src/version.h @ONLY)

#set(EXTENSIONS_PLUGINS_DIR_MACRO "${PLUGINS_DIR}/extensions")
#set(USER_TYPES_PLUGINS_DIR_MACRO "${PLUGINS_DIR}/user_types")

# setup bindings
#set(GEN_LANGUAGE_BINDINGS 0 CACHE BOOL "Enable language bindings generation.")
#set(GEN_CPP_BINDINGS 1 CACHE BOOL "Enable C++ bindings.")
# Python bindings depend on C++ bindings because of SWIG
#set(GEN_PYTHON_BINDINGS 1 CACHE BOOL "Enable Python bindings.")
#set(GEN_PYTHON_VERSION "3" CACHE STRING "Python version")
#set(GEN_JAVASCRIPT_BINDINGS 0 CACHE BOOL "Enable JavaScript bindings.")

set(libsrc
    src/common.c
    src/log.c
    src/hash_table.c
    src/set.c
    src/path.c
    src/diff.c
    src/context.c
    src/json.c
    src/tree_data.c
    src/tree_data_free.c
    src/tree_data_helpers.c
    src/tree_data_hash.c
    src/parser_xml.c
    src/parser_json.c
    src/parser_lyb.c
    src/out.c
    src/printer_data.c
    src/printer_xml.c
    src/printer_json.c
    src/printer_lyb.c
    src/schema_compile.c
    src/schema_compile_node.c
    src/schema_compile_amend.c
    src/schema_features.c
    src/tree_schema.c
    src/tree_schema_free.c
    src/tree_schema_helpers.c
    src/in.c
    src/parser_yang.c
    src/parser_yin.c
    src/parser_stmt.c
    src/printer_schema.c
    src/printer_yang.c
    src/printer_yin.c
    src/printer_tree.c
    src/plugins_types.c
    src/plugins_types_binary.c
    src/plugins_types_bits.c
    src/plugins_types_boolean.c
    src/plugins_types_decimal64.c
    src/plugins_types_empty.c
    src/plugins_types_enumeration.c
    src/plugins_types_identityref.c
    src/plugins_types_instanceid.c
    src/plugins_types_integer.c
    src/plugins_types_leafref.c
    src/plugins_types_string.c
    src/plugins_exts.c
    src/plugins_exts_metadata.c
    src/plugins_exts_nacm.c
    src/plugins_exts_yangdata.c
    src/xml.c
    src/xpath.c
    src/validation.c)

set(headers
    src/libyang.h
    src/context.h
    src/dict.h
    src/log.h
    src/in.h
    src/parser_data.h
    src/parser_schema.h
    src/plugins_types.h
    src/out.h
    src/printer_data.h
    src/printer_schema.h
    src/set.h
    src/tree.h
    src/tree_data.h
    src/tree_schema.h)

# source files to be covered by the 'format' target
set(format_sources
    compat/*
    src/*)
#
# options
#

if("${BUILD_TYPE_UPPER}" STREQUAL "DOCONLY")
    libyang_doc()
    return()
endif()

if(("${BUILD_TYPE_UPPER}" STREQUAL "DEBUG") OR ("${BUILD_TYPE_UPPER}" STREQUAL "RELWITHDEBINFO"))
    option(ENABLE_BUILD_TESTS "Build tests" ON)
    option(ENABLE_VALGRIND_TESTS "Build tests with valgrind" ON)
    # TODO enable when the internal docs ready
    set(INTERNAL_DOCS NO)
else()
    option(ENABLE_BUILD_TESTS "Build tests" OFF)
    option(ENABLE_VALGRIND_TESTS "Build tests with valgrind" OFF)
    set(INTERNAL_DOCS NO)
endif()
option(ENABLE_COVERAGE "Build code coverage report from tests" OFF)
option(ENABLE_FUZZ_TARGETS "Build target programs suitable for fuzzing with AFL" OFF)
#option(ENABLE_CALLGRIND_TESTS "Build performance tests to be run with callgrind" OFF)

#option(ENABLE_CACHE "Enable data caching for schemas and hash tables for data (time-efficient at the cost of increased space-complexity)" ON)
#option(ENABLE_LATEST_REVISIONS "Enable reusing of latest revisions of schemas" ON)
#set(PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/libyang" CACHE STRING "Directory with libyang plugins (extensions and user types)")

#if(ENABLE_CACHE)
#    set(LY_ENABLED_CACHE 1)
#endif()
#if(ENABLE_LATEST_REVISIONS)
#    set(LY_ENABLED_LATEST_REVISIONS 1)
#endif()

if(ENABLE_COVERAGE)
    if(NOT ENABLE_BUILD_TESTS)
        message(WARNING "you cannot generage coverage when tests are disabled. Enable test by additing parameter -DENABLE_BUILD_TESTS=ON or run cmake in some debug mode")
        set(ENABLE_COVERAGE OFF)
    endif()

    find_program(PATH_GCOV NAMES gcov)
    if(NOT PATH_GCOV)
        message(WARNING "'gcov' executable not found! Disabling building code coverage report.")
        set(ENABLE_COVERAGE OFF)
    endif()

    find_program(PATH_LCOV NAMES lcov)
    if(NOT PATH_LCOV)
        message(WARNING "'lcov' executable not found! Disabling building code coverage report.")
        set(ENABLE_COVERAGE OFF)
    endif()

    find_program(PATH_GENHTML NAMES genhtml)
    if(NOT PATH_GENHTML)
        message(WARNING "'genhtml' executable not found! Disabling building code coverage report.")
        set(ENABLE_COVERAGE OFF)
    endif()

    if(NOT CMAKE_COMPILER_IS_GNUCC)
        message(WARNING "Compiler is not gcc! Coverage may break the tests!")
    endif()

    if(ENABLE_COVERAGE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    endif()
endif()

# by default build shared library
# static build requires static libpcre2 library
option(ENABLE_STATIC "Build static (.a) library" OFF)

#
# targets
#

# link compat
use_compat()

# create static libyang library
if(ENABLE_STATIC)
    add_definitions(-DSTATIC)
    set(CMAKE_EXE_LINKER_FLAGS -static)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)       # remove -Wl,-Bdynamic
    set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
    add_library(yang STATIC ${libsrc} $<TARGET_OBJECTS:compat>)
else()
    set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
    add_library(yangobj OBJECT ${libsrc})
    add_library(yang SHARED $<TARGET_OBJECTS:yangobj> $<TARGET_OBJECTS:compat>)

    #link dl
    target_link_libraries(yang ${CMAKE_DL_LIBS})

    set_target_properties(yangobj PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")
endif()

set_target_properties(yang PROPERTIES VERSION ${LIBYANG_SOVERSION_FULL} SOVERSION ${LIBYANG_SOVERSION})

# link math
target_link_libraries(yang m)

# find pthreads
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
if(ENABLE_STATIC)
    target_link_libraries(yang -Wl,--whole-archive ${CMAKE_THREAD_LIBS_INIT} -Wl,--no-whole-archive)
else()
    target_link_libraries(yang ${CMAKE_THREAD_LIBS_INIT})
endif()

# find PCRE2 library
unset(PCRE2_LIBRARY CACHE)
find_package(PCRE2 10.30 REQUIRED)
include_directories(${PCRE2_INCLUDE_DIRS})
target_link_libraries(yang ${PCRE2_LIBRARIES})

install(TARGETS yang DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${headers} ${PROJECT_BINARY_DIR}/src/version.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libyang)

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    # generate and install pkg-config file
    configure_file("libyang.pc.in" "libyang.pc" @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libyang.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
    # check that pkg-config includes the used path
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable pc_path pkg-config RESULT_VARIABLE RETURN OUTPUT_VARIABLE PC_PATH ERROR_QUIET)
    if(RETURN EQUAL 0)
	string(STRIP "${PC_PATH}" PC_PATH)
	set(PC_PATH "${PC_PATH}:$ENV{PKG_CONFIG_PATH}")
        string(REGEX MATCH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig" SUBSTR "${PC_PATH}")
        string(LENGTH "${SUBSTR}" SUBSTR_LEN)
        if(SUBSTR_LEN EQUAL 0)
            message(WARNING "pkg-config will not detect the new package after installation, adjust PKG_CONFIG_PATH using \"export PKG_CONFIG_PATH=\${PKG_CONFIG_PATH}:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig\".")
        endif()
    endif()
endif()

# YANG extensions plugins
#set(EXTENSIONS_LIST "nacm" "metadata" "yangdata")
# if the tests are enabled, build libyang_ext_test
if(ENABLE_BUILD_TESTS)
    find_package(CMocka 1.0.0)
#    if(CMOCKA_FOUND AND CMAKE_BUILD_TYPE MATCHES Debug)
#        list(APPEND EXTENSIONS_LIST "libyang_ext_test")
#    endif(CMOCKA_FOUND AND CMAKE_BUILD_TYPE MATCHES Debug)
endif(ENABLE_BUILD_TESTS)

#if(ENABLE_STATIC)
#    set(EXTENSIONS_LIST_SIZE " 0 ")
#    set(ITEM 0)
#    foreach(EXTENSION ${EXTENSIONS_LIST})
#        add_library(${EXTENSION} STATIC "src/extensions/${EXTENSION}.c")
#        target_link_libraries(yang ${EXTENSION})
#        set(EXTENSIONS_LIST_SIZE "${EXTENSIONS_LIST_SIZE} + lyext_size(${EXTENSION})")
#        set(EXTERN_EXTENSIONS_LIST "${EXTERN_EXTENSIONS_LIST}extern struct lyext_plugin_list ${EXTENSION}[];\n")
#        set(MEMCPY_EXTENSIONS_LIST "${MEMCPY_EXTENSIONS_LIST}    lyext_add(plugin, count, ${EXTENSION});\n")
#        set(STATIC_LOADED_PLUGINS "${STATIC_LOADED_PLUGINS} \"${EXTENSION}\",")
#        MATH(EXPR ITEM "${ITEM}+1")
#    endforeach()
#else()
#    add_subdirectory(src/extensions)
#endif(ENABLE_STATIC)

# YANG user types plugins ("user_ipv4" is just an example, not installed by default)
#set(USER_TYPE_LIST "user_date_and_time")
#if(ENABLE_STATIC)
#    set(USER_TYPE_LIST_SIZE " 0 ")
#    foreach(USER_TYPE ${USER_TYPE_LIST})
#        add_library(${USER_TYPE} STATIC "src/user_types/${USER_TYPE}.c")
#        target_link_libraries(yang ${USER_TYPE})
#        set(USER_TYPE_LIST_SIZE "${USER_TYPE_LIST_SIZE} + lytype_size(${USER_TYPE})")
#        set(EXTERN_USER_TYPE_LIST "${EXTERN_USER_TYPE_LIST}extern struct lytype_plugin_list ${USER_TYPE}[];\n")
#        set(MEMCPY_USER_TYPE_LIST "${MEMCPY_USER_TYPE_LIST}    lytype_add(plugin, count, ${USER_TYPE});\n")
#        set(STATIC_LOADED_PLUGINS "${STATIC_LOADED_PLUGINS} \"${USER_TYPE}\",")
#        MATH(EXPR ITEM "${ITEM}+1")
#    endforeach()
#    set(STATIC_LOADED_PLUGINS_COUNT "${ITEM}")
#else()
#    add_subdirectory(src/user_types)
#endif(ENABLE_STATIC)
#
#configure_file(${PROJECT_SOURCE_DIR}/src/plugin_config.h.in ${PROJECT_BINARY_DIR}/src/plugin_config.h)

# tests
if(ENABLE_VALGRIND_TESTS)
    set(ENABLE_BUILD_TESTS ON)
endif()

if(ENABLE_BUILD_TESTS)
    if(CMOCKA_FOUND)
        enable_testing()
        add_subdirectory(tests)
    else()
        message(STATUS "Disabling tests because of missing CMocka")
        set(ENABLE_BUILD_TESTS OFF)
    endif()
endif()

if(ENABLE_FUZZ_TARGETS)
    set(FUZZER "AFL" CACHE STRING "fuzzer type")
    if(FUZZER STREQUAL "LibFuzzer")
        if (NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
            message(FATAL_ERROR "LibFuzzer works only with clang")
        endif()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
    endif()
endif()

# tools - yanglint, yangre
add_subdirectory(tools)

# generate doxygen documentation for libyang API
libyang_doc()

# generate API/ABI report
if ("${BUILD_TYPE_UPPER}" STREQUAL "ABICHECK")
    libyang_abicheck()
endif()

# source code format
if ("${BUILD_TYPE_UPPER}" STREQUAL "DEBUG")
	source_format(${format_sources})
endif()

# clean cmake cache
add_custom_target(cclean
        COMMAND make clean
        COMMAND find . -iname '*cmake*' -not -name CMakeLists.txt -not -path './CMakeModules*' -exec rm -rf {} +
        COMMAND rm -rf Makefile Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

