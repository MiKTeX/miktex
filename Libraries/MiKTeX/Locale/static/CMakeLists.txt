## CMakeLists.txt
##
## Copyright (C) 2020-2021 Christian Schenk
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

add_library(${loc_lib_name} STATIC ${sources})

set_property(TARGET ${loc_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${loc_lib_name}
  PUBLIC
    -DMIKTEX_LOC_STATIC
)

target_include_directories(${loc_lib_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${loc_lib_name}
  PRIVATE
    ${util_lib_name}
  PUBLIC
    ${config_lib_name}
    ${res_lib_name}
)

if(MIKTEX_MACOS_BUNDLE)
  target_link_libraries(${loc_lib_name}
    PRIVATE
      ${CoreFoundation}
  )
endif()

if(USE_SYSTEM_BOOST)
  target_link_libraries(${loc_lib_name} PRIVATE Boost::locale)
endif()

source_group(Public FILES ${public_headers})
