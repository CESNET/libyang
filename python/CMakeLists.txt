if(NOT ${GEN_PYTHON_VERSION} EQUAL "3")
    message(FATAL_ERROR "Python cffi bindings only support python 3.")
endif()

unset(PYTHON_LIBRARY CACHE)
unset(PYTHON_EXECUTABLE CACHE)
unset(PYTHON_INCLUDE_DIR CACHE)

find_package(PythonInterp 3 REQUIRED)
find_package(PythonLibs 3 REQUIRED)
foreach(dep setuptools cffi)
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} -B -c "import ${dep}"
        RESULT_VARIABLE PYTHON_DEP_RESULT)
    if(NOT ${PYTHON_DEP_RESULT} EQUAL 0)
        message(FATAL_ERROR "Missing required python module: ${dep}")
    endif()
endforeach()

set(PYTHON_MODULE_PATH_CMD "
from distutils.sysconfig import get_python_lib
print(get_python_lib(prefix='${CMAKE_INSTALL_PREFIX}', plat_specific=True))
")
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -B -c "${PYTHON_MODULE_PATH_CMD}"
    OUTPUT_VARIABLE PYTHON_MODULE_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(PYTHON_LIB_BUILDDIR_CMD "
import sys
from distutils.util import get_platform
print('lib.' + get_platform() + '-{}.{}'.format(*sys.version_info[:2]))
")
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -B -c "${PYTHON_LIB_BUILDDIR_CMD}"
    OUTPUT_VARIABLE PYTHON_LIB_BUILDDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
configure_file(pydistutils.cfg.in .pydistutils.cfg @ONLY)
# Cmake does not support exporting specific environment variables for
# custom_target commands. The only way is to use a shell script.
configure_file(run_python.sh.in run_python.sh @ONLY)
configure_file(
    ${CMAKE_SOURCE_DIR}/src/libyang.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/libyang/libyang.h @ONLY)
foreach(header ${headers})
    configure_file(
        ${CMAKE_SOURCE_DIR}/${header}
        ${CMAKE_CURRENT_BINARY_DIR}/include/libyang/ COPYONLY)
endforeach()

add_custom_target(
    python_cffi ALL
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_python.sh ./setup.py build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS yang ${EXTENSIONS_LIST} ${USER_TYPE_LIST})
add_custom_target(
    python_cffi_sdist ALL
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_python.sh ./setup.py sdist
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(ENABLE_BUILD_TESTS)
    set(PYTHON_TESTS
        tests/test_context.py
        tests/test_data.py
        tests/test_diff.py
        tests/test_schema.py)
    foreach(test_file ${PYTHON_TESTS})
        string(
            REGEX REPLACE "tests/test_([a-z]+)\\.py" "test_python_\\1"
            test_name ${test_file})
        add_test(
            NAME ${test_name}
            COMMAND ${PYTHON_EXECUTABLE} -B -m unittest -cv ${test_file}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
        set_property(
            TEST ${test_name}
            PROPERTY ENVIRONMENT
                PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/${PYTHON_LIB_BUILDDIR}
                LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}
                LIBYANG_EXTENSIONS_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/extensions
                LIBYANG_USER_TYPES_PLUGINS_DIR=${CMAKE_BINARY_DIR}/src/user_types)
    endforeach()
endif()

install(CODE "
execute_process(
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_python.sh ./setup.py install --root=\$ENV{DESTDIR}/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
")
