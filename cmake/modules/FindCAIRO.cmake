## FindCAIRO.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(CAIRO_INCLUDE_DIR
    NAMES
        cairo.h
    PATH_SUFFIXES
        cairo
)

find_library(CAIRO_LIBRARY
    NAMES
        cairo
)

find_package_handle_standard_args(CAIRO DEFAULT_MSG CAIRO_LIBRARY CAIRO_INCLUDE_DIR)

if(CAIRO_FOUND)
    set(CAIRO_INCLUDE_DIRS ${CAIRO_INCLUDE_DIR})
    set(CAIRO_LIBRARIES ${CAIRO_LIBRARY})
else()
    set(CAIRO_INCLUDE_DIRS)
    set(CAIRO_LIBRARIES)
endif()

mark_as_advanced(CAIRO_LIBRARY CAIRO_INCLUDE_DIR)
