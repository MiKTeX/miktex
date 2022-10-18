## CMakeLists.txt
##
## Copyright (C) 2008-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}")

set(inipool_sources
    inipool.cpp
)

add_executable(inipool ${inipool_sources})

set_property(TARGET inipool PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
