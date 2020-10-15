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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/xpdf")

set(projdir ${CMAKE_CURRENT_SOURCE_DIR})

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo
  ${CMAKE_CURRENT_SOURCE_DIR}/source/xpdf
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo
  ${CMAKE_CURRENT_SOURCE_DIR}/source/xpdf
)

configure_file(
  source/aconf.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/aconf.h
)

configure_file(
  xpdf-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/xpdf-version.h
)

set(xpdf_sources
  source/fofi/FoFiBase.cc
  source/fofi/FoFiEncodings.cc
  source/fofi/FoFiIdentifier.cc
  source/fofi/FoFiTrueType.cc
  source/fofi/FoFiType1.cc
  source/fofi/FoFiType1C.cc

  source/goo/FixedPoint.cc
  source/goo/GHash.cc
  source/goo/GList.cc
  source/goo/GString.cc
  source/goo/gfile.cc
  source/goo/gmem.cc
  source/goo/gmempp.cc

  source/xpdf/AcroForm.cc
  source/xpdf/Annot.cc
  source/xpdf/Array.cc
  source/xpdf/BuiltinFont.cc
  source/xpdf/BuiltinFontTables.cc
  source/xpdf/CMap.cc
  source/xpdf/Catalog.cc
  source/xpdf/CharCodeToUnicode.cc
  source/xpdf/Decrypt.cc
  source/xpdf/Dict.cc
  source/xpdf/Error.cc
  source/xpdf/FontEncodingTables.cc
  source/xpdf/Form.cc
  source/xpdf/Function.cc
  source/xpdf/Gfx.cc
  source/xpdf/GfxFont.cc
  source/xpdf/GfxState.cc
  source/xpdf/GlobalParams.cc
  source/xpdf/JArithmeticDecoder.cc
  source/xpdf/JBIG2Stream.cc
  source/xpdf/JPXStream.cc
  source/xpdf/Lexer.cc
  source/xpdf/Link.cc
  source/xpdf/NameToCharCode.cc
  source/xpdf/Object.cc
  source/xpdf/OptionalContent.cc
  source/xpdf/Outline.cc
  source/xpdf/OutputDev.cc
  source/xpdf/PDF417Barcode.cc
  source/xpdf/PDFDoc.cc
  source/xpdf/PDFDocEncoding.cc
  source/xpdf/PSTokenizer.cc
  source/xpdf/Page.cc
  source/xpdf/Parser.cc
  source/xpdf/SecurityHandler.cc
  source/xpdf/Stream.cc
  source/xpdf/TextString.cc
  source/xpdf/UnicodeMap.cc
  source/xpdf/UnicodeRemapping.cc
  source/xpdf/UTF8.cc
  source/xpdf/XFAForm.cc
  source/xpdf/XRef.cc
  source/xpdf/Zoox.cc
)

list(APPEND xpdf_sources
  source/fofi/FoFiBase.h
  source/fofi/FoFiEncodings.h
  source/fofi/FoFiIdentifier.h
  source/fofi/FoFiTrueType.h
  source/fofi/FoFiType1.h
  source/fofi/FoFiType1C.h
  source/goo/FixedPoint.h
  source/goo/GHash.h
  source/goo/GList.h
  source/goo/GMutex.h
  source/goo/GString.h
  source/goo/gfile.h
  source/goo/gmem.h
  source/goo/gmempp.h
  source/goo/gtypes.h
  source/goo/parseargs.h
  source/xpdf/AcroForm.h
  source/xpdf/Annot.h
  source/xpdf/Array.h
  source/xpdf/BuiltinFont.h
  source/xpdf/BuiltinFontTables.h
  source/xpdf/CMap.h
  source/xpdf/Catalog.h
  source/xpdf/CharCodeToUnicode.h
  source/xpdf/CharTypes.h
  source/xpdf/CompactFontTables.h
  source/xpdf/Decrypt.h
  source/xpdf/Dict.h
  source/xpdf/DisplayState.h
  source/xpdf/Error.h
  source/xpdf/ErrorCodes.h
  source/xpdf/FontEncodingTables.h
  source/xpdf/Form.h
  source/xpdf/Function.h
  source/xpdf/Gfx.h
  source/xpdf/GfxFont.h
  source/xpdf/GfxState.h
  source/xpdf/GlobalParams.h
  source/xpdf/HTMLGen.h
  source/xpdf/ImageOutputDev.h
  source/xpdf/JArithmeticDecoder.h
  source/xpdf/JBIG2Stream.h
  source/xpdf/JPXStream.h
  source/xpdf/Lexer.h
  source/xpdf/Link.h
  source/xpdf/NameToCharCode.h
  source/xpdf/NameToUnicodeTable.h
  source/xpdf/Object.h
  source/xpdf/OptionalContent.h
  source/xpdf/Outline.h
  source/xpdf/OutputDev.h
  source/xpdf/PDF417Barcode.h
  source/xpdf/PDFCore.h
  source/xpdf/PDFDoc.h
  source/xpdf/PDFDocEncoding.h
  source/xpdf/PSOutputDev.h
  source/xpdf/PSTokenizer.h
  source/xpdf/Page.h
  source/xpdf/Parser.h
  source/xpdf/PreScanOutputDev.h
  source/xpdf/SecurityHandler.h
  source/xpdf/SplashOutputDev.h
  source/xpdf/Stream-CCITT.h
  source/xpdf/Stream.h
  source/xpdf/TextOutputDev.h
  source/xpdf/TextString.h
  source/xpdf/TileCache.h
  source/xpdf/TileCompositor.h
  source/xpdf/TileMap.h
  source/xpdf/UTF8.h
  source/xpdf/UnicodeMap.h
  source/xpdf/UnicodeMapTables.h
  source/xpdf/UnicodeRemapping.h
  source/xpdf/UnicodeTypeTable.h
  source/xpdf/WebFont.h
  source/xpdf/XFAForm.h
  source/xpdf/XRef.h
  source/xpdf/Zoox.h
  source/xpdf/config.h
)

add_library(${xpdf_lib_name} STATIC ${xpdf_sources})

set_property(TARGET ${xpdf_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${xpdf_lib_name}
  PRIVATE
    -DPDF_PARSER_ONLY
)

target_include_directories(${xpdf_lib_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${xpdf_lib_name}
  PRIVATE
    ${core_dll_name}
    ${util_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${xpdf_lib_name}
    PRIVATE
      ${utf8wrap_dll_name}
  )
endif()
