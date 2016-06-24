## CMakeLists.txt					-*- CMake -*-
##
## Copyright (C) 2010-2016 Christian Schenk
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

set(tests 1 2)

foreach(t ${tests})
  add_executable(core_fndb_test${t} ${t}.cpp ${test_sources})
  set_property(TARGET core_fndb_test${t} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  merge_trustinfo_manifest(core_fndb_test${t} asInvoker)
  merge_compatibility_manifest(core_fndb_test${t})
  target_link_libraries(core_fndb_test${t}
    ${core_dll_name}
    ${log4cxx_dll_name}
    Threads::Threads
    miktex-popt-wrapper
  )
  add_test(
    NAME core_fndb_test${t}
    COMMAND $<TARGET_FILE:core_fndb_test${t}>
    	--data-root=${dataroot}
	--install-root=${installroot}
  )
endforeach(t)
