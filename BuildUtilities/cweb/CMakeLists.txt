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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}/CWeb")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
  ${CMAKE_CURRENT_SOURCE_DIR}/texlive
)

add_definitions(
  -DUSE_MIKTEX_EXIT
)

set(initctangle_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/initcommon.c
  ${CMAKE_CURRENT_SOURCE_DIR}/initctangle.c
  ${CMAKE_CURRENT_SOURCE_DIR}/initcweb.h
  ${MIKTEX_LIBRARY_WRAPPER}
  ctangle-version.h
)

set_source_files_properties(
  ${CMAKE_CURRENT_SOURCE_DIR}/initcommon.c
  ${CMAKE_CURRENT_SOURCE_DIR}/initctangle.c
  PROPERTIES LANGUAGE CXX
)

if(MIKTEX_NATIVE_WINDOWS)
  set(initctangle_sources ${initctangle_sources} ctangle.rc)
endif()

add_executable(initctangle ${initctangle_sources})

set_property(TARGET initctangle PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(initctangle
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${w2cemu_dll_name}
)

merge_trustinfo_manifest(initctangle asInvoker)
merge_compatibility_manifest(initctangle)

set(ctangle_sources
  ${CMAKE_CURRENT_BINARY_DIR}/common.c
  ${CMAKE_CURRENT_BINARY_DIR}/ctangle.c
  ${CMAKE_CURRENT_BINARY_DIR}/cweb.h
  ${MIKTEX_LIBRARY_WRAPPER}
  ctangle-version.h
  texlive/help.h
)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/common.c
  ${CMAKE_CURRENT_BINARY_DIR}/ctangle.c
  ${CMAKE_CURRENT_BINARY_DIR}/cweb.h
  PROPERTIES GENERATED TRUE
)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/common.c
  ${CMAKE_CURRENT_BINARY_DIR}/ctangle.c
  PROPERTIES LANGUAGE CXX
)

set_source_files_properties(
  ${MIKTEX_LIBRARY_WRAPPER}
  PROPERTIES COMPILE_FLAGS "-DCPLUSPLUSMAIN"
)

if(MIKTEX_NATIVE_WINDOWS)
  set(ctangle_sources ${ctangle_sources} ctangle.rc)
endif()

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/ctangle.c
  COMMAND
    initctangle
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ctangle.w
    ${CMAKE_CURRENT_SOURCE_DIR}/texlive/ctang-w2c.ch
    ${CMAKE_CURRENT_BINARY_DIR}/ctangle.c
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ctangle.w
  DEPENDS
    initctangle
    ${CMAKE_CURRENT_SOURCE_DIR}/source/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ctangle.w
    ${CMAKE_CURRENT_SOURCE_DIR}/texlive/ctang-w2c.ch
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/common.c
    ${CMAKE_CURRENT_BINARY_DIR}/cweb.h
  COMMAND
    initctangle
    ${CMAKE_CURRENT_SOURCE_DIR}/source/common.w
    ${CMAKE_CURRENT_SOURCE_DIR}/texlive/comm-w2c.ch
    ${CMAKE_CURRENT_BINARY_DIR}/common.c
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    ${CMAKE_CURRENT_SOURCE_DIR}/source/common.w
  DEPENDS
    initctangle
    ${CMAKE_CURRENT_SOURCE_DIR}/source/common.w
    ${CMAKE_CURRENT_SOURCE_DIR}/texlive/comm-w2c.ch
  VERBATIM
)

add_executable(${MIKTEX_PREFIX}ctangle ${ctangle_sources})

set_property(TARGET ${MIKTEX_PREFIX}ctangle PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(${MIKTEX_PREFIX}ctangle asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}ctangle)

target_link_libraries(${MIKTEX_PREFIX}ctangle
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${w2cemu_dll_name}
)

set(cweave_sources
  ${CMAKE_CURRENT_BINARY_DIR}/common.c
  ${CMAKE_CURRENT_BINARY_DIR}/cweave.c
  ${CMAKE_CURRENT_BINARY_DIR}/cweb.h
  ${MIKTEX_LIBRARY_WRAPPER}
  cweave-version.h
  texlive/help.h
)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/cweave.c
  PROPERTIES
    GENERATED TRUE
    LANGUAGE CXX
)

if(MIKTEX_NATIVE_WINDOWS)
  set(cweave_sources ${cweave_sources} cweave.rc)
endif()

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/cweave.c
  COMMAND
    ${MIKTEX_PREFIX}ctangle
    ${CMAKE_CURRENT_SOURCE_DIR}/source/cweave.w
    ${CMAKE_CURRENT_SOURCE_DIR}/texlive/cweav-w2c.ch
    ${CMAKE_CURRENT_BINARY_DIR}/cweave.c
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
  MAIN_DEPENDENCY
    ${CMAKE_CURRENT_SOURCE_DIR}/source/cweave.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
    ${CMAKE_CURRENT_SOURCE_DIR}/source/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/cweave.w
    ${CMAKE_CURRENT_SOURCE_DIR}/source/prod.w
    ${CMAKE_CURRENT_SOURCE_DIR}/texlive/cweav-w2c.ch
  VERBATIM
)

add_executable(${MIKTEX_PREFIX}cweave ${cweave_sources})

set_property(TARGET ${MIKTEX_PREFIX}cweave PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(${MIKTEX_PREFIX}cweave asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}cweave)

target_link_libraries(${MIKTEX_PREFIX}cweave
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${w2cemu_dll_name}
)

install(
  TARGETS
    ${MIKTEX_PREFIX}ctangle
    ${MIKTEX_PREFIX}cweave
 DESTINATION
   "${MIKTEX_BINARY_DESTINATION_DIR}"
)
