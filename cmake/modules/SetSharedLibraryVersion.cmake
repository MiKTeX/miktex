## SetSharedLibraryVersion.cmake
##
## Copyright (C) 2017-2020 Christian Schenk
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

macro(set_shared_library_version_properties _target _version _soversion)
  set_target_properties(${_target}
    PROPERTIES
      VERSION "${_version}"
      SOVERSION "${_soversion}"
)
endmacro()

macro(set_shared_library_version _target _soversion _j2000)
  math(EXPR b "${_j2000} / 256")
  math(EXPR c "${_j2000} % 256")
  set_target_properties(${_target}
    PROPERTIES
      VERSION "${_soversion}.${b}.${c}"
      SOVERSION "${_soversion}"
)
endmacro()
