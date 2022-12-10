## CMakeLists.txt
##
## Copyright (C) 2015-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_VALIDATION_FOLDER}/chktex")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MIKTEX_NATIVE_WINDOWS)
    set(HAVE_ACCESS 1)
    set(HAVE_CLOSEDIR 1)
    set(HAVE_OPENDIR 1)
    set(HAVE_READDIR 1)
endif()

set(HAVE_POSIX_ERE 1)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
    miktex-chktex-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-chktex-version.h
)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
    -DKPATHSEA
    -DTEX_LIVE
    -DUSE_MIKTEX_EXIT
    -Dmain=Main
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

set(chktex_c_sources
    source/ChkTeX.c
    source/FindErrs.c
    source/OpSys.c
    source/Resource.c
    source/Utility.c
)

set(chktex_h_sources
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-chktex-version.h
    source/ChkTeX.h
    source/FindErrs.h
    source/OpSys.h
    source/Resource.h
    source/Utility.h
    source/types.h
)

#set_source_files_properties(
#  ${chktex_c_sources}
#  PROPERTIES LANGUAGE CXX
#)

set(chktex_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${chktex_c_sources}
    ${chktex_h_sources}
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        miktex-chktex.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-chktex.rc
    )
    list(APPEND chktex_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-chktex.rc
        ${MIKTEX_COMMON_MANIFEST}
    )
endif()

add_executable(${MIKTEX_PREFIX}chktex ${chktex_sources})

set_property(TARGET ${MIKTEX_PREFIX}chktex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}chktex
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}chktex
        ${regex_dll_name}
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}chktex DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
