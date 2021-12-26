## CMakeLists.txt
##
## Copyright (C) 2021 Christian Schenk
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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/miktex")

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
  miktex-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/miktex-version.h
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
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

set(miktex_sources
  internal.h
  miktex.cpp
  shims/mkfntmap.cpp
  shims/mkfntmap.h
  shims/updmap.cpp
  shims/updmap.h
  topics/Topic.cpp
  topics/Topic.h
  topics/filesystem/topic.cpp
  topics/filesystem/topic.h
  topics/filesystem/commands/commands.h
  topics/filesystem/commands/watch.cpp
  topics/fontmaps/topic.cpp
  topics/fontmaps/topic.h
  topics/fontmaps/commands/FontMapManager.cpp
  topics/fontmaps/commands/FontMapManager.h
  topics/fontmaps/commands/commands.h
  topics/fontmaps/commands/set-option.cpp
  topics/fontmaps/commands/show-option.cpp
  topics/fontmaps/commands/update.cpp
)

list(APPEND miktex_sources
  ${CMAKE_CURRENT_BINARY_DIR}/miktex-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    miktex.rc.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/miktex.rc
  )
  list(APPEND miktex_sources
    ${MIKTEX_COMMON_MANIFEST}
    ${CMAKE_CURRENT_BINARY_DIR}/miktex.rc
  )
endif()

add_executable(miktex ${miktex_sources})

set_property(TARGET miktex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(TARGETS miktex DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

if (USE_SYSTEM_LOG4CXX)
  target_link_libraries(miktex MiKTeX::Imported::LOG4CXX)
else()
  target_link_libraries(miktex ${log4cxx_dll_name})
endif()

if (USE_SYSTEM_FMT)
  target_link_libraries(miktex MiKTeX::Imported::FMT)
else()
  target_link_libraries(miktex ${fmt_dll_name})
endif()

target_link_libraries(miktex
  ${core_dll_name}
  ${mpm_dll_name}
  ${setup_dll_name}
  miktex-popt-wrapper
)
