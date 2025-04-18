## CMakeLists.txt
##
## Copyright (C) 2017-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(InstallQtPlugin)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_CONSOLE_FOLDER}/Qt")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(console_sources
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    DocumentationPage.cpp
    DocumentationPage.h
    DocumentationProxyModel.cpp
    DocumentationProxyModel.h
    DocumentationTableModel.cpp
    DocumentationTableModel.h
    FormatDefinitionDialog.cpp
    FormatDefinitionDialog.h
    FormatProxyModel.cpp
    FormatProxyModel.h
    FormatTableModel.cpp
    FormatTableModel.h
    LanguageTableModel.cpp
    LanguageTableModel.h
    PackageProxyModel.cpp
    PackageProxyModel.h
    PackageTableModel.cpp
    PackageTableModel.h
    RepositoryTableModel.cpp
    RepositoryTableModel.h
    RootTableModel.cpp
    RootTableModel.h
    UILanguageTableModel.cpp
    UILanguageTableModel.h
    UpdateTableModel.cpp
    UpdateTableModel.h
    common.h
    console-version.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

list(APPEND l_sources ${console_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        win/console.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/console.rc
    )
    list(APPEND console_sources
        ${CMAKE_CURRENT_BINARY_DIR}/console.rc
        ${MIKTEX_COMMON_MANIFEST}
    )
endif()

if(MIKTEX_MACOS_BUNDLE)
    # FIXME
    set(MACOSX_BUNDLE_ICON_FILE miktex-console.icns)
    list(APPEND console_sources
        ${MIKTEX_CONSOLE_ICNS}
    )
    set_source_files_properties(${MIKTEX_CONSOLE_ICNS}
        PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
endif()

set(console_moc_sources
    DocumentationPage.h
    DocumentationProxyModel.h
    DocumentationTableModel.h
    FormatDefinitionDialog.h
    FormatProxyModel.h
    FormatTableModel.h
    LanguageTableModel.h
    PackageProxyModel.h
    PackageTableModel.h
    RepositoryTableModel.h
    RootTableModel.h
    UILanguageTableModel.h
    UpdateTableModel.h
    mainwindow.h
)

set(console_ui_files
    formatdefinitiondialog.ui
    mainwindow.ui
)

list(APPEND l_sources ${console_ui_files})

set(ui_languages ${MIKTEX_INTERNAL_LOCALE})

foreach(l ${MIKTEX_TRANSLATION_DONE_LOCALES})
    list(APPEND ui_languages ${l})
    list(APPEND translations
        translations/console_${l}.ts
    )
    set(qresource_files
        "${qresource_files}<file alias='console_${l}.qm'>${CMAKE_CURRENT_BINARY_DIR}/console_${l}.qm</file>"
    )
endforeach()

list(SORT ui_languages)

get_target_property(LUPDATE_EXECUTABLE Qt${QT_SERIES}::lupdate LOCATION)

add_custom_target(gen-console-translations
    COMMAND
        ${LUPDATE_EXECUTABLE} ${l_sources} -ts ${translations}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET gen-console-translations PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

if(QT_SERIES STREQUAL "5")
    qt5_add_translation(qm_files ${translations})
else()
    qt_add_translation(qm_files ${translations})
endif()

configure_file(
    translations.qrc.in
    ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
)

set(console_qrc_files
    ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
    console.qrc
)

if(QT_SERIES STREQUAL "5")
    qt5_wrap_cpp(console_mocs ${console_moc_sources})
else()
    qt_wrap_cpp(console_mocs ${console_moc_sources})
endif()

if(QT_SERIES STREQUAL "5")
    qt5_wrap_ui(console_ui_headers ${console_ui_files})
else()
    qt_wrap_ui(console_ui_headers ${console_ui_files})
endif()

if(QT_SERIES STREQUAL "5")
    qt5_wrap_cpp(console_ui_mocs ${console_ui_headers})
else()
    qt_wrap_cpp(console_ui_mocs ${console_ui_headers})
endif()

if(QT_SERIES STREQUAL "5")
    qt5_add_resources(console_rcc_sources ${console_qrc_files})
else()
    qt_add_resources(console_rcc_sources ${console_qrc_files})
endif()

set(sources
    ${console_sources}
    ${console_mocs}
    ${console_ui_mocs}
    ${console_rcc_sources}
)

set(libs
    ${core_dll_name}
    ${loc_dll_name}
    ${mpm_dll_name}
    ${setup_dll_name}
    ${ui_qt_dll_name}
    Qt${QT_SERIES}::Widgets
    miktex-popt-wrapper
)

if (USE_SYSTEM_LOG4CXX)
    list(APPEND libs MiKTeX::Imported::LOG4CXX)
else()
    list(APPEND libs ${log4cxx_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND libs
        Comctl32.lib
    )
endif()

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

# stibizt: https://github.com/Kitware/CMake/blob/master/Source/QtDialog/CMakeLists.txt
if(MIKTEX_MACOS_BUNDLE)
    install_qt_plugin("Qt${QT_SERIES}::QCocoaIntegrationPlugin" QT_PLUGINS)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
        "[Paths]\nPlugins = PlugIns\n")
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
        DESTINATION "${MIKTEX_MACOS_BUNDLE_DIR}/Resources"
    )
    get_property(_Qt${QT_SERIES}_Core_LOCATION TARGET Qt${QT_SERIES}::Core PROPERTY LOCATION)
    get_filename_component(Qt_BIN_DIR "${_Qt${QT_SERIES}_Core_LOCATION}" PATH)
    get_filename_component(Qt_BIN_DIR "${Qt_BIN_DIR}" PATH)
endif()

add_executable(${MIKTEX_PROG_NAME_CONSOLE} WIN32 MACOSX_BUNDLE ${sources})
set_property(TARGET ${MIKTEX_PROG_NAME_CONSOLE} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(${MIKTEX_PROG_NAME_CONSOLE} ${libs})  
if(MIKTEX_MACOS_BUNDLE)
    set_target_properties(${MIKTEX_PROG_NAME_CONSOLE}
        PROPERTIES
        OUTPUT_NAME ${MIKTEX_MACOS_BUNDLE_NAME}
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME ${MIKTEX_MACOS_BUNDLE_NAME}
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${MIKTEX_MAJOR_VERSION}.${MIKTEX_MINOR_VERSION}.${MIKTEX_PATCH_VERSION}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.miktex.console"
    )
endif()
if(UNIX AND NOT APPLE)
    # Gnome
    install(
        FILES "${CMAKE_SOURCE_DIR}/Resources/Icons/MiKTeX/128x128/apps/miktex-console.png"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pixmaps"
    )
    # KDE
    foreach (size IN ITEMS 32 128)
        install(
        FILES "${CMAKE_SOURCE_DIR}/Resources/Icons/MiKTeX/${size}x${size}/apps/miktex-console.png"
        DESTINATION "${MIKTEX_XDG_APPLICATIONS_DIR}/icons/hicolor/${size}x${size}/apps"
        )
    endforeach()
    install(
        FILES unx/miktex-console.desktop
        DESTINATION "${MIKTEX_XDG_APPLICATIONS_DIR}"
    )
    install(
        FILES unx/miktex-console.policy
        DESTINATION "${MIKTEX_POLKIT_ACTIONS_DIR}"
    )
endif()

install(TARGETS miktex-console
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR}
)

# stibizt: https://github.com/Kitware/CMake/blob/master/Source/QtDialog/CMakeLists.txt
if(MIKTEX_MACOS_BUNDLE)
    set(fixup_exe "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MIKTEX_MACOS_BUNDLE_DIR}/MacOS/${MIKTEX_MACOS_BUNDLE_NAME}")
    # FIXME: non-standard icu lib dir
    list(APPEND lib_dirs "/usr/local/opt/poppler-qt5/lib")
    # FIXME: non-standard poppler-qt5 lib dir
    list(APPEND lib_dirs "/usr/local/opt/icu4c/lib")
    list(APPEND lib_dirs "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MIKTEX_LIBRARY_DESTINATION_DIR}")
    install(CODE "
        include(${CMAKE_SOURCE_DIR}/cmake/modules/BundleUtilitiesOverrides.cmake)
        include(BundleUtilities)
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        fixup_bundle(\"${fixup_exe}\" \"${QT_PLUGINS}\" \"${Qt_BIN_DIR};${QT_LIBRARY_DIR};${QT_BINARY_DIR};${lib_dirs}\")
    ")
endif()

if(MIKTEX_NATIVE_WINDOWS)
    add_executable(${MIKTEX_PROG_NAME_CONSOLE_ADMIN} WIN32 ${sources})
    set_property(TARGET ${MIKTEX_PROG_NAME_CONSOLE_ADMIN} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
    add_link_flags(${MIKTEX_PROG_NAME_CONSOLE_ADMIN} "/MANIFESTUAC:level='requireAdministrator'")
    target_link_libraries(${MIKTEX_PROG_NAME_CONSOLE_ADMIN} ${libs})
    install(TARGETS ${MIKTEX_PROG_NAME_CONSOLE_ADMIN}
        RUNTIME DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR}
    )
endif()

if(MIKTEX_MACOS_BUNDLE)
    install(
        PROGRAMS
        mac/miktex-console_admin
        mac/Terminal
        DESTINATION "${MIKTEX_MACOS_BUNDLE_DIR}/Resources"
    )
endif()

if(MIKTEX_MACOS_BUNDLE AND WITH_CODE_SIGNING)
    configure_file(
        ${CMAKE_SOURCE_DIR}/BuildUtilities/sign/mac/codesign_executable.in
        ${CMAKE_BINARY_DIR}/BuildUtilities/sign/mac/tmp/codesign_executable
        @ONLY
    )
    configure_file(
        ${CMAKE_SOURCE_DIR}/BuildUtilities/sign/mac/codesign_miktex.in
        ${CMAKE_BINARY_DIR}/BuildUtilities/sign/mac/tmp/codesign_miktex
        @ONLY
    )
    file(
        COPY
            ${CMAKE_BINARY_DIR}/BuildUtilities/sign/mac/tmp/codesign_executable
            ${CMAKE_BINARY_DIR}/BuildUtilities/sign/mac/tmp/codesign_miktex
        DESTINATION
            ${CMAKE_BINARY_DIR}/BuildUtilities/sign/mac
        FILE_PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
    install(CODE
        "execute_process(COMMAND \"${CMAKE_BINARY_DIR}/BuildUtilities/sign/mac/codesign_miktex\" \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MIKTEX_MACOS_BUNDLE_NAME}.app\")"
    )
endif()
