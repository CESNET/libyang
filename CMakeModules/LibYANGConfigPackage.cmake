# Generate the LibYANG CMake config package for the build tree and the installation
#
# gen_libyang_config_package(SOVERSION <ver> VERSION <ver> HAVE_CBOR <bool>)
#   SOVERSION  package version, the full SO version of the library
#   VERSION    libyang project version
#   HAVE_CBOR  whether libyang supports the CBOR data format, an empty value means no
include(CMakePackageConfigHelpers)

function(gen_libyang_config_package)
    # the arguments into CONFIG_<keyword> variables, used by the template
    cmake_parse_arguments(PARSE_ARGV 0 CONFIG "" "SOVERSION;VERSION;HAVE_CBOR" "")
    set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/LibYANG)
    if(CONFIG_HAVE_CBOR)
        set(CONFIG_HAVE_CBOR TRUE)
    else()
        set(CONFIG_HAVE_CBOR FALSE)
    endif()

    # create the version file
    write_basic_package_version_file(${PROJECT_BINARY_DIR}/LibYANGConfigVersion.cmake
        VERSION ${CONFIG_SOVERSION} COMPATIBILITY SameMajorVersion)
    install(FILES ${PROJECT_BINARY_DIR}/LibYANGConfigVersion.cmake DESTINATION ${CONFIG_INSTALL_DIR})

    # the targets file, for the build tree and the installation
    export(EXPORT LibYANGTargets NAMESPACE LibYANG:: FILE ${PROJECT_BINARY_DIR}/LibYANGTargets.cmake)
    install(EXPORT LibYANGTargets NAMESPACE LibYANG:: DESTINATION ${CONFIG_INSTALL_DIR})

    # the installed config, generated aside so that it does not replace the build tree one
    configure_package_config_file(${PROJECT_SOURCE_DIR}/LibYANGConfig.cmake.in ${PROJECT_BINARY_DIR}/CMakeFiles/LibYANGConfig.cmake
        INSTALL_DESTINATION ${CONFIG_INSTALL_DIR})
    install(FILES ${PROJECT_BINARY_DIR}/CMakeFiles/LibYANGConfig.cmake DESTINATION ${CONFIG_INSTALL_DIR})

    # the build tree config, the build directory is both its location and its prefix
    configure_package_config_file(${PROJECT_SOURCE_DIR}/LibYANGConfig.cmake.in ${PROJECT_BINARY_DIR}/LibYANGConfig.cmake
        INSTALL_DESTINATION ${PROJECT_BINARY_DIR} INSTALL_PREFIX ${PROJECT_BINARY_DIR})
endfunction()
