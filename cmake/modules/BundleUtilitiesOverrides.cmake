## BundleUtilitiesOverrides.cmake
##
## Copyright (C) 2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

function(gp_item_default_embedded_path_override item default_embedded_path_var)
    set(path "${${default_embedded_path_var}}")
    if(item MATCHES "\\.dylib$")
        set(path "@executable_path/../miktex-lib")
    endif()
    set(${default_embedded_path_var} "${path}" PARENT_SCOPE)
endfunction()
