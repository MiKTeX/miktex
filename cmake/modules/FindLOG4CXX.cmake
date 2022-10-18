## FindLOG4CXX.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_path(LOG4CXX_INCLUDE_DIR
    NAMES
      log4cxx/log4cxx.h
)

find_library(LOG4CXX_LIBRARY
    NAMES
      log4cxx
)

find_package_handle_standard_args(LOG4CXX DEFAULT_MSG LOG4CXX_LIBRARY LOG4CXX_INCLUDE_DIR)

if(LOG4CXX_FOUND)
    set(LOG4CXX_INCLUDE_DIRS ${LOG4CXX_INCLUDE_DIR})
    set(LOG4CXX_LIBRARIES ${LOG4CXX_LIBRARY})
else()
    set(LOG4CXX_INCLUDE_DIRS)
    set(LOG4CXX_LIBRARIES)
endif()

mark_as_advanced(LOG4CXX_LIBRARY LOG4CXX_INCLUDE_DIR)
