## luasocket.cmake
##
## Copyright (C) 2010-2019 Christian Schenk
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

set(luasocket_sources
  source/luasocket/src/auxiliar.c
  source/luasocket/src/auxiliar.h
  source/luasocket/src/buffer.c
  source/luasocket/src/buffer.h
  source/luasocket/src/compat.c
  source/luasocket/src/compat.h
  source/luasocket/src/except.c
  source/luasocket/src/except.h
  source/luasocket/src/inet.c
  source/luasocket/src/inet.h
  source/luasocket/src/io.c
  source/luasocket/src/io.h
  source/luasocket/src/lua_preload.c
  source/luasocket/src/luasocket.c
  source/luasocket/src/luasocket.h
  source/luasocket/src/mime.c
  source/luasocket/src/mime.h
  source/luasocket/src/options.c
  source/luasocket/src/options.h
  source/luasocket/src/select.c
  source/luasocket/src/select.h
  source/luasocket/src/serial.c
  source/luasocket/src/socket.c
  source/luasocket/src/socket.h
  source/luasocket/src/tcp.c
  source/luasocket/src/tcp.h
  source/luasocket/src/timeout.c
  source/luasocket/src/timeout.h
  source/luasocket/src/udp.c
  source/luasocket/src/udp.h
  source/luasocket/src/usocket.h
)

set(luasocket_lua_sources
  ftp.lua
  headers.lua
  http.lua
  ltn12.lua
  mbox.lua
  mime.lua
  smtp.lua
  socket.lua
  tp.lua
  url.lua
)

foreach(f ${luasocket_lua_sources})
  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/luasocket-${f}.h
    COMMAND
      bintoc B true
        < ${CMAKE_CURRENT_SOURCE_DIR}/source/luasocket/src/${f}
        > ${CMAKE_CURRENT_BINARY_DIR}/luasocket-${f}.h.tmp
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/luasocket-${f}.h.tmp
        ${CMAKE_CURRENT_BINARY_DIR}/luasocket-${f}.h
    COMMAND
      ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/luasocket-${f}.h.tmp
    DEPENDS
      bintoc
      ${CMAKE_CURRENT_SOURCE_DIR}/source/luasocket/src/${f}
    VERBATIM
  )
  list(APPEND generated_luasocket_sources ${CMAKE_CURRENT_BINARY_DIR}/luasocket-${f}.h)
endforeach()

add_custom_target(
  gen-luasocket-sources
  DEPENDS
    ${generated_luasocket_sources}
)

list(APPEND luasocket_sources ${generated_luasocket_sources})

add_library(luatex-lua53socket-objects OBJECT ${luasocket_sources})

set_property(TARGET luatex-lua53socket-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(luatex-lua53socket-objects gen-luasocket-sources)

set_source_files_properties(
  ${generated_luasocket_sources}
  PROPERTIES
    GENERATED TRUE
)

target_compile_definitions(luatex-lua53socket-objects
  PRIVATE
    -DLUASOCKET_DEBUG
)

target_link_libraries(luatex-lua53socket-objects
  PUBLIC
    ${lua53_target_name}
)
