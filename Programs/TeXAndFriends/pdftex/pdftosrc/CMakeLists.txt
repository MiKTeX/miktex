## CMakeLists.txt: miktex-pdftosrc build rules
##
## Copyright (C) 2021 Christian Schenk
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

set(pdftosrc_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${projdir}/source/pdftosrc.cc
    pdftosrc-version.h
)

set_source_files_properties(
    ${MIKTEX_LIBRARY_WRAPPER}
    PROPERTIES COMPILE_FLAGS "-DCPLUSPLUSMAIN"
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND pdftosrc_sources
        ${MIKTEX_COMMON_MANIFEST}
        pdftosrc.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}pdftosrc ${pdftosrc_sources})

set_property(TARGET ${MIKTEX_PREFIX}pdftosrc PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/pdftosrc)

if(USE_SYSTEM_JPEG)
    target_link_libraries(${MIKTEX_PREFIX}pdftosrc MiKTeX::Imported::JPEG)
else()
    target_link_libraries(${MIKTEX_PREFIX}pdftosrc ${jpeg_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${MIKTEX_PREFIX}pdftosrc MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${MIKTEX_PREFIX}pdftosrc ${zlib_dll_name})
endif()

target_link_libraries(${MIKTEX_PREFIX}pdftosrc
    ${app_dll_name}
    ${core_dll_name}
    ${w2cemu_dll_name}
    ${xpdf_lib_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}pdftosrc
        ${getopt_dll_name}
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}pdftosrc DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
