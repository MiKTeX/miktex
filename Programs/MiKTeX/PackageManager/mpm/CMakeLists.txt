## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PACKAGE_MANAGER_FOLDER}")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(mpm_sources
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    internal.h
    mpm-version.h
    mpm.cpp
)

add_definitions(
    -DHAVE_CONFIG_H=1
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND mpm_sources
        ${MIKTEX_COMMON_MANIFEST}
        mpm.rc
    )
endif()

add_executable(${MIKTEX_PROG_NAME_MPM} ${mpm_sources})

set_property(TARGET ${MIKTEX_PROG_NAME_MPM} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_FMT)
    target_link_libraries(${MIKTEX_PROG_NAME_MPM} MiKTeX::Imported::FMT)
else()
    target_link_libraries(${MIKTEX_PROG_NAME_MPM} ${fmt_dll_name})
endif()

if(USE_SYSTEM_LOG4CXX)
    target_link_libraries(${MIKTEX_PROG_NAME_MPM} MiKTeX::Imported::LOG4CXX)
else()
    target_link_libraries(${MIKTEX_PROG_NAME_MPM} ${log4cxx_dll_name})
endif()

target_link_libraries(${MIKTEX_PROG_NAME_MPM}
    ${core_dll_name}
    ${mpm_dll_name}
    miktex-popt-wrapper
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PROG_NAME_MPM}
        Wldap32
        shlwapi
        wininet
        winmm
        ws2_32
    )
endif()

install(TARGETS ${MIKTEX_PROG_NAME_MPM} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
