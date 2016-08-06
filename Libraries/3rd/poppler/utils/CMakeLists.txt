## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2015-2016 Christian Schenk
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
  add_executable(pdftoppm ${pdftoppm_sources})
  set_property(TARGET pdftoppm PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_link_libraries(pdftoppm
    ${app_dll_name}
    ${poppler_lib_name}
  )
  install(TARGETS pdftoppm DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
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
  add_executable(pdftocairo ${pdftocairo_sources})
  set_property(TARGET pdftocairo PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_link_libraries(pdftocairo
    ${app_dll_name}
    ${poppler_lib_name}
  )
  install(TARGETS pdftocairo DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
endif()

set(pdfdetach_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${common_sources}
  ${projdir}/source/utils/pdfdetach.cc
)
add_executable(pdfdetach ${pdfdetach_sources})
set_property(TARGET pdfdetach PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(pdfdetach
  ${app_dll_name}
  ${poppler_lib_name}
)
install(TARGETS pdfdetach DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdffonts_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${common_sources}
  ${projdir}/source/utils/pdffonts.cc
)
add_executable(pdffonts ${pdffonts_sources})
set_property(TARGET pdffonts PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(pdffonts
  ${app_dll_name}
  ${poppler_lib_name}
)
install(TARGETS pdffonts DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdfimages_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${common_sources}
  ${projdir}/source/utils/ImageOutputDev.cc
  ${projdir}/source/utils/ImageOutputDev.h
  ${projdir}/source/utils/JSInfo.cc
  ${projdir}/source/utils/JSInfo.h
  ${projdir}/source/utils/pdfimages.cc
)
add_executable(pdfimages ${pdfimages_sources})
set_property(TARGET pdfimages PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(pdfimages
  ${app_dll_name}
  ${poppler_lib_name}
)
install(TARGETS pdfimages DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdfinfo_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${common_sources}
  ${projdir}/source/utils/JSInfo.cc
  ${projdir}/source/utils/JSInfo.h
  ${projdir}/source/utils/pdfinfo.cc
  ${projdir}/source/utils/printencodings.cc
)
add_executable(pdfinfo ${pdfinfo_sources})
set_property(TARGET pdfinfo PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(pdfinfo
  ${app_dll_name}
  ${poppler_lib_name}
)
install(TARGETS pdfinfo DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdftops_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${common_sources}
  ${projdir}/source/utils/pdftops.cc
)
add_executable(pdftops ${pdftops_sources})
set_property(TARGET pdftops PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(pdftops
  ${app_dll_name}
  ${poppler_lib_name}
)
install(TARGETS pdftops DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdftotext_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${common_sources}
  ${projdir}/source/utils/pdftotext.cc
  ${projdir}/source/utils/printencodings.cc
)
add_executable(pdftotext ${pdftotext_sources})
set_property(TARGET pdftotext PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(pdftotext
  ${app_dll_name}
  ${poppler_lib_name}
)
install(TARGETS pdftotext DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdftohtml_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${common_sources}
  ${projdir}/source/utils/HtmlFonts.cc
  ${projdir}/source/utils/HtmlLinks.cc
  ${projdir}/source/utils/HtmlOutputDev.cc
  ${projdir}/source/utils/pdftohtml.cc
)
add_executable(pdftohtml ${pdftohtml_sources})
set_property(TARGET pdftohtml PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(pdftohtml
  ${app_dll_name}
  ${poppler_lib_name}
)
install(TARGETS pdftohtml DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdfseparate_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${common_sources}
  ${projdir}/source/utils/pdfseparate.cc
)
add_executable(pdfseparate ${pdfseparate_sources})
set_property(TARGET pdfseparate PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(pdfseparate
  ${app_dll_name}
  ${poppler_lib_name}
)
install(TARGETS pdfseparate DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(pdfunite_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${common_sources}
  ${projdir}/source/utils/pdfunite.cc
)
add_executable(pdfunite ${pdfunite_sources})
set_property(TARGET pdfunite PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(pdfunite
  ${app_dll_name}
  ${poppler_lib_name}
)
install(TARGETS pdfunite DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
