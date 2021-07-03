## ExecutableNames.cmake
##
## Copyright (C) 2018-2021 Christian Schenk
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

if(MIKTEX_NATIVE_WINDOWS)
  set(MIKTEX_EXECUTABLE_FILE_SUFFIX ".exe")
else()
  set(MIKTEX_EXECUTABLE_FILE_SUFFIX "")
endif()

macro(define_executable _name)
  if(${ARGC} GREATER 1)
    set(_prog_name ${ARGV1})
  else()
    set(_prog_name "${MIKTEX_PREFIX}${_name}")
  endif()
  string(TOUPPER "${_name}" _name_u)
  set(MIKTEX_PROG_NAME_${_name_u} "${_prog_name}")
  set(MIKTEX_${_name_u}_EXECUTABLE "${_prog_name}${MIKTEX_EXECUTABLE_FILE_SUFFIX}")
  set(MIKTEX_PROG_NAME_${_name_u}_ADMIN "${_prog_name}${MIKTEX_ADMIN_SUFFIX}")
  set(MIKTEX_${_name_u}_ADMIN_EXECUTABLE "${_prog_name}${MIKTEX_ADMIN_SUFFIX}${MIKTEX_EXECUTABLE_FILE_SUFFIX}")
  set(MIKTEX_PROG_NAME_${_name_u}_STANDALONE "${_prog_name}${MIKTEX_STANDALONE_SUFFIX}")
  set(MIKTEX_${_name_u}_STANDALONE_EXECUTABLE "${_prog_name}${MIKTEX_STANDALONE_SUFFIX}${MIKTEX_EXECUTABLE_FILE_SUFFIX}")
endmacro()

define_executable(arctrl arctrl)
define_executable(console)
define_executable(epstopdf)
define_executable(findtexmf findtexmf)
define_executable(gsf2pk gsf2pk)
define_executable(initexmf initexmf)
define_executable(makebase)
define_executable(makefmt)
define_executable(makemf)
define_executable(makepk)
define_executable(maketfm)
define_executable(miktex miktex)
define_executable(mkfntmap mkfntmap)
define_executable(mpm mpm)
define_executable(mthelp mthelp)
define_executable(mtprint mtprint)
define_executable(setup miktexsetup)
define_executable(synctex)
define_executable(tdsutil tdsutil)
define_executable(texify texify)
define_executable(yap yap)
