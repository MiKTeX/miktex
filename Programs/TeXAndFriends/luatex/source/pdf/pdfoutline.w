% pdfoutline.w
%
% Copyright 2009-2011 Taco Hoekwater <taco@@luatex.org>
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

@ Data structure of outlines; it's not able to write out outline entries before
all outline entries are defined, so memory allocated for outline entries can't
not be deallocated and will stay in memory. For this reason we will store data of
outline entries in |pdf->mem| instead of |mem|

@c
#define pdfmem_outline_size 8 /* size of memory in |pdf->mem| which |obj_outline_ptr| points to */

#define obj_outline_count                obj_info      /* count of all opened children */
#define obj_outline_ptr                  obj_aux       /* pointer to |pdf->mem| */

#define obj_outline_title(pdf,A)         pdf->mem[obj_outline_ptr(pdf,A)]
#define obj_outline_parent(pdf,A)        pdf->mem[obj_outline_ptr(pdf,A) + 1]
#define obj_outline_prev(pdf,A)          pdf->mem[obj_outline_ptr(pdf,A) + 2]
#define obj_outline_next(pdf,A)          pdf->mem[obj_outline_ptr(pdf,A) + 3]
#define obj_outline_first(pdf,A)         pdf->mem[obj_outline_ptr(pdf,A) + 4]
#define obj_outline_last(pdf,A)          pdf->mem[obj_outline_ptr(pdf,A) + 5]
#define obj_outline_action_objnum(pdf,A) pdf->mem[obj_outline_ptr(pdf,A) + 6]  /* object number of action */
#define obj_outline_attr(pdf,A)          pdf->mem[obj_outline_ptr(pdf,A) + 7]

#define set_obj_outline_count(pdf,A,B)         obj_outline_count(pdf,A)=B
#define set_obj_outline_ptr(pdf,A,B)           obj_outline_ptr(pdf,A)=B
#define set_obj_outline_action_objnum(pdf,A,B) obj_outline_action_objnum(pdf,A)=B
#define set_obj_outline_title(pdf,A,B)         obj_outline_title(pdf,A)=B
#define set_obj_outline_prev(pdf,A,B)          obj_outline_prev(pdf,A)=B
#define set_obj_outline_next(pdf,A,B)          obj_outline_next(pdf,A)=B
#define set_obj_outline_first(pdf,A,B)         obj_outline_first(pdf,A)=B
#define set_obj_outline_last(pdf,A,B)          obj_outline_last(pdf,A)=B
#define set_obj_outline_parent(pdf,A,B)        obj_outline_parent(pdf,A)=B
#define set_obj_outline_attr(pdf,A,B)          obj_outline_attr(pdf,A)=B

@ @c
static int open_subentries(PDF pdf, halfword p)
{
    int c, l, r;
    int k = 0;
    if (obj_outline_first(pdf, p) != 0) {
        l = obj_outline_first(pdf, p);
        do {
            k++;
            c = open_subentries(pdf, l);
            if (obj_outline_count(pdf, l) > 0)
                k = k + c;
            obj_outline_parent(pdf, l) = p;
            r = obj_outline_next(pdf, l);
            if (r == 0)
                obj_outline_last(pdf, p) = l;
            l = r;
        } while (l != 0);
    }
    if (obj_outline_count(pdf, p) > 0)
        obj_outline_count(pdf, p) = k;
    else
        obj_outline_count(pdf, p) = -k;
    return k;
}

@ return number of outline entries in the same level with |p|

@c
static int outline_list_count(PDF pdf, pointer p)
{
    int k = 1;
    while (obj_outline_prev(pdf, p) != 0) {
        k++;
        p = obj_outline_prev(pdf, p);
    }
    return k;
}

@ @c
void scan_pdfoutline(PDF pdf)
{
    halfword q, r;
    int i, k, l;
    int j = 0;
    halfword p = null;
    if (scan_keyword("attr")) {
        scan_toks(false, true);
        r = def_ref;
    } else {
        r = 0;
    }
    if (scan_keyword("useobjnum")) {
        scan_int();
        j = cur_val;
    } else {
        p = scan_action(pdf);
    }
    if (scan_keyword("count")) {
        scan_int();
        i = cur_val;
    } else {
        i = 0;
    }
    scan_toks(false, true);
    q = def_ref;
    if (j == 0) {
        j = pdf_create_obj(pdf, obj_type_others, 0);
        pdf_begin_obj(pdf, j, OBJSTM_ALWAYS);
        write_action(pdf, p);
        pdf_end_obj(pdf);
        delete_action_ref(p);
    }
    k = pdf_create_obj(pdf, obj_type_outline, 0);
    set_obj_outline_ptr(pdf, k, pdf_get_mem(pdf, pdfmem_outline_size));
    set_obj_outline_action_objnum(pdf, k, j);
    set_obj_outline_count(pdf, k, i);
    l = pdf_create_obj(pdf, obj_type_others, 0);
    pdf_begin_obj(pdf, l, OBJSTM_ALWAYS);
    {
        char *s = tokenlist_to_cstring(q, true, NULL);
        pdf_print_str_ln(pdf, s);
        xfree(s);
    }
    delete_token_ref(q);
    pdf_end_obj(pdf);
    set_obj_outline_title(pdf, k, l);
    set_obj_outline_prev(pdf, k, 0);
    set_obj_outline_next(pdf, k, 0);
    set_obj_outline_first(pdf, k, 0);
    set_obj_outline_last(pdf, k, 0);
    set_obj_outline_parent(pdf, k, pdf->parent_outline);
    set_obj_outline_attr(pdf, k, r);
    if (pdf->first_outline == 0)
        pdf->first_outline = k;
    if (pdf->last_outline == 0) {
        if (pdf->parent_outline != 0)
            set_obj_outline_first(pdf, pdf->parent_outline, k);
    } else {
        set_obj_outline_next(pdf, pdf->last_outline, k);
        set_obj_outline_prev(pdf, k, pdf->last_outline);
    }
    pdf->last_outline = k;
    if (obj_outline_count(pdf, k) != 0) {
        pdf->parent_outline = k;
        pdf->last_outline = 0;
    } else if ((pdf->parent_outline != 0) &&
               (outline_list_count(pdf, k) ==
                abs(obj_outline_count(pdf, pdf->parent_outline)))) {
        j = pdf->last_outline;
        do {
            set_obj_outline_last(pdf, pdf->parent_outline, j);
            j = pdf->parent_outline;
            pdf->parent_outline = obj_outline_parent(pdf, pdf->parent_outline);
        } while (!((pdf->parent_outline == 0) ||
                   (outline_list_count(pdf, j) <
                    abs(obj_outline_count(pdf, pdf->parent_outline)))));
        if (pdf->parent_outline == 0)
            pdf->last_outline = pdf->first_outline;
        else
            pdf->last_outline = obj_outline_first(pdf, pdf->parent_outline);
        while (obj_outline_next(pdf, pdf->last_outline) != 0)
            pdf->last_outline = obj_outline_next(pdf, pdf->last_outline);
    }
}

@ In the end we must flush PDF objects that cannot be written out immediately
after shipping out pages.

@c
int print_outlines(PDF pdf)
{
    int k, l, a;
    int outlines;
    if (pdf->first_outline != 0) {
        outlines = pdf_create_obj(pdf, obj_type_others, 0);
        l = pdf->first_outline;
        k = 0;
        do {
            k++;
            a = open_subentries(pdf, l);
            if (obj_outline_count(pdf, l) > 0)
                k = k + a;
            set_obj_outline_parent(pdf, l, pdf->obj_ptr);
            l = obj_outline_next(pdf, l);
        } while (l != 0);
        pdf_begin_obj(pdf, outlines, OBJSTM_ALWAYS);
        pdf_begin_dict(pdf);
        pdf_dict_add_name(pdf, "Type", "Outlines");
        pdf_dict_add_ref(pdf, "First", pdf->first_outline);
        pdf_dict_add_ref(pdf, "Last", pdf->last_outline);
        pdf_dict_add_int(pdf, "Count", k);
        pdf_end_dict(pdf);
        pdf_end_obj(pdf);
        /* Output PDF outline entries */

        k = pdf->head_tab[obj_type_outline];
        while (k != 0) {
            if (obj_outline_parent(pdf, k) == pdf->parent_outline) {
                if (obj_outline_prev(pdf, k) == 0)
                    pdf->first_outline = k;
                if (obj_outline_next(pdf, k) == 0)
                    pdf->last_outline = k;
            }
            pdf_begin_obj(pdf, k, OBJSTM_ALWAYS);
            pdf_begin_dict(pdf);
            pdf_dict_add_ref(pdf, "Title", obj_outline_title(pdf, k));
            pdf_dict_add_ref(pdf, "A", obj_outline_action_objnum(pdf, k));
            if (obj_outline_parent(pdf, k) != 0)
                pdf_dict_add_ref(pdf, "Parent", obj_outline_parent(pdf, k));
            if (obj_outline_prev(pdf, k) != 0)
                pdf_dict_add_ref(pdf, "Prev", obj_outline_prev(pdf, k));
            if (obj_outline_next(pdf, k) != 0)
                pdf_dict_add_ref(pdf, "Next", obj_outline_next(pdf, k));
            if (obj_outline_first(pdf, k) != 0)
                pdf_dict_add_ref(pdf, "First", obj_outline_first(pdf, k));
            if (obj_outline_last(pdf, k) != 0)
                pdf_dict_add_ref(pdf, "Last", obj_outline_last(pdf, k));
            if (obj_outline_count(pdf, k) != 0)
                pdf_dict_add_int(pdf, "Count", obj_outline_count(pdf, k));
            if (obj_outline_attr(pdf, k) != 0) {
                pdf_out(pdf, '\n');
                pdf_print_toks(pdf, obj_outline_attr(pdf, k));
                pdf_out(pdf, '\n');
                delete_token_ref(obj_outline_attr(pdf, k));
                set_obj_outline_attr(pdf, k, null);
            }
            pdf_end_dict(pdf);
            pdf_end_obj(pdf);
            k = obj_link(pdf, k);
        }

    } else {
        outlines = 0;
    }
    return outlines;
}
