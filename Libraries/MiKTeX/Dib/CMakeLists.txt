## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2020 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/dib")

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(public_headers
  include/miktex/Graphics/DibChunker
  include/miktex/Graphics/DibChunker.h
)

set(${dib_dll_name}_sources
  ${public_headers}
  DibChunker.cpp
  dib-version.h
  internal.h
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/dib.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/dib.rc
  )
  set(${dib_dll_name}_sources ${${dib_dll_name}_sources}
    ${CMAKE_CURRENT_BINARY_DIR}/dib.rc
  )
endif()

add_library(${dib_dll_name} SHARED ${${dib_dll_name}_sources})

set_property(TARGET ${dib_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${dib_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_MAJOR_MINOR_STR}"
    SOVERSION "1"
)

target_compile_definitions(${dib_dll_name}
  PUBLIC
    -DMIKTEX_DIB_SHARED
)

target_include_directories(${dib_dll_name}
  PUBLIC
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_DIB_DIR}/include
)

target_link_libraries(${dib_dll_name}
  PRIVATE
    ${core_dll_name}
)

if(USE_SYSTEM_FMT)
  target_link_libraries(${dib_dll_name} PRIVATE MiKTeX::Imported::FMT)
else()
  target_link_libraries(${dib_dll_name} PRIVATE ${fmt_dll_name})
endif()

set(chunkdib_sources chunkdib.cpp)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND chunkdib_sources
    ${MIKTEX_COMMON_MANIFEST}
  )
endif()

add_executable(chunkdib ${chunkdib_sources})

set_property(TARGET chunkdib PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(chunkdib ${dib_dll_name})

target_link_libraries(chunkdib
  ${app_dll_name}
  ${core_dll_name}
  ${dib_dll_name}
  ${getopt_dll_name}
  miktex-popt-wrapper
)  

if (USE_SYSTEM_FMT)
  target_link_libraries(chunkdib MiKTeX::Imported::FMT)
else()
  target_link_libraries(chunkdib ${fmt_dll_name})
endif()

install(TARGETS ${dib_dll_name} chunkdib
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

install(
  FILES
    include/miktex/Graphics/DibChunker
    include/miktex/Graphics/DibChunker.h
  DESTINATION
    ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/Graphics
)

source_group(Public FILES ${public_headers})
