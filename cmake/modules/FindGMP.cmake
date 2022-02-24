## FindGMP.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(GMP_INCLUDE_DIR
    NAMES
        gmp.h
)

find_library(GMP_LIBRARY
    NAMES
        gmp
)

find_package_handle_standard_args(GMP DEFAULT_MSG GMP_LIBRARY GMP_INCLUDE_DIR)

if(GMP_FOUND)
    set(GMP_INCLUDE_DIRS ${GMP_INCLUDE_DIR})
    set(GMP_LIBRARIES ${GMP_LIBRARY})
else()
    set(GMP_INCLUDE_DIRS)
    set(GMP_LIBRARIES)
endif()

mark_as_advanced(GMP_LIBRARY GMP_INCLUDE_DIR)
