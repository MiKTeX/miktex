## CMakeLists.txt					-*- CMake -*-
##
## Copyright (C) 2009-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/poppler")

set(projdir ${CMAKE_CURRENT_SOURCE_DIR})
  
set(POPPLER_MAJOR_VERSION "0")
set(POPPLER_MINOR_VERSION "42")
set(POPPLER_MICRO_VERSION "0")
set(POPPLER_VERSION "${POPPLER_MAJOR_VERSION}.${POPPLER_MINOR_VERSION}.${POPPLER_MICRO_VERSION}")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo
)

set(ENABLE_LIBCURL TRUE)
set(ENABLE_LIBJPEG TRUE)
set(ENABLE_SPLASH TRUE)
set(ENABLE_ZLIB TRUE)
set(ENABLE_ZLIB_UNCOMPRESS TRUE)

if(MIKTEX_NATIVE_WINDOWS)
  set(WITH_FONTCONFIGURATION_WIN32 TRUE)
  set(WITH_FONTCONFIGURATION_FONTCONFIG FALSE)
else()
  set(WITH_FONTCONFIGURATION_WIN32 FALSE)
  set(WITH_FONTCONFIGURATION_FONTCONFIG FALSE)
endif()

set(MULTITHREADED TRUE)
set(OPI_SUPPORT TRUE)
set(TEXTOUT_WORD_LIST TRUE)

set(HAVE_LIBZ TRUE)
set(HAVE_SPLASH TRUE)
set(HAVE_FREETYPE_H TRUE)

add_definitions(-DHAVE_CONFIG_H=1)

ignore_warnings()

configure_file(
  source/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
  source/poppler/poppler-config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/poppler/poppler-config.h
)

set(${poppler_lib_name}_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gfile.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gmempp.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooHash.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooList.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooTimer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooString.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gmem.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/FixedPoint.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/NetPBMWriter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/PNGWriter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/TiffWriter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/JpegWriter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/ImgWriter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gstrtod.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/grandom.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiBase.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiEncodings.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiTrueType.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiType1.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiType1C.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiIdentifier.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Annot.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Array.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/BuiltinFont.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/BuiltinFontTables.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CachedFile.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Catalog.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CharCodeToUnicode.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CMap.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/DateInfo.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Decrypt.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Dict.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Error.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FileSpec.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FontEncodingTables.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Form.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FontInfo.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Function.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Gfx.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/GfxFont.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/GfxState.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/GlobalParams.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Hints.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JArithmeticDecoder.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JBIG2Stream.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Lexer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Link.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Linearization.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/LocalPDFDocBuilder.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/MarkedContentOutputDev.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/NameToCharCode.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Object.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/OptionalContent.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Outline.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/OutputDev.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Page.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PageTransition.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Parser.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDoc.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDocEncoding.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDocFactory.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PopplerCache.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/ProfileData.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PreScanOutputDev.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PSTokenizer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/SignatureInfo.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Stream.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/StructTreeRoot.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/StructElement.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/strtok_r.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeMap.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeTypeTable.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UTF.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/XRef.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PSOutputDev.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/TextOutputDev.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PageLabelInfo.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/SecurityHandler.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/StdinCachedFile.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/StdinPDFDocBuilder.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Sound.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/ViewerPreferences.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/XpdfPluginAPI.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Movie.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Rendition.cc
)

if(ENABLE_SPLASH)
  set(${poppler_lib_name}_sources ${${poppler_lib_name}_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/SplashOutputDev.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/Splash.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashBitmap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashClip.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFTFont.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFTFontEngine.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFTFontFile.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFont.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFontEngine.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFontFile.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFontFileID.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashPath.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashPattern.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashScreen.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashState.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashT1Font.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashT1FontEngine.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashT1FontFile.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashXPath.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashXPathScanner.cc
  )
endif()

if(ENABLE_LIBCURL)
  set(${poppler_lib_name}_sources ${${poppler_lib_name}_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CurlCachedFile.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CurlPDFDocBuilder.cc
  )
endif()

if(ENABLE_LIBJPEG)
  set(${poppler_lib_name}_sources ${${poppler_lib_name}_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/DCTStream.cc
  )
endif()

if(ENABLE_ZLIB)
  set(${poppler_lib_name}_sources ${${poppler_lib_name}_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FlateStream.cc
  )
endif()

if(ENABLE_ZLIB_UNCOMPRESS)
  set(${poppler_lib_name}_sources ${${poppler_lib_name}_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FlateEncoder.cc
  )
endif()

if(ENABLE_NSS3)
  set(${poppler_lib_name}_sources ${${poppler_lib_name}_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/SignatureHandler.cc
  )
endif()

set(${poppler_lib_name}_sources ${${poppler_lib_name}_sources}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JPXStream.cc
)

set(${poppler_lib_name}_sources ${${poppler_lib_name}_sources}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiBase.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiEncodings.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiIdentifier.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiTrueType.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiType1.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiType1C.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/FixedPoint.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooHash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooLikely.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooList.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooMutex.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooString.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooTimer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/ImgWriter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/JpegWriter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/NetPBMWriter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/PNGWriter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/TiffWriter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gfile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gmem.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/grandom.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gstrtod.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gtypes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gtypes_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Annot.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Array.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/BuiltinFont.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/BuiltinFontTables.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CMap.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CachedFile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CairoFontEngine.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CairoOutputDev.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CairoRescaleBox.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Catalog.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CharCodeToUnicode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CharTypes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CompactFontTables.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CurlCachedFile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CurlPDFDocBuilder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/DCTStream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/DateInfo.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Decrypt.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Dict.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Error.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/ErrorCodes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FileSpec.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FlateStream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FontEncodingTables.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FontInfo.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Form.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Function.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Gfx.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/GfxFont.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/GfxState.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/GfxState_helpers.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/GlobalParams.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Hints.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JArithmeticDecoder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JBIG2Stream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JPEG2000Stream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JPXStream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Lexer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Linearization.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Link.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/LocalPDFDocBuilder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/MarkedContentOutputDev.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Movie.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/NameToCharCode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/NameToUnicodeTable.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Object.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/OptionalContent.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Outline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/OutputDev.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDoc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDocBuilder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDocEncoding.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDocFactory.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PSOutputDev.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PSTokenizer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Page.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PageLabelInfo.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PageLabelInfo_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PageTransition.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Parser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PopplerCache.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PreScanOutputDev.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/ProfileData.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Rendition.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/SecurityHandler.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Sound.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/SplashOutputDev.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/StdinCachedFile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/StdinPDFDocBuilder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Stream-CCITT.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Stream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/StructElement.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/StructTreeRoot.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/TextOutputDev.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UTF.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UTF8.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeCClassTables.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeCompTables.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeDecompTables.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeMap.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeMapTables.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeTypeTable.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/ViewerPreferences.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/XRef.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/XpdfPluginAPI.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/Splash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashBitmap.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashClip.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashErrorCodes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFTFont.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFTFontEngine.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFTFontFile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFont.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFontEngine.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFontFile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFontFileID.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashGlyphBitmap.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashMath.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashPath.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashPattern.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashScreen.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashState.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashT1Font.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashT1FontEngine.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashT1FontFile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashTypes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashXPath.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashXPathScanner.h

  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/poppler/poppler-config.h
)

add_library(${poppler_lib_name} STATIC ${${poppler_lib_name}_sources})

set_property(TARGET ${poppler_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${poppler_lib_name}
  INTERFACE
    -DPOPPLER_VERSION="${POPPLER_VERSION}"
)

target_include_directories(${poppler_lib_name}
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include/poppler
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler
)

target_link_libraries(${poppler_lib_name}
  PUBLIC
    ${core_dll_name}
    ${freetype2_dll_name}
    ${png_dll_name}
    ${tiff_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ${zlib_dll_name}
)

if(ENABLE_LIBJPEG)
  target_link_libraries(${poppler_lib_name}
    PUBLIC
      ${curl_dll_name}
  )
endif()

if(ENABLE_LIBJPEG)
  target_link_libraries(${poppler_lib_name}
    PUBLIC
      ${jpeg_dll_name}
  )
endif()

if(ENABLE_ZLIB)
  target_link_libraries(${poppler_lib_name}
    PUBLIC
      ${zlib_dll_name}
  )
endif()

if(WITH_FONTCONFIGURATION_FONTCONFIG)
  target_link_libraries(${poppler_lib_name}
    PUBLIC
      ${fontconfig_dll_name}
  )
endif()

if(WITH_UI_QT)
  add_subdirectory(qt-shared)
endif()

if(WITH_UI_QT)
  add_subdirectory(utils)
endif()
