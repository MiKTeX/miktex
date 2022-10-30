## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER ${MIKTEX_CURRENT_FOLDER}/pdfTeX)

if(LINK_EVERYTHING_STATICALLY)
    use_static_crt()
endif()

default_char_type_is_unsigned()

set(C4P_FLAGS
    --auto-exit=10
    --chars-are-unsigned
    --emit-optimize-pragmas
)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_INCLUDE_DIR}
)

add_definitions(
    -DHAVE_C4P_PRE_H
    -DHAVE_GETPDFMAJORVERSION
    -DPDF_PARSER_ONLY
    -DSYNCTEX_ENGINE_H="synctex-pdftex.h"
    -DTeX
    -DUSE_GZIP
    -D__SyncTeX__
    -DpdfTeX
)

include(webify.cmake)

set(pdftex_web_file ${CMAKE_CURRENT_BINARY_DIR}/pdftex-final.web)
set(pdftex_header_file ${CMAKE_CURRENT_BINARY_DIR}/pdftexd.h)

file(READ ${MIKTEX_DYN_TEX_SCRIPT} tex_dyn_sed_contents)
file(READ ${MIKTEX_DYN_ETEX_SCRIPT} etex_dyn_sed_contents)

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed "${tex_dyn_sed_contents}")
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed "${etex_dyn_sed_contents}")
endif()

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/openbsd-compat.h)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/openbsd-compat.h "
#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#define snprintf _snprintf
#if ! defined(__GNUC__)
#  define __attribute__(x)
#endif
#endif
"
    )
endif()

set(cpp_files
    ${CMAKE_CURRENT_BINARY_DIR}/pdftex_pool.cpp
    ${projdir}/source/pdftoepdf.cc
    miktex-pdftex.cpp
)

set(c_files
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex.c
    ${projdir}/source/avl.c
    ${projdir}/source/avlstuff.c
    ${projdir}/source/epdf.c
    ${projdir}/source/mapfile.c
    ${projdir}/source/pdftexextra.c
    ${projdir}/source/pkin.c
    ${projdir}/source/subfont.c
    ${projdir}/source/tounicode.c
    ${projdir}/source/utils.c
    ${projdir}/source/vfpacket.c
    ${projdir}/source/writeenc.c
    ${projdir}/source/writefont.c
    ${projdir}/source/writeimg.c
    ${projdir}/source/writejbig2.c
    ${projdir}/source/writejpg.c
    ${projdir}/source/writepng.c
    ${projdir}/source/writet1.c
    ${projdir}/source/writet3.c
    ${projdir}/source/writettf.c
    ${projdir}/source/writezip.c
)

set_source_files_properties(${c_files} PROPERTIES LANGUAGE CXX)

set(h_files
    ${CMAKE_BINARY_DIR}/include/miktex/pdftex.defaults.h
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-common.h
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-pdftex.h
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex.h
    ${projdir}/source/avl.h
    ${projdir}/source/avlstuff.h
    ${projdir}/source/image.h
    ${projdir}/source/pdftex-common.h
    ${projdir}/source/pdftex.h
    ${projdir}/source/pdftex_version.h
    ${projdir}/source/pdftexextra.h
    ${projdir}/source/ptexlib.h
    ${projdir}/source/ptexmac.h
    ${projdir}/source/writettf.h
    c4p_pre.h
    miktex-first.h
    miktex-pdftex.h
    miktex-pdftex-version.h
)

set(pdftex_target_name pdftex)

set(${pdftex_target_name}_sources
    ${c_files}
    ${cpp_files}
    ${h_files}
)

set_source_files_properties(
    ${${pdftex_target_name}_sources}
    PROPERTIES COMPILE_FLAGS "-DC4PEXTERN=extern"
)

include(l10n.cmake)

create_web_app(pdfTeX)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/pdftex_pool.cpp
    COMMAND
        inipool
            ${CMAKE_CURRENT_BINARY_DIR}/pdftex.pool
            miktex-pdftex.h
            ${pdftex_progclass}
            ${pdftex_prog}
        > ${CMAKE_CURRENT_BINARY_DIR}/pdftex_pool.cpp
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/pdftex.pool
        inipool
)

add_dependencies(${pdftex_target_name} gen-defaults)

set_source_files_properties(
    ${CMAKE_BINARY_DIR}/include/miktex/pdftex.defaults.h
    PROPERTIES
        GENERATED TRUE
)

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/pdftex.cc
    PROPERTIES COMPILE_FLAGS "-DCOMPILING_PDFTEX_CC"
)

if(LINK_EVERYTHING_STATICALLY)
    if(USE_SYSTEM_JPEG)
        target_link_libraries(${MIKTEX_PREFIX}pdftex MiKTeX::Imported::JPEG)
    else()
        target_link_libraries(${MIKTEX_PREFIX}pdftex ${jpeg_lib_name})
    endif()
    if(USE_SYSTEM_PNG)
        target_link_libraries(${MIKTEX_PREFIX}pdftex MiKTeX::Imported::PNG)
    else()
        target_link_libraries(${MIKTEX_PREFIX}pdftex ${png_lib_name})
    endif()
    target_link_libraries(${MIKTEX_PREFIX}pdftex
        ${kpsemu_lib_name}
        ${md5_lib_name}
        ${w2cemu_lib_name}
        ${web2c_sources_lib_name}
        ${xpdf_lib_name}
    )
    if(MIKTEX_NATIVE_WINDOWS)
        target_link_libraries(${MIKTEX_PREFIX}pdftex
        ${unxemu_lib_name}
        ${utf8wrap_lib_name}
        )
    endif()
else()
    if(USE_SYSTEM_JPEG)
        target_link_libraries(${pdftex_target_name} PRIVATE MiKTeX::Imported::JPEG)
    else()
        target_link_libraries(${pdftex_target_name} PRIVATE ${jpeg_dll_name})
    endif()
    if(USE_SYSTEM_PNG)
        target_link_libraries(${pdftex_target_name} PRIVATE MiKTeX::Imported::PNG)
    else()
        target_link_libraries(${pdftex_target_name} PRIVATE ${png_dll_name})
    endif()
    target_link_libraries(${pdftex_target_name}
        PRIVATE
        ${kpsemu_dll_name}
        ${md5_dll_name}
        ${w2cemu_dll_name}
        ${web2c_sources_dll_name}
        ${xpdf_lib_name}
    )
    if(MIKTEX_NATIVE_WINDOWS)
        target_link_libraries(${pdftex_target_name}
        PRIVATE
            ${unxemu_dll_name}
            ${utf8wrap_dll_name}
        )
    endif()
endif()

if(NOT LINK_EVERYTHING_STATICALLY)
    if(USE_SYSTEM_ZLIB)
        target_link_libraries(${pdftex_target_name} PRIVATE MiKTeX::Imported::ZLIB)
    else()
        target_link_libraries(${pdftex_target_name} PRIVATE ${zlib_dll_name})
    endif()
    if(MIKTEX_NATIVE_WINDOWS)
        target_link_libraries(${pdftex_target_name}
        PRIVATE
            ${regex_dll_name}
        )
    endif()
endif()

if(NOT LINK_EVERYTHING_STATICALLY)
    delay_load(${pdftex_target_name}
        ${png_dll_name}
        ${regex_dll_name}
    )
endif()
