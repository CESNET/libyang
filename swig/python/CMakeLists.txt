set(PYTHON_SWIG_BINDING libyang)
include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_SWIG_FLAGS "-c++")
set(CMAKE_SWIG_FLAGS "-I${PROJECT_SOURCE_DIR}")
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})

set_source_files_properties(${PYTHON_SWIG_BINDING}.i PROPERTIES CPLUSPLUS ON)

swig_add_module(${PYTHON_SWIG_BINDING} python ${PYTHON_SWIG_BINDING}.i)
swig_link_libraries(${PYTHON_SWIG_BINDING} ${PYTHON_LIBRARIES} libyang_cpp)

# Generate header with SWIG run-time functions
execute_process(COMMAND ${SWIG_EXECUTABLE} -python -external-runtime ${CMAKE_CURRENT_BINARY_DIR}/swigpyrun.h)

file(COPY "examples" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

execute_process(COMMAND
    ${PYTHON_EXECUTABLE} -c
    "from distutils.sysconfig import get_python_lib; print(get_python_lib())"
OUTPUT_VARIABLE PYTHON_MODULE_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)

install( FILES "${CMAKE_CURRENT_BINARY_DIR}/_${PYTHON_SWIG_BINDING}.so" DESTINATION ${PYTHON_MODULE_PATH})
install( FILES "${CMAKE_CURRENT_BINARY_DIR}/${PYTHON_SWIG_BINDING}.py" DESTINATION ${PYTHON_MODULE_PATH})
install( FILES "${CMAKE_CURRENT_BINARY_DIR}/swigpyrun.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libyang)
