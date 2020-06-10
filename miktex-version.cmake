## miktex-version.cmake: define the MiKTeX version number
##
## Copyright (C) 2006-2020 Christian Schenk
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
set(MIKTEX_YEAR_VERSION 2020)
set(MIKTEX_MONTH_VERSION 6)
set(MIKTEX_DAY_VERSION 14)
set(MIKTEX_INTERNAL_J2000_VERSION 7470)

set(MIKTEX_DAY_VERSION_IS_PATCH_VERSION FALSE)

## internal major/minor/patch version
math(EXPR
  MIKTEX_INTERNAL_MAJOR_VERSION
  "${MIKTEX_YEAR_VERSION} - 2000"
)
set(MIKTEX_INTERNAL_MINOR_VERSION ${MIKTEX_MONTH_VERSION})
if(MIKTEX_DAY_VERSION_IS_PATCH_VERSION)
  set(MIKTEX_INTERNAL_PATCH_VERSION ${MIKTEX_DAY_VERSION})
else()
  set(MIKTEX_INTERNAL_PATCH_VERSION 0)
endif()

## date based release version number
math(EXPR
  MIKTEX_YYYYMMDD_VERSION
  "${MIKTEX_YEAR_VERSION} * 1000 + (${MIKTEX_MONTH_VERSION} * 100) + ${MIKTEX_DAY_VERSION}"
)
string(SUBSTRING ${MIKTEX_YYYYMMDD_VERSION} 0 4 yyyy)
string(SUBSTRING ${MIKTEX_YYYYMMDD_VERSION} 4 2 mm)
string(SUBSTRING ${MIKTEX_YYYYMMDD_VERSION} 6 4 dd)
set(MIKTEX_DATE_VERSION "${yyyy}-${mm}-${dd}")

## the version of the binary package
set(MIKTEX_PACKAGE_REVISION
  1
  CACHE STRING
  "The MiKTeX package revision."
)

## major/minor version for configuration files
set(MIKTEX_CONFIG_MAJOR_VERSION 2)
set(MIKTEX_CONFIG_MINOR_VERSION 9)

## major/minor version for package manager files
set(MIKTEX_PACKAGES_MAJOR_VERSION 2)
set(MIKTEX_PACKAGES_MINOR_VERSION 9)

if(MIKTEX_WINDOWS)
  ## major/minor version for Windows related COMponents files and directories
  set(MIKTEX_COM_MAJOR_VERSION 2)
  set(MIKTEX_COM_MINOR_VERSION 9)
endif()

if(MIKTEX_LINUX)
  set(MIKTEX_LINUX_DIST
    "ubuntu"
    CACHE STRING
    "The Linux distribution (ubuntu, debian, fedora, ...)."
  )
  set(MIKTEX_LINUX_DIST_VERSION
    "16.04"
    CACHE STRING
    "The Linux distribution version (16.04, 9, 28, ...)."
  )
  set(MIKTEX_LINUX_DIST_CODE_NAME
    "xenial"
    CACHE STRING
    "The Linux distribution code name (xenial, stretch, ...)."
  )
endif()

## cached date
set(MIKTEX_YEAR_VERSION            ${MIKTEX_CACHED_YEAR})

## the release number; only meaningful for rc/beta builds
set(MIKTEX_RELEASE_NUM          1)

## the release state; one of:
##   0 (stable)
##   1 (next)
##   4 (debug)
set(MIKTEX_RELEASE_STATE
  0
  CACHE STRING
  "The MiKTeX release state."
)

## sanity check
if(MIKTEX_RELEASE_STATE EQUAL 0)
elseif(MIKTEX_RELEASE_STATE EQUAL 1)
elseif(MIKTEX_RELEASE_STATE EQUAL 4)
else()
  message("${MIKTEX_RELEASE_STATE} is an unknown release state.")
  message("Falling back to release state 4 (debug).")
  set(MIKTEX_RELEASE_STATE
      4
      CACHE STRING
      "MiKTeX release state; one of 0 (stable), 1 (next), 4 (debug)."
      FORCE)
endif()
