## CMakeLists.txt
##
## Copyright (C) 2015-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_CURRENT_FOLDER}/utils")

set(common_sources
    ${projdir}/source/utils/parseargs.cc
)

set_source_files_properties(${MIKTEX_LIBRARY_WRAPPER}
    PROPERTIES
        COMPILE_FLAGS "-DCPLUSPLUSMAIN -DBEQUIET"
)

if(ENABLE_SPLASH)
    set(pdftoppm_sources
        ${MIKTEX_LIBRARY_WRAPPER}
        ${common_sources}
        ${projdir}/source/utils/pdftoppm.cc
    )
    add_executable(${MIKTEX_PREFIX}pdftoppm ${pdftoppm_sources})
    set_property(TARGET ${MIKTEX_PREFIX}pdftoppm PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
    target_link_libraries(${MIKTEX_PREFIX}pdftoppm
        ${app_dll_name}
        ${poppler_dll_name}
    )
    if(MIKTEX_NATIVE_WINDOWS)
        target_link_libraries(${MIKTEX_PREFIX}pdftoppm
            ${utf8wrap_dll_name}
        )
    endif()
    install(TARGETS ${MIKTEX_PREFIX}pdftoppm DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
endif()

if(HAVE_CAIRO)
    set(pdftocairo_sources
        ${MIKTEX_LIBRARY_WRAPPER}
        ${common_sources}
        ${projdir}/source/poppler/CairoFontEngine.cc
        ${projdir}/source/poppler/CairoOutputDev.cc
        ${projdir}/source/poppler/CairoRescaleBox.cc
        ${projdir}/source/utils/pdftocairo-win32.cc
        ${projdir}/source/utils/pdftocairo.cc
    )
    add_executable(${MIKTEX_PREFIX}pdftocairo ${pdftocairo_sources})
    set_property(TARGET ${MIKTEX_PREFIX}pdftocairo PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
    target_link_libraries(${MIKTEX_PREFIX}pdftocairo
        ${app_dll_name}
        ${poppler_dll_name}
    )
    if(USE_SYSTEM_CAIRO)
        target_link_libraries(${MIKTEX_PREFIX}pdftocairo MiKTeX::Imported::CAIRO)
    else()
        target_link_libraries(${MIKTEX_PREFIX}pdftocairo ${cairo_dll_name})
    endif()
    if(MIKTEX_NATIVE_WINDOWS)
        target_link_libraries(${MIKTEX_PREFIX}pdftocairo
            ${unxemu_dll_name}
            ${utf8wrap_dll_name}
        )
    endif()
    install(TARGETS ${MIKTEX_PREFIX}pdftocairo DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
endif()

set(pdfdetach_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${common_sources}
    ${projdir}/source/utils/pdfdetach.cc
)

add_executable(${MIKTEX_PREFIX}pdfdetach ${pdfdetach_sources})

set_property(TARGET ${MIKTEX_PREFIX}pdfdetach PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}pdfdetach
    ${app_dll_name}
    ${poppler_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}pdfdetach
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}pdfdetach DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdffonts_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${common_sources}
    ${projdir}/source/utils/pdffonts.cc
)

add_executable(${MIKTEX_PREFIX}pdffonts ${pdffonts_sources})

set_property(TARGET ${MIKTEX_PREFIX}pdffonts PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}pdffonts
    ${app_dll_name}
    ${poppler_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}pdffonts
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}pdffonts DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdfimages_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${common_sources}
    ${projdir}/source/utils/ImageOutputDev.cc
    ${projdir}/source/utils/ImageOutputDev.h
    ${projdir}/source/utils/pdfimages.cc
)

add_executable(${MIKTEX_PREFIX}pdfimages ${pdfimages_sources})

set_property(TARGET ${MIKTEX_PREFIX}pdfimages PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}pdfimages
    ${app_dll_name}
    ${poppler_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}pdfimages
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}pdfimages DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdfinfo_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${common_sources}
    ${projdir}/source/utils/pdfinfo.cc
    ${projdir}/source/utils/printencodings.cc
)

add_executable(${MIKTEX_PREFIX}pdfinfo ${pdfinfo_sources})

set_property(TARGET ${MIKTEX_PREFIX}pdfinfo PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}pdfinfo
    ${app_dll_name}
    ${poppler_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}pdfinfo
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}pdfinfo DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdftops_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${common_sources}
    ${projdir}/source/utils/pdftops.cc
)

add_executable(${MIKTEX_PREFIX}pdftops ${pdftops_sources})

set_property(TARGET ${MIKTEX_PREFIX}pdftops PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}pdftops
    ${app_dll_name}
    ${poppler_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}pdftops
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}pdftops DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdftotext_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${common_sources}
    ${projdir}/source/utils/pdftotext.cc
    ${projdir}/source/utils/printencodings.cc
)

add_executable(${MIKTEX_PREFIX}pdftotext ${pdftotext_sources})

set_property(TARGET ${MIKTEX_PREFIX}pdftotext PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}pdftotext
    ${app_dll_name}
    ${poppler_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}pdftotext
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}pdftotext DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdftohtml_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${common_sources}
    ${projdir}/source/utils/HtmlFonts.cc
    ${projdir}/source/utils/HtmlLinks.cc
    ${projdir}/source/utils/InMemoryFile.cc
    ${projdir}/source/utils/HtmlOutputDev.cc
    ${projdir}/source/utils/pdftohtml.cc
)

add_executable(${MIKTEX_PREFIX}pdftohtml ${pdftohtml_sources})

set_property(TARGET ${MIKTEX_PREFIX}pdftohtml PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}pdftohtml
  ${app_dll_name}
  ${poppler_dll_name}
)

if(USE_SYSTEM_PNG)
    target_link_libraries(${MIKTEX_PREFIX}pdftohtml MiKTeX::Imported::PNG)
else()
    target_link_libraries(${MIKTEX_PREFIX}pdftohtml ${png_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}pdftohtml
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}pdftohtml DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdfseparate_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${common_sources}
    ${projdir}/source/utils/pdfseparate.cc
)

add_executable(${MIKTEX_PREFIX}pdfseparate ${pdfseparate_sources})

set_property(TARGET ${MIKTEX_PREFIX}pdfseparate PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}pdfseparate
    ${app_dll_name}
    ${poppler_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}pdfseparate
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}pdfseparate DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdfunite_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${common_sources}
    ${projdir}/source/utils/pdfunite.cc
)

add_executable(${MIKTEX_PREFIX}pdfunite ${pdfunite_sources})

set_property(TARGET ${MIKTEX_PREFIX}pdfunite PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}pdfunite
    ${app_dll_name}
    ${poppler_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}pdfunite
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}pdfunite DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

if(ENABLE_NSS3)
    set(pdfsig_sources
        ${MIKTEX_LIBRARY_WRAPPER}
        ${common_sources}
        ${projdir}/source/utils/pdfsig.cc
    )
    add_executable(${MIKTEX_PREFIX}pdfsig ${pdfsig_sources})
    set_property(TARGET ${MIKTEX_PREFIX}pdfsig PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
    target_link_libraries(${MIKTEX_PREFIX}pdfsig
        ${app_dll_name}
        ${poppler_dll_name}
    )
    install(TARGETS ${MIKTEX_PREFIX}pdfsig DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
endif()

set(pdfattach_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${common_sources}
    ${projdir}/source/utils/pdfattach.cc
)

add_executable(${MIKTEX_PREFIX}pdfattach ${pdfattach_sources})

set_property(TARGET ${MIKTEX_PREFIX}pdfattach PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}pdfattach
  ${app_dll_name}
  ${poppler_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}pdfattach
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}pdfattach DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
