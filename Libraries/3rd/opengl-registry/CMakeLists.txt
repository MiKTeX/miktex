## CMakeLists.txt
##
## Copyright (C) 2018-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/opengl-registry")

set(interface_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api
)

add_library(${opengl_registry_dll_name} INTERFACE)

target_link_libraries(${opengl_registry_dll_name}
    INTERFACE
        ${egl_registry_dll_name}
)

target_include_directories(${opengl_registry_dll_name}
    INTERFACE
        ${interface_include_directories}
)

add_library(${opengl_registry_lib_name} INTERFACE)

target_link_libraries(${opengl_registry_lib_name}
    INTERFACE
        ${egl_registry_lib_name}
)

target_include_directories(${opengl_registry_lib_name}
    INTERFACE
        ${interface_include_directories}
)
