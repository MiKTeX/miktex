## CMakeLists.txt						-*- CMake -*-
##
## Copyright (C) 2007-2016 Christian Schenk
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

set(sandbox "${CMAKE_CURRENT_BINARY_DIR}/sandbox")
set(installroot "${sandbox}/texmf")
set(dataroot "${sandbox}/localtexmf")

make_directory(${installroot}/miktex/config)
make_directory(${installroot}/tex/latex/base)
make_directory(${installroot}/tex/test/base)
make_directory(${installroot}/ab/cd/ef/xxx)
make_directory(${installroot}/jk/lm/no/xxx)
make_directory(${dataroot}/miktex/log)

set(test_sources
  ${CMAKE_SOURCE_DIR}/Libraries/MiKTeX/Core/include/miktex/Core/Test.h
)

configure_file(
  log4cxx.xml.in
  ${installroot}/miktex/config/log4cxx.xml
)

file(WRITE ${installroot}/ab/cd/ef/xxx/xyz.txt ".")
file(WRITE ${installroot}/jk/lm/no/xxx/xyz.txt ".")
file(WRITE ${installroot}/tex/latex/base/test.cls ".")
file(WRITE ${installroot}/tex/test/base/test.tex ".")

add_subdirectory(strings)
add_subdirectory(cfg)
add_subdirectory(pathname)
add_subdirectory(compression)
add_subdirectory(thread)
add_subdirectory(tempdir)
add_subdirectory(expansion)
add_subdirectory(fndb)
add_subdirectory(filesystem)
add_subdirectory(file)
