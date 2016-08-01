% pdfcolorstack.w
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

@* Color Stack and Matrix Transformation Support.

@ In the following array and especially stack data structures are used.

They have the following properties:

\item{-} They automatically grow dynamically.
\item{-} The size never decreases.
\item{-} The variable with name ending in "size" contains the number how many
    entries the data structure can hold.
\item{-} The variable with name ending in "used" contains the number of
    actually used entries.
\item{-} Memory of strings in stack entries must be allocated and
    freed if the stack is cleared.

@ Color Stack
@c

#define MAX_COLORSTACKS 32768

/*
    The colorstack number is stored in two bytes (info field of the node);
    condition (newcolorstack): |MAX_COLORSTACKS mod STACK_INCREMENT = 0|
*/

#define COLOR_DEFAULT "0 g 0 G"

typedef struct {
    char **page_stack;
    char **form_stack;
    char *page_current;
    char *form_current;
    char *form_init;
    int page_size;
    int form_size;
    int page_used;
    int form_used;
    int literal_mode;
    boolean page_start;
} colstack_type;

static colstack_type *colstacks = NULL;
static int colstacks_size = 0;
static int colstacks_used = 0;

@ Initialization is done, if the color stacks are used, |init_colorstacks()| is defined
as macro to avoid unnecessary procedure calls.

@c
#define init_colorstacks() if (colstacks_size == 0) colstacks_first_init();

static void colstacks_first_init(void)
{
    colstacks_size = STACK_INCREMENT;
    colstacks = xtalloc((unsigned) colstacks_size, colstack_type);
    colstacks_used = 1;
    colstacks[0].page_stack = NULL;
    colstacks[0].form_stack = NULL;
    colstacks[0].page_size = 0;
    colstacks[0].form_size = 0;
    colstacks[0].page_used = 0;
    colstacks[0].form_used = 0;
    colstacks[0].page_current = xstrdup(COLOR_DEFAULT);
    colstacks[0].form_current = xstrdup(COLOR_DEFAULT);
    colstacks[0].form_init = xstrdup(COLOR_DEFAULT);
    colstacks[0].literal_mode = direct_always;
    colstacks[0].page_start = true;
}

@ @c
int colorstackused(void)
{
    init_colorstacks();
    return colstacks_used;
}

@   A new color stack is setup with the given parameters. The stack number is returned
or -1 in case of error (no room).

@c
int newcolorstack(const char *str, int literal_mode, boolean page_start)
{
    colstack_type *colstack;
    int colstack_num;
    init_colorstacks();
    /* make room */
    if (colstacks_used == MAX_COLORSTACKS) {
        return -1;
    }
    if (colstacks_used == colstacks_size) {
        colstacks_size += STACK_INCREMENT;
        /*
            If |(MAX_COLORSTACKS mod STACK_INCREMENT = 0)| then we don't
            need to check the case that size overruns |MAX_COLORSTACKS|.
        */
        colstacks = xreallocarray(colstacks, colstack_type, (unsigned) colstacks_size);
    }
    /* claim new color stack */
    colstack_num = colstacks_used++;
    colstack = &colstacks[colstack_num];
    /* configure the new color stack */
    colstack->page_stack = NULL;
    colstack->form_stack = NULL;
    colstack->page_size = 0;
    colstack->page_used = 0;
    colstack->form_size = 0;
    colstack->form_used = 0;
    colstack->literal_mode = literal_mode;
    colstack->page_start = page_start;
    colstack->page_current = NULL;
    colstack->form_current = NULL;
    colstack->form_init = NULL;
    if (str) {
        colstack->page_current = xstrdup(str);
        colstack->form_current = xstrdup(str);
        colstack->form_init = xstrdup(str);
    }
    return colstack_num;
}

@ @c
#define get_colstack(n) (&colstacks[n])

@ Puts a string on top of the string pool and updates |pool_ptr|.

@c static void put_cstring_on_str_pool(char *str)
{
    int save_selector = selector;
    selector = new_string;
    if (str == NULL || *str == 0) {
        return;
    }
    tprint(str);
    selector = save_selector;
}

@ @c
static int colorstackset(int colstack_no, str_number s)
{
    colstack_type *colstack = get_colstack(colstack_no);

    if (global_shipping_mode == SHIPPING_PAGE) {
        xfree(colstack->page_current);
        colstack->page_current = makecstring(s);
    } else {
        xfree(colstack->form_current);
        colstack->form_current = makecstring(s);
    }
    return colstack->literal_mode;
}

@ @c
int colorstackcurrent(int colstack_no)
{
    colstack_type *colstack = get_colstack(colstack_no);

    if (global_shipping_mode == SHIPPING_PAGE) {
        put_cstring_on_str_pool(colstack->page_current);
    } else {
        put_cstring_on_str_pool(colstack->form_current);
    }
    return colstack->literal_mode;
}

@ @c
static int colorstackpush(int colstack_no, str_number s)
{
    colstack_type *colstack = get_colstack(colstack_no);
    char *str;
    if (global_shipping_mode == SHIPPING_PAGE) {
        if (colstack->page_used == colstack->page_size) {
            colstack->page_size += STACK_INCREMENT;
            xretalloc(colstack->page_stack, (unsigned) colstack->page_size, char *);
        }
        colstack->page_stack[colstack->page_used++] = colstack->page_current;
        str = makecstring(s);
        if (*str == 0) {
            colstack->page_current = NULL;
        } else {
            colstack->page_current = xstrdup(str);
        }
        free(str);
    } else {
        if (colstack->form_used == colstack->form_size) {
            colstack->form_size += STACK_INCREMENT;
            xretalloc(colstack->form_stack, (unsigned) colstack->form_size, char *);
        }
        colstack->form_stack[colstack->form_used++] = colstack->form_current;
        str = makecstring(s);
        if (*str == 0) {
            colstack->form_current = NULL;
        } else {
            colstack->form_current = xstrdup(str);
        }
        free(str);
    }
    return colstack->literal_mode;
}

@ @c
int colorstackpop(int colstack_no)
{
    colstack_type *colstack = get_colstack(colstack_no);
    if (global_shipping_mode == SHIPPING_PAGE) {
        if (colstack->page_used == 0) {
            formatted_warning("pdf backend","pop empty color page stack %u",(unsigned int) colstack_no);
            return colstack->literal_mode;
        }
        xfree(colstack->page_current);
        colstack->page_current = colstack->page_stack[--colstack->page_used];
        put_cstring_on_str_pool(colstack->page_current);
    } else {
        if (colstack->form_used == 0) {
            formatted_warning("pdf backend","pop empty color form stack %u",(unsigned int) colstack_no);
            return colstack->literal_mode;
        }
        xfree(colstack->form_current);
        colstack->form_current = colstack->form_stack[--colstack->form_used];
        put_cstring_on_str_pool(colstack->form_current);
    }
    return colstack->literal_mode;
}

@ @c
void colorstackpagestart(void)
{
    int i, j;
    colstack_type *colstack;
    if (global_shipping_mode == SHIPPING_PAGE) {
        /* see procedure |pdf_out_colorstack_startpage| */
        return;
    }

    for (i = 0; i < colstacks_used; i++) {
        colstack = &colstacks[i];
        for (j = 0; j < colstack->form_used; j++) {
            xfree(colstack->form_stack[j]);
        }
        colstack->form_used = 0;
        xfree(colstack->form_current);
        if (colstack->form_init == NULL) {
            colstack->form_current = NULL;
        } else {
            colstack->form_current = xstrdup(colstack->form_init);
        }
    }
}

@ @c
int colorstackskippagestart(int colstack_no)
{
    colstack_type *colstack = get_colstack(colstack_no);
    if (!colstack->page_start) {
        return 1;
    }
    if (colstack->page_current == NULL) {
        return 0;
    }
    if (strcmp(COLOR_DEFAULT, colstack->page_current) == 0) {
        return 2;
    }
    return 0;
}


@ @c
void pdf_out_colorstack(PDF pdf, halfword p)
{
    int old_setting;
    str_number s;
    int cmd = pdf_colorstack_cmd(p);
    int stack_no = pdf_colorstack_stack(p);
    int literal_mode = 0;
    if (stack_no >= colorstackused()) {
        tprint_nl("");
        tprint("Color stack ");
        print_int(stack_no);
        tprint(" is not initialized for use!");
        tprint_nl("");
        return;
    }
    switch (cmd) {
        case colorstack_set:
        case colorstack_push:
            old_setting = selector;
            selector = new_string;
            show_token_list(token_link(pdf_colorstack_data(p)), null, -1);
            selector = old_setting;
            s = make_string();
            if (cmd == colorstack_set)
                literal_mode = colorstackset(stack_no, s);
            else
                literal_mode = colorstackpush(stack_no, s);
            if (str_length(s) > 0)
                pdf_literal(pdf, s, literal_mode, false);
            flush_str(s);
            return;
            break;
        case colorstack_pop:
            literal_mode = colorstackpop(stack_no);
            break;
        case colorstack_current:
            literal_mode = colorstackcurrent(stack_no);
            break;
        default:
            break;
    }
    if (cur_length > 0) {
        s = make_string();
        pdf_literal(pdf, s, literal_mode, false);
        flush_str(s);
    }
}

@ @c
void pdf_out_colorstack_startpage(PDF pdf)
{
    int start_status;
    int literal_mode;
    str_number s;
    int i = 0;
    int max = colorstackused();
    while (i < max) {
        start_status = colorstackskippagestart(i);
        if (start_status == 0) {
            literal_mode = colorstackcurrent(i);
            if (cur_length > 0) {
                s = make_string();
                pdf_literal(pdf, s, literal_mode, false);
                flush_str(s);
            }
        }
        i++;
    }
}
