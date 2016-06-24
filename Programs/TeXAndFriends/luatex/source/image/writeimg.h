/* writeimg.h

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

#ifndef WRITEIMG_H
#  define WRITEIMG_H

#  include "dvi/dvigen.h"
#  include "image.h"
#  include "pdf/pdfpage.h"

typedef image_dict *idict_entry;
idict_entry *idict_array;

void new_img_pdfstream_struct(image_dict *);
image *new_image(void);
image_dict *new_image_dict(void);
void free_image_dict(image_dict * p);
void read_img(image_dict * p);
void scan_pdfximage(PDF pdf);
void scan_pdfrefximage(PDF pdf);
scaled_whd tex_scale(scaled_whd nat, scaled_whd tex);
scaled_whd scale_img(image_dict *, scaled_whd, int);
void write_img(PDF, image_dict *);
void pdf_write_image(PDF pdf, int n);
void check_pdfstream_dict(image_dict *);
void write_pdfstream(PDF, image_dict *);
void idict_to_array(image_dict *);
void dumpimagemeta(void);
void undumpimagemeta(PDF, int, int);
scaled_whd scan_alt_rule(void);
size_t read_file_to_buf(PDF pdf, FILE * f, size_t len);
void pdf_dict_add_img_filename(PDF pdf, image_dict * idict);

#endif
