## CMakeLists.txt
##
## Copyright (C) 2020-2026 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/texmf/test")

default_char_type_is_signed()

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

create_web_app(TestWebApp)
