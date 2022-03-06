## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER ${MIKTEX_CURRENT_FOLDER}/upBibTeX)

default_char_type_is_unsigned()

list(APPEND C4P_FLAGS
    --chars-are-unsigned
)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(upbibtex_target_name upbibtex)

configure_file(
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_BIBTEX_DIR}/dyn.sed
    ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed
    COPYONLY
)

include(webify.cmake)

set(upbibtex_web_file ${CMAKE_CURRENT_BINARY_DIR}/upbibtex-final.web)

create_web_app(upBibTeX)

if(LINK_EVERYTHING_STATICALLY)
    target_link_libraries(${MIKTEX_PREFIX}upbibtex
        texjp-ukanji
    )
else()
    target_link_libraries(${upbibtex_target_name}
        PRIVATE
            texjp-ukanji
    )
endif()
