## libluatex.cmake
##
## Copyright (C) 2010-2019 Christian Schenk
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

list(APPEND libluatex_common_sources
  source/lua/lstrlibext.c
  source/lua/helpers.c
  source/lua/texluac.c 
)

list(APPEND libluatex_common_sources
  source/luatex-common.h
  source/luatex.h
  source/luatexcallbackids.h
  source/ptexlib.h
)

list(APPEND libluatex_common_sources
  source/luafontloader/src/ffdummies.c
  source/luafontloader/src/ffdummies.h
  source/luafontloader/src/luafflib.c
)

list(APPEND libluatex_common_sources
  source/dvi/dvigen.c
  source/dvi/dvigen.h
)

list(APPEND libluatex_common_sources
  source/font/dofont.c
  source/font/luafont.c
  source/font/luatexfont.h
  source/font/mapfile.c
  source/font/mapfile.h
  source/font/pkin.c
  source/font/sfnt.c
  source/font/sfnt.h
  source/font/texfont.c
  source/font/texfont.h
  source/font/tfmofm.c
  source/font/tounicode.c
  source/font/tt_glyf.c
  source/font/tt_glyf.h
  source/font/tt_table.c
  source/font/tt_table.h
  source/font/vfovf.c
  source/font/vfpacket.c
  source/font/writecff.c
  source/font/writecff.h
  source/font/writeenc.c
  source/font/writefont.c
  source/font/writet1.c
  source/font/writet3.c
  source/font/writettf.c
  source/font/writettf.h
  source/font/writetype0.c
  source/font/writetype2.c 
)

list(APPEND libluatex_common_sources
  source/image/epdf.h
  source/image/image.h
  source/image/pdftoepdf.c
  source/image/pdftoepdf.h
  source/image/writeimg.c
  source/image/writeimg.h
  source/image/writejbig2.c
  source/image/writejbig2.h
  source/image/writejp2.c
  source/image/writejp2.h
  source/image/writejpg.c
  source/image/writejpg.h
  source/image/writepng.c
  source/image/writepng.h
)

list(APPEND libluatex_common_sources
  source/lang/hnjalloc.c
  source/lang/hnjalloc.h
  source/lang/hyphen.c
  source/lang/hyphen.h
  source/lang/texlang.c
  source/lang/texlang.h
)

list(APPEND libluatex_common_sources
  source/lua/helpers.c
  source/lua/lcallbacklib.c
  source/lua/lfontlib.c
  source/lua/limglib.c
  source/lua/liolibext.c
  source/lua/lkpselib.c
  source/lua/llanglib.c
  source/lua/llualib.c
  source/lua/lnewtokenlib.c
  source/lua/lnodelib.c
  source/lua/loslibext.c
  source/lua/lpdfelib.c
  source/lua/lpdflib.c
  source/lua/lpdfscannerlib.c
  source/lua/lstatslib.c
  source/lua/ltexiolib.c
  source/lua/ltexlib.c
  source/lua/luanode.c
  source/lua/luatex-core.c
  source/lua/luatoken.c
  source/lua/mplibstuff.c 
)

list(APPEND libluatex_common_sources
  source/pdf/pdfaction.c
  source/pdf/pdfaction.h
  source/pdf/pdfannot.c
  source/pdf/pdfannot.h
  source/pdf/pdfcolorstack.c
  source/pdf/pdfcolorstack.h
  source/pdf/pdfdest.c
  source/pdf/pdfdest.h
  source/pdf/pdffont.c
  source/pdf/pdffont.h
  source/pdf/pdfgen.c
  source/pdf/pdfgen.h
  source/pdf/pdfglyph.c
  source/pdf/pdfglyph.h
  source/pdf/pdfimage.c
  source/pdf/pdfimage.h
  source/pdf/pdflink.c
  source/pdf/pdflink.h
  source/pdf/pdflistout.c
  source/pdf/pdflistout.h
  source/pdf/pdfliteral.c
  source/pdf/pdfliteral.h
  source/pdf/pdfobj.c
  source/pdf/pdfobj.h
  source/pdf/pdfoutline.c
  source/pdf/pdfoutline.h
  source/pdf/pdfpage.c
  source/pdf/pdfpage.h
  source/pdf/pdfpagetree.c
  source/pdf/pdfpagetree.h
  source/pdf/pdfrule.c
  source/pdf/pdfrule.h
  source/pdf/pdfsaverestore.c
  source/pdf/pdfsaverestore.h
  source/pdf/pdfsetmatrix.c
  source/pdf/pdfsetmatrix.h
  source/pdf/pdfshipout.c
  source/pdf/pdfshipout.h
  source/pdf/pdftables.c
  source/pdf/pdftables.h
  source/pdf/pdfthread.c
  source/pdf/pdfthread.h
  source/pdf/pdftypes.h
  source/pdf/pdfxform.c 
  source/pdf/pdfxform.h
)

list(APPEND libluatex_common_sources
  source/tex/align.c
  source/tex/align.h
  source/tex/arithmetic.c
  source/tex/arithmetic.h
  source/tex/backend.c
  source/tex/backend.h
  source/tex/buildpage.c
  source/tex/buildpage.h
  source/tex/commands.c
  source/tex/commands.h
  source/tex/conditional.c
  source/tex/conditional.h
  source/tex/directions.c
  source/tex/directions.h
  source/tex/dumpdata.c
  source/tex/dumpdata.h
  source/tex/equivalents.c
  source/tex/equivalents.h
  source/tex/errors.c
  source/tex/errors.h
  source/tex/expand.c
  source/tex/expand.h
  source/tex/extensions.c
  source/tex/extensions.h
  source/tex/filename.c
  source/tex/filename.h
  source/tex/inputstack.c
  source/tex/inputstack.h
  source/tex/linebreak.c
  source/tex/linebreak.h
  source/tex/mainbody.c
  source/tex/mainbody.h
  source/tex/maincontrol.c
  source/tex/maincontrol.h
  source/tex/mathcodes.c
  source/tex/mathcodes.h
  source/tex/memoryword.c
  source/tex/memoryword.h
  source/tex/mlist.c
  source/tex/mlist.h
  source/tex/nesting.c
  source/tex/nesting.h
  source/tex/packaging.c
  source/tex/packaging.h
  source/tex/postlinebreak.c
  source/tex/postlinebreak.h
  source/tex/primitive.c
  source/tex/primitive.h
  source/tex/scanning.c
  source/tex/scanning.h
  source/tex/stringpool.c
  source/tex/stringpool.h
  source/tex/texdeffont.c
  source/tex/texdeffont.h
  source/tex/texfileio.c
  source/tex/texfileio.h
  source/tex/texmath.c
  source/tex/texmath.h
  source/tex/texnodes.c
  source/tex/texnodes.h
  source/tex/textcodes.c
  source/tex/textcodes.h
  source/tex/textoken.c 
  source/tex/textoken.h
)

list(APPEND libluatex_common_sources
  source/utils/avl.c
  source/utils/avl.h
  source/utils/avlstuff.c
  source/utils/avlstuff.h
  source/utils/managed-sa.c
  source/utils/managed-sa.h
  source/utils/unistring.c
  source/utils/unistring.h
)

list(APPEND libluatex_common_sources
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-common.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-luatex.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex.c
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex.h
)

list(APPEND libluatex_luatex_sources
  source/lua/luainit.c
  source/lua/luastuff.c
  source/lua/luatex-api.h
  source/tex/printing.c
  source/tex/printing.h
  source/utils/utils.c
  source/utils/utils.h
)

add_library(luatex-lua53tex-objects OBJECT ${libluatex_common_sources} ${libluatex_luatex_sources})

set_property(TARGET luatex-lua53tex-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(luatex-lua53tex-objects
  PRIVATE
    source/luafontloader/fontforge/fontforge
    source/luafontloader/fontforge/inc
    source/utils
)

if(USE_SYSTEM_PNG)
  target_link_libraries(luatex-lua53tex-objects PUBLIC MiKTeX::Imported::PNG)
else()
  target_link_libraries(luatex-lua53tex-objects PUBLIC ${png_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
  target_link_libraries(luatex-lua53tex-objects PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(luatex-lua53tex-objects PUBLIC ${zlib_dll_name})
endif()

target_link_libraries(luatex-lua53tex-objects
  PUBLIC
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${lua53_target_name}
    ${metapost_dll_name}
    ${w2cemu_dll_name}
    luatex-lua53fontforge-objects
    luatex-lua53misc-objects
    luatex-lua53pplib-objects
)

if(WITH_LUA54TEX)
  add_library(luatex-lua54tex-objects OBJECT ${libluatex_common_sources} ${libluatex_luatex_sources})
  set_property(TARGET luatex-lua54tex-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_include_directories(luatex-lua54tex-objects
    PRIVATE
      source/luafontloader/fontforge/fontforge
      source/luafontloader/fontforge/inc
  )
  if(USE_SYSTEM_PNG)
    target_link_libraries(luatex-lua54tex-objects PUBLIC MiKTeX::Imported::PNG)
  else()
    target_link_libraries(luatex-lua54tex-objects PUBLIC ${png_dll_name})
  endif()
  if(USE_SYSTEM_ZLIB)
    target_link_libraries(luatex-lua54tex-objects PUBLIC MiKTeX::Imported::ZLIB)
  else()
    target_link_libraries(luatex-lua54tex-objects PUBLIC ${zlib_dll_name})
  endif()
  target_link_libraries(luatex-lua54tex-objects
    PUBLIC
      ${core_dll_name}
      ${kpsemu_dll_name}
      ${lua54_target_name}
      ${metapost_dll_name}
      ${w2cemu_dll_name}
      luatex-lua54fontforge-objects
      luatex-lua54misc-objects
      luatex-lua54pplib-objects
  )
endif()

###############################################################################
## luahbtex-lua53tex-objects
###############################################################################

configure_file(
  source/lua/luainit.c
  ${CMAKE_CURRENT_BINARY_DIR}/luainit-hb.c
  COPYONLY
)

configure_file(
  source/lua/luastuff.c
  ${CMAKE_CURRENT_BINARY_DIR}/luastuff-hb.c
  COPYONLY
)

configure_file(
  source/lua/luatex-api.h
  ${CMAKE_CURRENT_BINARY_DIR}/luatex-api-hb.h
  COPYONLY
)

configure_file(
  source/tex/printing.c
  ${CMAKE_CURRENT_BINARY_DIR}/printing-hb.c
  COPYONLY
)

configure_file(
  source/utils/utils.c
  ${CMAKE_CURRENT_BINARY_DIR}/utils-hb.c
  COPYONLY
)

list(APPEND libluatex_luahbtex_sources
  ${CMAKE_CURRENT_BINARY_DIR}/luainit-hb.c
  ${CMAKE_CURRENT_BINARY_DIR}/luastuff-hb.c
  ${CMAKE_CURRENT_BINARY_DIR}/luatex-api-hb.h
  ${CMAKE_CURRENT_BINARY_DIR}/printing-hb.c
  ${CMAKE_CURRENT_BINARY_DIR}/utils-hb.c
  source/tex/printing.h
  source/utils/utils.h
)

add_library(luahbtex-lua53tex-objects OBJECT ${libluatex_common_sources} ${libluatex_luahbtex_sources})

set_property(TARGET luahbtex-lua53tex-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(luahbtex-lua53tex-objects
  PRIVATE
    -DLUATEX_HARFBUZZ_ENABLED
)

target_include_directories(luahbtex-lua53tex-objects
  PRIVATE
    source/luafontloader/fontforge/fontforge
    source/luafontloader/fontforge/inc
    source/utils
)

if(USE_SYSTEM_PNG)
  target_link_libraries(luahbtex-lua53tex-objects PUBLIC MiKTeX::Imported::PNG)
else()
  target_link_libraries(luahbtex-lua53tex-objects PUBLIC ${png_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
  target_link_libraries(luahbtex-lua53tex-objects PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(luahbtex-lua53tex-objects PUBLIC ${zlib_dll_name})
endif()

if(USE_SYSTEM_HARFBUZZ_ICU)
  target_link_libraries(luahbtex-lua53tex-objects PUBLIC MiKTeX::Imported::HARFBUZZ_ICU)
  target_link_libraries(luahbtex-lua53tex-objects PUBLIC MiKTeX::Imported::HARFBUZZ)
else()
  target_link_libraries(luahbtex-lua53tex-objects PUBLIC ${harfbuzz_dll_name})
endif()

target_link_libraries(luahbtex-lua53tex-objects
  PUBLIC
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${lua53_target_name}
    ${metapost_dll_name}
    ${w2cemu_dll_name}
    luatex-lua53fontforge-objects
    luatex-lua53misc-objects
    luatex-lua53pplib-objects
)

if(WITH_LUA54TEX)
  add_library(luahbtex-lua54tex-objects OBJECT ${libluatex_common_sources} ${libluatex_luahbtex_sources})
  set_property(TARGET luahbtex-lua54tex-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_compile_definitions(luahbtex-lua54tex-objects
    PRIVATE
      -DLUATEX_HARFBUZZ_ENABLED
  )
  target_include_directories(luahbtex-lua54tex-objects
    PRIVATE
      source/luafontloader/fontforge/fontforge
      source/luafontloader/fontforge/inc
  )
  if(USE_SYSTEM_PNG)
    target_link_libraries(luahbtex-lua54tex-objects PUBLIC MiKTeX::Imported::PNG)
  else()
    target_link_libraries(luahbtex-lua54tex-objects PUBLIC ${png_dll_name})
  endif()
  if(USE_SYSTEM_ZLIB)
    target_link_libraries(luahbtex-lua54tex-objects PUBLIC MiKTeX::Imported::ZLIB)
  else()
    target_link_libraries(luahbtex-lua54tex-objects PUBLIC ${zlib_dll_name})
  endif()
  target_link_libraries(luahbtex-lua54tex-objects
    PUBLIC
      ${core_dll_name}
      ${kpsemu_dll_name}
      ${lua54_target_name}
      ${metapost_dll_name}
      ${w2cemu_dll_name}
      luatex-lua54fontforge-objects
      luatex-lua54misc-objects
      luatex-lua54pplib-objects
  )
endif()
