## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2021 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_TEX_AND_FRIENDS_FOLDER}/pdfTeX")

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
  ${CMAKE_CURRENT_SOURCE_DIR}/source
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_INCLUDE_DIR}
)

add_definitions(
  -DHAVE_C4P_PRE_H
  -DHAVE_GETPDFMAJORVERSION
  -DPDF_PARSER_ONLY
  -DSYNCTEX_ENGINE_H="synctex-pdftex.h"
  -DUSE_GZIP
  -D__SyncTeX__
  -DpdfTeX
  -DTeX
)

if(MSVC)
  add_definitions(
    -D_UNICODE -DUNICODE
  )
endif()

set(pdftex_ch_synctex
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-def.ch0
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-mem.ch0
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-e-mem.ch0
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-e-mem.ch1
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-rec.ch0
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-rec.ch1
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-e-rec.ch0
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-pdf-rec.ch2
)

set(pdftosrc_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  pdftosrc-version.h
  source/pdftosrc.cc
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND pdftosrc_sources
    ${MIKTEX_COMMON_MANIFEST}
    pdftosrc.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}pdftosrc ${pdftosrc_sources})
set_property(TARGET ${MIKTEX_PREFIX}pdftosrc PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
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

set(ttf2afm_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  source/ttf2afm.c
  ttf2afm-version.h
)

set_source_files_properties(
  source/ttf2afm.c
  PROPERTIES LANGUAGE CXX
)

set_source_files_properties(
  ${MIKTEX_LIBRARY_WRAPPER}
  PROPERTIES COMPILE_FLAGS "-DCPLUSPLUSMAIN"
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND ttf2afm_sources
    ${MIKTEX_COMMON_MANIFEST}
    ttf2afm.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}ttf2afm ${ttf2afm_sources})
set_property(TARGET ${MIKTEX_PREFIX}ttf2afm PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(${MIKTEX_PREFIX}ttf2afm
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${texmf_dll_name}
)
if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}ttf2afm
    ${getopt_dll_name}
    ${utf8wrap_dll_name}
  )
endif()
install(TARGETS ${MIKTEX_PREFIX}ttf2afm DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(miktex_synctex_changefiles ${TEX_MIKTEX_SYNCTEX_CH})

set(miktex_tex_change_files
  ${MLTEX_MIKTEX_CH}
  ${ENCTEX_MIKTEX_CH}
  ${TEX_MIKTEX_CH}
  ${TEX_MIKTEX_ENCTEX_CH}
  ${TEX_MIKTEX_HASH_CH}
  ${TEX_MIKTEX_HYPH_CH}
  ${TEX_MIKTEX_POOL_CH}
  ${TEX_MIKTEX_QUIET_CH}
  ${TEX_MIKTEX_SRC_CH}
  ${miktex_synctex_changefiles}
  ${TEX_MIKTEX_STAT_CH}
  ${TEX_MIKTEX_WRITE18_CH}
)

set(pdftex_web_file ${CMAKE_CURRENT_SOURCE_DIR}/source/pdftex.web)
set(pdftex_change_file ${CMAKE_CURRENT_BINARY_DIR}/pdftex-miktex-tied.ch)
set(pdftex_header_file ${CMAKE_CURRENT_BINARY_DIR}/pdftexd.h)

add_custom_command(
  OUTPUT
    ${pdftex_change_file}
  COMMAND
    ${MIKTEX_PREFIX}tie
    -c ${pdftex_change_file}
    ${pdftex_web_file}
    ${pdftex_ch_synctex}
    ${ETEX_MIKTEX_PRE_CH}
    ${CMAKE_CURRENT_SOURCE_DIR}/pdftex-pre.ch
    ${miktex_tex_change_files}
    #${ETEX_MIKTEX_POST_CH}
    ${ETEX_MIKTEX_CH}
    ${CMAKE_CURRENT_SOURCE_DIR}/pdftex-post.ch
    ${CMAKE_CURRENT_SOURCE_DIR}/pdftex-miktex.ch
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    ${MIKTEX_PREFIX}tie
    ${pdftex_web_file}
    ${ETEX_MIKTEX_PRE_CH}
    ${miktex_tex_change_files}
    #${ETEX_MIKTEX_POST_CH}
    ${CMAKE_CURRENT_SOURCE_DIR}/pdftex-post.ch
    ${CMAKE_CURRENT_SOURCE_DIR}/pdftex-miktex.ch
)

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

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex.c
  source/avl.c
  source/avlstuff.c
  source/epdf.c
  source/mapfile.c
  source/pkin.c
  source/subfont.c
  source/tounicode.c
  source/utils.c
  source/vfpacket.c
  source/writeenc.c
  source/writefont.c
  source/writeimg.c
  source/writejbig2.c
  source/writejpg.c
  source/writepng.c
  source/writet1.c
  source/writet3.c
  source/writettf.c
  source/writezip.c
  PROPERTIES LANGUAGE CXX
)

set(pdftex_target_name pdftex)

set(${pdftex_target_name}_sources
  ${CMAKE_BINARY_DIR}/include/miktex/pdftex.defaults.h
  ${CMAKE_CURRENT_BINARY_DIR}/pdftex_pool.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-common.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-pdftex.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex.c
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex.h
  c4p_pre.h
  miktex-first.h
  pdftex-miktex.cpp
  pdftex-miktex.h
  pdftex-version.h
  source/avl.c
  source/avl.h
  source/avlstuff.c
  source/avlstuff.h
  source/epdf.c
  source/image.h
  source/mapfile.c
  source/pdftex-common.h
  source/pdftex.h
  source/pdftexextra.c
  source/pdftoepdf.cc
  source/pkin.c
  source/ptexlib.h
  source/ptexmac.h
  source/subfont.c
  source/tounicode.c
  source/utils.c
  source/vfpacket.c
  source/writeenc.c
  source/writefont.c
  source/writeimg.c
  source/writejbig2.c
  source/writejpg.c
  source/writepng.c
  source/writet1.c
  source/writet3.c
  source/writettf.c
  source/writettf.h
  source/writezip.c
)

set_source_files_properties(source/pdftexextra.c PROPERTIES LANGUAGE CXX)

set_source_files_properties(
  ${${pdftex_target_name}_sources}
  PROPERTIES COMPILE_FLAGS "-DC4PEXTERN=extern"
)

# L10n
set(MIKTEX_COMP_ID "miktex-pdftex")
list(APPEND l_sources pdftex-miktex.h)
add_custom_target(${MIKTEX_COMP_ID}-pot
  COMMAND
    ${XGETTEXT_EXECUTABLE} --join-existing --keyword=T_ --from-code=UTF-8 --add-comments --output=po/${MIKTEX_COMP_ID}.pot ${l_sources}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set_property(TARGET ${MIKTEX_COMP_ID}-pot PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
add_custom_target(gen-${MIKTEX_COMP_ID}-translations)
set_property(TARGET gen-${MIKTEX_COMP_ID}-translations PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
add_dependencies(gen-${MIKTEX_COMP_ID}-translations ${MIKTEX_COMP_ID}-pot)
add_custom_target(${MIKTEX_COMP_ID}-mo ALL)
set_property(TARGET ${MIKTEX_COMP_ID}-mo PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
foreach(l ${MIKTEX_TRANSLATION_DONE_LOCALES})
  add_custom_target(${MIKTEX_COMP_ID}-po-${l}
    COMMAND
      ${MSGMERGE_EXECUTABLE} --update --backup=none po/${l}/${MIKTEX_COMP_ID}.po po/${MIKTEX_COMP_ID}.pot
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_SOURCE_DIR}
  )
  set_property(TARGET ${MIKTEX_COMP_ID}-po-${l} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
  add_dependencies(gen-${MIKTEX_COMP_ID}-translations ${MIKTEX_COMP_ID}-po-${l})
  add_custom_target(${MIKTEX_COMP_ID}-mo-${l} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo)
  set_property(TARGET ${MIKTEX_COMP_ID}-mo-${l} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
  add_dependencies(${MIKTEX_COMP_ID}-mo ${MIKTEX_COMP_ID}-mo-${l})
  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo
    COMMAND
      ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}
    COMMAND
      ${MSGFMT_EXECUTABLE} --output-file=locale/${l}/${MIKTEX_COMP_ID}.mo ${CMAKE_CURRENT_SOURCE_DIR}/po/${l}/${MIKTEX_COMP_ID}.po
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/po/${l}/${MIKTEX_COMP_ID}.po
  )
  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h
    COMMAND
      ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}
    COMMAND
      bintoc messages_${l}
        < ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo
        > ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h.tmp
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h.tmp
      ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h
    COMMAND
      ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h.tmp
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo
      bintoc
    VERBATIM
  )
  list(APPEND generated_${MIKTEX_COMP_ID}_sources ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h)
  set(resource_includes
    "${resource_includes}#include \"${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h\""
  )
  set(resource_adders
    "${resource_adders}R_(\":/${l}/LC_MESSAGES/${MIKTEX_COMP_ID}.mo\", messages_${l});"
  )
endforeach()
configure_file(
  PdfTeXResources.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/PdfTeXResources.h
)
add_custom_target(gen-${MIKTEX_COMP_ID}-sources DEPENDS
  ${generated_${MIKTEX_COMP_ID}_sources}
)
set_property(TARGET gen-${MIKTEX_COMP_ID}-sources PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
list(APPEND ${pdftex_target_name}_sources
   ${CMAKE_CURRENT_BINARY_DIR}/PdfTeXResources.h
  ${generated_${MIKTEX_COMP_ID}_sources}
)
set_source_files_properties(
  ${generated_${MIKTEX_COMP_ID}_sources}
  PROPERTIES
    GENERATED TRUE
)

create_web_app(pdfTeX)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/pdftex_pool.cpp
  COMMAND
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/pdftex.pool
    pdftex-miktex.h
    ${pdftex_progclass}
    ${pdftex_prog}
    > ${CMAKE_CURRENT_BINARY_DIR}/pdftex_pool.cpp
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/pdftex.pool
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

## dev targets

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/pdftex-tied.web
    ${CMAKE_CURRENT_BINARY_DIR}/pdftex-tied-n.web
  COMMAND
    ${MIKTEX_PREFIX}tie
    -m ${CMAKE_CURRENT_BINARY_DIR}/pdftex-tied.web
    ${pdftex_web_file}
    ${pdftex_ch_synctex}
    ${ETEX_MIKTEX_PRE_CH}
    ${CMAKE_CURRENT_SOURCE_DIR}/pdftex-pre.ch
    ${miktex_tex_change_files}
    #${ETEX_MIKTEX_POST_CH}
    ${ETEX_MIKTEX_CH}
    ${CMAKE_CURRENT_SOURCE_DIR}/pdftex-post.ch
  COMMAND
    web-n
    < ${CMAKE_CURRENT_BINARY_DIR}/pdftex-tied.web
    > ${CMAKE_CURRENT_BINARY_DIR}/pdftex-tied-n.web
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    ${MIKTEX_PREFIX}tie
    web-n
    ${pdftex_web_file}
    ${ETEX_MIKTEX_PRE_CH}
    ${miktex_tex_change_files}
    #${ETEX_MIKTEX_PRE_CH}
    ${CMAKE_CURRENT_SOURCE_DIR}/pdftex-post.ch
  VERBATIM
)

add_custom_target(pdftex-dev ALL
  DEPENDS
    ${pdftex_change_file}
    ${CMAKE_CURRENT_BINARY_DIR}/pdftex-tied-n.web
)

set_property(TARGET pdftex-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
