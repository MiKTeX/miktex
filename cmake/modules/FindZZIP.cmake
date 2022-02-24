## FindZZIP.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(ZZIP_INCLUDE_DIR
    NAMES
        zzip/lib.h
)

find_library(ZZIP_LIBRARY
    NAMES
        zzip
        zzip-0
)

find_package_handle_standard_args(ZZIP DEFAULT_MSG ZZIP_LIBRARY ZZIP_INCLUDE_DIR)

if(ZZIP_FOUND)
    set(ZZIP_INCLUDE_DIRS ${ZZIP_INCLUDE_DIR})
    set(ZZIP_LIBRARIES ${ZZIP_LIBRARY})
else()
    set(ZZIP_INCLUDE_DIRS)
    set(ZZIP_LIBRARIES)
endif()

mark_as_advanced(ZZIP_LIBRARY ZZIP_INCLUDE_DIR)
