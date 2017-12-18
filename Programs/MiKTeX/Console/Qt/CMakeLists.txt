## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2017 Christian Schenk
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

include(InstallQt5Plugin)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_CONSOLE_FOLDER}/Qt")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(console_qt_sources
  console-version.h
  main.cpp
  mainwindow.cpp
  mainwindow.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND console_qt_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/console.rc
    ${MIKTEX_COMPATIBILITY_MANIFEST}
  )
endif()

set(console_qt_moc_sources
  mainwindow.h
)

set(console_qt_ui_files
  mainwindow.ui
)

set(console_qt_rcs_files
  console.qrc
)

qt5_wrap_cpp(console_qt_mocs ${console_qt_moc_sources})

qt5_wrap_ui(console_qt_ui_headers ${console_qt_ui_files})

qt5_wrap_cpp(console_qt_ui_mocs ${console_qt_ui_headers})

qt5_add_resources(console_qt_rcc_sources ${console_qt_rcs_files})

set(sources
  ${console_qt_sources}
  ${console_qt_mocs}
  ${console_qt_ui_mocs}
  ${console_qt_rcc_sources}
)

set(libs
  ${core_dll_name}
  ${mpm_dll_name}
  ${ui_qt_dll_name}
  Qt5::Widgets
  miktex-popt-wrapper
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND libs
    Comctl32.lib
  )
endif()

# stibizt: https://github.com/Kitware/CMake/blob/master/Source/QtDialog/CMakeLists.txt
if(MIKTEX_MACOS_BUNDLE)
  install_qt5_plugin("Qt5::QCocoaIntegrationPlugin" QT_PLUGINS)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    "[Paths]\nPlugins = ${_qt_plugin_dir}\n")
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    DESTINATION "${MIKTEX_MACOS_BUNDLE_DIR}/Resources"
  )
  get_property(_Qt5_Core_LOCATION TARGET Qt5::Core PROPERTY LOCATION)
  get_filename_component(Qt_BIN_DIR "${_Qt5_Core_LOCATION}" PATH)
  get_filename_component(Qt_BIN_DIR "${Qt_BIN_DIR}" PATH)
endif()

add_executable(console_qt WIN32 MACOSX_BUNDLE ${sources})
set_property(TARGET console_qt PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(console_qt ${libs})  
if(MIKTEX_MACOS_BUNDLE)
  set_target_properties(console_qt
    PROPERTIES
      OUTPUT_NAME ${MIKTEX_PRODUCT_NAME}
      MACOSX_BUNDLE_SHORT_VERSION_STRING "${MIKTEX_MAJOR_VERSION}.${MIKTEX_MINOR_VERSION}.${MIKTEX_J2000_VERSION}"
      MACOSX_BUNDLE_GUI_IDENTIFIER "org.miktex.miktex"
  )
endif()
install(TARGETS console_qt
  RUNTIME DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR}
  BUNDLE DESTINATION .
)

# stibizt: https://github.com/Kitware/CMake/blob/master/Source/QtDialog/CMakeLists.txt
if(MIKTEX_MACOS_BUNDLE)
  set(fixup_exe "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MIKTEX_MACOS_BUNDLE_DIR}/MacOS/${MIKTEX_PRODUCT_NAME}")
  # FIXME: icu lib dir
  set(libs "/usr/local/opt/icu4c/lib;\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MIKTEX_LIBRARY_DESTINATION_DIR}")
  install(CODE "
    include(BundleUtilities)
    set(BU_CHMOD_BUNDLE_ITEMS ON)
    fixup_bundle(\"${fixup_exe}\" \"${QT_PLUGINS}\" \"${Qt_BIN_DIR};${QT_LIBRARY_DIR};${QT_BINARY_DIR};${libs}\")
  ")
endif()

if(MIKTEX_NATIVE_WINDOWS)
  add_executable(console_qt${MIKTEX_ADMIN_SUFFIX} WIN32 ${sources})
  set_property(TARGET console_qt${MIKTEX_ADMIN_SUFFIX} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  add_link_flags(console_qt${MIKTEX_ADMIN_SUFFIX} "/MANIFESTUAC:level='requireAdministrator'")
  target_link_libraries(console_qt${MIKTEX_ADMIN_SUFFIX} ${libs})
  install(TARGETS console_qt${MIKTEX_ADMIN_SUFFIX}
    RUNTIME DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR}
  )
endif()
