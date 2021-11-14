## CMakeLists.txt: build MiKTeX Fontmap Maintenance Utility
##
## Copyright (C) 2006-2021 Christian Schenk
## 
## This file is licensed under the GNU General Public License version 2
## or any later version.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/${MIKTEX_PROG_NAME_MKFNTMAP}")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(mkfntmap_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  internal.h
  mkfntmap-version.h
  mkfntmap.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    mkfntmap.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/mkfntmap.rc
  )
  list(APPEND mkfntmap_sources
    ${CMAKE_CURRENT_BINARY_DIR}/mkfntmap.rc
    ${MIKTEX_COMMON_MANIFEST}
  )
endif()

add_executable(${MIKTEX_PROG_NAME_MKFNTMAP} ${mkfntmap_sources})

set_property(TARGET ${MIKTEX_PROG_NAME_MKFNTMAP} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_FMT)
  target_link_libraries(${MIKTEX_PROG_NAME_MKFNTMAP} MiKTeX::Imported::FMT)
else()
  target_link_libraries(${MIKTEX_PROG_NAME_MKFNTMAP} ${fmt_dll_name})
endif()

if(USE_SYSTEM_LOG4CXX)
  target_link_libraries(${MIKTEX_PROG_NAME_MKFNTMAP} MiKTeX::Imported::LOG4CXX)
else()
  target_link_libraries(${MIKTEX_PROG_NAME_MKFNTMAP} ${log4cxx_dll_name})
endif()

target_link_libraries(${MIKTEX_PROG_NAME_MKFNTMAP}
  ${app_dll_name}
  ${core_dll_name}
  miktex-popt-wrapper
)

install(TARGETS ${MIKTEX_PROG_NAME_MKFNTMAP}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
