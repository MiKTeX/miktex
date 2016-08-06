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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_SETUP_FOLDER}/MFC")

use_static_crt()

set(CMAKE_MFC_FLAG 1)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -D_UNICODE -DUNICODE
)

set(ENABLE_ADDTEXMF FALSE)
set(SHOW_FOLDER_PAGE FALSE)

configure_file(
  res/Setup.manifest.in
  ${CMAKE_CURRENT_BINARY_DIR}/res/Setup.manifest
)

configure_file(
  Setup.rc.in
  ${CMAKE_CURRENT_BINARY_DIR}/Setup.rc
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(setup_mfc_sources
  ${CMAKE_CURRENT_BINARY_DIR}/ConnectionSettingsDialog.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ConnectionSettingsDialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/ProxyAuthenticationDialog.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ProxyAuthenticationDialog.h
  FileCopyPage.cpp
  FileCopyPage.h
  FinishPage.cpp
  FinishPage.h
  InfoListPage.cpp
  InfoListPage.h
  InstallDirPage.cpp
  InstallDirPage.h
  LicensePage.cpp
  LicensePage.h
  LocalRepositoryPage.cpp
  LocalRepositoryPage.h
  MDTaskPage.cpp
  MDTaskPage.h
  PackageSetPage.cpp
  PackageSetPage.h
  RemoteRepositoryPage.cpp
  RemoteRepositoryPage.h
  SettingsPage.cpp
  SettingsPage.h
  Setup-version.h
  Setup.cpp
  Setup.h
  SetupWizard.cpp
  SetupWizard.h
  SharedInstallationPage.cpp
  SharedInstallationPage.h
  StdAfx.h
  TaskPage.cpp
  TaskPage.h
  WelcomePage.cpp
  WelcomePage.h
  resource.h
  targetver.h
)

if(ENABLE_ADDTEXMF)
  list(APPEND setup_mfc_sources
    AddTEXMFPage.cpp
    AddTEXMFPage.h
    EditTEXMFPage.cpp
    EditTEXMFPage.h
  )
endif()

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

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND setup_mfc_sources
    ${CMAKE_CURRENT_BINARY_DIR}/Setup.rc)
endif()

add_executable(setup WIN32 ${setup_mfc_sources})

set_property(TARGET setup PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  add_link_flags(setup "/ENTRY:\"wWinMainCRTStartup\"")
endif()

target_link_libraries(setup
  ${core_lib_name}
  ${getopt_lib_name}
  ${mpm_lib_name}
  ${setup_lib_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(setup
    version
    Wldap32
    wininet
    winmm
    ws2_32
  )
endif()

merge_compatibility_manifest(setup)
merge_dpi_aware_manifest(setup)
merge_trustinfo_manifest(setup highestAvailable)

if(MIKTEX_NATIVE_WINDOWS)
  add_custom_command(
    TARGET
      setup
    POST_BUILD
    COMMAND
      ${MT_EXECUTABLE}
      -nologo
      -manifest ${CMAKE_CURRENT_BINARY_DIR}/res/Setup.manifest
      -updateresource:$<TARGET_FILE:setup>\;1
    VERBATIM
  )
endif()

install(TARGETS setup DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/LICENSE.TXT
  COMMAND
    ${PANDOC_EXECUTABLE}
    -f docbook
    -t plain
    -o ${CMAKE_CURRENT_BINARY_DIR}/LICENSE.TXT
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_DOC_DIR}/license.xml
  MAIN_DEPENDENCY
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_DOC_DIR}/license.xml
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  VERBATIM
)

add_custom_target(gen-license-text ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/LICENSE.TXT
)

set_property(TARGET gen-license-text PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(setup gen-license-text)
