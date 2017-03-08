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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/graphite2")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src
)

add_definitions(
  -DGRAPHITE2_EXPORTING
  -DUNICODE
  -D_UNICODE
)

ignore_warnings()

set(GRAPHITE2_VM_TYPE auto)
set(GRAPHITE2_NSEGCACHE TRUE)
set(GRAPHITE2_NFILEFACE FALSE)
set(GRAPHITE2_NTRACING TRUE)

string(TOLOWER ${GRAPHITE2_VM_TYPE} GRAPHITE2_VM_TYPE)

if(GRAPHITE2_VM_TYPE STREQUAL "auto")
  if(CMAKE_BUILD_TYPE MATCHES "[Rr]el(ease|[Ww]ith[Dd]eb[Ii]nfo)")
    set(GRAPHITE2_VM_TYPE "direct")
  else()
    set(GRAPHITE2_VM_TYPE "call")
  endif()
endif()

if(GRAPHITE2_VM_TYPE STREQUAL "direct" AND NOT (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_BASE_NAME STREQUAL "clang++"))
  set(GRAPHITE2_VM_TYPE "call")
endif()

set(GRAPHITE_API_CURRENT 3)
set(GRAPHITE_API_REVISION 0)
set(GRAPHITE_API_AGE 1)
set(GRAPHITE_VERSION ${GRAPHITE_API_CURRENT}.${GRAPHITE_API_REVISION}.${GRAPHITE_API_AGE})
set(GRAPHITE_SO_VERSION ${GRAPHITE_API_CURRENT})

set(SEGCACHE
  source/src/SegCache.cpp
  source/src/SegCacheEntry.cpp
  source/src/SegCacheStore.cpp
)

if(GRAPHITE2_NSEGCACHE)
  add_definitions(-DGRAPHITE2_NSEGCACHE)
  set(SEGCACHE)
endif()

set(FILEFACE
  source/src/FileFace.cpp
  )

if(GRAPHITE2_NFILEFACE)
  add_definitions(-DGRAPHITE2_NFILEFACE)
  set(FILEFACE)
endif()

set(TRACING
  source/src/json.cpp
)

if(GRAPHITE2_NTRACING)
  add_definitions(-DGRAPHITE2_NTRACING)
  set(TRACING)
endif()

set(GRAPHITE_HEADERS 
  source/include/graphite2/Font.h
  source/include/graphite2/Segment.h
  source/include/graphite2/Types.h
  source/include/graphite2/Log.h
)

file(GLOB PRIVATE_HEADERS source/src/inc/*.h) 

set(${graphite2_dll_name}_sources
  source/src/${GRAPHITE2_VM_TYPE}_machine.cpp
  source/src/CachedFace.cpp
  source/src/CmapCache.cpp
  source/src/Code.cpp
  source/src/Collider.cpp
  source/src/Decompressor.cpp
  source/src/Face.cpp
  source/src/FeatureMap.cpp
  source/src/Font.cpp
  source/src/GlyphCache.cpp
  source/src/GlyphFace.cpp
  source/src/Intervals.cpp
  source/src/Justifier.cpp
  source/src/NameTable.cpp
  source/src/Pass.cpp
  source/src/Position.cpp
  source/src/Segment.cpp
  source/src/Silf.cpp
  source/src/Slot.cpp
  source/src/Sparse.cpp
  source/src/TtfUtil.cpp
  source/src/UtfCodec.cpp
  source/src/gr_char_info.cpp
  source/src/gr_face.cpp
  source/src/gr_features.cpp
  source/src/gr_font.cpp
  source/src/gr_logging.cpp
  source/src/gr_segment.cpp
  source/src/gr_slot.cpp
  source/src/inc/CachedFace.h
  source/src/inc/CharInfo.h
  source/src/inc/CmapCache.h
  source/src/inc/Code.h
  source/src/inc/Collider.h
  source/src/inc/Compression.h
  source/src/inc/Decompressor.h
  source/src/inc/Endian.h
  source/src/inc/Error.h
  source/src/inc/Face.h
  source/src/inc/FeatureMap.h
  source/src/inc/FeatureVal.h
  source/src/inc/FileFace.h
  source/src/inc/Font.h
  source/src/inc/GlyphCache.h
  source/src/inc/GlyphFace.h
  source/src/inc/Intervals.h
  source/src/inc/List.h
  source/src/inc/Machine.h
  source/src/inc/Main.h
  source/src/inc/NameTable.h
  source/src/inc/Pass.h
  source/src/inc/Position.h
  source/src/inc/Rule.h
  source/src/inc/SegCache.h
  source/src/inc/SegCacheEntry.h
  source/src/inc/SegCacheStore.h
  source/src/inc/Segment.h
  source/src/inc/Silf.h
  source/src/inc/Slot.h
  source/src/inc/Sparse.h
  source/src/inc/TtfTypes.h
  source/src/inc/TtfUtil.h
  source/src/inc/UtfCodec.h
  source/src/inc/bits.h
  source/src/inc/debug.h
  source/src/inc/json.h
  source/src/inc/locale2lcid.h
  source/src/inc/opcode_table.h
  source/src/inc/opcodes.h

  ${FILEFACE}
  ${SEGCACHE}
  ${TRACING}
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    graphite2.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/graphite2.rc
  )
  set(${graphite2_dll_name}_sources
    ${${graphite2_dll_name}_sources}
    ${CMAKE_BINARY_DIR}/${MIKTEX_REL_GRAPHITE2_DIR}/graphite2.rc
  )
endif()

add_library(${graphite2_dll_name} SHARED ${${graphite2_dll_name}_sources})

set_property(TARGET ${graphite2_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${graphite2_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_MAJOR_MINOR_STR}"
    SOVERSION "1"
)

target_include_directories(${graphite2_dll_name}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include
)

rebase(${graphite2_dll_name})

install(TARGETS ${graphite2_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
