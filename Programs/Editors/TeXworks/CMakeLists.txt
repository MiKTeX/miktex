## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2009-2020 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_EDITORS_FOLDER}/TeXworks")

set(projdir ${CMAKE_CURRENT_SOURCE_DIR})

file(STRINGS source/src/TWVersion.h TeXworks_VER_MAJOR REGEX "VER_MAJOR")
string(REGEX MATCH "([0-9]+)" TeXworks_VER_MAJOR ${TeXworks_VER_MAJOR})
file(STRINGS source/src/TWVersion.h TeXworks_VER_MINOR REGEX "VER_MINOR")
string(REGEX MATCH "([0-9]+)" TeXworks_VER_MINOR ${TeXworks_VER_MINOR})
file(STRINGS source/src/TWVersion.h TeXworks_VER_PATCH REGEX "VER_BUGFIX")
string(REGEX MATCH "([0-9]+)" TeXworks_VER_PATCH ${TeXworks_VER_PATCH})
set(TeXworks_VERSION ${TeXworks_VER_MAJOR}.${TeXworks_VER_MINOR}.${TeXworks_VER_PATCH})

set(TW_BUILD_ID "MiKTeX")

set(CMAKE_AUTOMOC TRUE)

include_directories(BEFORE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DNOMINMAX
  )
endif()

add_definitions(
  -DHAVE_POPPLER_XPDF_HEADERS
  -DQT_STATICPLUGIN
  -DQT_VERSION_MAJOR=5
  -DTW_BUILD_ID=${TW_BUILD_ID}
  -DUSE_POPPLERQT
)

add_definitions(
  -DSTATIC_LUA_SCRIPTING_PLUGIN
)

#add_definitions(
#  -DSTATIC_PYTHON_SCRIPTING_PLUGIN
#)

set(texworks_sources
  source/src/BibTeXFile.cpp
  source/src/BibTeXFile.h
  source/src/CitationSelectDialog.cpp
  source/src/CitationSelectDialog.h
  source/src/ClickableLabel.cpp
  source/src/ClickableLabel.h
  source/src/ClosableTabWidget.cpp
  source/src/ClosableTabWidget.h
  source/src/CommandlineParser.cpp
  source/src/CommandlineParser.h
  source/src/CompletingEdit.cpp
  source/src/CompletingEdit.h
  source/src/ConfirmDelete.cpp
  source/src/ConfirmDelete.h
  source/src/DefaultPrefs.h
  source/src/Engine.cpp
  source/src/Engine.h
  source/src/FindDialog.cpp
  source/src/FindDialog.h
  source/src/HardWrapDialog.cpp
  source/src/HardWrapDialog.h
  source/src/InterProcessCommunicator.h
  source/src/PDFDocumentWindow.cpp
  source/src/PDFDocumentWindow.h
  source/src/PrefsDialog.cpp
  source/src/PrefsDialog.h
  source/src/ResourcesDialog.cpp
  source/src/ResourcesDialog.h
  source/src/ScreenCalibrationWidget.cpp
  source/src/ScreenCalibrationWidget.h
  source/src/ScriptManagerWidget.cpp
  source/src/ScriptManagerWidget.h
  source/src/Settings.cpp
  source/src/Settings.h
  source/src/TWApp.cpp
  source/src/TWApp.h
  source/src/TWScriptManager.cpp
  source/src/TWScriptManager.h
  source/src/TWScriptableWindow.cpp
  source/src/TWScriptableWindow.h
  source/src/TWSynchronizer.cpp
  source/src/TWSynchronizer.h
  source/src/TWSystemCmd.cpp
  source/src/TWSystemCmd.h
  source/src/TWTextCodecs.cpp
  source/src/TWTextCodecs.h
  source/src/TWUtils.cpp
  source/src/TWUtils.h
  source/src/TWVersion.h
  source/src/TeXDocks.cpp
  source/src/TeXDocks.h
  source/src/TeXDocumentWindow.cpp
  source/src/TeXDocumentWindow.h
  source/src/TeXHighlighter.cpp
  source/src/TeXHighlighter.h
  source/src/TemplateDialog.cpp
  source/src/TemplateDialog.h
  source/src/document/Document.h
  source/src/document/SpellChecker.cpp
  source/src/document/SpellChecker.h
  source/src/document/TeXDocument.cpp
  source/src/document/TeXDocument.h
  source/src/document/TextDocument.cpp
  source/src/document/TextDocument.h
  source/src/main.cpp
  source/src/scripting/JSScript.cpp
  source/src/scripting/JSScript.h
  source/src/scripting/JSScriptInterface.cpp
  source/src/scripting/JSScriptInterface.h
  source/src/scripting/Script.cpp
  source/src/scripting/Script.h
  source/src/scripting/ScriptAPI.cpp
  source/src/scripting/ScriptAPI.h
  source/src/scripting/ScriptAPIInterface.h
  source/src/scripting/ScriptLanguageInterface.h
  source/src/ui/LineNumberWidget.cpp
  source/src/ui/LineNumberWidget.h
  source/src/utils/FileVersionDatabase.cpp
  source/src/utils/FileVersionDatabase.h

  miktex/miktex-texworks.cpp
  miktex/miktex-texworks.hpp
  texworks-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND texworks_sources
    source/src/InterProcessCommunicatorWin.cpp
  )
elseif(APPLE)
  list(APPEND texworks_sources
    source/src/InterProcessCommunicatorDummy.cpp
  )
else()
  list(APPEND texworks_sources
    source/src/InterProcessCommunicatorDBus.cpp
  )
endif()

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND texworks_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    texworks.rc
  )
endif()

set(texworks_ui_files
  source/src/CitationSelectDialog.ui
  source/src/CompletingEdit.ui
  source/src/ConfirmDelete.ui
  source/src/Find.ui
  source/src/HardWrapDialog.ui
  source/src/PDFDocumentWindow.ui
  source/src/PDFFind.ui
  source/src/PrefsDialog.ui
  source/src/Replace.ui
  source/src/ResourcesDialog.ui
  source/src/ScriptManagerWidget.ui
  source/src/SearchResults.ui
  source/src/TeXDocumentWindow.ui
  source/src/TemplateDialog.ui
  source/src/ToolConfig.ui
)

qt5_wrap_ui(texworks_ui_headers ${texworks_ui_files})

set(texworks_ts_files
  source/trans/TeXworks_af.ts
  source/trans/TeXworks_ar.ts
  source/trans/TeXworks_ca.ts
  source/trans/TeXworks_cs.ts
  source/trans/TeXworks_de.ts
  source/trans/TeXworks_es.ts
  source/trans/TeXworks_fa.ts
  source/trans/TeXworks_fo.ts
  source/trans/TeXworks_fr.ts
  source/trans/TeXworks_it.ts
  source/trans/TeXworks_ja.ts
  source/trans/TeXworks_ko.ts
  source/trans/TeXworks_nl.ts
  source/trans/TeXworks_nn.ts
  source/trans/TeXworks_pl.ts
  source/trans/TeXworks_pt_BR.ts
  source/trans/TeXworks_ru.ts
  source/trans/TeXworks_sl.ts
  source/trans/TeXworks_tr.ts
  source/trans/TeXworks_zh_CN.ts
)

qt5_add_translation(texworks_qm_sources ${texworks_ts_files})

configure_file(TeXworks_trans.qrc.in ${CMAKE_CURRENT_BINARY_DIR}/TeXworks_trans.qrc)

set(texworks_qrc_files
  ${CMAKE_CURRENT_BINARY_DIR}/TeXworks_trans.qrc
  miktex.qrc
  source/res/resfiles.qrc
  source/res/resources.qrc
)

qt5_add_resources(texworks_rcc_sources ${texworks_qrc_files})

set(libs
  twluaplugin-static
  #twpythonplugin-static
  qtpdf-static
  synctex-static
  ${core_dll_name}
  Qt5::Concurrent
  Qt5::Core
  Qt5::Gui
  Qt5::PrintSupport
  Qt5::UiTools
  Qt5::Widgets
  Qt5::Script
  Qt5::ScriptTools
  Qt5::Xml
)

if(MIKTEX_UNIX_ALIKE AND NOT MIKTEX_MACOS_BUNDLE)
  list(APPEND libs Qt5::DBus)
endif()

if(USE_SYSTEM_HUNSPELL)
  list(APPEND libs MiKTeX::Imported::HUNSPELL)
else()
  list(APPEND libs ${hunspell_dll_name})
endif()

if(USE_SYSTEM_LOG4CXX)
  list(APPEND libs MiKTeX::Imported::LOG4CXX)
else()
  list(APPEND libs ${log4cxx_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND libs
    ${utf8wrap_dll_name}
    Shlwapi.lib
  )
endif()

if(APPLE)
  list(APPEND libs
    "-framework CoreServices"
  )
endif()

add_executable(${MIKTEX_PREFIX}texworks WIN32
  ${texworks_sources}
  ${texworks_ui_headers}
  ${texworks_rcc_sources}
  ${texworks_qm_files}
)

set_property(TARGET ${MIKTEX_PREFIX}texworks PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}texworks ${libs})

install(TARGETS ${MIKTEX_PREFIX}texworks DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

add_subdirectory(plugins/lua)
#add_subdirectory(plugins/python)

add_subdirectory(modules/QtPDF)
add_subdirectory(modules/synctex)
