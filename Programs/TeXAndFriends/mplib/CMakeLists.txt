## CMakeLists.txt
##
## Copyright (C) 2010-2025 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_TEX_AND_FRIENDS_FOLDER}/MetaPost")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

add_definitions(
    -DAPPTAG=mpost
    -DUSE_MIKTEX_EXIT
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

configure_file(
    miktex-metapost-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-metapost-version.h
)

###############################################################################
## metapost-backends
###############################################################################

set(metapost_backends_cweb_sources
    ${CMAKE_CURRENT_BINARY_DIR}/mplibpng.h
    ${CMAKE_CURRENT_BINARY_DIR}/mplibsvg.h
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathinterval.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathinterval.h
    ${CMAKE_CURRENT_BINARY_DIR}/mppngout.h
    ${CMAKE_CURRENT_BINARY_DIR}/mpsvgout.h
    ${CMAKE_CURRENT_BINARY_DIR}/pngout.c
    ${CMAKE_CURRENT_BINARY_DIR}/pngout.c
    ${CMAKE_CURRENT_BINARY_DIR}/svgout.c
)

set_source_files_properties(
    ${metapost_backends_cweb_sources}
    PROPERTIES GENERATED TRUE
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/mppngout.h
        ${CMAKE_CURRENT_BINARY_DIR}/mplibpng.h
    COMMAND
        ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/pngout
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        source/pngout.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/svgout.c
        ${CMAKE_CURRENT_BINARY_DIR}/mpsvgout.h
        ${CMAKE_CURRENT_BINARY_DIR}/mplibsvg.h
    COMMAND
        ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/svgout
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        source/svgout.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/mpmathinterval.c
        ${CMAKE_CURRENT_BINARY_DIR}/mpmathinterval.h
    COMMAND
        ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpmathinterval
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        source/mpmathinterval.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

set(metapost_backends_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-metapost-version.h
    ${CMAKE_CURRENT_BINARY_DIR}/mplib.h
    ${CMAKE_CURRENT_BINARY_DIR}/mplibps.h
    ${CMAKE_CURRENT_BINARY_DIR}/mpmath.h
    ${metapost_backends_cweb_sources}
)

add_library(${metapost_backends_lib_name} STATIC ${metapost_backends_sources})

set_property(TARGET ${metapost_backends_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${metapost_backends_lib_name}
    PUBLIC
        ${CMAKE_BINARY_DIR}/${MIKTEX_REL_MP_DIR}
)

target_link_libraries(${metapost_backends_lib_name}
    PRIVATE
        ${core_dll_name}
        ${kpsemu_dll_name}
        ${w2cemu_dll_name}
)

if(USE_SYSTEM_CAIRO)
    target_link_libraries(${metapost_backends_lib_name} PRIVATE MiKTeX::Imported::CAIRO)
else()
    target_link_libraries(${metapost_backends_lib_name} PRIVATE ${cairo_dll_name})
endif()

if(USE_SYSTEM_GMP)
    target_link_libraries(${metapost_backends_lib_name} PRIVATE MiKTeX::Imported::GMP)
else()
    target_link_libraries(${metapost_backends_lib_name} PRIVATE ${gmp_lib_name})
endif()

if(USE_SYSTEM_MPFI)
    target_link_libraries(${metapost_backends_lib_name} PRIVATE MiKTeX::Imported::MPFI)
else()
    target_link_libraries(${metapost_backends_lib_name} PRIVATE ${mpfi_lib_name})
endif()

if(USE_SYSTEM_MPFR)
    target_link_libraries(${metapost_backends_lib_name} PRIVATE MiKTeX::Imported::MPFR)
else()
    target_link_libraries(${metapost_backends_lib_name} PRIVATE ${mpfr_lib_name})
endif()

if(USE_SYSTEM_PNG)
    target_link_libraries(${metapost_backends_lib_name} PRIVATE MiKTeX::Imported::PNG)
else()
    target_link_libraries(${metapost_backends_lib_name} PRIVATE ${png_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${metapost_backends_lib_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${metapost_backends_lib_name} PRIVATE ${zlib_dll_name})
endif()

install(TARGETS ${metapost_backends_lib_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

###############################################################################
## metapost-core
###############################################################################

set(metapost_core_cweb_sources
    ${CMAKE_CURRENT_BINARY_DIR}/mp.c
    ${CMAKE_CURRENT_BINARY_DIR}/mplib.h
    ${CMAKE_CURRENT_BINARY_DIR}/mplibps.h
    ${CMAKE_CURRENT_BINARY_DIR}/mpmath.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpmath.h
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathdecimal.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathdecimal.h
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathdouble.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathdouble.h
    ${CMAKE_CURRENT_BINARY_DIR}/mpmp.h
    ${CMAKE_CURRENT_BINARY_DIR}/mppsout.h
    ${CMAKE_CURRENT_BINARY_DIR}/mpstrings.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpstrings.h
    ${CMAKE_CURRENT_BINARY_DIR}/psout.c
    ${CMAKE_CURRENT_BINARY_DIR}/tfmin.c
)

set_source_files_properties(
    ${metapost_core_cweb_sources}
    PROPERTIES GENERATED TRUE
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/mp.c
        ${CMAKE_CURRENT_BINARY_DIR}/mplib.h
        ${CMAKE_CURRENT_BINARY_DIR}/mpmp.h
    COMMAND
        ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mp
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        source/mp.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/mpmath.c
        ${CMAKE_CURRENT_BINARY_DIR}/mpmath.h
    COMMAND
        ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpmath
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        source/mpmath.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/mpmathdecimal.c
        ${CMAKE_CURRENT_BINARY_DIR}/mpmathdecimal.h
    COMMAND
        ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpmathdecimal
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        source/mpmathdecimal.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/mpmathdouble.c
        ${CMAKE_CURRENT_BINARY_DIR}/mpmathdouble.h
    COMMAND
        ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpmathdouble
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        source/mpmathdouble.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/mpstrings.c
        ${CMAKE_CURRENT_BINARY_DIR}/mpstrings.h
    COMMAND
        ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpstrings
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        source/mpstrings.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/psout.c
        ${CMAKE_CURRENT_BINARY_DIR}/mppsout.h
        ${CMAKE_CURRENT_BINARY_DIR}/mplibps.h
    COMMAND
        ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/psout
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        source/psout.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/tfmin.c
    COMMAND
        ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/tfmin
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        source/tfmin.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

set(metapost_core_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-metapost-version.h
    ${metapost_core_cweb_sources}
)

add_library(${metapost_core_lib_name} STATIC ${metapost_core_sources})

set_property(TARGET ${metapost_core_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${metapost_core_lib_name}
    PUBLIC
        ${CMAKE_BINARY_DIR}/${MIKTEX_REL_MP_DIR}
)

target_link_libraries(${metapost_core_lib_name}
    PRIVATE
        ${core_dll_name}
        ${kpsemu_dll_name}
        ${metapost_backends_lib_name}
        ${metapost_util_lib_name}
        ${w2cemu_dll_name}
)

if(USE_SYSTEM_PNG)
    target_link_libraries(${metapost_core_lib_name} PRIVATE MiKTeX::Imported::PNG)
else()
    target_link_libraries(${metapost_core_lib_name} PRIVATE ${png_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${metapost_core_lib_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${metapost_core_lib_name} PRIVATE ${zlib_dll_name})
endif()

install(TARGETS ${metapost_core_lib_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

###############################################################################
## metapost-extramath
###############################################################################

set(metapost_extramath_cweb_sources
    ${CMAKE_CURRENT_BINARY_DIR}/mplib.h
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathbinary.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathbinary.h
)

set_source_files_properties(
    ${metapost_extramath_cweb_sources}
    PROPERTIES GENERATED TRUE
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/mpmathbinary.c
        ${CMAKE_CURRENT_BINARY_DIR}/mpmathbinary.h
    COMMAND
        ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpmathbinary
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        source/mpmathbinary.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

set(metapost_extramath_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-metapost-version.h
    ${metapost_extramath_cweb_sources}
)

add_library(${metapost_extramath_lib_name} STATIC ${metapost_extramath_sources})

set_property(TARGET ${metapost_extramath_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${metapost_extramath_lib_name}
    PUBLIC
        ${CMAKE_BINARY_DIR}/${MIKTEX_REL_MP_DIR}
)

target_link_libraries(${metapost_extramath_lib_name}
    PRIVATE
        ${core_dll_name}
        ${kpsemu_dll_name}
        ${w2cemu_dll_name}
)

if(USE_SYSTEM_MPFR)
    target_link_libraries(${metapost_extramath_lib_name} PRIVATE MiKTeX::Imported::MPFR)
else()
    target_link_libraries(${metapost_extramath_lib_name} PRIVATE ${mpfr_lib_name})
endif()

install(TARGETS ${metapost_extramath_lib_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

###############################################################################
## metapost-util
###############################################################################

set(metapost_util_sources
    source/avl.c
    source/avl.h
    source/decContext.c
    source/decContext.h
    source/decNumber.c
    source/decNumber.h
    source/decNumberLocal.h
)

list(APPEND metapost_util_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-metapost-version.h
)

add_library(${metapost_util_lib_name} STATIC ${metapost_util_sources})

set_property(TARGET ${metapost_util_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${metapost_util_lib_name}
    PUBLIC
        ${CMAKE_BINARY_DIR}/${MIKTEX_REL_MP_DIR}
)

target_link_libraries(${metapost_util_lib_name}
    PRIVATE
        ${core_dll_name}
        ${kpsemu_dll_name}
        ${w2cemu_dll_name}
)

install(TARGETS ${metapost_util_lib_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

###############################################################################
## metapost
###############################################################################

set(metapost_sources
    ${metapost_backends_sources}
    ${metapost_core_sources}
    ${metapost_extramath_sources}
    ${metapost_util_sources}
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-metapost.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/windows/miktex-metapost.rc
    )
    list(APPEND metapost_sources ${CMAKE_CURRENT_BINARY_DIR}/windows/miktex-metapost.rc)
endif()

add_library(${metapost_dll_name} SHARED ${metapost_sources})

set_property(TARGET ${metapost_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_property(TARGET ${metapost_dll_name} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

target_include_directories(${metapost_dll_name}
    PUBLIC
        ${CMAKE_BINARY_DIR}/${MIKTEX_REL_MP_DIR}
)

if(USE_SYSTEM_CAIRO)
    target_link_libraries(${metapost_dll_name} PRIVATE MiKTeX::Imported::CAIRO)
else()
    target_link_libraries(${metapost_dll_name} PRIVATE ${cairo_dll_name})
endif()

if(USE_SYSTEM_GMP)
    target_link_libraries(${metapost_dll_name} PRIVATE MiKTeX::Imported::GMP)
else()
    target_link_libraries(${metapost_dll_name} PRIVATE ${gmp_lib_name})
endif()

if(USE_SYSTEM_PNG)
    target_link_libraries(${metapost_dll_name} PRIVATE MiKTeX::Imported::PNG)
else()
    target_link_libraries(${metapost_dll_name} PRIVATE ${png_dll_name})
endif()

if(USE_SYSTEM_MPFI)
    target_link_libraries(${metapost_dll_name} PRIVATE MiKTeX::Imported::MPFI)
else()
    target_link_libraries(${metapost_dll_name} PRIVATE ${mpfi_lib_name})
endif()

if(USE_SYSTEM_MPFR)
    target_link_libraries(${metapost_dll_name} PRIVATE MiKTeX::Imported::MPFR)
else()
    target_link_libraries(${metapost_dll_name} PRIVATE ${mpfr_lib_name})
endif()

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${metapost_dll_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${metapost_dll_name} PRIVATE ${zlib_dll_name})
endif()

target_link_libraries(${metapost_dll_name}
    PRIVATE
        ${core_dll_name}
        ${kpsemu_dll_name}
        ${w2cemu_dll_name}
)

install(TARGETS ${metapost_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

###############################################################################
## mpost
###############################################################################

set(mpost_cweb_sources
    ${CMAKE_CURRENT_BINARY_DIR}/mpost.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpxout.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpxout.h
)

set_source_files_properties(
    ${mpost_cweb_sources}
    PROPERTIES GENERATED TRUE
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/mpost.c
    COMMAND
        ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpost
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        source/mpost.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/mpxout.c
        ${CMAKE_CURRENT_BINARY_DIR}/mpxout.h
    COMMAND
        ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpxout
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        source/mpxout.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

set(mpost_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-metapost-version.h
    ${MIKTEX_LIBRARY_WRAPPER}
    ${mpost_cweb_sources}
    miktex/mpost.cpp
    miktex/mpost.h
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-mpost.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/windows/miktex-mpost.rc
    )
    list(APPEND mpost_sources
        ${CMAKE_CURRENT_BINARY_DIR}/windows/miktex-mpost.rc
        ${MIKTEX_COMMON_MANIFEST}
    )
endif()

add_executable(${MIKTEX_PREFIX}mpost ${mpost_sources})

set_property(TARGET ${MIKTEX_PREFIX}mpost PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}mpost
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${metapost_dll_name}
    ${w2cemu_dll_name}
)

if(USE_SYSTEM_FMT)
    target_link_libraries(${MIKTEX_PREFIX}mpost MiKTeX::Imported::FMT)
else()
    target_link_libraries(${MIKTEX_PREFIX}mpost ${fmt_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}mpost
        ${getopt_dll_name}
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}mpost DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}")
