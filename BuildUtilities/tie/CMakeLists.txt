## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}/Tie")

include_directories(BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

set(tie_sources ${CMAKE_CURRENT_BINARY_DIR}/tie.c)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/tie.c
    COMMAND
        ${MIKTEX_PREFIX}ctangle
        ${CMAKE_CURRENT_SOURCE_DIR}/source/tie.w
        ${CMAKE_CURRENT_SOURCE_DIR}/tie-miktex.ch 
        ${CMAKE_CURRENT_BINARY_DIR}/tie.c
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
    MAIN_DEPENDENCY
        source/tie.w
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/tie-miktex.ch
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_executable(${MIKTEX_PREFIX}tie ${tie_sources})

set_property(TARGET ${MIKTEX_PREFIX}tie PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(TARGETS ${MIKTEX_PREFIX}tie DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}")
