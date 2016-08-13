% buildpage.w
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

@ @c
#define mode mode_par
#define head head_par
#define tail tail_par

@ When \TeX\ appends new material to its main vlist in vertical mode, it uses
a method something like |vsplit| to decide where a page ends, except that
the calculations are done ``on line'' as new items come in.
The main complication in this process is that insertions must be put
into their boxes and removed from the vlist, in a more-or-less optimum manner.

We shall use the term ``current page'' for that part of the main vlist that
is being considered as a candidate for being broken off and sent to the
user's output routine. The current page starts at |vlink(page_head)|, and
it ends at |page_tail|.  We have |page_head=page_tail| if this list is empty.
@^current page@>

Utter chaos would reign if the user kept changing page specifications
while a page is being constructed, so the page builder keeps the pertinent
specifications frozen as soon as the page receives its first box or
insertion.  The global variable |page_contents| is |empty| when the
current page contains only mark nodes and content-less whatsit nodes; it
is |inserts_only| if the page contains only insertion nodes in addition to
marks and whatsits.  Glue nodes, kern nodes, and penalty nodes are
discarded until a box or rule node appears, at which time |page_contents|
changes to |box_there|.  As soon as |page_contents| becomes non-|empty|,
the current |vsize| and |max_depth| are squirreled away into |page_goal|
and |page_max_depth|; the latter values will be used until the page has
been forwarded to the user's output routine. The \.{\\topskip} adjustment
is made when |page_contents| changes to |box_there|.

Although |page_goal| starts out equal to |vsize|, it is decreased by the
scaled natural height-plus-depth of the insertions considered so far, and by
the \.{\\skip} corrections for those insertions. Therefore it represents
the size into which the non-inserted material should fit, assuming that
all insertions in the current page have been made.

The global variables |best_page_break| and |least_page_cost| correspond
respectively to the local variables |best_place| and |least_cost| in the
|vert_break| routine that we have already studied; i.e., they record the
location and value of the best place currently known for breaking the
current page. The value of |page_goal| at the time of the best break is
stored in |best_size|.

@c
halfword page_tail;             /* the final node on the current page */
int page_contents;              /* what is on the current page so far? */
scaled page_max_depth;          /* maximum box depth on page being built */
halfword best_page_break;       /* break here to get the best page known so far */
int least_page_cost;            /* the score for this currently best page */
scaled best_size;               /* its |page_goal| */

@ The page builder has another data structure to keep track of insertions.
This is a list of four-word nodes, starting and ending at |page_ins_head|.
That is, the first element of the list is node |r@t$_1$@>=vlink(page_ins_head)|;
node $r_j$ is followed by |r@t$_{j+1}$@>=vlink(r@t$_j$@>)|; and if there are
|n| items we have |r@t$_{n+1}$@>=page_ins_head|. The |subtype| field of
each node in this list refers to an insertion number; for example, `\.{\\insert
250}' would correspond to a node whose |subtype| is |qi(250)|
(the same as the |subtype| field of the relevant |ins_node|). These |subtype|
fields are in increasing order, and |subtype(page_ins_head)=65535|, so
|page_ins_head| serves as a convenient sentinel
at the end of the list. A record is present for each insertion number that
appears in the current page.

The |type| field in these nodes distinguishes two possibilities that
might occur as we look ahead before deciding on the optimum page break.
If |type(r)=inserting_node|, then |height(r)| contains the total of the
height-plus-depth dimensions of the box and all its inserts seen so far.
 |type(r)=split_up_node|, then no more insertions will be made into this box,
because at least one previous insertion was too big to fit on the current
page; |broken_ptr(r)| points to the node where that insertion will be
split, if \TeX\ decides to split it, |broken_ins(r)| points to the
insertion node that was tentatively split, and |height(r)| includes also the
natural height plus depth of the part that would be split off.

In both cases, |last_ins_ptr(r)| points to the last |ins_node|
encountered for box |qo(subtype(r))| that would be at least partially
inserted on the next page; and |best_ins_ptr(r)| points to the last
such |ins_node| that should actually be inserted, to get the page with
minimum badness among all page breaks considered so far. We have
|best_ins_ptr(r)=null| if and only if no insertion for this box should
be made to produce this optimum page.

@ Pages are built by appending nodes to the current list in \TeX's
vertical mode, which is at the outermost level of the semantic nest. This
vlist is split into two parts; the ``current page'' that we have been
talking so much about already, and the ``contribution list'' that receives
new nodes as they are created.  The current page contains everything that
the page builder has accounted for in its data structures, as described
above, while the contribution list contains other things that have been
generated by other parts of \TeX\ but have not yet been
seen by the page builder.
The contribution list starts at |vlink(contrib_head)|, and it ends at the
current node in \TeX's vertical mode.

When \TeX\ has appended new material in vertical mode, it calls the procedure
|build_page|, which tries to catch up by moving nodes from the contribution
list to the current page. This procedure will succeed in its goal of
emptying the contribution list, unless a page break is discovered, i.e.,
unless the current page has grown to the point where the optimum next
page break has been determined. In the latter case, the nodes after the
optimum break will go back onto the contribution list, and control will
effectively pass to the user's output routine.

We make |type(page_head)=glue_node|, so that an initial glue node on
the current page will not be considered a valid breakpoint.

@c
void initialize_buildpage(void)
{
    subtype(page_ins_head) = 65535;
    type(page_ins_head) = split_up_node;
    vlink(page_ins_head) = page_ins_head;

    type(page_head) = glue_node;
    subtype(page_head) = normal;
}


@ An array |page_so_far| records the heights and depths of everything
on the current page. This array contains six |scaled| numbers, like the
similar arrays already considered in |line_break| and |vert_break|; and it
also contains |page_goal| and |page_depth|, since these values are
all accessible to the user via |set_page_dimen| commands. The
value of |page_so_far[1]| is also called |page_total|.  The stretch
and shrink components of the \.{\\skip} corrections for each insertion are
included in |page_so_far|, but the natural space components of these
corrections are not, since they have been subtracted from |page_goal|.

The variable |page_depth| records the depth of the current page; it has been
adjusted so that it is at most |page_max_depth|. The variable
|last_glue| points to the glue specification of the most recent node
contributed from the contribution list, if this was a glue node; otherwise
|last_glue=max_halfword|. (If the contribution list is nonempty,
however, the value of |last_glue| is not necessarily accurate.)
The variables |last_penalty|, |last_kern|, and |last_node_type|
are similar.  And
finally, |insert_penalties| holds the sum of the penalties associated with
all split and floating insertions.

@c
scaled page_so_far[8];          /* height and glue of the current page */
halfword last_glue;             /* used to implement \.{\\lastskip} */
int last_penalty;               /* used to implement \.{\\lastpenalty} */
scaled last_kern;               /* used to implement \.{\\lastkern} */
int last_node_type;             /* used to implement \.{\\lastnodetype} */
int insert_penalties;           /* sum of the penalties for held-over insertions */

#define print_plus(A,B) do { \
    if (page_so_far[(A)]!=0) { \
        tprint(" plus "); \
        print_scaled(page_so_far[(A)]); \
        tprint((B)); \
    } \
} while (0)

void print_totals(void)
{
    print_scaled(page_total);
    print_plus(2, "");
    print_plus(3, "fil");
    print_plus(4, "fill");
    print_plus(5, "filll");
    if (page_shrink != 0) {
        tprint(" minus ");
        print_scaled(page_shrink);
    }
}

@ Here is a procedure that is called when the |page_contents| is changing
from |empty| to |inserts_only| or |box_there|.

@c
#define do_all_six(A) A(1);A(2);A(3);A(4);A(5);A(6);A(7)
#define set_page_so_far_zero(A) page_so_far[(A)]=0

void freeze_page_specs(int s)
{
    page_contents = s;
    page_goal = vsize_par;
    page_max_depth = max_depth_par;
    page_depth = 0;
    do_all_six(set_page_so_far_zero);
    least_page_cost = awful_bad;
    if (tracing_pages_par > 0) {
        begin_diagnostic();
        tprint_nl("%% goal height=");
        print_scaled(page_goal);
        tprint(", max depth=");
        print_scaled(page_max_depth);
        end_diagnostic(false);
    }
}

@ The global variable |output_active| is true during the time the
user's output routine is driving \TeX.

@c
boolean output_active;          /* are we in the midst of an output routine? */

@ The page builder is ready to start a fresh page if we initialize
the following state variables. (However, the page insertion list is initialized
elsewhere.)

@c
void start_new_page(void)
{
    page_contents = empty;
    page_tail = page_head;
    vlink(page_head) = null;
    last_glue = max_halfword;
    last_penalty = 0;
    last_kern = 0;
    last_node_type = -1;
    page_depth = 0;
    page_max_depth = 0;
}

@ At certain times box \.{\\outputbox} is supposed to be void (i.e., |null|),
or an insertion box is supposed to be ready to accept a vertical list.
If not, an error message is printed, and the following subroutine
flushes the unwanted contents, reporting them to the user.

@c
static void box_error(int n)
{
    error();
    begin_diagnostic();
    tprint_nl("The following box has been deleted:");
    show_box(box(n));
    end_diagnostic(true);
    flush_node_list(box(n));
    box(n) = null;
}

@ The following procedure guarantees that a given box register
does not contain an \.{\\hbox}.

@c
static void ensure_vbox(int n)
{
    halfword p;                 /* the box register contents */
    p = box(n);
    if (p != null && type(p) == hlist_node) {
        print_err("Insertions can only be added to a vbox");
        help3("Tut tut: You're trying to \\insert into a",
              "\\box register that now contains an \\hbox.",
              "Proceed, and I'll discard its present contents.");
        box_error(n);
    }
}

@ \TeX\ is not always in vertical mode at the time |build_page|
is called; the current mode reflects what \TeX\ should return to, after
the contribution list has been emptied. A call on |build_page| should
be immediately followed by `|goto big_switch|', which is \TeX's central
control point.

@c
void build_page(void)
{                               /* append contributions to the current page */
    halfword p;                 /* the node being appended */
    halfword q, r;              /* nodes being examined */
    int b, c;                   /* badness and cost of current page */
    int pi = 0;                 /* penalty to be added to the badness */
    int n;                      /* insertion box number */
    scaled delta, h, w;         /* sizes used for insertion calculations */
    if ((vlink(contrib_head) == null) || output_active)
        return;
    do {
      CONTINUE:
        p = vlink(contrib_head);
        /* Update the values of |last_glue|, |last_penalty|, and |last_kern| */
        if (last_glue != max_halfword) {
            flush_node(last_glue);
            last_glue = max_halfword;
        }
        last_penalty = 0;
        last_kern = 0;
        last_node_type = type(p) + 1;
        if (type(p) == glue_node) {
            last_glue = new_glue(p);
        } else if (type(p) == penalty_node) {
            last_penalty = penalty(p);
        } else if (type(p) == kern_node) {
            last_kern = width(p);
        }

        /* Move node |p| to the current page; if it is time for a page break,
           put the nodes following the break back onto the contribution list,
           and |return| to the users output routine if there is one */

        /* The code here is an example of a many-way switch into routines that
           merge together in different places. Some people call this unstructured
           programming, but the author doesn't see much wrong with it, as long as
           the various labels have a well-understood meaning.
         */
        /* If the current page is empty and node |p| is to be deleted, |goto done1|;
           otherwise use node |p| to update the state of the current page;
           if this node is an insertion, |goto contribute|; otherwise if this node
           is not a legal breakpoint, |goto contribute| or |update_heights|;
           otherwise set |pi| to the penalty associated with this breakpoint */
        /* The title of this section is already so long, it seems best to avoid
           making it more accurate but still longer, by mentioning the fact that a
           kern node at the end of the contribution list will not be contributed until
           we know its successor. */
        switch (type(p)) {
        case hlist_node:
        case vlist_node:
        case rule_node:
            if (page_contents < box_there) {
                /* Initialize the current page, insert the \.{\\topskip} glue
                   ahead of |p|, and |goto continue| */
                if (page_contents == empty)
                    freeze_page_specs(box_there);
                else
                    page_contents = box_there;
                q = new_skip_param(top_skip_code);
                if ((type(p) == hlist_node) && is_mirrored(body_direction_par)) {
                    if (width(q) > depth(p))
                        width(q) = width(q) - depth(p);
                    else
                        width(q) = 0;
                } else {
                    if (width(q) > height(p))
                        width(q) = width(q) - height(p);
                    else
                        width(q) = 0;
                }
                couple_nodes(q, p);
                couple_nodes(contrib_head, q);
                goto CONTINUE;

            } else {
                /* Prepare to move a box or rule node to the current page,
                   then |goto contribute| */
                if ((type(p) == hlist_node) && is_mirrored(body_direction_par)) {
                    page_total = page_total + page_depth + depth(p);
                    page_depth = height(p);
                } else {
                    page_total = page_total + page_depth + height(p);
                    page_depth = depth(p);
                }
                goto CONTRIBUTE;

            }
            break;
        case boundary_node:
        case whatsit_node:
            goto CONTRIBUTE;
            break;
        case glue_node:
            if (page_contents < box_there)
                goto DONE1;
            else if (precedes_break(page_tail))
                pi = 0;
            else
                goto UPDATE_HEIGHTS;
            break;
        case kern_node:
            if (page_contents < box_there)
                goto DONE1;
            else if (vlink(p) == null)
                goto EXIT;
            else if (type(vlink(p)) == glue_node)
                pi = 0;
            else
                goto UPDATE_HEIGHTS;
            break;
        case penalty_node:
            if (page_contents < box_there)
                goto DONE1;
            else
                pi = penalty(p);
            break;
        case mark_node:
            goto CONTRIBUTE;
            break;
        case ins_node:
            /* Append an insertion to the current page and |goto contribute| */
            if (page_contents == empty)
                freeze_page_specs(inserts_only);
            n = subtype(p);
            r = page_ins_head;
            while (n >= subtype(vlink(r)))
                r = vlink(r);
            if (subtype(r) != n) {
                /* Create a page insertion node with |subtype(r)=qi(n)|, and
                   include the glue correction for box |n| in the
                   current page state */
                /* We take note of the value of \.{\\skip} |n| and the height plus depth
                   of \.{\\box}~|n| only when the first \.{\\insert}~|n| node is
                   encountered for a new page. A user who changes the contents of \.{\\box}~|n|
                   after that first \.{\\insert}~|n| had better be either extremely careful
                   or extremely lucky, or both. */

                q = new_node(inserting_node, n);
                try_couple_nodes(q, vlink(r));
                couple_nodes(r, q);
                r = q;
                ensure_vbox(n);
                if (box(n) == null)
                    height(r) = 0;
                else
                    height(r) = height(box(n)) + depth(box(n));
                best_ins_ptr(r) = null;
                q = skip(n);
                if (count(n) == 1000)
                    h = height(r);
                else
                    h = x_over_n(height(r), 1000) * count(n);
                page_goal = page_goal - h - width(q);
                if (stretch_order(q) > 1)
                    page_so_far[1 + stretch_order(q)] =
                        page_so_far[1 + stretch_order(q)] + stretch(q);
                else
                    page_so_far[2 + stretch_order(q)] =
                        page_so_far[2 + stretch_order(q)] + stretch(q);
                page_shrink = page_shrink + shrink(q);
                if ((shrink_order(q) != normal) && (shrink(q) != 0)) {
                    print_err("Infinite glue shrinkage inserted from \\skip");
                    print_int(n);
                    help3
                        ("The correction glue for page breaking with insertions",
                         "must have finite shrinkability. But you may proceed,",
                         "since the offensive shrinkability has been made finite.");
                    error();
                }

            }
            if (type(r) == split_up_node) {
                insert_penalties = insert_penalties + float_cost(p);
            } else {
                last_ins_ptr(r) = p;
                delta = page_goal - page_total - page_depth + page_shrink;
                /* this much room is left if we shrink the maximum */
                if (count(n) == 1000)
                    h = height(p);
                else
                    h = x_over_n(height(p), 1000) * count(n);   /* this much room is needed */
                if (((h <= 0) || (h <= delta))
                    && (height(p) + height(r) <= dimen(n))) {
                    page_goal = page_goal - h;
                    height(r) = height(r) + height(p);
                } else {
                    /* Find the best way to split the insertion, and change
                       |type(r)| to |split_up_node| */
                    /* Here is the code that will split a long footnote between pages, in an
                       emergency. The current situation deserves to be recapitulated: Node |p|
                       is an insertion into box |n|; the insertion will not fit, in its entirety,
                       either because it would make the total contents of box |n| greater than
                       \.{\\dimen} |n|, or because it would make the incremental amount of growth
                       |h| greater than the available space |delta|, or both. (This amount |h| has
                       been weighted by the insertion scaling factor, i.e., by \.{\\count} |n|
                       over 1000.) Now we will choose the best way to break the vlist of the
                       insertion, using the same criteria as in the \.{\\vsplit} operation.
                     */
                    if (count(n) <= 0) {
                        w = max_dimen;
                    } else {
                        w = page_goal - page_total - page_depth;
                        if (count(n) != 1000)
                            w = x_over_n(w, count(n)) * 1000;
                    }
                    if (w > dimen(n) - height(r))
                        w = dimen(n) - height(r);
                    q = vert_break(ins_ptr(p), w, depth(p));
                    height(r) = height(r) + best_height_plus_depth;
                    if (tracing_pages_par > 0) {
                        /* Display the insertion split cost */
                        begin_diagnostic();
                        tprint_nl("% split");
                        print_int(n);
                        tprint(" to ");
                        print_scaled(w);
                        print_char(',');
                        print_scaled(best_height_plus_depth);
                        tprint(" p=");
                        if (q == null)
                            print_int(eject_penalty);
                        else if (type(q) == penalty_node)
                            print_int(penalty(q));
                        else
                            print_char('0');
                        end_diagnostic(false);

                    }
                    if (count(n) != 1000)
                        best_height_plus_depth =
                            x_over_n(best_height_plus_depth, 1000) * count(n);
                    page_goal = page_goal - best_height_plus_depth;
                    type(r) = split_up_node;
                    broken_ptr(r) = q;
                    broken_ins(r) = p;
                    if (q == null)
                        insert_penalties = insert_penalties + eject_penalty;
                    else if (type(q) == penalty_node)
                        insert_penalties = insert_penalties + penalty(q);
                }
            }
            goto CONTRIBUTE;

            break;
        default:
            fprintf(stderr, "type(p)=%d\n", type(p));
            confusion("page");
            break;
        }

        /* Check if node |p| is a new champion breakpoint; then if it is time for
           a page break, prepare for output, and either fire up the users
           output routine and |return| or ship out the page and |goto done| */

        if (pi < inf_penalty) {
            /* Compute the badness, |b|, of the current page,
               using |awful_bad| if the box is too full */
            if (page_total < page_goal) {
                if ((page_so_far[3] != 0) || (page_so_far[4] != 0) ||
                    (page_so_far[5] != 0))
                    b = 0;
                else
                    b = badness(page_goal - page_total, page_so_far[2]);
            } else if (page_total - page_goal > page_shrink) {
                b = awful_bad;
            } else {
                b = badness(page_total - page_goal, page_shrink);
            }

            if (b < awful_bad) {
                if (pi <= eject_penalty)
                    c = pi;
                else if (b < inf_bad)
                    c = b + pi + insert_penalties;
                else
                    c = deplorable;
            } else {
                c = b;
            }
            if (insert_penalties >= 10000)
                c = awful_bad;
            if (tracing_pages_par > 0) {
                /* Display the page break cost */
                begin_diagnostic();
                tprint_nl("%");
                tprint(" t=");
                print_totals();
                tprint(" g=");
                print_scaled(page_goal);
                tprint(" b=");
                if (b == awful_bad)
                    print_char('*');
                else
                    print_int(b);
                tprint(" p=");
                print_int(pi);
                tprint(" c=");
                if (c == awful_bad)
                    print_char('*');
                else
                    print_int(c);
                if (c <= least_page_cost)
                    print_char('#');
                end_diagnostic(false);

            }
            if (c <= least_page_cost) {
                best_page_break = p;
                best_size = page_goal;
                least_page_cost = c;
                r = vlink(page_ins_head);
                while (r != page_ins_head) {
                    best_ins_ptr(r) = last_ins_ptr(r);
                    r = vlink(r);
                }
            }
            if ((c == awful_bad) || (pi <= eject_penalty)) {
                fire_up(p);     /* output the current page at the best place */
                if (output_active)
                    goto EXIT;  /* user's output routine will act */
                goto DONE;      /* the page has been shipped out by default output routine */
            }
        }

        if ((type(p) < glue_node) || (type(p) > kern_node))
            goto CONTRIBUTE;

      UPDATE_HEIGHTS:          /* go here to record glue in the |active_height| table */

        /* Update the current page measurements with respect to the
           glue or kern specified by node~|p| */
        if (type(p) != kern_node) {
            if (stretch_order(p) > 1)
                page_so_far[1 + stretch_order(p)] = page_so_far[1 + stretch_order(p)] + stretch(p);
            else
                page_so_far[2 + stretch_order(p)] = page_so_far[2 + stretch_order(p)] + stretch(p);
            page_shrink = page_shrink + shrink(p);
            if ((shrink_order(p) != normal) && (shrink(p) != 0)) {
                print_err("Infinite glue shrinkage found on current page");
                help4("The page about to be output contains some infinitely",
                      "shrinkable glue, e.g., `\\vss' or `\\vskip 0pt minus 1fil'.",
                      "Such glue doesn't belong there; but you can safely proceed,",
                      "since the offensive shrinkability has been made finite.");
                error();
                reset_glue_to_zero(p);
                shrink_order(p) = normal;
            }
        }
        page_total = page_total + page_depth + width(p);
        page_depth = 0;

      CONTRIBUTE:              /* go here to link a node into the current page */

        /* Make sure that |page_max_depth| is not exceeded */
        if (page_depth > page_max_depth) {
            page_total = page_total + page_depth - page_max_depth;
            page_depth = page_max_depth;
        }

        /* Link node |p| into the current page and |goto done| */
        couple_nodes(page_tail, p);
        page_tail = p;
        try_couple_nodes(contrib_head,vlink(p));
        vlink(p) = null;
        goto DONE;
      DONE1:
        /* Recycle node |p| */
        try_couple_nodes(contrib_head,vlink(p));
        vlink(p) = null;
        if (saving_vdiscards_par > 0) {
            if (page_disc == null) {
                page_disc = p;
            } else {
                couple_nodes(tail_page_disc, p);
            }
            tail_page_disc = p;
        } else {
            flush_node_list(p);
        }
      DONE:
        ;
    } while (vlink(contrib_head) != null);
    /* Make the contribution list empty by setting its tail to |contrib_head| */
    contrib_tail = contrib_head;
  EXIT:
    ;
}

@ When the page builder has looked at as much material as could appear before
the next page break, it makes its decision. The break that gave minimum
badness will be used to put a completed ``page'' into box \.{\\outputbox}, with insertions
appended to their other boxes.

We also set the values of |top_mark|, |first_mark|, and |bot_mark|. The
program uses the fact that |bot_mark(x)<>null| implies |first_mark(x)<>null|;
it also knows that |bot_mark(x)=null| implies |top_mark(x)=first_mark(x)=null|.

The |fire_up| subroutine prepares to output the current page at the best
place; then it fires up the user's output routine, if there is one,
or it simply ships out the page. There is one parameter, |c|, which represents
the node that was being contributed to the page when the decision to
force an output was made.

@c
void fire_up(halfword c)
{
    halfword p, q, r, s;        /* nodes being examined and/or changed */
    halfword prev_p;            /* predecessor of |p| */
    int n;                      /* insertion box number */
    boolean wait;               /* should the present insertion be held over? */
    int save_vbadness;          /* saved value of |vbadness| */
    scaled save_vfuzz;          /* saved value of |vfuzz| */
    halfword save_split_top_skip;       /* saved value of |split_top_skip| */
    halfword i;                 /* for looping through the marks */

    /* Set the value of |output_penalty| */
    if (type(best_page_break) == penalty_node) {
        geq_word_define(int_base + output_penalty_code,
                        penalty(best_page_break));
        penalty(best_page_break) = inf_penalty;
    } else {
        geq_word_define(int_base + output_penalty_code, inf_penalty);
    }

    for (i = 0; i <= biggest_used_mark; i++) {
        if (bot_mark(i) != null) {
            if (top_mark(i) != null)
                delete_token_ref(top_mark(i));
            set_top_mark(i, bot_mark(i));
            add_token_ref(top_mark(i));
            delete_first_mark(i);
        }
    }
    /* Put the optimal current page into box |output_box|, update |first_mark| and
       |bot_mark|, append insertions to their boxes, and put the
       remaining nodes back on the contribution list; */

    /* As the page is finally being prepared for output,
       pointer |p| runs through the vlist, with |prev_p| trailing behind;
       pointer |q| is the tail of a list of insertions that
       are being held over for a subsequent page. */

    if (c == best_page_break)
        best_page_break = null; /* |c| not yet linked in */
    /* Ensure that box |output_box| is empty before output */
    if (box(output_box_par) != null) {
        print_err("\\box");
        print_int(output_box_par);
        tprint(" is not void");
        help2("You shouldn't use \\box\\outputbox except in \\output routines.",
              "Proceed, and I'll discard its present contents.");
        box_error(output_box_par);
    }

    insert_penalties = 0;       /* this will count the number of insertions held over */
    save_split_top_skip = split_top_skip_par;
    if (holding_inserts_par <= 0) {
        /* Prepare all the boxes involved in insertions to act as queues */
        /* If many insertions are supposed to go into the same box, we want to know
           the position of the last node in that box, so that we don't need to waste time
           when linking further information into it. The |last_ins_ptr| fields of the
           page insertion nodes are therefore used for this purpose during the
           packaging phase. */

        r = vlink(page_ins_head);
        while (r != page_ins_head) {
            if (best_ins_ptr(r) != null) {
                n = subtype(r);
                ensure_vbox(n);
                if (box(n) == null)
                    box(n) = new_null_box();
                p = box(n) + list_offset;
                while (vlink(p) != null)
                    p = vlink(p);
                last_ins_ptr(r) = p;
            }
            r = vlink(r);
        }

    }
    q = hold_head;
    vlink(q) = null;
    prev_p = page_head;
    p = vlink(prev_p);
    while (p != best_page_break) {
        if (type(p) == ins_node) {
            if (holding_inserts_par <= 0) {
                /* Either insert the material specified by node |p| into the
                   appropriate box, or hold it for the next page;
                   also delete node |p| from the current page */
                /* We will set |best_ins_ptr:=null| and package the box corresponding to
                   insertion node~|r|, just after making the final insertion into that box.
                   If this final insertion is `|split_up_node|', the remainder after splitting
                   and pruning (if any) will be carried over to the next page. */
                r = vlink(page_ins_head);
                while (subtype(r) != subtype(p))
                    r = vlink(r);
                if (best_ins_ptr(r) == null) {
                    wait = true;
                } else {
                    wait = false;
                    s = last_ins_ptr(r);
                    vlink(s) = ins_ptr(p);
                    if (best_ins_ptr(r) == p) {
                        halfword t; /* was a global temp_ptr */
                        /* Wrap up the box specified by node |r|, splitting node |p| if
                           called for; set |wait:=true| if node |p| holds a remainder after
                           splitting */
                        if (type(r) == split_up_node) {
                            if ((broken_ins(r) == p) && (broken_ptr(r) != null)) {
                                while (vlink(s) != broken_ptr(r))
                                    s = vlink(s);
                                vlink(s) = null;
                                split_top_skip_par = split_top_ptr(p);
                                ins_ptr(p) =
                                    prune_page_top(broken_ptr(r), false);
                                if (ins_ptr(p) != null) {
                                    t = vpack(ins_ptr(p), 0, additional, -1);
                                    height(p) = height(t) + depth(t);
                                    list_ptr(t) = null;
                                    flush_node(t);
                                    wait = true;
                                }
                            }
                        }
                        best_ins_ptr(r) = null;
                        n = subtype(r);
                        t = list_ptr(box(n));
                        list_ptr(box(n)) = null;
                        flush_node(box(n));
                        box(n) = vpack(t, 0, additional, body_direction_par);

                    } else {
                        while (vlink(s) != null)
                            s = vlink(s);
                        last_ins_ptr(r) = s;
                    }
                }
                /* Either append the insertion node |p| after node |q|, and remove it
                   from the current page, or delete |node(p)| */
                try_couple_nodes(prev_p, vlink(p));
                vlink(p) = null;
                if (wait) {
                    couple_nodes(q, p);
                    q = p;
                    incr(insert_penalties);
                } else {
                    ins_ptr(p) = null;
                    flush_node(p);
                }
                p = prev_p;

            }
        } else if (type(p) == mark_node) {
            /* Update the values of |first_mark| and |bot_mark| */
            if (first_mark(mark_class(p)) == null) {
                set_first_mark(mark_class(p), mark_ptr(p));
                add_token_ref(first_mark(mark_class(p)));
            }
            if (bot_mark(mark_class(p)) != null)
                delete_token_ref(bot_mark(mark_class(p)));
            set_bot_mark(mark_class(p), mark_ptr(p));
            add_token_ref(bot_mark(mark_class(p)));

        }
        prev_p = p;
        p = vlink(prev_p);
    }
    split_top_skip_par = save_split_top_skip;
    /* Break the current page at node |p|, put it in box~|output_box|,
       and put the remaining nodes on the contribution list */
    /* When the following code is executed, the current page runs from node
       |vlink(page_head)| to node |prev_p|, and the nodes from |p| to |page_tail|
       are to be placed back at the front of the contribution list. Furthermore
       the heldover insertions appear in a list from |vlink(hold_head)| to |q|; we
       will put them into the current page list for safekeeping while the user's
       output routine is active.  We might have |q=hold_head|; and |p=null| if
       and only if |prev_p=page_tail|. Error messages are suppressed within
       |vpackage|, since the box might appear to be overfull or underfull simply
       because the stretch and shrink from the \.{\\skip} registers for inserts
       are not actually present in the box. */

    if (p != null) {
        if (vlink(contrib_head) == null) {
            contrib_tail = page_tail;
        }
        couple_nodes(page_tail,vlink(contrib_head));
        couple_nodes(contrib_head, p);
        vlink(prev_p) = null;
    }
    save_vbadness = vbadness_par;
    vbadness_par = inf_bad;
    save_vfuzz = vfuzz_par;
    vfuzz_par = max_dimen;          /* inhibit error messages */
    box(output_box_par) = filtered_vpackage(vlink(page_head),
        best_size, exactly, page_max_depth, output_group, body_direction_par, 0, 0);
    vbadness_par = save_vbadness;
    vfuzz_par = save_vfuzz;
    if (last_glue != max_halfword)
        flush_node(last_glue);
    /* Start a new current page */
    start_new_page();           /* this sets |last_glue:=max_halfword| */
    if (q != hold_head) {
        vlink(page_head) = vlink(hold_head);
        page_tail = q;
    }

    /* Delete the page-insertion nodes */
    r = vlink(page_ins_head);
    while (r != page_ins_head) {
	    /* todo: couple */
        q = vlink(r);
        flush_node(r);
        r = q;
    }
    vlink(page_ins_head) = page_ins_head;

    for (i = 0; i <= biggest_used_mark; i++) {
        if ((top_mark(i) != null) && (first_mark(i) == null)) {
            set_first_mark(i, top_mark(i));
            add_token_ref(top_mark(i));
        }
    }
    if (output_routine_par != null) {
        if (dead_cycles >= max_dead_cycles_par) {
            /* Explain that too many dead cycles have occurred in a row */
            print_err("Output loop---");
            print_int(dead_cycles);
            tprint(" consecutive dead cycles");
            help3("I've concluded that your \\output is awry; it never does a",
                  "\\shipout, so I'm shipping \\box\\outputbox out myself. Next time",
                  "increase \\maxdeadcycles if you want me to be more patient!");
            error();

        } else {
            /* Fire up the users output routine and |return| */
            output_active = true;
            incr(dead_cycles);
            push_nest();
            mode = -vmode;
            prev_depth_par = ignore_depth;
            mode_line_par = -line;
            begin_token_list(output_routine_par, output_text);
            new_save_level(output_group);
            normal_paragraph();
            scan_left_brace();
            return;

        }
    }
    /* Perform the default output routine */
    /* The list of heldover insertions, running from |vlink(page_head)| to
       |page_tail|, must be moved to the contribution list when the user has
       specified no output routine. */
    if (vlink(page_head) != null) {
        if (vlink(contrib_head) == null) {
            contrib_tail = page_tail;
        } else {
            vlink(page_tail) = vlink(contrib_head);
        }
        vlink(contrib_head) = vlink(page_head);
        vlink(page_head) = null;
        page_tail = page_head;
    }
    flush_node_list(page_disc);
    page_disc = null;
    ship_out(static_pdf, box(output_box_par), SHIPPING_PAGE);
    box(output_box_par) = null;
}

@ When the user's output routine finishes, it has constructed a vlist
in internal vertical mode, and \TeX\ will do the following:

@c
void resume_after_output(void)
{
    if ((iloc != null)
        || ((token_type != output_text) && (token_type != backed_up))) {
        /* Recover from an unbalanced output routine */
        print_err("Unbalanced output routine");
        help2("Your sneaky output routine has problematic {'s and/or }'s.",
              "I can't handle that very well; good luck.");
        error();
        do {
            get_token();
        } while (iloc != null);
        /* loops forever if reading from a file, since |null=min_halfword<=0| */

    }
    end_token_list();           /* conserve stack space in case more outputs are triggered */
    end_graf(bottom_level);
    unsave();
    output_active = false;
    insert_penalties = 0;
    /* Ensure that box |output_box| is empty after output */
    if (box(output_box_par) != null) {
        print_err("Output routine didn't use all of \\box");
        print_int(output_box_par);
        help3("Your \\output commands should empty \\box\\outputbox,",
              "e.g., by saying `\\shipout\\box\\outputbox'.",
              "Proceed; I'll discard its present contents.");
        box_error(output_box_par);
    }

    if (tail != head) {         /* current list goes after heldover insertions */
        try_couple_nodes(page_tail, vlink(head));
        page_tail = tail;
    }
    if (vlink(page_head) != null) {     /* and both go before heldover contributions */
        if (vlink(contrib_head) == null)
            contrib_tail = page_tail;
        try_couple_nodes(page_tail, vlink(contrib_head));
        try_couple_nodes(contrib_head, vlink(page_head));
        vlink(page_head) = null;
        page_tail = page_head;
    }
    flush_node_list(page_disc);
    page_disc = null;
    pop_nest();
    normal_page_filter(after_output);
    build_page();
}
