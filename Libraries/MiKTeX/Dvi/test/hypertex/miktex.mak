## miktex.mak: MiKTeX DVI Library test suite
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

check: test1

test1:
	latex -c-style-errors test001
	latex -c-style-errors test001
	dviscan --logfilter=hypertex test001.dvi
	del test001.aux
	del test001.dvi
	del test001.log
