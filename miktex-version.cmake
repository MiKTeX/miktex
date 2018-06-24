## miktex-version.cmake: define the MiKTeX version number
##
## Copyright (C) 2006-2018 Christian Schenk
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

## major version number
set(MIKTEX_MAJOR_VERSION        2)

## minor version number
set(MIKTEX_MINOR_VERSION        9)

## milestone; measured in: days since January 1, 2000
set(MIKTEX_J2000_VERSION        6730)

## the version of the binary package
set(MIKTEX_PACKAGE_REVISION
  1
  CACHE STRING
  "The MiKTeX package revision."
)

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
