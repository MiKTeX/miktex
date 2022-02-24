## UseStaticCRT.cmake
##
## Copyright (C) 2006-202 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

###############################################################################
# add_cxx_flag
# 
# Add the specified C++ compiler flag.
###############################################################################

macro(add_cxx_flag _flag)
    foreach(c "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
        set(CMAKE_CXX_FLAGS${c} "${CMAKE_CXX_FLAGS${c}} ${_flag}")
    endforeach()
endmacro()

###############################################################################
# add_c_flag
# 
# Add the specified C compiler flag.
###############################################################################

macro(add_c_flag _flag)
    foreach(c "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
        set(CMAKE_C_FLAGS${c} "${CMAKE_C_FLAGS${c}} ${_flag}")
    endforeach()
endmacro()

###############################################################################
# remove_cxx_flag
# 
# Remove the specified C++ compiler flag (a regular expression).
###############################################################################

macro(remove_cxx_flag _flag_regex)
    foreach(c "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
        string(
            REGEX REPLACE
                "${_flag_regex}( |$)" ""
                CMAKE_CXX_FLAGS${c}
                "${CMAKE_CXX_FLAGS${c}}"
        )
    endforeach()
endmacro()

###############################################################################
# remove_c_flag
# 
# Remove the specified C compiler flag (a regular expression).
###############################################################################

macro(remove_c_flag _flag_regex)
    foreach(c "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
        string(
            REGEX REPLACE
                "${_flag_regex}( |$)" ""
                CMAKE_C_FLAGS${c}
                "${CMAKE_C_FLAGS${c}}"
        )
    endforeach()
endmacro()

###############################################################################
# use_static_crt
# 
# Prepare to link against the static C runtime library.
###############################################################################

macro(use_static_crt)
    if(MSVC)
        foreach(c "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
            string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS${c} "${CMAKE_C_FLAGS${c}}")
            string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS${c} "${CMAKE_CXX_FLAGS${c}}")
        endforeach()
    endif()
    remove_definitions(
        -D_AFXDLL
        -D_ATL_DLL
    )
endmacro()
