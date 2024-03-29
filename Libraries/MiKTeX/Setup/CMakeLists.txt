## CMakeLists.txt
##
## Copyright (C) 2013-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/setup")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(public_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Setup/SetupService
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Setup/SetupService.h
)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
    setup-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/setup-version.h
)

set(setup_sources
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${CMAKE_CURRENT_BINARY_DIR}/setup-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LogFile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SetupService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Setup/SetupService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
    ${public_headers}
)

list(APPEND l_sources LogFile.cpp SetupService.cpp)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND setup_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/win/winSetupService.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/winSetupService.h
    )
    list(APPEND l_sources win/winSetupService.cpp)
else()
    list(APPEND setup_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/unx/unxSetupService.cpp
    )
endif()

add_custom_target(${MIKTEX_COMP_ID}-pot
    COMMAND
        ${XGETTEXT_EXECUTABLE} --join-existing --keyword=T_ --from-code=UTF-8 --add-comments --output=po/${MIKTEX_COMP_ID}.pot ${l_sources}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)
set_property(TARGET ${MIKTEX_COMP_ID}-pot PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

add_custom_target(gen-${MIKTEX_COMP_ID}-translations)
set_property(TARGET gen-${MIKTEX_COMP_ID}-translations PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
add_dependencies(gen-${MIKTEX_COMP_ID}-translations ${MIKTEX_COMP_ID}-pot)

add_custom_target(${MIKTEX_COMP_ID}-mo ALL)
set_property(TARGET ${MIKTEX_COMP_ID}-mo PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

foreach(l ${MIKTEX_TRANSLATION_DONE_LOCALES})
    add_custom_target(${MIKTEX_COMP_ID}-po-${l}
        COMMAND
            ${MSGMERGE_EXECUTABLE} --update --backup=none po/${l}/${MIKTEX_COMP_ID}.po po/${MIKTEX_COMP_ID}.pot
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_property(TARGET ${MIKTEX_COMP_ID}-po-${l} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
    add_dependencies(gen-${MIKTEX_COMP_ID}-translations ${MIKTEX_COMP_ID}-po-${l})
    add_custom_target(${MIKTEX_COMP_ID}-mo-${l} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo)
    set_property(TARGET ${MIKTEX_COMP_ID}-mo-${l} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
    add_dependencies(${MIKTEX_COMP_ID}-mo ${MIKTEX_COMP_ID}-mo-${l})
    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}
        COMMAND
            ${MSGFMT_EXECUTABLE} --output-file=locale/${l}/${MIKTEX_COMP_ID}.mo ${CMAKE_CURRENT_SOURCE_DIR}/po/${l}/${MIKTEX_COMP_ID}.po
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/po/${l}/${MIKTEX_COMP_ID}.po
    )
    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}
        COMMAND
            bintoc messages_${l}
                < ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo
                > ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h.tmp
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h.tmp
            ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h
        COMMAND
            ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h.tmp
        DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo
            bintoc
        VERBATIM
    )
    list(APPEND generated_${MIKTEX_COMP_ID}_sources ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h)
    set(resource_includes
        "${resource_includes}#include \"${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h\"\n"
    )
    set(resource_adders
        "${resource_adders}R_(\":/${l}/LC_MESSAGES/${MIKTEX_COMP_ID}.mo\", messages_${l});\n"
    )
endforeach()

configure_file(
    SetupResources.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/SetupResources.h
)

add_custom_target(gen-${MIKTEX_COMP_ID}-sources DEPENDS
    ${generated_${MIKTEX_COMP_ID}_sources}
)

set_property(TARGET gen-${MIKTEX_COMP_ID}-sources PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

list(APPEND setup_sources
    ${CMAKE_CURRENT_BINARY_DIR}/SetupResources.h
    ${generated_${MIKTEX_COMP_ID}_sources}
)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
