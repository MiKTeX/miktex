## CMakeLists.txt                                       -*- CMake -*-
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_PREPROCESSORS_FOLDER}/autosp")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

add_definitions(
  -DHAVE_CONFIG_H
)

set(autosp_sources
  source/autosp.c
  source/autosp.h
  source/process_command.c
  source/process_command.h
  source/process_score.c
  source/process_score.h
  source/spacing_note.c
  source/status.c
  source/utils.c
  source/utils.h
  source/xtuplet.c
)

list(APPEND autosp_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  autosp-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND autosp_sources
    ${MIKTEX_COMMON_MANIFEST}
    autosp.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}autosp ${autosp_sources})

set_property(TARGET ${MIKTEX_PREFIX}autosp PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}autosp ${getopt_dll_name})
endif()

install(TARGETS ${MIKTEX_PREFIX}autosp DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(rebar_sources
  source/rebar.c
  source/utils.c
  source/utils.h
)

list(APPEND rebar_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  rebar-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND rebar_sources
    ${MIKTEX_COMMON_MANIFEST}
    rebar.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}rebar ${rebar_sources})

set_property(TARGET ${MIKTEX_PREFIX}rebar PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}rebar ${getopt_dll_name})
endif()

install(TARGETS ${MIKTEX_PREFIX}rebar DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(tex2aspc_sources
  source/tex2aspc.c
  source/utils.c
  source/utils.h
)

list(APPEND tex2aspc_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  tex2aspc-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND tex2aspc_sources
    ${MIKTEX_COMMON_MANIFEST}
    tex2aspc.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}tex2aspc ${tex2aspc_sources})

set_property(TARGET ${MIKTEX_PREFIX}tex2aspc PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}tex2aspc ${getopt_dll_name})
endif()

install(TARGETS ${MIKTEX_PREFIX}tex2aspc DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
