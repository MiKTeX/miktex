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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/ui-mfc")

set(CMAKE_MFC_FLAG 2)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mikui.rc.in
  ${CMAKE_CURRENT_BINARY_DIR}/mikui.rc
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/res/mikui.rc2.in
  ${CMAKE_CURRENT_BINARY_DIR}/res/mikui.rc2
)

add_definitions(
  -D_AFXDLL
  -D_AFXEXT
  -D_UNICODE -DUNICODE
)

set(public_headers
  include/miktex/UI/MFC/ErrorDialog.h
  include/miktex/UI/MFC/PackageTreeCtrl.h
  include/miktex/UI/MFC/ProgressDialog.h
  include/miktex/UI/MFC/PropSheetPackage.h
  include/miktex/UI/MFC/Prototypes.h
  include/miktex/UI/MFC/SiteWizSheet.h
  include/miktex/UI/MFC/TextViewerDialog.h
  include/miktex/UI/MFC/UpdateDialog.h
)

set(${ui_mfc_dll_name}_sources
  ${CMAKE_CURRENT_BINARY_DIR}/mikui.rc
  ${public_headers}
  ConnectionSettingsDialog.cpp
  ConnectionSettingsDialog.h
  ErrorDialog.cpp
  GiveBackDialog.cpp
  GiveBackDialog.h
  InstallPackageDialog.cpp
  InstallPackageDialog.h
  PackageTreeCtrl.cpp
  ProgressDialog.cpp
  PropPageFiles.cpp
  PropPageFiles.h
  PropPagePackage.cpp
  PropPagePackage.h
  PropSheetPackage.cpp
  ProxyAuthenticationDialog.cpp
  ProxyAuthenticationDialog.h
  SearchPackageDialog.cpp
  SearchPackageDialog.h
  SiteWizDrive.cpp
  SiteWizDrive.h
  SiteWizLocal.cpp
  SiteWizLocal.h
  SiteWizRemote.cpp
  SiteWizRemote.h
  SiteWizSheet.cpp
  SiteWizType.cpp
  SiteWizType.h
  StdAfx.h
  TextViewerDialog.cpp
  UpdateDialog.cpp
  internal.h
  mikui-version.h
  mikui.cpp
  resource.h
)

add_library(${ui_mfc_dll_name} SHARED ${${ui_mfc_dll_name}_sources})

set_property(TARGET ${ui_mfc_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${ui_mfc_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_SERIES_STR}"
    SOVERSION "1"
)

target_compile_definitions(${ui_mfc_dll_name}
  PUBLIC
    -DMIKTEX_UI_MFC
)

target_include_directories(${ui_mfc_dll_name}
  PUBLIC
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_MFC_DIR}/include
)

target_link_libraries(${ui_mfc_dll_name}
  PRIVATE
    ${core_dll_name}
  PUBLIC
    ${mpm_dll_name}
)

rebase(${ui_mfc_dll_name})

install(TARGETS ${ui_mfc_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

if(INSTALL_MIKTEX_HEADERS)
  install(
    FILES
      include/miktex/UI/MFC/ErrorDialog
      include/miktex/UI/MFC/ErrorDialog.h
      include/miktex/UI/MFC/PackageTreeCtrl
      include/miktex/UI/MFC/PackageTreeCtrl.h
      include/miktex/UI/MFC/ProgressDialog
      include/miktex/UI/MFC/ProgressDialog.h
      include/miktex/UI/MFC/PropSheetPackage
      include/miktex/UI/MFC/PropSheetPackage.h
      include/miktex/UI/MFC/Prototypes
      include/miktex/UI/MFC/Prototypes.h
      include/miktex/UI/MFC/SiteWizSheet
      include/miktex/UI/MFC/SiteWizSheet.h
      include/miktex/UI/MFC/TextViewerDialog
      include/miktex/UI/MFC/TextViewerDialog.h
      include/miktex/UI/MFC/UpdateDialog
      include/miktex/UI/MFC/UpdateDialog.h
    DESTINATION
      ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/UI/MFC
  )
endif()

source_group(Public FILES ${public_headers})
