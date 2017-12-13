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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_FONT_UTILITIES_FOLDER}/ps2pk")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(PACKAGE_VERSION "1.7")

configure_file(
  c-auto.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/c-auto.h
)

add_definitions(
  -DKPATHSEA
  -DUSE_MIKTEX_EXIT
  -Dmain=Main
)

ignore_warnings()

set(mag_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  source/basics.c
  source/basics.h
  source/mag.c
)

set(pfb2pfa_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  source/basics.c
  source/basics.h
  source/filenames.c
  source/filenames.h
  source/pfb2pfa.c
)

set(pk2bm_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  source/basics.c
  source/basics.h
  source/pk2bm.c
  source/pkin.c
  source/pkin.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND pk2bm_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    pk2bm.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}pk2bm ${pk2bm_sources})

set_property(TARGET ${MIKTEX_PREFIX}pk2bm PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}pk2bm
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}pk2bm
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}pk2bm DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(ps2pk_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  source/Xstuff.h
  source/arith.c
  source/arith.h
  source/basics.c
  source/basics.h
  source/blues.h
  source/bstring.c
  source/cluts.h
  source/curves.c
  source/curves.h
  source/digit.h
  source/encoding.c
  source/ffilest.h
  source/filenames.c
  source/filenames.h
  source/font.h
  source/fontfcn.c
  source/fontfcn.h
  source/fontfile.h
  source/fontmisc.h
  source/fonts.h
  source/fontstruct.h
  source/fontxlfd.h
  source/fsmasks.h
  source/hdigit.h
  source/hints.c
  source/hints.h
  source/lines.c
  source/lines.h
  source/objects.c
  source/objects.h
  source/paths.c
  source/paths.h
  source/pictures.h
  source/pkout.c
  source/pkout.h
  source/ps2pk.c
  source/psearch.h
  source/regions.c
  source/regions.h
  source/scanfont.c
  source/spaces.c
  source/spaces.h
  source/strokes.h
  source/t1funcs.c
  source/t1hdigit.h
  source/t1info.c
  source/t1intf.h
  source/t1io.c
  source/t1snap.c
  source/t1stdio.h
  source/token.c
  source/token.h
  source/tokst.h
  source/type1.c
  source/types.h
  source/util.c
  source/util.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND ps2pk_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    ps2pk.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}ps2pk ${ps2pk_sources})

set_property(TARGET ${MIKTEX_PREFIX}ps2pk PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}ps2pk
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}ps2pk
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}ps2pk DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
