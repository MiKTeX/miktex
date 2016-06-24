## CMakeLists.txt					-*- CMake -*-
##
## Copyright (C) 2015-2016 Christian Schenk
## 
## This file is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published
## by the Free Software Foundation; either version 2, or (at your
## option) any later version.
## 
## This file is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this file; if not, write to the Free Software
## Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
## USA.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_CURRENT_FOLDER}/test")

add_executable(gmp-test test.c)

set_property(TARGET gmp-test PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(gmp-test ${gmp_lib_name})

add_test(NAME gmp_test COMMAND $<TARGET_FILE:gmp-test>)

set(HAVE_GMP_TESTS 0)

if(HAVE_GMP_TESTS)
  add_definitions(-DRETSIGTYPE=void)

  include_directories(${fqsrcdir}/tests)
  
  set(gmp_tests_lib_sources
    ${fqsrcdir}/tests/memory.c
    ${fqsrcdir}/tests/misc.c
    ${fqsrcdir}/tests/refmpf.c
    ${fqsrcdir}/tests/refmpn.c
    ${fqsrcdir}/tests/refmpq.c
    ${fqsrcdir}/tests/refmpz.c
    ${fqsrcdir}/tests/spinner.c
    ${fqsrcdir}/tests/tests.h
    ${fqsrcdir}/tests/trace.c
  )

  add_library(gmp-tests-static STATIC ${gmp_tests_lib_sources})

  set_property(TARGET gmp-tests-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

  target_link_libraries(gmp-tests-static
    PUBLIC
      ${gmp_lib_name}
  )

  set(gmp_tests
    t-addsub
  )

  foreach(t ${gmp_tests})
    add_executable(gmp-${t} ${fqsrcdir}/tests/mpz/${t}.c)
    set_property(TARGET gmp-${t} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
    target_link_libraries(gmp-${t} gmp-tests-static)
  endforeach()
endif()
