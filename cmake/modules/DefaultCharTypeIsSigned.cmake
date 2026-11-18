## DefaultCharTypeIsSigned.cmake
##
## Copyright (C) 2026 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

macro(default_char_type_is_signed)
    if(NOT MSVC)
        foreach(c "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
            set(CMAKE_C_FLAGS${c} "${CMAKE_C_FLAGS${c}} -fsigned-char")
            set(CMAKE_CXX_FLAGS${c} "${CMAKE_CXX_FLAGS${c}} -fsigned-char")
        endforeach()
    endif()
endmacro()
