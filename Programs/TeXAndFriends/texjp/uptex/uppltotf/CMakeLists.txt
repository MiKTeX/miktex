## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER ${MIKTEX_CURRENT_FOLDER}/upPLtoTF)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(uppltotf_target_name uppltotf)

include(webify.cmake)

set(uppltotf_web_file ${CMAKE_CURRENT_BINARY_DIR}/uppltotf-final.web)

create_web_app(upPLtoTF)

if(LINK_EVERYTHING_STATICALLY)
    target_link_libraries(${MIKTEX_PREFIX}uppltotf
        texjp-ukanji
    )
else()
    target_link_libraries(${uppltotf_target_name}
        PRIVATE
            texjp-ukanji
    )
endif()
