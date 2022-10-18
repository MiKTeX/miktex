## CMakeLists.txt
##
## Copyright (C) 2007-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}")

use_static_crt()

set(padout_sources
    padout.cpp
)

add_executable(padout ${padout_sources})

set_property(TARGET padout PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(TARGETS padout DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
