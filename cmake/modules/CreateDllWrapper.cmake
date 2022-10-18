## CreateDllWrapper.cmake
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

macro(create_dll_wrapper _name _libname)
    if(${ARGC} GREATER 2)
        set(_funcname ${ARGV2})
    else()
        string(TOLOWER "${_name}" _funcname)
    endif()
    configure_file(
        ${MIKTEX_ALIAS_WRAPPER}
        ${CMAKE_CURRENT_BINARY_DIR}/${_name}wrapper.cpp
        COPYONLY
    )
    set(_sources ${CMAKE_CURRENT_BINARY_DIR}/${_name}wrapper.cpp)
    if(MIKTEX_NATIVE_WINDOWS)
        list(APPEND _sources
        ${MIKTEX_COMMON_MANIFEST}
        )
    endif()
    add_executable(${_name} ${_sources})
    if(MIKTEX_CURRENT_FOLDER)
        set_property(TARGET ${_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
    endif()
    if(MIKTEX_NATIVE_WINDOWS)
        set_source_files_properties(
            ${CMAKE_CURRENT_BINARY_DIR}/${_name}wrapper.cpp
            COMPILE_FLAGS "-DFUNC=${_funcname} -D_UNICODE"
        )
    else()
        set_source_files_properties(
            ${CMAKE_CURRENT_BINARY_DIR}/${_name}wrapper.cpp
            COMPILE_FLAGS "-DFUNC=${_funcname}"
        )
    endif()
    target_link_libraries(${_name} ${core_dll_name} ${_libname})
    install(TARGETS ${_name} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
endmacro()
