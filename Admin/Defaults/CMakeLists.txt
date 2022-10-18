## CMakeLists.txt
##
## Copyright (C) 2015-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_ADMIN_FOLDER}/Defaults")

set(all_defaults_cmake
    bibtex.defaults.in
    mf.defaults.in
    mfapp.defaults.in
    mpost.defaults.in
    pdftex.defaults.in
    texapp.defaults.in
    texmfapp.defaults.in
)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/miktex)

foreach(d ${all_defaults_cmake})
    get_filename_component(fn ${d} NAME_WE)
    configure_file(
        ${d}
        ${CMAKE_CURRENT_BINARY_DIR}/${fn}.defaults
        NEWLINE_STYLE UNIX
    )
    add_custom_command(
        OUTPUT
            ${CMAKE_BINARY_DIR}/include/miktex/${fn}.defaults.h
        COMMAND
            cfg
                --print-classes
                ${CMAKE_CURRENT_BINARY_DIR}/${fn}.defaults
                > ${fn}.defaults.h
        WORKING_DIRECTORY
            ${CMAKE_BINARY_DIR}/include/miktex
        MAIN_DEPENDENCY
            ${CMAKE_CURRENT_BINARY_DIR}/${fn}.defaults
        DEPENDS
            cfg
        VERBATIM
    )
    list(APPEND generated_sources ${CMAKE_BINARY_DIR}/include/miktex/${fn}.defaults.h)
endforeach()

add_custom_target(gen-defaults
    DEPENDS
        ${generated_sources}
)

set_property(TARGET gen-defaults PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
