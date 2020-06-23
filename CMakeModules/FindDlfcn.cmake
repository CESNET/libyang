# Copyright (C) 2020 Neural Technologies.
# Created by Leonce Mekinda <leonce.mekinda@neuralt.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
#
# - Find dlfcn
# Find the dlfcn headers and libraries.
#
# DLFCN_INCLUDE_DIRS - where to find dlfcn.h, etc.
# DLFCN_LIBRARIES - List of libraries when using dlfcn.
# DLFCN_FOUND - True if pthread found.

# Look for the header file.
find_path(DLFCN_INCLUDE_DIR NAMES dlfcn.h)

# Look for the library.
find_library(DLFCN_LIBRARY NAMES dlfcn)

# Handle the QUIETLY and REQUIRED arguments and set DLFCN_FOUND to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(dlfcn DEFAULT_MSG DLFCN_LIBRARY DLFCN_INCLUDE_DIR)

# Copy the results to the output variables.
if(DLFCN_FOUND)
    set(DLFCN_LIBRARIES ${DLFCN_LIBRARY})
    set(DLFCN_INCLUDE_DIRS ${DLFCN_INCLUDE_DIR})
else(DLFCN_FOUND)
    set(DLFCN_LIBRARIES)
    set(DLFCN_INCLUDE_DIRS)
endif()

mark_as_advanced(DLFCN_INCLUDE_DIRS DLFCN_LIBRARIES)
