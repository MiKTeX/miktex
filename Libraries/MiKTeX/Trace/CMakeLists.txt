## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2022 Christian Schenk
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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/trace")

set(trace_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
set(trace_binary_dir ${CMAKE_CURRENT_BINARY_DIR})

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(public_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Trace/StopWatch
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Trace/StopWatch.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Trace/Trace
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Trace/Trace.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Trace/TraceCallback
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Trace/TraceCallback.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Trace/TraceStream
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Trace/TraceStream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Trace/config.h
)

configure_file(
  trace-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/trace-version.h
)

set(trace_sources
  ${CMAKE_CURRENT_BINARY_DIR}/trace-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/StopWatch.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TraceStream.cpp
  ${public_headers}
)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
