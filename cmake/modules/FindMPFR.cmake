## FindMPFR.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(MPFR_INCLUDE_DIR
    NAMES
        mpfr.h
)

find_library(MPFR_LIBRARY
    NAMES
        mpfr
)

find_package_handle_standard_args(MPFR DEFAULT_MSG MPFR_LIBRARY MPFR_INCLUDE_DIR)

if(MPFR_FOUND)
    set(MPFR_INCLUDE_DIRS ${MPFR_INCLUDE_DIR})
    set(MPFR_LIBRARIES ${MPFR_LIBRARY})
else()
    set(MPFR_INCLUDE_DIRS)
    set(MPFR_LIBRARIES)
endif()

mark_as_advanced(MPFR_LIBRARY MPFR_INCLUDE_DIR)
