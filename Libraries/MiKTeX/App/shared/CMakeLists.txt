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

set(${app_dll_name}_sources ${app_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../app.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/app.rc
  )
  list(APPEND ${app_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/app.rc
  )
endif()

add_library(${app_dll_name} SHARED ${${app_dll_name}_sources})

set_property(TARGET ${app_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version(
  ${app_dll_name}
  ${MIKTEX_COMP_INTERFACE_VERSION}
  ${MIKTEX_COMP_J2000_VERSION}
)

target_compile_definitions(${app_dll_name}
  PUBLIC
    -DMIKTEX_APP_SHARED
)

target_include_directories(${app_dll_name}
  PUBLIC
    ${public_include_directories}
)

if(USE_SYSTEM_FMT)
  target_link_libraries(${app_dll_name} PRIVATE MiKTeX::Imported::FMT)
else()
  target_link_libraries(${app_dll_name} PRIVATE ${fmt_dll_name})
endif()

if(USE_SYSTEM_LOG4CXX)
  target_link_libraries(${app_dll_name} PRIVATE MiKTeX::Imported::LOG4CXX)
else()
  target_link_libraries(${app_dll_name} PRIVATE ${log4cxx_dll_name})
endif()

target_link_libraries(${app_dll_name}
  PUBLIC
    ${core_dll_name}
    ${mpm_dll_name}
    ${setup_dll_name}
)

if(WITH_UI_QT)
  target_link_libraries(${app_dll_name}
    PRIVATE
      ${ui_qt_dll_name}
  )
endif()

delay_load(${app_dll_name}
  ${mpm_dll_name}
  ${ui_qt_dll_name}
)

install(TARGETS ${app_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

source_group(Public FILES ${public_headers})
