## CMakeLists.txt					-*- CMake -*-
##
## Copyright (C) 2006-2018 Christian Schenk
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

add_library(${extractor_lib_name} STATIC ${extractor_sources})

set_property(TARGET ${extractor_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${extractor_lib_name}
  PUBLIC
    -DMIKTEX_EXTRACTOR_STATIC
)

target_include_directories(${extractor_lib_name}
  PUBLIC
    ${public_include_directories}
)

if(USE_SYSTEM_BZIP2)
  target_link_libraries(${extractor_lib_name} PUBLIC MiKTeX::Imported::BZIP2)
else()
  target_link_libraries(${extractor_lib_name} PUBLIC ${bzip2_lib_name})
endif()

if(USE_SYSTEM_FMT)
  target_link_libraries(${extractor_lib_name} PUBLIC MiKTeX::Imported::FMT)
else()
  target_link_libraries(${extractor_lib_name} PUBLIC ${fmt_lib_name})
endif()

if(USE_SYSTEM_MSPACK)
  target_link_libraries(${extractor_lib_name} PUBLIC MiKTeX::Imported::MSPACK)
else()
  target_link_libraries(${extractor_lib_name} PUBLIC ${mspack_lib_name})
endif()

target_link_libraries(${extractor_lib_name}
  PUBLIC
    ${core_lib_name}   
)

source_group(Public FILES ${public_headers})
