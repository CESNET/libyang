set(PYTHON_SWIG_BINDING yang)
set(PYTHON_SWIG_TARGET yang${CUR_PYTHON_VERSION})
include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/cpp/src)

set(CMAKE_SWIG_FLAGS "-c++")
set(CMAKE_SWIG_FLAGS "-I${PROJECT_SOURCE_DIR}" "-I${PROJECT_SOURCE_DIR}/cpp/src")
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})

set_source_files_properties(${PYTHON_SWIG_BINDING}.i PROPERTIES CPLUSPLUS ON PREFIX "" SWIG_MODULE_NAME ${PYTHON_SWIG_BINDING})

if(${CMAKE_VERSION} VERSION_LESS "3.8.0")
    swig_add_module(${PYTHON_SWIG_TARGET} python ${PYTHON_SWIG_BINDING}.i)
else()
    swig_add_library(${PYTHON_SWIG_TARGET} LANGUAGE python SOURCES ${PYTHON_SWIG_BINDING}.i)
endif()
swig_link_libraries(${PYTHON_SWIG_TARGET} ${PYTHON_LIBRARIES} libyang-cpp)

set_target_properties(_${PYTHON_SWIG_TARGET} PROPERTIES OUTPUT_NAME "_yang${PYTHON_EXT_SUFFIX}" SUFFIX "")

# Generate header with SWIG run-time functions
execute_process(COMMAND ${SWIG_EXECUTABLE} -python -external-runtime ${CMAKE_CURRENT_BINARY_DIR}/swigpyrun.h)

file(COPY "examples" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

install( TARGETS _${PYTHON_SWIG_TARGET} DESTINATION ${PYTHON_MODULE_PATH})
install( FILES "${CMAKE_CURRENT_BINARY_DIR}/${PYTHON_SWIG_BINDING}.py" DESTINATION ${PYTHON_MODULE_PATH})
install( FILES "${CMAKE_CURRENT_BINARY_DIR}/swigpyrun.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libyang)

if(ENABLE_BUILD_TESTS)
    set(PY2_SWIG_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(PY2_TEST_DIR ${PY2_SWIG_DIR}/tests)

    file(COPY "tests" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    file(COPY "run_python_test.sh" DESTINATION "${PY2_SWIG_DIR}")

    macro(ADD_PYTHON_TEST TEST_NAME)
        add_test(NAME python_${TEST_NAME}
            COMMAND sh ${PY2_SWIG_DIR}/run_python_test.sh
            "${CMAKE_BINARY_DIR}/src:${CMAKE_BINARY_DIR}/tests:${PY2_TEST_DIR}"
            "${PY2_SWIG_DIR}:${PROJECT_SOURCE_DIR}/swig/python:${PYTHON_BUILD_DIR}"
            "${PYTHON_EXECUTABLE}"
            "${PY2_SWIG_DIR}/tests/${TEST_NAME}.py"
            "${CMAKE_BINARY_DIR}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
    endmacro(ADD_PYTHON_TEST)

    ADD_PYTHON_TEST(test_libyang)
    ADD_PYTHON_TEST(test_tree_data)
    ADD_PYTHON_TEST(test_tree_schema)

    add_custom_command(TARGET ${SWIG_MODULE_${PYTHON_SWIG_TARGET}_REAL_NAME} POST_BUILD
            COMMAND cp "${CMAKE_CURRENT_BINARY_DIR}/_yang${PYTHON_EXT_SUFFIX}" ${PY2_SWIG_DIR}/tests/_yang.so
            COMMAND cp "${CMAKE_CURRENT_BINARY_DIR}/${PYTHON_SWIG_BINDING}.py" ${PY2_SWIG_DIR}/tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.py.in" "${CMAKE_CURRENT_BINARY_DIR}/tests/config.py" ESCAPE_QUOTES @ONLY)
endif()
