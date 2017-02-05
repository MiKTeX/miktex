## dyn.sed: make dynamic arrays
## 
## Copyright (C) 1991-2017 Christian Schenk
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
s/^\([_A-Za-z0-9]*\) \(bcharlabel\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(buffer\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(charbase\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(depthbase\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(extenbase\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontarea\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontbc\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontbchar\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontcheck\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontdsize\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontec\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontfalsebchar\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontglue\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontinfo\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontname\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontparams\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontsize\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontused\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fullsourcefilenamestack\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(heightbase\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(hyphenchar\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(inputfile\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(inputfilemode\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(inputfiletranslation\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(inputstack\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(italicbase\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(kernbase\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(ligkernbase\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(linestack\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(mem\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(nameoffile\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(nest\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(parambase\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(paramstack\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(savestack\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(skewchar\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(sourcefilenamestack\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(strpool\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(strstart\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(trickbuf\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(triec\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(triehash\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(triel\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(trieo\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(trier\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(trietaken\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(widthbase\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
