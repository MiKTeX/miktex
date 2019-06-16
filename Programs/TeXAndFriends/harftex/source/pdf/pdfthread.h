/* pdfthread.h

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


#ifndef PDFTHREAD_H
#  define PDFTHREAD_H

/* data structure of threads; words 1..4 represent the coordinates of the  corners */

#  define obj_thread_first obj_aux    /* pointer to the first bead */

/* data structure of beads */
#  define pdfmem_bead_size 5   /* size of memory in |pdf->mem| which  |obj_bead_ptr| points to */

#  define set_pdf_thread_attr(A,B)      pdf_thread_attr(A)=B
#  define set_pdf_thread_id(A,B)        pdf_thread_id(A)=B
#  define set_pdf_thread_named_id(A,B)  pdf_thread_named_id(A)=B

#  define obj_bead_ptr                  obj_aux /* pointer to |pdf->mem| */
#  define obj_bead_rect(pdf,A)          pdf->mem[obj_bead_ptr(pdf,A)]
#  define obj_bead_page(pdf,A)          pdf->mem[obj_bead_ptr(pdf,A) + 1]
#  define obj_bead_next(pdf,A)          pdf->mem[obj_bead_ptr(pdf,A) + 2]
#  define obj_bead_prev(pdf,A)          pdf->mem[obj_bead_ptr(pdf,A) + 3]
#  define obj_bead_attr(pdf,A)          pdf->mem[obj_bead_ptr(pdf,A) + 4]
#  define obj_bead_data                 obj_bead_rect
#  define set_obj_bead_data             set_obj_bead_rect

#  define set_obj_bead_rect(pdf,A,B)    obj_bead_rect(pdf,A)=B
#  define set_obj_bead_page(pdf,A,B)    obj_bead_page(pdf,A)=B
#  define set_obj_bead_next(pdf,A,B)    obj_bead_next(pdf,A)=B
#  define set_obj_bead_prev(pdf,A,B)    obj_bead_prev(pdf,A)=B
#  define set_obj_bead_attr(pdf,A,B)    obj_bead_attr(pdf,A)=B


/*
    pointer to the corresponding whatsit node; |obj_bead_rect| is needed only when the bead
    rectangle has been written out and after that |obj_bead_data| is not needed any more
    so we can use this field for both
*/

extern void append_bead(PDF pdf, halfword p);
extern void do_thread(PDF pdf, halfword parent_box, halfword p, scaledpos cur);
extern void append_thread(PDF pdf, halfword parent_box, scaledpos cur);
extern void end_thread(PDF pdf, halfword p);
extern void scan_thread_id(void);

extern void thread_title(PDF pdf, int t);
extern void pdf_fix_thread(PDF pdf, int t);
extern void out_thread(PDF pdf, int t);

extern void check_running_thread(PDF pdf, halfword this_box, scaledpos cur);
extern void print_bead_rectangles(PDF pdf);
extern void flush_beads_list(void);

#endif
