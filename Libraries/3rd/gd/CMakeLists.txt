## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/gd")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DBGDWIN32
    )
endif()

set(ENABLE_GD_FORMATS FALSE)

configure_file(
    source/src/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
    miktex-gd-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-gd-version.h
)

set(sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-gd-version.h
    
    source/src/gd.c
    source/src/gd_avif.c
    source/src/gd_bmp.c
    source/src/gd_color.c
    source/src/gd_color_map.c
    source/src/gd_color_match.c
    source/src/gd_crop.c
    source/src/gd_filename.c
    source/src/gd_filter.c
    source/src/gd_gd.c
    source/src/gd_gd2.c
    source/src/gd_gif_in.c
    source/src/gd_gif_out.c
    source/src/gd_heif.c
    source/src/gd_interpolation.c
    source/src/gd_io.c
    source/src/gd_io_dp.c
    source/src/gd_io_file.c
    source/src/gd_io_ss.c
    source/src/gd_io_stream.cxx
    source/src/gd_jpeg.c
    source/src/gd_matrix.c
    source/src/gd_nnquant.c
    source/src/gd_png.c
    source/src/gd_rotate.c
    source/src/gd_security.c
    source/src/gd_ss.c
    source/src/gd_tga.c
    source/src/gd_tiff.c
    source/src/gd_topal.c
    source/src/gd_transform.c
    source/src/gd_version.c
    source/src/gd_wbmp.c
    source/src/gd_webp.c
    source/src/gd_xbm.c
    source/src/gdcache.c
    source/src/gdfontg.c
    source/src/gdfontl.c
    source/src/gdfontmb.c
    source/src/gdfonts.c
    source/src/gdfontt.c
    source/src/gdft.c
    source/src/gdfx.c
    source/src/gdhelpers.c
    source/src/gdkanji.c
    source/src/gdpp.cxx
    source/src/gdtables.c
    source/src/gdxpm.c
    source/src/wbmp.c

    source/src/bmp.h
    source/src/entities.h
    source/src/gd.h
    source/src/gdcache.h
    source/src/gdfontg.h
    source/src/gdfontl.h
    source/src/gdfontmb.h
    source/src/gdfonts.h
    source/src/gdfontt.h
    source/src/gdfx.h
    source/src/gdhelpers.h
    source/src/gdpp.h
    source/src/gd_color.h
    source/src/gd_color_map.h
    source/src/gd_errors.h
    source/src/gd_intern.h
    source/src/gd_io.h
    source/src/gd_io_stream.h
    source/src/gd_nnquant.h
    source/src/gd_tga.h
    source/src/jisx0208.h
    source/src/wbmp.h
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-gd.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-gd.rc
    )
    list(APPEND sources ${CMAKE_CURRENT_BINARY_DIR}/miktex-gd.rc)
endif()

add_library(${gd_dll_name} SHARED ${sources})

set_property(TARGET ${gd_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${gd_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${gd_dll_name}
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/include
)

if(USE_SYSTEM_FONTCONFIG)
    target_link_libraries(${gd_dll_name} PRIVATE MiKTeX::Imported::FONTCONFIG)
else()
    target_link_libraries(${gd_dll_name} PRIVATE ${fontconfig_dll_name})
endif()

if(USE_SYSTEM_FREETYPE2)
    target_link_libraries(${gd_dll_name} PRIVATE MiKTeX::Imported::FREETYPE2)
else()
    target_link_libraries(${gd_dll_name} PRIVATE ${freetype2_dll_name})
endif()

if(USE_SYSTEM_JPEG)
    target_link_libraries(${gd_dll_name} PRIVATE MiKTeX::Imported::JPEG)
else()
    target_link_libraries(${gd_dll_name} PRIVATE ${jpeg_dll_name})
endif()

if(USE_SYSTEM_PNG)
    target_link_libraries(${gd_dll_name} PRIVATE MiKTeX::Imported::PNG)
else()
    target_link_libraries(${gd_dll_name} PRIVATE ${png_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${gd_dll_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${gd_dll_name} PRIVATE ${zlib_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${gd_dll_name}
        PRIVATE
            ${utf8wrap_dll_name}
    )
endif()

delay_load(${gd_dll_name}
    ${fontconfig_dll_name}
    ${freetype2_dll_name}
    ${jpeg_dll_name}
    ${png_dll_name}
    ${utf8wrap_dll_name}
    ${zlib_dll_name}
)

install(TARGETS ${gd_dll_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

set(public_headers
    gd.h
    gd_color_map.h
    gd_errors.h
    gd_io.h
    gdcache.h
    gdfontg.h
    gdfontl.h
    gdfontmb.h
    gdfonts.h
    gdfontt.h
    gdfx.h
)

foreach(h ${public_headers})
    configure_file(
        source/src/${h}
        ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
        COPYONLY
    )
endforeach()
