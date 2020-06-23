# Copyright (C) 2020 Neural Technologies.
# Created by Leonce Mekinda <leonce.mekinda@neuralt.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
#
# - Find dirent
# Find the dirent.h header file
#
# DIRENT_INCLUDE_DIRS - where to find dirent.h, etc.
# DIRENT_FOUND - True if dirent.h found.

# Look for the header file.
find_path(DIRENT_INCLUDE_DIR NAMES dirent.h)

# Handle the QUIETLY and REQUIRED arguments and set DIRENT_FOUND to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(dirent DEFAULT_MSG DIRENT_INCLUDE_DIR)

# Copy the results to the output variables.
if(DIRENT_FOUND)
    set(DIRENT_INCLUDE_DIRS ${DIRENT_INCLUDE_DIR})
else(DIRENT_FOUND)
    set(DIRENT_INCLUDE_DIRS)
endif()

mark_as_advanced(DIRENT_INCLUDE_DIRS)
