## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2017-2019 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_ADMIN_FOLDER}/Config")

if(MIKTEX_CORE_AUTO_ADMIN)
  set(Core_AutoAdmin "t")
else()
  set(Core_AutoAdmin "?")
endif()

if(MIKTEX_MPM_AUTO_ADMIN)
  set(MPM_AutoAdmin "t")
else()
  set(MPM_AutoAdmin "?")
endif()

if(MIKTEX_MPM_AUTO_INSTALL)
  set(MPM_AutoInstall "t")
else()
  set(MPM_AutoInstall "?")
endif()

configure_file(
  miktex-config.ini.in
  ${CMAKE_CURRENT_BINARY_DIR}/miktex-config.ini
  NEWLINE_STYLE UNIX
)
