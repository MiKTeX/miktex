## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2008-2021 Christian Schenk
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
  -DHAVE_C4P_POST_H
  -DHAVE_C4P_PRE_H
  -DLE_USE_CMEMORY
  -DSYNCTEX_ENGINE_H="synctex-xetex.h"
  -DTeX
  -DXETEX_OTHER
  -DXeTeX
  -D__SyncTeX__
)

set(C4P_FLAGS
  --auto-exit=10
  --base-class=XeTeXProgramBase
  --chars-are-unsigned
  --declare-c-type=realpoint
  --declare-c-type=realrect
  --declare-c-type=transform
  --declare-c-type=unicodefile
  --declare-c-type=voidpointer
  --emit-optimize-pragmas
)

include(webify.cmake)

set(xetex_web_file ${CMAKE_CURRENT_BINARY_DIR}/xetex-final.web)
set(xetex_header_file ${CMAKE_CURRENT_BINARY_DIR}/xetexd.h)

file(READ ${MIKTEX_DYN_ETEX_SCRIPT} etex_dyn_sed_contents)
file(READ ${MIKTEX_DYN_TEX_SCRIPT} tex_dyn_sed_contents)

file(WRITE  ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed "${etex_dyn_sed_contents}")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed "${tex_dyn_sed_contents}")

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex.c
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
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-common.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-xetex.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex.c
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex.h
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
  source/xetexextra.c
  source/xetexextra.h
  miktex-xetex-version.h
  miktex-xetex.cpp
  miktex-xetex.h
)

set_source_files_properties(source/xetexextra.c PROPERTIES LANGUAGE CXX)

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
    miktex-xetex.h
    ${xetex_progclass}
    ${xetex_prog}
    > ${CMAKE_CURRENT_BINARY_DIR}/xetex_pool.cpp
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/xetex.pool
    inipool
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
  target_link_libraries(${xetex_target_name} PRIVATE ${harfbuzz_dll_name})
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

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${xetex_target_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${xetex_target_name} PRIVATE ${zlib_dll_name})
endif()

target_link_libraries(${xetex_target_name}
  PRIVATE
    ${kpsemu_dll_name}
    ${pplib_lib_name}
    ${teckit_dll_name}
    ${w2cemu_dll_name}
    ${web2c_sources_lib_name}
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
