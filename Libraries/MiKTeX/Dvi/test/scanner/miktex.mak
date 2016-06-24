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

.cpp{$(outdir)\}.obj:
	$(cc) $(cppstandard) \
		$(ccopt_output_directory)$(outdir)\ \
		-DTRACEFLAGS=\"config,error\" \
		-DBINDIR=\"$(bindir:\=/)\" \
		$<

{$(outdir)\}.obj{$(outdir)\}.exe:
	$(link) $(lstandard) \
		-out:$@ \
		$< \
		$(miktex_lib) \
		$(dvi_lib) \
		$(popt_lib)
	mt -nologo -manifest $@.manifest -outputresource:$@;1

all: $(outdir) $(outdir)\1.exe

install: 

check: \
			common-check $(outdir) \
			$(outdir)\1.exe \
			$(outdir)\2.exe \
			test.dvi \

	 $(outdir)\1.exe
	 $(outdir)\2.exe

test.dvi:
	$(weave) $(miktexsrcdir)\$(rel_texdir)\tex.web nul test.tex
	$(tex) -src-specials test.tex
	del test.tex

# _____________________________________________________________________________
#
# clean-up
# _____________________________________________________________________________

mostlyclean: common-mostlyclean

clean: common-clean mostlyclean

distclean: common-distclean clean

maintainer-clean: common-maintainer-clean distclean

# _____________________________________________________________________________
#
# dependencies
# _____________________________________________________________________________

!include <common-dependencies.inc>

depend: 1.cpp 2.cpp
	$(MAKEDEPEND) $**
	$(fixdep)

# DO NOT DELETE

$(outdir)\1.obj: $(miktexsrcdir)/Libraries/MiKTeX/Core/include/miktex/test.h
$(outdir)\1.obj: $(miktexsrcdir)/Libraries/MiKTeX/Core/include/miktex/char.h
$(outdir)\1.obj: $(miktexsrcdir)/Libraries/MiKTeX/Core/include/miktex/core.h
$(outdir)\1.obj: $(miktexsrcdir)/Libraries/3rd/libmd5/include/miktex/md5.h
$(outdir)\1.obj: $(miktexsrcdir)/Libraries/MiKTeX/Core/include/miktex/paths.h
$(outdir)\1.obj: $(miktexsrcdir)/Libraries/3rd/libpopt/popt-miktex.h
$(outdir)\1.obj: $(miktexsrcdir)/Libraries/3rd/libpopt/popt.h
$(outdir)\1.obj: $(miktexsrcdir)/Libraries/MiKTeX/Dvi/include/miktex/dvi.h
$(outdir)\2.obj: $(miktexsrcdir)/Libraries/MiKTeX/Core/include/miktex/test.h
$(outdir)\2.obj: $(miktexsrcdir)/Libraries/MiKTeX/Core/include/miktex/char.h
$(outdir)\2.obj: $(miktexsrcdir)/Libraries/MiKTeX/Core/include/miktex/core.h
$(outdir)\2.obj: $(miktexsrcdir)/Libraries/3rd/libmd5/include/miktex/md5.h
$(outdir)\2.obj: $(miktexsrcdir)/Libraries/MiKTeX/Core/include/miktex/paths.h
$(outdir)\2.obj: $(miktexsrcdir)/Libraries/3rd/libpopt/popt-miktex.h
$(outdir)\2.obj: $(miktexsrcdir)/Libraries/3rd/libpopt/popt.h
$(outdir)\2.obj: $(miktexsrcdir)/Libraries/MiKTeX/Dvi/include/miktex/dvi.h
