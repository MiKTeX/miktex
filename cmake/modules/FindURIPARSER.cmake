## FindURIPARSER.cmake
##
## Copyright (C) 2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(URIPARSER_INCLUDE_DIR
    NAMES
        uriparser/Uri.h
)

find_library(URIPARSER_LIBRARY
    NAMES
        uriparser
)

find_package_handle_standard_args(URIPARSER DEFAULT_MSG URIPARSER_LIBRARY URIPARSER_INCLUDE_DIR)

if(URIPARSER_FOUND)
    set(URIPARSER_INCLUDE_DIRS ${URIPARSER_INCLUDE_DIR})
    set(URIPARSER_LIBRARIES ${URIPARSER_LIBRARY})
else()
    set(URIPARSER_INCLUDE_DIRS)
    set(URIPARSER_LIBRARIES)
endif()

mark_as_advanced(URIPARSER_LIBRARY URIPARSER_INCLUDE_DIR)
