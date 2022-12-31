## CMakeLists.txt
##
## Copyright (C) 2018-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/nlohmann-json")

set(interface_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/source/single_include
)

add_library(${nlohmann_json_dll_name} INTERFACE)

target_include_directories(${nlohmann_json_dll_name}
    INTERFACE
        ${interface_include_directories}
)

add_library(${nlohmann_json_lib_name} INTERFACE)

target_include_directories(${nlohmann_json_lib_name}
    INTERFACE
        ${interface_include_directories}
)
