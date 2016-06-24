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
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_after\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_before\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_bisectstack\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_buffer\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_c4p_free\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_delta\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_deltax\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_deltay\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_envmove\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_inputstack\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_ligkern\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_mem\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_move\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_nodetoround\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_paramstack\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_psi\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_strpool\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_strref\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_strstart\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_theta\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_trickbuf\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_uu\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_vv\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_wasfree\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
s/^C4PEXTERN \([_A-Za-z0-9]*\) \([gm]_ww\)\[ [A-Za-z0-9]* \];$/C4PEXTERN \1 *\2;/
