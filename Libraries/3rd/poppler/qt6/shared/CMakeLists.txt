## CMakeLists.txt
##
## Copyright (C) 2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(CMAKE_AUTOMOC ON)

include_directories(BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

configure_file(
    ${projdir}/source/qt6/src/poppler-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/poppler-version.h
    @ONLY
)

set(${poppler_qt_dll_name}_sources
    ${projdir}/source/qt6/src/poppler-annotation.cc
    ${projdir}/source/qt6/src/poppler-document.cc
    ${projdir}/source/qt6/src/poppler-embeddedfile.cc
    ${projdir}/source/qt6/src/poppler-fontinfo.cc
    ${projdir}/source/qt6/src/poppler-form.cc
    ${projdir}/source/qt6/src/poppler-link.cc
    ${projdir}/source/qt6/src/poppler-link-extractor.cc
    ${projdir}/source/qt6/src/poppler-movie.cc
    ${projdir}/source/qt6/src/poppler-optcontent.cc
    ${projdir}/source/qt6/src/poppler-page.cc
    ${projdir}/source/qt6/src/poppler-base-converter.cc
    ${projdir}/source/qt6/src/poppler-pdf-converter.cc
    ${projdir}/source/qt6/src/poppler-private.cc
    ${projdir}/source/qt6/src/poppler-ps-converter.cc
    ${projdir}/source/qt6/src/poppler-qiodeviceinstream.cc
    ${projdir}/source/qt6/src/poppler-qiodeviceoutstream.cc
    ${projdir}/source/qt6/src/poppler-sound.cc
    ${projdir}/source/qt6/src/poppler-textbox.cc
    ${projdir}/source/qt6/src/poppler-page-transition.cc
    ${projdir}/source/qt6/src/poppler-media.cc
    ${projdir}/source/qt6/src/poppler-outline.cc
    ${projdir}/source/qt6/src/QPainterOutputDev.cc
    ${projdir}/source/qt6/src/poppler-version.cpp

    ${projdir}/source/qt6/src/poppler-annotation-helper.h
    ${projdir}/source/qt6/src/poppler-annotation-private.h
    ${projdir}/source/qt6/src/poppler-annotation.h
    ${projdir}/source/qt6/src/poppler-converter-private.h
    ${projdir}/source/qt6/src/poppler-embeddedfile-private.h
    ${projdir}/source/qt6/src/poppler-form.h
    ${projdir}/source/qt6/src/poppler-link-extractor-private.h
    ${projdir}/source/qt6/src/poppler-link-private.h
    ${projdir}/source/qt6/src/poppler-link.h
    ${projdir}/source/qt6/src/poppler-media.h
    ${projdir}/source/qt6/src/poppler-optcontent-private.h
    ${projdir}/source/qt6/src/poppler-optcontent.h
    ${projdir}/source/qt6/src/poppler-outline-private.h
    ${projdir}/source/qt6/src/poppler-page-private.h
    ${projdir}/source/qt6/src/poppler-page-transition-private.h
    ${projdir}/source/qt6/src/poppler-page-transition.h
    ${projdir}/source/qt6/src/poppler-private.h
    ${projdir}/source/qt6/src/poppler-qiodeviceinstream-private.h
    ${projdir}/source/qt6/src/poppler-qiodeviceoutstream-private.h
    ${projdir}/source/qt6/src/poppler-qt6.h
    ${projdir}/source/qt6/src/QPainterOutputDev.h
)

add_library(${poppler_qt_dll_name} SHARED ${${poppler_qt_dll_name}_sources})

generate_export_header(${poppler_qt_dll_name}
    BASE_NAME poppler-qt6
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/poppler-export.h
)

set_property(TARGET ${poppler_qt_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${poppler_qt_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${poppler_qt_dll_name}
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${projdir}/source/qt6/src
)

target_link_libraries(${poppler_qt_dll_name}
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Xml
        ${core_dll_name}
    PUBLIC
        ${poppler_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${poppler_qt_dll_name}
        PRIVATE
            ${unxemu_dll_name}
            ${utf8wrap_dll_name}
    )
endif()

if(USE_SYSTEM_FREETYPE2)
    target_link_libraries(${poppler_qt_dll_name} PRIVATE MiKTeX::Imported::FREETYPE2)
else()
    target_link_libraries(${poppler_qt_dll_name} PRIVATE ${freetype2_dll_name})
endif()

install(TARGETS ${poppler_qt_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
