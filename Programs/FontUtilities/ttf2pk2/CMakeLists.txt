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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_FONT_UTILITIES_FOLDER}/ttf2pk")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -DHAVE_LIBKPATHSEA
  -DHAVE_CONFIG_H
  -Dmain=Main
)

ignore_warnings()

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(common_sources
  source/errormsg.c
  source/errormsg.h
  source/filesrch.c
  source/filesrch.h
  source/ftlib.c
  source/ftlib.h
  source/ligkern.c
  source/ligkern.h
  source/newobj.c
  source/newobj.h
  source/parse.c
  source/parse.h
  source/subfont.c
  source/subfont.h
  source/texenc.c
  source/texenc.h
  source/ttfenc.c
  source/ttfenc.h
)

set(ttf2pk_sources
  ${common_sources}
  ${MIKTEX_LIBRARY_WRAPPER}
  source/pklib.c
  source/pklib.h
  source/ttf2pk.c
  source/ttflib.c
  source/ttflib.h
  ttf2pk-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND ttf2pk_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    ttf2pk.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}ttf2pk ${ttf2pk_sources})

set_property(TARGET ${MIKTEX_PREFIX}ttf2pk PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_FREETYPE2)
  target_link_libraries(${MIKTEX_PREFIX}ttf2pk MiKTeX::Imported::FREETYPE2)
else()
  target_link_libraries(${MIKTEX_PREFIX}ttf2pk ${freetype2_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${MIKTEX_PREFIX}ttf2pk MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${MIKTEX_PREFIX}ttf2pk ${zlib_dll_name})
endif()

target_link_libraries(${MIKTEX_PREFIX}ttf2pk
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}ttf2pk
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}ttf2pk DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(ttf2tfm_sources
  ${common_sources}
  ${MIKTEX_LIBRARY_WRAPPER}
  source/case.c
  source/case.h
  source/tfmaux.c
  source/tfmaux.h
  source/ttf2tfm.c
  source/ttf2tfm.h
  source/ttfaux.c
  source/ttfaux.h
  source/vplaux.c
  source/vplaux.h
  ttf2tfm-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND ttf2tfm_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    ttf2tfm.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}ttf2tfm ${ttf2tfm_sources})

set_property(TARGET ${MIKTEX_PREFIX}ttf2tfm PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_FREETYPE2)
  target_link_libraries(${MIKTEX_PREFIX}ttf2tfm MiKTeX::Imported::FREETYPE2)
else()
  target_link_libraries(${MIKTEX_PREFIX}ttf2tfm ${freetype2_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${MIKTEX_PREFIX}ttf2tfm MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${MIKTEX_PREFIX}ttf2tfm ${zlib_dll_name})
endif()

target_link_libraries(${MIKTEX_PREFIX}ttf2tfm
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}ttf2tfm
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}ttf2tfm DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
