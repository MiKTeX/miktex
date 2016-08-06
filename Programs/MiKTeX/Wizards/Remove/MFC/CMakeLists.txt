## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_UNINSTALL_FOLDER}/MFC")

use_static_crt()

set(CMAKE_MFC_FLAG 1)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -D_UNICODE -DUNICODE
)

configure_file(
  res/Remove.manifest.in
  ${CMAKE_CURRENT_BINARY_DIR}/res/Remove.manifest
)

configure_file(
  Remove.rc.in
  ${CMAKE_CURRENT_BINARY_DIR}/Remove.rc
)

set(remove_mfc_sources
  FinishPage.cpp
  FinishPage.h
  Remove-version.h
  Remove.cpp
  Remove.h
  RemoveFilesPage.cpp
  RemoveFilesPage.h
  RemoveWizard.cpp
  RemoveWizard.h
  StdAfx.h
  WelcomePage.cpp
  WelcomePage.h
  resource.h
  targetver.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND remove_mfc_sources
    ${CMAKE_CURRENT_BINARY_DIR}/Remove.rc)
endif()

set(libs
  ${core_lib_name}
  ${getopt_lib_name}
  ${mpm_lib_name}
  ${setup_lib_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND libs
    Wldap32
    version
    wininet
    winmm
    ws2_32
  )
endif()

add_executable(remove_mfc WIN32 ${remove_mfc_sources})

set_property(TARGET remove_mfc PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  add_link_flags(remove_mfc "/ENTRY:\"wWinMainCRTStartup\"")
endif()

target_link_libraries(remove_mfc ${libs})

merge_compatibility_manifest(remove_mfc)
merge_dpi_aware_manifest(remove_mfc)
merge_trustinfo_manifest(remove_mfc asInvoker)

if(MIKTEX_NATIVE_WINDOWS)
  add_custom_command(
    TARGET
      remove_mfc
    POST_BUILD
    COMMAND
      ${MT_EXECUTABLE}
      -nologo
      -manifest ${CMAKE_CURRENT_BINARY_DIR}/res/Remove.manifest
      -updateresource:$<TARGET_FILE:remove_mfc>\;1
    VERBATIM
  )
endif()

install(
  FILES $<TARGET_FILE:remove_mfc>
  DESTINATION ${MIKTEX_INTERNAL_BINARY_DIR}
  RENAME uninstall.exe
)

add_executable(remove_mfc${MIKTEX_ADMIN_SUFFIX} WIN32 ${remove_mfc_sources})

set_property(TARGET remove_mfc${MIKTEX_ADMIN_SUFFIX} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  add_link_flags(remove_mfc${MIKTEX_ADMIN_SUFFIX} "/ENTRY:\"wWinMainCRTStartup\"")
endif()

target_link_libraries(remove_mfc${MIKTEX_ADMIN_SUFFIX} ${libs})

merge_compatibility_manifest(remove_mfc${MIKTEX_ADMIN_SUFFIX})
merge_dpi_aware_manifest(remove_mfc${MIKTEX_ADMIN_SUFFIX})
merge_trustinfo_manifest(remove_mfc${MIKTEX_ADMIN_SUFFIX} requireAdministrator)

if(MIKTEX_NATIVE_WINDOWS)
  add_custom_command(
    TARGET
      remove_mfc${MIKTEX_ADMIN_SUFFIX}
    POST_BUILD
    COMMAND
      ${MT_EXECUTABLE}
      -nologo
      -manifest ${CMAKE_CURRENT_BINARY_DIR}/res/Remove.manifest
      -updateresource:$<TARGET_FILE:remove_mfc${MIKTEX_ADMIN_SUFFIX}>\;1
    VERBATIM
  )
endif()

install(
  FILES $<TARGET_FILE:remove_mfc${MIKTEX_ADMIN_SUFFIX}>
  DESTINATION ${MIKTEX_INTERNAL_BINARY_DIR}
  RENAME uninstall${MIKTEX_ADMIN_SUFFIX}.exe
)
