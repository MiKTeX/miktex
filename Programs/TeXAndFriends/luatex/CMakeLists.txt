## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2010-2019 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_TEX_AND_FRIENDS_FOLDER}/LuaTeX")

# FIXME: this breaks luatex under xenial:
# set(lua53_target_name ${lua53_dll_name})
if(MIKTEX_NATIVE_WINDOWS)
  set(lua53_target_name ${lua53_dll_name})
else()
  set(lua53_target_name ${lua53_dll_name})
endif()

if(WITH_LUA54TEX)
  # FIXME: this breaks luatex under xenial:
  # set(lua54_target_name ${lua54_dll_name})
  if(MIKTEX_NATIVE_WINDOWS)
    set(lua54_target_name ${lua54_dll_name})
  else()
    set(lua54_target_name ${lua54_lib_name})
  endif()
endif()

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}
)

add_definitions(
  -DAPPTAG=luatex
  -DMIKTEX_LUATEX
  -DNO_DUMP_SHARE
  -DSYNCTEX_ENGINE_H="synctex-luatex.h"
  -DUSE_MIKTEX_EXIT
  -DpdfTeX
)

if(MIKTEX_UNIX_ALIKE)
  add_definitions(-DLUA_USE_POSIX)
endif()

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
    -DWIN32_LEAN_AND_MEAN
  )
endif()

if(WITH_CONTEXT_SUPPORT)
  add_definitions(-DWITH_CONTEXT_SUPPORT=1)
endif()

###############################################################################
## luatex-miktex-static
###############################################################################

set(miktex_sources
  miktex/luatex.h
  miktex/miktex.cpp
)

add_library(luatex-miktex-static STATIC ${miktex_sources})

set_property(TARGET luatex-miktex-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(luatex-miktex-static
  PUBLIC
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
)

###############################################################################
## luatex-luasocket-static
###############################################################################

set(luasocket_sources
  source/luasocket/src/auxiliar.c
  source/luasocket/src/auxiliar.h
  source/luasocket/src/buffer.c
  source/luasocket/src/buffer.h
  source/luasocket/src/compat.c
  source/luasocket/src/compat.h
  source/luasocket/src/except.c
  source/luasocket/src/except.h
  source/luasocket/src/inet.c
  source/luasocket/src/inet.h
  source/luasocket/src/io.c
  source/luasocket/src/io.h
  source/luasocket/src/lua_preload.c
  source/luasocket/src/luasocket.c
  source/luasocket/src/luasocket.h
  source/luasocket/src/mime.c
  source/luasocket/src/mime.h
  source/luasocket/src/options.c
  source/luasocket/src/options.h
  source/luasocket/src/select.c
  source/luasocket/src/select.h
  source/luasocket/src/serial.c
  source/luasocket/src/socket.c
  source/luasocket/src/socket.h
  source/luasocket/src/tcp.c
  source/luasocket/src/tcp.h
  source/luasocket/src/timeout.c
  source/luasocket/src/timeout.h
  source/luasocket/src/udp.c
  source/luasocket/src/udp.h
  source/luasocket/src/usocket.h
)

set(luasocket_lua_sources
  ftp.lua
  headers.lua
  http.lua
  ltn12.lua
  mbox.lua
  mime.lua
  smtp.lua
  socket.lua
  tp.lua
  url.lua
)

foreach(f ${luasocket_lua_sources})
  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/luasocket-${f}.h
    COMMAND
      bintoc B true
        < ${CMAKE_CURRENT_SOURCE_DIR}/source/luasocket/src/${f}
        > ${CMAKE_CURRENT_BINARY_DIR}/luasocket-${f}.h.tmp
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/luasocket-${f}.h.tmp
        ${CMAKE_CURRENT_BINARY_DIR}/luasocket-${f}.h
    COMMAND
      ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/luasocket-${f}.h.tmp
    DEPENDS
      bintoc
      ${CMAKE_CURRENT_SOURCE_DIR}/source/luasocket/src/${f}
    VERBATIM
  )
  list(APPEND generated_luasocket_sources ${CMAKE_CURRENT_BINARY_DIR}/luasocket-${f}.h)
endforeach()

add_custom_target(gen-luasocket-sources DEPENDS
  ${generated_luasocket_sources}
)

list(APPEND luasocket_sources ${generated_luasocket_sources})

add_library(luatex-lua53socket-static STATIC ${luasocket_sources})
set_property(TARGET luatex-lua53socket-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
add_dependencies(luatex-lua53socket-static gen-luasocket-sources)
set_source_files_properties(
  ${generated_luasocket_sources}
  PROPERTIES
    GENERATED TRUE
)
target_compile_definitions(luatex-lua53socket-static
  PRIVATE
    -DLUASOCKET_DEBUG
)
target_link_libraries(luatex-lua53socket-static
  PUBLIC
    ${lua53_target_name}
)

if(WITH_LUA54TEX)
  add_library(luatex-lua54socket-static STATIC ${luasocket_sources})
  set_property(TARGET luatex-lua54socket-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  add_dependencies(luatex-lua54socket-static gen-luasocket-sources)
  target_compile_definitions(luatex-lua54socket-static
    PRIVATE
      -DLUASOCKET_DEBUG
  )
  target_link_libraries(luatex-lua54socket-static
    PUBLIC
      ${lua54_target_name}
  )
endif()

###############################################################################
## luatex-luamisc-static
###############################################################################

set(luamisc_sources
  source/luafilesystem/src/lfs.c
  source/luafilesystem/src/lfs.h
  source/luamd5/luamd5.h
  source/luamd5/md5.c
  source/luamd5/md5lib.c
  source/luapeg/lpeg.c
  source/luapeg/lpeg.h
  source/luazip/src/luazip.c
  source/luazip/src/luazip.h
  source/luazlib/lgzip.c
  source/luazlib/lzlib.c
  source/slnunicode/slnunico.c
)

add_library(luatex-lua53misc-static STATIC ${luamisc_sources})
set_property(TARGET luatex-lua53misc-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
if(USE_SYSTEM_ZLIB)
  target_link_libraries(luatex-lua53misc-static PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(luatex-lua53misc-static PUBLIC ${zlib_dll_name})
endif()
if(USE_SYSTEM_ZZIP)
  target_link_libraries(luatex-lua53misc-static PUBLIC MiKTeX::Imported::ZZIP)
else()
  target_link_libraries(luatex-lua53misc-static PUBLIC ${zzip_dll_name})
endif()
target_link_libraries(luatex-lua53misc-static
  PUBLIC
    ${core_dll_name}
    ${lua53_target_name}
)
if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(luatex-lua53misc-static
    PUBLIC
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()

if(WITH_LUA54TEX)
  add_library(luatex-lua54misc-static STATIC ${luamisc_sources})
  set_property(TARGET luatex-lua54misc-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  if(USE_SYSTEM_ZLIB)
    target_link_libraries(luatex-lua54misc-static PUBLIC MiKTeX::Imported::ZLIB)
  else()
    target_link_libraries(luatex-lua54misc-static PUBLIC ${zlib_dll_name})
  endif()
  if(USE_SYSTEM_ZZIP)
    target_link_libraries(luatex-lua54misc-static PUBLIC MiKTeX::Imported::ZZIP)
  else()
    target_link_libraries(luatex-lua54misc-static PUBLIC ${zzip_dll_name})
  endif()
  target_link_libraries(luatex-lua54misc-static
    PUBLIC
      ${core_dll_name}
      ${lua54_target_name}
  )
  if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(luatex-lua54misc-static
      PUBLIC
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
  endif()
endif()

###############################################################################
## luatex-luapplib-static
###############################################################################

set(luapplib_sources
  source/luapplib/ppapi.h
  source/luapplib/pparray.c
  source/luapplib/pparray.h
  source/luapplib/ppconf.h
  source/luapplib/ppcrypt.c
  source/luapplib/ppcrypt.h
  source/luapplib/ppdict.c
  source/luapplib/ppdict.h
  source/luapplib/ppfilter.h
  source/luapplib/ppheap.c
  source/luapplib/ppheap.h
  source/luapplib/pplib.h
  source/luapplib/ppload.c
  source/luapplib/ppload.h
  source/luapplib/ppstream.c
  source/luapplib/ppstream.h
  source/luapplib/ppxref.c
  source/luapplib/ppxref.h
  source/luapplib/util/utilarm.h
  source/luapplib/util/utilbasexx.c
  source/luapplib/util/utilbasexx.h
  source/luapplib/util/utilcrypt.c
  source/luapplib/util/utilcrypt.h
  source/luapplib/util/utilcryptdef.h
  source/luapplib/util/utildecl.h
  source/luapplib/util/utilflate.c
  source/luapplib/util/utilflate.h
  source/luapplib/util/utilfpred.c
  source/luapplib/util/utilfpred.h
  source/luapplib/util/utiliof.c
  source/luapplib/util/utiliof.h
  source/luapplib/util/utillog.c
  source/luapplib/util/utillog.h
  source/luapplib/util/utillzw.c
  source/luapplib/util/utillzw.h
  source/luapplib/util/utilmd5.c
  source/luapplib/util/utilmd5.h
  source/luapplib/util/utilmem.c
  source/luapplib/util/utilmem.h
  source/luapplib/util/utilnumber.c
  source/luapplib/util/utilnumber.h
  source/luapplib/util/utilplat.h
  source/luapplib/util/utilsha.c
  source/luapplib/util/utilsha.h
)

add_library(luatex-lua53pplib-static STATIC ${luapplib_sources})
set_property(TARGET luatex-lua53pplib-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_include_directories(luatex-lua53pplib-static
  PUBLIC
    source/luapplib/util
    source/luapplib/zlib
)
if(USE_SYSTEM_ZLIB)
  target_link_libraries(luatex-lua53pplib-static PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(luatex-lua53pplib-static PUBLIC ${zlib_dll_name})
endif()
target_link_libraries(luatex-lua53pplib-static
  PUBLIC
    ${core_dll_name}
    ${lua53_target_name}
)
if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(luatex-lua53pplib-static
    PUBLIC
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()

if(WITH_LUA54TEX)
  add_library(luatex-lua54pplib-static STATIC ${luapplib_sources})
  set_property(TARGET luatex-lua54pplib-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_include_directories(luatex-lua54pplib-static
    PUBLIC
      source/luapplib/util
      source/luapplib/zlib
  )
  if(USE_SYSTEM_ZLIB)
    target_link_libraries(luatex-lua54pplib-static PUBLIC MiKTeX::Imported::ZLIB)
  else()
    target_link_libraries(luatex-lua54pplib-static PUBLIC ${zlib_dll_name})
  endif()
  target_link_libraries(luatex-lua54pplib-static
    PUBLIC
      ${core_dll_name}
      ${lua54_target_name}
  )
  if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(luatex-lua54pplib-static
      PUBLIC
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
  endif()
endif()

###############################################################################
## luatex-unilib-static
###############################################################################

set(unilib_sources
  source/unilib/ArabicForms.c
  source/unilib/alphabet.c
  source/unilib/char.c
  source/unilib/chardata.h
  source/unilib/charset.h
  source/unilib/cjk.c
  source/unilib/combiners.h
  source/unilib/gwwiconv.c
  source/unilib/gwwiconv.h
  source/unilib/ucharmap.c
  source/unilib/unialt.c
  source/unilib/unibasics.h
  source/unilib/usprintf.c
  source/unilib/ustring.c
  source/unilib/ustring.h
  source/unilib/utype.c
  source/unilib/utype.h
)

add_library(luatex-unilib-static STATIC ${unilib_sources})

set_property(TARGET luatex-unilib-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(luatex-unilib-static
  PUBLIC
    source/unilib
)

target_link_libraries(luatex-unilib-static
  PUBLIC
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

###############################################################################
## luatex-luafontforge-static
###############################################################################

configure_file(
  ff-config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/ff-config.h
)

set(luafontforge_sources
  source/luafontloader/fontforge/fontforge/PfEd.h
  source/luafontloader/fontforge/fontforge/autohint.c
  source/luafontloader/fontforge/fontforge/baseviews.h
  source/luafontloader/fontforge/fontforge/clipnoui.c
  source/luafontloader/fontforge/fontforge/configure-pfaedit.h
  source/luafontloader/fontforge/fontforge/cvundoes.c
  source/luafontloader/fontforge/fontforge/dumppfa.c
  source/luafontloader/fontforge/fontforge/edgelist.h
  source/luafontloader/fontforge/fontforge/edgelist2.h
  source/luafontloader/fontforge/fontforge/encoding.c
  source/luafontloader/fontforge/fontforge/encoding.h
  source/luafontloader/fontforge/fontforge/featurefile.c
  source/luafontloader/fontforge/fontforge/fontforgevw.h
  source/luafontloader/fontforge/fontforge/fontviewbase.c
  source/luafontloader/fontforge/fontforge/fvcomposit.c
  source/luafontloader/fontforge/fontforge/fvfonts.c
  source/luafontloader/fontforge/fontforge/lookups.c
  source/luafontloader/fontforge/fontforge/lookups.h
  source/luafontloader/fontforge/fontforge/macbinary.c
  source/luafontloader/fontforge/fontforge/macenc.c
  source/luafontloader/fontforge/fontforge/mathconstants.c
  source/luafontloader/fontforge/fontforge/memory.c
  source/luafontloader/fontforge/fontforge/mm.c
  source/luafontloader/fontforge/fontforge/mm.h
  source/luafontloader/fontforge/fontforge/namelist.c
  source/luafontloader/fontforge/fontforge/noprefs.c
  source/luafontloader/fontforge/fontforge/nouiutil.c
  source/luafontloader/fontforge/fontforge/parsepfa.c
  source/luafontloader/fontforge/fontforge/parsettf.c
  source/luafontloader/fontforge/fontforge/parsettfatt.c
  source/luafontloader/fontforge/fontforge/pfaedit.h
  source/luafontloader/fontforge/fontforge/plugins.h
  source/luafontloader/fontforge/fontforge/psfont.h
  source/luafontloader/fontforge/fontforge/psread.c
  source/luafontloader/fontforge/fontforge/pua.c
  source/luafontloader/fontforge/fontforge/python.c
  source/luafontloader/fontforge/fontforge/scripting.h
  source/luafontloader/fontforge/fontforge/sd.h
  source/luafontloader/fontforge/fontforge/sfd1.c
  source/luafontloader/fontforge/fontforge/sfd1.h
  source/luafontloader/fontforge/fontforge/splinechar.c
  source/luafontloader/fontforge/fontforge/splinefill.c
  source/luafontloader/fontforge/fontforge/splinefont.c
  source/luafontloader/fontforge/fontforge/splinefont.h
  source/luafontloader/fontforge/fontforge/splineorder2.c
  source/luafontloader/fontforge/fontforge/splineoverlap.c
  source/luafontloader/fontforge/fontforge/splinerefigure.c
  source/luafontloader/fontforge/fontforge/splinesave.c
  source/luafontloader/fontforge/fontforge/splinesaveafm.c
  source/luafontloader/fontforge/fontforge/splinestroke.c
  source/luafontloader/fontforge/fontforge/splineutil.c
  source/luafontloader/fontforge/fontforge/splineutil2.c
  source/luafontloader/fontforge/fontforge/start.c
  source/luafontloader/fontforge/fontforge/stemdb.c
  source/luafontloader/fontforge/fontforge/stemdb.h
  source/luafontloader/fontforge/fontforge/tottf.c
  source/luafontloader/fontforge/fontforge/tottfgpos.c
  source/luafontloader/fontforge/fontforge/ttf.h
  source/luafontloader/fontforge/fontforge/ttfspecial.c
  source/luafontloader/fontforge/fontforge/uiinterface.h
  source/luafontloader/fontforge/fontforge/unicoderange.h
  source/luafontloader/fontforge/gutils/fsys.c
  source/luafontloader/fontforge/inc/basics.h
  source/luafontloader/fontforge/inc/gfile.h
  source/luafontloader/fontforge/inc/gimage.h
  source/luafontloader/fontforge/inc/intl.h
  source/luafontloader/src/ffdummies.c
  source/luafontloader/src/ffdummies.h
  source/luafontloader/src/luafflib.c
)

add_library(luatex-lua53fontforge-static STATIC ${luafontforge_sources})
prevent_all_warnings_on_bad_code_target(luatex-lua53fontforge-static)
set_property(TARGET luatex-lua53fontforge-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_compile_definitions(luatex-lua53fontforge-static
  PRIVATE
    -DLUA_FF_LIB=1
    -DUSE_OUR_MEMORY=1
    -DX_DISPLAY_MISSING=1
    -D_NO_PYTHON=1
)
target_include_directories(luatex-lua53fontforge-static
  PRIVATE
    source/luafontloader/fontforge/fontforge
    source/luafontloader/fontforge/inc
)
target_link_libraries(luatex-lua53fontforge-static
  PUBLIC
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${lua53_target_name}
    ${w2cemu_dll_name}
    luatex-unilib-static
)
if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(luatex-lua53fontforge-static
    PUBLIC
      ${utf8wrap_dll_name}
  )
endif()

if(WITH_LUA54TEX)
  add_library(luatex-lua54fontforge-static STATIC ${luafontforge_sources})
  set_property(TARGET luatex-lua54fontforge-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_compile_definitions(luatex-lua54fontforge-static
    PRIVATE
      -DLUA_FF_LIB=1
      -DUSE_OUR_MEMORY=1
      -DX_DISPLAY_MISSING=1
      -D_NO_PYTHON=1
  )
  target_include_directories(luatex-lua54fontforge-static
    PRIVATE
      source/luafontloader/fontforge/fontforge
      source/luafontloader/fontforge/inc
  )
  target_link_libraries(luatex-lua54fontforge-static
    PUBLIC
      ${core_dll_name}
      ${kpsemu_dll_name}
      ${lua54_target_name}
      ${w2cemu_dll_name}
      luatex-unilib-static
  )
  if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(luatex-lua54fontforge-static
      PUBLIC
        ${utf8wrap_dll_name}
    )
  endif()
endif()

###############################################################################
## luatex-luaffi-static
###############################################################################

set(luaffi_sources
  source/luaffi/call.c
  source/luaffi/call_arm.h
  source/luaffi/call_x64.h
  source/luaffi/call_x64win.h
  source/luaffi/call_x86.h
  source/luaffi/ctype.c
  source/luaffi/ffi.c
  source/luaffi/ffi.h
  source/luaffi/parser.c
) 

add_library(luatex-lua53ffi-static STATIC ${luaffi_sources})
set_property(TARGET luatex-lua53ffi-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_include_directories(luatex-lua53ffi-static
  PRIVATE
    source/luaffi/dynasm
)
target_link_libraries(luatex-lua53ffi-static
  PUBLIC
    ${lua53_target_name}
)
if(USE_SYSTEM_ZLIB)
  target_link_libraries(luatex-lua53ffi-static PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(luatex-lua53ffi-static PUBLIC ${zlib_dll_name})
endif()

if(WITH_LUA54TEX)
  add_library(luatex-lua54ffi-static STATIC ${luaffi_sources})
  set_property(TARGET luatex-lua54ffi-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_include_directories(luatex-lua54ffi-static
    PRIVATE
      source/luaffi/dynasm
  )
  target_link_libraries(luatex-lua54ffi-static
    PUBLIC
      ${lua54_target_name}
  )
  if(USE_SYSTEM_ZLIB)
    target_link_libraries(luatex-lua54ffi-static PUBLIC MiKTeX::Imported::ZLIB)
  else()
    target_link_libraries(luatex-lua54ffi-static PUBLIC ${zlib_dll_name})
  endif()
endif()

###############################################################################
## luatex-static
###############################################################################

set(luatexlib_sources
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_MP_DIR}/source/lmplib.c
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-common.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex-luatex.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex.c
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}/synctex.h
  source/dvi/dvigen.c
  source/dvi/dvigen.h
  source/font/dofont.c
  source/font/luafont.c
  source/font/luatexfont.h
  source/font/mapfile.c
  source/font/mapfile.h
  source/font/pkin.c
  source/font/sfnt.c
  source/font/sfnt.h
  source/font/texfont.c
  source/font/texfont.h
  source/font/tfmofm.c
  source/font/tounicode.c
  source/font/tt_glyf.c
  source/font/tt_glyf.h
  source/font/tt_table.c
  source/font/tt_table.h
  source/font/vfovf.c
  source/font/vfpacket.c
  source/font/writecff.c
  source/font/writecff.h
  source/font/writeenc.c
  source/font/writefont.c
  source/font/writet1.c
  source/font/writet3.c
  source/font/writettf.c
  source/font/writettf.h
  source/font/writetype0.c
  source/font/writetype2.c 
  source/image/epdf.h
  source/image/image.h
  source/image/pdftoepdf.c
  source/image/pdftoepdf.h
  source/image/writeimg.c
  source/image/writeimg.h
  source/image/writejbig2.c
  source/image/writejbig2.h
  source/image/writejp2.c
  source/image/writejp2.h
  source/image/writejpg.c
  source/image/writejpg.h
  source/image/writepng.c
  source/image/writepng.h
  source/lang/hnjalloc.c
  source/lang/hnjalloc.h
  source/lang/hyphen.c
  source/lang/hyphen.h
  source/lang/texlang.c
  source/lang/texlang.h
  source/lua/lstrlibext.c
  source/lua/helpers.c
  source/lua/texluac.c 
  source/lua/lcallbacklib.c
  source/lua/lfontlib.c
  source/lua/limglib.c
  source/lua/lpdfelib.c
  source/lua/lpdfscannerlib.c
  source/lua/lkpselib.c
  source/lua/llanglib.c
  source/lua/llualib.c
  source/lua/lnodelib.c
  source/lua/liolibext.c
  source/lua/loslibext.c
  source/lua/lpdflib.c
  source/lua/lstatslib.c
  source/lua/ltexiolib.c
  source/lua/ltexlib.c
  source/lua/lnewtokenlib.c
  source/lua/luatex-api.h
  source/lua/luatex-core.c
  source/lua/helpers.c
  source/lua/luainit.c
  source/lua/luanode.c
  source/lua/luastuff.c
  source/lua/luatoken.c
  source/lua/mplibstuff.c 
  source/luatex.h
  source/pdf/pdfaction.c
  source/pdf/pdfaction.h
  source/pdf/pdfannot.c
  source/pdf/pdfannot.h
  source/pdf/pdfcolorstack.c
  source/pdf/pdfcolorstack.h
  source/pdf/pdfdest.c
  source/pdf/pdfdest.h
  source/pdf/pdffont.c
  source/pdf/pdffont.h
  source/pdf/pdfgen.c
  source/pdf/pdfgen.h
  source/pdf/pdfglyph.c
  source/pdf/pdfglyph.h
  source/pdf/pdfimage.c
  source/pdf/pdfimage.h
  source/pdf/pdflink.c
  source/pdf/pdflink.h
  source/pdf/pdflistout.c
  source/pdf/pdflistout.h
  source/pdf/pdfliteral.c
  source/pdf/pdfliteral.h
  source/pdf/pdfobj.c
  source/pdf/pdfobj.h
  source/pdf/pdfoutline.c
  source/pdf/pdfoutline.h
  source/pdf/pdfpage.c
  source/pdf/pdfpage.h
  source/pdf/pdfpagetree.c
  source/pdf/pdfpagetree.h
  source/pdf/pdfrule.c
  source/pdf/pdfrule.h
  source/pdf/pdfsaverestore.c
  source/pdf/pdfsaverestore.h
  source/pdf/pdfsetmatrix.c
  source/pdf/pdfsetmatrix.h
  source/pdf/pdfshipout.c
  source/pdf/pdfshipout.h
  source/pdf/pdftables.c
  source/pdf/pdftables.h
  source/pdf/pdfthread.c
  source/pdf/pdfthread.h
  source/pdf/pdftypes.h
  source/pdf/pdfxform.c
  source/pdf/pdfxform.h
  source/tex/align.c
  source/tex/align.h
  source/tex/arithmetic.c
  source/tex/arithmetic.h
  source/tex/backend.c
  source/tex/backend.h
  source/tex/buildpage.c
  source/tex/buildpage.h
  source/tex/commands.c
  source/tex/commands.h
  source/tex/conditional.c
  source/tex/conditional.h
  source/tex/directions.c
  source/tex/directions.h
  source/tex/dumpdata.c
  source/tex/dumpdata.h
  source/tex/equivalents.c
  source/tex/equivalents.h
  source/tex/errors.c
  source/tex/errors.h
  source/tex/expand.c
  source/tex/expand.h
  source/tex/extensions.c
  source/tex/extensions.h
  source/tex/filename.c
  source/tex/filename.h
  source/tex/inputstack.c
  source/tex/inputstack.h
  source/tex/linebreak.c
  source/tex/linebreak.h
  source/tex/mainbody.c
  source/tex/mainbody.h
  source/tex/maincontrol.c
  source/tex/maincontrol.h
  source/tex/mathcodes.c
  source/tex/mathcodes.h
  source/tex/memoryword.c
  source/tex/memoryword.h
  source/tex/mlist.c
  source/tex/mlist.h
  source/tex/nesting.c
  source/tex/nesting.h
  source/tex/packaging.c
  source/tex/packaging.h
  source/tex/postlinebreak.c
  source/tex/postlinebreak.h
  source/tex/primitive.c
  source/tex/primitive.h
  source/tex/printing.c
  source/tex/printing.h
  source/tex/scanning.c
  source/tex/scanning.h
  source/tex/stringpool.c
  source/tex/stringpool.h
  source/tex/texdeffont.c
  source/tex/texdeffont.h
  source/tex/texfileio.c
  source/tex/texfileio.h
  source/tex/texmath.c
  source/tex/texmath.h
  source/tex/texnodes.c
  source/tex/texnodes.h
  source/tex/textcodes.c
  source/tex/textcodes.h
  source/tex/textoken.c
  source/tex/textoken.h
  source/utils/avl.c
  source/utils/avl.h
  source/utils/avlstuff.c
  source/utils/avlstuff.h
  source/utils/managed-sa.c
  source/utils/managed-sa.h
  source/utils/unistring.c
  source/utils/unistring.h
  source/utils/utils.c
  source/utils/utils.h
)

add_library(lua53tex-static STATIC ${luatexlib_sources})
set_property(TARGET lua53tex-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_compile_definitions(lua53tex-static
  PRIVATE
    -DLUA_FF_LIB=1
)
if(USE_SYSTEM_PNG)
  target_link_libraries(lua53tex-static PUBLIC MiKTeX::Imported::PNG)
else()
  target_link_libraries(lua53tex-static PUBLIC ${png_dll_name})
endif()
if(USE_SYSTEM_POPPLER)
  target_link_libraries(lua53tex-static PUBLIC MiKTeX::Imported::POPPLER)
else()
  target_link_libraries(lua53tex-static PUBLIC ${poppler_dll_name})
endif()
if(USE_SYSTEM_ZLIB)
  target_link_libraries(lua53tex-static PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(lua53tex-static PUBLIC ${zlib_dll_name})
endif()
target_link_libraries(lua53tex-static
  PUBLIC
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${lua53_target_name}
    ${metapost_dll_name}
    ${w2cemu_dll_name}
    luatex-miktex-static
    luatex-lua53ffi-static
    luatex-lua53misc-static
    luatex-lua53pplib-static
    luatex-lua53socket-static
    luatex-lua53fontforge-static
)

if(WITH_LUA54TEX)
  add_library(lua54tex-static STATIC ${luatexlib_sources})
  set_property(TARGET lua54tex-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_compile_definitions(lua54tex-static
    PRIVATE
      -DLUA_FF_LIB=1
  )
  if(USE_SYSTEM_PNG)
    target_link_libraries(lua54tex-static PUBLIC MiKTeX::Imported::PNG)
  else()
    target_link_libraries(lua54tex-static PUBLIC ${png_dll_name})
  endif()
  if(USE_SYSTEM_POPPLER)
    target_link_libraries(lua54tex-static PUBLIC MiKTeX::Imported::POPPLER)
  else()
    target_link_libraries(lua54tex-static PUBLIC ${poppler_dll_name})
  endif()
  if(USE_SYSTEM_ZLIB)
    target_link_libraries(lua54tex-static PUBLIC MiKTeX::Imported::ZLIB)
  else()
    target_link_libraries(lua54tex-static PUBLIC ${zlib_dll_name})
  endif()
  target_link_libraries(lua54tex-static
    PUBLIC
      ${core_dll_name}
      ${kpsemu_dll_name}
      ${lua54_target_name}
      ${metapost_dll_name}
      ${w2cemu_dll_name}
      luatex-miktex-static
      luatex-lua54ffi-static
      luatex-lua54misc-static
      luatex-lua54pplib-static
      luatex-lua54socket-static
      luatex-lua54fontforge-static
  )
endif()

###############################################################################
## luatex-program
###############################################################################

set(luatex_program_sources
  source/luatex.c
  source/luatex.h
  source/luatex_svnversion.h
  luatex-version.h
)

set(lua53tex_program_sources ${luatex_program_sources})
if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/luatex.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/luatex.rc
    )
  list(APPEND lua53tex_program_sources
    ${CMAKE_CURRENT_BINARY_DIR}/luatex.rc
    ${MIKTEX_COMPATIBILITY_MANIFEST}
  )
endif()

set(libs
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${md5_dll_name}
  ${w2cemu_dll_name}

  luatex-unilib-static

  luatex-miktex-static
)

if(USE_SYSTEM_PNG)
  list(APPEND libs MiKTeX::Imported::PNG)
else()
  list(APPEND libs ${png_dll_name})
endif()

if(USE_SYSTEM_POPPLER)
  list(APPEND libs MiKTeX::Imported::POPPLER)
else()
  list(APPEND libs ${poppler_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
  list(APPEND libs MiKTeX::Imported::ZLIB)
else()
  list(APPEND libs ${zlib_dll_name})
endif()

if(USE_SYSTEM_ZZIP)
  list(APPEND libs MiKTeX::Imported::ZZIP)
else()
  list(APPEND libs ${zzip_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND libs
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32.lib
    wsock32.lib
  )
endif()

set(libs53 ${libs})
list(APPEND libs53
  ${lua53_target_name}
  luatex-lua53fontforge-static
  luatex-lua53ffi-static
  luatex-lua53misc-static
  luatex-lua53pplib-static
  luatex-lua53socket-static
  lua53tex-static
)

if(WITH_LUA54TEX)
  set(lua54tex_program_sources ${luatex_program_sources})
  if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/lua54tex.rc.in
      ${CMAKE_CURRENT_BINARY_DIR}/lua54tex.rc
    )
    list(APPEND lua54tex_program_sources
      ${CMAKE_CURRENT_BINARY_DIR}/lua54tex.rc
      ${MIKTEX_COMPATIBILITY_MANIFEST}
    )
  endif()
endif()

if(WITH_LUA54TEX)
  set(libs54 ${libs})
  list(APPEND libs54
    ${lua54_target_name}
    luatex-lua54fontforge-static
    luatex-lua54ffi-static
    luatex-lua54misc-static
    luatex-lua54pplib-static
    luatex-lua54socket-static
    lua54tex-static
  )
endif()

set(program_libs53 ${libs53})

if(WITH_LUA54TEX)
  set(program_libs54 ${libs54})
endif()

###############################################################################
## luatex
###############################################################################

add_executable(${MIKTEX_PREFIX}luatex ${lua53tex_program_sources} ${MIKTEX_LIBRARY_WRAPPER})
set_property(TARGET ${MIKTEX_PREFIX}luatex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(${MIKTEX_PREFIX}luatex
  ${app_dll_name}
  ${core_dll_name}
  ${program_libs53}
)
install(TARGETS ${MIKTEX_PREFIX}luatex DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}")

if(WITH_LUA54TEX)
  add_executable(${MIKTEX_PREFIX}lua54tex ${lua54tex_program_sources} ${MIKTEX_LIBRARY_WRAPPER})
  set_property(TARGET ${MIKTEX_PREFIX}lua54tex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_link_libraries(${MIKTEX_PREFIX}lua54tex
    ${app_dll_name}
    ${core_dll_name}
    ${program_libs54}
  )
  install(TARGETS ${MIKTEX_PREFIX}lua54tex DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}")
endif()

###############################################################################
## runtexlua
###############################################################################

add_executable(runtexlua ${lua53tex_program_sources} runtexlua.cpp)
set_property(TARGET runtexlua PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
if(MIKTEX_MACOS_BUNDLE)
  set_target_properties(runtexlua PROPERTIES INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif(APPLE)
  set_target_properties(runtexlua PROPERTIES INSTALL_RPATH "@loader_path/../../${CMAKE_INSTALL_LIBDIR}")
else()
  set_target_properties(runtexlua PROPERTIES INSTALL_RPATH "$ORIGIN/../../${CMAKE_INSTALL_LIBDIR}")
endif()
target_link_libraries(runtexlua
  ${app_dll_name}
  ${core_dll_name}
  ${program_libs53}
)
install(TARGETS runtexlua DESTINATION "${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}")

if(WITH_LUA54TEX)
  add_executable(runtexlua54 ${lua54tex_program_sources} runtexlua.cpp)
  set_property(TARGET runtexlua54 PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  if(MIKTEX_MACOS_BUNDLE)
    set_target_properties(runtexlua54 PROPERTIES INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
  elseif(APPLE)
    set_target_properties(runtexlua54 PROPERTIES INSTALL_RPATH "@loader_path/../../${CMAKE_INSTALL_LIBDIR}")
  else()
    set_target_properties(runtexlua54 PROPERTIES INSTALL_RPATH "$ORIGIN/../../${CMAKE_INSTALL_LIBDIR}")
  endif()
  target_link_libraries(runtexlua54
    ${app_dll_name}
    ${core_dll_name}
    ${program_libs54}
  )
  install(TARGETS runtexlua54 DESTINATION "${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}")
endif()

###############################################################################
## runmtxrun
###############################################################################

if(WITH_CONTEXT_SUPPORT)
  add_executable(runmtxrun ${luatex_program_sources} runmtxrun.cpp)
  set_property(TARGET runmtxrun PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_link_libraries(runmtxrun
    ${app_dll_name}
    ${core_dll_name}
    ${program_libs}
  )
  install(TARGETS runmtxrun DESTINATION "${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}")
endif()

###############################################################################
## run tests
###############################################################################

add_subdirectory(test)
