## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/MakeUtilities")

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()  

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(common_sources
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    MakeUtility.h
)

set(exes
    makebase
    makefmt
    makemf
    makepk
    maketfm
)

foreach(t ${exes})
    set(${t}_sources ${t}.cpp ${t}-version.h ${common_sources})
    if(MIKTEX_NATIVE_WINDOWS)
        list(APPEND ${t}_sources
            ${MIKTEX_COMMON_MANIFEST}
            ${t}.rc
        )
    endif()
    string(TOUPPER "${t}" _prog_name_u)
    add_executable(${MIKTEX_PROG_NAME_${_prog_name_u}} ${${t}_sources})
    set_property(TARGET ${MIKTEX_PROG_NAME_${_prog_name_u}} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
    if(USE_SYSTEM_FMT)
        target_link_libraries(${MIKTEX_PROG_NAME_${_prog_name_u}} MiKTeX::Imported::FMT)
    else()
        target_link_libraries(${MIKTEX_PROG_NAME_${_prog_name_u}} ${fmt_dll_name})
    endif()
    if(USE_SYSTEM_LOG4CXX)
        target_link_libraries(${MIKTEX_PROG_NAME_${_prog_name_u}} MiKTeX::Imported::LOG4CXX)
    else()
        target_link_libraries(${MIKTEX_PROG_NAME_${_prog_name_u}} ${log4cxx_dll_name})
    endif()
    target_link_libraries(${MIKTEX_PROG_NAME_${_prog_name_u}}
        ${app_dll_name}
        ${core_dll_name}
        ${texmf_dll_name}
    )
    if(MIKTEX_NATIVE_WINDOWS)
        target_link_libraries(${MIKTEX_PROG_NAME_${_prog_name_u}}
        ${getopt_dll_name}
        )
    endif()
    install(TARGETS ${MIKTEX_PROG_NAME_${_prog_name_u}}
        RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
        LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
        ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    )
endforeach()
