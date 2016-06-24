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

add_executable(mpfr-test test.c)

set_property(TARGET mpfr-test PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(mpfr-test ${mpfr_lib_name})

add_test(NAME mpfr_test COMMAND $<TARGET_FILE:mpfr-test>)
