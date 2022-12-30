## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/jpeg")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(-DIN_JPEG_DLL)
endif()

configure_file(
    miktex-libjpeg-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-libjpeg-version.h
)

    set(${jpeg_dll_name}_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jaricom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jcapimin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jcapistd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jcarith.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jccoefct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jccolor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jcdctmgr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jchuff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jcinit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jcmainct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jcmarker.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jcmaster.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jcomapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jcparam.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jcprepct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jcsample.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jctrans.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdapimin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdapistd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdarith.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdatadst.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdatasrc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdcoefct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdcolor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jddctmgr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdhuff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdinput.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdmainct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdmarker.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdmaster.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdmerge.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdpostct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdsample.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jdtrans.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jerror.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jfdctflt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jfdctfst.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jfdctint.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jidctflt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jidctfst.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jidctint.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jquant1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jquant2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jutils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jmemmgr.c

    ${CMAKE_CURRENT_SOURCE_DIR}/source/jmemnobs.c

    ${CMAKE_CURRENT_BINARY_DIR}/include/jconfig.h

    ${CMAKE_CURRENT_BINARY_DIR}/miktex-libjpeg-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        source/jconfig.vc
        ${CMAKE_CURRENT_BINARY_DIR}/include/jconfig.h
        COPYONLY
    )
    configure_file(
        windows/miktex-libjpeg.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-libjpeg.rc
    )
    list(APPEND ${jpeg_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-libjpeg.rc
    )
endif()

add_library(${jpeg_dll_name} SHARED ${${jpeg_dll_name}_sources})

set_property(TARGET ${jpeg_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${jpeg_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${jpeg_dll_name}
    INTERFACE
        -DHAVE_LIBJPEG=1
)

target_include_directories(${jpeg_dll_name}
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/include
)

install(TARGETS ${jpeg_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

set(public_headers
    jpeglib.h
    jmorecfg.h
    jerror.h
)

foreach(h ${public_headers})
    configure_file(
        source/${h}
        ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
        COPYONLY
    )
endforeach()
