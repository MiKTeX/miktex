/* utils.h

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


#ifndef UTILS_H
#  define UTILS_H

extern int epochseconds;
extern int microseconds;

void make_subset_tag(fd_entry *);

__attribute__ ((format(printf, 1, 2)))
void tex_printf(const char *, ...);

void garbage_warning(void);
size_t xfwrite(void *, size_t size, size_t nmemb, FILE *);
int xfflush(FILE *);
int xgetc(FILE *);
int xputc(int, FILE *);
scaled ext_xn_over_d(scaled, scaled, scaled);
void initversionstring(char **versions);
extern void check_buffer_overflow(int wsize);
extern void check_pool_overflow(int wsize);

extern char *cur_file_name;

#  include "luatex-common.h"

#endif                          /* UTILS_H */
