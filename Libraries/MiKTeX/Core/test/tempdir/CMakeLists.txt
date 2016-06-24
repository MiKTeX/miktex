## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2009-2016 Christian Schenk
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

add_executable(core_tempdir_test1 1.cpp ${test_sources})

set_property(TARGET core_tempdir_test1 PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(core_tempdir_test1 asInvoker)
merge_compatibility_manifest(core_tempdir_test1)

target_link_libraries(core_tempdir_test1
  ${core_dll_name}
  ${log4cxx_dll_name}
  miktex-popt-wrapper
)

add_test(
  NAME core_tempdir_test1
  COMMAND $<TARGET_FILE:core_tempdir_test1>
    --data-root=${dataroot}
    --install-root=${installroot}
)
