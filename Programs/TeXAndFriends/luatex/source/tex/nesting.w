% nesting.w
%
% Copyright 2009-2010 Taco Hoekwater <taco@@luatex.org>
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

@ these are for |show_activities|
@c
#define page_goal page_so_far[0]
#define count(A) eqtb[count_base+(A)].cint


@ \TeX\ is typically in the midst of building many lists at once. For example,
when a math formula is being processed, \TeX\ is in math mode and
working on an mlist; this formula has temporarily interrupted \TeX\ from
being in horizontal mode and building the hlist of a paragraph; and this
paragraph has temporarily interrupted \TeX\ from being in vertical mode
and building the vlist for the next page of a document. Similarly, when a
\.{\\vbox} occurs inside of an \.{\\hbox}, \TeX\ is temporarily
interrupted from working in restricted horizontal mode, and it enters
internal vertical mode.  The ``semantic nest'' is a stack that
keeps track of what lists and modes are currently suspended.

At each level of processing we are in one of six modes:

\yskip\hang|vmode| stands for vertical mode (the page builder);

\hang|hmode| stands for horizontal mode (the paragraph builder);

\hang|mmode| stands for displayed formula mode;

\hang|-vmode| stands for internal vertical mode (e.g., in a \.{\\vbox});

\hang|-hmode| stands for restricted horizontal mode (e.g., in an \.{\\hbox});

\hang|-mmode| stands for math formula mode (not displayed).

\yskip\noindent The mode is temporarily set to zero while processing \.{\\write}
texts in the |ship_out| routine.

Numeric values are assigned to |vmode|, |hmode|, and |mmode| so that
\TeX's ``big semantic switch'' can select the appropriate thing to
do by computing the value |abs(mode)+cur_cmd|, where |mode| is the current
mode and |cur_cmd| is the current command code.

@c
static const char *string_mode(int m)
{                               /* prints the mode represented by |m| */
    if (m > 0) {
        switch (m / (max_command_cmd + 1)) {
        case 0:
            return "vertical mode";
            break;
        case 1:
            return "horizontal mode";
            break;
        case 2:
            return "display math mode";
            break;
        default:
            break;
        }
    } else if (m == 0) {
        return "no mode";
    } else {
        switch ((-m) / (max_command_cmd + 1)) {
        case 0:
            return "internal vertical mode";
            break;
        case 1:
            return "restricted horizontal mode";
            break;
        case 2:
            return "math mode";
            break;
        default:
            break;
        }
    }
    return "unknown mode";
}

@ @c
void print_mode(int m)
{                               /* prints the mode represented by |m| */
    tprint(string_mode(m));
}

@ @c
void print_in_mode(int m)
{                               /* prints the mode represented by |m| */
    tprint("' in ");
    tprint(string_mode(m));
}

@ @c
int get_mode_id(void)
{                               /* returns the mode represented by |m| */
    int m = cur_list.mode_field;
    if (m > 0) {
        switch (m / (max_command_cmd + 1)) {
        case 0:
            return 'v';
            break;
        case 1:
            return 'h';
            break;
        case 2:
            return 'm';
            break;
        default:
            return '\0';
            break;
        }
    } else if (m == 0) {
        return 'n';;
    } else {
        switch ((-m) / (max_command_cmd + 1)) {
        case 0:
            return 'V';
            break;
        case 1:
            return 'H';
            break;
        case 2:
            return 'M';
            break;
        default:
            return '\0';
            break;
        }
    }
}


@ The state of affairs at any semantic level can be represented by
five values:

\yskip\hang|mode| is the number representing the semantic mode, as
just explained.

\yskip\hang|head| is a |pointer| to a list head for the list being built;
|link(head)| therefore points to the first element of the list, or
to |null| if the list is empty.

\yskip\hang|tail| is a |pointer| to the final node of the list being
built; thus, |tail=head| if and only if the list is empty.

\yskip\hang|prev_graf| is the number of lines of the current paragraph that
have already been put into the present vertical list.

\yskip\hang|aux| is an auxiliary |memory_word| that gives further information
that is needed to characterize the situation.

\yskip\noindent
In vertical mode, |aux| is also known as |prev_depth|; it is the scaled
value representing the depth of the previous box, for use in baseline
calculations, or it is |<=-1000|pt if the next box on the vertical list is to
be exempt from baseline calculations.  In horizontal mode, |aux| is also
known as |space_factor|; it holds the current space factor used in
spacing calculations. In math mode, |aux| is also known as |incompleat_noad|;
if not |null|, it points to a record that represents the numerator of a
generalized fraction for which the denominator is currently being formed
in the current list.

There is also a sixth quantity, |mode_line|, which correlates
the semantic nest with the user's input; |mode_line| contains the source
line number at which the current level of nesting was entered. The negative
of this line number is the |mode_line| at the level of the
user's output routine.

A seventh quantity, |eTeX_aux|, is used by the extended features eTeX.
In math mode it is known as |delim_ptr| and points to the most
recent |fence_noad|  of a |math_left_group|.

In horizontal mode, the |prev_graf| field is used for initial language data.

The semantic nest is an array called |nest| that holds the |mode|, |head|,
|tail|, |prev_graf|, |aux|, and |mode_line| values for all semantic levels
below the currently active one. Information about the currently active
level is kept in the global quantities |mode|, |head|, |tail|, |prev_graf|,
|aux|, and |mode_line|, which live in a struct that is ready to
be pushed onto |nest| if necessary.

The math field is used by various bits and pieces in |texmath.w|

@ This implementation of
\TeX\ uses two different conventions for representing sequential stacks.
@^stack conventions@>@^conventions for representing stacks@>

\yskip\hang 1) If there is frequent access to the top entry, and if the
stack is essentially never empty, then the top entry is kept in a global
variable (even better would be a machine register), and the other entries
appear in the array $\\{stack}[0\to(\\{ptr}-1)]$. The
semantic stack is handled this way.

\yskip\hang 2) If there is infrequent top access, the entire stack contents
are in the array $\\{stack}[0\to(\\{ptr}-1)]$. For example, the |save_stack|
is treated this way, as we have seen.

@c
list_state_record *nest;
int nest_ptr;                   /* first unused location of |nest| */
int max_nest_stack;             /* maximum of |nest_ptr| when pushing */
int shown_mode;                 /* most recent mode shown by \.{\\tracingcommands} */
halfword save_tail;             /* save |tail| so we can examine whether we have an auto
                                   kern before a glue */

@ We will see later that the vertical list at the bottom semantic level is split
into two parts; the ``current page'' runs from |page_head| to |page_tail|,
and the ``contribution list'' runs from |contrib_head| to |tail| of
semantic level zero. The idea is that contributions are first formed in
vertical mode, then ``contributed'' to the current page (during which time
the page-breaking decisions are made). For now, we don't need to know
any more details about the page-building process.

@c
void initialize_nesting(void)
{
    nest_ptr = 0;
    max_nest_stack = 0;
    shown_mode = 0;
    cur_list.mode_field = vmode;
    cur_list.head_field = contrib_head;
    cur_list.tail_field = contrib_head;
    cur_list.eTeX_aux_field = null;
    cur_list.prev_depth_field = ignore_depth;
    cur_list.space_factor_field = 1000;
    cur_list.incompleat_noad_field = null;
    cur_list.ml_field = 0;
    cur_list.pg_field = 0;
    cur_list.dirs_field = null;
    init_math_fields();
}



@ Here is a common way to make the current list grow: 

@c
void tail_append(halfword p)
{
    couple_nodes(cur_list.tail_field, p);
    cur_list.tail_field = vlink(cur_list.tail_field);
}


@ @c
halfword pop_tail(void)
{
    halfword n, r;
    if (cur_list.tail_field != cur_list.head_field) {
        r = cur_list.tail_field;
        if (vlink(alink(cur_list.tail_field)) == cur_list.tail_field) {
            n = alink(cur_list.tail_field);
        } else {
            n = cur_list.head_field;
            while (vlink(n) != cur_list.tail_field)
                n = vlink(n);
        }
        flush_node(cur_list.tail_field);
        cur_list.tail_field = n;
        vlink(n) = null;
        return r;
    } else {
        return null;
    }
}

@ When \TeX's work on one level is interrupted, the state is saved by
calling |push_nest|. This routine changes |head| and |tail| so that
a new (empty) list is begun; it does not change |mode| or |aux|.

@c
void push_nest(void)
{                               /* enter a new semantic level, save the old */
    if (nest_ptr > max_nest_stack) {
        max_nest_stack = nest_ptr;
        if (nest_ptr == nest_size)
            overflow("semantic nest size", (unsigned) nest_size);
    }
    incr(nest_ptr);
    cur_list.mode_field = nest[nest_ptr - 1].mode_field;
    cur_list.head_field = new_node(temp_node, 0);
    cur_list.tail_field = cur_list.head_field;
    cur_list.eTeX_aux_field = null;
    cur_list.ml_field = line;
    cur_list.pg_field = 0;
    cur_list.dirs_field = null;
    cur_list.prev_depth_field = nest[nest_ptr - 1].prev_depth_field;
    cur_list.space_factor_field = nest[nest_ptr - 1].space_factor_field;
    cur_list.incompleat_noad_field = nest[nest_ptr - 1].incompleat_noad_field;
    init_math_fields();
}


@ Conversely, when \TeX\ is finished on the current level, the former
state is restored by calling |pop_nest|. This routine will never be
called at the lowest semantic level, nor will it be called unless |head|
is a node that should be returned to free memory.

@c
void pop_nest(void)
{                               /* leave a semantic level, re-enter the old */
    flush_node(cur_list.head_field);
    decr(nest_ptr);
}

@ Here is a procedure that displays what \TeX\ is working on, at all levels. 

@c
void show_activities(void)
{
    int p;                      /* index into |nest| */
    int m;                      /* mode */
    halfword q, r;              /* for showing the current page */
    int t;                      /* ditto */
    tprint_nl("");
    print_ln();
    for (p = nest_ptr; p >= 0; p--) {
        m = nest[p].mode_field;
        tprint_nl("### ");
        print_mode(m);
        tprint(" entered at line ");
        print_int(abs(nest[p].ml_field));
        /* we dont do this any more */
#if 0
        
           if (m == hmode)
           if (nest[p].pg_field != 040600000) {
           tprint(" (language");
           print_int(nest[p].pg_field % 0200000);
           tprint(":hyphenmin");
           print_int(nest[p].pg_field / 020000000);
           print_char(',');
           print_int((nest[p].pg_field / 0200000) % 0100);
           print_char(')');
           }
#endif
        if (nest[p].ml_field < 0)
            tprint(" (\\output routine)");
        if (p == 0) {
            /* Show the status of the current page */
            if (page_head != page_tail) {
                tprint_nl("### current page:");
                if (output_active)
                    tprint(" (held over for next output)");
                show_box(vlink(page_head));
                if (page_contents > empty) {
                    tprint_nl("total height ");
                    print_totals();
                    tprint_nl(" goal height ");
                    print_scaled(page_goal);
                    r = vlink(page_ins_head);
                    while (r != page_ins_head) {
                        print_ln();
                        tprint_esc("insert");
                        t = subtype(r);
                        print_int(t);
                        tprint(" adds ");
                        if (count(t) == 1000)
                            t = height(r);
                        else
                            t = x_over_n(height(r), 1000) * count(t);
                        print_scaled(t);
                        if (type(r) == split_up_node) {
                            q = page_head;
                            t = 0;
                            do {
                                q = vlink(q);
                                if ((type(q) == ins_node)
                                    && (subtype(q) == subtype(r)))
                                    incr(t);
                            } while (q != broken_ins(r));
                            tprint(", #");
                            print_int(t);
                            tprint(" might split");
                        }
                        r = vlink(r);
                    }
                }
            }
            if (vlink(contrib_head) != null)
                tprint_nl("### recent contributions:");
        }
        show_box(vlink(nest[p].head_field));
        /* Show the auxiliary field, |a| */
        switch (abs(m) / (max_command_cmd + 1)) {
        case 0:
            tprint_nl("prevdepth ");
            if (nest[p].prev_depth_field <= ignore_depth)
                tprint("ignored");
            else
                print_scaled(nest[p].prev_depth_field);
            if (nest[p].pg_field != 0) {
                tprint(", prevgraf ");
                print_int(nest[p].pg_field);
                if (nest[p].pg_field != 1)
                    tprint(" lines");
                else
                    tprint(" line");
            }
            break;
        case 1:
            tprint_nl("spacefactor ");
            print_int(nest[p].space_factor_field);
            /* we dont do this any more, this was aux.rh originally */
#if 0
               if (m > 0) {
               if (nest[p].current_language_field > 0) {
               tprint(", current language ");
               print_int(nest[p].current_language_field);
               }
               }
#endif
            break;
        case 2:
            if (nest[p].incompleat_noad_field != null) {
                tprint("this will be denominator of:");
                show_box(nest[p].incompleat_noad_field);
            }
        }                       /* there are no other cases */

    }
}
