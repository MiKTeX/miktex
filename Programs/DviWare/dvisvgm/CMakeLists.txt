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
  source/libs/clipper
  source/libs/xxHash
  source/libs/woff2/brotli/enc
  source/libs/woff2/src
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
  )
endif()

if(CMAKE_CL_64)
  add_definitions(
    -D_WIN64
  )
endif()

set(HAVE_CXX11 1)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(libclipper_sources
  source/libs/clipper/clipper.cpp
  source/libs/clipper/clipper.hpp
)

set(libxxhash_sources
  source/libs/xxHash/xxhash.c
  source/libs/xxHash/xxhash.h
)

set(libbrotli_sources
  source/libs/woff2/brotli/enc/backward_references.cc
  source/libs/woff2/brotli/enc/backward_references.h
  source/libs/woff2/brotli/enc/bit_cost.h
  source/libs/woff2/brotli/enc/block_splitter.cc
  source/libs/woff2/brotli/enc/block_splitter.h
  source/libs/woff2/brotli/enc/brotli_bit_stream.cc
  source/libs/woff2/brotli/enc/brotli_bit_stream.h
  source/libs/woff2/brotli/enc/cluster.h
  source/libs/woff2/brotli/enc/command.h
  source/libs/woff2/brotli/enc/compress_fragment.cc
  source/libs/woff2/brotli/enc/compress_fragment.h
  source/libs/woff2/brotli/enc/compress_fragment_two_pass.cc
  source/libs/woff2/brotli/enc/compress_fragment_two_pass.h
  source/libs/woff2/brotli/enc/compressor.h
  source/libs/woff2/brotli/enc/context.h
  source/libs/woff2/brotli/enc/dictionary.cc
  source/libs/woff2/brotli/enc/dictionary.h
  source/libs/woff2/brotli/enc/dictionary_hash.h
  source/libs/woff2/brotli/enc/encode.cc
  source/libs/woff2/brotli/enc/encode.h
  source/libs/woff2/brotli/enc/encode_parallel.cc
  source/libs/woff2/brotli/enc/encode_parallel.h
  source/libs/woff2/brotli/enc/entropy_encode.cc
  source/libs/woff2/brotli/enc/entropy_encode.h
  source/libs/woff2/brotli/enc/entropy_encode_static.h
  source/libs/woff2/brotli/enc/fast_log.h
  source/libs/woff2/brotli/enc/find_match_length.h
  source/libs/woff2/brotli/enc/hash.h
  source/libs/woff2/brotli/enc/histogram.cc
  source/libs/woff2/brotli/enc/histogram.h
  source/libs/woff2/brotli/enc/literal_cost.cc
  source/libs/woff2/brotli/enc/literal_cost.h
  source/libs/woff2/brotli/enc/metablock.cc
  source/libs/woff2/brotli/enc/metablock.h
  source/libs/woff2/brotli/enc/port.h
  source/libs/woff2/brotli/enc/prefix.h
  source/libs/woff2/brotli/enc/ringbuffer.h
  source/libs/woff2/brotli/enc/static_dict.cc
  source/libs/woff2/brotli/enc/static_dict.h
  source/libs/woff2/brotli/enc/static_dict_lut.h
  source/libs/woff2/brotli/enc/streams.cc
  source/libs/woff2/brotli/enc/streams.h
  source/libs/woff2/brotli/enc/transform.h
  source/libs/woff2/brotli/enc/types.h
  source/libs/woff2/brotli/enc/utf8_util.cc
  source/libs/woff2/brotli/enc/utf8_util.h
  source/libs/woff2/brotli/enc/write_bits.h
)

set(libwoff2_sources
  source/libs/woff2/src/buffer.h
  source/libs/woff2/src/file.h
  source/libs/woff2/src/font.cc
  source/libs/woff2/src/font.h
  source/libs/woff2/src/glyph.cc
  source/libs/woff2/src/glyph.h
  source/libs/woff2/src/normalize.cc
  source/libs/woff2/src/normalize.h
  source/libs/woff2/src/port.h
  source/libs/woff2/src/round.h
  source/libs/woff2/src/store_bytes.h
  source/libs/woff2/src/table_tags.cc
  source/libs/woff2/src/table_tags.h
  source/libs/woff2/src/transform.cc
  source/libs/woff2/src/transform.h
  source/libs/woff2/src/variable_length.cc
  source/libs/woff2/src/variable_length.h
  source/libs/woff2/src/woff2_common.cc
  source/libs/woff2/src/woff2_common.h
  source/libs/woff2/src/woff2_dec.h
  source/libs/woff2/src/woff2_enc.cc
  source/libs/woff2/src/woff2_enc.h
  source/libs/woff2/src/woff2_out.cc
  source/libs/woff2/src/woff2_out.h
)

set(libdvisvgm_sources
  source/src/AGLTable.hpp
  source/src/BasicDVIReader.cpp
  source/src/BasicDVIReader.hpp
  source/src/Bezier.cpp
  source/src/Bezier.hpp
  source/src/BgColorSpecialHandler.cpp
  source/src/BgColorSpecialHandler.hpp
  source/src/Bitmap.cpp
  source/src/Bitmap.hpp
  source/src/BoundingBox.cpp
  source/src/BoundingBox.hpp
  source/src/CLCommandLine.cpp
  source/src/CLCommandLine.hpp
  source/src/CLOption.hpp
  source/src/CMap.cpp
  source/src/CMap.hpp
  source/src/CMapManager.cpp
  source/src/CMapManager.hpp
  source/src/CMapReader.cpp
  source/src/CMapReader.hpp
  source/src/CRC32.cpp
  source/src/CRC32.hpp
  source/src/Calculator.cpp
  source/src/Calculator.hpp
  source/src/CharMapID.cpp
  source/src/CharMapID.hpp
  source/src/Character.hpp
  source/src/Color.cpp
  source/src/Color.hpp
  source/src/ColorSpecialHandler.cpp
  source/src/ColorSpecialHandler.hpp
  source/src/DLLoader.cpp
  source/src/DLLoader.hpp
  source/src/DVIActions.hpp
  source/src/DVIReader.cpp
  source/src/DVIReader.hpp
  source/src/DVIToSVG.cpp
  source/src/DVIToSVG.hpp
  source/src/DVIToSVGActions.cpp
  source/src/DVIToSVGActions.hpp
  source/src/DependencyGraph.hpp
  source/src/Directory.cpp
  source/src/Directory.hpp
  source/src/DvisvgmSpecialHandler.cpp
  source/src/DvisvgmSpecialHandler.hpp
  source/src/EPSFile.cpp
  source/src/EPSFile.hpp
  source/src/EPSToSVG.cpp
  source/src/EPSToSVG.hpp
  source/src/EmSpecialHandler.cpp
  source/src/EmSpecialHandler.hpp
  source/src/EncFile.cpp
  source/src/EncFile.hpp
  source/src/FileFinder.cpp
  source/src/FileFinder.hpp
  source/src/FilePath.cpp
  source/src/FilePath.hpp
  source/src/FileSystem.cpp
  source/src/FileSystem.hpp
  source/src/FixWord.hpp
  source/src/Font.cpp
  source/src/Font.hpp
  source/src/FontCache.cpp
  source/src/FontCache.hpp
  source/src/FontEncoding.cpp
  source/src/FontEncoding.hpp
  source/src/FontEngine.cpp
  source/src/FontEngine.hpp
  source/src/FontManager.cpp
  source/src/FontManager.hpp
  source/src/FontMap.cpp
  source/src/FontMap.hpp
  source/src/FontMetrics.cpp
  source/src/FontMetrics.hpp
  source/src/FontStyle.hpp
  source/src/FontWriter.cpp
  source/src/FontWriter.hpp
  source/src/GFGlyphTracer.cpp
  source/src/GFGlyphTracer.hpp
  source/src/GFReader.cpp
  source/src/GFReader.hpp
  source/src/GFTracer.cpp
  source/src/GFTracer.hpp
  source/src/Ghostscript.cpp
  source/src/Ghostscript.hpp
  source/src/Glyph.hpp
  source/src/GlyphTracerMessages.hpp
  source/src/GraphicsPath.hpp
  source/src/HtmlSpecialHandler.cpp
  source/src/HtmlSpecialHandler.hpp
  source/src/InputBuffer.cpp
  source/src/InputBuffer.hpp
  source/src/InputReader.cpp
  source/src/InputReader.hpp
  source/src/JFM.cpp
  source/src/JFM.hpp
  source/src/Length.cpp
  source/src/Length.hpp
  source/src/MapLine.cpp
  source/src/MapLine.hpp
  source/src/Matrix.cpp
  source/src/Matrix.hpp
  source/src/Message.cpp
  source/src/Message.hpp
  source/src/MessageException.hpp
  source/src/MetafontWrapper.cpp
  source/src/MetafontWrapper.hpp
  source/src/NoPsSpecialHandler.cpp
  source/src/NoPsSpecialHandler.hpp
  source/src/NumericRanges.hpp
  source/src/PSFilter.hpp
  source/src/PSInterpreter.cpp
  source/src/PSInterpreter.hpp
  source/src/PSPattern.cpp
  source/src/PSPattern.hpp
  source/src/PSPreviewFilter.cpp
  source/src/PSPreviewFilter.hpp
  source/src/PageRanges.cpp
  source/src/PageRanges.hpp
  source/src/PageSize.cpp
  source/src/PageSize.hpp
  source/src/Pair.hpp
  source/src/PapersizeSpecialHandler.cpp
  source/src/PapersizeSpecialHandler.hpp
  source/src/PathClipper.cpp
  source/src/PathClipper.hpp
  source/src/PdfSpecialHandler.cpp
  source/src/PdfSpecialHandler.hpp
  source/src/PreScanDVIReader.cpp
  source/src/PreScanDVIReader.hpp
  source/src/Process.cpp
  source/src/Process.hpp
  source/src/PsSpecialHandler.cpp
  source/src/PsSpecialHandler.hpp
  source/src/RangeMap.cpp
  source/src/RangeMap.hpp
  source/src/SVGCharHandler.cpp
  source/src/SVGCharHandler.hpp
  source/src/SVGCharHandlerFactory.cpp
  source/src/SVGCharHandlerFactory.hpp
  source/src/SVGCharPathHandler.cpp
  source/src/SVGCharPathHandler.hpp
  source/src/SVGCharTspanTextHandler.cpp
  source/src/SVGCharTspanTextHandler.hpp
  source/src/SVGOutput.cpp
  source/src/SVGOutput.hpp
  source/src/SVGSingleCharTextHandler.cpp
  source/src/SVGSingleCharTextHandler.hpp
  source/src/SVGTree.cpp
  source/src/SVGTree.hpp
  source/src/ShadingPatch.cpp
  source/src/ShadingPatch.hpp
  source/src/SignalHandler.cpp
  source/src/SignalHandler.hpp
  source/src/SpecialActions.hpp
  source/src/SpecialHandler.hpp
  source/src/SpecialManager.cpp
  source/src/SpecialManager.hpp
  source/src/StreamReader.cpp
  source/src/StreamReader.hpp
  source/src/StreamWriter.cpp
  source/src/StreamWriter.hpp
  source/src/Subfont.cpp
  source/src/Subfont.hpp
  source/src/System.cpp
  source/src/System.hpp
  source/src/TFM.cpp
  source/src/TFM.hpp
  source/src/TensorProductPatch.cpp
  source/src/TensorProductPatch.hpp
  source/src/Terminal.cpp
  source/src/Terminal.hpp
  source/src/ToUnicodeMap.cpp
  source/src/ToUnicodeMap.hpp
  source/src/TpicSpecialHandler.cpp
  source/src/TpicSpecialHandler.hpp
  source/src/TriangularPatch.cpp
  source/src/TriangularPatch.hpp
  source/src/Unicode.cpp
  source/src/Unicode.hpp
  source/src/VFActions.hpp
  source/src/VFReader.cpp
  source/src/VFReader.hpp
  source/src/VectorIterator.hpp
  source/src/VectorStream.hpp
  source/src/XMLDocument.cpp
  source/src/XMLDocument.hpp
  source/src/XMLNode.cpp
  source/src/XMLNode.hpp
  source/src/XMLString.cpp
  source/src/XMLString.hpp
  source/src/ZLibOutputStream.hpp
  source/src/macros.hpp
  source/src/psdefs.cpp
  source/src/version.hpp
)

list(APPEND libdvisvgm_sources
  source/src/ffwrapper.c
  source/src/ffwrapper.h
)

set(dvisvgm_c_sources
  source/src/dvisvgm.cpp
)

set(dvisvgm_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${MIKTEX_LIBRARY_WRAPPER}
  ${dvisvgm_c_sources}
  ${libbrotli_sources}
  ${libclipper_sources}
  ${libdvisvgm_sources}
  ${libwoff2_sources}
  ${libxxhash_sources}
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
  ${ff_woff_lib_name}
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
