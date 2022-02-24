## FindHARFBUZZ_ICU.cmake
##
## Copyright (C) 2017-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(HARFBUZZ_INCLUDE_DIR
    NAMES
        hb.h
    PATH_SUFFIXES
        harfbuzz
)

find_library(HARFBUZZ_LIBRARY
    NAMES
        harfbuzz
)

find_package_handle_standard_args(HARFBUZZ DEFAULT_MSG HARFBUZZ_LIBRARY HARFBUZZ_INCLUDE_DIR)

if(HARFBUZZ_FOUND)
    set(HARFBUZZ_INCLUDE_DIRS ${HARFBUZZ_INCLUDE_DIR})
    set(HARFBUZZ_LIBRARIES ${HARFBUZZ_LIBRARY})
else()
    set(HARFBUZZ_INCLUDE_DIRS)
    set(HARFBUZZ_LIBRARIES)
endif()

mark_as_advanced(HARFBUZZ_LIBRARY HARFBUZZ_INCLUDE_DIR)
