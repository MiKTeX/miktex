## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2013-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/ff-woff")

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge
  ${CMAKE_CURRENT_SOURCE_DIR}/source/inc
)

set(ff_woff_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/PfEd.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/configure-fontforge.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/edgelist.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/edgelist2.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/encoding.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/fontforge-config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/fontforge.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/fontforgevw.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/libffstamp.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/namehash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/psfont.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/sd.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/sfd1.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/splinefont.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/stemdb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/tables.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/ttf.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/ttfinstrs.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/uiinterface.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/alphabet.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/asmfpst.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/autohint.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/char.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/cjk.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/cvundoes.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/dumppfa.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/encoding.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/fvfonts.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/gwwiconv.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/lookups.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/macbinary.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/macenc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/mathconstants.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/memory.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/mm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/namelist.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/nouiutil.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/nowakowskittfinstr.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/parsepfa.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/parsettf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/parsettfatt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/psread.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/pua.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/sfd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/sfd1.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/splinechar.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/splinefont.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/splineorder2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/splineoverlap.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/splinerefigure.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/splinesave.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/splinesaveafm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/splineutil.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/splineutil2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/start.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/stemdb.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/tottf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/tottfaat.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/tottfgpos.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/tottfvar.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/ttfinstrs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/ttfspecial.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/unialt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/ustring.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/utils.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/utype.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge/woff.c
)

add_library(${ff_woff_lib_name} STATIC ${ff_woff_sources})

set_property(TARGET ${ff_woff_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${ff_woff_lib_name}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fontforge
    ${CMAKE_CURRENT_SOURCE_DIR}/source/inc
)

target_link_libraries(${ff_woff_lib_name}
  PUBLIC
    ${zlib_dll_name}
)
