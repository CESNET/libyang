# - Find pcre
# Find the native PCRE2 headers and libraries.
#
# PCRE2_INCLUDE_DIRS    - where to find pcre.h, etc.
# PCRE2_LIBRARIES   - List of libraries when using pcre.
# PCRE2_FOUND   - True if pcre found.
include(FindPackageHandleStandardArgs)

if(PCRE2_LIBRARIES AND PCRE2_INCLUDE_DIRS)
    # in cache already
    set(PCRE2_FOUND TRUE)
else()
    find_path(PCRE2_INCLUDE_DIR
        NAMES
        pcre2.h
        PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
        ${CMAKE_INCLUDE_PATH}
        ${CMAKE_INSTALL_PREFIX}/include)

    # Look for the library.
    find_library(PCRE2_LIBRARY
        NAMES
        libpcre2.a
        pcre2-8
        PATHS
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /usr/local/lib64
        /opt/local/lib
        /sw/lib
        ${CMAKE_LIBRARY_PATH}
        ${CMAKE_INSTALL_PREFIX}/lib)

    if(PCRE2_LIBRARY)
        # Check required version
        execute_process(COMMAND pcre2-config --version OUTPUT_VARIABLE PCRE2_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    set(PCRE2_INCLUDE_DIRS ${PCRE2_INCLUDE_DIR})
    set(PCRE2_LIBRARIES ${PCRE2_LIBRARY})
    mark_as_advanced(PCRE2_INCLUDE_DIRS PCRE2_LIBRARIES)

    # Handle the QUIETLY and REQUIRED arguments and set PCRE2_FOUND to TRUE if all listed variables are TRUE.
    find_package_handle_standard_args(PCRE2 FOUND_VAR PCRE2_FOUND
        REQUIRED_VARS PCRE2_LIBRARY PCRE2_INCLUDE_DIR
        VERSION_VAR PCRE2_VERSION)
endif()
