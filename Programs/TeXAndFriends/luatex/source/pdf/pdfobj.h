/* pdfobj.h

   Copyright 2009-2011 Taco Hoekwater <taco@luatex.org>

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

#ifndef PDFOBJ_H
#  define PDFOBJ_H

/* data structure for \.{\\pdfextension obj} and \.{\\pdfextension refobj} */

#  define set_pdf_obj_objnum(A, B) pdf_obj_objnum(A) = (B)

#  define pdfmem_obj_size 5 /* size of memory in |mem| which |obj_data_ptr| holds */

#  define obj_obj_data(pdf, A)             pdf->mem[obj_data_ptr((pdf), (A)) + 0] /* object data */
#  define obj_obj_stream_attr(pdf, A)      pdf->mem[obj_data_ptr((pdf), (A)) + 1] /* additional attributes into stream dict */
#  define obj_obj_flags(pdf, A)            pdf->mem[obj_data_ptr((pdf), (A)) + 2] /* stream/file flags */
#  define obj_obj_pdfcompresslevel(pdf, A) pdf->mem[obj_data_ptr((pdf), (A)) + 3] /* overrides \pdfcompresslevel */
#  define obj_obj_objstm_threshold(pdf, A) pdf->mem[obj_data_ptr((pdf), (A)) + 4] /* for object stream compression */

#  define OBJ_FLAG_ISSTREAM              (1 << 0)
#  define OBJ_FLAG_ISFILE                (1 << 1)

#  define obj_obj_is_stream(pdf,A)       ((obj_obj_flags((pdf), (A)) & OBJ_FLAG_ISSTREAM) != 0)
#  define set_obj_obj_is_stream(pdf,A)   ((obj_obj_flags((pdf), (A)) |= OBJ_FLAG_ISSTREAM))
#  define unset_obj_obj_is_stream(pdf,A) ((obj_obj_flags((pdf), (A)) &= ~OBJ_FLAG_ISSTREAM))

#  define obj_obj_is_file(pdf,A)         ((obj_obj_flags((pdf), (A)) & OBJ_FLAG_ISFILE) != 0)
#  define set_obj_obj_is_file(pdf,A)     ((obj_obj_flags((pdf), (A)) |= OBJ_FLAG_ISFILE))
#  define unset_obj_obj_is_file(pdf,A)   ((obj_obj_flags((pdf), (A)) &= ~OBJ_FLAG_ISFILE))

extern void init_obj_obj(PDF pdf, int k);
extern void pdf_write_obj(PDF pdf, int n);
extern void scan_obj(PDF pdf);
extern void scan_refobj(PDF pdf);
extern void scan_refobj_lua(PDF pdf, int k);
extern void pdf_ref_obj(PDF pdf, halfword p);
extern void pdf_ref_obj_lua(PDF pdf, int k);

#endif
