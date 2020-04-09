set(PACKAGE "libyang")
set(CPP_PACKAGE "libyang-cpp")
set(PYTHON_PACKAGE "python3-yang")

find_program(RPM_BUILDER NAMES rpmbuild)

# setup package build
configure_file(${PROJECT_SOURCE_DIR}/packages/${PACKAGE}.spec.in ${PROJECT_BINARY_DIR}/build-packages/${PACKAGE}.spec)

# this just prints a message pointing to the new debian python script
add_custom_target(build-deb
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                  COMMAND build-packages/local-deb.sh
)

if(NOT RPM_BUILDER)
    message(STATUS "Missing tools (rpm package) for building RPM package.")
else()
    # target for local build rpm package
    message(STATUS "To build local RPM package, use \"build-rpm\" target.")
    string(REPLACE ${PROJECT_SOURCE_DIR} "." EXCLUDE_BUILD_DIR ${PROJECT_BINARY_DIR})
    add_custom_target(build-rpm
                      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                      COMMAND build-packages/local-rpm.sh
    )
    configure_file(${PROJECT_SOURCE_DIR}/packages/local-rpm.sh.in ${PROJECT_BINARY_DIR}/build-packages/local-rpm.sh @ONLY)
endif()
