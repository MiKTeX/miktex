## miktex-version.cmake
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

## date based version
set(MIKTEX_YEAR_VERSION 2022)
set(MIKTEX_MONTH_VERSION 7)
set(MIKTEX_DAY_VERSION 26)

set(MIKTEX_DAY_VERSION_IS_PATCH_VERSION TRUE)

## internal major/minor/patch version
math(EXPR
    MIKTEX_MAJOR_VERSION
    "${MIKTEX_YEAR_VERSION} - 2000"
)
set(MIKTEX_MINOR_VERSION ${MIKTEX_MONTH_VERSION})
if(MIKTEX_DAY_VERSION_IS_PATCH_VERSION)
    set(MIKTEX_PATCH_VERSION ${MIKTEX_DAY_VERSION})
else()
    set(MIKTEX_PATCH_VERSION 0)
endif()


## old (pre Jun 2020) version
set(MIKTEX_LEGACY_MAJOR_VERSION 2)
set(MIKTEX_LEGACY_MINOR_VERSION 9)

## major/minor version for package database files
set(MIKTEX_PACKAGES_MAJOR_VERSION 2)
set(MIKTEX_PACKAGES_MINOR_VERSION 9)

if(WIN32 AND NOT UNIX)
    ## major/minor version for Windows related COMponents files and directories
    set(MIKTEX_COM_MAJOR_VERSION 2)
    set(MIKTEX_COM_MINOR_VERSION 9)
endif()

## the version of the MiKTeX binary package
set(MIKTEX_PACKAGE_REVISION
    1
    CACHE STRING
    "The MiKTeX package revision."
)
