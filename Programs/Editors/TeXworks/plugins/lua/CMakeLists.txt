## CMakeLists.txt
##
## Copyright (C) 2010-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(twluaplugin_sources
    ${projdir}/source/plugins-src/TWLuaPlugin/LuaScript.cpp
    ${projdir}/source/plugins-src/TWLuaPlugin/LuaScript.h
    ${projdir}/source/plugins-src/TWLuaPlugin/LuaScriptInterface.cpp
    ${projdir}/source/plugins-src/TWLuaPlugin/LuaScriptInterface.h
)

add_library(twluaplugin-static STATIC
    ${twluaplugin_sources}
)

set_property(TARGET twluaplugin-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(twluaplugin-static
    PUBLIC
        ${core_dll_name}
        ${lua53_dll_name}
        Qt${QT_SERIES}::Core
        Qt${QT_SERIES}::Gui
        Qt${QT_SERIES}::UiTools
        Qt${QT_SERIES}::Widgets
)

if(QT_SERIES STREQUAL "6")
    target_link_libraries(twluaplugin-static
        PRIVATE
            Qt${QT_SERIES}::Core5Compat
    )
endif()
