/* pdfsaverestore.h

   Copyright 2009 Taco Hoekwater <taco@luatex.org>

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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PDFSAVERESTORE_H
#  define PDFSAVERESTORE_H

/* stack for positions of \pdfsave */

typedef struct {
    scaledpos pos;
    int matrix_stack;
} pos_entry;

extern pos_entry *pos_stack; /* the stack */
extern int pos_stack_size;   /* initially empty */
extern int pos_stack_used;   /* used entries */

extern void pdf_out_save(PDF pdf, halfword p);
extern void pdf_out_restore(PDF pdf, halfword p);

#endif
