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
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_bibfile\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_biblist\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_spreamble\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_entryints\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_entrystrs\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_wizfunctions\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_fieldinfo\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_strpool\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
