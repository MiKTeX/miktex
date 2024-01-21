## CMakeLists.txt
##
## Copyright (C) 2015-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(tests 1)

foreach(t ${tests})
    add_executable(core_filesystem_test${t} ${t}.cpp ${test_sources})
    set_property(TARGET core_filesystem_test${t} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
    if(USE_SYSTEM_LOG4CXX)
        target_link_libraries(core_filesystem_test${t} MiKTeX::Imported::LOG4CXX)
    else()
        target_link_libraries(core_filesystem_test${t} ${log4cxx_dll_name})
    endif()
    target_link_libraries(core_filesystem_test${t}
        ${core_dll_name}
        miktex-popt-wrapper
    )
    add_test(
        NAME core_filesystem_test${t}
        COMMAND $<TARGET_FILE:core_filesystem_test${t}>
    )
endforeach(t)
