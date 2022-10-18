## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}")

use_static_crt()

set(touch_sources
    touch-version.h
    touch.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND touch_sources
        ${MIKTEX_COMMON_MANIFEST}
        touch.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}touch ${touch_sources})

set_property(TARGET ${MIKTEX_PREFIX}touch PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}touch
    ${core_lib_name}
    miktex-popt-wrapper-static
)
