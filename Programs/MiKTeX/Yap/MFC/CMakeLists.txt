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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/Yap")

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
  res/yap.manifest.in
  ${CMAKE_CURRENT_BINARY_DIR}/res/yap.manifest
)

configure_file(
  yap.rc.in
  ${CMAKE_CURRENT_BINARY_DIR}/yap.rc
)

set(yap_mfc_sources
  ${CMAKE_CURRENT_BINARY_DIR}/yaphelp.h
  AddChangeEditorDialog.cpp
  AddChangeEditorDialog.h
  AdvancedOptionsPage.cpp
  AdvancedOptionsPage.h
  ChildFrame.cpp
  ChildFrame.h
  DisplayOptionsPage.cpp
  DisplayOptionsPage.h
  DocumentFontsPage.cpp
  DocumentFontsPage.h
  DviDoc.cpp
  DviDoc.h
  DviDraw.cpp
  DviDraw.h
  DviMagnifyingGlass.cpp
  DviMagnifyingGlass.h
  DviStatusBar.cpp
  DviStatusBar.h
  DviView.cpp
  DviView.h
  DvipsDialog.cpp
  DvipsDialog.h
  GotoPageDialog.cpp
  GotoPageDialog.h
  InverseSearchOptionsPage.cpp
  InverseSearchOptionsPage.h
  MagnifyingGlassOptionsPage.cpp
  MagnifyingGlassOptionsPage.h
  MainFrame.cpp
  MainFrame.h
  PrintOptionsPage.cpp
  PrintOptionsPage.h
  ProgressIndicatorDialog.cpp
  ProgressIndicatorDialog.h
  Ruler.cpp
  Ruler.h
  SecurityOptionsPage.cpp
  SecurityOptionsPage.h
  SourceSpecialsDialog.cpp
  SourceSpecialsDialog.h
  SplashWindow.cpp
  SplashWindow.h
  StdAfx.h
  UIOptionsPage.cpp
  UIOptionsPage.h
  YapConfig.cpp
  draw.cpp
  hypertex.cpp
  msdnutil.cpp
  print.cpp
  source.cpp
  tpic.cpp
  winplace.cpp
  winplace.h
  yap.cpp
  yap.h
  yap-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND yap_mfc_sources
    ${CMAKE_CURRENT_BINARY_DIR}/yap.rc)
endif()

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/yaphelp.h
  COMMAND
    ${CMAKE_COMMAND}
    -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/yaphelp1.h
    ${CMAKE_CURRENT_BINARY_DIR}/yaphelp.h
  COMMAND
    ${MAKEHM_EXECUTABLE}
    /h
    ID_,HID_,0x10000
    IDM_,HIDM_,0x10000
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.h
    >> ${CMAKE_CURRENT_BINARY_DIR}/yaphelp.h
  COMMAND
    ${MAKEHM_EXECUTABLE}
    /h
    IDP_,HIDP_,0x30000
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.h
    >> ${CMAKE_CURRENT_BINARY_DIR}/yaphelp.h
  COMMAND
    ${MAKEHM_EXECUTABLE}
    /h
    IDR_,HIDR_,0x20000
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.h
    >> ${CMAKE_CURRENT_BINARY_DIR}/yaphelp.h
  COMMAND
    ${MAKEHM_EXECUTABLE}
    /h
    IDD_,HIDD_,0x20000
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.h
    >> ${CMAKE_CURRENT_BINARY_DIR}/yaphelp.h
  COMMAND
    ${MAKEHM_EXECUTABLE}
    /h
    IDW_,HIDW_,0x50000
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.h
    >> ${CMAKE_CURRENT_BINARY_DIR}/yaphelp.h
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/yaphelp1.h
  VERBATIM
)

add_executable(yap WIN32 ${yap_mfc_sources})

set_property(TARGET yap PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  add_link_flags(yap "/ENTRY:\"wWinMainCRTStartup\"")
endif()

target_link_libraries(yap
  ${core_dll_name}
  ${dvi_dll_name}
  ${ui_mfc_dll_name}
  miktex-popt-wrapper
)

merge_trustinfo_manifest(yap asInvoker)
merge_compatibility_manifest(yap)

if(MIKTEX_NATIVE_WINDOWS)
  add_custom_command(
    TARGET
      yap
    POST_BUILD
    COMMAND
      ${MT_EXECUTABLE}
      -nologo
      -manifest ${CMAKE_CURRENT_BINARY_DIR}/res/yap.manifest
      -updateresource:$<TARGET_FILE:yap>\;1
    VERBATIM
  )
endif()

install(TARGETS yap DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(screenshots_dir ${CMAKE_CURRENT_SOURCE_DIR}/../Screenshots)

set(screenshots_png
  ${screenshots_dir}/propui.png
  ${screenshots_dir}/propdisplay.png
  ${screenshots_dir}/propprinter.png
  ${screenshots_dir}/propmagglass.png
  ${screenshots_dir}/propinvsearch.png
  ${screenshots_dir}/propsecurity.png
  ${screenshots_dir}/propadvanced.png
  ${screenshots_dir}/ruler.png
  ${screenshots_dir}/magglass.png
)

set(guide_dir ${CMAKE_CURRENT_SOURCE_DIR}/../Documentation/Guide)

set(yap_guide_xml_files
  ${guide_dir}/commandline.xml
  ${guide_dir}/customizing.xml
  ${guide_dir}/dvisearch.xml
  ${guide_dir}/keyboard.xml
  ${guide_dir}/menus.xml
  ${guide_dir}/printing.xml
  ${guide_dir}/tools.xml
  ${guide_dir}/yap.xml
)

set(xslt_path "${CMAKE_CURRENT_BINARY_DIR}")
set(xslt_path "${xslt_path} ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_DOC_DIR}/Manual")
set(xslt_path "${xslt_path} ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_DOC_DIR}")
set(xslt_path "${xslt_path} ${guide_dir}")

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/yap.chm
  COMMAND
    ${CMAKE_COMMAND}
    -E remove_directory
    ${CMAKE_CURRENT_BINARY_DIR}/scratch
  COMMAND
    ${CMAKE_COMMAND}
    -E copy_directory
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_DOCBOOK_XSL_DIR}/images
    ${CMAKE_CURRENT_BINARY_DIR}/scratch/images
  COMMAND
    ${CMAKE_COMMAND}
    -E copy_directory
    ${screenshots_dir}
    ${CMAKE_CURRENT_BINARY_DIR}/scratch/images
  COMMAND
    ${CMAKE_COMMAND}
    -E copy_if_different
    ${MIKTEX_HELP_STYLES}
    ${CMAKE_CURRENT_BINARY_DIR}/scratch
  COMMAND
    ${CMAKE_COMMAND}
    -E env "XML_CATALOG_FILES=${MIKTEX_XML_CATALOG_FILE}"
    ${XSLTPROC_EXECUTABLE}
    --path "${xslt_path}"
    --stringparam base.dir scratch/
    --stringparam chunk.fast 1
    --stringparam manifest.in.base.dir 1
    ${MIKTEX_DOCBOOK_HTMLHELP}
    ${guide_dir}/yap.xml
  COMMAND
    ${CMAKE_COMMAND}
    -E copy
    ${CMAKE_CURRENT_BINARY_DIR}/yaphelp.h
    ${CMAKE_CURRENT_BINARY_DIR}/scratch/context.h
  COMMAND
    ${MAKEHM_EXECUTABLE}
    /h
    /a afxhh.h
    IDW_,HIDW_,0x50000
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.h
    >> ${CMAKE_CURRENT_BINARY_DIR}/scratch/context.h
  COMMAND
    ${CMAKE_COMMAND}
    -E chdir ${CMAKE_CURRENT_BINARY_DIR}/scratch
    ${HHC_EXECUTABLE_WRAPPER} htmlhelp
  COMMAND
    ${CMAKE_COMMAND}
    -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/scratch/htmlhelp.chm
    ${CMAKE_CURRENT_BINARY_DIR}/yap.chm
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/yaphelp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.h
    ${screenshots_png}
    ${yap_guide_xml_files}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  VERBATIM
)

add_custom_target(gen-help ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/yap.chm
)

set_property(TARGET gen-help PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/yap.chm
  DESTINATION
    ${MIKTEX_DOC_DESTINATION_DIR}
)
