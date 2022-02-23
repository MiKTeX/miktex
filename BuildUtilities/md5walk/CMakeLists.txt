## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}")

use_static_crt()

set(md5walk_sources
    md5walk-version.h
    md5walk.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND md5walk_sources
        ${MIKTEX_COMMON_MANIFEST}
        md5walk.rc
    )
endif()

add_executable(md5walk ${md5walk_sources})

set_property(TARGET md5walk PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(md5walk
    ${core_lib_name}
    miktex-popt-wrapper-static
)

if (USE_SYSTEM_FMT)
    target_link_libraries(md5walk MiKTeX::Imported::FMT)
else()
    target_link_libraries(md5walk ${fmt_lib_name})
endif()

install(TARGETS md5walk DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
