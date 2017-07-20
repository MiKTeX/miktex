## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2010-2017 Christian Schenk
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

set(tests
  1
  2
  3
  4
)

set(exes
  1-1
  1-2
  3-1
)

foreach(t ${tests})
  add_executable(core_process_test${t} ${t}.cpp ${test_sources})
  set_property(TARGET core_process_test${t} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  merge_trustinfo_manifest(core_process_test${t} asInvoker)
  merge_compatibility_manifest(core_process_test${t})
  if(USE_SYSTEM_LOG4CXX)
    target_link_libraries(core_process_test${t} MiKTeX::Imported::LOG4CXX)
  else()
    target_link_libraries(core_process_test${t} ${log4cxx_dll_name})
  endif()
  target_link_libraries(core_process_test${t}
    ${core_dll_name}
    Threads::Threads
    miktex-popt-wrapper
  )
  add_test(
    NAME core_process_test${t}
    COMMAND $<TARGET_FILE:core_process_test${t}>
  )
endforeach()

foreach(x ${exes})
  add_executable(core_process_test${x} ${x}.cpp ${test_sources})
  set_property(TARGET core_process_test${x} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  merge_trustinfo_manifest(core_process_test${x} asInvoker)
  merge_compatibility_manifest(core_process_test${x})
  if(USE_SYSTEM_LOG4CXX)
    target_link_libraries(core_process_test${x} MiKTeX::Imported::LOG4CXX)
  else()
    target_link_libraries(core_process_test${x} ${log4cxx_dll_name})
  endif()
  target_link_libraries(core_process_test${x}
    ${core_dll_name}
    Threads::Threads
    miktex-popt-wrapper
  )
endforeach()
