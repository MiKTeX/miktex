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

set(uptftopl_target_name uptftopl)

include(webify.cmake)

set(uptftopl_web_file ${CMAKE_CURRENT_BINARY_DIR}/uptftopl-final.web)

create_web_app(upTFtoPL)

if(LINK_EVERYTHING_STATICALLY)
    target_link_libraries(${MIKTEX_PREFIX}uptftopl
        texjp-ukanji
    )
else()
    target_link_libraries(${uptftopl_target_name}
        PRIVATE
            texjp-ukanji
    )
endif()
