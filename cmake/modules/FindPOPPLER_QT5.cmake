## FindPOPPLER_QT5.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(POPPLER_QT5_INCLUDE_DIR
    NAMES
        poppler-qt5.h
    PATH_SUFFIXES
        poppler/qt5
)

find_library(POPPLER_QT5_LIBRARY
    NAMES
        poppler-qt5
)

find_package_handle_standard_args(POPPLER_QT5 DEFAULT_MSG POPPLER_QT5_LIBRARY POPPLER_QT5_INCLUDE_DIR)

if(POPPLER_QT5_FOUND)
    set(POPPLER_QT5_INCLUDE_DIRS ${POPPLER_QT5_INCLUDE_DIR})
    set(POPPLER_QT5_LIBRARIES ${POPPLER_QT5_LIBRARY})
else()
    set(POPPLER_QT5_INCLUDE_DIRS)
    set(POPPLER_QT5_LIBRARIES)
endif()

mark_as_advanced(POPPLER_QT5_LIBRARY POPPLER_QT5_INCLUDE_DIR)
