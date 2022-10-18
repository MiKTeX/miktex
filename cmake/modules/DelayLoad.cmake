## DelayLoad.cmake
##
## Copyright (C) 2007-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

macro(delay_load _target)
    if(MIKTEX_NATIVE_WINDOWS AND MSVC AND MIKTEX_ENABLE_DELAY_LOAD)
        foreach(_dll_name ${ARGN})
            add_link_flags(${_target} "/DELAYLOAD:${_dll_name}.dll")
        endforeach()
        target_link_libraries(${_target} PRIVATE delayimp)
    endif()
endmacro()
