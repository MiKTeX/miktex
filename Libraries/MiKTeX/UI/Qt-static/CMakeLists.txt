## CMakeLists.txt					-*- CMake -*-
##
## Copyright (C) 2009-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/ui-qt-static")

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

include_directories(BEFORE ${CMAKE_BINARY_DIR}/${MIKTEX_REL_UI_QT_DIR})
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR})

use_static_crt()
use_static_libraries()

set(${ui_qt_lib_name}_sources
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/ConnectionSettingsDialog.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/ConnectionSettingsDialog.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/ErrorDialog.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/ErrorDialog.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/InstallPackageDialog.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/InstallPackageDialog.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/ProxyAuthenticationDialog.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/ProxyAuthenticationDialog.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/RepositoryTableModel.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/RepositoryTableModel.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizDrive.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizDrive.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizLocal.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizLocal.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizRemote.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizRemote.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizSheet.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizSheet.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizType.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizType.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/StdAfx.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/UpdateDialog.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/UpdateDialog.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/include/miktex/UI/Qt/ErrorDialog.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/include/miktex/UI/Qt/Prototypes.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/include/miktex/UI/Qt/SiteWizSheet.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/include/miktex/UI/Qt/UpdateDialog.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/internal.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/mikuiqt-version.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/mikuiqt.cpp
)

set(${ui_qt_lib_name}_moc_sources
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/ConnectionSettingsDialog.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/ErrorDialog.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/InstallPackageDialog.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/ProxyAuthenticationDialog.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/RepositoryTableModel.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizDrive.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizLocal.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizRemote.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizSheet.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizType.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/UpdateDialog.h
)

set(${ui_qt_lib_name}_ui_files
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/ConnectionSettingsDialog.ui
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/ErrorDialog.ui
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/InstallPackageDialog.ui
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/ProxyAuthenticationDialog.ui
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizDrive.ui
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizLocal.ui
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizRemote.ui
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/SiteWizType.ui
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/UpdateDialog.ui
)

qt5_wrap_cpp(${ui_qt_lib_name}_mocs ${${ui_qt_lib_name}_moc_sources})

qt5_wrap_ui(${ui_qt_lib_name}_ui_headers ${${ui_qt_lib_name}_ui_files})

qt5_wrap_cpp(${ui_qt_lib_name}_ui_mocs ${${ui_qt_lib_name}_ui_headers})

add_library(${ui_qt_lib_name} STATIC
  ${${ui_qt_lib_name}_sources}
  ${${ui_qt_lib_name}_mocs}
  ${${ui_qt_lib_name}_ui_mocs}
)

set_property(TARGET ${ui_qt_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${ui_qt_lib_name}
  PUBLIC
    -DMIKTEX_UI_QT
)

target_include_directories(${ui_qt_lib_name}
  PUBLIC
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_QT_DIR}/include
)

if(INSTALL_STATIC_LIBRARIES)
  install(TARGETS ${ui_qt_lib_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  )
endif()
