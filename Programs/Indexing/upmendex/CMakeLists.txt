## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2020 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_INDEXING_FOLDER}/upmendex")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
  c-auto.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/c-auto.h
)

configure_file(
  upmendex-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/upmendex-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    upmendex.rc.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/upmendix.rc
  )
endif()

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

set(upmendex_sources
  ${CMAKE_CURRENT_BINARY_DIR}/c-auto.h
  ${CMAKE_CURRENT_BINARY_DIR}/upmendex-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  source/convert.c
  source/exkana.h
  source/exvar.h
  source/fread.c
  source/fwrite.c
  source/kana.h
  source/kp.c
  source/kp.h
  source/ktable.h
  source/main.c
  source/mendex.h
  source/pageread.c
  source/qsort.c
  source/qsort.h
  source/sort.c
  source/styfile.c
  source/var.h
  source/version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND upmendex_sources
    ${MIKTEX_COMMON_MANIFEST}
    ${CMAKE_CURRENT_BINARY_DIR}/upmendex.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}upmendex ${upmendex_sources} ${headers})

set_property(TARGET ${MIKTEX_PREFIX}upmendex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}upmendex
  ${app_dll_name}
  ${kpsemu_dll_name}

)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}upmendex
    ${utf8wrap_dll_name}
  )
endif()

if(USE_SYSTEM_ICU)
  target_link_libraries(${MIKTEX_PREFIX}upmendex ICU::i18n)
  target_link_libraries(${MIKTEX_PREFIX}upmendex ICU::uc)
else()
  # not an option
endif()

install(TARGETS ${MIKTEX_PREFIX}upmendex DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

