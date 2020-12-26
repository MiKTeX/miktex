## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2017-2020 Christian Schenk
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

set(console_sources
  FormatDefinitionDialog.cpp
  FormatDefinitionDialog.h
  FormatTableModel.cpp
  FormatTableModel.h
  LanguageTableModel.cpp
  LanguageTableModel.h
  PackageProxyModel.cpp
  PackageProxyModel.h
  PackageTableModel.cpp
  PackageTableModel.h
  RepositoryTableModel.cpp
  RepositoryTableModel.h
  RootTableModel.cpp
  RootTableModel.h
  UpdateTableModel.cpp
  UpdateTableModel.h
  console-version.h
  main.cpp
  mainwindow.cpp
  mainwindow.h
)

list(APPEND l_sources ${console_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    win/console.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/console.rc
  )
  list(APPEND console_sources
    ${CMAKE_CURRENT_BINARY_DIR}/console.rc
    ${MIKTEX_COMMON_MANIFEST}
  )
endif()

if(MIKTEX_MACOS_BUNDLE)
  # FIXME
  set(MACOSX_BUNDLE_ICON_FILE miktex-console.icns)
  list(APPEND console_sources
    ${MIKTEX_CONSOLE_ICNS}
  )
  set_source_files_properties(${MIKTEX_CONSOLE_ICNS}
    PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources
  )
endif()

set(console_moc_sources
  FormatDefinitionDialog.h
  FormatTableModel.h
  LanguageTableModel.h
  PackageProxyModel.h
  PackageTableModel.h
  RepositoryTableModel.h
  RootTableModel.h
  UpdateTableModel.h
  mainwindow.h
)

set(console_ui_files
  formatdefinitiondialog.ui
  mainwindow.ui
)

list(APPEND l_sources ${console_ui_files})

foreach(l ${MIKTEX_TRANSLATION_LOCALES})
  list(APPEND translations
    translations/console_${l}.ts
  )
  set(qresource_files
    "${qresource_files}<file alias='console_${l}.qm'>${CMAKE_CURRENT_BINARY_DIR}/console_${l}.qm</file>"
  )
endforeach()

get_target_property(LUPDATE_EXECUTABLE Qt5::lupdate LOCATION)

add_custom_target(gen-console-translations
  COMMAND
    ${LUPDATE_EXECUTABLE} ${l_sources} -ts ${translations}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
)

qt5_add_translation(qm_files ${translations})

configure_file(
  translations.qrc.in
  ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
)

set(console_qrc_files
  ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
  console.qrc
)

qt5_wrap_cpp(console_mocs ${console_moc_sources})

qt5_wrap_ui(console_ui_headers ${console_ui_files})

qt5_wrap_cpp(console_ui_mocs ${console_ui_headers})

qt5_add_resources(console_rcc_sources ${console_qrc_files})

set(sources
  ${console_sources}
  ${console_mocs}
  ${console_ui_mocs}
  ${console_rcc_sources}
)

set(libs
  ${core_dll_name}
  ${mpm_dll_name}
  ${setup_dll_name}
  ${ui_qt_dll_name}
  Qt5::Widgets
  miktex-popt-wrapper
)

if (USE_SYSTEM_LOG4CXX)
  list(APPEND libs MiKTeX::Imported::LOG4CXX)
else()
  list(APPEND libs ${log4cxx_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND libs
    Comctl32.lib
  )
endif()

# stibizt: https://github.com/Kitware/CMake/blob/master/Source/QtDialog/CMakeLists.txt
if(MIKTEX_MACOS_BUNDLE)
  install_qt5_plugin("Qt5::QCocoaIntegrationPlugin" QT_PLUGINS)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    "[Paths]\nPlugins = PlugIns\n")
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    DESTINATION "${MIKTEX_MACOS_BUNDLE_DIR}/Resources"
  )
  get_property(_Qt5_Core_LOCATION TARGET Qt5::Core PROPERTY LOCATION)
  get_filename_component(Qt_BIN_DIR "${_Qt5_Core_LOCATION}" PATH)
  get_filename_component(Qt_BIN_DIR "${Qt_BIN_DIR}" PATH)
endif()

add_executable(${MIKTEX_PROG_NAME_CONSOLE} WIN32 MACOSX_BUNDLE ${sources})
set_property(TARGET ${MIKTEX_PROG_NAME_CONSOLE} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(${MIKTEX_PROG_NAME_CONSOLE} ${libs})  
if(MIKTEX_MACOS_BUNDLE)
  set_target_properties(${MIKTEX_PROG_NAME_CONSOLE}
    PROPERTIES
      OUTPUT_NAME ${MIKTEX_MACOS_BUNDLE_NAME}
      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist.in"
      MACOSX_BUNDLE_BUNDLE_NAME ${MIKTEX_MACOS_BUNDLE_NAME}
      MACOSX_BUNDLE_SHORT_VERSION_STRING "${MIKTEX_MAJOR_VERSION}.${MIKTEX_MINOR_VERSION}.${MIKTEX_PATCH_VERSION}"
      MACOSX_BUNDLE_GUI_IDENTIFIER "org.miktex.console"
  )
endif()
if(UNIX AND NOT APPLE)
  # Gnome
  install(
    FILES "${CMAKE_SOURCE_DIR}/Resources/Icons/MiKTeX/128x128/apps/miktex-console.png"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pixmaps"
  )
  # KDE
  foreach (size IN ITEMS 32 128)
    install(
      FILES "${CMAKE_SOURCE_DIR}/Resources/Icons/MiKTeX/${size}x${size}/apps/miktex-console.png"
      DESTINATION "${MIKTEX_XDG_APPLICATIONS_DIR}/icons/hicolor/${size}x${size}/apps"
    )
  endforeach()
  install(
    FILES unx/miktex-console.desktop
    DESTINATION "${MIKTEX_XDG_APPLICATIONS_DIR}"
  )
  install(
    FILES unx/miktex-console.policy
    DESTINATION "${MIKTEX_POLKIT_ACTIONS_DIR}"
  )
endif()
install(TARGETS miktex-console
  RUNTIME DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR}
  BUNDLE DESTINATION .
)

# stibizt: https://github.com/Kitware/CMake/blob/master/Source/QtDialog/CMakeLists.txt
if(MIKTEX_MACOS_BUNDLE)
  set(fixup_exe "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MIKTEX_MACOS_BUNDLE_DIR}/MacOS/${MIKTEX_MACOS_BUNDLE_NAME}")
  # FIXME: icu lib dir
  set(libs "/usr/local/opt/icu4c/lib;\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MIKTEX_LIBRARY_DESTINATION_DIR}")
  install(CODE "
    include(${CMAKE_SOURCE_DIR}/cmake/modules/BundleUtilitiesOverrides.cmake)
    include(BundleUtilities)
    set(BU_CHMOD_BUNDLE_ITEMS ON)
    fixup_bundle(\"${fixup_exe}\" \"${QT_PLUGINS}\" \"${Qt_BIN_DIR};${QT_LIBRARY_DIR};${QT_BINARY_DIR};${libs}\")
  ")
endif()

if(MIKTEX_NATIVE_WINDOWS)
  add_executable(${MIKTEX_PROG_NAME_CONSOLE_ADMIN} WIN32 ${sources})
  set_property(TARGET ${MIKTEX_PROG_NAME_CONSOLE_ADMIN} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  add_link_flags(${MIKTEX_PROG_NAME_CONSOLE_ADMIN} "/MANIFESTUAC:level='requireAdministrator'")
  target_link_libraries(${MIKTEX_PROG_NAME_CONSOLE_ADMIN} ${libs})
  install(TARGETS ${MIKTEX_PROG_NAME_CONSOLE_ADMIN}
    RUNTIME DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR}
  )
endif()

if(MIKTEX_MACOS_BUNDLE)
  install(
    PROGRAMS
      mac/miktex-console_admin
      mac/Terminal
    DESTINATION "${MIKTEX_MACOS_BUNDLE_DIR}/Resources"
  )
endif()

if(MIKTEX_MACOS_BUNDLE AND WITH_CODE_SIGNING)
  configure_file(
    ${CMAKE_SOURCE_DIR}/BuildUtilities/sign/mac/codesign_executable.in
    ${CMAKE_BINARY_DIR}/BuildUtilities/sign/mac/tmp/codesign_executable
    @ONLY
  )
  configure_file(
    ${CMAKE_SOURCE_DIR}/BuildUtilities/sign/mac/codesign_miktex.in
    ${CMAKE_BINARY_DIR}/BuildUtilities/sign/mac/tmp/codesign_miktex
    @ONLY
  )
  file(
    COPY
      ${CMAKE_BINARY_DIR}/BuildUtilities/sign/mac/tmp/codesign_executable
      ${CMAKE_BINARY_DIR}/BuildUtilities/sign/mac/tmp/codesign_miktex
    DESTINATION
      ${CMAKE_BINARY_DIR}/BuildUtilities/sign/mac
    FILE_PERMISSIONS
      OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )
  install(CODE
    "execute_process(COMMAND \"${CMAKE_BINARY_DIR}/BuildUtilities/sign/mac/codesign_miktex\" \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MIKTEX_MACOS_BUNDLE_NAME}.app\")"
  )
endif()
