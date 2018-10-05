/* pdftoepdf.h

   Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
   Copyright 2006-2011 Taco Hoekwater <taco@luatex.org>

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


#ifndef PDFTOEPDF_H
#  define PDFTOEPDF_H

#  include "image.h"

void read_pdf_info(image_dict *);
void flush_pdf_info(image_dict *);
/*   flush_pdfstream_info(image_dict *); */ /* somehow doesn't work due to xfree macro */
void unrefPdfDocument(char *);
void unrefMemStreamPdfDocument(char *);
void write_epdf(PDF, image_dict *, int suppress_optional_info);
void epdf_check_mem(void);
void copyReal(PDF pdf, double d);

int poppler_version_major(void);
int poppler_version_minor(void);
int poppler_version_micro(void);

/* epdf.c --- this should go in an own header file */

extern void epdf_free(void);

#endif                          /* PDFTOEPDF_H */
