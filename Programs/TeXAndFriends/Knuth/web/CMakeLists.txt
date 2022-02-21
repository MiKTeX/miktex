## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_KNUTH_FOLDER}/web")

if(LINK_EVERYTHING_STATICALLY)
    use_static_crt()
endif()

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/inittangle.cc
        ${CMAKE_CURRENT_BINARY_DIR}/inittangle.h
        ${CMAKE_CURRENT_BINARY_DIR}/inittangledefs.h
    COMMAND
        c4p
            --def-filename=inittangledefs.h
            --dll
            --include-filename=init/miktex-inittangle.h
            --header-file=inittangle.h
            --one=inittangle
            --using-namespace=MiKTeX::TeXAndFriends
            -C
            --class=InitTangleProgram
            ${CMAKE_CURRENT_SOURCE_DIR}/init/inittangle.p
    MAIN_DEPENDENCY
    ${CMAKE_CURRENT_SOURCE_DIR}/init/inittangle.p
    DEPENDS
        c4p
    VERBATIM
)

configure_file(
    ${MIKTEX_ALIAS_WRAPPER}
    ${CMAKE_CURRENT_BINARY_DIR}/inittanglewrapper.cpp
    COPYONLY
)

set(inittangle_sources
    ${CMAKE_CURRENT_BINARY_DIR}/inittangle.cc
    ${CMAKE_CURRENT_BINARY_DIR}/inittangle.h
    ${CMAKE_CURRENT_BINARY_DIR}/inittangledefs.h
    ${CMAKE_CURRENT_BINARY_DIR}/inittanglewrapper.cpp
    init/miktex-inittangle.cpp
    init/miktex-inittangle.h
    miktex-tangle-version.h
    miktex-tangle.h
)

add_executable(inittangle ${inittangle_sources})

if(MIKTEX_NATIVE_WINDOWS)
    set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/inittanglewrapper.cpp
        PROPERTIES COMPILE_FLAGS
        "-DFUNC=MiKTeX_INITTANGLE -D_UNICODE"
    )
else()
    set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/inittanglewrapper.cpp
        PROPERTIES COMPILE_FLAGS
        "-DFUNC=MiKTeX_INITTANGLE"
    )
endif()

target_link_libraries(inittangle
    PUBLIC
        ${app_dll_name}
        ${core_dll_name}
        ${texmf_dll_name}
        miktex-popt-wrapper
)

set_property(TARGET inittangle PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/tangle.p
        ${CMAKE_CURRENT_BINARY_DIR}/tangle.pool
    COMMAND
        inittangle
            ${CMAKE_CURRENT_SOURCE_DIR}/source/tangle.web
            ${CMAKE_CURRENT_SOURCE_DIR}/miktex-tangle.ch
            ${CMAKE_CURRENT_BINARY_DIR}/tangle.p
            ${CMAKE_CURRENT_BINARY_DIR}/tangle.pool
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_SOURCE_DIR}/source/tangle.web
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/miktex-tangle.ch
        inittangle
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/tangle.cc
        ${CMAKE_CURRENT_BINARY_DIR}/tangle.h
        ${CMAKE_CURRENT_BINARY_DIR}/tangledefs.h
    COMMAND
        c4p
            --def-filename=tangledefs.h
            --dll
            --include-filename=miktex-tangle.h
            --header-file=tangle.h
            --one=tangle
            --using-namespace=MiKTeX::TeXAndFriends
            -C
            --class=TangleProgram
            ${CMAKE_CURRENT_BINARY_DIR}/tangle.p
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/tangle.p
    DEPENDS
        c4p
    VERBATIM
)

configure_file(
    ${MIKTEX_ALIAS_WRAPPER}
    ${CMAKE_CURRENT_BINARY_DIR}/tanglewrapper.cpp
    COPYONLY
)

set(tangle_sources
    ${CMAKE_CURRENT_BINARY_DIR}/tangle.cc
    ${CMAKE_CURRENT_BINARY_DIR}/tangle.h
    ${CMAKE_CURRENT_BINARY_DIR}/tangledefs.h
    ${CMAKE_CURRENT_BINARY_DIR}/tanglewrapper.cpp
    miktex-tangle-version.h
    miktex-tangle.cpp
    miktex-tangle.h
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/miktex-tangle.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-tangle.rc
    )
    list(APPEND tangle_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-tangle.rc
        ${MIKTEX_COMMON_MANIFEST}
    )
endif()

add_executable(${MIKTEX_PREFIX}tangle ${tangle_sources})

if(MIKTEX_NATIVE_WINDOWS)
    set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/tanglewrapper.cpp
        PROPERTIES COMPILE_FLAGS
        "-DFUNC=MiKTeX_TANGLE -D_UNICODE"
    )
else()
    set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/tanglewrapper.cpp
        PROPERTIES COMPILE_FLAGS
        "-DFUNC=MiKTeX_TANGLE"
    )
endif()

target_link_libraries(${MIKTEX_PREFIX}tangle
    PUBLIC
        ${app_dll_name}
        ${core_dll_name}
        ${texmf_dll_name}
        miktex-popt-wrapper
)

install(
    TARGETS
        ${MIKTEX_PREFIX}tangle
    DESTINATION
        "${MIKTEX_BINARY_DESTINATION_DIR}"
)

set_property(TARGET ${MIKTEX_PREFIX}tangle PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

create_web_app(Weave)
