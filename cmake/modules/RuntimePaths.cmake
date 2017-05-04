## RuntimePaths.cmake
##
## Copyright (C) 2006-2017 Christian Schenk
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

if(MIKTEX_WORDS_BIGENDIAN)
  set(MIKTEX_REL_MIKTEX_BASE_DIR "miktex/data/be")
  set(MIKTEX_REL_MIKTEX_FMT_DIR "miktex/data/be")
  set(MIKTEX_REL_MIKTEX_FNDB_DIR "miktex/data/be")
else()
  set(MIKTEX_REL_MIKTEX_BASE_DIR "miktex/data/le")
  set(MIKTEX_REL_MIKTEX_FMT_DIR "miktex/data/le")
  set(MIKTEX_REL_MIKTEX_FNDB_DIR "miktex/data/le")
endif()

if(MIKTEX_UNIX_ALIKE)
  set(MIKTEX_SYSTEM_VAR_CACHE_DIR "/var/cache" CACHE PATH "Directory for application cache data." FORCE)
  set(MIKTEX_SYSTEM_VAR_LIB_DIR "/var/lib" CACHE PATH "Directory for state information." FORCE)
endif()
