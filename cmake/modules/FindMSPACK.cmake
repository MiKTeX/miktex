## FindMSPACK.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(MSPACK_INCLUDE_DIR
    NAMES
        mspack.h
)

find_library(MSPACK_LIBRARY
    NAMES
        mspack
)

find_package_handle_standard_args(MSPACK DEFAULT_MSG MSPACK_LIBRARY MSPACK_INCLUDE_DIR)

if(MSPACK_FOUND)
    set(MSPACK_INCLUDE_DIRS ${MSPACK_INCLUDE_DIR})
    set(MSPACK_LIBRARIES ${MSPACK_LIBRARY})
else()
    set(MSPACK_INCLUDE_DIRS)
    set(MSPACK_LIBRARIES)
endif()

mark_as_advanced(MSPACK_LIBRARY MSPACK_INCLUDE_DIR)
