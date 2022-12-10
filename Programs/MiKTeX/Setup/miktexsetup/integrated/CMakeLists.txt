## CMakeLists.txt
##
## Copyright (C) 2014-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_SETUP_FOLDER}/${MIKTEX_PROG_NAME_SETUP}")

add_executable(${MIKTEX_PROG_NAME_SETUP} ${miktexsetup_sources})

set_property(TARGET ${MIKTEX_PROG_NAME_SETUP} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if (USE_SYSTEM_FMT)
    target_link_libraries(${MIKTEX_PROG_NAME_SETUP} MiKTeX::Imported::FMT)
else()
    target_link_libraries(${MIKTEX_PROG_NAME_SETUP} ${fmt_dll_name})
endif()

target_link_libraries(${MIKTEX_PROG_NAME_SETUP}
    ${core_dll_name}
    ${setup_dll_name}
    miktex-popt-wrapper
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PROG_NAME_SETUP}
        Wldap32
        shlwapi
        version
        wininet
        winmm
        ws2_32
    )
endif()

install(TARGETS ${MIKTEX_PROG_NAME_SETUP} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
