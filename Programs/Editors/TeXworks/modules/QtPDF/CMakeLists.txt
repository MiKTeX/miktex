## CMakeLists.txt
##
## Copyright (C) 2008-2025 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_EDITORS_FOLDER}/TeXworks/QtPDF")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(qtpdf_sources
    ${projdir}/source/modules/QtPDF/src/GuidelineEditDialog.cpp
    ${projdir}/source/modules/QtPDF/src/InfoWidgets.cpp
    ${projdir}/source/modules/QtPDF/src/PDFActions.cpp
    ${projdir}/source/modules/QtPDF/src/PDFAnnotations.cpp
    ${projdir}/source/modules/QtPDF/src/PDFBackend.cpp
    ${projdir}/source/modules/QtPDF/src/PDFDocumentScene.cpp
    ${projdir}/source/modules/QtPDF/src/PDFDocumentTools.cpp
    ${projdir}/source/modules/QtPDF/src/PDFDocumentView.cpp
    ${projdir}/source/modules/QtPDF/src/PDFDocumentWidget.cpp
    ${projdir}/source/modules/QtPDF/src/PDFFontDescriptor.cpp
    ${projdir}/source/modules/QtPDF/src/PDFGuideline.cpp
    ${projdir}/source/modules/QtPDF/src/PDFPageCache.cpp
    ${projdir}/source/modules/QtPDF/src/PDFPageLayout.cpp
    ${projdir}/source/modules/QtPDF/src/PDFPageProcessingThread.cpp
    ${projdir}/source/modules/QtPDF/src/PDFPageTile.cpp
    ${projdir}/source/modules/QtPDF/src/PDFRuler.cpp
    ${projdir}/source/modules/QtPDF/src/PDFSearcher.cpp
    ${projdir}/source/modules/QtPDF/src/PDFToC.cpp
    ${projdir}/source/modules/QtPDF/src/PDFTransitions.cpp
    ${projdir}/source/modules/QtPDF/src/PaperSizes.cpp

    ${projdir}/source/modules/QtPDF/src/backends/PopplerQtBackend.cpp
)

set(qtpdf_headers
    ${projdir}/source/modules/QtPDF/src/GuidelineEditDialog.h
    ${projdir}/source/modules/QtPDF/src/InfoWidgets.h
    ${projdir}/source/modules/QtPDF/src/PDFActions.h
    ${projdir}/source/modules/QtPDF/src/PDFAnnotations.h
    ${projdir}/source/modules/QtPDF/src/PDFBackend.h
    ${projdir}/source/modules/QtPDF/src/PDFDocumentScene.h
    ${projdir}/source/modules/QtPDF/src/PDFDocumentTools.h
    ${projdir}/source/modules/QtPDF/src/PDFDocumentView.h
    ${projdir}/source/modules/QtPDF/src/PDFDocumentWidget.h
    ${projdir}/source/modules/QtPDF/src/PDFFontDescriptor.h
    ${projdir}/source/modules/QtPDF/src/PDFFontInfo.h
    ${projdir}/source/modules/QtPDF/src/PDFGuideline.h
    ${projdir}/source/modules/QtPDF/src/PDFPageCache.h
    ${projdir}/source/modules/QtPDF/src/PDFPageLayout.h
    ${projdir}/source/modules/QtPDF/src/PDFPageProcessingThread.h
    ${projdir}/source/modules/QtPDF/src/PDFPageTile.h
    ${projdir}/source/modules/QtPDF/src/PDFRuler.h
    ${projdir}/source/modules/QtPDF/src/PDFSearcher.h
    ${projdir}/source/modules/QtPDF/src/PDFToC.h
    ${projdir}/source/modules/QtPDF/src/PDFTransitions.h
    ${projdir}/source/modules/QtPDF/src/PaperSizes.h

    ${projdir}/source/modules/QtPDF/src/backends/PopplerQtBackend.h
)

set(qtpdf_ui_files
    ${projdir}/source/modules/QtPDF/src/GuidelineEditDialog.ui
)

if(QT_SERIES STREQUAL "5")
    qt5_wrap_ui(qtpdf_ui_headers ${qtpdf_ui_files})
else()
    qt_wrap_ui(qtpdf_ui_headers ${qtpdf_ui_files})
endif()

set(qtpdf_ts_files
    ${projdir}/source/modules/QtPDF/trans/QtPDF_af.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_ar.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_ca.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_cs.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_de.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_es.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_fa.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_fo.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_fr.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_it.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_ja.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_ko.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_nl.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_nn.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_pl.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_pt_BR.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_ru.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_sl.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_tr.ts
    ${projdir}/source/modules/QtPDF/trans/QtPDF_zh_CN.ts
)

if(QT_SERIES STREQUAL "5")
    qt5_add_translation(qtpdf_qm_sources ${qtpdf_ts_files})
else()
    qt_add_translation(qtpdf_qm_sources ${qtpdf_ts_files})
endif()
  
configure_file(QtPDF_trans.qrc.in ${CMAKE_CURRENT_BINARY_DIR}/QtPDF_trans.qrc)

set(qtpdf_qrc_files
    ${CMAKE_CURRENT_BINARY_DIR}/QtPDF_trans.qrc
    ${projdir}/source/modules/QtPDF/QtPDF_icons.qrc
)

if(QT_SERIES STREQUAL "5")
    qt5_add_resources(qtpdf_rcc_sources ${qtpdf_qrc_files})
else()
    qt_add_resources(qtpdf_rcc_sources ${qtpdf_qrc_files})
endif()

add_library(qtpdf-static STATIC
    ${qtpdf_headers}
    ${qtpdf_ui_headers}
    ${qtpdf_rcc_sources}
    ${qtpdf_sources}
)

set_property(TARGET qtpdf-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(qtpdf-static
    PUBLIC
        USE_POPPLERQT
)

target_include_directories(qtpdf-static
    PUBLIC
        ${projdir}/source/modules/QtPDF/src
        ${projdir}/source/modules/QtPDF/src/backend
)

if(USE_SYSTEM_POPPLER_QT)
    target_link_libraries(qtpdf-static PUBLIC MiKTeX::Imported::POPPLER_QT)
else()
    target_link_libraries(qtpdf-static PUBLIC ${poppler_qt_dll_name})
endif()

target_link_libraries(qtpdf-static
    PRIVATE
        Qt${QT_SERIES}::Concurrent
        Qt${QT_SERIES}::Core
        Qt${QT_SERIES}::Gui
        Qt${QT_SERIES}::Widgets
        Qt${QT_SERIES}::Xml
)

if(QT_SERIES STREQUAL "6")
    target_link_libraries(qtpdf-static
        PRIVATE
            Qt${QT_SERIES}::Core5Compat
    )
endif()
