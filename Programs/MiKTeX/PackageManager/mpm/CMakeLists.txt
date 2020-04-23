## CMakeLists.txt                                       -*- CMake -*-
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PACKAGE_MANAGER_FOLDER}")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(mpm_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  internal.h
  mpm-version.h
  mpm.cpp
)

add_definitions(
  -DHAVE_CONFIG_H=1
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND mpm_sources
    ${MIKTEX_COMMON_MANIFEST}
    mpm.rc
  )
endif()

add_executable(${MIKTEX_PROG_NAME_MPM} ${mpm_sources})

set_property(TARGET ${MIKTEX_PROG_NAME_MPM} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_FMT)
  target_link_libraries(${MIKTEX_PROG_NAME_MPM} MiKTeX::Imported::FMT)
else()
  target_link_libraries(${MIKTEX_PROG_NAME_MPM} ${fmt_dll_name})
endif()

if(USE_SYSTEM_LOG4CXX)
  target_link_libraries(${MIKTEX_PROG_NAME_MPM} MiKTeX::Imported::LOG4CXX)
else()
  target_link_libraries(${MIKTEX_PROG_NAME_MPM} ${log4cxx_dll_name})
endif()

target_link_libraries(${MIKTEX_PROG_NAME_MPM}
  ${core_dll_name}
  ${mpm_dll_name}
  ${setup_dll_name}
  miktex-popt-wrapper
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PROG_NAME_MPM}
    Wldap32
    shlwapi
    wininet
    winmm
    ws2_32
  )
endif()

install(TARGETS ${MIKTEX_PROG_NAME_MPM} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
