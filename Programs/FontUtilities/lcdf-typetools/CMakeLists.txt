## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2020 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_FONT_UTILITIES_FOLDER}/lcdf-typetools")

set(WORDS_BIGENDIAN ${MIKTEX_WORDS_BIGENDIAN})
set(WORDS_BIGENDIAN_SET 1)
set(HAVE_DECL_STRNLEN 1)
set(HAVE_NEW_HDR 1)
set(HAVE_PERMSTRING 1)
set(HAVE_UINTPTR_T 1)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  source/include
)

configure_file(
  autoconf.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/autoconf.h
)

configure_file(
  lcdf-typetools-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/lcdf-typetools-version.h
)

add_definitions(
  -DCPLUSPLUSMAIN
  -DHAVE_CONFIG_H
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -D_UNICODE -DUNICODE
  )
endif()

set(headers
  source/include/config.h
  source/include/lcdf/bezier.hh
  source/include/lcdf/clp.h
  source/include/lcdf/error.hh
  source/include/lcdf/filename.hh
  source/include/lcdf/globmatch.hh
  source/include/lcdf/hashcode.hh
  source/include/lcdf/hashmap.hh
  source/include/lcdf/inttypes.h
  source/include/lcdf/landmark.hh
  source/include/lcdf/md5.h
  source/include/lcdf/permstr.hh
  source/include/lcdf/point.hh
  source/include/lcdf/slurper.hh
  source/include/lcdf/straccum.hh
  source/include/lcdf/string.hh
  source/include/lcdf/strtonum.h
  source/include/lcdf/transform.hh
  source/include/lcdf/vector.hh
  source/include/efont/afm.hh
  source/include/efont/afmparse.hh
  source/include/efont/afmw.hh
  source/include/efont/amfm.hh
  source/include/efont/cff.hh
  source/include/efont/encoding.hh
  source/include/efont/findmet.hh
  source/include/efont/metrics.hh
  source/include/efont/otf.hh
  source/include/efont/otfcmap.hh
  source/include/efont/otfdata.hh
  source/include/efont/otfgpos.hh
  source/include/efont/otfgsub.hh
  source/include/efont/otfname.hh
  source/include/efont/otfos2.hh
  source/include/efont/otfpost.hh
  source/include/efont/pairop.hh
  source/include/efont/psres.hh
  source/include/efont/t1bounds.hh
  source/include/efont/t1cs.hh
  source/include/efont/t1csgen.hh
  source/include/efont/t1font.hh
  source/include/efont/t1interp.hh
  source/include/efont/t1item.hh
  source/include/efont/t1mm.hh
  source/include/efont/t1rw.hh
  source/include/efont/t1unparser.hh
  source/include/efont/ttfcs.hh
  source/include/efont/ttfhead.hh
  source/include/efont/ttfkern.hh
)

###############################################################################
## liblcdf

set(liblcdf_sources
  source/liblcdf/bezier.cc
  source/liblcdf/clp.c
  source/liblcdf/error.cc
  source/liblcdf/filename.cc
  source/liblcdf/globmatch.cc
  source/liblcdf/landmark.cc
  source/liblcdf/md5.c
  source/liblcdf/permstr.cc
  source/liblcdf/point.cc
  source/liblcdf/slurper.cc
  source/liblcdf/straccum.cc
  source/liblcdf/string.cc
  source/liblcdf/strtonum.c
  source/liblcdf/transform.cc
  source/liblcdf/vectorv.cc
)

add_library(liblcdf ${liblcdf_sources} ${headers})

set_property(TARGET liblcdf PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(liblcdf
    PRIVATE
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()

###############################################################################
## libefont

set(libefont_sources
  source/libefont/afm.cc
  source/libefont/afmparse.cc
  source/libefont/afmw.cc
  source/libefont/amfm.cc
  source/libefont/cff.cc
  source/libefont/encoding.cc
  source/libefont/findmet.cc
  source/libefont/metrics.cc
  source/libefont/otf.cc
  source/libefont/otfcmap.cc
  source/libefont/otfdata.cc
  source/libefont/otfdescrip.cc
  source/libefont/otfgpos.cc
  source/libefont/otfgsub.cc
  source/libefont/otfname.cc
  source/libefont/otfos2.cc
  source/libefont/otfpost.cc
  source/libefont/pairop.cc
  source/libefont/psres.cc
  source/libefont/t1bounds.cc
  source/libefont/t1cs.cc
  source/libefont/t1csgen.cc
  source/libefont/t1interp.cc
  source/libefont/t1item.cc
  source/libefont/t1font.cc
  source/libefont/t1fontskel.cc
  source/libefont/t1mm.cc
  source/libefont/t1rw.cc
  source/libefont/t1unparser.cc
  source/libefont/ttfcs.cc
  source/libefont/ttfhead.cc
  source/libefont/ttfkern.cc
)

add_library(libefont ${libefont_sources} ${headers})

set_property(TARGET libefont PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(libefont
    PRIVATE
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
      ws2_32
  )
endif()

###############################################################################
## otfinfo

set(otfinfo_sources
  ${CMAKE_CURRENT_BINARY_DIR}/lcdf-typetools-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  source/otfinfo/otfinfo.cc
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND otfinfo_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    otfinfo.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}otfinfo ${otfinfo_sources} ${headers})

set_property(TARGET ${MIKTEX_PREFIX}otfinfo PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}otfinfo
  ${app_dll_name}
  ${core_dll_name}
  libefont
  liblcdf
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}otfinfo
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}otfinfo DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
