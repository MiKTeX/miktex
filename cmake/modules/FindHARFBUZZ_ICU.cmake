## FindHARFBUZZ_ICU.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(HARFBUZZ_ICU_INCLUDE_DIR
    NAMES
        hb-icu.h
    PATH_SUFFIXES
        harfbuzz
)

find_library(HARFBUZZ_ICU_LIBRARY
    NAMES
        harfbuzz-icu
)

find_package_handle_standard_args(HARFBUZZ_ICU DEFAULT_MSG HARFBUZZ_ICU_LIBRARY HARFBUZZ_ICU_INCLUDE_DIR)

if(HARFBUZZ_ICU_FOUND)
    set(HARFBUZZ_ICU_INCLUDE_DIRS ${HARFBUZZ_ICU_INCLUDE_DIR})
    set(HARFBUZZ_ICU_LIBRARIES ${HARFBUZZ_ICU_LIBRARY})
else()
    set(HARFBUZZ_ICU_INCLUDE_DIRS)
    set(HARFBUZZ_ICU_LIBRARIES)
endif()

mark_as_advanced(HARFBUZZ_ICU_LIBRARY HARFBUZZ_ICU_INCLUDE_DIR)
