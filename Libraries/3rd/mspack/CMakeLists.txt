## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/mspack")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack
)

add_definitions(
    -DHAVE_CONFIG_H
)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
    miktex-mspack-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-mspack-version.h
)

set(mspack_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/cab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/cabc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/cabd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/chm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/chmc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/chmd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/crc32.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/crc32.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/des.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/hlp.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/hlpc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/hlpd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/kwaj.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/kwajc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/kwajd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/lit.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/litc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/litd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/lzss.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/lzssd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/lzx.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/lzxc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/lzxd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/mspack.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/mszip.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/mszipc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/mszipd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/oab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/oabc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/oabd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/qtm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/qtmd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/readbits.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/readhuff.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/sha.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/system.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/system.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/szdd.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/szddc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/szddd.c

  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${CMAKE_CURRENT_BINARY_DIR}/miktex-mspack-version.h
)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
