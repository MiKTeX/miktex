/* pdfsetmatrix.h

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

#ifndef PDFSETMATRIX_H
#  define PDFSETMATRIX_H

#  define set_pdf_setmatrix_data(A,B) pdf_setmatrix_data(A)=B

typedef struct {
    double a;
    double b;
    double c;
    double d;
    double e;
    double f;
} matrix_entry;

extern matrix_entry *matrix_stack;
extern int matrix_stack_size;
extern int matrix_stack_used;

scaled getllx(void);
scaled getlly(void);
scaled geturx(void);
scaled getury(void);
void matrixtransformpoint(scaled x, scaled y);
void matrixtransformrect(scaled llx, scaled lly, scaled urx, scaled ury);
boolean matrixused(void);
void matrixrecalculate(scaled urx);

extern void pdf_out_setmatrix(PDF pdf, halfword p);

#endif
