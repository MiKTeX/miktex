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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PACKAGE_MANAGER_FOLDER}/MFC")

set(CMAKE_MFC_FLAG 2)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -D_AFXDLL
  -D_UNICODE -DUNICODE
)

configure_file(
  res/mpm.manifest.in
  ${CMAKE_CURRENT_BINARY_DIR}/res/mpm.manifest
)

configure_file(
  mpm.rc.in
  ${CMAKE_CURRENT_BINARY_DIR}/mpm.rc
)

set(mpm_mfc_sources
  MainFrame.cpp
  MainFrame.h
  MpmDoc.cpp
  MpmDoc.h
  MpmView.cpp
  MpmView.h
  mpm-version.h
  mpm.cpp
  mpm.h
  resource.h
  stdafx.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND mpm_mfc_sources
    ${CMAKE_CURRENT_BINARY_DIR}/mpm.rc)
endif()

set(libs
  ${core_dll_name}
  ${mpm_dll_name}
  ${ui_mfc_dll_name}
)

add_executable(mpm_mfc WIN32 ${mpm_mfc_sources})

set_property(TARGET mpm_mfc PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  add_link_flags(mpm_mfc "/ENTRY:\"wWinMainCRTStartup\"")
endif()

target_link_libraries(mpm_mfc ${libs})

merge_compatibility_manifest(mpm_mfc)
merge_dpi_aware_manifest(mpm_mfc)
merge_trustinfo_manifest(mpm_mfc asInvoker)

if(MIKTEX_NATIVE_WINDOWS)
  add_custom_command(
    TARGET
      mpm_mfc
    POST_BUILD
    COMMAND
      ${MT_EXECUTABLE}
      -nologo
      -manifest ${CMAKE_CURRENT_BINARY_DIR}/res/mpm.manifest
      -updateresource:$<TARGET_FILE:mpm_mfc>\;1
    VERBATIM
  )
endif()

install(TARGETS mpm_mfc DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

add_executable(mpm_mfc${MIKTEX_ADMIN_SUFFIX} WIN32 ${mpm_mfc_sources})

set_property(TARGET mpm_mfc${MIKTEX_ADMIN_SUFFIX} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  add_link_flags(mpm_mfc${MIKTEX_ADMIN_SUFFIX} "/ENTRY:\"wWinMainCRTStartup\"")
endif()

target_link_libraries(mpm_mfc${MIKTEX_ADMIN_SUFFIX} ${libs})

merge_compatibility_manifest(mpm_mfc${MIKTEX_ADMIN_SUFFIX})
merge_dpi_aware_manifest(mpm_mfc${MIKTEX_ADMIN_SUFFIX})
merge_trustinfo_manifest(mpm_mfc${MIKTEX_ADMIN_SUFFIX} requireAdministrator)

if(MIKTEX_NATIVE_WINDOWS)
  add_custom_command(
    TARGET
      mpm_mfc${MIKTEX_ADMIN_SUFFIX}
    POST_BUILD
    COMMAND
      ${MT_EXECUTABLE}
      -nologo
      -manifest ${CMAKE_CURRENT_BINARY_DIR}/res/mpm.manifest
      -updateresource:$<TARGET_FILE:mpm_mfc${MIKTEX_ADMIN_SUFFIX}>\;1
    VERBATIM
  )
endif()

install(TARGETS mpm_mfc${MIKTEX_ADMIN_SUFFIX} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
