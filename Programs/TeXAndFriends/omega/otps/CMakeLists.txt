## CMakeLists.txt                                       -*- CMake -*-
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_OMEGA_FOLDER}/otps")

add_definitions(
  -DKPATHSEA
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)

set(mkocp_sources
  mkocp.cpp
  mkocp-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND mkocp_sources
    ${MIKTEX_COMMON_MANIFEST}
    mkocp.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}mkocp ${mkocp_sources})

set_property(TARGET ${MIKTEX_PREFIX}mkocp PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}mkocp
  ${app_dll_name}
  ${core_dll_name}
)

install(TARGETS ${MIKTEX_PREFIX}mkocp DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(outocp_sources
  source/outocp.c
  outocp-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND outocp_sources
    ${MIKTEX_COMMON_MANIFEST}
    outocp.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}outocp ${outocp_sources})

set_property(TARGET ${MIKTEX_PREFIX}outocp PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}outocp
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${texmf_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}outocp
    ${getopt_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}outocp DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(otp2ocp_sources
  ${CMAKE_CURRENT_BINARY_DIR}/y_tab.c
  ${CMAKE_CURRENT_BINARY_DIR}/y_tab.h
  ${CMAKE_CURRENT_BINARY_DIR}/yy.lex.c
  ${MIKTEX_LIBRARY_WRAPPER}
  otp2ocp-version.h
  source/otp2ocp.c
  source/routines.c
  source/otp.h
  source/yystype.h
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/y_tab.c
    ${CMAKE_CURRENT_BINARY_DIR}/y_tab.h
  COMMAND
    ${BISON_EXECUTABLE}
      --output=${CMAKE_CURRENT_BINARY_DIR}/y_tab.c
      --defines=${CMAKE_CURRENT_BINARY_DIR}/y_tab.h
      ${CMAKE_CURRENT_SOURCE_DIR}/source/otp.y
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/otp.y
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/yy.lex.c
  COMMAND
    ${FLEX_EXECUTABLE}
    -o${CMAKE_CURRENT_BINARY_DIR}/yy.lex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/otp.l
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/otp.l
  VERBATIM
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND otp2ocp_sources
    ${MIKTEX_COMMON_MANIFEST}
    otp2ocp.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}otp2ocp ${otp2ocp_sources})

set_property(TARGET ${MIKTEX_PREFIX}otp2ocp PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}otp2ocp
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${texmf_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}otp2ocp
    ${getopt_dll_name}
    ${unxemu_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}otp2ocp DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
