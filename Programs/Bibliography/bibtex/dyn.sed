## dyn.sed: make dynamic arrays
## 
## Copyright (C) 1991-2020 Christian Schenk
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

s/^\([_A-Za-z0-9]*\) \(bibfile\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(biblist\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(citeinfo\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(citelist\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(entryexists\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(entryints\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(entrystrs\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fieldinfo\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fntype\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(glbstrend\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(glbstrptr\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(glbstrstr\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(globalstrs\)\[ [0-9]* \]\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(hashilk\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(hashnext\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(hashtext\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(ilkinfo\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(litstack\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(litstktype\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(namesepchar\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(nametok\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(singlfunction\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(spreamble\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(strpool\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(strstart\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(typelist\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(wizfunctions\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^typedef \([_A-Za-z0-9]*\) \(buftype\)\[ [0-9]* \];$/typedef \1 *\2;/
