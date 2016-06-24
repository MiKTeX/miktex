% pdfpagetree.w
%
% Copyright 2006-2011 Taco Hoekwater <taco@@luatex.org>
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

@* Page diversions.

@ @c
# define PAGES_TREE_KIDSMAX 10

static struct avl_table *divert_list_tree = NULL;

typedef struct pages_entry_ {
    int objnum;                   /* object number of this /Pages object */
    int number_of_pages;          /* total number of all pages below */
    int number_of_kids;           /* number of direct kid objects */
    int kids[PAGES_TREE_KIDSMAX]; /* array of kid object numbers */
    struct pages_entry_ *next;
} pages_entry;

typedef struct divert_list_entry_ {
    int divnum;
    pages_entry *first;
    pages_entry *last;
} divert_list_entry;

static int comp_divert_list_entry(const void *pa, const void *pb, void *p)
{
    (void) p;
    if (((const divert_list_entry *) pa)->divnum > ((const divert_list_entry *) pb)->divnum)
        return 1;
    if (((const divert_list_entry *) pa)->divnum < ((const divert_list_entry *) pb)->divnum)
        return -1;
    return 0;
}

@ @c
static pages_entry *new_pages_entry(PDF pdf)
{
    int i;
    pages_entry *p = xtalloc(1, pages_entry);
    p->number_of_pages = p->number_of_kids = 0;
    for (i = 0; i < PAGES_TREE_KIDSMAX; i++)
        p->kids[i] = 0;
    p->next = NULL;
    p->objnum = pdf_create_obj(pdf, obj_type_pages, 0);
    return p;
}

@ @c
static divert_list_entry *new_divert_list_entry(void)
{
    divert_list_entry *d;
    d = xtalloc(1, divert_list_entry);
    d->first = d->last = NULL;
    return d;
}

@ @c
static void ensure_list_tree(void)
{
    if (divert_list_tree == NULL) {
        divert_list_tree = avl_create(comp_divert_list_entry, NULL, &avl_xallocator);
    }
}

@ @c
static divert_list_entry *get_divert_list(int divnum)
{
    divert_list_entry *d, tmp;
    void **aa;
    tmp.divnum = divnum;
    d = (divert_list_entry *) avl_find(divert_list_tree, &tmp);
    if (d == NULL) {
        d = new_divert_list_entry();
        d->divnum = divnum;
        /* the next bit of code can actually be removed */
        aa = avl_probe(divert_list_tree, d);
        if (aa==NULL) {
            normal_error("pdf backend","page list lookup error");
        }
    }
    return d;
}

@ |pdf_do_page_divert()| returns the current /Parent object number
@c
int pdf_do_page_divert(PDF pdf, int objnum, int divnum)
{
    divert_list_entry *d;
    pages_entry *p;
    /* initialize the tree */
    ensure_list_tree();
    /* make sure we have a list for this diversion */
    d = get_divert_list(divnum);
    if (d->first == NULL || d->last->number_of_kids == PAGES_TREE_KIDSMAX) {
        /* append a new |pages_entry| */
        p = new_pages_entry(pdf);
        if (d->first == NULL)
            d->first = p;
        else
            d->last->next = p;
        d->last = p;
    }
    p = d->last;
    p->kids[p->number_of_kids++] = objnum;
    p->number_of_pages++;
    return p->objnum;
}

@ @c
static void movelist(divert_list_entry * d, divert_list_entry * dto)
{
    if (d != NULL && d->first != NULL && d->divnum != dto->divnum) {
        /* no undivert of empty list or into self */
        if (dto->first == NULL)
            dto->first = d->first;
        else
            dto->last->next = d->first;
        dto->last = d->last;
        /* one could as well remove this |divert_list_entry| */
        d->first = d->last = NULL;
    }
}

@ undivert from diversion |divnum| into diversion |curdivnum|
@c
void pdf_do_page_undivert(int divnum, int curdivnum)
{
    divert_list_entry *d, *dto, tmp;
    struct avl_traverser t;
    /*  initialize the tree */
    ensure_list_tree();
    /* find the diversion |curdivnum| list where diversion |divnum| should go */
    dto = get_divert_list(curdivnum);
    if (divnum == 0) {
        /* 0 = special case: undivert {\it all\/} lists */
        avl_t_init(&t, divert_list_tree);
        for (d = avl_t_first(&t, divert_list_tree); d != NULL;
             d = avl_t_next(&t))
            movelist(d, dto);
    } else {
        tmp.divnum = divnum;
        d = (divert_list_entry *) avl_find(divert_list_tree, &tmp);
        movelist(d, dto);
    }
}

@ write a /Pages object
@c

static void write_pages(PDF pdf, pages_entry * p, int parent)
{
    int i;
    int pages_attributes ;
    pdf_begin_obj(pdf, p->objnum, OBJSTM_ALWAYS);
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "Pages");
    if (parent == 0) {
        /* it's root */
        pages_attributes = pdf_pages_attr; /* lookup once */
        if (pages_attributes != null) {
            pdf_print_toks(pdf, pages_attributes);
            pdf_out(pdf, ' ');
        }
        print_pdf_table_string(pdf, "pagesattributes");
        pdf_out(pdf, ' ');
    } else
        pdf_dict_add_ref(pdf, "Parent", parent);
    pdf_dict_add_int(pdf, "Count", (int) p->number_of_pages);
    pdf_add_name(pdf, "Kids");
    pdf_begin_array(pdf);
    for (i = 0; i < p->number_of_kids; i++)
        pdf_add_ref(pdf, (int) p->kids[i]);
    pdf_end_array(pdf);
    pdf_end_dict(pdf);
    pdf_end_obj(pdf);
}

@ loop over all /Pages objects, output them, create their parents,
recursing bottom up, return the /Pages root object number

@c
static int output_pages_list(PDF pdf, pages_entry * pe)
{
    pages_entry *p, *q, *r;
    if (pe->next == NULL) {
        /* everything fits into one |pages_entry| */
        write_pages(pdf, pe, 0); /* /Pages root found */
        return pe->objnum;
    }
    q = r = new_pages_entry(pdf); /* one level higher needed */
    for (p = pe; p != NULL; p = p->next) {
        if (q->number_of_kids == PAGES_TREE_KIDSMAX) {
            q->next = new_pages_entry(pdf);
            q = q->next;
        }
        q->kids[q->number_of_kids++] = p->objnum;
        q->number_of_pages += p->number_of_pages;
        write_pages(pdf, p, q->objnum);
    }
    return output_pages_list(pdf, r); /* recurse through next higher level */
}

@ @c
int output_pages_tree(PDF pdf)
{
    divert_list_entry *d;
    pdf_do_page_undivert(0, 0); /* concatenate all diversions into diversion 0 */
    d = get_divert_list(0);     /* get diversion 0 */
    return output_pages_list(pdf, d->first);
}
