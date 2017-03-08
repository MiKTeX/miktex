## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/jpeg")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(-DIN_JPEG_DLL)
endif()

ignore_warnings()

set(${jpeg_dll_name}_sources
  #COMOBJECTS
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jaricom.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jcomapi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jutils.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jerror.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jmemmgr.c
  #SYSDEPMEM
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jmemnobs.c
  #CLIBOBJECTS
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jcapimin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jcapistd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jcarith.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jctrans.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jcparam.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdatadst.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jcinit.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jcmaster.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jcmarker.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jcmainct.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jcprepct.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jccoefct.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jccolor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jcsample.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jchuff.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jcdctmgr.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jfdctfst.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jfdctflt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jfdctint.c
  #DLIBOBJECTS
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdapimin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdapistd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdarith.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdtrans.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdatasrc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdmaster.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdinput.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdmarker.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdhuff.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdmainct.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdcoefct.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdpostct.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jddctmgr.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jidctfst.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jidctflt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jidctint.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdsample.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdcolor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jquant1.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jquant2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/jdmerge.c

  ${CMAKE_CURRENT_BINARY_DIR}/include/jconfig.h
  libjpeg-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    source/jconfig.vc
    ${CMAKE_CURRENT_BINARY_DIR}/include/jconfig.h
    COPYONLY
  )
  configure_file(
    libjpeg.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/libjpeg.rc
  )
  set(${jpeg_dll_name}_sources ${${jpeg_dll_name}_sources}
    ${CMAKE_CURRENT_BINARY_DIR}/libjpeg.rc
  )
endif()

add_library(${jpeg_dll_name} SHARED ${${jpeg_dll_name}_sources})

set_property(TARGET ${jpeg_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${jpeg_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_MAJOR_MINOR_STR}"
    SOVERSION "1"
)

target_compile_definitions(${jpeg_dll_name}
  INTERFACE
    -DHAVE_LIBJPEG=1
)

target_include_directories(${jpeg_dll_name}
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

rebase(${jpeg_dll_name})

install(TARGETS ${jpeg_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

set(public_headers
  jpeglib.h
  jmorecfg.h
  jerror.h
)

foreach(h ${public_headers})
  configure_file(
    source/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
    COPYONLY
  )
endforeach()
