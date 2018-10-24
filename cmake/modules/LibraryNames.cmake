## LibraryNames.cmake
##
## Copyright (C) 2006-2018 Christian Schenk
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

## shared library names; naming convention:
##    MiKTeXMN-NAME[-REV]
## where:
##    NAME is the component name
##    MN is the MiKTeX major/minor version (as an integer)
##    REV is the DLL revision (relative to the current major/minor version)

set(miktex_dll_prefix MiKTeX${MIKTEX_MAJOR_MINOR_INT})

if(MIKTEX_RELEASE_STATE EQUAL 0)
  set(miktex_dll_prefix MiKTeX${MIKTEX_MAJOR_MINOR_INT})
elseif(MIKTEX_RELEASE_STATE EQUAL 1)
  set(miktex_dll_prefix MiKTeX${MIKTEX_MAJOR_MINOR_INT}-next)
elseif(MIKTEX_RELEASE_STATE EQUAL 4)
  set(miktex_dll_prefix MiKTeX${MIKTEX_MAJOR_MINOR_INT}-debug)
endif()

macro(define_library _name)
  set(_lib_name ${_name})
  if(${ARGC} GREATER 1)
    set(_var_name_prefix ${ARGV1})
  else()
    set(_var_name_prefix ${_lib_name})
  endif()
  if(NOT LINK_EVERYTHING_STATICALLY)
    set(${_var_name_prefix}_dll_name "${miktex_dll_prefix}-${_lib_name}")
  endif()
  set(${_var_name_prefix}_lib_name "${_lib_name}-static")
endmacro()

define_library(app)
define_library(apr)
define_library(aprutil apr_util)
define_library(bzip2)
define_library(cairo)
define_library(core)
define_library(curl)
define_library(dib)
define_library(dvi)
define_library(egl_registry)
define_library(expat)
define_library(extractor)
define_library(fmt)
define_library(fontconfig)
define_library(freeglut)
define_library(freetype2)
define_library(fribidi)
define_library(gc)
define_library(gd)
define_library(getopt)
define_library(gmp)
define_library(graphite2)
define_library(harfbuzz)
define_library(hunspell)
define_library(jpeg)
define_library(kpathsea kpsemu)
define_library(libatomic_ops)
define_library("libressl-crypto" libressl_crypto)
define_library("libressl-ssl" libressl_ssl)
define_library(log4cxx)
define_library(lua52)
define_library(lua53)
define_library(luajit)
define_library(lzma2 lzma)
define_library(md5)
define_library(mkfntmap)
define_library(metapost)
define_library(mpfr)
define_library(mspack)
define_library(nlohmann_json)
define_library(opengl_registry)
define_library(packagemanager mpm)
define_library(pbm)
define_library(pgm)
define_library(pixman)
define_library(png)
define_library(pnm)
define_library(poppler)
define_library("poppler-Qt" poppler_qt)
define_library(popt)
define_library(ppm)
define_library(psres)
define_library(regex)
define_library(setup)
define_library(teckit)
define_library(tex)
define_library(texmf)
define_library(trace)
define_library("ui-Qt" ui_qt)
define_library(unxemu)
define_library(uriparser)
define_library(utf8wrap)
define_library(util)
define_library(web2c w2cemu)
define_library(zlib)
define_library(zzip)

if(MIKTEX_NATIVE_WINDOWS)
  set(core_ps_dll_name  "${core_dll_name}-PS")
  set(mpm_ps_dll_name   "${mpm_dll_name}-PS")
  set(mpm_tlb_name      "${miktex_dll_prefix}-packageManager")
  set(session_tlb_name  "${miktex_dll_prefix}-session")
endif()
