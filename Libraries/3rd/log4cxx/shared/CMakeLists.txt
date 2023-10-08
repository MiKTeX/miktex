## CMakeLists.txt
##
## Copyright (C) 2015-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${log4cxx_dll_name}_sources ${log4cxx_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-log4cxx.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-log4cxx.rc
    )
    list(APPEND ${log4cxx_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-log4cxx.rc
    )
endif()

add_library(${log4cxx_dll_name} SHARED ${${log4cxx_dll_name}_sources})

set_property(TARGET ${log4cxx_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${log4cxx_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${log4cxx_dll_name}
    PRIVATE
        -DLOG4CXX
)

target_include_directories(${log4cxx_dll_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${log4cxx_dll_name}
    PRIVATE
        ${system_libraries}
)

if(USE_SYSTEM_APR)
    target_link_libraries(${log4cxx_dll_name} PUBLIC MiKTeX::Imported::APR)
else()
    target_link_libraries(${log4cxx_dll_name} PUBLIC ${apr_dll_name})
endif()

if(USE_SYSTEM_APRUTIL)
  target_link_libraries(${log4cxx_dll_name} PUBLIC MiKTeX::Imported::APRUTIL)
else()
  target_link_libraries(${log4cxx_dll_name} PUBLIC ${apr_util_dll_name})
endif()

if(USE_SYSTEM_FMT)
  target_link_libraries(${log4cxx_dll_name} PUBLIC MiKTeX::Imported::FMT)
else()
  target_link_libraries(${log4cxx_dll_name} PUBLIC ${fmt_dll_name})
endif()

install(TARGETS ${log4cxx_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
