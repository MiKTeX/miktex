## CMakeLists.txt
##
## Copyright (C) 2013-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

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

set(GRAPHITE2_VM_TYPE auto)
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
set(GRAPHITE_API_REVISION 2)
set(GRAPHITE_API_AGE 1)
set(GRAPHITE_VERSION ${GRAPHITE_API_CURRENT}.${GRAPHITE_API_REVISION}.${GRAPHITE_API_AGE})
set(GRAPHITE_SO_VERSION ${GRAPHITE_API_CURRENT})

if(GRAPHITE2_NSEGCACHE)
    add_definitions(-DGRAPHITE2_NSEGCACHE)
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
    source/src/gr_char_info.cpp
    source/src/gr_features.cpp
    source/src/gr_face.cpp
    source/src/gr_font.cpp
    source/src/gr_logging.cpp
    source/src/gr_segment.cpp
    source/src/gr_slot.cpp
    source/src/CmapCache.cpp
    source/src/Code.cpp
    source/src/Collider.cpp
    source/src/Decompressor.cpp
    source/src/Face.cpp
    source/src/FeatureMap.cpp
    source/src/Font.cpp
    source/src/GlyphFace.cpp
    source/src/GlyphCache.cpp
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

    ${FILEFACE}
    ${TRACING}

    ${GRAPHITE_HEADERS}
    ${PRIVATE_HEADERS}
)

configure_file(
    miktex-graphite2-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-graphite2-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-graphite2.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-graphite2.rc
    )
    list(APPEND ${graphite2_dll_name}_sources
        ${CMAKE_BINARY_DIR}/${MIKTEX_REL_GRAPHITE2_DIR}/miktex-graphite2.rc
    )
endif()

add_library(${graphite2_dll_name} SHARED ${${graphite2_dll_name}_sources})

set_property(TARGET ${graphite2_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${graphite2_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${graphite2_dll_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/include
)

install(TARGETS ${graphite2_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
