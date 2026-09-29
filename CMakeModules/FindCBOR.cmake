# Try to find libcbor
# Once done this will define
#
# Read-Only variables:
#  CBOR_FOUND - system has libcbor
#  CBOR_INCLUDE_DIR - the libcbor include directory
#  CBOR_LIBRARY - Link these to use libcbor

find_path(CBOR_INCLUDE_DIR
    NAMES
        cbor.h
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
        ${CMAKE_INCLUDE_PATH}
        ${CMAKE_INSTALL_PREFIX}/include
)

find_library(CBOR_LIBRARY
    NAMES
        cbor
        libcbor
    PATHS
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /usr/local/lib64
        /opt/local/lib
        /sw/lib
        ${CMAKE_LIBRARY_PATH}
        ${CMAKE_INSTALL_PREFIX}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CBOR FOUND_VAR CBOR_FOUND REQUIRED_VARS CBOR_INCLUDE_DIR CBOR_LIBRARY)
