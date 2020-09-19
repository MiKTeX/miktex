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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_FONT_UTILITIES_FOLDER}/CJK utils")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -DUSE_MIKTEX_EXIT
)

set(bg5conv_sources
  ${CMAKE_CURRENT_BINARY_DIR}/bg5conv.c
  bg5conv-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND bg5conv_sources
    ${MIKTEX_COMMON_MANIFEST}
    bg5conv.rc
  )
endif()

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/bg5conv.c
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/Bg5conv/bg5conv.w
  MAIN_DEPENDENCY
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/Bg5conv/bg5conv.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  VERBATIM
)

add_executable(${MIKTEX_PREFIX}bg5conv ${bg5conv_sources})

set_property(TARGET ${MIKTEX_PREFIX}bg5conv PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(TARGETS ${MIKTEX_PREFIX}bg5conv DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

foreach(t cef5conv cefconv cefsconv)
  set(${t}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/${t}.c
    ${t}-version.h
  )
  if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND ${t}_sources
      ${MIKTEX_COMMON_MANIFEST}
      ${t}.rc
    )
  endif()
  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/${t}.c
    COMMAND
      ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/CEFconv/${t}.w
    MAIN_DEPENDENCY
      ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/CEFconv/${t}.w
    DEPENDS
      ${MIKTEX_PREFIX}ctangle
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
  )
  add_executable(${MIKTEX_PREFIX}${t} ${${t}_sources})
  set_property(TARGET ${MIKTEX_PREFIX}${t} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  install(TARGETS ${MIKTEX_PREFIX}${t} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
endforeach()

set(extconv_sources
  ${CMAKE_CURRENT_BINARY_DIR}/extconv.c
  extconv-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
 list(APPEND extconv_sources
    ${MIKTEX_COMMON_MANIFEST}
    extconv.rc
 )
endif()

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/extconv.c
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/extconv/extconv.w
  MAIN_DEPENDENCY
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/extconv/extconv.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  VERBATIM
)

add_executable(${MIKTEX_PREFIX}extconv ${extconv_sources})

set_property(TARGET ${MIKTEX_PREFIX}extconv PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(TARGETS ${MIKTEX_PREFIX}extconv DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(hbf2gf_sources
  ${CMAKE_CURRENT_BINARY_DIR}/hbf2gf.c
  ${MIKTEX_LIBRARY_WRAPPER}
  hbf2gf-version.h
  source/utils/hbf2gf/hbf.c
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND hbf2gf_sources
    ${MIKTEX_COMMON_MANIFEST}
    hbf2gf.rc
  )
endif()

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/hbf2gf.c
  COMMAND
    ${MIKTEX_PREFIX}ctangle
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/hbf2gf/hbf2gf.w
    ${CMAKE_CURRENT_SOURCE_DIR}/hbf2gf-miktex.ch
  MAIN_DEPENDENCY
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/hbf2gf/hbf2gf.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
    ${CMAKE_CURRENT_SOURCE_DIR}/hbf2gf-miktex.ch
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  VERBATIM
)

add_executable(${MIKTEX_PREFIX}hbf2gf ${hbf2gf_sources})

set_property(TARGET ${MIKTEX_PREFIX}hbf2gf PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${MIKTEX_PREFIX}hbf2gf
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/hbf2gf
)

target_link_libraries(${MIKTEX_PREFIX}hbf2gf
  ${app_dll_name}
  ${core_dll_name}
  ${texmf_dll_name}
  ${w2cemu_dll_name}
)

install(TARGETS ${MIKTEX_PREFIX}hbf2gf DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(sjisconv_sources
  ${CMAKE_CURRENT_BINARY_DIR}/sjisconv.c
  sjisconv-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND sjisconv_sources
    ${MIKTEX_COMMON_MANIFEST}
    sjisconv.rc
  )
endif()

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/sjisconv.c
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/SJISconv/sjisconv.w
  MAIN_DEPENDENCY
    ${CMAKE_CURRENT_SOURCE_DIR}/source/utils/SJISconv/sjisconv.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  VERBATIM
)

add_executable(${MIKTEX_PREFIX}sjisconv ${sjisconv_sources})

set_property(TARGET ${MIKTEX_PREFIX}sjisconv PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(TARGETS ${MIKTEX_PREFIX}sjisconv DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

add_subdirectory(cjklatex)
