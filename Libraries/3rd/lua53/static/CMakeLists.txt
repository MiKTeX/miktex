## CMakeLists.txt
##
## Copyright (C) 2010-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

add_library(${lua53_lib_name} STATIC ${liblua_sources})

set_property(TARGET ${lua53_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${lua53_lib_name}
    PUBLIC
        ${public_compile_definitions}
        -DMIKTEX_LUA_STATIC
)

target_link_libraries(${lua53_lib_name}
    PRIVATE
        ${core_dll_name}
)

target_include_directories(${lua53_lib_name}
    PUBLIC
        ${public_include_directories}
)

if(MIKTEX_UNIX_ALIKE)
    target_link_libraries(${lua53_lib_name}
        PRIVATE
            ${CMAKE_DL_LIBS}
            m
    )
endif()

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${lua53_lib_name}
        PRIVATE
            ${utf8wrap_dll_name}
    )
endif()

if(INSTALL_STATIC_LIBRARIES)
    install(TARGETS ${lua53_lib_name}
        ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
        LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
        RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    )
endif()
