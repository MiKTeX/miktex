## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER ${MIKTEX_CURRENT_FOLDER}/pTFtoPL)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(ptftopl_target_name ptftopl)

include(webify.cmake)

set(ptftopl_web_file ${CMAKE_CURRENT_BINARY_DIR}/ptftopl-final.web)

create_web_app(pTFtoPL)

if(LINK_EVERYTHING_STATICALLY)
    target_link_libraries(${MIKTEX_PREFIX}ptftopl
        texjp-kanji
    )
else()
    target_link_libraries(${ptftopl_target_name}
        PRIVATE
            texjp-kanji
    )
endif()
