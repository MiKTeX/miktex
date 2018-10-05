## miktex.mak:
## Copyright (C) 1996-2016 Christian Schenk
## 
## This file is part of Yap.
## 
## Yap is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
## 
## Yap is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
## License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with Yap; if not, write to the Free Software Foundation, 59
## Temple Place - Suite 330, Boston, MA 02111-1307, USA.

!include <miktex.inc>

.SUFFIXES: .tex .dvi

.tex.dvi:
	latex --src $<

all: test001.dvi

check:

install:

uninstall:

mostlyclean:
	-del /f *.aux
	-del /f *.log

clean: mostlyclean

distclean: clean

maintainer-clean: distclean

depend:
