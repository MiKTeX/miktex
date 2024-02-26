## CMakeLists.txt
##
## Copyright (C) 2006-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/${MIKTEX_PROG_NAME_TEXIFY}")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(WITH_TEXINFO TRUE)

if(WITH_TEXINFO)
    add_definitions(
        -DWITH_TEXINFO
    )
endif()

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

configure_file(
    mcd-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/mcd-version.h
)

set(mcd_sources
    ${CMAKE_CURRENT_BINARY_DIR}/mcd-version.h
    mcd.cpp
)

if(WITH_TEXINFO)
    list(APPEND mcd_sources texinfo.cpp)
endif()

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/mcd.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/mcd.rc
    )
    list(APPEND mcd_sources
        ${CMAKE_CURRENT_BINARY_DIR}/mcd.rc
        ${MIKTEX_COMMON_MANIFEST}
    )
endif()

add_executable(${MIKTEX_PROG_NAME_TEXIFY} ${mcd_sources})

set_property(TARGET ${MIKTEX_PROG_NAME_TEXIFY} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PROG_NAME_TEXIFY}
    ${app_dll_name}
    ${core_dll_name}
    ${texmf_dll_name}
    miktex-popt-wrapper
)

if(USE_SYSTEM_FMT)
    target_link_libraries(${MIKTEX_PROG_NAME_TEXIFY} MiKTeX::Imported::FMT)
else()
    target_link_libraries(${MIKTEX_PROG_NAME_TEXIFY} ${fmt_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PROG_NAME_TEXIFY}
        ${regex_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PROG_NAME_TEXIFY}
      RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
      LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
      ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
