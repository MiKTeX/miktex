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

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(mpc_sources
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    mpc-version.h
    mpc.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND mpc_sources
        ${MIKTEX_COMMON_MANIFEST}
        mpc.rc
    )
endif()

add_executable(mpc ${mpc_sources})

set_property(TARGET mpc PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(mpc
    ${core_dll_name}
    ${mpm_dll_name}
    miktex-popt-wrapper
)

if (USE_SYSTEM_FMT)
    target_link_libraries(mpc MiKTeX::Imported::FMT)
else()
    target_link_libraries(mpc ${fmt_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(mpc
        Wldap32
        shlwapi
        version
        wininet
        winmm
        ws2_32
    )
endif()

if(MIKTEX_NATIVE_WINDOWS)
    install(TARGETS mpc DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
endif()
