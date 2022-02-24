## FindFRIBIDI.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

find_package(PkgConfig QUIET)

pkg_check_modules(FRIBIDI_PC REQUIRED fribidi)

find_path(FRIBIDI_INCLUDE_DIR
    NAMES
        fribidi/fribidi.h
    HINTS
        ${FRIBIDI_PC_INCLUDEDIR}
        ${FRIBIDI_PC_INCLUDE_DIRS}
)

find_library(FRIBIDI_LIBRARY
    NAMES
        fribidi
    HINTS
        ${FRIBIDI_PC_LIBDIR}
        ${FRIBIDI_PC_LIBRARY_DIRS}
)

find_package_handle_standard_args(FRIBIDI DEFAULT_MSG FRIBIDI_LIBRARY FRIBIDI_INCLUDE_DIR)

if(FRIBIDI_FOUND)
    set(FRIBIDI_INCLUDE_DIRS ${FRIBIDI_PC_INCLUDE_DIRS} ${FRIBIDI_PC_INCLUDEDIR})
    foreach(lib ${FRIBIDI_PC_LIBRARIES})
        find_library(${lib}_LIBRARY
            NAMES
                ${lib}
            HINTS
                ${FRIBIDI_PC_LIBRARY_DIRS}
        )
        list(APPEND FRIBIDI_LIBRARIES ${${lib}_LIBRARY})
    endforeach()
    message(INFO "fribidi libs: ${FRIBIDI_LIBRARIES}")
    message(INFO "fribidi incs: ${FRIBIDI_INCLUDE_DIRS}")
else()
    set(FRIBIDI_INCLUDE_DIRS)
    set(FRIBIDI_LIBRARIES)
endif()

mark_as_advanced(FRIBIDI_LIBRARY FRIBIDI_INCLUDE_DIR)
