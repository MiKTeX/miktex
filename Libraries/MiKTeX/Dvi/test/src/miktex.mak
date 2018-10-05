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

cc_options = $(cflags) $(cdebug) $(cvarsdll) $(cflagseh) \
		$(incdirs) $(miktexvars)

.cpp{$(outdir)\}.obj:
	$(cc) $(cc_options) $(cflagseh) $(ccopt_output_directory)$(outdir)\ $<

{$(outdir)\}.obj{$(outdir)\}.exe:
	$(link) $(conlflags) $(ldebug) -out:$@ $< \
		$(conlibsdll) $(miktex_lib) $(popt_lib) $(dvi_lib)

rootrel = .\dvidir
rootfq = $(miktexsrcdir)\libdvi\test\src\dvidir

# _____________________________________________________________________________
#
# QRT
# _____________________________________________________________________________

check: common-check $(outdir) $(rootrel)\test.dvi .\test-rel.dvi \
		$(rootrel)\test-fq.dvi $(outdir)\1.exe
	 $(outdir)\1.exe

$(rootrel)\test.tex: $(rootrel)
	  $(weave) $(miktexsrcdir)\tex\tex.web nul $(rootrel)\test.tex

$(rootrel)\test.dvi: $(rootrel)\test.tex
	cd $(rootrel) \
	  & $(tex) -src-specials test.tex

.\test-rel.dvi: $(rootrel)\test.tex
	$(tex) -src-specials -job-name=test-rel \
		-include-directory=$(rootrel) \
		$(rootrel)\test.tex

$(rootrel)\test-fq.dvi: $(rootrel)\test.tex
	$(tex) -src-specials -job-name=test-fq \
		-output-directory=$(rootfq) $(rootfq)\test.tex

$(rootrel):
	$(mkpath) $@

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

depend: 1.cpp
	$(MAKEDEPEND) -I..\.. $**
	$(fixdep)

# DO NOT DELETE
