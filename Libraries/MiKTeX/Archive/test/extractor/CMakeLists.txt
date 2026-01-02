## CMakeLists.txt
##
## Copyright (C) 2026 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

add_executable(archive_extractor_test1
    ${CMAKE_CURRENT_SOURCE_DIR}/1.cpp
    ${test_sources}
)

set_property(TARGET archive_extractor_test1 PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_LOG4CXX)
    target_link_libraries(archive_extractor_test1 MiKTeX::Imported::LOG4CXX)
else()
    target_link_libraries(archive_extractor_test1 ${log4cxx_dll_name})
endif()

target_link_libraries(archive_extractor_test1
    ${archive_dll_name}
    ${core_dll_name}
    miktex-popt-wrapper
)

add_test(
    NAME archive_extractor_test1
    COMMAND $<TARGET_FILE:archive_extractor_test1>
)
