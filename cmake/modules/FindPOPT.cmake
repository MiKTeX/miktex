## FindPOPT.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(POPT_INCLUDE_DIR
    NAMES
        popt.h
)

find_library(POPT_LIBRARY
    NAMES
        popt
)

find_package_handle_standard_args(POPT DEFAULT_MSG POPT_LIBRARY POPT_INCLUDE_DIR)

if(POPT_FOUND)
    set(POPT_INCLUDE_DIRS ${POPT_INCLUDE_DIR})
    set(POPT_LIBRARIES ${POPT_LIBRARY})
else()
    set(POPT_INCLUDE_DIRS)
    set(POPT_LIBRARIES)
endif()

mark_as_advanced(POPT_LIBRARY POPT_INCLUDE_DIR)
