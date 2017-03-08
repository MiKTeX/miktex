## CMakeLists.txt					-*- CMake -*-
##
## Copyright (C) 2010-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/zzip")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

add_definitions(
  -DZZIP_HAVE_STDINT_H=1
  -DZZIP_EXPORTS=1
)

ignore_warnings()

set(sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/dir.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/err.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/fetch.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/file.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/info.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/plugin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/stat.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/write.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/zip.c

  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/__debug.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/__dirent.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/__fnmatch.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/__hints.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/__mmap.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/_config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/_msvc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/autoconf.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/conf.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/fetch.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/file.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/format.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/fseeko.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/info.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/lib.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/memdisk.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/mmapped.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/plugin.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/stdint.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/write.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/zzip.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/zzip32.h

  ${CMAKE_CURRENT_SOURCE_DIR}/zzip-version.h
)

set(public_headers
  _msvc.h
  conf.h
  fetch.h
  file.h
  format.h
  fseeko.h
  info.h
  lib.h
  memdisk.h
  mmapped.h
  plugin.h
  stdint.h
  types.h
  write.h
  zzip.h
  zzip32.h
)

foreach(hdr ${public_headers})
  configure_file(
     ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/${hdr}
     ${CMAKE_CURRENT_BINARY_DIR}/include/zzip/${hdr}
     COPYONLY
  )
endforeach()

set(${zzip_dll_name}_sources ${sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    zzip.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/zzip.rc
  )
  set(${zzip_dll_name}_sources ${${zzip_dll_name}_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/_msvc.h
    ${CMAKE_CURRENT_BINARY_DIR}/zzip.rc
  )
endif()

add_library(${zzip_dll_name} SHARED ${${zzip_dll_name}_sources})

set_property(TARGET ${zzip_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${zzip_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_MAJOR_MINOR_STR}"
    SOVERSION "1"
)

target_include_directories(${zzip_dll_name}
  PUBLIC
    ${public_include_directories}
)

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${zzip_dll_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${zzip_dll_name} PRIVATE ${zlib_dll_name})
endif()

rebase(${zzip_dll_name})

install(TARGETS ${zzip_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
