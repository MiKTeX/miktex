## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2009-2017 Christian Schenk
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

add_definitions(-Dpoppler_qt5_EXPORTS)

set(CMAKE_AUTOMOC ON)

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(${poppler_qt_dll_name}_sources
  ${projdir}/source/qt5/src/poppler-annotation.cc
  ${projdir}/source/qt5/src/poppler-document.cc
  ${projdir}/source/qt5/src/poppler-embeddedfile.cc
  ${projdir}/source/qt5/src/poppler-fontinfo.cc
  ${projdir}/source/qt5/src/poppler-form.cc
  ${projdir}/source/qt5/src/poppler-link.cc
  ${projdir}/source/qt5/src/poppler-link-extractor.cc
  ${projdir}/source/qt5/src/poppler-movie.cc
  ${projdir}/source/qt5/src/poppler-optcontent.cc
  ${projdir}/source/qt5/src/poppler-page.cc
  ${projdir}/source/qt5/src/poppler-base-converter.cc
  ${projdir}/source/qt5/src/poppler-pdf-converter.cc
  ${projdir}/source/qt5/src/poppler-private.cc
  ${projdir}/source/qt5/src/poppler-ps-converter.cc
  ${projdir}/source/qt5/src/poppler-qiodeviceoutstream.cc
  ${projdir}/source/qt5/src/poppler-sound.cc
  ${projdir}/source/qt5/src/poppler-textbox.cc
  ${projdir}/source/qt5/src/poppler-page-transition.cc
  ${projdir}/source/qt5/src/poppler-media.cc
  ${projdir}/source/qt5/src/ArthurOutputDev.cc
  
  ${projdir}/source/qt5/src/ArthurOutputDev.h
  ${projdir}/source/qt5/src/poppler-annotation-helper.h
  ${projdir}/source/qt5/src/poppler-annotation-private.h
  ${projdir}/source/qt5/src/poppler-annotation.h
  ${projdir}/source/qt5/src/poppler-converter-private.h
  ${projdir}/source/qt5/src/poppler-embeddedfile-private.h
  ${projdir}/source/qt5/src/poppler-export.h
  ${projdir}/source/qt5/src/poppler-form.h
  ${projdir}/source/qt5/src/poppler-link-extractor-private.h
  ${projdir}/source/qt5/src/poppler-link-private.h
  ${projdir}/source/qt5/src/poppler-link.h
  ${projdir}/source/qt5/src/poppler-media.h
  ${projdir}/source/qt5/src/poppler-optcontent-private.h
  ${projdir}/source/qt5/src/poppler-optcontent.h
  ${projdir}/source/qt5/src/poppler-page-private.h
  ${projdir}/source/qt5/src/poppler-page-transition-private.h
  ${projdir}/source/qt5/src/poppler-page-transition.h
  ${projdir}/source/qt5/src/poppler-private.h
  ${projdir}/source/qt5/src/poppler-qiodeviceoutstream-private.h
  ${projdir}/source/qt5/src/poppler-qt5.h
)

add_library(${poppler_qt_dll_name} SHARED ${${poppler_qt_dll_name}_sources})

set_property(TARGET ${poppler_qt_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${poppler_qt_dll_name}
  PROPERTIES
   VERSION "${MIKTEX_MAJOR_MINOR_STR}"
   SOVERSION 1
)

target_include_directories(${poppler_qt_dll_name}
  PUBLIC
    ${projdir}/source/qt5/src
)

target_link_libraries(${poppler_qt_dll_name}
  PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Xml
    ${core_dll_name}
  PUBLIC
    ${poppler_dll_name}
)

rebase(${poppler_qt_dll_name})

install(TARGETS ${poppler_qt_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
