## dyn.sed: make dynamic arrays
## 
## Copyright (C) 1991-2016 Christian Schenk
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
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_bcharlabel\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_buffer\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_charbase\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_depthbase\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_extenbase\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fontarea\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fontbc\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fontbchar\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fontcheck\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fontdsize\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fontec\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fontfalsebchar\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fontglue\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fontinfo\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fontname\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fontparams\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fontsize\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fontused\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fullsourcefilenamestack\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_heightbase\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_hyphenchar\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_inputfile\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_inputfilemode\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_inputfiletranslation\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_inputstack\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_italicbase\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_kernbase\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_ligkernbase\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_linestack\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_mem\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_nameoffile\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_nest\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_parambase\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_paramstack\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_savestack\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_skewchar\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_sourcefilenamestack\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_strpool\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_strstart\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_trickbuf\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_triec\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_triehash\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_triel\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_trieo\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_trier\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_trietaken\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_widthbase\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
