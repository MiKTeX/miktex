## RuntimePaths.cmake
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

if(MIKTEX_WORDS_BIGENDIAN)
    set(MIKTEX_REL_MIKTEX_BASE_DIR "miktex/data/be")
    set(MIKTEX_REL_MIKTEX_FMT_DIR "miktex/data/be")
    set(MIKTEX_REL_MIKTEX_FNDB_DIR "miktex/data/be")
else()
    set(MIKTEX_REL_MIKTEX_BASE_DIR "miktex/data/le")
    set(MIKTEX_REL_MIKTEX_FMT_DIR "miktex/data/le")
    set(MIKTEX_REL_MIKTEX_FNDB_DIR "miktex/data/le")
endif()

if(MIKTEX_UNIX_ALIKE)
    set(MIKTEX_SYSTEM_ETC_FONTS_CONFD_DIR "/etc/fonts/conf.d" CACHE PATH "System-wide directory for font configuration data.")
    set(MIKTEX_SYSTEM_LINK_TARGET_DIR "/usr/local/bin" CACHE PATH "System-wide directory in which to create symbolic links to MiKTeX binaries.")
    set(MIKTEX_SYSTEM_VAR_CACHE_DIR "/var/cache" CACHE PATH "Directory for application cache data.")
    set(MIKTEX_SYSTEM_VAR_LIB_DIR "/var/lib" CACHE PATH "Directory for state information.")
    set(MIKTEX_SYSTEM_VAR_LOG_DIR "/var/log" CACHE PATH "Directory for log files.")
    set(MIKTEX_USER_LINK_TARGET_DIR "~/bin" CACHE STRING "Per-user directory in which to create symbolic links to MiKTeX binaries.")
endif()
