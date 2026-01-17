## CMakeLists.txt
##
## Copyright (C) 2006-2026 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_INDEXING_FOLDER}/MakeIndex")

add_definitions(
    -DHAVE_CTYPE_H
    -DHAVE_LOCALE_H
    -DHAVE_SETLOCALE
    -DHAVE_STRCHR
    -DHAVE_STRINGS_H
    -DHAVE_STRRCHR
    -UDEBUG
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DIBM_PC_MICROSOFT
        -DUNICODE
        -D_UNICODE
    )
endif()

set(makeindex_c_sources
    source/genind.c
    source/mkind.c
    source/qsort.c
    source/scanid.c
    source/scanst.c
    source/sortid.c
)

configure_file(
    miktex-makeindex-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-makeindex-version.h
)

set(makeindex_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-makeindex-version.h
    ${MIKTEX_LIBRARY_WRAPPER}
    ${makeindex_c_sources}
    source/genind.h
    source/mkind.h
    source/scanid.h
    source/scanst.h
)

set_source_files_properties(${MIKTEX_LIBRARY_WRAPPER}
    PROPERTIES
        COMPILE_FLAGS "-DCPLUSPLUSMAIN -DBEQUIET"
)

set_source_files_properties(${makeindex_c_sources}
    PROPERTIES
        LANGUAGE CXX
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-makeindex.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-makeindex.rc
    )
endif()

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND makeindex_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-makeindex.rc
        ${MIKTEX_COMMON_MANIFEST}
    )
endif()

add_executable(${MIKTEX_PREFIX}makeindex ${makeindex_sources})

set_property(TARGET ${MIKTEX_PREFIX}makeindex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}makeindex
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${texmf_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}makeindex
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}makeindex DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
