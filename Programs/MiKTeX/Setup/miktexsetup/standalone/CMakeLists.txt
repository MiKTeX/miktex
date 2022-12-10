## CMakeLists.txt
##
## Copyright (C) 2014-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_SETUP_FOLDER}/${MIKTEX_PROG_NAME_SETUP_STANDALONE}")

use_static_crt()

add_definitions(
    -DMIKTEX_SETUP_STANDALONE
)

add_executable(${MIKTEX_PROG_NAME_SETUP_STANDALONE} ${miktexsetup_sources})

set_property(TARGET ${MIKTEX_PROG_NAME_SETUP_STANDALONE} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if (USE_SYSTEM_FMT)
    target_link_libraries(${MIKTEX_PROG_NAME_SETUP_STANDALONE} MiKTeX::Imported::FMT)
else()
    target_link_libraries(${MIKTEX_PROG_NAME_SETUP_STANDALONE} ${fmt_lib_name})
endif()

target_link_libraries(${MIKTEX_PROG_NAME_SETUP_STANDALONE}
    ${core_lib_name}
    ${setup_lib_name}
    miktex-popt-wrapper-static
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PROG_NAME_SETUP_STANDALONE}
        Wldap32
        shlwapi
        version
        wininet
        winmm
        ws2_32
    )
endif()

install(TARGETS ${MIKTEX_PROG_NAME_SETUP_STANDALONE} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
