## MergeTrustInfoManifest.cmake
##
## Copyright (C) 2006-2016 Christian Schenk
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

macro(merge_trustinfo_manifest _target _level)
  if(MIKTEX_NATIVE_WINDOWS AND MSVC)
    add_link_flags(${_target} "/MANIFESTUAC:NO")
    add_custom_command(
      TARGET
        ${_target}
      POST_BUILD
        COMMAND
	  ${MT_EXECUTABLE} -nologo
	  -manifest ${CMAKE_SOURCE_DIR}/Resources/Manifests/${_level}.manifest
	  -updateresource:$<TARGET_FILE:${_target}>\;1
    )
  endif()
endmacro()
