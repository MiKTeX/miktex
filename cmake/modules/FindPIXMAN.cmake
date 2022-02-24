## FindPIXMAN.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(PIXMAN_INCLUDE_DIR
    NAMES
        pixman.h
    PATH_SUFFIXES
        pixman-1
)

find_library(PIXMAN_LIBRARY
    NAMES
        pixman-1
)

find_package_handle_standard_args(PIXMAN DEFAULT_MSG PIXMAN_LIBRARY PIXMAN_INCLUDE_DIR)

if(PIXMAN_FOUND)
    set(PIXMAN_INCLUDE_DIRS ${PIXMAN_INCLUDE_DIR})
    set(PIXMAN_LIBRARIES ${PIXMAN_LIBRARY})
else()
    set(PIXMAN_INCLUDE_DIRS)
    set(PIXMAN_LIBRARIES)
endif()

mark_as_advanced(PIXMAN_LIBRARY PIXMAN_INCLUDE_DIR)
