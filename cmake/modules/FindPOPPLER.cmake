## FindPOPPLER.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(POPPLER_INCLUDE_DIR
    NAMES
        PDFDoc.h
    PATH_SUFFIXES
        poppler
)

find_library(POPPLER_LIBRARY
    NAMES
        poppler
)

find_package_handle_standard_args(POPPLER DEFAULT_MSG POPPLER_LIBRARY POPPLER_INCLUDE_DIR)

if(POPPLER_FOUND)
    set(POPPLER_INCLUDE_DIRS ${POPPLER_INCLUDE_DIR})
    set(POPPLER_LIBRARIES ${POPPLER_LIBRARY})
else()
    set(POPPLER_INCLUDE_DIRS)
    set(POPPLER_LIBRARIES)
endif()

if(POPPLER_FOUND)
    file(STRINGS "${POPPLER_INCLUDE_DIR}/poppler-config.h" POPPLER_CONFIG_H REGEX "^#define POPPLER_VERSION \"[^\"]*\"$")
    string(REGEX REPLACE "^.*POPPLER_VERSION \"([0-9]+).*$" "\\1" POPPLER_VERSION_MAJOR "${POPPLER_CONFIG_H}")
    string(REGEX REPLACE "^.*POPPLER_VERSION \"[0-9]+\\.([0-9]+).*$" "\\1" POPPLER_VERSION_MINOR  "${POPPLER_CONFIG_H}")
    string(REGEX REPLACE "^.*POPPLER_VERSION \"[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" POPPLER_VERSION_PATCH "${POPPLER_CONFIG_H}")
    set(POPPLER_VERSION_TWEAK "0")
    set(POPPLER_VERSION_COUNT "3")
    set(POPPLER_VERSION "${POPPLER_VERSION_MAJOR}.${POPPLER_VERSION_MINOR}.${POPPLER_VERSION_PATCH}")
endif()

mark_as_advanced(POPPLER_LIBRARY POPPLER_INCLUDE_DIR)
