## CMakeLists.txt
##
## Copyright (C) 2006-2025 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_DVIWARE_FOLDER}/dvipng")

## cannot compile as C++: too many implicit casts
set(DVIPNG_COMPILE_AS_CXX 0)

set(HAVE_FT2 1)
set(HAVE_FT_LIBRARY_VERSION 1)
set(HAVE_GDIMAGECREATEFROMJPEG 1)
set(HAVE_GDIMAGECREATEFROMPNGPTR 1)
set(HAVE_GDIMAGECREATETRUECOLOR 1)
set(HAVE_GDIMAGEGIF 1)
set(HAVE_GDIMAGEPNGEX 1)
set(HAVE_GD_H 1)
set(HAVE_KPATHSEA_KPATHSEA_H 1)
set(HAVE_KPSE_ENC_FORMATS 1)
set(HAVE_LIBGD 1)
set(HAVE_LIBKPATHSEA 1)
set(HAVE_LIBM)
set(HAVE_LIBPNG)
set(HAVE_LIBZ)
set(HAVE_PNG_H 1)

set(PACKAGE_NAME "dvipng")
set(PACKAGE_STRING "dvipng 1.16")
set(PACKAGE_VERSION "1.16")

configure_file(
    miktex-dvipng-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-dvipng-version.h
)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(dvipng_c_sources
    source/color.c
    source/draw.c
    source/dvi.c
    source/dvipng.c
    source/enc.c
    source/font.c
    source/fontmap.c
    source/ft.c
    source/misc.c
    source/papersiz.c
    source/pk.c
    source/ppagelist.c
    source/set.c
    source/sfd.c
    source/special.c
    source/tfm.c
    source/vf.c
)

set(dvipng_sources
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-dvipng-version.h
    ${MIKTEX_LIBRARY_WRAPPER}
    ${dvipng_c_sources}
    source/commands.h
    source/dvipng.h
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -D_UNICODE
        -DUNICODE
    )
endif()

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-dvipng.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-dvipng.rc
    )
    list(APPEND dvipng_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-dvipng.rc
        ${MIKTEX_COMMON_MANIFEST}
    )
endif()

if(DVIPNG_COMPILE_AS_CXX)
    set_source_files_properties(${dvipng_c_sources}
        PROPERTIES
        LANGUAGE CXX
    )
    set_source_files_properties(${MIKTEX_LIBRARY_WRAPPER}
        PROPERTIES
        COMPILE_FLAGS "-DCPLUSPLUSMAIN -DBEQUIET"
    )
else()
    set_source_files_properties(${MIKTEX_LIBRARY_WRAPPER}
        PROPERTIES
        COMPILE_FLAGS "-DBEQUIET"
    )
endif()

add_executable(${MIKTEX_PREFIX}dvipng ${dvipng_sources})

set_property(TARGET ${MIKTEX_PREFIX}dvipng PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_FREETYPE2)
    target_link_libraries(${MIKTEX_PREFIX}dvipng MiKTeX::Imported::FREETYPE2)
else()
    target_link_libraries(${MIKTEX_PREFIX}dvipng ${freetype2_dll_name})
endif()

if(USE_SYSTEM_GD)
    target_link_libraries(${MIKTEX_PREFIX}dvipng MiKTeX::Imported::GD)
else()
    target_link_libraries(${MIKTEX_PREFIX}dvipng ${gd_dll_name})
endif()

target_link_libraries(${MIKTEX_PREFIX}dvipng
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${texmf_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}dvipng
        ${getopt_dll_name}
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}dvipng DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
