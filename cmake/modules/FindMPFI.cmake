## FindMPFI.cmake
##
## Copyright (C) 2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(MPFI_INCLUDE_DIR
    NAMES
        mpfi.h
)

find_library(MPFI_LIBRARY
    NAMES
        mpfi
)

find_package_handle_standard_args(MPFI DEFAULT_MSG MPFI_LIBRARY MPFI_INCLUDE_DIR)

if(MPFI_FOUND)
    set(MPFI_INCLUDE_DIRS ${MPFI_INCLUDE_DIR})
    set(MPFI_LIBRARIES ${MPFI_LIBRARY})
else()
    set(MPFI_INCLUDE_DIRS)
    set(MPFI_LIBRARIES)
endif()

mark_as_advanced(MPFI_LIBRARY MPFI_INCLUDE_DIR)
