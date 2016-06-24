/* luatex-common.h

   Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
   Copyright 2006-2012 Taco Hoekwater <taco@luatex.org>
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


/* This file contains declarations used in C code as well as in C++ code.
*/

#ifndef LUATEX_COMMON_H
#  define LUATEX_COMMON_H

/* utils.c */

extern void tex_error(const char *msg, const char **hlp);
extern void normal_error(const char *t, const char *p);
extern void normal_warning(const char *t, const char *p);
extern void formatted_error(const char *t, const char *fmt, ...);
extern void formatted_warning(const char *t, const char *fmt, ...);

#endif /* LUATEX_COMMON_H */
