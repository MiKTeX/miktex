## CMakeLists.txt
##
## Copyright (C) 2010-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(twpythonplugin_sources
    ${projdir}/source/plugins-src/TWPythonPlugin/PythonScript.cpp
    ${projdir}/source/plugins-src/TWPythonPlugin/PythonScript.h
    ${projdir}/source/plugins-src/TWPythonPlugin/PythonScriptInterface.cpp
    ${projdir}/source/plugins-src/TWPythonPlugin/PythonScriptInterface.h
)

add_library(twpythonplugin-static STATIC
    ${twpythonplugin_sources}
)

set_property(TARGET twpythonplugin-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(twpythonplugin-static
    PUBLIC
        ${core_dll_name}
        Qt5::Core
        Qt5::Gui
        Qt5::UiTools
        Qt5::Widgets
)
