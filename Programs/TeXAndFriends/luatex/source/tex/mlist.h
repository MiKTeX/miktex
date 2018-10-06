/* mlist.h

   Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
   Copyright 2006-2009 Taco Hoekwater <taco@luatex.org>

   This file is part of LuaTeX.

   LuaTeX is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   LuaTeX is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU General Public License along
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


#ifndef MLIST_H
#  define MLIST_H 1

extern int cur_size;

extern void run_mlist_to_hlist(halfword, boolean, int);
extern void mlist_to_hlist(halfword, boolean, int);
extern void fixup_math_parameters(int fam_id, int size_id, int f, int lvl);

extern scaled get_math_quad_style(int a);
extern scaled get_math_quad_size(int a);

extern pointer make_extensible(internal_font_number fnt, halfword chr, scaled v, scaled min_overlap, int horizontal, halfword att);

#endif
