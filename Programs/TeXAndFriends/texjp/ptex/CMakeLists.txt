## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_CURRENT_FOLDER}/pTeX")

set(projdir ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(C4P_FLAGS
    --declare-c-type=cinttype
)

if(MSVC)
    add_definitions(
        -D_UNICODE -DUNICODE
    )
endif()

include(libkanji.cmake)

add_subdirectory(pbibtex)
add_subdirectory(pdvitype)
add_subdirectory(ppltotf)
add_subdirectory(ptex)
add_subdirectory(ptftopl)