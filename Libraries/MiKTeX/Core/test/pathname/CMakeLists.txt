## CMakeLists.txt
##
## Copyright (C) 2006-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

add_executable(core_pathname_test1 1.cpp ${test_sources})

set_property(TARGET core_pathname_test1 PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_LOG4CXX)
    target_link_libraries(core_pathname_test1 MiKTeX::Imported::LOG4CXX)
else()
    target_link_libraries(core_pathname_test1 ${log4cxx_dll_name})
endif()

target_link_libraries(core_pathname_test1
    ${core_dll_name}
    miktex-popt-wrapper
)

add_test(
    NAME core_pathname_test1
    COMMAND $<TARGET_FILE:core_pathname_test1>
)
