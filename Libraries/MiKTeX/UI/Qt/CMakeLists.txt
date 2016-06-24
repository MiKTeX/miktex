## CMakeLists.txt					-*- CMake -*-
##
## Copyright (C) 2008-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/ui-qt")

hide_symbols()

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(public_headers
  include/miktex/UI/Qt/ErrorDialog.h
  include/miktex/UI/Qt/Prototypes.h
  include/miktex/UI/Qt/SiteWizSheet.h
  include/miktex/UI/Qt/UpdateDialog.h
)  

set(${ui_qt_dll_name}_sources
  ${public_headers}
  ConnectionSettingsDialog.cpp
  ConnectionSettingsDialog.h
  ErrorDialog.cpp
  ErrorDialog.h
  InstallPackageDialog.cpp
  InstallPackageDialog.h
  ProxyAuthenticationDialog.cpp
  ProxyAuthenticationDialog.h
  RepositoryTableModel.cpp
  RepositoryTableModel.h
  SiteWizDrive.cpp
  SiteWizDrive.h
  SiteWizLocal.cpp
  SiteWizLocal.h
  SiteWizRemote.cpp
  SiteWizRemote.h
  SiteWizSheet.cpp
  SiteWizSheet.h
  SiteWizType.cpp
  SiteWizType.h
  StdAfx.h
  UpdateDialog.cpp
  UpdateDialog.h
  internal.h
  mikuiqt-version.h
  mikuiqt.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/mikuiqt.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/mikuiqt.rc
  )
  set(${ui_qt_dll_name}_sources ${${ui_qt_dll_name}_sources}
    ${CMAKE_CURRENT_BINARY_DIR}/mikuiqt.rc
  )
endif()

set(${ui_qt_dll_name}_moc_sources
  ConnectionSettingsDialog.h
  ErrorDialog.h
  InstallPackageDialog.h
  ProxyAuthenticationDialog.h
  RepositoryTableModel.h
  SiteWizDrive.h
  SiteWizLocal.h
  SiteWizRemote.h
  SiteWizSheet.h
  SiteWizType.h
  UpdateDialog.h
)

set(${ui_qt_dll_name}_ui_files
  ConnectionSettingsDialog.ui
  ErrorDialog.ui
  InstallPackageDialog.ui
  ProxyAuthenticationDialog.ui
  SiteWizDrive.ui
  SiteWizLocal.ui
  SiteWizRemote.ui
  SiteWizType.ui
  UpdateDialog.ui
)

set(${ui_qt_dll_name}_rcs_files
  mikuiqt.qrc
)

qt5_wrap_cpp(${ui_qt_dll_name}_mocs ${${ui_qt_dll_name}_moc_sources})

qt5_wrap_ui(${ui_qt_dll_name}_ui_headers ${${ui_qt_dll_name}_ui_files})

qt5_wrap_cpp(${ui_qt_dll_name}_ui_mocs ${${ui_qt_dll_name}_ui_headers})

qt5_add_resources(${ui_qt_dll_name}_rcc_sources ${${ui_qt_dll_name}_rcs_files})

add_library(${ui_qt_dll_name} SHARED
  ${${ui_qt_dll_name}_sources}
  ${${ui_qt_dll_name}_mocs}
  ${${ui_qt_dll_name}_ui_mocs}
  ${${ui_qt_dll_name}_rcc_sources}
)

set_property(TARGET ${ui_qt_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${ui_qt_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_SERIES_STR}"
    SOVERSION "1"
)

target_compile_definitions(${ui_qt_dll_name}
  PUBLIC
    -DMIKTEX_UI_QT
    -DMIKTEX_UI_QT_SHARED
)

target_include_directories(${ui_qt_dll_name}
  PUBLIC
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/include
)

target_link_libraries(${ui_qt_dll_name}
  PRIVATE
    ${core_dll_name}
    ${mpm_dll_name}
    Qt5::Gui
    Qt5::Widgets
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${ui_qt_dll_name}
    PRIVATE
      Qt5::WinExtras
      Secur32
      comctl32
  )
endif()

rebase(${ui_qt_dll_name})

install(TARGETS ${ui_qt_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

if(INSTALL_MIKTEX_HEADERS)
  install(
    FILES
      include/miktex/UI/Qt/ErrorDialog
      include/miktex/UI/Qt/ErrorDialog.h
      include/miktex/UI/Qt/SiteWizSheet
      include/miktex/UI/Qt/SiteWizSheet.h
      include/miktex/UI/Qt/Prototypes
      include/miktex/UI/Qt/Prototypes.h
      include/miktex/UI/Qt/UpdateDialog
      include/miktex/UI/Qt/UpdateDialog.h
    DESTINATION
      ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/UI/Qt
  )
  install(
    FILES
      ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_DIR}/include/miktex/UI/UI
      ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_DIR}/include/miktex/UI/UI.h
    DESTINATION
      ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/UI
  )
endif()

source_group(Public FILES ${public_headers})
