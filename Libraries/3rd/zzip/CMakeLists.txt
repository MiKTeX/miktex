## CMakeLists.txt
##
## Copyright (C) 2010-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/zzip")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(ZZIP_HAVE_DIRENT_H HAVE_DIRENT_H)
set(ZZIP_HAVE_STDINT_H HAVE_STDINT_H)
set(ZZIP_HAVE_SYS_MMAN_H HAVE_SYS_MMAN_H)
set(ZZIP_HAVE_SYS_PARAM_H HAVE_SYS_PARAM_H)
set(ZZIP_HAVE_SYS_STAT_H HAVE_SYS_STAT_H)
set(ZZIP_HAVE_SYS_TYPES_H HAVE_SYS_TYPES_H)
set(ZZIP_HAVE_UNISTD_H HAVE_UNISTD_H)

add_definitions(
    -DZZIP_EXPORTS=1
    -D_USE_MMAP=1
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
        -DUNICODE
        -DZZIP_HAVE_DIRENT_H=1
        -DZZIP_HAVE_STDINT_H=1
        -D_UNICODE
  )
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/_config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/zzip/_config.h
)

configure_file(
    miktex-zzip-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-zzip-version.h
)

set(sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/dir.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/err.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/fetch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/file.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/info.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/plugin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/stat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/write.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/zip.c

    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/__debug.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/__dirent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/__fnmatch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/__hints.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/__mmap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/_msvc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/autoconf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/conf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/fetch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/file.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/format.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/fseeko.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/info.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/lib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/memdisk.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/mmapped.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/plugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/stdint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/write.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/zzip.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/zzip32.h

    ${CMAKE_CURRENT_BINARY_DIR}/include/zzip/_config.h
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-zzip-version.h
)

set(public_headers
    _msvc.h
    conf.h
    fetch.h
    file.h
    format.h
    fseeko.h
    info.h
    lib.h
    memdisk.h
    mmapped.h
    plugin.h
    stdint.h
    types.h
    write.h
    zzip.h
    zzip32.h
)

foreach(hdr ${public_headers})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/${hdr}
        ${CMAKE_CURRENT_BINARY_DIR}/include/zzip/${hdr}
        COPYONLY
    )
endforeach()

set(${zzip_dll_name}_sources ${sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-zzip.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-zzip.rc
    )
    list(APPEND ${zzip_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-zzip.rc
        ${CMAKE_CURRENT_SOURCE_DIR}/source/zzip/_msvc.h
    )
endif()

add_library(${zzip_dll_name} SHARED ${${zzip_dll_name}_sources})

set_property(TARGET ${zzip_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${zzip_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${zzip_dll_name}
    PUBLIC
        ${public_include_directories}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${zzip_dll_name}
        PRIVATE
            ${unxemu_dll_name}
            ${utf8wrap_dll_name}
    )
endif()

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${zzip_dll_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${zzip_dll_name} PRIVATE ${zlib_dll_name})
endif()

install(TARGETS ${zzip_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
