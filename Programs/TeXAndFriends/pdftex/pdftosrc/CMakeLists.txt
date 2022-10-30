## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(pdftosrc_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${projdir}/source/pdftosrc.cc
    pdftosrc-version.h
)

set_source_files_properties(
    ${MIKTEX_LIBRARY_WRAPPER}
    PROPERTIES COMPILE_FLAGS "-DCPLUSPLUSMAIN"
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND pdftosrc_sources
        ${MIKTEX_COMMON_MANIFEST}
        pdftosrc.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}pdftosrc ${pdftosrc_sources})

set_property(TARGET ${MIKTEX_PREFIX}pdftosrc PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/pdftosrc)

if(USE_SYSTEM_JPEG)
    target_link_libraries(${MIKTEX_PREFIX}pdftosrc MiKTeX::Imported::JPEG)
else()
    target_link_libraries(${MIKTEX_PREFIX}pdftosrc ${jpeg_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${MIKTEX_PREFIX}pdftosrc MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${MIKTEX_PREFIX}pdftosrc ${zlib_dll_name})
endif()

target_link_libraries(${MIKTEX_PREFIX}pdftosrc
    ${app_dll_name}
    ${core_dll_name}
    ${w2cemu_dll_name}
    ${xpdf_lib_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}pdftosrc
        ${getopt_dll_name}
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}pdftosrc DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
