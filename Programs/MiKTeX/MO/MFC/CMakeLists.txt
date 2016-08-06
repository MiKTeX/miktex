## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2011-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_SETTINGS_FOLDER}/MFC")

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
  res/mo.manifest.in
  ${CMAKE_CURRENT_BINARY_DIR}/res/mo.manifest
)

configure_file(
  mo.rc.in
  ${CMAKE_CURRENT_BINARY_DIR}/mo.rc
)

set(mo_mfc_sources
  FormatDefinitionDialog.cpp
  FormatDefinitionDialog.h
  PropPageFormats.cpp
  PropPageFormats.h
  PropPageGeneral.cpp
  PropPageGeneral.h
  PropPageLanguages.cpp
  PropPageLanguages.h
  PropPagePackages.cpp
  PropPagePackages.h
  PropPageRoots.cpp
  PropPageRoots.h
  PropSheet.cpp
  PropSheet.h
  StdAfx.h
  mo-version.h
  mo.cpp
  mo.h
  resource.h
  resource.hm
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND mo_mfc_sources
    ${CMAKE_CURRENT_BINARY_DIR}/mo.rc)
endif()

set(libs
  ${core_dll_name}
  ${mpm_dll_name}
  ${ui_mfc_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND libs
    htmlhelp.lib
  )
endif()

add_executable(mo WIN32 ${mo_mfc_sources})

set_property(TARGET mo PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  add_link_flags(mo "/ENTRY:\"wWinMainCRTStartup\"")
endif()

target_link_libraries(mo ${libs})

merge_compatibility_manifest(mo)
merge_dpi_aware_manifest(mo)
merge_trustinfo_manifest(mo asInvoker)

if(MIKTEX_NATIVE_WINDOWS)
  add_custom_command(
    TARGET
      mo
    POST_BUILD
    COMMAND
      ${MT_EXECUTABLE}
      -nologo
      -manifest ${CMAKE_CURRENT_BINARY_DIR}/res/mo.manifest
      -updateresource:$<TARGET_FILE:mo>\;1
    VERBATIM
  )
endif()

install(TARGETS mo DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

add_executable(mo${MIKTEX_ADMIN_SUFFIX} WIN32 ${mo_mfc_sources})

set_property(TARGET mo${MIKTEX_ADMIN_SUFFIX} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  add_link_flags(mo${MIKTEX_ADMIN_SUFFIX} "/ENTRY:\"wWinMainCRTStartup\"")
endif()

target_link_libraries(mo${MIKTEX_ADMIN_SUFFIX} ${libs})

merge_compatibility_manifest(mo${MIKTEX_ADMIN_SUFFIX})
merge_dpi_aware_manifest(mo${MIKTEX_ADMIN_SUFFIX})
merge_trustinfo_manifest(mo${MIKTEX_ADMIN_SUFFIX} requireAdministrator)

if(MIKTEX_NATIVE_WINDOWS)
  add_custom_command(
    TARGET
      mo${MIKTEX_ADMIN_SUFFIX}
    POST_BUILD
    COMMAND
      ${MT_EXECUTABLE}
      -nologo
      -manifest ${CMAKE_CURRENT_BINARY_DIR}/res/mo.manifest
      -updateresource:$<TARGET_FILE:mo${MIKTEX_ADMIN_SUFFIX}>\;1
    VERBATIM
  )
endif()

install(TARGETS mo${MIKTEX_ADMIN_SUFFIX} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
