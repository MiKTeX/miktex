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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_UPDATE_FOLDER}/MFC")

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
  res/Update.manifest.in
  ${CMAKE_CURRENT_BINARY_DIR}/res/Update.manifest
)

configure_file(
  Update.rc.in
  ${CMAKE_CURRENT_BINARY_DIR}/Update.rc
)

set(update_mfc_sources
  ${CMAKE_CURRENT_BINARY_DIR}/ConnectionSettingsDialog.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ConnectionSettingsDialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/ProxyAuthenticationDialog.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ProxyAuthenticationDialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/Update.rc
  ${CMAKE_CURRENT_BINARY_DIR}/res/Update.manifest
  ${MIKTEX_COMPATIBILITY_MANIFEST}
  ${MIKTEX_DPIAWARE_MANIFEST}
  CdPage.cpp
  CdPage.h
  FileCopyPage.cpp
  FileCopyPage.h
  FinishPage.cpp
  FinishPage.h
  LocalPage.cpp
  LocalPage.h
  PackageListPage.cpp
  PackageListPage.h
  RemotePage.cpp
  RemotePage.h
  StdAfx.h
  TypePage.cpp
  TypePage.h
  Update-version.h
  Update.cpp
  Update.h
  UpdateWizard.cpp
  UpdateWizard.h
  WelcomePage.cpp
  WelcomePage.h
  resource.h
  targetver.h
)

foreach(f
    ConnectionSettingsDialog.cpp
    ConnectionSettingsDialog.h
    ProxyAuthenticationDialog.cpp
    ProxyAuthenticationDialog.h)
  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/${f}
    COMMAND
      ${CMAKE_COMMAND}
      -E copy_if_different
      ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_MFC_DIR}/${f}
      ${CMAKE_CURRENT_BINARY_DIR}/${f}
    DEPENDS
      ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_MFC_DIR}/${f}
    VERBATIM
  )
endforeach()

set(libs
  ${core_lib_name}
  ${getopt_lib_name}
  ${mpm_lib_name}
  Wldap32
  version
  wininet
  winmm
  ws2_32
)

if (USE_SYSTEM_LOG4CXX)
  list(APPEND libs MiKTeX::Imported::LOG4CXX)
else()
  list(APPEND libs ${log4cxx_lib_name})
endif()

add_executable(update_mfc WIN32 ${update_mfc_sources})

set_property(TARGET update_mfc PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_link_flags(update_mfc "/ENTRY:\"wWinMainCRTStartup\"")

target_link_libraries(update_mfc ${libs})

install(
  FILES $<TARGET_FILE:update_mfc>
  DESTINATION ${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}
  RENAME ${MIKTEX_PREFIX}update.exe
)

add_executable(update_mfc${MIKTEX_ADMIN_SUFFIX} WIN32 ${update_mfc_sources})

set_property(TARGET update_mfc${MIKTEX_ADMIN_SUFFIX} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_link_flags(update_mfc${MIKTEX_ADMIN_SUFFIX} "/MANIFESTUAC:level='requireAdministrator'")
add_link_flags(update_mfc${MIKTEX_ADMIN_SUFFIX} "/ENTRY:\"wWinMainCRTStartup\"")

target_link_libraries(update_mfc${MIKTEX_ADMIN_SUFFIX} ${libs})

install(
  FILES $<TARGET_FILE:update_mfc${MIKTEX_ADMIN_SUFFIX}>
  DESTINATION ${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}
  RENAME ${MIKTEX_PREFIX}update${MIKTEX_ADMIN_SUFFIX}.exe
)
