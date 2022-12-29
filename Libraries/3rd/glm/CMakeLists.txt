## CMakeLists.txt
##
## Copyright (C) 2020-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/glm")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

set(glm_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/glm/detail/glm.cpp
)

configure_file(
    miktex-glm-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-glm-version.h
)

list(APPEND glm_sources
  ${CMAKE_CURRENT_BINARY_DIR}/miktex-glm-version.h
)

add_subdirectory(static)
