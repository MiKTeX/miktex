## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2019 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/etc")

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

set(executables
  rungs
)

set(internal_executables
  runexe
  runjava
  runperl
  runpython
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND internal_executables
    runbat
  )
else()
  list(APPEND internal_executables
    runsh
  )
endif()

foreach(t ${executables})
  set(${t}_sources ${t}.cpp)
  if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND ${t}_sources
      ${MIKTEX_COMPATIBILITY_MANIFEST}
    )
  endif()
  add_executable(${MIKTEX_PREFIX}${t} ${${t}_sources})
  set_property(TARGET ${MIKTEX_PREFIX}${t} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_link_libraries(${MIKTEX_PREFIX}${t}
    ${app_dll_name}
    ${core_dll_name}
  )
  install(TARGETS ${MIKTEX_PREFIX}${t} DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}")
endforeach()

foreach(t ${internal_executables})
  set(${t}_sources ${t}.cpp)
  if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND ${t}_sources
      ${MIKTEX_COMPATIBILITY_MANIFEST}
    )
  endif()
  add_executable(${t} ${${t}_sources})
  set_property(TARGET ${t} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  if(MIKTEX_MACOS_BUNDLE)
    set_target_properties(${t} PROPERTIES INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
  elseif (APPLE)
    set_target_properties(${t} PROPERTIES INSTALL_RPATH "@loader_path/../../${CMAKE_INSTALL_LIBDIR}")
  else()
    set_target_properties(${t} PROPERTIES INSTALL_RPATH "$ORIGIN/../../${CMAKE_INSTALL_LIBDIR}")
  endif()
  target_link_libraries(${t}
    ${app_dll_name}
    ${core_dll_name}
  )
  install(TARGETS ${t} DESTINATION "${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}")
endforeach()
