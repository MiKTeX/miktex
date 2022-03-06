## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER ${MIKTEX_CURRENT_FOLDER}/pDVItype)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(pdvitype_target_name pdvitype)

include(webify.cmake)

set(pdvitype_web_file ${CMAKE_CURRENT_BINARY_DIR}/pdvitype-final.web)

create_web_app(pDVItype)

if(LINK_EVERYTHING_STATICALLY)
    target_link_libraries(${MIKTEX_PREFIX}pdvitype
        texjp-kanji
    )
else()
    target_link_libraries(${pdvitype_target_name}
        PRIVATE
            texjp-kanji
    )
endif()
