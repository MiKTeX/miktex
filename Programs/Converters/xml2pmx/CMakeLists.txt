## CMakeLists.txt
##
## Copyright (C) 2021 Christian Schenk
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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_CONVERTERS_FOLDER}/xml2pmx")

set(LOG_PAGESIZE 12)
set(MAGIC "OBCX")
set(PACKAGE_BUGREPORT "rdt@cs.queensu.ca")
set(PAGESIZE  4096)
set(PRELOAD TRUE)
set(REVID "xml2pmx")
set(SEGMEM TRUE)
set(SIG "0x00030290")

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
  xml2pmx-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/xml2pmx-version.h
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(
  -Dmain=Main
  -DUSE_MIKTEX_EXIT=1
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

set(sources
  source/Args.c
  source/Builtin.c
  source/Files.c
  source/dynlink.c
  source/exec.h
  source/gc.c
  source/image.c
  source/interp.c
  source/keiko.h
  source/loader.c
  source/obcommon.h
  source/obx.h
  source/primtab.h
  source/primtab0.c
  source/support.c
  source/wrapper.c
  source/xmain.c
)

list(APPEND sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${CMAKE_CURRENT_BINARY_DIR}/xml2pmx-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    xml2pmx.rc.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/xml2pmx.rc
  )
  list(APPEND sources
    ${MIKTEX_COMMON_MANIFEST}
    ${CMAKE_CURRENT_BINARY_DIR}/xml2pmx.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}xml2pmx ${sources})

set_property(TARGET ${MIKTEX_PREFIX}xml2pmx PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}xml2pmx
  ${app_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}xml2pmx
    ${getopt_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}xml2pmx DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
