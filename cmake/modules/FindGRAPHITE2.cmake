## FindGRAPHITE2.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(GRAPHITE2_INCLUDE_DIR
    NAMES
        graphite2/Font.h
)

find_library(GRAPHITE2_LIBRARY
    NAMES
        graphite2
)

find_package_handle_standard_args(GRAPHITE2 DEFAULT_MSG GRAPHITE2_LIBRARY GRAPHITE2_INCLUDE_DIR)

if(GRAPHITE2_FOUND)
    set(GRAPHITE2_INCLUDE_DIRS ${GRAPHITE2_INCLUDE_DIR})
    set(GRAPHITE2_LIBRARIES ${GRAPHITE2_LIBRARY})
else()
    set(GRAPHITE2_INCLUDE_DIRS)
    set(GRAPHITE2_LIBRARIES)
endif()

mark_as_advanced(GRAPHITE2_LIBRARY GRAPHITE2_INCLUDE_DIR)
