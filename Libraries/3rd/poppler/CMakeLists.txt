## CMakeLists.txt
##
## Copyright (C) 2000-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/poppler")

set(projdir ${CMAKE_CURRENT_SOURCE_DIR})
  
include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include/poppler
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler
)

set(ENABLE_LIBCURL TRUE)
set(ENABLE_LIBJPEG TRUE)
set(ENABLE_LIBPNG TRUE)
set(ENABLE_SPLASH TRUE)
set(ENABLE_TIFF TRUE)

set(DEFAULT_SIGNATURE_BACKEND "None")

if(MIKTEX_NATIVE_WINDOWS)
    set(WITH_FONTCONFIGURATION_WIN32 TRUE)
    set(WITH_FONTCONFIGURATION_FONTCONFIG FALSE)
else()
    set(WITH_FONTCONFIGURATION_WIN32 FALSE)
    set(WITH_FONTCONFIGURATION_FONTCONFIG FALSE)
endif()

## see: https://github.com/MiKTeX/miktex/issues/20
set(MULTITHREADED TRUE)
set(OPI_SUPPORT TRUE)
set(TEXTOUT_WORD_LIST TRUE)

set(HAVE_LIBZ TRUE)
set(HAVE_SPLASH TRUE)
set(HAVE_FREETYPE_H TRUE)

set(HAVE_CAIRO TRUE)
set(HAVE_DCT_DECODER TRUE)
set(HAVE_JPX_DECODER TRUE)
set(HAVE_UNISTD_H TRUE)

add_definitions(-DHAVE_CONFIG_H=1)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(-DNOMINMAX=1)
endif()

configure_file(
    miktex-poppler-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-poppler-version.h
)

configure_file(
    source/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
    source/poppler/poppler-config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/poppler/poppler-config.h
)

set(poppler_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooString.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooTimer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/ImgWriter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/JpegWriter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/NetPBMWriter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/PNGWriter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/TiffWriter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/ft_utils.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gbase64.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gbasename.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gfile.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/glibc.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/glibc_strtok_r.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/grandom.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gstrtod.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiBase.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiEncodings.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiTrueType.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiType1.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiType1C.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiIdentifier.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Annot.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/AnnotStampImageHelper.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Array.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CachedFile.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Catalog.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CharCodeToUnicode.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CMap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CryptoSignBackend.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/DateInfo.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Decrypt.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Dict.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Error.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FDPDFDocBuilder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FILECacheLoader.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FileSpec.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FlateEncoder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FontEncodingTables.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Form.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FontInfo.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Function.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Gfx.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/GfxFont.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/GfxState.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/GlobalParams.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Hints.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/ImageEmbeddingUtils.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JArithmeticDecoder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JBIG2Stream.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JSInfo.cc
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
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDocBuilder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDocEncoding.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDocFactory.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/ProfileData.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PreScanOutputDev.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PSTokenizer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/SignatureInfo.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Stream.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/StructTreeRoot.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/StructElement.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeMap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeMapFuncs.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeTypeTable.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UTF.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/XRef.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PSOutputDev.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/TextOutputDev.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PageLabelInfo.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/SecurityHandler.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Sound.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/ViewerPreferences.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Movie.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Rendition.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CertificateInfo.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/BBoxOutputDev.cc
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
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashXPath.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashXPathScanner.cc
)

if(ENABLE_LIBCURL)
    list(APPEND poppler_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CurlCachedFile.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CurlPDFDocBuilder.cc
    )
endif()

if(ENABLE_LIBJPEG)
    list(APPEND poppler_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/DCTStream.cc
    )
endif()

list(APPEND poppler_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JPXStream.cc
)

list(APPEND poppler_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CourierWidths.pregenerated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CourierBoldWidths.pregenerated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CourierBoldObliqueWidths.pregenerated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CourierObliqueWidths.pregenerated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/HelveticaWidths.pregenerated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/HelveticaBoldWidths.pregenerated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/HelveticaBoldObliqueWidths.pregenerated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/HelveticaObliqueWidths.pregenerated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/SymbolWidths.pregenerated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/TimesBoldWidths.pregenerated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/TimesBoldItalicWidths.pregenerated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/TimesItalicWidths.pregenerated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/TimesRomanWidths.pregenerated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/ZapfDingbatsWidths.pregenerated.c
)

list(APPEND poppler_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiEncodings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiIdentifier.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiTrueType.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiType1.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/fofi/FoFiType1C.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/ft_utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gbase64.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gbasename.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gdir.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gfile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/glibc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gmem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooCheckedOps.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooLikely.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooString.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/GooTimer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/grandom.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/gstrtod.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/ImgWriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/JpegWriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/NetPBMWriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/PNGWriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/goo/TiffWriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Annot.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/AnnotStampImageHelper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_approved.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_as_is.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_confidential.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_departmental.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_draft.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_experimental.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_expired.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_final.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_for_comment.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_for_public_release.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_not_approved.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_not_for_public_release.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_sold.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/annot_stamp_top_secret.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Array.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/BBoxOutputDev.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/BuiltinFont.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/BuiltinFontWidth.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CachedFile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CairoFontEngine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CairoOutputDev.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CairoRescaleBox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Catalog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CertificateInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CharCodeToUnicode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CharTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CIDFontsWidthsBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CMap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CryptoSignBackend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CurlCachedFile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/CurlPDFDocBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/DateInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/DCTStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Decrypt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Dict.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/DistinguishedNameParser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Error.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/ErrorCodes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FDPDFDocBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FILECacheLoader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FileSpec.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/FlateEncoder.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/GPGMECryptoSignBackend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/HashAlgorithm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Hints.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/ImageEmbeddingUtils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JArithmeticDecoder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JBIG2Stream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JPEG2000Stream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JPXStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/JSInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Lexer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Linearization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Link.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/LocalPDFDocBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/MarkedContentOutputDev.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Movie.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/NameToCharCode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/NameToUnicodeTable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/NSSCryptoSignBackend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Object.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/OptionalContent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Outline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/OutputDev.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Page.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PageLabelInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PageLabelInfo_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PageTransition.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Parser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDoc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDocBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDocEncoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PDFDocFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PopplerCache.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PreScanOutputDev.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/ProfileData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PSOutputDev.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/PSTokenizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Rendition.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/SecurityHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/SignatureInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Sound.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/SplashOutputDev.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Stream-CCITT.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/Stream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/StructElement.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/StructTreeRoot.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/TextOutputDev.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeCClassTables.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeCompTables.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeDecompTables.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeMap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeMapFuncs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeMapTables.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UnicodeTypeTable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/UTF.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/ViewerPreferences.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/poppler/XRef.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/Splash.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashBitmap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashClip.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashErrorCodes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFont.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFontEngine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFontFile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFontFileID.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFTFont.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFTFontEngine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashFTFontFile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashGlyphBitmap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashMath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashPath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashPattern.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashScreen.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashXPath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/splash/SplashXPathScanner.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/HtmlFonts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/HtmlLinks.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/HtmlOutputDev.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/HtmlUtils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/ImageOutputDev.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/InMemoryFile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/numberofcharacters.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/parseargs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/pdftocairo-win32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/printencodings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/sanitychecks.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/Win32Console.h

    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/poppler/poppler-config.h
)

add_subdirectory(shared)

if(WITH_UI_QT)
    add_subdirectory(qt5/shared)
endif()

set(poppler_target_library ${poppler_dll_name})

generate_export_header(${poppler_target_library}
    BASE_NAME poppler-private
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/poppler_private_export.h
)

add_subdirectory(utils)
