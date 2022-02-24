## FindFONTCONFIG.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(FONTCONFIG_INCLUDE_DIR
    NAMES
        fontconfig/fontconfig.h
)

find_library(FONTCONFIG_LIBRARY
    NAMES
        fontconfig
)

find_package_handle_standard_args(FONTCONFIG DEFAULT_MSG FONTCONFIG_LIBRARY FONTCONFIG_INCLUDE_DIR)

if(FONTCONFIG_FOUND)
    set(FONTCONFIG_INCLUDE_DIRS ${FONTCONFIG_INCLUDE_DIR})
    set(FONTCONFIG_LIBRARIES ${FONTCONFIG_LIBRARY})
else()
    set(FONTCONFIG_INCLUDE_DIRS)
    set(FONTCONFIG_LIBRARIES)
endif()

mark_as_advanced(FONTCONFIG_LIBRARY FONTCONFIG_INCLUDE_DIR)
