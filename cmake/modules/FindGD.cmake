## FindGD.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(GD_INCLUDE_DIR
    NAMES
        gd.h
    PATH_SUFFIXES
        gd
)

find_library(GD_LIBRARY
    NAMES
        gd
)

find_package_handle_standard_args(GD DEFAULT_MSG GD_LIBRARY GD_INCLUDE_DIR)

if(GD_FOUND)
    set(GD_INCLUDE_DIRS ${GD_INCLUDE_DIR})
    set(GD_LIBRARIES ${GD_LIBRARY})
else()
    set(GD_INCLUDE_DIRS)
    set(GD_LIBRARIES)
endif()

mark_as_advanced(GD_LIBRARY GD_INCLUDE_DIR)
