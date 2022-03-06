## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER ${MIKTEX_CURRENT_FOLDER}/pPLtoTF)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(ppltotf_target_name ppltotf)

include(webify.cmake)

set(ppltotf_web_file ${CMAKE_CURRENT_BINARY_DIR}/ppltotf-final.web)

create_web_app(pPLtoTF)

if(LINK_EVERYTHING_STATICALLY)
    target_link_libraries(${MIKTEX_PREFIX}ppltotf
        texjp-kanji
    )
else()
    target_link_libraries(${ppltotf_target_name}
        PRIVATE
            texjp-kanji
    )
endif()
