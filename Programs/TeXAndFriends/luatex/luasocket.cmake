## luasocket.cmake
##
## Copyright (C) 2010-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

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
set_property(TARGET gen-luasocket-sources PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

list(APPEND luasocket_sources ${generated_luasocket_sources})

add_library(luatex-luasocket-objects OBJECT ${luasocket_sources})

set_property(TARGET luatex-luasocket-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(luatex-luasocket-objects gen-luasocket-sources)

set_source_files_properties(
    ${generated_luasocket_sources}
    PROPERTIES
        GENERATED TRUE
)

target_compile_definitions(luatex-luasocket-objects
    PRIVATE
        -DLUASOCKET_DEBUG
)

target_link_libraries(luatex-luasocket-objects
    PUBLIC
        ${lua53_target_name}
)
