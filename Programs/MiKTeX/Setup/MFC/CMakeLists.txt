## CMakeLists.txt
##
## Copyright (C) 2006-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_SETUP_FOLDER}/MFC")

use_static_mfc()

set(CMAKE_MFC_FLAG 1)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
    -DUNICODE
    -D_UNICODE
)

configure_file(
    res/Setup.manifest.in
    ${CMAKE_CURRENT_BINARY_DIR}/res/Setup.manifest
)

configure_file(
    res/Setup.rc2.in
    ${CMAKE_CURRENT_BINARY_DIR}/res/Setup.rc2
)

configure_file(
    Setup.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Setup.rc
)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(setup_mfc_sources
    ${CMAKE_CURRENT_BINARY_DIR}/Setup.rc
    ${CMAKE_CURRENT_BINARY_DIR}/res/Setup.manifest
    ${MIKTEX_COMMON_MANIFEST}
    ${MIKTEX_DPIAWARE_MANIFEST}
    ConnectionSettingsDialog.cpp
    ConnectionSettingsDialog.h
    FileCopyPage.cpp
    FileCopyPage.h
    FinishPage.cpp
    FinishPage.h
    InfoListPage.cpp
    InfoListPage.h
    InstallDirPage.cpp
    InstallDirPage.h
    LicensePage.cpp
    LicensePage.h
    LocalRepositoryPage.cpp
    LocalRepositoryPage.h
    MDTaskPage.cpp
    MDTaskPage.h
    PackageSetPage.cpp
    PackageSetPage.h
    PostInstallPage.cpp
    PostInstallPage.h
    ProxyAuthenticationDialog.cpp
    ProxyAuthenticationDialog.h
    RemoteRepositoryPage.cpp
    RemoteRepositoryPage.h
    SettingsPage.cpp
    SettingsPage.h
    Setup-version.h
    Setup.cpp
    Setup.h
    SetupWizard.cpp
    SetupWizard.h
    SharedInstallationPage.cpp
    SharedInstallationPage.h
    StdAfx.h
    TaskPage.cpp
    TaskPage.h
    WelcomePage.cpp
    WelcomePage.h
    resource.h
    targetver.h
)

set(libs
    ${core_lib_name}
    ${fmt_lib_name}
    ${getopt_lib_name}
    ${mpm_lib_name}
    ${setup_lib_name}
    Secur32
    Wldap32
    version
    wininet
    winmm
    ws2_32
)  

add_executable(setup WIN32 ${setup_mfc_sources})
set_property(TARGET setup PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
add_dependencies(setup gen-COPYING-rtf)
add_link_flags(setup "/ENTRY:\"wWinMainCRTStartup\"")
target_link_libraries(setup ${libs})
install(TARGETS setup DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

add_executable(setup${MIKTEX_ADMIN_SUFFIX} WIN32 ${setup_mfc_sources})
set_property(TARGET setup${MIKTEX_ADMIN_SUFFIX} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
add_dependencies(setup${MIKTEX_ADMIN_SUFFIX} gen-COPYING-rtf)
add_link_flags(setup${MIKTEX_ADMIN_SUFFIX} "/MANIFESTUAC:level='requireAdministrator'")
add_link_flags(setup${MIKTEX_ADMIN_SUFFIX} "/ENTRY:\"wWinMainCRTStartup\"")
target_link_libraries(setup${MIKTEX_ADMIN_SUFFIX} ${libs})
install(TARGETS setup${MIKTEX_ADMIN_SUFFIX} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

add_executable(setup_highest WIN32 ${setup_mfc_sources})
set_property(TARGET setup_highest PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
add_dependencies(setup_highest gen-COPYING-rtf)
add_link_flags(setup_highest "/MANIFESTUAC:level='highestAvailable'")
add_link_flags(setup_highest "/ENTRY:\"wWinMainCRTStartup\"")
target_link_libraries(setup_highest ${libs})
install(TARGETS setup_highest DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
