set(JAVA_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(JUNIT_JAR ${JAVA_TEST_DIR}/junit-4.12.jar:${JAVA_TEST_DIR}/hamcrest-core-1.3.jar)
set(JAVA_TEST_SRC
        LibyangTest
        TreeDataTest
        TreeSchemaTest)
configure_file("${JAVA_TEST_DIR}/Constants.java.in" "${JAVA_TEST_DIR}/Constants.java" ESCAPE_QUOTES @ONLY)

add_custom_target("java_tests")

foreach(JTEST ${JAVA_TEST_SRC})

    add_custom_command(TARGET "java_tests"
            COMMAND
            ${Java_JAVAC_EXECUTABLE} -cp .:${NATIVE_JAR}:${JUNIT_JAR} ${JAVA_TEST_DIR}/${JTEST}.java)

    add_test(NAME java_${JTEST}
            COMMAND ${Java_JAVA_EXECUTABLE}  -Djava.library.path=${CMAKE_SWIG_OUTDIR} -cp ..:.:${NATIVE_JAR}:${JUNIT_JAR} ${JTEST}
            WORKING_DIRECTORY ${JAVA_TEST_DIR})

endforeach()