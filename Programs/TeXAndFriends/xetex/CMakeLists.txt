## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2008-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_TEX_AND_FRIENDS_FOLDER}/XeTeX")

default_char_type_is_unsigned()

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)

include_directories(
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_INCLUDE_DIR}
)
  
add_definitions(
  -DHAVE_C4P_PRE_H
  -DLE_USE_CMEMORY
  -DSYNCTEX_ENGINE_H="synctex-xetex.h"
  -DXETEX_OTHER
  -DXeTeX
  -D__SyncTeX__
)

ignore_warnings()

set(C4P_FLAGS
  --auto-exit=10
  --chars-are-unsigned
  --declare-c-type=realpoint
  --declare-c-type=realrect
  --declare-c-type=transform
  --declare-c-type=unicodefile
  --declare-c-type=voidpointer
  --emit-optimize-pragmas
)

set(xetex_ch_synctex
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-xe-def.ch0
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-mem.ch0
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-e-mem.ch0
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-e-mem.ch1
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-rec.ch0
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-e-rec.ch0
)

set(xetex_post_ch_synctex
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-xe-rec.ch3
)

set(miktex_tex_change_files
  ${MLTEX_MIKTEX_CH}
  ${TEX_MIKTEX_CH}
  ${TEX_MIKTEX_HASH_CH}
  ${TEX_MIKTEX_HYPH_CH}
  ${TEX_MIKTEX_QUIET_CH}
  ${TEX_MIKTEX_SRC_CH}
  ${TEX_MIKTEX_SYNCTEX_CH}
  ${TEX_MIKTEX_STAT_CH}
  ${TEX_MIKTEX_WRITE18_CH}
  ${TEX_MIKTEX_WEB2C_CH}
  ${TEX_MIKTEX_POOL_CH}
)

set(pre_xetex_change_files
  ${xetex_ch_synctex}
  ${ETEX_MIKTEX_PRE_CH}
  ${ETEX_MIKTEX_CH}
  ${CMAKE_CURRENT_SOURCE_DIR}/miktex-adapter-0.ch
  ${miktex_tex_change_files}
)

set(pre_xetex1_web_file ${CMAKE_CURRENT_BINARY_DIR}/pre-xetex1.web)

add_custom_command(
  OUTPUT
    ${pre_xetex1_web_file}
  COMMAND
    ${MIKTEX_PREFIX}tie
    -m ${pre_xetex1_web_file}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/xetex.web
    ${pre_xetex_change_files}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/xetex.web
  DEPENDS
    ${MIKTEX_PREFIX}tie
    ${pre_xetex_change_files}
  VERBATIM
)

set(pre_xetex2_web_file ${CMAKE_CURRENT_BINARY_DIR}/pre-xetex2.web)

add_custom_command(
  OUTPUT
    ${pre_xetex2_web_file}
  COMMAND
    ${MIKTEX_PREFIX}tie
    -m ${pre_xetex2_web_file}
    ${pre_xetex1_web_file}
    ${CMAKE_CURRENT_SOURCE_DIR}/miktex-adapter-1.ch
    ${CMAKE_CURRENT_SOURCE_DIR}/miktex-adapter-2.ch
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    ${MIKTEX_PREFIX}tie
    ${pre_xetex1_web_file}
    ${CMAKE_CURRENT_SOURCE_DIR}/miktex-adapter-1.ch
    ${CMAKE_CURRENT_SOURCE_DIR}/miktex-adapter-2.ch
  VERBATIM
)

set(xetex_web_file ${CMAKE_CURRENT_BINARY_DIR}/xetex.web)
set(xetex_change_file ${CMAKE_CURRENT_SOURCE_DIR}/xetex-miktex.ch)

set(xetex_header_file ${CMAKE_CURRENT_BINARY_DIR}/xetexd.h)

add_custom_command(
  OUTPUT
    ${xetex_web_file}
  COMMAND
    ${MIKTEX_PREFIX}tie
    -m ${xetex_web_file}
    ${pre_xetex2_web_file}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/xetex.ch
    ${xetex_post_ch_synctex}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    ${MIKTEX_PREFIX}tie
    ${pre_xetex2_web_file}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/xetex.ch
    ${xetex_post_ch_synctex}
  VERBATIM
)

file(READ ${MIKTEX_DYN_TEX_SCRIPT} tex_dyn_sed_contents)
file(READ ${MIKTEX_DYN_ETEX_SCRIPT} etex_dyn_sed_contents)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed "${tex_dyn_sed_contents}")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed "${etex_dyn_sed_contents}")

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex.c
  source/XeTeX_ext.c
  source/XeTeX_pic.c
  source/image/bmpimage.c
  source/image/jpegimage.c
  source/image/mfileio.c
  source/image/numbers.c
  source/image/pngimage.c
  source/trans.c
  PROPERTIES LANGUAGE CXX
)

set(xetex_target_name xetex)

set(${xetex_target_name}_sources
  ${CMAKE_CURRENT_BINARY_DIR}/xetex_pool.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-common.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-xetex.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex.c
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex.h
  c4p_pre.h
  miktex-first.h
  source/XeTeXFontInst.cpp
  source/XeTeXFontInst.h
  source/XeTeXFontMgr.cpp
  source/XeTeXFontMgr.h
  source/XeTeXFontMgr_FC.cpp
  source/XeTeXFontMgr_FC.h
  source/XeTeXLayoutInterface.cpp
  source/XeTeXLayoutInterface.h
  source/XeTeXOTMath.cpp
  source/XeTeXOTMath.h
  source/XeTeX_ext.c
  source/XeTeX_ext.h
  source/XeTeX_pic.c
  source/XeTeX_web.h
  source/XeTeXswap.h
  source/etex_version.h
  source/hz.cpp
  source/image/bmpimage.c
  source/image/jpegimage.c
  source/image/mfileio.c
  source/image/numbers.c
  source/image/pngimage.c
  source/pdfimage.cpp
  source/pdfimage.h
  source/trans.c
  source/trans.h
  source/xetex.h
  source/xetex_version.h
  xetex-miktex.cpp
  xetex-miktex.h
  xetex-version.h
)

set_source_files_properties(
  ${${xetex_target_name}_sources}
  PROPERTIES COMPILE_FLAGS "-DC4PEXTERN=extern"
)

set(TANGLE_FLAGS -omega)

create_web_app(XeTeX)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/xetex_pool.cpp
  COMMAND
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/xetex.pool
    xetex-miktex.h
    ${xetex_progclass}
    ${xetex_prog}
    > ${CMAKE_CURRENT_BINARY_DIR}/xetex_pool.cpp
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/xetex.pool
)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/xetex.cc
  PROPERTIES COMPILE_FLAGS "-DCOMPILING_XETEX_CC"
)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/xetex.cc
  PROPERTIES COMPILE_FLAGS "-DCOMPILING_XETEX_CC"
)

if(USE_SYSTEM_FONTCONFIG)
  target_link_libraries(${xetex_target_name} PRIVATE MiKTeX::Imported::FONTCONFIG)
else()
  target_link_libraries(${xetex_target_name} PRIVATE ${fontconfig_dll_name})
endif()

if(USE_SYSTEM_FREETYPE2)
  target_link_libraries(${xetex_target_name} PRIVATE MiKTeX::Imported::FREETYPE2)
else()
  target_link_libraries(${xetex_target_name} PRIVATE ${freetype2_dll_name})
endif()

if(USE_SYSTEM_GRAPHITE2)
  target_link_libraries(${xetex_target_name} PRIVATE MiKTeX::Imported::GRAPHITE2)
else()
  target_link_libraries(${xetex_target_name} PRIVATE ${graphite2_dll_name})
endif()

if(USE_SYSTEM_HARFBUZZ_ICU)
  target_link_libraries(${xetex_target_name} PRIVATE MiKTeX::Imported::HARFBUZZ_ICU)
  target_link_libraries(${xetex_target_name} PRIVATE MiKTeX::Imported::HARFBUZZ)
else()
  target_link_libraries(${xetex_target_name} PRIVATE ${harfbuzz_lib_name})
endif()

if(USE_SYSTEM_ICU)
  target_link_libraries(${xetex_target_name} PRIVATE ICU::i18n)
  target_link_libraries(${xetex_target_name} PRIVATE ICU::uc)
else()
  # not an option
endif()

if(USE_SYSTEM_JPEG)
  target_link_libraries(${xetex_target_name} PRIVATE MiKTeX::Imported::JPEG)
else()
  target_link_libraries(${xetex_target_name} PRIVATE ${jpeg_dll_name})
endif()

if(USE_SYSTEM_PNG)
  target_link_libraries(${xetex_target_name} PRIVATE MiKTeX::Imported::PNG)
else()
  target_link_libraries(${xetex_target_name} PRIVATE ${png_dll_name})
endif()

if(USE_SYSTEM_POPPLER)
  target_link_libraries(${xetex_target_name} PRIVATE MiKTeX::Imported::POPPLER)
else()
  target_link_libraries(${xetex_target_name} PRIVATE ${poppler_lib_name})
endif()

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${xetex_target_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${xetex_target_name} PRIVATE ${zlib_dll_name})
endif()

target_link_libraries(${xetex_target_name}
  PRIVATE
    ${kpsemu_dll_name}
    ${teckit_dll_name}
    ${w2cemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${xetex_target_name}
    PRIVATE
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()

delay_load(${xetex_target_name}
  ${fontconfig_dll_name}
  ${freetype2_dll_name}
  ${graphite2_dll_name}
  ${jpeg_dll_name}
#  ${kpsemu_dll_name}
  ${png_dll_name}
  ${teckit_dll_name}
  ${unxemu_dll_name}
  ${w2cemu_dll_name}
)

## dev targets

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/xetex-n.web
  COMMAND
    web-n
    < ${CMAKE_CURRENT_SOURCE_DIR}/source/xetex.web
    > ${CMAKE_CURRENT_BINARY_DIR}/xetex-n.web
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    web-n
    ${CMAKE_CURRENT_SOURCE_DIR}/source/xetex.web
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/pre-xetex1-n.web
  COMMAND
    web-n
    < ${pre_xetex1_web_file}
    > ${CMAKE_CURRENT_BINARY_DIR}/pre-xetex1-n.web
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    web-n
    ${pre_xetex1_web_file}
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/pre-xetex2-n.web
  COMMAND
    web-n
    < ${pre_xetex2_web_file}
    > ${CMAKE_CURRENT_BINARY_DIR}/pre-xetex2-n.web
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    web-n
    ${pre_xetex2_web_file}
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/xetex-final-n.web
  COMMAND
    web-n
    < ${xetex_web_file}
    > ${CMAKE_CURRENT_BINARY_DIR}/xetex-final-n.web
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    web-n
    ${xetex_web_file}
  VERBATIM
)

add_custom_target(xetex-dev ALL
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/xetex-n.web
    ${CMAKE_CURRENT_BINARY_DIR}/pre-xetex1-n.web
    ${CMAKE_CURRENT_BINARY_DIR}/pre-xetex2-n.web
    ${CMAKE_CURRENT_BINARY_DIR}/xetex-final-n.web
)

set_property(TARGET xetex-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
