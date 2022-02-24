## FindAPRUTIL.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(APRUTIL_INCLUDE_DIR
    NAMES
        apr_anylock.h
    PATH_SUFFIXES
        apr-1
        apr-1.0
)

find_library(APRUTIL_LIBRARY
    NAMES
        aprutil-1
)

find_package_handle_standard_args(APRUTIL DEFAULT_MSG APRUTIL_LIBRARY APRUTIL_INCLUDE_DIR)

if(APRUTIL_FOUND)
    set(APRUTIL_INCLUDE_DIRS ${APRUTIL_INCLUDE_DIR})
    set(APRUTIL_LIBRARIES ${APRUTIL_LIBRARY})
else()
    set(APRUTIL_INCLUDE_DIRS)
    set(APRUTIL_LIBRARIES)
endif()

mark_as_advanced(APRUTIL_LIBRARY APRUTIL_INCLUDE_DIR)
