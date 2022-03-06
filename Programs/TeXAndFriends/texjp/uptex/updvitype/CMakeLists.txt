## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER ${MIKTEX_CURRENT_FOLDER}/upDVItype)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(updvitype_target_name updvitype)

include(webify.cmake)

set(updvitype_web_file ${CMAKE_CURRENT_BINARY_DIR}/updvitype-final.web)

create_web_app(upDVItype)

if(LINK_EVERYTHING_STATICALLY)
    target_link_libraries(${MIKTEX_PREFIX}updvitype
        texjp-kanji
    )
else()
    target_link_libraries(${updvitype_target_name}
        PRIVATE
            texjp-ukanji
    )
endif()
