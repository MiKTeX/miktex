## FindAPR.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(APR_INCLUDE_DIR
    NAMES
        apr.h
    PATH_SUFFIXES
        apr-1
        apr-1.0
)

find_library(APR_LIBRARY
    NAMES
        apr-1
)

find_package_handle_standard_args(APR DEFAULT_MSG APR_LIBRARY APR_INCLUDE_DIR)

if(APR_FOUND)
    set(APR_INCLUDE_DIRS ${APR_INCLUDE_DIR})
    set(APR_LIBRARIES ${APR_LIBRARY})
else()
    set(APR_INCLUDE_DIRS)
    set(APR_LIBRARIES)
endif()

mark_as_advanced(APR_LIBRARY APR_INCLUDE_DIR)
