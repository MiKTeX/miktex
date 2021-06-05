## miktex-version.cmake: define the MiKTeX version number
##
## Copyright (C) 2006-2021 Christian Schenk
## 
## This file is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
## 
## This file is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this file; if not, write to the Free Software Foundation,
## 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

## date based version
set(MIKTEX_YEAR_VERSION 2021)
set(MIKTEX_MONTH_VERSION 6)
set(MIKTEX_DAY_VERSION 7)

set(MIKTEX_DAY_VERSION_IS_PATCH_VERSION FALSE)

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
