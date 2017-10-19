## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2013-2017 Christian Schenk
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

add_library(${harfbuzz_lib_name} STATIC ${harfbuzz_sources})

set_property(TARGET ${harfbuzz_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${harfbuzz_lib_name}
  PUBLIC
    ${public_include_directories}
)

if(USE_SYSTEM_GRAPHITE2)
  target_link_libraries(${harfbuzz_lib_name} PUBLIC MiKTeX::Imported::GRAPHITE2)
else()
  target_link_libraries(${harfbuzz_lib_name} PUBLIC ${graphite2_dll_name})
endif()

target_link_libraries(${harfbuzz_lib_name}
  PUBLIC
    ICU::i18n
    ICU::uc
)
