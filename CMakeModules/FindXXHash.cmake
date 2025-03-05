# Try to find XXHash
# Once done this will define
#
# Read-Only variables:
#  XXHASH_FOUND - system has XXHash
#  XXHASH_INCLUDE_DIR - the XXHash include directory
#  XXHASH_LIBRARY - Link these to use XXHash

find_path(XXHASH_INCLUDE_DIR
    NAMES
        xxhash.h
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
        ${CMAKE_INCLUDE_PATH}
        ${CMAKE_INSTALL_PREFIX}/include
)

find_library(XXHASH_LIBRARY
    NAMES
        xxhash
        libxxhash
    PATHS
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
find_package_handle_standard_args(XXHash FOUND_VAR XXHASH_FOUND REQUIRED_VARS XXHASH_INCLUDE_DIR XXHASH_LIBRARY)
