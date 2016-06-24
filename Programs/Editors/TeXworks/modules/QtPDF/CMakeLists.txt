## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2008-2016 Christian Schenk
## 
## This file is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published
## by the Free Software Foundation; either version 2, or (at your
## option) any later version.
## 
## This file is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this file; if not, write to the Free Software
## Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
## USA.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_EDITORS_FOLDER}/TeXworks/QtPDF")

hide_symbols()

set(qtpdf_sources
  ${projdir}/source/modules/QtPDF/src/PDFDocumentView.cpp
  ${projdir}/source/modules/QtPDF/src/PDFDocumentWidget.cpp
  ${projdir}/source/modules/QtPDF/src/PDFDocumentTools.cpp
  ${projdir}/source/modules/QtPDF/src/PDFBackend.cpp
  ${projdir}/source/modules/QtPDF/src/PDFTransitions.cpp
  ${projdir}/source/modules/QtPDF/src/PDFActions.cpp
  ${projdir}/source/modules/QtPDF/src/PDFAnnotations.cpp

  ${projdir}/source/modules/QtPDF/src/backends/PopplerQtBackend.cpp
)

set(qtpdf_headers
  ${projdir}/source/modules/QtPDF/src/PDFDocumentView.h
  ${projdir}/source/modules/QtPDF/src/PDFDocumentWidget.h
  ${projdir}/source/modules/QtPDF/src/PDFDocumentTools.h
  ${projdir}/source/modules/QtPDF/src/PDFBackend.h
  ${projdir}/source/modules/QtPDF/src/PDFTransitions.h
  ${projdir}/source/modules/QtPDF/src/PDFActions.h
  ${projdir}/source/modules/QtPDF/src/PDFAnnotations.h

  ${projdir}/source/modules/QtPDF/src/backends/PopplerQtBackend.h
)

set(qtpdf_rcs_files
  ${projdir}/source/modules/QtPDF/QtPDF_icons.qrc
  ${projdir}/modules/QtPDF/QtPDF_trans.qrc
)

qt5_add_resources(qtpdf_rcc_sources ${qtpdf_rcs_files})

add_library(qtpdf-static STATIC
  ${qtpdf_sources}
  ${qtpdf_headers}
  ${qtpdf_rcc_sources}
)

set_property(TARGET qtpdf-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(qtpdf-static
  PUBLIC
    ${projdir}/source/modules/QtPDF/src
    ${projdir}/source/modules/QtPDF/src/backend
)

target_link_libraries(qtpdf-static
  PRIVATE
    Qt5::Concurrent
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
  PUBLIC
    ${poppler_qt_dll_name}
)
