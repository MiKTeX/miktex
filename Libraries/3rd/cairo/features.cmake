## features.cmake
##
## Copyright (C) 2013-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(sources.cmake)

if(CAIRO_HAS_DWRITE_FONT)
    list(APPEND feature_headers ${cairo_feature_headers_cairo_dwrite_font})
    list(APPEND feature_sources ${cairo_feature_sources_cairo_dwrite_font})
endif()

if(CAIRO_HAS_FT_FONT)
    list(APPEND feature_headers ${cairo_feature_headers_cairo_ft_font})
    list(APPEND feature_sources ${cairo_feature_sources_cairo_ft_font})
endif()

if(CAIRO_HAS_PNG_FUNCTIONS)
    list(APPEND feature_headers ${cairo_feature_headers_cairo_png})
    list(APPEND feature_sources ${cairo_feature_sources_cairo_png})
endif()

if(CAIRO_HAS_PDF_SURFACE)
    list(APPEND feature_headers ${cairo_feature_headers_cairo_pdf})
    list(APPEND feature_sources ${cairo_feature_sources_cairo_pdf})
endif()

if(CAIRO_HAS_PS_SURFACE)
    list(APPEND feature_headers ${cairo_feature_headers_cairo_ps})
    list(APPEND feature_sources ${cairo_feature_sources_cairo_ps})
endif()

if(CAIRO_HAS_SCRIPT_SURFACE)
    list(APPEND feature_headers ${cairo_feature_headers_cairo_script})
    list(APPEND feature_sources ${cairo_feature_sources_cairo_script})
endif()

if(CAIRO_HAS_SVG_SURFACE)
    list(APPEND feature_headers ${cairo_feature_headers_cairo_svg})
    list(APPEND feature_sources ${cairo_feature_sources_cairo_svg})
endif()

if(CAIRO_HAS_TEE_SURFACE)
    list(APPEND feature_headers ${cairo_feature_headers_cairo_tee})
    list(APPEND feature_sources ${cairo_feature_sources_cairo_tee})
endif()

if(CAIRO_HAS_WIN32_FONT)
    list(APPEND feature_headers ${cairo_feature_headers_cairo_win32_font})
    list(APPEND feature_sources ${cairo_feature_sources_cairo_win32_font})
endif()

if(CAIRO_HAS_WIN32_SURFACE)
    list(APPEND feature_headers ${cairo_feature_headers_cairo_win32})
    list(APPEND feature_sources ${cairo_feature_sources_cairo_win32})
endif()
