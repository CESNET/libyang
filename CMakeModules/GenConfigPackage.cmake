# Generate the CMake config package of a library for the build tree and the installation
#
# gen_config_package(LIB_NAME LIB_SOVERSION_FULL [EXPORTS <export-set>...] [PATH_VARS <var>...])
#   LIB_NAME            package name, the template is <LIB_NAME>Config.cmake.in in the source root
#   LIB_SOVERSION_FULL  package version, the full SO version of the library
#   EXPORTS             optional, export sets installed besides <LIB_NAME>Targets
#   PATH_VARS           optional, template variables with installed paths, <var>_BUILD holds the build tree path
#
# The template reads its other variables from the scope of the caller.
include(CMakePackageConfigHelpers)

function(gen_config_package LIB_NAME LIB_SOVERSION_FULL)
    # the optional arguments into LIB_EXPORTS and LIB_PATH_VARS
    cmake_parse_arguments(PARSE_ARGV 2 LIB "" "" "EXPORTS;PATH_VARS")
    set(LIB_CONFIG_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_NAME})
    set(LIB_TEMPLATE ${PROJECT_SOURCE_DIR}/${LIB_NAME}Config.cmake.in)

    # create the version file
    write_basic_package_version_file(${PROJECT_BINARY_DIR}/${LIB_NAME}ConfigVersion.cmake
        VERSION ${LIB_SOVERSION_FULL} COMPATIBILITY SameMajorVersion)
    install(FILES ${PROJECT_BINARY_DIR}/${LIB_NAME}ConfigVersion.cmake DESTINATION ${LIB_CONFIG_DIR})

    # the targets file of each export set, for the build tree and the installation
    foreach(LIB_EXPORT IN ITEMS ${LIB_NAME}Targets ${LIB_EXPORTS})
        export(EXPORT ${LIB_EXPORT} NAMESPACE ${LIB_NAME}:: FILE ${PROJECT_BINARY_DIR}/${LIB_EXPORT}.cmake)
        install(EXPORT ${LIB_EXPORT} NAMESPACE ${LIB_NAME}:: DESTINATION ${LIB_CONFIG_DIR})
    endforeach()

    # the installed config, generated aside so that it does not replace the build tree one
    configure_package_config_file(${LIB_TEMPLATE} ${PROJECT_BINARY_DIR}/CMakeFiles/${LIB_NAME}Config.cmake
        INSTALL_DESTINATION ${LIB_CONFIG_DIR} PATH_VARS ${LIB_PATH_VARS})
    install(FILES ${PROJECT_BINARY_DIR}/CMakeFiles/${LIB_NAME}Config.cmake DESTINATION ${LIB_CONFIG_DIR})

    # each path variable set to its <var>_BUILD value
    foreach(LIB_VAR IN LISTS LIB_PATH_VARS)
        set(${LIB_VAR} ${${LIB_VAR}_BUILD})
    endforeach()

    # the build tree config, the build directory is both its location and its prefix
    configure_package_config_file(${LIB_TEMPLATE} ${PROJECT_BINARY_DIR}/${LIB_NAME}Config.cmake
        INSTALL_DESTINATION ${PROJECT_BINARY_DIR} INSTALL_PREFIX ${PROJECT_BINARY_DIR} PATH_VARS ${LIB_PATH_VARS})
endfunction()
