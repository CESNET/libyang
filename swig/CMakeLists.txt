set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-g -O0")
# temporary bugfix
add_compile_options(-std=c++11)

# find SWIG package
if(GEN_LANGUAGE_BINDINGS)
    find_package(SWIG REQUIRED)
    if(NOT SWIG_FOUND)
        message(WARNING "SWIG library not found")
    else()
        include(${SWIG_USE_FILE})
    endif()
endif()

# find Python package
if(GEN_PYTHON_BINDINGS AND SWIG_FOUND)
    message(STATUS "Python version ${GEN_PYTHON_VERSION} was selected")
    if(${GEN_PYTHON_VERSION} STREQUAL "2")
        find_package(PythonLibs 2 REQUIRED)
        find_package(PythonInterp REQUIRED)
        if(NOT PYTHONLIBS_FOUND)
            message(WARNING "Did not found Python version 2.x")
            message(STATUS "Sysrepo supports Python 2.x and Python 3.x")
        endif()
    elseif(${GEN_PYTHON_VERSION} STREQUAL "3")
        find_package(PythonLibs 3 REQUIRED)
        find_package(PythonInterp REQUIRED)
        if(NOT PYTHONLIBS_FOUND)
            message(WARNING "Did not found Python version 3.x")
            message(STATUS "Sysrepo supports Python 2.x and Python 3.x")
        endif()
    else()
        message(WARNING "Sysrepo supports Python 2.x and Python 3.x")
    endif()
endif()

project(libyang_cpp)
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

add_library(libyang_cpp SHARED ${LIBYANG_CPP_SOURCES})
SET_TARGET_PROPERTIES(libyang_cpp PROPERTIES
    VERSION ${LIBYANG_VERSION} SOVERSION ${LIBYANG_SOVERSION} PREFIX "")

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

include_directories(${CMAKE_SOURCE_DIR})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/cpp/src")
target_link_libraries(libyang_cpp yang)

# install binary
install(TARGETS libyang_cpp DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${LIBYANG_HPP_SOURCES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libyang)

if (GEN_CPP_BINDINGS)
    add_subdirectory(cpp/examples)
endif()

if(PYTHONLIBS_FOUND AND PYTHONINTERP_FOUND AND (${GEN_PYTHON_VERSION} STREQUAL "2" OR ${GEN_PYTHON_VERSION} STREQUAL "3"))
    add_subdirectory(python)
endif()

if(GEN_JAVASCRIPT_BINDING)
    include(javascript/CMakeLists.txt)
endif()
