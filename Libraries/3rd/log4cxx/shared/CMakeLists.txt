## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2015-2020 Christian Schenk
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

set(${log4cxx_dll_name}_sources ${log4cxx_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../log4cxx.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/log4cxx.rc
  )
  list(APPEND ${log4cxx_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/log4cxx.rc
  )
endif()

add_library(${log4cxx_dll_name} SHARED ${${log4cxx_dll_name}_sources})

set_property(TARGET ${log4cxx_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${log4cxx_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_MAJOR_MINOR_STR}"
    SOVERSION "1"
)

target_compile_definitions(${log4cxx_dll_name}
  PRIVATE
    -DLOG4CXX
)

target_include_directories(${log4cxx_dll_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${log4cxx_dll_name}
  PRIVATE
    ${system_libraries}
)

if(USE_SYSTEM_APR)
  target_link_libraries(${log4cxx_dll_name} PUBLIC MiKTeX::Imported::APR)
else()
  target_link_libraries(${log4cxx_dll_name} PUBLIC ${apr_dll_name})
endif()

if(USE_SYSTEM_APRUTIL)
  target_link_libraries(${log4cxx_dll_name} PUBLIC MiKTeX::Imported::APRUTIL)
else()
  target_link_libraries(${log4cxx_dll_name} PUBLIC ${apr_util_dll_name})
endif()

install(TARGETS ${log4cxx_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
