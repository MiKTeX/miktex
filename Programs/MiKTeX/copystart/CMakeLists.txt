## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/copystart")

use_static_crt()

add_definitions(
  -DUNICODE
  -D_UNICODE
)

set(libs
  ${core_lib_name}
)

list(APPEND libs
  shlwapi
)

set(copystart_sources copystart.cpp)

list(APPEND copystart_sources
  ${MIKTEX_COMPATIBILITY_MANIFEST}
)

add_executable(copystart WIN32 ${copystart_sources})

set_property(TARGET copystart PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(copystart ${libs})

install(
  TARGETS copystart
  DESTINATION "${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}"
)

add_executable(copystart${MIKTEX_ADMIN_SUFFIX} WIN32 ${copystart_sources})

set_property(TARGET copystart${MIKTEX_ADMIN_SUFFIX} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_link_flags(copystart${MIKTEX_ADMIN_SUFFIX} "/MANIFESTUAC:level='requireAdministrator'")

target_link_libraries(copystart${MIKTEX_ADMIN_SUFFIX} ${libs})

install(
    TARGETS copystart${MIKTEX_ADMIN_SUFFIX}
    DESTINATION "${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}"
)
