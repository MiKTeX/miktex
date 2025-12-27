## CMakeLists.txt
##
## Copyright (C) 2006-2025 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BIBLIOGRAPHY_FOLDER}/BibTeX")

default_char_type_is_unsigned()

set(C4P_FLAGS
    --chars-are-unsigned
)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(bibtex_sources
    ${CMAKE_BINARY_DIR}/include/miktex/bibtex.defaults.h
)

create_web_app(BibTeX)

add_dependencies(bibtex gen-defaults)

set_source_files_properties(
    ${CMAKE_BINARY_DIR}/include/miktex/bibtex.defaults.h
    PROPERTIES
        GENERATED TRUE
)

target_link_libraries(bibtex
    PRIVATE
        ${w2cemu_dll_name}
)

# Last but not least: developer's convenience

set(web_files ${CMAKE_CURRENT_SOURCE_DIR}/source/bibtex.web)

foreach(_path ${web_files})
    get_filename_component(_name ${_path} NAME_WE)
    set(_out ${CMAKE_CURRENT_BINARY_DIR}/${_name}-n.web)
    add_custom_command(
        OUTPUT
            ${_out}
        COMMAND
            web-n < ${_path} > ${_out}
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}
        MAIN_DEPENDENCY
            ${_path}
        DEPENDS
            web-n
        VERBATIM
    )
    list(APPEND web_n_files ${_out})
endforeach()

add_custom_target(bibtex-dev ALL DEPENDS ${web_n_files})

set_property(TARGET bibtex-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
