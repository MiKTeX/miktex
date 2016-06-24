## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2016 Christian Schenk
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
  -D_UNICODE
  -DUNICODE
)

set(libs
  ${core_lib_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND libs
    shlwapi
  )
endif()

add_executable(copystart WIN32 copystart.cpp)

set_property(TARGET copystart PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(copystart ${libs})

merge_trustinfo_manifest(copystart asInvoker)
merge_compatibility_manifest(copystart)

install(
  TARGETS copystart
  DESTINATION "${MIKTEX_INTERNAL_BINARY_DIR}"
)

add_executable(copystart${MIKTEX_ADMIN_SUFFIX} WIN32 copystart.cpp)

set_property(TARGET copystart${MIKTEX_ADMIN_SUFFIX} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(copystart${MIKTEX_ADMIN_SUFFIX} ${libs})

merge_trustinfo_manifest(copystart${MIKTEX_ADMIN_SUFFIX} requireAdministrator)
merge_compatibility_manifest(copystart${MIKTEX_ADMIN_SUFFIX})

install(
  TARGETS copystart${MIKTEX_ADMIN_SUFFIX}
  DESTINATION "${MIKTEX_INTERNAL_BINARY_DIR}"
)
