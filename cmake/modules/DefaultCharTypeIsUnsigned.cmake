## DefaultCharTypeIsUnsigned.cmake
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

macro(default_char_type_is_unsigned)
    if(MSVC)
        foreach(c "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
            set(CMAKE_C_FLAGS${c} "${CMAKE_C_FLAGS${c}} /J")
            set(CMAKE_CXX_FLAGS${c} "${CMAKE_CXX_FLAGS${c}} /J")
        endforeach()
    else()
        foreach(c "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
            set(CMAKE_C_FLAGS${c} "${CMAKE_C_FLAGS${c}} -funsigned-char")
            set(CMAKE_CXX_FLAGS${c} "${CMAKE_CXX_FLAGS${c}} -funsigned-char")
        endforeach()
    endif()
endmacro()
