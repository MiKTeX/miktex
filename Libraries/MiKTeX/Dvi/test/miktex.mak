## miktex.mak:
##
## Copyright (C) 1996-2016 Christian Schenk
## 
## This file is part of the MiKTeX DVI Library.
## 
## The MiKTeX DVI Library is free software; you can redistribute it
## and/or modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2, or
## (at your option) any later version.
## 
## The MiKTeX DVI Library is distributed in the hope that it will be
## useful, but WITHOUT ANY WARRANTY; without even the implied warranty
## of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with the MiKTeX DVI Library; if not, write to the Free
## Software Foundation, 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

!include <miktex.inc>

subdirs = \
	scanner \
#	src \

check depend:
	for %d in ($(subdirs)) do \
		cd %d & $(MAKE) -f $(miktex_mak) check & cd ..
