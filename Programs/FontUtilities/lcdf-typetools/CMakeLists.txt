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
set(HAVE_IOSTREAM 1)
set(HAVE_NEW_HDR 1)
set(HAVE_PERMSTRING 1)
set(HAVE_UINTPTR_T 1)

set(HAVE_ADOBE_CODE 1)
set(HAVE_AUTO_CFFTOT1 1)
set(HAVE_AUTO_T1DOTLESSJ 1)
set(HAVE_AUTO_TTFTOTYPE42 1)
set(HAVE_AUTO_UPDMAP 1)

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
  -DHAVE_DECL_KPSE_ENC_FORMAT
  -DHAVE_DECL_KPSE_OPENTYPE_FORMAT
  -DHAVE_KPATHSEA
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DNOMINMAX
    -DUNICODE
    -D_UNICODE
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
## cfftot1

set(cfftot1_sources
  ${CMAKE_CURRENT_BINARY_DIR}/lcdf-typetools-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  source/cfftot1/cfftot1.cc
  source/cfftot1/maket1font.cc
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND cfftot1_sources
    ${MIKTEX_COMMON_MANIFEST}
    cfftot1.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}cfftot1 ${cfftot1_sources} ${headers})

set_property(TARGET ${MIKTEX_PREFIX}cfftot1 PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}cfftot1
  ${app_dll_name}
  ${core_dll_name}
  libefont
  liblcdf
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}cfftot1
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}cfftot1 DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## mmafm

set(mmafm_sources
  ${CMAKE_CURRENT_BINARY_DIR}/lcdf-typetools-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  source/mmafm/main.cc
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND mmafm_sources
    ${MIKTEX_COMMON_MANIFEST}
    mmafm.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}mmafm ${mmafm_sources} ${headers})

set_property(TARGET ${MIKTEX_PREFIX}mmafm PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}mmafm
  ${app_dll_name}
  ${core_dll_name}
  libefont
  liblcdf
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}mmafm
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}mmafm DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## mmpfb

set(mmpfb_sources
  ${CMAKE_CURRENT_BINARY_DIR}/lcdf-typetools-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  source/mmpfb/main.cc
  source/mmpfb/myfont.cc
  source/mmpfb/myfont.hh
  source/mmpfb/t1minimize.cc
  source/mmpfb/t1minimize.hh
  source/mmpfb/t1rewrit.cc
  source/mmpfb/t1rewrit.hh
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND mmpfb_sources
    ${MIKTEX_COMMON_MANIFEST}
    mmpfb.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}mmpfb ${mmpfb_sources} ${headers})

set_property(TARGET ${MIKTEX_PREFIX}mmpfb PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}mmpfb
  ${app_dll_name}
  ${core_dll_name}
  libefont
  liblcdf
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}mmpfb
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}mmpfb DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## otfinfo

set(otfinfo_sources
  ${CMAKE_CURRENT_BINARY_DIR}/lcdf-typetools-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  source/otfinfo/otfinfo.cc
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND otfinfo_sources
    ${MIKTEX_COMMON_MANIFEST}
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

###############################################################################
## otftotfm

set(otftotfm_sources
  ${CMAKE_CURRENT_BINARY_DIR}/lcdf-typetools-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  source/otftotfm/automatic.cc
  source/otftotfm/automatic.hh
  source/otftotfm/dvipsencoding.cc
  source/otftotfm/dvipsencoding.hh
  source/otftotfm/glyphfilter.cc
  source/otftotfm/glyphfilter.hh
  source/otftotfm/kpseinterface.c
  source/otftotfm/kpseinterface.h
  source/otftotfm/metrics.cc
  source/otftotfm/metrics.hh
  source/otftotfm/otftotfm.cc
  source/otftotfm/otftotfm.hh
  source/otftotfm/secondary.cc
  source/otftotfm/secondary.hh
  source/otftotfm/setting.hh
  source/otftotfm/uniprop.cc
  source/otftotfm/uniprop.hh
  source/otftotfm/util.cc
  source/otftotfm/util.hh
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND otftotfm_sources
    ${MIKTEX_COMMON_MANIFEST}
    otftotfm.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}otftotfm ${otftotfm_sources} ${headers})

set_property(TARGET ${MIKTEX_PREFIX}otftotfm PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}otftotfm
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  libefont
  liblcdf
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}otftotfm
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}otftotfm DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## t1dotlessj

set(t1dotlessj_sources
  ${CMAKE_CURRENT_BINARY_DIR}/lcdf-typetools-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  source/t1dotlessj/t1dotlessj.cc
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND t1dotlessj_sources
    ${MIKTEX_COMMON_MANIFEST}
    t1dotlessj.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}t1dotlessj ${t1dotlessj_sources} ${headers})

set_property(TARGET ${MIKTEX_PREFIX}t1dotlessj PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}t1dotlessj
  ${app_dll_name}
  ${core_dll_name}
  libefont
  liblcdf
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}t1dotlessj
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}t1dotlessj DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## t1lint

set(t1lint_sources
  ${CMAKE_CURRENT_BINARY_DIR}/lcdf-typetools-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  source/t1lint/cscheck.cc
  source/t1lint/cscheck.hh
  source/t1lint/t1lint.cc
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND t1lint_sources
    ${MIKTEX_COMMON_MANIFEST}
    t1lint.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}t1lint ${t1lint_sources} ${headers})

set_property(TARGET ${MIKTEX_PREFIX}t1lint PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}t1lint
  ${app_dll_name}
  ${core_dll_name}
  libefont
  liblcdf
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}t1lint
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}t1lint DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## t1rawafm

set(t1rawafm_sources
  ${CMAKE_CURRENT_BINARY_DIR}/lcdf-typetools-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  source/t1rawafm/t1rawafm.cc
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND t1rawafm_sources
    ${MIKTEX_COMMON_MANIFEST}
    t1rawafm.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}t1rawafm ${t1rawafm_sources} ${headers})

set_property(TARGET ${MIKTEX_PREFIX}t1rawafm PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}t1rawafm
  ${app_dll_name}
  ${core_dll_name}
  libefont
  liblcdf
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}t1rawafm
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}t1rawafm DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## t1reencode

set(t1reencode_sources
  ${CMAKE_CURRENT_BINARY_DIR}/lcdf-typetools-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  source/t1reencode/t1reencode.cc
  source/t1reencode/util.cc
  source/t1reencode/util.hh
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND t1reencode_sources
    ${MIKTEX_COMMON_MANIFEST}
    t1reencode.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}t1reencode ${t1reencode_sources} ${headers})

set_property(TARGET ${MIKTEX_PREFIX}t1reencode PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}t1reencode
  ${app_dll_name}
  ${core_dll_name}
  libefont
  liblcdf
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}t1reencode
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}t1reencode DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## t1testpage

set(t1testpage_sources
  ${CMAKE_CURRENT_BINARY_DIR}/lcdf-typetools-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  source/t1testpage/t1testpage.cc
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND t1testpage_sources
    ${MIKTEX_COMMON_MANIFEST}
    t1testpage.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}t1testpage ${t1testpage_sources} ${headers})

set_property(TARGET ${MIKTEX_PREFIX}t1testpage PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}t1testpage
  ${app_dll_name}
  ${core_dll_name}
  libefont
  liblcdf
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}t1testpage
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}t1testpage DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## ttftotype42

set(ttftotype42_sources
  ${CMAKE_CURRENT_BINARY_DIR}/lcdf-typetools-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  source/ttftotype42/ttftotype42.cc
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND ttftotype42_sources
    ${MIKTEX_COMMON_MANIFEST}
    ttftotype42.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}ttftotype42 ${ttftotype42_sources} ${headers})

set_property(TARGET ${MIKTEX_PREFIX}ttftotype42 PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}ttftotype42
  ${app_dll_name}
  ${core_dll_name}
  libefont
  liblcdf
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}ttftotype42
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}ttftotype42 DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

