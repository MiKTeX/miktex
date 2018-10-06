## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2017 Christian Schenk
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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_DVIWARE_FOLDER}/dvipdfm-x")

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
  dvipdfm-x-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/dvipdfm-x-version.h
)

configure_file(
  dvipdfm-x.rc.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/dvipdfm-x.rc
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

set(dvipdfm_x_sources
  source/agl.c
  source/agl.h
  source/bmpimage.c
  source/bmpimage.h
  source/cff.c
  source/cff.h
  source/cff_dict.c
  source/cff_dict.h
  source/cff_limits.h
  source/cff_stdstr.h
  source/cff_types.h
  source/cid.c
  source/cid.h
  source/cid_basefont.h
  source/cid_p.h
  source/cidtype0.c
  source/cidtype0.h
  source/cidtype2.c
  source/cidtype2.h
  source/cmap.c
  source/cmap.h
  source/cmap_p.h
  source/cmap_read.c
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
  source/dvi.c
  source/dvi.h
  source/dvicodes.h
  source/dvipdfmx.c
  source/dvipdfmx.h
  source/epdf.c
  source/epdf.h
  source/error.c
  source/error.h
  source/fontmap.c
  source/fontmap.h
  source/jp2image.c
  source/jp2image.h
  source/jpegimage.c
  source/jpegimage.h
  source/mem.c
  source/mem.h
  source/mfileio.c
  source/mfileio.h
  source/mpost.c
  source/mpost.h
  source/numbers.c
  source/numbers.h
  source/otl_opt.c
  source/otl_opt.h
  source/pdfcolor.c
  source/pdfcolor.h
  source/pdfdev.c
  source/pdfdev.h
  source/pdfdoc.c
  source/pdfdoc.h
  source/pdfdraw.c
  source/pdfdraw.h
  source/pdfencoding.c
  source/pdfencoding.h
  source/pdfencrypt.c
  source/pdfencrypt.h
  source/pdffont.c
  source/pdffont.h
  source/pdflimits.h
  source/pdfnames.c
  source/pdfnames.h
  source/pdfobj.c
  source/pdfobj.h
  source/pdfparse.c
  source/pdfparse.h
  source/pdfresource.c
  source/pdfresource.h
  source/pdfximage.c
  source/pdfximage.h
  source/pkfont.c
  source/pkfont.h
  source/pngimage.c
  source/pngimage.h
  source/pst.c
  source/pst.h
  source/pst_obj.c
  source/pst_obj.h
  source/sfnt.c
  source/sfnt.h
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
  source/spc_pdfm.c
  source/spc_pdfm.h
  source/spc_tpic.c
  source/spc_tpic.h
  source/spc_util.c
  source/spc_util.h
  source/spc_xtx.c
  source/spc_xtx.h
  source/specials.c
  source/specials.h
  source/subfont.c
  source/subfont.h
  source/system.h
  source/t1_char.c
  source/t1_char.h
  source/t1_load.c
  source/t1_load.h
  source/tfm.c
  source/tfm.h
  source/truetype.c
  source/truetype.h
  source/tt_aux.c
  source/tt_aux.h
  source/tt_cmap.c
  source/tt_cmap.h
  source/tt_glyf.c
  source/tt_glyf.h
  source/tt_gsub.c
  source/tt_gsub.h
  source/tt_post.c
  source/tt_post.h
  source/tt_table.c
  source/tt_table.h
  source/type0.c
  source/type0.h
  source/type1.c
  source/type1.h
  source/type1c.c
  source/type1c.h
  source/unicode.c
  source/unicode.h
  source/vf.c
  source/vf.h
  source/xbb.c
)

list(APPEND dvipdfm_x_sources
  miktex/dvipdfm-x.h
  miktex/miktex.cpp
)

list(APPEND dvipdfm_x_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${CMAKE_CURRENT_BINARY_DIR}/dvipdfm-x-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND dvipdfm_x_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    ${CMAKE_CURRENT_BINARY_DIR}/dvipdfm-x.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}dvipdfmx ${dvipdfm_x_sources} ${MIKTEX_LIBRARY_WRAPPER})

set_property(TARGET ${MIKTEX_PREFIX}dvipdfmx PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}dvipdfmx
  ${app_dll_name}
  ${kpsemu_dll_name}
)

if(USE_SYSTEM_PNG)
  target_link_libraries(${MIKTEX_PREFIX}dvipdfmx MiKTeX::Imported::PNG)
else()
  target_link_libraries(${MIKTEX_PREFIX}dvipdfmx ${png_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${MIKTEX_PREFIX}dvipdfmx MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${MIKTEX_PREFIX}dvipdfmx ${zlib_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}dvipdfmx
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}dvipdfmx DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(dvipdft_sources
  dvipdft.cpp
  dvipdft-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND dvipdft_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    dvipdft.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}dvipdft ${dvipdft_sources})

set_property(TARGET ${MIKTEX_PREFIX}dvipdft PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}dvipdft
  ${app_dll_name}
  ${core_dll_name}
)

install(TARGETS ${MIKTEX_PREFIX}dvipdft DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
