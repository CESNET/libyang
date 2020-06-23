# Copyright (C) 2020 Neural Technologies.
# Created by Leonce Mekinda <leonce.mekinda@neuralt.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
#
# - Find POSIX Threads
# Find the POSIX Threads headers and libraries.
#
# PTHREAD_INCLUDE_DIRS - where to find pthread.h, etc.
# PTHREAD_LIBRARIES - List of libraries when using pthreads.
# PTHREAD_FOUND - True if pthread found.

# Look for the header file.
find_path(PTHREAD_INCLUDE_DIR NAMES pthread.h)

# Look for the library.
find_library(PTHREAD_LIBRARY NAMES pthread)

# Handle the QUIETLY and REQUIRED arguments and set PTHREAD_FOUND to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pthread DEFAULT_MSG PTHREAD_LIBRARY PTHREAD_INCLUDE_DIR)

# Copy the results to the output variables.
if(PTHREAD_FOUND)
    set(PTHREAD_LIBRARIES ${PTHREAD_LIBRARY})
    set(PTHREAD_INCLUDE_DIRS ${PTHREAD_INCLUDE_DIR})
else(PTHREAD_FOUND)
    set(PTHREAD_LIBRARIES)
    set(PTHREAD_INCLUDE_DIRS)
endif()

mark_as_advanced(PTHREAD_INCLUDE_DIRS PTHREAD_LIBRARIES)
