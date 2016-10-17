## CMakeLists.txt                                       -*- CMake -*-
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_EDITORS_FOLDER}/TeXworks")

set(projdir ${CMAKE_CURRENT_SOURCE_DIR})

file(STRINGS source/src/TWVersion.h TeXworks_VER_MAJOR REGEX "VER_MAJOR")
string(REGEX MATCH "([0-9]+)" TeXworks_VER_MAJOR ${TeXworks_VER_MAJOR})
file(STRINGS source/src/TWVersion.h TeXworks_VER_MINOR REGEX "VER_MINOR")
string(REGEX MATCH "([0-9]+)" TeXworks_VER_MINOR ${TeXworks_VER_MINOR})
file(STRINGS source/src/TWVersion.h TeXworks_VER_PATCH REGEX "VER_BUGFIX")
string(REGEX MATCH "([0-9]+)" TeXworks_VER_PATCH ${TeXworks_VER_PATCH})
set(TeXworks_VERSION ${TeXworks_VER_MAJOR}.${TeXworks_VER_MINOR}.${TeXworks_VER_PATCH})

configure_file(
  source/GitArchiveInfo.txt
  ${CMAKE_CURRENT_BINARY_DIR}/GitRev.h
  COPYONLY
)

file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/GitRev.h TeXworks_GIT_INFO)
list(GET TeXworks_GIT_INFO 1 TeXworks_GIT_COMMIT)
string(REGEX REPLACE "#define GIT_COMMIT_HASH \"([a-f0-9]+)\"" "\\1" TeXworks_GIT_HASH "${TeXworks_GIT_COMMIT}")
list(GET TeXworks_GIT_INFO 2 TeXworks_GIT_COMMIT)
string(REGEX REPLACE "#define GIT_COMMIT_DATE \"([-:0-9 Z]+)\"" "\\1" TeXworks_GIT_DATE "${TeXworks_GIT_COMMIT}")

set(TW_BUILD_ID "MiKTeX")

set(CMAKE_AUTOMOC TRUE)

include_directories(BEFORE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src
)

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
  source/src/ClickableLabel.cpp
  source/src/ClickableLabel.h
  source/src/ClosableTabWidget.cpp
  source/src/ClosableTabWidget.h
  source/src/CommandlineParser.cpp
  source/src/CommandlineParser.h
  source/src/CompletingEdit.cpp
  source/src/CompletingEdit.h
  source/src/ConfigurableApp.cpp
  source/src/ConfigurableApp.h
  source/src/ConfirmDelete.cpp
  source/src/ConfirmDelete.h
  source/src/DefaultBinaryPathsWin.h
  source/src/DefaultPrefs.h
  source/src/FindDialog.cpp
  source/src/FindDialog.h
  source/src/HardWrapDialog.cpp
  source/src/HardWrapDialog.h
  source/src/PDFDocument.cpp
  source/src/PDFDocument.h
  source/src/PrefsDialog.cpp
  source/src/PrefsDialog.h
  source/src/ResourcesDialog.cpp
  source/src/ResourcesDialog.h
  source/src/ScriptManager.cpp
  source/src/ScriptManager.h
  source/src/TWApp.cpp
  source/src/TWApp.h
  source/src/TWScript.cpp
  source/src/TWScript.h
  source/src/TWScriptAPI.cpp
  source/src/TWScriptAPI.h
  source/src/TWScriptable.cpp
  source/src/TWScriptable.h
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
  source/src/TeXDocument.cpp
  source/src/TeXDocument.h
  source/src/TeXHighlighter.cpp
  source/src/TeXHighlighter.h
  source/src/TemplateDialog.cpp
  source/src/TemplateDialog.h
  source/src/main.cpp
  source/src/synctex_parser.c
  source/src/synctex_parser.h
  source/src/synctex_parser_utils.c
  source/src/synctex_parser_utils.h

  ${CMAKE_CURRENT_BINARY_DIR}/GitRev.h

  miktex/miktex-texworks.cpp
  miktex/miktex-texworks.h
  miktex/miktex-texworks.hpp
  texworks-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  set(texworks_sources ${texworks_sources}
    texworks.rc
  )
endif()

set(texworks_ui_files
  source/src/ConfirmDelete.ui
  source/src/Find.ui
  source/src/HardWrapDialog.ui
  source/src/PDFDocument.ui
  source/src/PDFFind.ui
  source/src/PrefsDialog.ui
  source/src/Replace.ui
  source/src/ResourcesDialog.ui
  source/src/ScriptManager.ui
  source/src/SearchResults.ui
  source/src/TeXDocument.ui
  source/src/TemplateDialog.ui
  source/src/ToolConfig.ui
)

set(texworks_rcs_files
  TeXworks_trans.qrc
  miktex.qrc
  source/res/resfiles.qrc
  source/res/resources.qrc
)

qt5_wrap_ui(texworks_ui_headers ${texworks_ui_files})

qt5_add_resources(texworks_rcc_sources ${texworks_rcs_files})

set(libs
  twluaplugin-static
  #twpythonplugin-static
  qtpdf-static
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

if(USE_SYSTEM_ZLIB)
  list(APPEND libs MiKTeX::Imported::ZLIB)
else()
  list(APPEND libs ${zlib_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND libs
    ${utf8wrap_dll_name}
    Shlwapi.lib
  )
endif()

add_executable(${MIKTEX_PREFIX}texworks WIN32
  ${texworks_sources}
  ${texworks_ui_headers}
  ${texworks_rcc_sources}
)

set_property(TARGET ${MIKTEX_PREFIX}texworks PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}texworks ${libs})

merge_trustinfo_manifest(${MIKTEX_PREFIX}texworks asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}texworks)

install(TARGETS ${MIKTEX_PREFIX}texworks DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

add_subdirectory(plugins/lua)
#add_subdirectory(plugins/python)

add_subdirectory(modules/QtPDF)
