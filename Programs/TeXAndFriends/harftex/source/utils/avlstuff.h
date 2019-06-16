/* avlstuff.h

   Copyright (c) 2004-2007 Han The Thanh, <thanh@pdftex.org>
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


#ifndef AVLSTUFF_H
#  define AVLSTUFF_H

#  include "avl.h"

extern struct libavl_allocator avl_xallocator;

int comp_int_entry(const void *, const void *, void *);
int comp_string_entry(const void *, const void *, void *);

#endif                          /* AVLSTUFF_H */
