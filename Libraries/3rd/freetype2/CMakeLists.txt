## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2007-2020 Christian Schenk
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
  -DFT2_DLLIMPORT=1
)

add_definitions(
  -DFT2_BUILD_LIBRARY
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DDLL_EXPORT
  )
endif()

configure_file(
  freetype2-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/freetype2-version.h
)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/source/builds/unix/ftconfig.h.in
  ftconfig_h
)

string(REGEX REPLACE
  "#undef +(HAVE_FCNTL_H)" "#define \\1 1"
  ftconfig_h "${ftconfig_h}"
)

string(REGEX REPLACE
  "#undef +(HAVE_STDINT_H)" "#define \\1 1"
  ftconfig_h "${ftconfig_h}"
)

string(REGEX REPLACE
  "#undef +(HAVE_UNISTD_H)" "#define \\1 1"
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
  source/src/base/ftfstype.c
  source/src/base/ftgasp.c
  source/src/base/ftglyph.c
  source/src/base/ftgxval.c
  source/src/base/ftinit.c
  source/src/base/ftmm.c
  source/src/base/ftotval.c
  source/src/base/ftpatent.c
  source/src/base/ftpfr.c
  source/src/base/ftstroke.c
  source/src/base/ftsynth.c
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
  source/include/freetype/config/integer-types.h
  source/include/freetype/config/mac-support.h
  source/include/freetype/config/public-macros.h
  source/include/freetype/freetype.h
  source/include/freetype/ftadvanc.h
  source/include/freetype/ftbbox.h
  source/include/freetype/ftbdf.h
  source/include/freetype/ftbitmap.h
  source/include/freetype/ftbzip2.h
  source/include/freetype/ftcache.h
  source/include/freetype/ftchapters.h
  source/include/freetype/ftcid.h
  source/include/freetype/ftcolor.h
  source/include/freetype/ftdriver.h
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
  source/include/freetype/ftparams.h
  source/include/freetype/ftpfr.h
  source/include/freetype/ftrender.h
  source/include/freetype/ftsizes.h
  source/include/freetype/ftsnames.h
  source/include/freetype/ftstroke.h
  source/include/freetype/ftsynth.h
  source/include/freetype/ftsystem.h
  source/include/freetype/fttrigon.h
  source/include/freetype/fttypes.h
  source/include/freetype/ftwinfnt.h
  source/include/freetype/internal/autohint.h
  source/include/freetype/internal/cffotypes.h
  source/include/freetype/internal/cfftypes.h
  source/include/freetype/internal/compiler-macros.h
  source/include/freetype/internal/ftcalc.h
  source/include/freetype/internal/ftdebug.h
  source/include/freetype/internal/ftdrv.h
  source/include/freetype/internal/ftgloadr.h
  source/include/freetype/internal/fthash.h
  source/include/freetype/internal/ftmemory.h
  source/include/freetype/internal/ftobjs.h
  source/include/freetype/internal/ftpsprop.h
  source/include/freetype/internal/ftrfork.h
  source/include/freetype/internal/ftserv.h
  source/include/freetype/internal/ftstream.h
  source/include/freetype/internal/fttrace.h
  source/include/freetype/internal/ftvalid.h
  source/include/freetype/internal/psaux.h
  source/include/freetype/internal/pshints.h
  source/include/freetype/internal/services/svbdf.h
  source/include/freetype/internal/services/svcfftl.h
  source/include/freetype/internal/services/svcid.h
  source/include/freetype/internal/services/svfntfmt.h
  source/include/freetype/internal/services/svgldict.h
  source/include/freetype/internal/services/svgxval.h
  source/include/freetype/internal/services/svkern.h
  source/include/freetype/internal/services/svmetric.h
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
  source/include/freetype/internal/wofftypes.h
  source/include/freetype/t1tables.h
  source/include/freetype/ttnameid.h
  source/include/freetype/tttables.h
  source/include/freetype/tttags.h
  source/include/ft2build.h
  source/src/autofit/afangles.h
  source/src/autofit/afblue.h
  source/src/autofit/afcjk.h
  source/src/autofit/afcover.h
  source/src/autofit/afdummy.h
  source/src/autofit/aferrors.h
  source/src/autofit/afglobal.h
  source/src/autofit/afhints.h
  source/src/autofit/afindic.h
  source/src/autofit/aflatin.h
  source/src/autofit/aflatin2.h
  source/src/autofit/afloader.h
  source/src/autofit/afmodule.h
  source/src/autofit/afranges.h
  source/src/autofit/afscript.h
  source/src/autofit/afshaper.h
  source/src/autofit/afstyles.h
  source/src/autofit/aftypes.h
  source/src/autofit/afwarp.h
  source/src/autofit/afwrtsys.h
  source/src/base/ftbase.h
  source/src/base/md5.h
  source/src/bdf/bdf.h
  source/src/bdf/bdfdrivr.h
  source/src/bdf/bdferror.h
  source/src/cache/ftccache.h
  source/src/cache/ftccback.h
  source/src/cache/ftcerror.h
  source/src/cache/ftcglyph.h
  source/src/cache/ftcimage.h
  source/src/cache/ftcmanag.h
  source/src/cache/ftcmru.h
  source/src/cache/ftcsbits.h
  source/src/cff/cffcmap.h
  source/src/cff/cffdrivr.h
  source/src/cff/cfferrs.h
  source/src/cff/cffgload.h
  source/src/cff/cffload.h
  source/src/cff/cffobjs.h
  source/src/cff/cffparse.h
  source/src/cff/cfftoken.h
  source/src/cid/ciderrs.h
  source/src/cid/cidgload.h
  source/src/cid/cidload.h
  source/src/cid/cidobjs.h
  source/src/cid/cidparse.h
  source/src/cid/cidriver.h
  source/src/cid/cidtoken.h
  source/src/gxvalid/gxvalid.h
  source/src/gxvalid/gxvcommn.h
  source/src/gxvalid/gxverror.h
  source/src/gxvalid/gxvfeat.h
  source/src/gxvalid/gxvmod.h
  source/src/gxvalid/gxvmort.h
  source/src/gxvalid/gxvmorx.h
  source/src/gzip/ftzconf.h
  source/src/gzip/infblock.h
  source/src/gzip/infcodes.h
  source/src/gzip/inffixed.h
  source/src/gzip/inftrees.h
  source/src/gzip/infutil.h
  source/src/gzip/zlib.h
  source/src/gzip/zutil.h
  source/src/lzw/ftzopen.h
  source/src/otvalid/otvalid.h
  source/src/otvalid/otvcommn.h
  source/src/otvalid/otverror.h
  source/src/otvalid/otvgpos.h
  source/src/otvalid/otvmod.h
  source/src/pcf/pcf.h
  source/src/pcf/pcfdrivr.h
  source/src/pcf/pcferror.h
  source/src/pcf/pcfread.h
  source/src/pcf/pcfutil.h
  source/src/pfr/pfrcmap.h
  source/src/pfr/pfrdrivr.h
  source/src/pfr/pfrerror.h
  source/src/pfr/pfrgload.h
  source/src/pfr/pfrload.h
  source/src/pfr/pfrobjs.h
  source/src/pfr/pfrsbit.h
  source/src/pfr/pfrtypes.h
  source/src/psaux/afmparse.h
  source/src/psaux/cffdecode.h
  source/src/psaux/psarrst.h
  source/src/psaux/psauxerr.h
  source/src/psaux/psauxmod.h
  source/src/psaux/psblues.h
  source/src/psaux/psconv.h
  source/src/psaux/pserror.h
  source/src/psaux/psfixed.h
  source/src/psaux/psfont.h
  source/src/psaux/psft.h
  source/src/psaux/psglue.h
  source/src/psaux/pshints.h
  source/src/psaux/psintrp.h
  source/src/psaux/psobjs.h
  source/src/psaux/psread.h
  source/src/psaux/psstack.h
  source/src/psaux/pstypes.h
  source/src/psaux/t1cmap.h
  source/src/psaux/t1decode.h
  source/src/pshinter/pshalgo.h
  source/src/pshinter/pshglob.h
  source/src/pshinter/pshmod.h
  source/src/pshinter/pshnterr.h
  source/src/pshinter/pshrec.h
  source/src/psnames/psmodule.h
  source/src/psnames/psnamerr.h
  source/src/psnames/pstables.h
  source/src/raster/ftmisc.h
  source/src/raster/ftraster.h
  source/src/raster/ftrend1.h
  source/src/raster/rasterrs.h
  source/src/sfnt/pngshim.h
  source/src/sfnt/sfdriver.h
  source/src/sfnt/sferrors.h
  source/src/sfnt/sfobjs.h
  source/src/sfnt/sfwoff.h
  source/src/sfnt/sfwoff2.h
  source/src/sfnt/ttbdf.h
  source/src/sfnt/ttcmap.h
  source/src/sfnt/ttcmapc.h
  source/src/sfnt/ttcolr.h
  source/src/sfnt/ttcpal.h
  source/src/sfnt/ttkern.h
  source/src/sfnt/ttload.h
  source/src/sfnt/ttmtx.h
  source/src/sfnt/ttpost.h
  source/src/sfnt/ttsbit.h
  source/src/sfnt/woff2tags.h
  source/src/smooth/ftgrays.h
  source/src/smooth/ftsmerrs.h
  source/src/smooth/ftsmooth.h
  source/src/truetype/ttdriver.h
  source/src/truetype/tterrors.h
  source/src/truetype/ttgload.h
  source/src/truetype/ttgxvar.h
  source/src/truetype/ttinterp.h
  source/src/truetype/ttobjs.h
  source/src/truetype/ttpload.h
  source/src/truetype/ttsubpix.h
  source/src/type1/t1afm.h
  source/src/type1/t1driver.h
  source/src/type1/t1errors.h
  source/src/type1/t1gload.h
  source/src/type1/t1load.h
  source/src/type1/t1objs.h
  source/src/type1/t1parse.h
  source/src/type1/t1tokens.h
  source/src/type42/t42drivr.h
  source/src/type42/t42error.h
  source/src/type42/t42objs.h
  source/src/type42/t42parse.h
  source/src/type42/t42types.h
  source/src/winfonts/fnterrs.h
  source/src/winfonts/winfnt.h

  freetype2-version.h
)

if(UNIX)
  list(APPEND sources
    source/builds/unix/ftsystem.c)
else()
  list(APPEND sources
    source/src/base/ftsystem.c)
endif()


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
)

target_link_libraries(${freetype2_dll_name}
  PUBLIC
    ${utf8wrap_dll_name}
)

set_shared_library_version_properties(
  ${freetype2_dll_name}
  ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
  ${MIKTEX_COMP_INTERFACE_VERSION}
)

install(TARGETS ${freetype2_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
