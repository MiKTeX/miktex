## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2008-2023 Christian Schenk
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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/ui-qt")

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

set(CMAKE_AUTOMOC TRUE)

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(public_headers
  include/miktex/UI/Qt/ErrorDialog.h
  include/miktex/UI/Qt/PackageInfoDialog.h
  include/miktex/UI/Qt/Prototypes.h
  include/miktex/UI/Qt/SiteWizSheet.h
  include/miktex/UI/Qt/UpdateDialog.h
)  

configure_file(
  mikuiqt-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/mikuiqt-version.h
)

set(${ui_qt_dll_name}_sources
  ${CMAKE_CURRENT_BINARY_DIR}/mikuiqt-version.h
  ${public_headers}
  ConnectionSettingsDialog.cpp
  ConnectionSettingsDialog.h
  ConnectionSettingsDialog.h
  ErrorDialog.cpp
  ErrorDialog.h
  ErrorDialog.h
  FileTableModel.cpp
  FileTableModel.h
  FileTableModel.h
  InstallPackageDialog.cpp
  InstallPackageDialog.h
  InstallPackageDialog.h
  PackageInfoDialog.cpp
  PackageInfoDialog.h
  PackageInfoDialog.h
  ProxyAuthenticationDialog.cpp
  ProxyAuthenticationDialog.h
  ProxyAuthenticationDialog.h
  RepositoryTableModel.cpp
  RepositoryTableModel.h
  RepositoryTableModel.h
  SiteWizDrive.cpp
  SiteWizDrive.h
  SiteWizDrive.h
  SiteWizLocal.cpp
  SiteWizLocal.h
  SiteWizLocal.h
  SiteWizRemote.cpp
  SiteWizRemote.h
  SiteWizRemote.h
  SiteWizSheet.cpp
  SiteWizSheet.h
  SiteWizSheet.h
  SiteWizType.cpp
  SiteWizType.h
  SiteWizType.h
  UpdateDialog.cpp
  UpdateDialog.h
  UpdateDialog.h
  internal.h
  mikuiqt.cpp
)

list(APPEND l_sources ${${ui_qt_dll_name}_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/mikuiqt.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/mikuiqt.rc
  )
  set(${ui_qt_dll_name}_sources ${${ui_qt_dll_name}_sources}
    ${CMAKE_CURRENT_BINARY_DIR}/mikuiqt.rc
  )
endif()

set(${ui_qt_dll_name}_ui_files
  ConnectionSettingsDialog.ui
  ErrorDialog.ui
  InstallPackageDialog.ui
  PackageInfoDialog.ui
  ProxyAuthenticationDialog.ui
  SiteWizDrive.ui
  SiteWizLocal.ui
  SiteWizRemote.ui
  SiteWizType.ui
  UpdateDialog.ui
)

list(APPEND l_sources ${${ui_qt_dll_name}_ui_files})

foreach(l ${MIKTEX_TRANSLATION_DONE_LOCALES})
  list(APPEND translations
    translations/ui_${l}.ts
  )
  set(qresource_files
    "${qresource_files}<file alias='ui_${l}.qm'>${CMAKE_CURRENT_BINARY_DIR}/ui_${l}.qm</file>"
  )
endforeach()

get_target_property(LUPDATE_EXECUTABLE Qt${QT_SERIES}::lupdate LOCATION)

add_custom_target(gen-ui-translations
  COMMAND
    ${LUPDATE_EXECUTABLE} ${l_sources} -ts ${translations}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set_property(TARGET gen-ui-translations PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

if(QT_SERIES STREQUAL "5")
  qt5_add_translation(qm_files ${translations})
else()
  qt_add_translation(qm_files ${translations})
endif()

configure_file(
  translations.qrc.in
  ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
)

set(${ui_qt_dll_name}_qrc_files
  ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
  mikuiqt.qrc
)

if(QT_SERIES STREQUAL "5")
  qt5_wrap_ui(${ui_qt_dll_name}_ui_headers ${${ui_qt_dll_name}_ui_files})
else()
  qt_wrap_ui(${ui_qt_dll_name}_ui_headers ${${ui_qt_dll_name}_ui_files})
endif()

if(QT_SERIES STREQUAL "5")
  qt5_add_resources(${ui_qt_dll_name}_rcc_sources ${${ui_qt_dll_name}_qrc_files})
else()
  qt_add_resources(${ui_qt_dll_name}_rcc_sources ${${ui_qt_dll_name}_qrc_files})
endif()

add_library(${ui_qt_dll_name} SHARED
  ${${ui_qt_dll_name}_sources}
  ${${ui_qt_dll_name}_ui_headers}
  ${${ui_qt_dll_name}_rcc_sources}
)

set_property(TARGET ${ui_qt_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
  ${ui_qt_dll_name}
  ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
  ${MIKTEX_COMP_INTERFACE_VERSION}
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
    ${locale_dll_name}
    ${mpm_dll_name}
    ${setup_dll_name}
    Qt${QT_SERIES}::Gui
    Qt${QT_SERIES}::Widgets
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${ui_qt_dll_name}
    PRIVATE
      Secur32
  )
endif()

if(MIKTEX_NATIVE_WINDOWS AND QT_SERIES STREQUAL "5")
  target_link_libraries(${ui_qt_dll_name}
    PRIVATE
      Qt${QT_SERIES}::WinExtras
  )
endif()

install(TARGETS ${ui_qt_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

if(INSTALL_MIKTEX_HEADERS)
  install(
    FILES
      include/miktex/UI/Qt/ErrorDialog
      include/miktex/UI/Qt/ErrorDialog.h
      include/miktex/UI/Qt/PackageInfoDialog
      include/miktex/UI/Qt/PackageInfoDialog.h
      include/miktex/UI/Qt/Prototypes
      include/miktex/UI/Qt/Prototypes.h
      include/miktex/UI/Qt/SiteWizSheet
      include/miktex/UI/Qt/SiteWizSheet.h
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
