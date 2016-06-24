% pdfsaverestore.w
%
% Copyright 2010 Taco Hoekwater <taco@@luatex.org>
%
% This file is part of LuaTeX.
%
% LuaTeX is free software; you can redistribute it and/or modify it under
% the terms of the GNU General Public License as published by the Free
% Software Foundation; either version 2 of the License, or (at your
% option) any later version.
%
% LuaTeX is distributed in the hope that it will be useful, but WITHOUT
% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
% FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
% License for more details.
%
% You should have received a copy of the GNU General Public License along
% with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

@ @c

#include "ptexlib.h"

@ @c
pos_entry *pos_stack = 0; /* the stack */
int pos_stack_size = 0;   /* initially empty */
int pos_stack_used = 0;   /* used entries */

@ @c
static void checkpdfsave(scaledpos pos)
{
    pos_entry *new_stack;
    if (pos_stack_used >= pos_stack_size) {
        pos_stack_size += STACK_INCREMENT;
        new_stack = xtalloc((unsigned) pos_stack_size, pos_entry);
        memcpy((void *) new_stack, (void *) pos_stack, (unsigned) pos_stack_used * sizeof(pos_entry));
        xfree(pos_stack);
        pos_stack = new_stack;
    }
    pos_stack[pos_stack_used].pos.h = pos.h;
    pos_stack[pos_stack_used].pos.v = pos.v;
    if (global_shipping_mode == SHIPPING_PAGE) {
        pos_stack[pos_stack_used].matrix_stack = matrix_stack_used;
    }
    pos_stack_used++;
}

@ @c
static void checkpdfrestore(scaledpos pos)
{
    scaledpos diff;
    if (pos_stack_used == 0) {
        normal_warning("pdf backend", "'restore' is missing a 'save'");
        return;
    }
    pos_stack_used--;
    diff.h = pos.h - pos_stack[pos_stack_used].pos.h;
    diff.v = pos.v - pos_stack[pos_stack_used].pos.v;
    if (diff.h != 0 || diff.v != 0) {
        formatted_warning("pdf backend","misplaced 'restore' by (%dsp, %dsp)", (int) diff.h, (int) diff.v);
    }
    if (global_shipping_mode == SHIPPING_PAGE) {
        matrix_stack_used = pos_stack[pos_stack_used].matrix_stack;
    }
}

@ @c
void pdf_out_save(PDF pdf, halfword p)
{
    (void) p;
    checkpdfsave(pdf->posstruct->pos);
    pdf_literal(pdf, 'q', set_origin, false);
}

@ @c
void pdf_out_restore(PDF pdf, halfword p)
{
    (void) p;
    checkpdfrestore(pdf->posstruct->pos);
    pdf_literal(pdf, 'Q', set_origin, false);
}
