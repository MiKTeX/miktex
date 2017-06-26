## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2017 Christian Schenk
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

use_static_crt()

add_library(${mpm_lib_name} STATIC ${mpm_sources})

set_property(TARGET ${mpm_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${mpm_lib_name}
  PUBLIC
    -DMIKTEX_MPM_STATIC
)

target_include_directories(${mpm_lib_name}
    PUBLIC
      ${public_include_directories}
)

if(USE_SYSTEM_EXPAT)
  target_link_libraries(${mpm_lib_name} PUBLIC MiKTeX::Imported::EXPAT)
else()
  target_link_libraries(${mpm_lib_name} PUBLIC ${expat_lib_name})
endif()

target_link_libraries(${mpm_lib_name}
  PUBLIC
    ${core_lib_name}
    ${extractor_lib_name}
    ${md5_lib_name}
    ${nlohmann_json_lib_name}
  )

if(USE_SYSTEM_CURL)
  target_link_libraries(${mpm_lib_name} PUBLIC MiKTeX::Imported::CURL)
else()
  target_link_libraries(${mpm_lib_name}
    PUBLIC
      ${curl_lib_name}
      Ws2_32
      wininet
  )
endif()

source_group(Public FILES ${public_headers})
