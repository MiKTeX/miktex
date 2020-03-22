## gen-git-info.cmake: 
##
## Copyright (C) 2020 Christian Schenk
## 
## This file is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
## 
## This file is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this file; if not, write to the Free Software Foundation,
## 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

if("$Format:%%$" STREQUAL "%%")
  set(MIKTEX_HAVE_GIT_INFO 1)
  set(MIKTEX_GIT_COMMIT "$Format:%H$")
  set(MIKTEX_GIT_COMMIT_ABBREV "$Format:%h$")
  set(MIKTEX_GIT_AUTHOR_DATE $Format:%at$)
elseif(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/.git")
  if(GIT_FOUND)
    set(MIKTEX_HAVE_GIT_INFO 1)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} show --no-patch --pretty=%H
      WORKING_DIRECTORY "${SOURCE_DIR}"
      OUTPUT_VARIABLE MIKTEX_GIT_COMMIT
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
      COMMAND ${GIT_EXECUTABLE} show --no-patch --pretty=%h
      WORKING_DIRECTORY "${SOURCE_DIR}"
      OUTPUT_VARIABLE MIKTEX_GIT_COMMIT_ABBREV
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
      COMMAND ${GIT_EXECUTABLE} show --no-patch --pretty=%at
      WORKING_DIRECTORY "${SOURCE_DIR}"
      OUTPUT_VARIABLE MIKTEX_GIT_AUTHOR_DATE
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  else()
    set(MIKTEX_HAVE_GIT_INFO 0)
  endif()
else()
  set(MIKTEX_HAVE_GIT_INFO 0)
endif()

configure_file(
  ${GITINFO_H_IN}
  ${GITINFO_H}
)
