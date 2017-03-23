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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_DVIWARE_FOLDER}/dvips")

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -DKPATHSEA
  -DMIKTEX
  -DSHIFTLOWCHARS
)

if(MIKTEX_NATIV_WINDOWS)
  add_definitions(
    -D_UNICODE
    -DUNICODE
  )
endif()

ignore_warnings()

set(dvips_c_sources
  source/bbox.c
  source/color.c
  source/dopage.c
  source/dosection.c
  source/dospecial.c
  source/download.c
  source/dpicheck.c
  source/drawPS.c
  source/dviinput.c
  source/dvips.c
  source/emspecial.c
  source/finclude.c
  source/fontdef.c
  source/header.c
  source/hps.c
  source/loadfont.c
  source/makefont.c
  source/output.c
  source/papersiz.c
  source/pprescan.c
  source/prescan.c
  source/repack.c
  source/resident.c
  source/scalewidth.c
  source/scanpage.c
  source/search.c
  source/skippage.c
  source/t1part.c
  source/tfmload.c
  source/unpack.c
  source/virtualfont.c
  source/writet1.c
)

set(dvips_sources
  ${dvips_c_sources}
  ${MIKTEX_LIBRARY_WRAPPER}
  c-auto.h
  dvips-version.h
  source/config.h
  source/debug.h
  source/dvips.h
  source/paths.h
  source/protos.h
  source/protos_add.h
  source/ptexmac.h
  source/t1part.h
)

set_source_files_properties(${MIKTEX_LIBRARY_WRAPPER}
  PROPERTIES
    COMPILE_FLAGS "-DCPLUSPLUSMAIN -DBEQUIET"
)

set_source_files_properties(${dvips_c_sources}
  PROPERTIES
    LANGUAGE CXX
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND dvips_sources
    dvips.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}dvips ${dvips_sources})

set_property(TARGET ${MIKTEX_PREFIX}dvips PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(${MIKTEX_PREFIX}dvips asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}dvips)

target_link_libraries(${MIKTEX_PREFIX}dvips
  PRIVATE
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}dvips
    PRIVATE
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}dvips DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(afm2tfm_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  afm2tfm-version.h
  source/afm2tfm.c
  source/config.h
  source/dvips.h
)

set_source_files_properties(source/afm2tfm.c
  PROPERTIES
    LANGUAGE CXX
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND afm2tfm_sources
    afm2tfm.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}afm2tfm ${afm2tfm_sources})

set_property(TARGET ${MIKTEX_PREFIX}afm2tfm PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(${MIKTEX_PREFIX}afm2tfm asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}afm2tfm)

target_link_libraries(${MIKTEX_PREFIX}afm2tfm
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${texmf_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}afm2tfm
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}afm2tfm DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
