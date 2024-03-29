## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/texmf")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

configure_file(
    include/miktex/C4P/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}//include/miktex/C4P/config.h
)

configure_file(
    include/miktex/TeXAndFriends/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/TeXAndFriends/config.h
)

set(generated_texmf_sources
    ${CMAKE_BINARY_DIR}/include/miktex/mfapp.defaults.h
    ${CMAKE_BINARY_DIR}/include/miktex/texapp.defaults.h
    ${CMAKE_BINARY_DIR}/include/miktex/texmfapp.defaults.h
)

set(headers_no_ext
    miktex/C4P/C4P
    miktex/TeXAndFriends/CharacterConverterImpl
    miktex/TeXAndFriends/ETeXApp
    miktex/TeXAndFriends/ETeXMemoryHandlerImpl
    miktex/TeXAndFriends/ErrorHandlerImpl
    miktex/TeXAndFriends/FormatHandlerImpl
    miktex/TeXAndFriends/InitFinalizeImpl
    miktex/TeXAndFriends/InputOutputImpl
    miktex/TeXAndFriends/MetafontApp
    miktex/TeXAndFriends/MetafontMemoryHandlerImpl
    miktex/TeXAndFriends/Prototypes
    miktex/TeXAndFriends/StringHandlerImpl
    miktex/TeXAndFriends/TeXApp
    miktex/TeXAndFriends/TeXMFApp
    miktex/TeXAndFriends/TeXMFMemoryHandlerImpl
    miktex/TeXAndFriends/TeXMemoryHandlerImpl
    miktex/TeXAndFriends/WebApp
    miktex/TeXAndFriends/WebAppInputLine
)

foreach(h ${headers_no_ext})
    set(_path ${CMAKE_CURRENT_BINARY_DIR}/include/${h})
    list(APPEND public_headers_no_ext ${_path})
    if (NOT EXISTS ${_path})
        file(WRITE ${_path} "#pragma once\n#include <${h}.h>\n")
    endif()
endforeach()

set(public_headers_c4p
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/C4P/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/C4P/C4P.h
)

set(public_headers_texmf
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/TeXAndFriends/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/CharacterConverterImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/ETeXApp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/ETeXMemoryHandlerImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/ErrorHandlerImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/FormatHandlerImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/InitFinalizeImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/InputOutputImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/MetafontApp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/MetafontMemoryHandlerImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/Prototypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/StringHandlerImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/TeXApp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/TeXMFApp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/TeXMFMemoryHandlerImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/TeXMemoryHandlerImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/WebApp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/WebAppInputLine.h
)

set(public_headers
    ${public_headers_c4p}
    ${public_headers_texmf}

    ${public_headers_no_ext}
)

configure_file(
    texmf-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/texmf-version.h
)

set(texmf_sources
    ${CMAKE_CURRENT_BINARY_DIR}/texmf-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/c4plib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/c4pstart.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/etexapp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inputline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mfapp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/texapp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/texmfapp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/texmflib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webapp.cpp
    ${generated_texmf_sources}
    ${public_headers}
)

list(APPEND l_sources etexapp.cpp mfapp.cpp texapp.cpp texmfapp.cpp texmflib.cpp webapp.cpp)

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
    TeXMFResources.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/TeXMFResources.h
)

add_custom_target(gen-${MIKTEX_COMP_ID}-sources DEPENDS
    ${generated_${MIKTEX_COMP_ID}_sources}
)

set_property(TARGET gen-${MIKTEX_COMP_ID}-sources PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

list(APPEND texmf_sources
    ${CMAKE_CURRENT_BINARY_DIR}/TeXMFResources.h
    ${generated_${MIKTEX_COMP_ID}_sources}
)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

add_subdirectory(shared)

if(INSTALL_MIKTEX_HEADERS)
    install(
        FILES
            include/miktex/TeXAndFriends/ETeXApp
            include/miktex/TeXAndFriends/ETeXApp.h
            include/miktex/TeXAndFriends/MetafontApp
            include/miktex/TeXAndFriends/MetafontApp.h
            include/miktex/TeXAndFriends/Prototypes
            include/miktex/TeXAndFriends/Prototypes.h
            include/miktex/TeXAndFriends/TeXApp
            include/miktex/TeXAndFriends/TeXApp.h
            include/miktex/TeXAndFriends/TeXMFApp
            include/miktex/TeXAndFriends/TeXMFApp.h
            include/miktex/TeXAndFriends/WebApp
            include/miktex/TeXAndFriends/WebApp.h
            include/miktex/TeXAndFriends/WebAppInputLine
            include/miktex/TeXAndFriends/WebAppInputLine.h
        DESTINATION
            ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/TeXAndFriends
    )
endif()

add_subdirectory(test)
