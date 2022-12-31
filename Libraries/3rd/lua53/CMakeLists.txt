## CMakeLists.txt
##
## Copyright (C) 2010-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/lua53")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(public_compile_definitions
    -DLUAI_HASHLIMIT=6
    -DLUA_COMPAT_5_2
    -DLUA_COMPAT_MODULE
)

if(MIKTEX_UNIX_ALIKE)
    list(APPEND public_compile_definitions
        -DLUA_USE_DLOPEN
        -DLUA_USE_POSIX
    )
endif()

configure_file(
    miktex-liblua-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-liblua-version.h
)

list(APPEND configured_headers ${CMAKE_CURRENT_BINARY_DIR}/miktex-liblua-version.h)

set(public_headers
    lapi.h
    lauxlib.h
    lcode.h
    lctype.h
    ldebug.h
    ldo.h
    lfunc.h
    lgc.h
    llex.h
    llimits.h
    lmem.h
    lobject.h
    lopcodes.h
    lparser.h
    lstate.h
    lstring.h
    ltable.h
    ltm.h
    lua.h
    lua.hpp
    luaconf.h
    lualib.h
    lundump.h
    lvm.h
    lzio.h
)

foreach(h ${public_headers})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/source/src/${h}
        ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
        COPYONLY
    )
    list(APPEND configured_headers ${CMAKE_CURRENT_BINARY_DIR}/include/${h})
endforeach()

include(sources.cmake)

set(CMAKE_C_VISIBILITY_PRESET hidden)

add_subdirectory(shared)
set(lua53_target_library ${lua53_dll_name})

add_subdirectory(static)

generate_export_header(${lua53_target_library}
    BASE_NAME MIKTEX_LUA
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/lua/export.h
    STATIC_DEFINE MIKTEX_LUA_STATIC
)

add_executable(${MIKTEX_PREFIX}lua ${lua_sources})
set_property(TARGET ${MIKTEX_PREFIX}lua PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(${MIKTEX_PREFIX}lua ${lua53_target_library})
install(TARGETS ${MIKTEX_PREFIX}lua DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

add_executable(${MIKTEX_PREFIX}luac ${luac_sources})
set_property(TARGET ${MIKTEX_PREFIX}luac PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(${MIKTEX_PREFIX}luac ${lua53_target_library})
install(TARGETS ${MIKTEX_PREFIX}luac DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

add_subdirectory(test)
