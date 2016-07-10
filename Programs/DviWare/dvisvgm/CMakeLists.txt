## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2015-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_DVIWARE_FOLDER}/dvisvgm")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  source/clipper
  source/xxHash
)

add_definitions(
  -DHAVE_LIBKPATHSEA
  -DHAVE_LIBPOTRACE
  -DHAVE_LIBZ
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
    -D__WIN32__
  )
endif()

if(CMAKE_CL_64)
  add_definitions(
    -D__WIN64__
  )
endif()

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(libclipper_sources
  source/clipper/clipper.cpp
  source/clipper/clipper.hpp
)

set(libxxhash_sources
  source/xxHash/xxhash.c
  source/xxHash/xxhash.h
)

set(libdvisvgm_sources
  source/src/AGLTable.h
  source/src/BasicDVIReader.cpp
  source/src/BasicDVIReader.h
  source/src/Bezier.cpp
  source/src/Bezier.h
  source/src/BgColorSpecialHandler.cpp
  source/src/BgColorSpecialHandler.h
  source/src/Bitmap.cpp
  source/src/Bitmap.h
  source/src/BoundingBox.cpp
  source/src/BoundingBox.h
  source/src/CMap.cpp
  source/src/CMap.h
  source/src/CMapManager.cpp
  source/src/CMapManager.h
  source/src/CMapReader.cpp
  source/src/CMapReader.h
  source/src/CRC32.cpp
  source/src/CRC32.h
  source/src/Calculator.cpp
  source/src/Calculator.h
  source/src/CharMapID.cpp
  source/src/CharMapID.h
  source/src/Character.h
  source/src/CmdLineParserBase.cpp
  source/src/CmdLineParserBase.h
  source/src/Color.cpp
  source/src/Color.h
  source/src/ColorSpecialHandler.cpp
  source/src/ColorSpecialHandler.h
  source/src/CommandLine.cpp
  source/src/CommandLine.h
  source/src/DLLoader.cpp
  source/src/DLLoader.h
  source/src/DVIActions.h
  source/src/DVIReader.cpp
  source/src/DVIReader.h
  source/src/DVIToSVG.cpp
  source/src/DVIToSVG.h
  source/src/DVIToSVGActions.cpp
  source/src/DVIToSVGActions.h
  source/src/DependencyGraph.h
  source/src/Directory.cpp
  source/src/Directory.h
  source/src/DvisvgmSpecialHandler.cpp
  source/src/DvisvgmSpecialHandler.h
  source/src/EPSFile.cpp
  source/src/EPSFile.h
  source/src/EPSToSVG.cpp
  source/src/EPSToSVG.h
  source/src/EmSpecialHandler.cpp
  source/src/EmSpecialHandler.h
  source/src/EncFile.cpp
  source/src/EncFile.h
  source/src/FileFinder.cpp
  source/src/FileFinder.h
  source/src/FilePath.cpp
  source/src/FilePath.h
  source/src/FileSystem.cpp
  source/src/FileSystem.h
  source/src/Font.cpp
  source/src/Font.h
  source/src/FontCache.cpp
  source/src/FontCache.h
  source/src/FontEncoding.cpp
  source/src/FontEncoding.h
  source/src/FontEngine.cpp
  source/src/FontEngine.h
  source/src/FontManager.cpp
  source/src/FontManager.h
  source/src/FontMap.cpp
  source/src/FontMap.h
  source/src/FontMetrics.cpp
  source/src/FontMetrics.h
  source/src/FontStyle.h
  source/src/GFGlyphTracer.cpp
  source/src/GFGlyphTracer.h
  source/src/GFReader.cpp
  source/src/GFReader.h
  source/src/GFTracer.cpp
  source/src/GFTracer.h
  source/src/Ghostscript.cpp
  source/src/Ghostscript.h
  source/src/Glyph.h
  source/src/GlyphTracerMessages.h
  source/src/GraphicsPath.h
  source/src/HtmlSpecialHandler.cpp
  source/src/HtmlSpecialHandler.h
  source/src/InputBuffer.cpp
  source/src/InputBuffer.h
  source/src/InputReader.cpp
  source/src/InputReader.h
  source/src/JFM.cpp
  source/src/JFM.h
  source/src/Length.cpp
  source/src/Length.h
  source/src/MapLine.cpp
  source/src/MapLine.h
  source/src/Matrix.cpp
  source/src/Matrix.h
  source/src/Message.cpp
  source/src/Message.h
  source/src/MessageException.h
  source/src/MetafontWrapper.cpp
  source/src/MetafontWrapper.h
  source/src/NoPsSpecialHandler.cpp
  source/src/NoPsSpecialHandler.h
  source/src/NumericRanges.h
  source/src/PSFilter.h
  source/src/PSInterpreter.cpp
  source/src/PSInterpreter.h
  source/src/PSPattern.cpp
  source/src/PSPattern.h
  source/src/PSPreviewFilter.cpp
  source/src/PSPreviewFilter.h
  source/src/PageRanges.cpp
  source/src/PageRanges.h
  source/src/PageSize.cpp
  source/src/PageSize.h
  source/src/Pair.h
  source/src/PapersizeSpecialHandler.cpp
  source/src/PapersizeSpecialHandler.h
  source/src/PathClipper.cpp
  source/src/PathClipper.h
  source/src/PdfSpecialHandler.cpp
  source/src/PdfSpecialHandler.h
  source/src/PreScanDVIReader.cpp
  source/src/PreScanDVIReader.h
  source/src/Process.cpp
  source/src/Process.h
  source/src/PsSpecialHandler.cpp
  source/src/PsSpecialHandler.h
  source/src/RangeMap.cpp
  source/src/RangeMap.h
  source/src/SVGCharHandler.cpp
  source/src/SVGCharHandler.h
  source/src/SVGCharHandlerFactory.cpp
  source/src/SVGCharHandlerFactory.h
  source/src/SVGCharPathHandler.cpp
  source/src/SVGCharPathHandler.h
  source/src/SVGCharTspanTextHandler.cpp
  source/src/SVGCharTspanTextHandler.h
  source/src/SVGOutput.cpp
  source/src/SVGOutput.h
  source/src/SVGSingleCharTextHandler.cpp
  source/src/SVGSingleCharTextHandler.h
  source/src/SVGTree.cpp
  source/src/SVGTree.h
  source/src/ShadingPatch.cpp
  source/src/ShadingPatch.h
  source/src/SignalHandler.cpp
  source/src/SignalHandler.h
  source/src/SpecialActions.h
  source/src/SpecialHandler.h
  source/src/SpecialManager.cpp
  source/src/SpecialManager.h
  source/src/StreamReader.cpp
  source/src/StreamReader.h
  source/src/StreamWriter.cpp
  source/src/StreamWriter.h
  source/src/Subfont.cpp
  source/src/Subfont.h
  source/src/System.cpp
  source/src/System.h
  source/src/TFM.cpp
  source/src/TFM.h
  source/src/TensorProductPatch.cpp
  source/src/TensorProductPatch.h
  source/src/Terminal.cpp
  source/src/Terminal.h
  source/src/ToUnicodeMap.cpp
  source/src/ToUnicodeMap.h
  source/src/TpicSpecialHandler.cpp
  source/src/TpicSpecialHandler.h
  source/src/TriangularPatch.cpp
  source/src/TriangularPatch.h
  source/src/Unicode.cpp
  source/src/Unicode.h
  source/src/VFActions.h
  source/src/VFReader.cpp
  source/src/VFReader.h
  source/src/VectorIterator.h
  source/src/VectorStream.h
  source/src/XMLDocument.cpp
  source/src/XMLDocument.h
  source/src/XMLNode.cpp
  source/src/XMLNode.h
  source/src/XMLString.cpp
  source/src/XMLString.h
  source/src/macros.h
  source/src/psdefs.cpp
  source/src/types.h
)

set(dvisvgm_c_sources
  source/src/dvisvgm.cpp
  source/src/gzstream.cpp
  source/src/gzstream.h
)

set(dvisvgm_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${dvisvgm_c_sources}
  ${libclipper_sources}
  ${libdvisvgm_sources}
  ${libxxhash_sources}
  config.h
  dvisvgm-version.h
)

ignore_warnings()

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND dvisvgm_sources
    dvisvgm.rc
  )
endif()

set_source_files_properties(${MIKTEX_LIBRARY_WRAPPER}
  PROPERTIES
    COMPILE_FLAGS "-DCPLUSPLUSMAIN -DBEQUIET"
)

add_executable(dvisvgm ${dvisvgm_sources})

set_property(TARGET dvisvgm PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(dvisvgm asInvoker)
merge_compatibility_manifest(dvisvgm)

target_link_libraries(dvisvgm
  ${app_dll_name}
  ${core_dll_name}
  ${freetype2_dll_name}
  ${kpsemu_dll_name}
  ${potrace_lib_name}
  ${texmf_dll_name}
  ${zlib_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(dvisvgm
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
else()
  target_link_libraries(dvisvgm
    ${CMAKE_DL_LIBS}
  )
endif()

install(TARGETS dvisvgm DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
