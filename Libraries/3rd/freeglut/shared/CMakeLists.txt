## CMakeLists.txt
##
## Copyright (C) 2018-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${freeglut_dll_name}_sources ${freeglut_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-freeglut.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-freeglut.rc
    )
    list(APPEND ${freeglut_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-freeglut.rc
    )
endif()

add_library(${freeglut_dll_name} SHARED ${${freeglut_dll_name}_sources})

set_property(TARGET ${freeglut_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_property(TARGET ${freeglut_dll_name} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

set_shared_library_version_properties(
    ${freeglut_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${freeglut_dll_name} PUBLIC ${public_include_directories})

if(USE_SYSTEM_OPENGL AND MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${freeglut_dll_name} PUBLIC opengl32)
else()
    target_link_libraries(${freeglut_dll_name} PUBLIC MiKTeX::Imported::OPENGL)
endif()

target_link_libraries(${freeglut_dll_name}
    PUBLIC
        ${core_dll_name}
        ${util_dll_name}
)
      
if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${freeglut_dll_name}
        PUBLIC
        winmm
    )
endif()

install(TARGETS ${freeglut_dll_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

source_group(MiKTeX FILES ${miktex_sources})
