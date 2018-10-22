## version.cmake:
##
## Copyright (C) 2018 Christian Schenk
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

set(MIKTEX_COMP_J2000_VERSION 6568)

set(FRIBIDI_MAJOR_VERSION 1)
set(FRIBIDI_MINOR_VERSION 0)
set(FRIBIDI_MICRO_VERSION 5)
set(FRIBIDI_INTERFACE_VERSION 4)

set(FRIBIDI_VERSION "${FRIBIDI_MAJOR_VERSION}.${FRIBIDI_MINOR_VERSION}.${FRIBIDI_MICRO_VERSION}")

set(MIKTEX_COMP_ORIG_VERSION_STR "${FRIBIDI_VERSION}")

set(MIKTEX_COMP_INTERFACE_VERSION ${FRIBIDI_INTERFACE_VERSION})
