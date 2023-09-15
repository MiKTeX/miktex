## CMakeLists.txt
##
## Copyright (C) 2006-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/${MIKTEX_PROG_NAME_EPSTOPDF}")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

configure_file(
    miktex-epstopdf-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-epstopdf-version.h
)

set(epstopdf_sources
    miktex-epstopdf-version.h
    miktex-epstopdf.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-epstopdf.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/windows/miktex-epstopdf.rc
    )
    list(APPEND epstopdf_sources
        ${MIKTEX_COMMON_MANIFEST}
        windows/miktex-epstopdf.rc
    )
endif()

add_executable(${MIKTEX_PROG_NAME_EPSTOPDF} ${epstopdf_sources})

set_property(TARGET ${MIKTEX_PROG_NAME_EPSTOPDF} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PROG_NAME_EPSTOPDF}
    ${app_dll_name}
    ${texmf_dll_name}
    miktex-popt-wrapper
)

if(USE_SYSTEM_FMT)
    target_link_libraries(${MIKTEX_PROG_NAME_EPSTOPDF} MiKTeX::Imported::FMT)
else()
    target_link_libraries(${MIKTEX_PROG_NAME_EPSTOPDF} ${fmt_dll_name})
endif()

install(TARGETS ${MIKTEX_PROG_NAME_EPSTOPDF} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
