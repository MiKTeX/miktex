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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_UNINSTALL_FOLDER}/MFC")

set(CMAKE_MFC_FLAG 2)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -DUNICODE
  -D_UNICODE
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
  ${CMAKE_CURRENT_BINARY_DIR}/Remove.rc
  ${CMAKE_CURRENT_BINARY_DIR}/res/Remove.manifest
  ${MIKTEX_COMPATIBILITY_MANIFEST}
  ${MIKTEX_DPIAWARE_MANIFEST}
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

set(libs
  ${core_dll_name}
  ${getopt_dll_name}
  ${setup_dll_name}
  Wldap32
  version
  wininet
  winmm
  ws2_32
)

add_executable(remove_mfc WIN32 ${remove_mfc_sources})

set_property(TARGET remove_mfc PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_link_flags(remove_mfc "/ENTRY:\"wWinMainCRTStartup\"")

target_link_libraries(remove_mfc ${libs})

install(
  FILES $<TARGET_FILE:remove_mfc>
  DESTINATION ${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}
  RENAME uninstall.exe
)

add_executable(remove_mfc${MIKTEX_ADMIN_SUFFIX} WIN32 ${remove_mfc_sources})

set_property(TARGET remove_mfc${MIKTEX_ADMIN_SUFFIX} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_link_flags(remove_mfc${MIKTEX_ADMIN_SUFFIX} "/MANIFESTUAC:level='requireAdministrator'")
add_link_flags(remove_mfc${MIKTEX_ADMIN_SUFFIX} "/ENTRY:\"wWinMainCRTStartup\"")

target_link_libraries(remove_mfc${MIKTEX_ADMIN_SUFFIX} ${libs})

install(
  FILES $<TARGET_FILE:remove_mfc${MIKTEX_ADMIN_SUFFIX}>
  DESTINATION ${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}
  RENAME uninstall${MIKTEX_ADMIN_SUFFIX}.exe
)
