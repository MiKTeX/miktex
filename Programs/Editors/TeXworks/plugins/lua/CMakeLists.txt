## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2010-2020 Christian Schenk
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

set(twluaplugin_sources
  ${projdir}/source/plugins-src/TWLuaPlugin/LuaScript.cpp
  ${projdir}/source/plugins-src/TWLuaPlugin/LuaScript.h
  ${projdir}/source/plugins-src/TWLuaPlugin/LuaScriptInterface.cpp
  ${projdir}/source/plugins-src/TWLuaPlugin/LuaScriptInterface.h
)

add_library(twluaplugin-static STATIC
  ${twluaplugin_sources}
)

set_property(TARGET twluaplugin-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(twluaplugin-static
  PUBLIC
    ${core_dll_name}
    ${lua53_dll_name}
    Qt5::Core
    Qt5::Gui
    Qt5::UiTools
    Qt5::Widgets
)
