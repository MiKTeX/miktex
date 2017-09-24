## FindPOPPLER.cmake
##
## Copyright (C) 2016 Christian Schenk
## 
## This file is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published
## by the Free Software Foundation; either version 2, or (at your
## option) any later version.
## 
## This file is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this file; if not, write to the Free Software
## Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
## USA.

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
