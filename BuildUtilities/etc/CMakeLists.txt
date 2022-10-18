## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}")

add_executable(bintoc bintoc.cpp)

set_property(TARGET bintoc PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
    add_executable(headerguard headerguard.cpp)
    set_property(TARGET headerguard PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
    target_link_libraries(headerguard Rpcrt4)
endif()

add_executable(j2000 j2000.cpp)

set_property(TARGET j2000 PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_executable(web-n web-n.cpp)

set_property(TARGET web-n PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

configure_file(
    hhc.cmd.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/hhc.cmd
)
