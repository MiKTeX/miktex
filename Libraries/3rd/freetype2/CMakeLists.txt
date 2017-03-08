## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2007-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/freetype2")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/autofit
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/psaux
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/psnames
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/raster
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sfnt
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/smooth
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/truetype
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include
)

set(interface_definitions
  -DHAVE_FT2BUILD_H=1
  -DHAVE_LIBFREETYPE=1
)

add_definitions(
  -DFT2_BUILD_LIBRARY
)

ignore_warnings()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/freetype/config)

file(READ
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/freetype/config/ftoption.h
  ftoption_h
)

file(WRITE
  ${CMAKE_CURRENT_BINARY_DIR}/ftoption.h.tmp
  "${ftoption_h}"
)

configure_file(
  ${CMAKE_CURRENT_BINARY_DIR}/ftoption.h.tmp
  ${CMAKE_CURRENT_BINARY_DIR}/include/freetype/config/ftoption.h
  COPYONLY
)

file(READ
  ${CMAKE_CURRENT_SOURCE_DIR}/source/builds/unix/ftconfig.in
  ftconfig_h
)

string(REGEX REPLACE
  "#undef +(HAVE_FCNTL_H)" "#define \\1"
  ftconfig_h "${ftconfig_h}"
)

string(REGEX REPLACE
  "#undef +(HAVE_STDINT_H)" "#define \\1"
  ftconfig_h "${ftconfig_h}"
)

string(REGEX REPLACE
  "#undef +(HAVE_UNISTD_H)" "#define \\1"
  ftconfig_h "${ftconfig_h}"
)

string(REPLACE "/undef " "#undef "
  ftconfig_h "${ftconfig_h}"
)

file(WRITE
  ${CMAKE_CURRENT_BINARY_DIR}/ftconfig.h.tmp
  "${ftconfig_h}"
)

configure_file(
  ${CMAKE_CURRENT_BINARY_DIR}/ftconfig.h.tmp
  ${CMAKE_CURRENT_BINARY_DIR}/include/freetype/config/ftconfig.h
  COPYONLY
)

set(sources
  ${CMAKE_CURRENT_BINARY_DIR}/include/freetype/config/ftconfig.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/freetype/config/ftoption.h
  
  source/src/autofit/autofit.c
  source/src/base/ftbase.c
  source/src/base/ftbbox.c
  source/src/base/ftbdf.c
  source/src/base/ftbitmap.c
  source/src/base/ftcid.c
  source/src/base/ftfntfmt.c
  source/src/base/ftfstype.c
  source/src/base/ftgasp.c
  source/src/base/ftglyph.c
  source/src/base/ftgxval.c
  source/src/base/ftinit.c
  source/src/base/ftlcdfil.c
  source/src/base/ftmm.c
  source/src/base/ftotval.c
  source/src/base/ftpatent.c
  source/src/base/ftpfr.c
  source/src/base/ftstroke.c
  source/src/base/ftsynth.c
  source/src/base/ftsystem.c
  source/src/base/fttype1.c
  source/src/base/ftwinfnt.c
  source/src/bdf/bdf.c
  source/src/bzip2/ftbzip2.c
  source/src/cache/ftcache.c
  source/src/cff/cff.c
  source/src/cid/type1cid.c
  source/src/gzip/ftgzip.c
  source/src/lzw/ftlzw.c
  source/src/pcf/pcf.c
  source/src/pfr/pfr.c
  source/src/psaux/psaux.c
  source/src/pshinter/pshinter.c
  source/src/psnames/psnames.c
  source/src/raster/raster.c
  source/src/sfnt/sfnt.c
  source/src/smooth/smooth.c
  source/src/truetype/truetype.c
  source/src/type1/type1.c
  source/src/type42/type42.c
  source/src/winfonts/winfnt.c

  source/include/freetype/config/ftconfig.h
  source/include/freetype/config/ftheader.h
  source/include/freetype/config/ftmodule.h
  source/include/freetype/config/ftoption.h
  source/include/freetype/config/ftstdlib.h
  source/include/freetype/freetype.h
  source/include/freetype/ftadvanc.h
  source/include/freetype/ftautoh.h
  source/include/freetype/ftbbox.h
  source/include/freetype/ftbdf.h
  source/include/freetype/ftbitmap.h
  source/include/freetype/ftbzip2.h
  source/include/freetype/ftcache.h
  source/include/freetype/ftcffdrv.h
  source/include/freetype/ftchapters.h
  source/include/freetype/ftcid.h
  source/include/freetype/fterrdef.h
  source/include/freetype/fterrors.h
  source/include/freetype/ftfntfmt.h
  source/include/freetype/ftgasp.h
  source/include/freetype/ftglyph.h
  source/include/freetype/ftgxval.h
  source/include/freetype/ftgzip.h
  source/include/freetype/ftimage.h
  source/include/freetype/ftincrem.h
  source/include/freetype/ftlcdfil.h
  source/include/freetype/ftlist.h
  source/include/freetype/ftlzw.h
  source/include/freetype/ftmac.h
  source/include/freetype/ftmm.h
  source/include/freetype/ftmodapi.h
  source/include/freetype/ftmoderr.h
  source/include/freetype/ftotval.h
  source/include/freetype/ftoutln.h
  source/include/freetype/ftpfr.h
  source/include/freetype/ftrender.h
  source/include/freetype/ftsizes.h
  source/include/freetype/ftsnames.h
  source/include/freetype/ftstroke.h
  source/include/freetype/ftsynth.h
  source/include/freetype/ftsystem.h
  source/include/freetype/fttrigon.h
  source/include/freetype/ftttdrv.h
  source/include/freetype/fttypes.h
  source/include/freetype/ftwinfnt.h
  source/include/freetype/internal
  source/include/freetype/internal/autohint.h
  source/include/freetype/internal/ftcalc.h
  source/include/freetype/internal/ftdebug.h
  source/include/freetype/internal/ftdriver.h
  source/include/freetype/internal/ftgloadr.h
  source/include/freetype/internal/fthash.h
  source/include/freetype/internal/ftmemory.h
  source/include/freetype/internal/ftobjs.h
  source/include/freetype/internal/ftpic.h
  source/include/freetype/internal/ftrfork.h
  source/include/freetype/internal/ftserv.h
  source/include/freetype/internal/ftstream.h
  source/include/freetype/internal/fttrace.h
  source/include/freetype/internal/ftvalid.h
  source/include/freetype/internal/internal.h
  source/include/freetype/internal/psaux.h
  source/include/freetype/internal/pshints.h
  source/include/freetype/internal/services
  source/include/freetype/internal/services/svbdf.h
  source/include/freetype/internal/services/svcid.h
  source/include/freetype/internal/services/svfntfmt.h
  source/include/freetype/internal/services/svgldict.h
  source/include/freetype/internal/services/svgxval.h
  source/include/freetype/internal/services/svkern.h
  source/include/freetype/internal/services/svmm.h
  source/include/freetype/internal/services/svotval.h
  source/include/freetype/internal/services/svpfr.h
  source/include/freetype/internal/services/svpostnm.h
  source/include/freetype/internal/services/svprop.h
  source/include/freetype/internal/services/svpscmap.h
  source/include/freetype/internal/services/svpsinfo.h
  source/include/freetype/internal/services/svsfnt.h
  source/include/freetype/internal/services/svttcmap.h
  source/include/freetype/internal/services/svtteng.h
  source/include/freetype/internal/services/svttglyf.h
  source/include/freetype/internal/services/svwinfnt.h
  source/include/freetype/internal/sfnt.h
  source/include/freetype/internal/t1types.h
  source/include/freetype/internal/tttypes.h
  source/include/freetype/t1tables.h
  source/include/freetype/ttnameid.h
  source/include/freetype/tttables.h
  source/include/freetype/tttags.h
  source/include/freetype/ttunpat.h
  source/include/ft2build.h

  freetype2-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND sources
    source/builds/windows/ftdebug.c
  )
endif()

set(${freetype2_dll_name}_sources ${sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    freetype2.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/freetype2.rc
  )
  list(APPEND ${freetype2_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/freetype2.rc
  )
endif()

add_library(${freetype2_dll_name} SHARED ${${freetype2_dll_name}_sources})

set_property(TARGET ${freetype2_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${freetype2_dll_name}
  INTERFACE
    ${interface_definitions}
)

target_include_directories(${freetype2_dll_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${freetype2_dll_name}
  PRIVATE
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
)

set_target_properties(${freetype2_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_MAJOR_MINOR_STR}"
    SOVERSION "1"
)

rebase(${freetype2_dll_name})

install(TARGETS ${freetype2_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
