## CMakeLists.txt
##
## Copyright (C) 2017-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_ADMIN_FOLDER}/Config")

if(MIKTEX_CORE_AUTO_ADMIN)
    set(Core_AutoAdmin "t")
else()
    set(Core_AutoAdmin "?")
endif()

if(MIKTEX_MPM_AUTO_ADMIN)
    set(MPM_AutoAdmin "t")
else()
    set(MPM_AutoAdmin "?")
endif()

if(MIKTEX_MPM_AUTO_INSTALL)
    set(MPM_AutoInstall "t")
else()
    set(MPM_AutoInstall "?")
endif()

configure_file(
    miktex-config.ini.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-config.ini
    NEWLINE_STYLE UNIX
)
