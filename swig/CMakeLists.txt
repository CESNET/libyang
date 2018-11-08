project(libyang-cpp)

set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_PACKAGE "-g -O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-g -O0")

# temporary bugfix
add_compile_options(-std=c++11)

# find SWIG package
if(GEN_LANGUAGE_BINDINGS)
    find_package(SWIG 3.0.12 REQUIRED)
    if(NOT SWIG_FOUND)
        message(WARNING "SWIG library not found")
    else()
        include(${SWIG_USE_FILE})
    endif()
endif()

# find Python package
if(GEN_PYTHON_BINDINGS AND SWIG_FOUND)
    if(ENABLE_STATIC)
        message(WARNING "Can't create a static Python module")
    else()
        foreach(CUR_PYTHON_VERSION ${GEN_PYTHON_VERSION})
            unset(PYTHON_EXECUTABLE CACHE)
            unset(PYTHON_INCLUDE_PATH CACHE)
            unset(PYTHON_EXT_SUFFIX CACHE)
            unset(PYTHON_MODULE_PATH CACHE)

            # find_package(Python...) does not do the right thing here.  we
            # need the extension suffix, and we need support for things like
            # "python3.6" + "python3.6dm" (debug)
            #
            # Note PYTHON_LIBRARIES is intentionally not set here.  Python
            # symbols and library linkage are provided by the interpreter that
            # is loading the module -- the dynamic linker uses the symbols
            # from the executable.  libpython should only be linked by people
            # embedding the python interpreter, not by extensions.

            find_program(PYTHON_EXECUTABLE NAMES python${CUR_PYTHON_VERSION})
            if ("${PYTHON_EXECUTABLE}" STREQUAL "PYTHON_EXECUTABLE-NOTFOUND")
                message(FATAL_ERROR "cannot find python${CUR_PYTHON_VERSION} executable")
            endif()

            execute_process(COMMAND ${PYTHON_EXECUTABLE} -c
                    "from distutils.sysconfig import get_config_var; print(get_config_var('INCLUDEPY'))"
                    OUTPUT_VARIABLE PYTHON_INCLUDE_PATH
                    OUTPUT_STRIP_TRAILING_WHITESPACE )
            execute_process(COMMAND ${PYTHON_EXECUTABLE} -c
                    "from distutils.sysconfig import get_config_var; print(get_config_var('EXT_SUFFIX'))"
                    OUTPUT_VARIABLE PYTHON_EXT_SUFFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE )
            execute_process(COMMAND ${PYTHON_EXECUTABLE} -c
                    "from distutils.sysconfig import get_python_lib; print(get_python_lib(plat_specific=True))"
                    OUTPUT_VARIABLE PYTHON_MODULE_PATH
                    OUTPUT_STRIP_TRAILING_WHITESPACE )

            message(STATUS "Python version ${CUR_PYTHON_VERSION}: ${PYTHON_EXT_SUFFIX}")

            add_subdirectory(python python${CUR_PYTHON_VERSION})
        endforeach(CUR_PYTHON_VERSION)
    endif()
endif()

set(LIBYANG_CPP_SOURCES
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Xml.cpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Libyang.cpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Tree_Data.cpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Tree_Schema.cpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Internal.cpp)

set(LIBYANG_HPP_SOURCES
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Xml.hpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Libyang.hpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Tree_Data.hpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Tree_Schema.hpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Internal.hpp)

FUNCTION(PREPEND var prefix)
    SET(listVar "")
    FOREACH(f ${ARGN})
        LIST(APPEND listVar "${prefix}/${f}")
    ENDFOREACH(f)
    SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

PREPEND(abs_libsrc ${CMAKE_SOURCE_DIR} ${libsrc})

if(ENABLE_STATIC)
    add_library(libyang-cpp STATIC ${LIBYANG_CPP_SOURCES})
else(ENABLE_STATIC)
    add_library(libyang-cpp SHARED ${LIBYANG_CPP_SOURCES})
endif(ENABLE_STATIC)
SET_TARGET_PROPERTIES(libyang-cpp PROPERTIES
    VERSION ${LIBYANG_VERSION} SOVERSION ${LIBYANG_SOVERSION} PREFIX "")

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

include_directories(${CMAKE_SOURCE_DIR})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/cpp/src")
target_link_libraries(libyang-cpp yang)

if(PKG_CONFIG_FOUND)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libyang-cpp.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/libyang-cpp.pc" @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libyang-cpp.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()

# install binary
install(TARGETS libyang-cpp DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${LIBYANG_HPP_SOURCES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libyang)

if (GEN_CPP_BINDINGS)
    add_subdirectory(cpp/examples)
    if (ENABLE_BUILD_TESTS)
        add_subdirectory(cpp/tests)
    endif()
endif()

if(NOT ENABLE_STATIC AND GEN_JAVASCRIPT_BINDINGS)
    message(WARNING "Can't create Javascript bindings with a shared library, please use -DENABLE_STATIC")
elseif(ENABLE_STATIC AND GEN_JAVASCRIPT_BINDINGS)
    add_subdirectory(javascript)
endif()

if(ENABLE_STATIC AND GEN_JAVA_BINDINGS)
    message(WARNING "Can't create a static Java module")
elseif(NOT ENABLE_STATIC AND GEN_JAVA_BINDINGS)
    add_subdirectory(java)
endif()
