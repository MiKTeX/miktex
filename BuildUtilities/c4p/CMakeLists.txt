## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(c4p_sources
    ${CMAKE_CURRENT_BINARY_DIR}/gram.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gram.h
    ${CMAKE_CURRENT_BINARY_DIR}/scan.cpp
    c4p-version.h
    common.h
    main.cpp
    misc.cpp
    output.cpp
    output.h
    subrange.cpp
    symtab.cpp
    type.cpp
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/gram.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/gram.h
    COMMAND
        ${BISON_EXECUTABLE}
            --output=${CMAKE_CURRENT_BINARY_DIR}/gram.cpp
            --defines=${CMAKE_CURRENT_BINARY_DIR}/gram.h
            ${CMAKE_CURRENT_SOURCE_DIR}/gram.y
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/gram.y
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/scan.cpp
    COMMAND
        ${FLEX_EXECUTABLE}
            -o${CMAKE_CURRENT_BINARY_DIR}/scan.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/scan.l
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/scan.l
    VERBATIM
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND c4p_sources
        ${MIKTEX_COMMON_MANIFEST}
        c4p.rc
    )
endif()

add_executable(c4p ${c4p_sources})

set_property(TARGET c4p PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(NOT HAVE_GETOPT_LONG)
    target_link_libraries(c4p ${getopt_dll_name})
endif()

if (USE_SYSTEM_FMT)
    target_link_libraries(c4p MiKTeX::Imported::FMT)
else()
    target_link_libraries(c4p ${fmt_dll_name})
endif()
