## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/regex")

include_directories(BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories ${CMAKE_CURRENT_BINARY_DIR}/include)

configure_file(
    miktex-regex-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-regex-version.h
)

set(sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-regex-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/regex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/regex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/regex_internal.h
)

set(${regex_dll_name}_sources ${sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-regex.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-regex.rc
    )
    set(${regex_dll_name}_sources ${${regex_dll_name}_sources}
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-regex.rc
    )
endif()

add_library(${regex_dll_name} SHARED ${${regex_dll_name}_sources})

set_property(TARGET ${regex_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${regex_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${regex_dll_name}
    PUBLIC
        -DMIKTEX_REGEX_SHARED
)

target_include_directories(${regex_dll_name}
    PUBLIC
        ${public_include_directories}
)

install(TARGETS ${regex_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

set(public_headers
    regex.h
)

foreach(h ${public_headers})
    configure_file(
        source/posix/${h}
        ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
        COPYONLY
    )
endforeach()

