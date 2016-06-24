## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_DVIWARE_FOLDER}/dvipdfm-x")

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

add_definitions(
  -DHAVE_CONFIG_H
)

if(NOT HAVE_TIMEZONE)
  if(MSVC)
    add_definitions(
      -DHAVE_TIMEZONE
      -Dtimezone=_timezone
    )
  endif()
endif()

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

ignore_warnings()

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)  

set(libutil_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  source/agl.c
  source/agl.h
  source/bmpimage.c
  source/bmpimage.h
  source/cff_dict.h
  source/cff_limits.h
  source/cff_stdstr.h
  source/cff_types.h
  source/cid_basefont.h
  source/cidtype0.h
  source/cidtype2.h
  source/cmap.h
  source/cmap_p.h
  source/cmap_read.h
  source/cmap_write.c
  source/cmap_write.h
  source/cs_type2.c
  source/cs_type2.h
  source/dpxconf.c
  source/dpxconf.h
  source/dpxcrypt.c
  source/dpxcrypt.h
  source/dpxfile.c
  source/dpxfile.h
  source/dpxutil.c
  source/dpxutil.h
  source/dvi.h
  source/dvicodes.h
  source/dvipdfmx.h
  source/epdf.c
  source/epdf.h
  source/error.c
  source/error.h
  source/jp2image.c
  source/jp2image.h
  source/jpegimage.c
  source/jpegimage.h
  source/mem.c
  source/mem.h
  source/mfileio.c
  source/mfileio.h
  source/mpost.h
  source/numbers.c
  source/numbers.h
  source/otl_conf.c
  source/otl_conf.h
  source/otl_opt.c
  source/otl_opt.h
  source/pdfcolor.c
  source/pdfcolor.h
  source/pdfdev.h
  source/pdfdoc.h
  source/pdfdraw.c
  source/pdfdraw.h
  source/pdfencoding.c
  source/pdfencoding.h
  source/pdfencrypt.c
  source/pdfencrypt.h
  source/pdflimits.h
  source/pdfnames.h
  source/pdfobj.c
  source/pdfobj.h
  source/pdfparse.c
  source/pdfparse.h
  source/pdfresource.c
  source/pdfresource.h
  source/pdfximage.c
  source/pdfximage.h
  source/pkfont.h
  source/pngimage.c
  source/pngimage.h
  source/pst.c
  source/pst.h
  source/pst_obj.c
  source/pst_obj.h
  source/spc_color.c
  source/spc_color.h
  source/spc_dvipdfmx.c
  source/spc_dvipdfmx.h
  source/spc_dvips.c
  source/spc_dvips.h
  source/spc_html.c
  source/spc_html.h
  source/spc_misc.c
  source/spc_misc.h
  source/spc_pdfm.h
  source/spc_tpic.c
  source/spc_tpic.h
  source/spc_util.c
  source/spc_util.h
  source/spc_xtx.h
  source/specials.c
  source/specials.h
  source/subfont.c
  source/subfont.h
  source/system.h
  source/t1_char.h
  source/t1_load.h
  source/tfm.c
  source/tfm.h
  source/truetype.h
  source/tt_aux.h
  source/tt_glyf.h
  source/tt_gsub.h
  source/tt_post.h
  source/tt_table.h
  source/type0.h
  source/type1.h
  source/type1c.c
  source/type1c.h
  source/unicode.c
  source/unicode.h
  source/vf.c
  source/vf.h
  source/xbb.c
)

set(common_program_sources
  source/cff.c
  source/cff.h
  source/cff_dict.c
  source/cid.c
  source/cid.h
  source/cid_p.h
  source/cidtype0.c
  source/cidtype2.c
  source/cmap.c
  source/cmap_read.c
  source/dvi.c
  source/dvipdfmx.c
  source/fontmap.c
  source/fontmap.h
  source/mpost.c
  source/pdfdev.c
  source/pdfdoc.c
  source/pdffont.c
  source/pdffont.h
  source/pdfnames.c
  source/pkfont.c
  source/sfnt.c
  source/sfnt.h
  source/spc_pdfm.c
  source/spc_xtx.c
  source/t1_char.c
  source/t1_load.c
  source/truetype.c
  source/tt_aux.c
  source/tt_cmap.c
  source/tt_cmap.h
  source/tt_glyf.c
  source/tt_gsub.c
  source/tt_post.c
  source/tt_table.c
  source/type0.c
  source/type1.c
)

set(dvipdfmx_dll_sources ${libutil_sources} ${common_program_sources} dvipdfm-x-version.h)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND dvipdfmx_dll_sources
    dvipdfmx.rc
  )
endif()

add_library(
  ${dvipdfmx_dll_name}
  SHARED
  ${dvipdfmx_dll_sources}
)

set_property(TARGET ${dvipdfmx_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(
  TARGETS ${dvipdfmx_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

target_link_libraries(${dvipdfmx_dll_name}
  PRIVATE
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${png_dll_name}
    ${texmf_dll_name}
    ${zlib_dll_name}
)

if (MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${dvipdfmx_dll_name}
    PRIVATE
      ${getopt_dll_name}
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()

add_executable (${MIKTEX_PREFIX}dvipdfmx ${MIKTEX_LIBRARY_WRAPPER})

set_property(TARGET ${MIKTEX_PREFIX}dvipdfmx PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}dvipdfmx
  ${app_dll_name}
  ${core_dll_name}
  ${dvipdfmx_dll_name}
)

if (MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}dvipdfmx
    ${utf8wrap_dll_name}
  )
endif()

merge_trustinfo_manifest(${MIKTEX_PREFIX}dvipdfmx asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}dvipdfmx)

install(TARGETS ${MIKTEX_PREFIX}dvipdfmx DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(xdvipdfmx_sources
  ${common_program_sources}
  ${libutil_sources}
  ${MIKTEX_LIBRARY_WRAPPER}
  dvipdfm-x-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND xdvipdfmx_sources
    xdvipdfmx.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}xdvipdfmx ${xdvipdfmx_sources})

set_property(TARGET ${MIKTEX_PREFIX}xdvipdfmx PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_property(
  TARGET ${MIKTEX_PREFIX}xdvipdfmx
  APPEND PROPERTY COMPILE_DEFINITIONS "XETEX")

merge_trustinfo_manifest(${MIKTEX_PREFIX}xdvipdfmx asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}xdvipdfmx)

target_link_libraries(${MIKTEX_PREFIX}xdvipdfmx
  PRIVATE
    ${app_dll_name}
    ${core_dll_name}
    ${fontconfig_dll_name}
    ${freetype2_dll_name}
    ${kpsemu_dll_name}
    ${png_dll_name}
    ${zlib_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}xdvipdfmx
    PRIVATE
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()

delay_load(${MIKTEX_PREFIX}xdvipdfmx
  ${fontconfig_dll_name}
  ${freetype2_dll_name}
  ${png_dll_name}
  ${zlib_dll_name}
)

install(TARGETS ${MIKTEX_PREFIX}xdvipdfmx DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(dvipdft_sources
  dvipdft.cpp
  dvipdft-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND dvipdft_sources
    dvipdft.rc
  )
endif()

add_executable(dvipdft ${dvipdft_sources})

set_property(TARGET dvipdft PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(dvipdft asInvoker)
merge_compatibility_manifest(dvipdft)

target_link_libraries(dvipdft
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${texmf_dll_name}
)

install(TARGETS dvipdft DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
