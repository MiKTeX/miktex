## CMakeLists.txt
##
## Copyright (C) 2009-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

file(STRINGS source/src/TWVersion.h TeXworks_VER_MAJOR REGEX "VER_MAJOR")
string(REGEX MATCH "([0-9]+)" TeXworks_VER_MAJOR ${TeXworks_VER_MAJOR})
file(STRINGS source/src/TWVersion.h TeXworks_VER_MINOR REGEX "VER_MINOR")
string(REGEX MATCH "([0-9]+)" TeXworks_VER_MINOR ${TeXworks_VER_MINOR})
file(STRINGS source/src/TWVersion.h TeXworks_VER_PATCH REGEX "VER_BUGFIX")
string(REGEX MATCH "([0-9]+)" TeXworks_VER_PATCH ${TeXworks_VER_PATCH})
set(TeXworks_VERSION ${TeXworks_VER_MAJOR}.${TeXworks_VER_MINOR}.${TeXworks_VER_PATCH})

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_EDITORS_FOLDER}/TeXworks")

set(projdir ${CMAKE_CURRENT_SOURCE_DIR})

set(TW_BUILD_ID "MiKTeX")

set(WITH_QTSCRIPT TRUE)

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
#    -DSTATIC_PYTHON_SCRIPTING_PLUGIN
#)

configure_file(
    miktex-texworks-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-texworks-version.h
)

configure_file(
    miktex-texworks.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-texworks.rc
)

set(texworks_sources
    source/src/BibTeXFile.cpp
    source/src/CitationSelectDialog.cpp
    source/src/CompletingEdit.cpp
    source/src/Engine.cpp
    source/src/FindDialog.cpp
    source/src/HardWrapDialog.cpp
    source/src/main.cpp
    source/src/PDFDocumentWindow.cpp
    source/src/PrefsDialog.cpp
    source/src/ResourcesDialog.cpp
    source/src/ScriptManagerWidget.cpp
    source/src/Settings.cpp
    source/src/TemplateDialog.cpp
    source/src/TeXDocks.cpp
    source/src/TeXDocumentWindow.cpp
    source/src/TeXHighlighter.cpp
    source/src/TWApp.cpp
    source/src/TWScriptableWindow.cpp
    source/src/TWScriptManager.cpp
    source/src/TWSynchronizer.cpp
    source/src/TWUtils.cpp
    source/src/document/Document.cpp
    source/src/document/SpellChecker.cpp
    source/src/document/TextDocument.cpp
    source/src/document/TeXDocument.cpp
    source/src/scripting/ECMAScriptInterface.cpp
    source/src/scripting/ECMAScript.cpp
    source/src/scripting/ScriptAPI.cpp
    source/src/scripting/Script.cpp
    source/src/ui/ClickableLabel.cpp
    source/src/ui/ClosableTabWidget.cpp
    source/src/ui/LineNumberWidget.cpp
    source/src/ui/ListSelectDialog.cpp
    source/src/ui/RemoveAuxFilesDialog.cpp
    source/src/ui/ScreenCalibrationWidget.cpp
    source/src/utils/CommandlineParser.cpp
    source/src/utils/FileVersionDatabase.cpp
    source/src/utils/FullscreenManager.cpp
    source/src/utils/ResourcesLibrary.cpp
    source/src/utils/SystemCommand.cpp
    source/src/utils/TextCodecs.cpp
    source/src/utils/TypesetManager.cpp
    source/src/utils/VersionInfo.cpp
    source/src/BibTeXFile.h
    source/src/CitationSelectDialog.h
    source/src/CompletingEdit.h
    source/src/DefaultPrefs.h
    source/src/Engine.h
    source/src/FindDialog.h
    source/src/HardWrapDialog.h
    source/src/PDFDocumentWindow.h
    source/src/PrefsDialog.h
    source/src/ResourcesDialog.h
    source/src/ScriptManagerWidget.h
    source/src/Settings.h
    source/src/TemplateDialog.h
    source/src/TeXDocks.h
    source/src/TeXDocumentWindow.h
    source/src/TeXHighlighter.h
    source/src/TWApp.h
    source/src/TWScriptableWindow.h
    source/src/TWScriptManager.h
    source/src/TWSynchronizer.h
    source/src/TWUtils.h
    source/src/TWVersion.h
    source/src/InterProcessCommunicator.h
    source/src/document/Document.h
    source/src/document/SpellChecker.h
    source/src/document/TextDocument.h
    source/src/document/TeXDocument.h
    source/src/scripting/ScriptAPIInterface.h
    source/src/scripting/ScriptLanguageInterface.h
    source/src/scripting/ScriptAPI.h
    source/src/scripting/Script.h
    source/src/ui/ClickableLabel.h
    source/src/ui/ClosableTabWidget.h
    source/src/ui/LineNumberWidget.h
    source/src/ui/ListSelectDialog.h
    source/src/ui/RemoveAuxFilesDialog.h
    source/src/ui/ScreenCalibrationWidget.h
    source/src/utils/CommandlineParser.h
    source/src/utils/FileVersionDatabase.h
    source/src/utils/FullscreenManager.h
    source/src/utils/ResourcesLibrary.h
    source/src/utils/SystemCommand.h
    source/src/utils/TextCodecs.h
    source/src/utils/TypesetManager.h
    source/src/utils/VersionInfo.h

    GitRev.h
    miktex-texworks-version.h
    miktex/miktex-texworks.cpp
    miktex/miktex-texworks.hpp
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
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-texworks.rc
        ${MIKTEX_COMMON_MANIFEST}
    )
endif()

if(WITH_QTSCRIPT)
    list(APPEND texworks_sources
        source/src/scripting/JSScript.cpp
        source/src/scripting/JSScript.h
        source/src/scripting/JSScriptInterface.cpp
        source/src/scripting/JSScriptInterface.h
    )
    add_definitions(-DWITH_QTSCRIPT)
endif()

set(texworks_ui_files
    source/src/CitationSelectDialog.ui
    source/src/CompletingEdit.ui
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
    source/src/ui/ListSelectDialog.ui
)

if(QT_SERIES STREQUAL "5")
    qt5_wrap_ui(texworks_ui_headers ${texworks_ui_files})
else()
    qt_wrap_ui(texworks_ui_headers ${texworks_ui_files})
endif()

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

if(QT_SERIES STREQUAL "5")
    qt5_add_translation(texworks_qm_sources ${texworks_ts_files})
else()
    qt_add_translation(texworks_qm_sources ${texworks_ts_files})
endif()

configure_file(TeXworks_trans.qrc.in ${CMAKE_CURRENT_BINARY_DIR}/TeXworks_trans.qrc)

set(texworks_qrc_files
    ${CMAKE_CURRENT_BINARY_DIR}/TeXworks_trans.qrc
    miktex.qrc
    source/res/resfiles.qrc
    source/res/resources.qrc
)

if(QT_SERIES STREQUAL "5")
    qt5_add_resources(texworks_rcc_sources ${texworks_qrc_files})
else()
    qt_add_resources(texworks_rcc_sources ${texworks_qrc_files})
endif(

)
set(libs
    twluaplugin-static
    #twpythonplugin-static
    qtpdf-static
    synctex-static
    ${core_dll_name}
    Qt${QT_SERIES}::Concurrent
    Qt${QT_SERIES}::Core
    Qt${QT_SERIES}::Gui
    Qt${QT_SERIES}::PrintSupport
    Qt${QT_SERIES}::Qml
    Qt${QT_SERIES}::UiTools
    Qt${QT_SERIES}::Widgets
    Qt${QT_SERIES}::Script
    Qt${QT_SERIES}::ScriptTools
    Qt${QT_SERIES}::Xml
)

if (USE_SYSTEM_FMT)
    list(APPEND libs MiKTeX::Imported::FMT)
else()
    list(APPEND libs ${fmt_dll_name})
endif()

if(MIKTEX_UNIX_ALIKE AND NOT MIKTEX_MACOS_BUNDLE)
    list(APPEND libs Qt${QT_SERIES}::DBus)
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
