## SetSharedLibraryVersion.cmake
##
## Copyright (C) 2017-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

macro(set_shared_library_version_properties _target _version _soversion)
    set_target_properties(${_target}
        PROPERTIES
        VERSION "${_version}"
        SOVERSION "${_soversion}"
)
endmacro()

macro(set_shared_library_version _target _soversion _j2000)
    math(EXPR b "${_j2000} / 256")
    math(EXPR c "${_j2000} % 256")
    set_target_properties(${_target}
        PROPERTIES
        VERSION "${_soversion}.${b}.${c}"
        SOVERSION "${_soversion}"
    )
endmacro()
