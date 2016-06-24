% align.w
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

\def\<#1>{$#1$}

@ @c


#include "ptexlib.h"

@ @c
void fin_align(void);
void init_row(void);
void init_col(void);

#define noDEBUG

#define end_template_token (cs_token_flag+frozen_end_template)

#define prev_depth      cur_list.prev_depth_field
#define space_factor    cur_list.space_factor_field
#define incompleat_noad cur_list.incompleat_noad_field

#define every_cr          equiv(every_cr_loc)
#define display_indent    dimen_par(display_indent_code)
#define max_depth         dimen_par(max_depth_code)
#define overfull_rule     dimen_par(overfull_rule_code)

@ It's sort of a miracle whenever \.{\\halign} and \.{\\valign} work, because
they cut across so many of the control structures of \TeX.

Therefore the present page is probably not the best place for a beginner to
start reading this program; it is better to master everything else first.

Let us focus our thoughts on an example of what the input might be, in order
to get some idea about how the alignment miracle happens. The example doesn't
do anything useful, but it is sufficiently general to indicate all of the
special cases that must be dealt with; please do not be disturbed by its
apparent complexity and meaninglessness.
$$\vbox{\halign{\.{#}\hfil\cr
{}\\tabskip 2pt plus 3pt\cr
{}\\halign to 300pt\{u1\#v1\&\cr
\hskip 50pt\\tabskip 1pt plus 1fil u2\#v2\&\cr
\hskip 50pt u3\#v3\\cr\cr
\hskip 25pt a1\&\\omit a2\&\\vrule\\cr\cr
\hskip 25pt \\noalign\{\\vskip 3pt\}\cr
\hskip 25pt b1\\span b2\\cr\cr
\hskip 25pt \\omit\&c2\\span\\omit\\cr\}\cr}}$$
Here's what happens:

\yskip
(0) When `\.{\\halign to 300pt\{}' is scanned, the |scan_spec| routine
places the 300pt dimension onto the |save_stack|, and an |align_group|
code is placed above it. This will make it possible to complete the alignment
when the matching `\.\}' is found.

(1) The preamble is scanned next. Macros in the preamble are not expanded,
@^preamble@>
except as part of a tabskip specification. For example, if \.{u2} had been
a macro in the preamble above, it would have been expanded, since \TeX\
must look for `\.{minus...}' as part of the tabskip glue. A ``preamble list''
is constructed based on the user's preamble; in our case it contains the
following seven items:
$$\vbox{\halign{\.{#}\hfil\qquad&(#)\hfil\cr
{}\\glue 2pt plus 3pt&the tabskip preceding column 1\cr
{}\\alignrecord, width $-\infty$&preamble info for column 1\cr
{}\\glue 2pt plus 3pt&the tabskip between columns 1 and 2\cr
{}\\alignrecord, width $-\infty$&preamble info for column 2\cr
{}\\glue 1pt plus 1fil&the tabskip between columns 2 and 3\cr
{}\\alignrecord, width $-\infty$&preamble info for column 3\cr
{}\\glue 1pt plus 1fil&the tabskip following column 3\cr}}$$
These ``alignrecord'' entries have the same size as an |unset_node|,
since they will later be converted into such nodes. These alignrecord
nodes have no |depth| field; this is split into |u_part| and |v_part|,
and they point to token lists for the templates of the alignment. For
example, the |u_part| field in the first alignrecord points to the
token list `\.{u1}', i.e., the template preceding the `\.\#' for
column~1.  Furthermore, They have a |span_ptr| instead of a |node_attr|
field, and these |span_ptr| fields are initially set to the value
|end_span|, for reasons explained below.

(2) \TeX\ now looks at what follows the \.{\\cr} that ended the preamble.
It is not `\.{\\noalign}' or `\.{\\omit}', so this input is put back to
be read again, and the template `\.{u1}' is fed to the scanner. Just
before reading `\.{u1}', \TeX\ goes into restricted horizontal mode.
Just after reading `\.{u1}', \TeX\ will see `\.{a1}', and then (when the
{\.\&} is sensed) \TeX\ will see `\.{v1}'. Then \TeX\ scans an |endv|
token, indicating the end of a column. At this point an |unset_node| is
created, containing the contents of the current hlist (i.e., `\.{u1a1v1}').
The natural width of this unset node replaces the |width| field of the
alignrecord for column~1; in general, the alignrecords will record the
maximum natural width that has occurred so far in a given column.

(3) Since `\.{\\omit}' follows the `\.\&', the templates for column~2
are now bypassed. Again \TeX\ goes into restricted horizontal mode and
makes an |unset_node| from the resulting hlist; but this time the
hlist contains simply `\.{a2}'. The natural width of the new unset box
is remembered in the |width| field of the alignrecord for column~2.

(4) A third |unset_node| is created for column 3, using essentially the
mechanism that worked for column~1; this unset box contains `\.{u3\\vrule
v3}'. The vertical rule in this case has running dimensions that will later
extend to the height and depth of the whole first row, since each |unset_node|
in a row will eventually inherit the height and depth of its enclosing box.

(5) The first row has now ended; it is made into a single unset box
comprising the following seven items:
$$\vbox{\halign{\hbox to 325pt{\qquad\.{#}\hfil}\cr
{}\\glue 2pt plus 3pt\cr
{}\\unsetbox for 1 column: u1a1v1\cr
{}\\glue 2pt plus 3pt\cr
{}\\unsetbox for 1 column: a2\cr
{}\\glue 1pt plus 1fil\cr
{}\\unsetbox for 1 column: u3\\vrule v3\cr
{}\\glue 1pt plus 1fil\cr}}$$
The width of this unset row is unimportant, but it has the correct height
and depth, so the correct baselineskip glue will be computed as the row
is inserted into a vertical list.

(6) Since `\.{\\noalign}' follows the current \.{\\cr}, \TeX\ appends
additional material (in this case \.{\\vskip 3pt}) to the vertical list.
While processing this material, \TeX\ will be in internal vertical
mode, and |no_align_group| will be on |save_stack|.

(7) The next row produces an unset box that looks like this:
$$\vbox{\halign{\hbox to 325pt{\qquad\.{#}\hfil}\cr
{}\\glue 2pt plus 3pt\cr
{}\\unsetbox for 2 columns: u1b1v1u2b2v2\cr
{}\\glue 1pt plus 1fil\cr
{}\\unsetbox for 1 column: {\rm(empty)}\cr
{}\\glue 1pt plus 1fil\cr}}$$
The natural width of the unset box that spans columns 1~and~2 is stored
in a ``span node,'' which we will explain later; the |span_ptr| field of the
alignrecord for column~1 now points to the new span node, and the |span_ptr|
of the span node points to |end_span|.

(8) The final row produces the unset box
$$\vbox{\halign{\hbox to 325pt{\qquad\.{#}\hfil}\cr
{}\\glue 2pt plus 3pt\cr
{}\\unsetbox for 1 column: {\rm(empty)}\cr
{}\\glue 2pt plus 3pt\cr
{}\\unsetbox for 2 columns: u2c2v2\cr
{}\\glue 1pt plus 1fil\cr}}$$
A new span node is attached to the alignrecord for column 2.

(9) The last step is to compute the true column widths and to change all the
unset boxes to hboxes, appending the whole works to the vertical list that
encloses the \.{\\halign}. The rules for deciding on the final widths of
each unset column box will be explained below.

\yskip\noindent
Note that as \.{\\halign} is being processed, we fearlessly give up control
to the rest of \TeX. At critical junctures, an alignment routine is
called upon to step in and do some little action, but most of the time
these routines just lurk in the background. It's something like
post-hypnotic suggestion.

@ We have mentioned that alignrecords contain no |height| or |depth| fields.
Their |glue_sign| and |glue_order| are pre-empted as well, since it
is necessary to store information about what to do when a template ends.
This information is called the |extra_info| field.

@c
#define u_part(A) vlink((A)+depth_offset)       /* pointer to \<u_j> token list */
#define v_part(A) vinfo((A)+depth_offset)       /* pointer to \<v_j> token list */
#define span_ptr(A) vinfo((A)+1)        /* column spanning list */
#define extra_info(A) vinfo((A)+list_offset)    /* info to remember during template */

@ Alignments can occur within alignments, so a small stack is used to access
the alignrecord information. At each level we have a |preamble| pointer,
indicating the beginning of the preamble list; a |cur_align| pointer,
indicating the current position in the preamble list; a |cur_span| pointer,
indicating the value of |cur_align| at the beginning of a sequence of
spanned columns; a |cur_loop| pointer, indicating the tabskip glue before
an alignrecord that should be copied next if the current list is extended;
and the |align_state| variable, which indicates the nesting of braces so
that \.{\\cr} and \.{\\span} and tab marks are properly intercepted.
There also are pointers |cur_head| and |cur_tail| to the head and tail
of a list of adjustments being moved out from horizontal mode to
vertical~mode, and alike |cur_pre_head| and |cur_pre_tail| for pre-adjust
lists.

The current values of these nine quantities appear in global variables;
when they have to be pushed down, they are stored in 6-word nodes, and
|align_ptr| points to the topmost such node.

@c
#define preamble vlink(align_head)      /* the current preamble list */

pointer cur_align = null;       /* current position in preamble list */
pointer cur_span = null;        /* start of currently spanned columns in preamble list */
pointer cur_loop = null;        /* place to copy when extending a periodic preamble */
pointer align_ptr = null;       /* most recently pushed-down alignment stack node */
pointer cur_head = null, cur_tail = null;       /* adjustment list pointers */
pointer cur_pre_head = null, cur_pre_tail = null;       /* pre-adjustment list pointers */

/* The |align_state| and |preamble| variables are initialized elsewhere. */

@ Alignment stack maintenance is handled by a pair of trivial routines
called |push_alignment| and |pop_alignment|.

(HH:) It makes not much sense to add support for an \.{attr} keyword to
\.{\\halign} and \.{\\valign} because then we need to decide if we tag
rows or cells or both or come up with \.{cellattr} and \.{rowattr} and
such. But then it even makes sense to have explicit commands (in addition
to the seperator) to tags individual cells. Too muss hassle for now and the
advantages are not that large.

@c
static void push_alignment(void)
{
    pointer p;                  /* the new alignment stack node */
    p = new_node(align_stack_node, 0);
    vinfo(p + 1) = align_ptr;
    vlink(p + 1) = cur_align;
    vinfo(p + 2) = preamble;
    vlink(p + 2) = cur_span;
    vinfo(p + 3) = cur_loop;
    vlink(p + 3) = align_state;
    vinfo(p + 4) = cur_head;
    vlink(p + 4) = cur_tail;
    vinfo(p + 5) = cur_pre_head;
    vlink(p + 5) = cur_pre_tail;
    align_ptr = p;
    cur_head = new_node(temp_node, 0);
    cur_pre_head = new_node(temp_node, 0);
}

static void pop_alignment(void)
{
    pointer p;                  /* the top alignment stack node */
    flush_node(cur_head);
    flush_node(cur_pre_head);
    p = align_ptr;
    cur_pre_tail = vlink(p + 5);
    cur_pre_head = vinfo(p + 5);
    cur_tail = vlink(p + 4);
    cur_head = vinfo(p + 4);
    align_state = vlink(p + 3);
    cur_loop = vinfo(p + 3);
    cur_span = vlink(p + 2);
    preamble = vinfo(p + 2);
    cur_align = vlink(p + 1);
    align_ptr = vinfo(p + 1);
    flush_node(p);
}


@ \TeX\ has eight procedures that govern alignments: |init_align| and
|fin_align| are used at the very beginning and the very end; |init_row| and
|fin_row| are used at the beginning and end of individual rows; |init_span|
is used at the beginning of a sequence of spanned columns (possibly involving
only one column); |init_col| and |fin_col| are used at the beginning and
end of individual columns; and |align_peek| is used after \.{\\cr} to see
whether the next item is \.{\\noalign}.

We shall consider these routines in the order they are first used during
the course of a complete \.{\\halign}, namely |init_align|, |align_peek|,
|init_row|, |init_span|, |init_col|, |fin_col|, |fin_row|, |fin_align|.


@ The preamble is copied directly, except that \.{\\tabskip} causes a change
to the tabskip glue, thereby possibly expanding macros that immediately
follow it. An appearance of \.{\\span} also causes such an expansion.

Note that if the preamble contains `\.{\\global\\tabskip}', the `\.{\\global}'
token survives in the preamble and the `\.{\\tabskip}' defines new
tabskip glue (locally).

@c
static void get_preamble_token(void)
{
  RESTART:
    get_token();
    while ((cur_chr == span_code) && (cur_cmd == tab_mark_cmd)) {
        get_token();            /* this token will be expanded once */
        if (cur_cmd > max_command_cmd) {
            expand();
            get_token();
        }
    }
    if (cur_cmd == endv_cmd)
        fatal_error("(interwoven alignment preambles are not allowed)");
    if ((cur_cmd == assign_glue_cmd)
        && (cur_chr == glue_base + tab_skip_code)) {
        scan_optional_equals();
        scan_glue(glue_val_level);
        if (int_par(global_defs_code) > 0)
            geq_define(glue_base + tab_skip_code, glue_ref_cmd, cur_val);
        else
            eq_define(glue_base + tab_skip_code, glue_ref_cmd, cur_val);
        goto RESTART;
    }
}



@ When \.{\\halign} or \.{\\valign} has been scanned in an appropriate
mode, \TeX\ calls |init_align|, whose task is to get everything off to a
good start. This mostly involves scanning the preamble and putting its
information into the preamble list.
@^preamble@>

@c
void init_align(void)
{
    /* label done, done1, done2, continue; */
    pointer save_cs_ptr;        /* |warning_index| value for error messages */
    pointer p, r;               /* for short-term temporary use */
    save_cs_ptr = cur_cs;       /* \.{\\halign} or \.{\\valign}, usually */
    push_alignment();
    align_state = -1000000;     /* enter a new alignment level */

    /* When \.{\\halign} is used as a displayed formula, there should be
       no other pieces of mlists present. */

    if ((cur_list.mode_field == mmode)
        && ((cur_list.tail_field != cur_list.head_field)
            || (incompleat_noad != null))) {
        const char *hlp[] =
            { "Displays can use special alignments (like \\eqalignno)",
            "only if nothing but the alignment itself is between $$'s.",
            "So I've deleted the formulas that preceded this alignment.",
            NULL
        };
        tex_error("Improper \\halign inside $$'s", hlp);
        flush_math();
    }
    push_nest();                /* enter a new semantic level */
    /* In vertical modes, |prev_depth| already has the correct value. But
       if we are in |mmode| (displayed formula mode), we reach out to the
       enclosing vertical mode for the |prev_depth| value that produces the
       correct baseline calculations. */
    if (cur_list.mode_field == mmode) {
        cur_list.mode_field = -vmode;
        prev_depth = nest[nest_ptr - 2].prev_depth_field;
    } else if (cur_list.mode_field > 0) {
        cur_list.mode_field = -(cur_list.mode_field);
    }
    scan_spec(align_group);
    /* Scan the preamble */
    preamble = null;
    cur_align = align_head;
    cur_loop = null;
    scanner_status = aligning;
    warning_index = save_cs_ptr;
    align_state = -1000000;
    /* at this point, |cur_cmd=left_brace| */
    while (true) {
        /* Append the current tabskip glue to the preamble list */
        r = new_param_glue(tab_skip_code);
        vlink(cur_align) = r;
        cur_align = vlink(cur_align);

        if (cur_cmd == car_ret_cmd)
            break;              /* \.{\\cr} ends the preamble */

        /* Scan preamble text until |cur_cmd| is |tab_mark| or |car_ret| */
        /* Scan the template \<u_j>, putting the resulting token list in |hold_token_head| */
        /* Spaces are eliminated from the beginning of a template. */

        p = hold_token_head;
        token_link(p) = null;
        while (1) {
            get_preamble_token();
            if (cur_cmd == mac_param_cmd)
                break;
            if ((cur_cmd <= car_ret_cmd) && (cur_cmd >= tab_mark_cmd)
                && (align_state == -1000000)) {
                if ((p == hold_token_head) && (cur_loop == null)
                    && (cur_cmd == tab_mark_cmd)) {
                    cur_loop = cur_align;
                } else {
                    const char *hlp[] =
                        { "There should be exactly one # between &'s, when an",
                        "\\halign or \\valign is being set up. In this case you had",
                        "none, so I've put one in; maybe that will work.",
                        NULL
                    };
                    back_input();
                    tex_error("Missing # inserted in alignment preamble", hlp);
                    break;
                }
            } else if ((cur_cmd != spacer_cmd) || (p != hold_token_head)) {
                r = get_avail();
                token_link(p) = r;
                p = token_link(p);
                token_info(p) = cur_tok;
            }
        }
        r = new_node(align_record_node, 0);
        vlink(cur_align) = r;
        cur_align = vlink(cur_align);   /* a new alignrecord */
        span_ptr(cur_align) = end_span;
        width(cur_align) = null_flag;
        u_part(cur_align) = token_link(hold_token_head);
        /* Scan the template \<v_j>, putting the resulting token list in |hold_token_head| */

        p = hold_token_head;
        token_link(p) = null;
        while (1) {
          CONTINUE:
            get_preamble_token();
            if ((cur_cmd <= car_ret_cmd) && (cur_cmd >= tab_mark_cmd)
                && (align_state == -1000000))
                break;
            if (cur_cmd == mac_param_cmd) {
                const char *hlp[] =
                    { "There should be exactly one # between &'s, when an",
                    "\\halign or \\valign is being set up. In this case you had",
                    "more than one, so I'm ignoring all but the first.",
                    NULL
                };
                tex_error("Only one # is allowed per tab", hlp);
                goto CONTINUE;
            }
            r = get_avail();
            token_link(p) = r;
            p = token_link(p);
            token_info(p) = cur_tok;
        }
        r = get_avail();
        token_link(p) = r;
        p = token_link(p);
        token_info(p) = end_template_token;     /* put \.{\\endtemplate} at the end */

        v_part(cur_align) = token_link(hold_token_head);
    }
    scanner_status = normal;

    new_save_level(align_group);
    if (every_cr != null)
        begin_token_list(every_cr, every_cr_text);
    align_peek();               /* look for \.{\\noalign} or \.{\\omit} */
}


@ The tricky part about alignments is getting the templates into the
scanner at the right time, and recovering control when a row or column
is finished.

We usually begin a row after each \.{\\cr} has been sensed, unless that
\.{\\cr} is followed by \.{\\noalign} or by the right brace that terminates
the alignment. The |align_peek| routine is used to look ahead and do
the right thing; it either gets a new row started, or gets a \.{\\noalign}
started, or finishes off the alignment.

@c
void align_peek(void)
{
  RESTART:
    align_state = 1000000;
    do {
        get_x_or_protected();
    } while (cur_cmd == spacer_cmd);
    if (cur_cmd == no_align_cmd) {
        scan_left_brace();
        new_save_level(no_align_group);
        if (cur_list.mode_field == -vmode)
            normal_paragraph();
    } else if (cur_cmd == right_brace_cmd) {
        fin_align();
    } else if ((cur_cmd == car_ret_cmd) && (cur_chr == cr_cr_code)) {
        goto RESTART;           /* ignore \.{\\crcr} */
    } else {
        init_row();             /* start a new row */
        init_col();             /* start a new column and replace what we peeked at */
    }
}


@ The parameter to |init_span| is a pointer to the alignrecord where the
next column or group of columns will begin. A new semantic level is
entered, so that the columns will generate a list for subsequent packaging.

@c
static void init_span(pointer p)
{
    push_nest();
    if (cur_list.mode_field == -hmode) {
        space_factor = 1000;
    } else {
        prev_depth = ignore_depth;
        normal_paragraph();
    }
    cur_span = p;
}


@ To start a row (i.e., a `row' that rhymes with `dough' but not with `bough'),
we enter a new semantic level, copy the first tabskip glue, and change
from internal vertical mode to restricted horizontal mode or vice versa.
The |space_factor| and |prev_depth| are not used on this semantic level,
but we clear them to zero just to be tidy.

@c
void init_row(void)
{
    push_nest();
    cur_list.mode_field = (-hmode - vmode) - cur_list.mode_field;
    if (cur_list.mode_field == -hmode)
        space_factor = 0;
    else
        prev_depth = 0;
    tail_append(new_glue(preamble));
    subtype(cur_list.tail_field) = tab_skip_code + 1;
    cur_align = vlink(preamble);
    cur_tail = cur_head;
    cur_pre_tail = cur_pre_head;
    init_span(cur_align);
}


@ When a column begins, we assume that |cur_cmd| is either |omit| or else
the current token should be put back into the input until the \<u_j>
template has been scanned.  (Note that |cur_cmd| might be |tab_mark| or
|car_ret|.)  We also assume that |align_state| is approximately 1000000 at
this time.  We remain in the same mode, and start the template if it is
called for.

@c
void init_col(void)
{
    extra_info(cur_align) = cur_cmd;
    if (cur_cmd == omit_cmd)
        align_state = 0;
    else {
        back_input();
        begin_token_list(u_part(cur_align), u_template);
    }                           /* now |align_state=1000000| */
}


@ The scanner sets |align_state| to zero when the \<u_j> template ends. When
a subsequent \.{\\cr} or \.{\\span} or tab mark occurs with |align_state=0|,
the scanner activates the following code, which fires up the \<v_j> template.
We need to remember the |cur_chr|, which is either |cr_cr_code|, |cr_code|,
|span_code|, or a character code, depending on how the column text has ended.

This part of the program had better not be activated when the preamble
to another alignment is being scanned, or when no alignment preamble is active.

@c
void insert_vj_template(void)
{
    if ((scanner_status == aligning) || (cur_align == null))
        fatal_error("(interwoven alignment preambles are not allowed)");
    cur_cmd = extra_info(cur_align);
    extra_info(cur_align) = cur_chr;
    if (cur_cmd == omit_cmd)
        begin_token_list(omit_template, v_template);
    else
        begin_token_list(v_part(cur_align), v_template);
    align_state = 1000000;
}

/* Determine the stretch order */
#define determine_stretch_order() do {              \
        if (total_stretch[filll]!=0)  o=filll;      \
        else if (total_stretch[fill]!=0)  o=fill;   \
        else if (total_stretch[fil]!=0)  o=fil;     \
        else if (total_stretch[sfi]!=0)  o=sfi;     \
        else o=normal;                              \
    } while (0)


/* Determine the shrink order */
#define determine_shrink_order() do {              \
        if (total_shrink[filll]!=0)  o=filll;      \
        else if (total_shrink[fill]!=0)  o=fill;   \
        else if (total_shrink[fil]!=0)  o=fil;     \
        else if (total_shrink[sfi]!=0)  o=sfi;     \
        else o=normal;                              \
    } while (0)



@ When the |endv| command at the end of a \<v_j> template comes through the
scanner, things really start to happen; and it is the |fin_col| routine
that makes them happen. This routine returns |true| if a row as well as a
column has been finished.

@c
boolean fin_col(void)
{
    pointer p;                  /* the alignrecord after the current one */
    pointer q, r;               /* temporary pointers for list manipulation */
    pointer s;                  /* a new span node */
    pointer u;                  /* a new unset box */
    scaled w;                   /* natural width */
    unsigned char o;            /* order of infinity */
    halfword n;                 /* span counter */
    if (cur_align == null)
        confusion("endv");
    q = vlink(cur_align);
    if (q == null)
        confusion("endv");
    if (align_state < 500000)
        fatal_error("(interwoven alignment preambles are not allowed)");
    p = vlink(q);
    /* If the preamble list has been traversed, check that the row has ended */
    if ((p == null) && (extra_info(cur_align) < cr_code)) {
        if (cur_loop != null) {
            /* Lengthen the preamble periodically */
            r = new_node(align_record_node, 0);
            vlink(q) = r;
            p = vlink(q);       /* a new alignrecord */
            span_ptr(p) = end_span;
            width(p) = null_flag;
            cur_loop = vlink(cur_loop);

            /* Copy the templates from node |cur_loop| into node |p| */
            q = hold_token_head;
            r = u_part(cur_loop);
            while (r != null) {
                s = get_avail();
                token_link(q) = s;
                q = token_link(q);
                token_info(q) = token_info(r);
                r = token_link(r);
            }
            token_link(q) = null;
            u_part(p) = token_link(hold_token_head);
            q = hold_token_head;
            r = v_part(cur_loop);
            while (r != null) {
                s = get_avail();
                token_link(q) = s;
                q = token_link(q);
                token_info(q) = token_info(r);
                r = token_link(r);
            }
            token_link(q) = null;
            v_part(p) = token_link(hold_token_head);

            cur_loop = vlink(cur_loop);
            r = new_glue(cur_loop);
            vlink(p) = r;
        } else {
            const char *hlp[] =
                { "You have given more \\span or & marks than there were",
                "in the preamble to the \\halign or \\valign now in progress.",
                "So I'll assume that you meant to type \\cr instead.",
                NULL
            };
            extra_info(cur_align) = cr_code;
            tex_error("Extra alignment tab has been changed to \\cr", hlp);
        }
    }
    if (extra_info(cur_align) != span_code) {
        unsave();
        new_save_level(align_group);
        /* Package an unset box for the current column and record its width */
        if (cur_list.mode_field == -hmode) {
            adjust_tail = cur_tail;
            pre_adjust_tail = cur_pre_tail;
            u = filtered_hpack(cur_list.head_field, cur_list.tail_field, 0,
                               additional, align_set_group, -1, 0, 0);
            w = width(u);
            cur_tail = adjust_tail;
            adjust_tail = null;
            cur_pre_tail = pre_adjust_tail;
            pre_adjust_tail = null;
        } else {
            u = filtered_vpackage(vlink(cur_list.head_field),
                0, additional, 0, align_set_group, -1, 0, 0);
            w = height(u);
        }
        n = min_quarterword;    /* this represents a span count of 1 */
        if (cur_span != cur_align) {
            /* Update width entry for spanned columns */
            q = cur_span;
            do {
                incr(n);
                q = vlink(vlink(q));
            } while (q != cur_align);
            if (n > max_quarterword)
                confusion("too many spans");    /* this can happen, but won't */
            q = cur_span;
            while (span_span(span_ptr(q)) < n) {
                q = span_ptr(q);
            }
            if (span_span(span_ptr(q)) > n) {
                s = new_span_node(span_ptr(q), n, w);
                span_ptr(q) = s;
            } else if (width(span_ptr(q)) < w) {
                width(span_ptr(q)) = w;
            }

        } else if (w > width(cur_align)) {
            width(cur_align) = w;
        }
        type(u) = unset_node;
        span_count(u) = (quarterword) n;
        determine_stretch_order();
        glue_order(u) = o;
        glue_stretch(u) = total_stretch[o];
        determine_shrink_order();
        glue_sign(u) = o;
        glue_shrink(u) = total_shrink[o];
        pop_nest();
        vlink(cur_list.tail_field) = u;
        cur_list.tail_field = u;

        /* Copy the tabskip glue between columns */
        tail_append(new_glue(vlink(cur_align)));
        subtype(cur_list.tail_field) = tab_skip_code + 1;

        if (extra_info(cur_align) >= cr_code) {
            return true;
        }
        init_span(p);
    }
    align_state = 1000000;
    do {
        get_x_or_protected();
    } while (cur_cmd == spacer_cmd);
    cur_align = p;
    init_col();
    return false;
}



@ A span node is a 3-word record containing |width|, |span_span|, and
|span_ptr| fields. The |span_span| field indicates the number of
spanned columns; the |span_ptr| field points to a span node for the same
starting column, having a greater extent of spanning, or to
|end_span|, which has the largest possible |span_span| field; the |width|
field holds the largest natural width corresponding to a particular
set of spanned columns.

A list of the maximum widths so far, for spanned columns starting at a
given column, begins with the |span_ptr| field of the alignrecord for
that column. The code has to make sure that there is room for
|span_ptr| in both the alignrecord and the span nodes, which is why
|span_ptr| replaces |node_attr|.
@^data structure assumptions@>

The |new_span_node| function is defined in |texnodes.c|.

@c
#ifndef span_span
#  define span_span(A) vlink((A)+1)     /* that is normally |alink| */
#endif


@ At the end of a row, we append an unset box to the current vlist (for
\.{\\halign}) or the current hlist (for \.{\\valign}). This unset box
contains the unset boxes for the columns, separated by the tabskip glue.
Everything will be set later.

@c
void fin_row(void)
{
    pointer p;                  /* the new unset box */
    if (cur_list.mode_field == -hmode) {
        p = filtered_hpack(cur_list.head_field, cur_list.tail_field, 0,
                           additional, fin_row_group, -1, 0, 0);
        pop_nest();
        if (cur_pre_head != cur_pre_tail)
            append_list(cur_pre_head, cur_pre_tail);
        append_to_vlist(p,lua_key_index(alignment));
        if (cur_head != cur_tail)
            append_list(cur_head, cur_tail);
    } else {
        p = filtered_vpackage(vlink(cur_list.head_field),
            0, additional, max_depth, fin_row_group, -1, 0, 0);
        pop_nest();
        vlink(cur_list.tail_field) = p;
        cur_list.tail_field = p;
        space_factor = 1000;
    }
    type(p) = unset_node;
    glue_stretch(p) = 0;
    if (every_cr != null)
        begin_token_list(every_cr, every_cr_text);
    align_peek();
    /* note that |glue_shrink(p)=0| since |glue_shrink==shift_amount| */
}


@ Finally, we will reach the end of the alignment, and we can breathe a
sigh of relief that memory hasn't overflowed. All the unset boxes will now be
set so that the columns line up, taking due account of spanned columns.

@c
void fin_align(void)
{
    pointer p, q, r, s, u, rr;  /* registers for the list operations */
    scaled t, w;                /* width of column */
    scaled o;                   /* shift offset for unset boxes */
    halfword n;                 /* matching span amount */
    scaled rule_save;           /* temporary storage for |overfull_rule| */
    halfword pd;                /* temporary storage for |prev_depth| */
    halfword ng;                /*  temporary storage for |new_glue| */
    if (cur_group != align_group)
        confusion("align1");
    unsave();                   /* that |align_group| was for individual entries */
    if (cur_group != align_group)
        confusion("align0");
    unsave();                   /* that |align_group| was for the whole alignment */
    if (nest[nest_ptr - 1].mode_field == mmode)
        o = display_indent;
    else
        o = 0;
    /* Go through the preamble list, determining the column widths and
     * changing the alignrecords to dummy unset boxes
     */

/* It's time now to dismantle the preamble list and to compute the column
widths. Let $w_{ij}$ be the maximum of the natural widths of all entries
that span columns $i$ through $j$, inclusive. The alignrecord for column~$i$
contains $w_{ii}$ in its |width| field, and there is also a linked list of
the nonzero $w_{ij}$ for increasing $j$, accessible via the |info| field;
these span nodes contain the value $j-i+|min_quarterword|$ in their
|link| fields. The values of $w_{ii}$ were initialized to |null_flag|, which
we regard as $-\infty$.

The final column widths are defined by the formula
$$w_j=\max_{1\L i\L j}\biggl( w_{ij}-\sum_{i\L k<j}(t_k+w_k)\biggr),$$
where $t_k$ is the natural width of the tabskip glue between columns
$k$ and~$k+1$. However, if $w_{ij}=-\infty$ for all |i| in the range
|1<=i<=j| (i.e., if every entry that involved column~|j| also involved
column~|j+1|), we let $w_j=0$, and we zero out the tabskip glue after
column~|j|.

\TeX\ computes these values by using the following scheme: First $w_1=w_{11}$.
Then replace $w_{2j}$ by $\max(w_{2j},w_{1j}-t_1-w_1)$, for all $j>1$.
Then $w_2=w_{22}$. Then replace $w_{3j}$ by $\max(w_{3j},w_{2j}-t_2-w_2)$
for all $j>2$; and so on. If any $w_j$ turns out to be $-\infty$, its
value is changed to zero and so is the next tabskip.
*/
    q = vlink(preamble);
    do {
        flush_list(u_part(q));
        flush_list(v_part(q));
        p = vlink(vlink(q));
        if (width(q) == null_flag) {
            /* Nullify |width(q)| and the tabskip glue following this column */
            width(q) = 0;
            r = vlink(q);
            reset_glue_to_zero(r); /* is a lready copy */ 
        }
        if (span_ptr(q) != end_span) {
            /* Merge the widths in the span nodes of |q| with those of |p|,
               destroying the span nodes of |q| */
            /*
               Merging of two span-node lists is a typical exercise in the manipulation of
               linearly linked data structures. The essential invariant in the following
               |repeat| loop is that we want to dispense with node |r|, in |q|'s list,
               and |u| is its successor; all nodes of |p|'s list up to and including |s|
               have been processed, and the successor of |s| matches |r| or precedes |r|
               or follows |r|, according as |link(r)=n| or |link(r)>n| or |link(r)<n|.
             */
            t = width(q) + width(vlink(q));
            r = span_ptr(q);
            s = end_span;
            span_ptr(s) = p;
            n = min_quarterword + 1;
            do {
                width(r) = width(r) - t;
                u = span_ptr(r);
                while (span_span(r) > n) {
                    s = span_ptr(s);
                    n = span_span(span_ptr(s)) + 1;
                }
                if (span_span(r) < n) {
                    span_ptr(r) = span_ptr(s);
                    span_ptr(s) = r;
                    decr(span_span(r));
                    s = r;
                } else {
                    if (width(r) > width(span_ptr(s)))
                        width(span_ptr(s)) = width(r);
                    flush_node(r);
                }
                r = u;
            } while (r != end_span);
        }
        type(q) = unset_node;
        span_count(q) = min_quarterword;
        height(q) = 0;
        depth(q) = 0;
        glue_order(q) = normal;
        glue_sign(q) = normal;
        glue_stretch(q) = 0;
        glue_shrink(q) = 0;
        q = p;
    } while (q != null);

    /* Package the preamble list, to determine the actual tabskip glue amounts,
       and let |p| point to this prototype box */
    /* Now the preamble list has been converted to a list of alternating unset
       boxes and tabskip glue, where the box widths are equal to the final
       column sizes. In case of \.{\\valign}, we change the widths to heights,
       so that a correct error message will be produced if the alignment is
       overfull or underfull.
     */

    decr(save_ptr);
    pack_begin_line = -cur_list.ml_field;
    if (cur_list.mode_field == -vmode) {
        rule_save = overfull_rule;
        overfull_rule = 0;      /* prevent rule from being packaged */
        p = hpack(preamble, saved_value(0), saved_level(0), -1);
        overfull_rule = rule_save;
    } else {
        q = vlink(preamble);
        do {
            height(q) = width(q);
            width(q) = 0;
            q = vlink(vlink(q));
        } while (q != null);
        p = filtered_vpackage(preamble,
            saved_value(0), saved_level(0), max_depth, preamble_group, -1, 0, 0);
        q = vlink(preamble);
        do {
            width(q) = height(q);
            height(q) = 0;
            q = vlink(vlink(q));
        } while (q != null);
    }
    pack_begin_line = 0;

    /* Set the glue in all the unset boxes of the current list */
    q = vlink(cur_list.head_field);
    s = cur_list.head_field;
    while (q != null) {
        if (!is_char_node(q)) {
            if (type(q) == unset_node) {
                /* Set the unset box |q| and the unset boxes in it */
                /* The unset box |q| represents a row that contains one or more unset boxes,
                   depending on how soon \.{\\cr} occurred in that row. */

                if (cur_list.mode_field == -vmode) {
                    type(q) = hlist_node;
                    subtype(q) = align_row_list;
                    width(q) = width(p);
                } else {
                    type(q) = vlist_node;
                    subtype(q) = align_row_list;
                    height(q) = height(p);
                }
                glue_order(q) = glue_order(p);
                glue_sign(q) = glue_sign(p);
                glue_set(q) = glue_set(p);
                shift_amount(q) = o;
                r = vlink(list_ptr(q));
                assert (type(r) == unset_node);
                s = vlink(list_ptr(p));
                do {
                    /* Set the glue in node |r| and change it from an unset node */
                    /* A box made from spanned columns will be followed by tabskip glue nodes and
                       by empty boxes as if there were no spanning. This permits perfect alignment
                       of subsequent entries, and it prevents values that depend on floating point
                       arithmetic from entering into the dimensions of any boxes.
                     */
                    n = span_count(r);
                    t = width(s);
                    w = t;
                    u = hold_head;
                    while (n > min_quarterword) {
                        decr(n);
                        /* Append tabskip glue and an empty box to list |u|,
                           and update |s| and |t| as the prototype nodes are passed */

                        s = vlink(s);
                        ng = new_glue(s);
                        vlink(u) = ng;
                        u = vlink(u);
                        subtype(u) = tab_skip_code + 1;
                        t = t + width(s);
                        if (glue_sign(p) == stretching) {
                            if (stretch_order(s) == glue_order(p))
                                t = t + round(float_cast(glue_set(p)) * float_cast(stretch(s)));
                        } else if (glue_sign(p) == shrinking) {
                            if (shrink_order(s) == glue_order(p))
                                t = t - round(float_cast(glue_set(p)) * float_cast(shrink(s)));
                        }
                        s = vlink(s);
                        rr = new_null_box();
                        vlink(u) = rr;
                        u = vlink(u);
                        t = t + width(s);
                        subtype(u) = align_cell_list;
                        if (cur_list.mode_field == -vmode) {
                            width(u) = width(s);
                        } else {
                            type(u) = vlist_node;
                            height(u) = width(s);
                        }

                    }
                    if (cur_list.mode_field == -vmode) {
                        /* Make the unset node |r| into an |hlist_node| of width |w|,
                           setting the glue as if the width were |t| */

                        height(r) = height(q);
                        depth(r) = depth(q);
                        if (t == width(r)) {
                            glue_sign(r) = normal;
                            glue_order(r) = normal;
                            set_glue_ratio_zero(glue_set(r));
                        } else if (t > width(r)) {
                            glue_sign(r) = stretching;
                            if (glue_stretch(r) == 0)
                                set_glue_ratio_zero(glue_set(r));
                            else
                                glue_set(r) =
                                    unfloat((double) (t - width(r)) /
                                            glue_stretch(r));
                        } else {
                            glue_order(r) = glue_sign(r);
                            glue_sign(r) = shrinking;
                            if (glue_shrink(r) == 0)
                                set_glue_ratio_zero(glue_set(r));
                            else if ((glue_order(r) == normal)
                                     && (width(r) - t > glue_shrink(r)))
                                set_glue_ratio_one(glue_set(r));
                            else
                                glue_set(r) =
                                    unfloat((double) (width(r) - t) /
                                            glue_shrink(r));
                        }
                        width(r) = w;
                        type(r) = hlist_node;
                        subtype(r) = align_cell_list;

                    } else {
                        /* Make the unset node |r| into a |vlist_node| of height |w|,
                           setting the glue as if the height were |t| */

                        width(r) = width(q);
                        if (t == height(r)) {
                            glue_sign(r) = normal;
                            glue_order(r) = normal;
                            set_glue_ratio_zero(glue_set(r));
                        } else if (t > height(r)) {
                            glue_sign(r) = stretching;
                            if (glue_stretch(r) == 0)
                                set_glue_ratio_zero(glue_set(r));
                            else
                                glue_set(r) =
                                    unfloat((t - height(r)) / glue_stretch(r));
                        } else {
                            glue_order(r) = glue_sign(r);
                            glue_sign(r) = shrinking;
                            if (glue_shrink(r) == 0)
                                set_glue_ratio_zero(glue_set(r));
                            else if ((glue_order(r) == normal)
                                     && (height(r) - t > glue_shrink(r)))
                                set_glue_ratio_one(glue_set(r));
                            else
                                glue_set(r) =
                                    unfloat((height(r) - t) / glue_shrink(r));
                        }
                        height(r) = w;
                        type(r) = vlist_node;
                        subtype(r) = align_cell_list;

                    }
                    /* subtype(r) = 0; */
                    shift_amount(r) = 0;
                    if (u != hold_head) {       /* append blank boxes to account for spanned nodes */
                        vlink(u) = vlink(r);
                        vlink(r) = vlink(hold_head);
                        r = u;
                    }

                    r = vlink(vlink(r));
                    s = vlink(vlink(s));
                } while (r != null);

            } else if (type(q) == rule_node) {
                /* Make the running dimensions in rule |q| extend to the
                   boundaries of the alignment */
                if (is_running(width(q)))
                    width(q) = width(p);
                if (is_running(height(q)))
                    height(q) = height(p);
                if (is_running(depth(q)))
                    depth(q) = depth(p);
                if (o != 0) {
                    r = vlink(q);
                    vlink(q) = null;
                    q = hpack(q, 0, additional, -1);
                    shift_amount(q) = o;
                    subtype(q) = align_cell_list;
                    vlink(q) = r;
                    vlink(s) = q;
                }
            }
        }
        s = q;
        q = vlink(q);
    }
    flush_node_list(p);
    pop_alignment();
    /* Insert the current list into its environment */
    /* We now have a completed alignment, in the list that starts at |cur_list.head_field|
       and ends at |cur_list.tail_field|. This list will be merged with the one that encloses
       it. (In case the enclosing mode is |mmode|, for displayed formulas,
       we will need to insert glue before and after the display; that part of the
       program will be deferred until we're more familiar with such operations.)
     */
    pd = cur_list.prev_depth_field;
    p = vlink(cur_list.head_field);
    q = cur_list.tail_field;
    pop_nest();
    if (cur_list.mode_field == mmode) {
        finish_display_alignment(p, q, pd);
    } else {
	cur_list.prev_depth_field = pd; /* aux:=aux_save; */
        vlink(cur_list.tail_field) = p;
        if (p != null)
            cur_list.tail_field = q;
        if (cur_list.mode_field == vmode) {
            if (!output_active)
                lua_node_filter_s(buildpage_filter_callback,lua_key_index(alignment));
            build_page();
        }
    }
}

@ The token list |omit_template| just referred to is a constant token
list that contains the special control sequence \.{\\endtemplate} only.

@c
void initialize_alignments(void)
{
    token_info(omit_template) = end_template_token;     /* |link(omit_template)=null| */
    span_span(end_span) = max_quarterword + 1;
    span_ptr(end_span) = null;
}
