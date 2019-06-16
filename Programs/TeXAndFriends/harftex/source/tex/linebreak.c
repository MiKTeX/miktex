/*

Copyright 2006-2008 Taco Hoekwater <taco@luatex.org>

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

#include "ptexlib.h"

/*tex

    We come now to what is probably the most interesting algorithm of \TeX: the
    mechanism for choosing the ``best possible'' breakpoints that yield the
    individual lines of a paragraph. \TeX's line-breaking algorithm takes a given
    horizontal list and converts it to a sequence of boxes that are appended to
    the current vertical list. In the course of doing this, it creates a special
    data structure containing three kinds of records that are not used elsewhere
    in \TeX. Such nodes are created while a paragraph is being processed, and
    they are destroyed afterwards; thus, the other parts of \TeX\ do not need to
    know anything about how line-breaking is done.

    The method used here is based on an approach devised by Michael F. Plass and
    the author in 1977, subsequently generalized and improved by the same two
    people in 1980. A detailed discussion appears in {\sl SOFTWARE---Practice
    \AM\ Experience \bf11} (1981), 1119--1184, where it is shown that the
    line-breaking problem can be regarded as a special case of the problem of
    computing the shortest path in an acyclic network. The cited paper includes
    numerous examples and describes the history of line breaking as it has been
    practiced by printers through the ages. The present implementation adds two
    new ideas to the algorithm of 1980: Memory space requirements are
    considerably reduced by using smaller records for inactive nodes than for
    active ones, and arithmetic overflow is avoided by using ``delta distances''
    instead of keeping track of the total distance from the beginning of the
    paragraph to the current point.

    The |line_break| procedure should be invoked only in horizontal mode; it
    leaves that mode and places its output into the current vlist of the
    enclosing vertical mode (or internal vertical mode). There is one explicit
    parameter: |d| is true for partial paragraphs preceding display math mode; in
    this case the amount of additional penalty inserted before the final line is
    |display_widow_penalty| instead of |widow_penalty|.

    There are also a number of implicit parameters: The hlist to be broken starts
    at |vlink(head)|, and it is nonempty. The value of |prev_graf| in the
    enclosing semantic level tells where the paragraph should begin in the
    sequence of line numbers, in case hanging indentation or \.{\\parshape} are
    in use; |prev_graf| is zero unless this paragraph is being continued after a
    displayed formula. Other implicit parameters, such as the |par_shape_ptr| and
    various penalties to use for hyphenation, etc., appear in |eqtb|.

    After |line_break| has acted, it will have updated the current vlist and the
    value of |prev_graf|. Furthermore, the global variable |just_box| will point
    to the final box created by |line_break|, so that the width of this line can
    be ascertained when it is necessary to decide whether to use
    |above_display_skip| or |above_display_short_skip| before a displayed
    formula.

*/

/*tex The |hlist_node| for the last line of the new paragraph: */

halfword just_box;

/*tex

    In it's complete form, |line_break| is a rather lengthy procedure---sort of a
    small world unto itself---we must build it up little by little. Below you see
    only the general outline.

    The main task performed here is to move the list from |head| to |temp_head|
    and go into the enclosing semantic level. We also append the
    \.{\\parfillskip} glue to the end of the paragraph, removing a space (or
    other glue node) if it was there, since spaces usually precede blank lines
    and instances of `\.{\$\$}'. The |par_fill_skip| is preceded by an infinite
    penalty, so it will never be considered as a potential breakpoint.

    That code assumes that a |glue_node| and a |penalty_node| occupy the same
    number of |mem|~words.

    Most other processing is delegated to external functions.

*/

void line_break(boolean d, int line_break_context)
{
    /*tex Main direction of paragraph: */
    int paragraph_dir = 0;
    halfword final_par_glue;
    halfword start_of_par;
    int callback_id;
    /*tex this is for over/underfull box messages */
    pack_begin_line = cur_list.ml_field;
    alink(temp_head) = null;
    vlink(temp_head) = vlink(cur_list.head_field);
    new_hyphenation(temp_head, cur_list.tail_field);
    cur_list.tail_field = new_ligkern(temp_head, cur_list.tail_field);
    if (is_char_node(cur_list.tail_field)) {
        tail_append(new_penalty(inf_penalty,line_penalty));
    } else if (type(cur_list.tail_field) != glue_node) {
        tail_append(new_penalty(inf_penalty,line_penalty));
    } else {
        halfword t = alink(cur_list.tail_field);
		flush_node(cur_list.tail_field);
		cur_list.tail_field = t;
		tail_append(new_penalty(inf_penalty,line_penalty));
    }
    final_par_glue = new_param_glue(par_fill_skip_code);
    couple_nodes(cur_list.tail_field, final_par_glue);
    cur_list.tail_field = vlink(cur_list.tail_field);
    lua_node_filter(pre_linebreak_filter_callback, line_break_context, temp_head, addressof(cur_list.tail_field));
    last_line_fill = cur_list.tail_field;
    pop_nest();
    start_of_par = cur_list.tail_field;
    callback_id = callback_defined(linebreak_filter_callback);
    if (callback_id > 0) {
        callback_id = lua_linebreak_callback(d, temp_head, addressof(cur_list.tail_field));
        if (callback_id > 0) {
            /*tex find the correct value for the |just_box| */
            halfword box_search = cur_list.tail_field;
            just_box  = null;
            if (box_search != null) {
                do {
                    if (type(box_search) == hlist_node) {
                       just_box = box_search;
                    }
                    box_search = vlink(box_search);
                } while (box_search != null);
            }
            if (just_box == null) {
                help3(
                    "A linebreaking routine should return a non-empty list of nodes",
                    "and at least one of those has to be a \\hbox.",
                    "Sorry, I cannot recover from this."
                );
                print_err("Invalid linebreak_filter");
                succumb();
            }
        } else {
            if (tracing_paragraphs_par > 0) {
                begin_diagnostic();
                print_int(line);
                end_diagnostic(true);
            }
        }
    }
    if (callback_id == 0) {
        if ((!is_char_node(vlink(temp_head))) && ((type(vlink(temp_head)) == local_par_node))) {
            paragraph_dir = local_par_dir(vlink(temp_head));
        } else {
            confusion("weird par dir");
        }
        ext_do_line_break(
            paragraph_dir,
            pretolerance_par,
            tracing_paragraphs_par,
            tolerance_par,
            emergency_stretch_par,
            looseness_par,
            adjust_spacing_par,
            par_shape_par_ptr,
            adj_demerits_par,
            protrude_chars_par,
            line_penalty_par,
            last_line_fit_par,
            double_hyphen_demerits_par,
            final_hyphen_demerits_par,
            hang_indent_par,
            hsize_par,
            hang_after_par,
            left_skip_par,
            right_skip_par,
            inter_line_penalties_par_ptr,
            inter_line_penalty_par,
            club_penalty_par,
            club_penalties_par_ptr,
            (d ? display_widow_penalties_par_ptr : widow_penalties_par_ptr),
            (d ? display_widow_penalty_par : widow_penalty_par),
            broken_penalty_par,
            final_par_glue
        );
    }
    lua_node_filter(post_linebreak_filter_callback, line_break_context, start_of_par, addressof(cur_list.tail_field));
    pack_begin_line = 0;
}

/*tex

    Glue nodes in a horizontal list that is being paragraphed are not supposed to
    include ``infinite'' shrinkability; that is why the algorithm maintains four
    registers for stretching but only one for shrinking. If the user tries to
    introduce infinite shrinkability, the shrinkability will be reset to finite
    and an error message will be issued. A boolean variable |no_shrink_error_yet|
    prevents this error message from appearing more than once per paragraph.

*/

#define check_shrinkage(a) \
    if ((shrink_order((a))!=normal)&&(shrink((a))!=0)) \
        a=finite_shrink((a))

/*tex Have we complained about infinite shrinkage? */

static boolean no_shrink_error_yet;

/*tex Recovers from infinite shrinkage. */

static halfword finite_shrink(halfword p)
{
    const char *hlp[] = {
        "The paragraph just ended includes some glue that has",
        "infinite shrinkability, e.g., `\\hskip 0pt minus 1fil'.",
        "Such glue doesn't belong there---it allows a paragraph",
        "of any length to fit on one line. But it's safe to proceed,",
        "since the offensive shrinkability has been made finite.",
        NULL
    };
    if (no_shrink_error_yet) {
        no_shrink_error_yet = false;
        tex_error("Infinite glue shrinkage found in a paragraph", hlp);
    }
    shrink_order(p) = normal;
    return p;
}

/*tex

    A pointer variable |cur_p| runs through the given horizontal list as we look
    for breakpoints. This variable is global, since it is used both by
    |line_break| and by its subprocedure |try_break|.

    Another global variable called |threshold| is used to determine the
    feasibility of individual lines: breakpoints are feasible if there is a way
    to reach them without creating lines whose badness exceeds |threshold|. (The
    badness is compared to |threshold| before penalties are added, so that
    penalty values do not affect the feasibility of breakpoints, except that no
    break is allowed when the penalty is 10000 or more.) If |threshold| is 10000
    or more, all legal breaks are considered feasible, since the |badness|
    function specified above never returns a value greater than~10000.

    Up to three passes might be made through the paragraph in an attempt to find
    at least one set of feasible breakpoints. On the first pass, we have
    |threshold=pretolerance| and |second_pass=final_pass=false|. If this pass
    fails to find a feasible solution, |threshold| is set to |tolerance|,
    |second_pass| is set |true|, and an attempt is made to hyphenate as many
    words as possible. If that fails too, we add |emergency_stretch| to the
    background stretchability and set |final_pass=true|.

*/

/*tex is this our second attempt to break this paragraph? */

static boolean second_pass;

/*tex is this our final attempt to break this paragraph? */

static boolean final_pass;

/*tex maximum badness on feasible lines */

static int threshold;

/*tex

    The maximum fill level for |hlist_stack|. Maybe good if larger than |2 *
    max_quarterword|, so that box nesting level would overflow first.

*/

#define max_hlist_stack 512

/*tex stack for |find_protchar_left()| and |find_protchar_right()| */

static halfword hlist_stack[max_hlist_stack];

/*tex fill level for |hlist_stack| */

static short hlist_stack_level = 0;

static void push_node(halfword p)
{
    if (hlist_stack_level >= max_hlist_stack)
        normal_error("push_node","stack overflow");
    hlist_stack[hlist_stack_level++] = p;
}

static halfword pop_node(void)
{
    if (hlist_stack_level <= 0) {
        /*tex This can point to some bug. */
        normal_error("pop_node","stack underflow (internal error)");
    }
    return hlist_stack[--hlist_stack_level];
}

/*tex maximal stretch ratio of expanded fonts */

static int max_stretch_ratio = 0;

/*tex maximal shrink ratio of expanded fonts */

static int max_shrink_ratio = 0;

/*tex the current step of expanded fonts */

static int cur_font_step = 0;

static boolean check_expand_pars(internal_font_number f)
{
    int m;
    if ((font_step(f) == 0) || ((font_max_stretch(f) == 0) && (font_max_shrink(f) == 0)))
        return false;
    if (cur_font_step < 0)
        cur_font_step = font_step(f);
    else if (cur_font_step != font_step(f))
        normal_error("font expansion","using fonts with different step of expansion in one paragraph is not allowed");
    m = font_max_stretch(f);
    if (m != 0) {
        if (max_stretch_ratio < 0)
            max_stretch_ratio = m;
        else if (max_stretch_ratio != m)
            normal_error("font expansion","using fonts with different limit of expansion in one paragraph is not allowed");
    }
    m = font_max_shrink(f);
    if (m != 0) {
        if (max_shrink_ratio < 0)
            max_shrink_ratio = -m;
        else if (max_shrink_ratio != -m)
            normal_error("font expansion","using fonts with different limit of expansion in one paragraph is not allowed");
    }
    return true;
}

/*tex Search left to right from list head |l|, returns 1st non-skipable item */

halfword find_protchar_left(halfword l, boolean d)
{
    halfword t;
    boolean run;
    boolean done = false ;
    while ((vlink(l) != null) && (type(l) == hlist_node) && zero_dimensions(l) && (list_ptr(l) == null)) {
        /*tex For paragraph start with \.{\\parindent} = 0pt or any empty hbox. */
        l = vlink(l);
        done = true ;
    }
    if ((!done) && (type(l) == local_par_node)) {
        l = vlink(l);
        done = true ;
    }
    if ((!done) && d) {
        while ((vlink(l) != null) && (!(is_char_node(l) || non_discardable(l)))) {
            /*tex standard discardables at line break, \TeX book, p 95 */
            l = vlink(l);
        }
    }
    if (type(l) != glyph_node) {
        hlist_stack_level = 0;
        run = true;
        do {
            t = l;
            while (run && (type(l) == hlist_node) && (list_ptr(l) != null)) {
                push_node(l);
                l = list_ptr(l);
            }
            while (run && cp_skipable(l)) {
                while ((vlink(l) == null) && (hlist_stack_level > 0)) {
                    /*tex Don't visit this node again. */
                    l = pop_node();
                    run = false;
                }
                if ((vlink(l) != null) && (type(l) == boundary_node) && (subtype(l) == protrusion_boundary) &&
                        ((boundary_value(l) == 1) || (boundary_value(l) == 3))) {
                    /*tex Skip next node. */
                    l = vlink(l);
                }
                if (vlink(l) != null) {
                    l = vlink(l);
                } else if (hlist_stack_level == 0) {
                    run = false;
                }
            }
        } while (t != l);
    }
    return l;
}

/*tex

    Search right to left from list tail |r| to head |l|, returns 1st non-skipable
    item.

*/

halfword find_protchar_right(halfword l, halfword r)
{
    halfword t;
    boolean run = true;
    if (r == null)
        return null;
    hlist_stack_level = 0;
    do {
        t = r;
        while (run && (type(r) == hlist_node) && (list_ptr(r) != null)) {
            push_node(l);
            push_node(r);
            l = list_ptr(r);
            r = l;
            while (vlink(r) != null) {
                halfword s = r;
                r = vlink(r);
                alink(r) = s;
            }
        }
        while (run && cp_skipable(r)) {
            while ((r == l) && (hlist_stack_level > 0)) {
                /*tex Don't visit this node again. */
                r = pop_node();
                l = pop_node();
            }
            if ((r != l) && (r != null)) {
                if ((alink(r) != null) && (type(r) == boundary_node) && (subtype(r) == protrusion_boundary) &&
                        ((boundary_value(r) == 2) || (boundary_value(r) == 3))) {
                    /*tex Skip next node. */
                    r = alink(r);
                }
                if (alink(r) != null) {
                    r = alink(r);
                } else {
                    /*tex This is the input: \.{\\leavevmode\\penalty-10000\\penalty-10000} (bug \#268). */
                    run = false;
                }
            } else if ((r == l) && (hlist_stack_level == 0))
                run = false;
        }
    } while (t != r);
    return r;
}

#define left_pw(a) char_pw((a), left_side)
#define right_pw(a) char_pw((a), right_side)

/*tex

    When looking for optimal line breaks, \TeX\ creates a ``break node'' for each
    break that is {\sl feasible}, in the sense that there is a way to end a line
    at the given place without requiring any line to stretch more than a given
    tolerance. A break node is characterized by three things: the position of the
    break (which is a pointer to a |glue_node|, |math_node|, |penalty_node|, or
    |disc_node|); the ordinal number of the line that will follow this
    breakpoint; and the fitness classification of the line that has just ended,
    i.e., |tight_fit|, |decent_fit|, |loose_fit|, or |very_loose_fit|.

*/

typedef enum {
    /*tex fitness classification for lines stretching more than their stretchability */
    very_loose_fit = 0,
    /*tex fitness classification for lines stretching 0.5 to 1.0 of their stretchability */
    loose_fit,
    /*tex fitness classification for all other lines */
    decent_fit,
    /*tex fitness classification for lines shrinking 0.5 to 1.0 of their shrinkability */
    tight_fit
} fitness_value;

/*tex

    The algorithm essentially determines the best possible way to achieve each
    feasible combination of position, line, and fitness. Thus, it answers
    questions like, ``What is the best way to break the opening part of the
    paragraph so that the fourth line is a tight line ending at such-and-such a
    place?'' However, the fact that all lines are to be the same length after a
    certain point makes it possible to regard all sufficiently large line numbers
    as equivalent, when the looseness parameter is zero, and this makes it
    possible for the algorithm to save space and time.

    An ``active node'' and a ``passive node'' are created in |mem| for each
    feasible breakpoint that needs to be considered. Active nodes are three words
    long and passive nodes are two words long. We need active nodes only for
    breakpoints near the place in the paragraph that is currently being examined,
    so they are recycled within a comparatively short time after they are
    created.

    An active node for a given breakpoint contains six fields:

    \startitemize[n]

        \startitem
            |vlink| points to the next node in the list of active nodes; the last
            active node has |vlink=active|.
        \stopitem

        \startitem
            |break_node| points to the passive node associated with this
            breakpoint.
        \stopitem

        \startitem
            |line_number| is the number of the line that follows this breakpoint.
        \stopitem

        \startitem
            |fitness| is the fitness classification of the line ending at this
            breakpoint.
        \stopitem

        \startitem
            |type| is either |hyphenated_node| or |unhyphenated_node|, depending
            on whether this breakpoint is a |disc_node|.
        \stopitem

        \startitem
            |total_demerits| is the minimum possible sum of demerits over all
            lines leading from the beginning of the paragraph to this breakpoint.
        \stopitem

    \stopitemize

    The value of |vlink(active)| points to the first active node on a vlinked
    list of all currently active nodes. This list is in order by |line_number|,
    except that nodes with |line_number>easy_line| may be in any order relative
    to each other.

*/

void initialize_active(void)
{
    type(active) = hyphenated_node;
    line_number(active) = max_halfword;
    /*tex The |subtype| is never examined. */
    subtype(active) = 0;
}

/*tex

    The passive node for a given breakpoint contains eight fields:

    \startitemize

        \startitem
            |vlink| points to the passive node created just before this one, if
            any, otherwise it is |null|.
        \stopitem

        \startitem
            |cur_break| points to the position of this breakpoint in the
            horizontal list for the paragraph being broken.
        \stopitem

        \startitem
            |prev_break| points to the passive node that should precede this one
            in an optimal path to this breakpoint.
        \stopitem

        \startitem
            |serial| is equal to |n| if this passive node is the |n|th one
            created during the current pass. (This field is used only when
            printing out detailed statistics about the line-breaking
            calculations.)
        \stopitem

        \startitem
            |passive_pen_inter| holds the current \.{\\localinterlinepenalty}
        \stopitem

        \startitem
            |passive_pen_broken| holds the current \.{\\localbrokenpenalty}
        \stopitem

    \stopitemize

    There is a global variable called |passive| that points to the most recently
    created passive node. Another global variable, |printed_node|, is used to
    help print out the paragraph when detailed information about the
    line-breaking computation is being displayed.

*/

/*tex most recent node on passive list */

static halfword passive;

/*tex most recent node that has been printed */

static halfword printed_node;

/*tex the number of passive nodes allocated on this pass */

static halfword pass_number;

/*tex

    The active list also contains ``delta'' nodes that help the algorithm compute
    the badness of individual lines. Such nodes appear only between two active
    nodes, and they have |type=delta_node|. If |p| and |r| are active nodes and
    if |q| is a delta node between them, so that |vlink(p)=q| and |vlink(q)=r|,
    then |q| tells the space difference between lines in the horizontal list that
    start after breakpoint |p| and lines that start after breakpoint |r|. In
    other words, if we know the length of the line that starts after |p| and ends
    at our current position, then the corresponding length of the line that
    starts after |r| is obtained by adding the amounts in node~|q|. A delta node
    contains seven scaled numbers, since it must record the net change in glue
    stretchability with respect to all orders of infinity. The natural width
    difference appears in |mem[q+1].sc|; the stretch differences in units of pt,
    sfi, fil, fill, and filll appear in |mem[q+2..q+6].sc|; and the shrink
    difference appears in |mem[q+7].sc|. The |subtype| field of a delta node is
    not used.

    Actually, we have two more fields that are used by |pdftex|.

    As the algorithm runs, it maintains a set of seven delta-like registers for
    the length of the line following the first active breakpoint to the current
    position in the given hlist. When it makes a pass through the active list, it
    also maintains a similar set of seven registers for the length following the
    active breakpoint of current interest. A third set holds the length of an
    empty line (namely, the sum of \.{\\leftskip} and \.{\\rightskip}); and a
    fourth set is used to create new delta nodes.

    When we pass a delta node we want to do operations like:

    \starttyping
    for k := 1 to 7 do
        cur_active_width[k] := cur_active_width[k] + mem[q+k].sc|};
    \stoptyping

    and we want to do this without the overhead of |for| loops. The |do_all_six|
    macro makes such six-tuples convenient.

*/

/*tex distance from first active node to~|cur_p| */

static scaled active_width[10] = { 0 };

/*tex length of an ``empty'' line */

static scaled background[10] = { 0 };

/*tex length being computed after current break */

static scaled break_width[10] = { 0 };

/*tex Make |auto_breaking| accessible out of |line_break|: */

static boolean auto_breaking;

/*tex

    Let's state the principles of the delta nodes more precisely and concisely,
    so that the following programs will be less obscure. For each legal
    breakpoint~|p| in the paragraph, we define two quantities $\alpha(p)$ and
    $\beta(p)$ such that the length of material in a line from breakpoint~|p| to
    breakpoint~|q| is $\gamma+\beta(q)-\alpha(p)$, for some fixed $\gamma$.
    Intuitively, $\alpha(p)$ and $\beta(q)$ are the total length of material from
    the beginning of the paragraph to a point ``after'' a break at |p| and to a
    point ``before'' a break at |q|; and $\gamma$ is the width of an empty line,
    namely the length contributed by \.{\\leftskip} and \.{\\rightskip}.

    Suppose, for example, that the paragraph consists entirely of alternating
    boxes and glue skips; let the boxes have widths $x_1\ldots x_n$ and let the
    skips have widths $y_1\ldots y_n$, so that the paragraph can be represented
    by $x_1y_1\ldots x_ny_n$. Let $p_i$ be the legal breakpoint at $y_i$; then
    $\alpha(p_i)=x_1+y_1+\cdots+x_i+y_i$, and $\beta(p_i)= x_1+y_1+\cdots+x_i$.
    To check this, note that the length of material from $p_2$ to $p_5$, say, is
    $\gamma+x_3+y_3+x_4+y_4+x_5=\gamma+\beta(p_5) -\alpha(p_2)$.

    The quantities $\alpha$, $\beta$, $\gamma$ involve glue stretchability and
    shrinkability as well as a natural width. If we were to compute $\alpha(p)$
    and $\beta(p)$ for each |p|, we would need multiple precision arithmetic, and
    the multiprecise numbers would have to be kept in the active nodes. \TeX\
    avoids this problem by working entirely with relative differences or
    ``deltas.'' Suppose, for example, that the active list contains
    $a_1\,\delta_1\,a_2\,\delta_2\,a_3$, where the |a|'s are active breakpoints
    and the $\delta$'s are delta nodes. Then $\delta_1=\alpha(a_1)-\alpha(a_2)$
    and $\delta_2=\alpha(a_2)-\alpha(a_3)$. If the line breaking algorithm is
    currently positioned at some other breakpoint |p|, the |active_width| array
    contains the value $\gamma+\beta(p)-\alpha(a_1)$. If we are scanning through
    the list of active nodes and considering a tentative line that runs from
    $a_2$ to~|p|, say, the |cur_active_width| array will contain the value
    $\gamma+\beta(p)-\alpha(a_2)$. Thus, when we move from $a_2$ to $a_3$, we
    want to add $\alpha(a_2)-\alpha(a_3)$ to |cur_active_width|; and this is just
    $\delta_2$, which appears in the active list between $a_2$ and $a_3$. The
    |background| array contains $\gamma$. The |break_width| array will be used to
    calculate values of new delta nodes when the active list is being updated.

    The heart of the line-breaking procedure is `|try_break|', a subroutine that
    tests if the current breakpoint |cur_p| is feasible, by running through the
    active list to see what lines of text can be made from active nodes
    to~|cur_p|. If feasible breaks are possible, new break nodes are created. If
    |cur_p| is too far from an active node, that node is deactivated.

    The parameter |pi| to |try_break| is the penalty associated with a break at
    |cur_p|; we have |pi=eject_penalty| if the break is forced, and
    |pi=inf_penalty| if the break is illegal.

    The other parameter, |break_type|, is set to |hyphenated_node| or
    |unhyphenated_node|, depending on whether or not the current break is at a
    |disc_node|. The end of a paragraph is also regarded as `|hyphenated_node|';
    this case is distinguishable by the condition |cur_p=null|.

*/

/*tex running \.{\\localinterlinepenalty} */

static int internal_pen_inter;

/*tex running \.{\\localbrokenpenalty} */

static int internal_pen_broken;

/*tex running \.{\\localleftbox} */

static halfword internal_left_box;

/*tex running \.{\\localleftbox} width */

static int internal_left_box_width;

/*tex running \.{\\localleftbox} */

static halfword init_internal_left_box;

/*tex running \.{\\localleftbox} width */

static int init_internal_left_box_width;

/*tex running \.{\\localrightbox} */

static halfword internal_right_box;

/*tex running \.{\\localrightbox} width */

static int internal_right_box_width;

/*tex the length of discretionary material preceding a break */

static scaled disc_width[10] = { 0 };

/*tex

    As we consider various ways to end a line at |cur_p|, in a given line number
    class, we keep track of the best total demerits known, in an array with one
    entry for each of the fitness classifications. For example,
    |minimal_demerits[tight_fit]| contains the fewest total demerits of feasible
    line breaks ending at |cur_p| with a |tight_fit| line;
    |best_place[tight_fit]| points to the passive node for the break
    before~|cur_p| that achieves such an optimum; and |best_pl_line[tight_fit]|
    is the |line_number| field in the active node corresponding to
    |best_place[tight_fit]|. When no feasible break sequence is known, the
    |minimal_demerits| entries will be equal to |awful_bad|, which is $2^{30}-1$.
    Another variable, |minimum_demerits|, keeps track of the smallest value in
    the |minimal_demerits| array.

*/

/*tex best total demerits known for current line class and position, given the fitness */

static int minimal_demerits[4];

/*tex best total demerits known for current line class and position */

static int minimum_demerits;

/*tex how to achieve  |minimal_demerits| */

static halfword best_place[4];

/*tex corresponding line number */

static halfword best_pl_line[4];

/*tex

    The length of lines depends on whether the user has specified \.{\\parshape}
    or \.{\\hangindent}. If |par_shape_ptr| is not null, it points to a
    $(2n+1)$-word record in |mem|, where the |vinfo| in the first word contains
    the value of |n|, and the other $2n$ words contain the left margins and line
    lengths for the first |n| lines of the paragraph; the specifications for line
    |n| apply to all subsequent lines. If |par_shape_ptr=null|, the shape of the
    paragraph depends on the value of |n=hang_after|; if |n>=0|, hanging
    indentation takes place on lines |n+1|, |n+2|, \dots, otherwise it takes
    place on lines 1, \dots, $\vert n\vert$. When hanging indentation is active,
    the left margin is |hang_indent|, if |hang_indent>=0|, else it is 0; the line
    length is $|hsize|-\vert|hang_indent|\vert$. The normal setting is
    |par_shape_ptr=null|, |hang_after=1|, and |hang_indent=0|. Note that if
    |hang_indent=0|, the value of |hang_after| is irrelevant.

*/

/*tex line numbers |>easy_line| are equivalent in break nodes */

static halfword easy_line;

/*tex line numbers |>last_special_line| all have the same width */

static halfword last_special_line;

/*tex the width of all lines |<=last_special_line|, if no \.{\\parshape} has been specified */

static scaled first_width;

/*tex the width of all lines |>last_special_line| */

static scaled second_width;

/*tex left margin to go with |first_width| */

static scaled first_indent;

/*tex left margin to go with |second_width| */

static scaled second_indent;

/*tex use this passive node and its predecessors */

static halfword best_bet;

/*tex the demerits associated with |best_bet| */

static int fewest_demerits;

/*tex line number following the last line of the new paragraph */

static halfword best_line;

/*tex the difference between |line_number(best_bet)| and the optimum |best_line| */

static int actual_looseness;

/*tex the difference between the current line number and the optimum |best_line| */

static int line_diff;

/*tex

    \TeX\ makes use of the fact that |hlist_node|, |vlist_node|, |rule_node|,
    |ins_node|, |mark_node|, |adjust_node|, |disc_node|, |whatsit_node|, and
    |math_node| are at the low end of the type codes, by permitting a break at
    glue in a list if and only if the |type| of the previous node is less than
    |math_node|. Furthermore, a node is discarded after a break if its type is
    |math_node| or~more.

*/

#define do_all_six(a) a(1);a(2);a(3);a(4);a(5);a(6);a(7)
#define do_seven_eight(a) if (adjust_spacing > 1) { a(8);a(9); }
#define do_all_eight(a) do_all_six(a); do_seven_eight(a)
#define do_one_seven_eight(a) a(1); do_seven_eight(a)

#define store_background(a) {active_width[a]=background[a];}

#define kern_break() { \
    if ((!is_char_node(vlink(cur_p))) && auto_breaking) \
        if (type(vlink(cur_p))==glue_node) \
            ext_try_break(\
                0, \
                unhyphenated_node, \
                line_break_dir, \
                adjust_spacing, \
                par_shape_ptr, \
                adj_demerits, \
                tracing_paragraphs, \
                protrude_chars, \
                line_penalty, \
                last_line_fit, \
                double_hyphen_demerits, \
                final_hyphen_demerits, \
                first_p, \
                cur_p \
            ); \
    if (type(cur_p)!=math_node) \
        active_width[1] += width(cur_p); \
    else \
        active_width[1] += surround(cur_p); \
}

#define clean_up_the_memory() { \
    q=vlink(active); \
    while (q!=active) { \
        cur_p = vlink(q); \
        if (type(q)==delta_node) \
            flush_node(q); \
        else \
            flush_node(q); \
        q = cur_p; \
    } \
    q = passive;  \
    while (q!=null) { \
        cur_p = vlink(q); \
        flush_node(q); \
        q = cur_p; \
    } \
}

/*tex special algorithm for last line of paragraph? */

static boolean do_last_line_fit;

/*tex infinite stretch components of  |par_fill_skip| */

static scaled fill_width[4];

/*tex |shortfall|  corresponding to |minimal_demerits| */

static scaled best_pl_short[4];

/*tex corresponding glue stretch or shrink */

static scaled best_pl_glue[4];

#define reset_disc_width(a) disc_width[(a)] = 0

#define add_disc_width_to_break_width(a)     break_width[(a)] += disc_width[(a)]
#define sub_disc_width_from_active_width(a)  active_width[(a)] -= disc_width[(a)]

#define add_char_shrink(a,b)  a += char_shrink((b))
#define add_char_stretch(a,b) a += char_stretch((b))
#define sub_char_shrink(a,b)  a -= char_shrink((b))
#define sub_char_stretch(a,b) a -= char_stretch((b))

#define add_kern_shrink(a,b)  a += kern_shrink((b))
#define add_kern_stretch(a,b) a += kern_stretch((b))
#define sub_kern_shrink(a,b)  a -= kern_shrink((b))
#define sub_kern_stretch(a,b) a -= kern_stretch((b))

/*tex

    This function is used to add the width of a list of nodes (from a
    discretionary) to one of the width arrays.

    Replacement texts and discretionary texts are supposed to contain only
    character nodes, kern nodes, and box or rule nodes.

*/

#define bad_node_in_disc_error(p) { \
    if (type(p) == whatsit_node) { \
        formatted_error("linebreak","invalid node with type %s and subtype %i found in discretionary",node_data[type(p)].name,subtype(p)); \
    } else { \
        formatted_error("linebreak","invalid node with type %s found in discretionary",node_data[type(p)].name); \
    } \
}

static void add_to_widths(halfword s, int line_break_dir, int adjust_spacing, scaled * widths)
{
    while (s != null) {
        if (is_char_node(s)) {
            widths[1] += pack_width(line_break_dir, dir_TRT, s, true);
            if ((adjust_spacing > 1) && check_expand_pars(font(s))) {
                set_prev_char_p(s);
                add_char_stretch(widths[8], s);
                add_char_shrink(widths[9], s);
            };
        } else {
            switch (type(s)) {
                case hlist_node:
                case vlist_node:
                    widths[1] += pack_width(line_break_dir, box_dir(s), s, false);
                    break;
                case kern_node:
                    if ((adjust_spacing == 2) && (subtype(s) == normal)) {
                        add_kern_stretch(widths[8], s);
                        add_kern_shrink(widths[9], s);
                    }
                    /*tex fall through */
                case rule_node:
                    widths[1] += width(s);
                    break;
                case disc_node:
                    break;
                default:
                    bad_node_in_disc_error(s);
                    break;
            }
        }
        s = vlink(s);
    }
}

/*tex

    This function is used to substract the width of a list of nodes (from a
    discretionary) from one of the width arrays. It is used only once, but
    deserves it own function because of orthogonality with the |add_to_widths|
    function.

*/

static void sub_from_widths(halfword s, int line_break_dir, int adjust_spacing, scaled * widths)
{
    while (s != null) {
        /*tex Subtract the width of node |s| from |break_width|; */
        if (is_char_node(s)) {
            widths[1] -= pack_width(line_break_dir, dir_TRT, s, true);
            if ((adjust_spacing > 1) && check_expand_pars(font(s))) {
                set_prev_char_p(s);
                sub_char_stretch(widths[8], s);
                sub_char_shrink(widths[9], s);
            }
        } else {
            switch (type(s)) {
                case hlist_node:
                case vlist_node:
                    widths[1] -= pack_width(line_break_dir, box_dir(s), s, false);
                    break;
                case kern_node:
                    if ((adjust_spacing == 2) && (subtype(s) == normal)) {
                        sub_kern_stretch(widths[8], s);
                        sub_kern_shrink(widths[9], s);
                    }
                    /*tex fall through */
                case rule_node:
                    widths[1] -= width(s);
                    break;
                case disc_node:
                    break;
                default:
                    bad_node_in_disc_error(s);
                    break;
            }
        }
        s = vlink(s);
    }
}

/*tex

    When we insert a new active node for a break at |cur_p|, suppose this new
    node is to be placed just before active node |a|; then we essentially want to
    insert `$\delta\,|cur_p|\,\delta^\prime$' before |a|, where
    $\delta=\alpha(a)-\alpha(|cur_p|)$ and
    $\delta^\prime=\alpha(|cur_p|)-\alpha(a)$ in the notation explained above.
    The |cur_active_width| array now holds $\gamma+\beta(|cur_p|)-\alpha(a)$; so
    $\delta$ can be obtained by subtracting |cur_active_width| from the quantity
    $\gamma+\beta(|cur_p|)- \alpha(|cur_p|)$. The latter quantity can be regarded
    as the length of a line ``from |cur_p| to |cur_p|''; we call it the
    |break_width| at |cur_p|.

    The |break_width| is usually negative, since it consists of the background
    (which is normally zero) minus the width of nodes following~|cur_p| that are
    eliminated after a break. If, for example, node |cur_p| is a glue node, the
    width of this glue is subtracted from the background; and we also look ahead
    to eliminate all subsequent glue and penalty and kern and math nodes,
    subtracting their widths as well.

    Kern nodes do not disappear at a line break unless they are |explicit|.

*/

static void compute_break_width(int break_type, int line_break_dir, int adjust_spacing, halfword p)
{
    /*tex

        Glue and other 'whitespace' to be skipped after a break; used if
        unhyphenated, or |post_break==empty|.

    */
    halfword s = p;
    if (break_type > unhyphenated_node && p != null) {
        /*tex

            Compute the discretionary |break_width| values.

            When |p| is a discretionary break, the length of a line ``from |p| to
            |p|'' has to be defined properly so that the other calculations work
            out. Suppose that the pre-break text at |p| has length $l_0$, the
            post-break text has length $l_1$, and the replacement text has length
            |l|. Suppose also that |q| is the node following the replacement
            text. Then length of a line from |p| to |q| will be computed as
            $\gamma+\beta(q)-\alpha(|p|)$, where $\beta(q)=\beta(|p|)-l_0+l$. The
            actual length will be the background plus $l_1$, so the length from
            |p| to |p| should be $\gamma+l_0+l_1-l$. If the post-break text of
            the discretionary is empty, a break may also discard~|q|; in that
            unusual case we subtract the length of~|q| and any other nodes that
            will be discarded after the discretionary break.

            The value of $l_0$ need not be computed, since |line_break| will put
            it into the global variable |disc_width| before calling |try_break|.

            In case of nested discretionaries, we always follow the no-break
            path, as we are talking about the breaking on {\it this} position.

        */
        sub_from_widths(vlink_no_break(p), line_break_dir, adjust_spacing, break_width);
        add_to_widths(vlink_post_break(p), line_break_dir, adjust_spacing, break_width);
        do_one_seven_eight(add_disc_width_to_break_width);
        if (vlink_post_break(p) == null) {
            /*tex no |post_break|: 'skip' any 'whitespace' following */
            s = vlink(p);
        } else {
            s = null;
        }
    }
    while (s != null) {
        switch (type(s)) {
            case math_node:
                /*tex begin mathskip code */
                if (glue_is_zero(s)) {
                    break_width[1] -= surround(s);
                    break;
                } else {
                    /*tex fall through */
                }
                /*tex end mathskip code */
            case glue_node:
                /*tex Subtract glue from |break_width|; */
                break_width[1] -= width(s);
                break_width[2 + stretch_order(s)] -= stretch(s);
                break_width[7] -= shrink(s);
                break;
            case penalty_node:
                break;
            case kern_node:
                if (subtype(s) != explicit_kern && subtype(s) != italic_kern)
                    return;
                else
                    break_width[1] -= width(s);
                break;
            default:
                return;
        };
        s = vlink(s);
    }
}

static void print_break_node(halfword q, fitness_value fit_class, quarterword break_type, halfword cur_p)
{
    /*tex Print a symbolic description of the new break node. */
    tprint_nl("@@");
    print_int(serial(passive));
    tprint(": line ");
    print_int(line_number(q) - 1);
    print_char('.');
    print_int(fit_class);
    if (break_type == hyphenated_node)
        print_char('-');
    tprint(" t=");
    print_int(total_demerits(q));
    if (do_last_line_fit) {
        /*tex Print additional data in the new active node. */
        tprint(" s=");
        print_scaled(active_short(q));
        if (cur_p == null)
            tprint(" a=");
        else
            tprint(" g=");
        print_scaled(active_glue(q));
    }
    tprint(" -> @");
    if (prev_break(passive) == null)
        print_char('0');
    else
        print_int(serial(prev_break(passive)));
}

static void print_feasible_break(halfword cur_p, pointer r, halfword b, int pi, int d, boolean artificial_demerits)
{
    /*tex

        Print a symbolic description of this feasible break.

    */
    if (printed_node != cur_p) {
        /*tex

            Print the list between |printed_node| and |cur_p|, then set
            |printed_node:=cur_p|.

        */
        tprint_nl("");
        if (cur_p == null) {
            short_display(vlink(printed_node));
        } else {
            halfword save_link = vlink(cur_p);
            vlink(cur_p) = null;
            tprint_nl("");
            short_display(vlink(printed_node));
            vlink(cur_p) = save_link;
        }
        printed_node = cur_p;
    }
    tprint_nl("@");
    if (cur_p == null) {
        tprint_esc("par");
    } else if (type(cur_p) != glue_node) {
        if (type(cur_p) == penalty_node)
            tprint_esc("penalty");
        else if (type(cur_p) == disc_node)
            tprint_esc("discretionary");
        else if (type(cur_p) == kern_node)
            tprint_esc("kern");
        else
            tprint_esc("math");
    }
    tprint(" via @");
    if (break_node(r) == null)
        print_char('0');
    else
        print_int(serial(break_node(r)));
    tprint(" b=");
    if (b > inf_bad)
        print_char('*');
    else
        print_int(b);
    tprint(" p=");
    print_int(pi);
    tprint(" d=");
    if (artificial_demerits)
        print_char('*');
    else
        print_int(d);
}

#define add_disc_width_to_active_width(a)   active_width[a] += disc_width[a]
#define update_width(a) cur_active_width[a] += varmem[(r+(a))].cint

#define set_break_width_to_background(a) break_width[a]=background[(a)]

#define convert_to_break_width(a) \
  varmem[(prev_r+(a))].cint = varmem[(prev_r+(a))].cint-cur_active_width[(a)]+break_width[(a)]

#define store_break_width(a) active_width[(a)]=break_width[(a)]

#define new_delta_to_break_width(a) \
  varmem[(q+(a))].cint=break_width[(a)]-cur_active_width[(a)]

#define new_delta_from_break_width(a) \
  varmem[(q+(a))].cint=cur_active_width[(a)]-break_width[(a)]

#define copy_to_cur_active(a) cur_active_width[(a)]=active_width[(a)]

#define combine_two_deltas(a) varmem[(prev_r+(a))].cint += varmem[(r+(a))].cint
#define downdate_width(a) cur_active_width[(a)] -= varmem[(prev_r+(a))].cint
#define update_active(a) active_width[(a)]+=varmem[(r+(a))].cint

#define total_font_stretch cur_active_width[8]
#define total_font_shrink cur_active_width[9]

#define cal_margin_kern_var(a) { \
    character(cp) = character((a)); \
    font(cp) = font((a)); \
    do_subst_font(cp, 1000); \
    if (font(cp) != font((a))) \
        margin_kern_stretch += (left_pw((a)) - left_pw(cp)); \
    font(cp) = font((a)); \
    do_subst_font(cp, -1000); \
    if (font(cp) != font((a))) \
        margin_kern_shrink += (left_pw(cp) - left_pw((a))); \
}

static void ext_try_break(
    int pi,
    quarterword break_type,
    int line_break_dir,
    int adjust_spacing,
    int par_shape_ptr,
    int adj_demerits,
    int tracing_paragraphs,
    int protrude_chars,
    int line_penalty,
    int last_line_fit,
    int double_hyphen_demerits,
    int final_hyphen_demerits, halfword first_p, halfword cur_p
)
{
    /*tex runs through the active list */
    pointer r;
    scaled margin_kern_stretch;
    scaled margin_kern_shrink;
    halfword lp, rp, cp;
    /*tex stays a step behind |r| */
    halfword prev_r = active;
    /*tex a step behind |prev_r|, if |type(prev_r)=delta_node| */
    halfword prev_prev_r = null;
    /*tex maximum line number in current equivalence class of lines */
    halfword old_l = 0;
    /*tex have we found a feasible break at |cur_p|? */
    boolean no_break_yet = true;
    /*tex points to a new node being created */
    halfword q;
    /*tex line number of current active node */
    halfword l;
    /*tex should node |r| remain in the active list? */
    boolean node_r_stays_active;
    /*tex the current line will be justified to this width */
    scaled line_width = 0;
    /*tex possible fitness class of test line */
    fitness_value fit_class;
    /*tex badness of test line */
    halfword b;
    /*tex demerits of test line */
    int d;
    /*tex has |d| been forced to zero? */
    boolean artificial_demerits;
    /*tex used in badness calculations */
    scaled shortfall;
    /*tex glue stretch or shrink of test line, adjustment for last line */
    scaled g = 0;
    /*tex distance from current active node */
    scaled cur_active_width[10] = { 0 };
    /*tex Make sure that |pi| is in the proper range; */
    if (pi >= inf_penalty) {
        /*tex this breakpoint is inhibited by infinite penalty */
        return;
    } else if (pi <= -inf_penalty) {
        /*tex this breakpoint will be forced */
        pi = eject_penalty;
    }

    do_all_eight(copy_to_cur_active);

    while (1) {
        r = vlink(prev_r);
        /*tex

            If node |r| is of type |delta_node|, update |cur_active_width|, set
            |prev_r| and |prev_prev_r|, then |goto continue|. The following code
            uses the fact that |type(active)<>delta_node|.

        */
        if (type(r) == delta_node) {
            /*tex implicit */
            do_all_eight(update_width);
            prev_prev_r = prev_r;
            prev_r = r;
            continue;
        }
        /*tex

            If a line number class has ended, create new active nodes for the
            best feasible breaks in that class; then |return| if |r=active|,
            otherwise compute the new |line_width|.

            The first part of the following code is part of \TeX's inner loop, so
            we don't want to waste any time. The current active node, namely node
            |r|, contains the line number that will be considered next. At the
            end of the list we have arranged the data structure so that
            |r=active| and |line_number(active)>old_l|.

        */
        l = line_number(r);
        if (l > old_l) {
            /*tex now we are no longer in the inner loop */
            if ((minimum_demerits < awful_bad)
                && ((old_l != easy_line) || (r == active))) {
                /*tex

                    Create new active nodes for the best feasible breaks just
                    found. It is not necessary to create new active nodes having
                    |minimal_demerits| greater than
                    |minimum_demerits+abs(adj_demerits)|, since such active nodes
                    will never be chosen in the final paragraph breaks. This
                    observation allows us to omit a substantial number of
                    feasible breakpoints from further consideration.

                */
                if (no_break_yet) {
                    no_break_yet = false;
                    do_all_eight(set_break_width_to_background);
                    compute_break_width(break_type, line_break_dir, adjust_spacing, cur_p);
                }
                /*tex

                    Insert a delta node to prepare for breaks at |cur_p|. We use
                    the fact that |type(active)<>delta_node|.

                */
                if (type(prev_r) == delta_node) {
                    /*tex modify an existing delta node */
                    do_all_eight(convert_to_break_width);
                } else if (prev_r == active) {
                    /*tex no delta node needed at the beginning */
                    do_all_eight(store_break_width);
                } else {
                    q = new_node(delta_node, 0);
                    vlink(q) = r;
                    do_all_eight(new_delta_to_break_width);
                    vlink(prev_r) = q;
                    prev_prev_r = prev_r;
                    prev_r = q;
                }
                if (abs(adj_demerits) >= awful_bad - minimum_demerits)
                    minimum_demerits = awful_bad - 1;
                else
                    minimum_demerits += abs(adj_demerits);
                for (fit_class = very_loose_fit; fit_class <= tight_fit;
                     fit_class++) {
                    if (minimal_demerits[fit_class] <= minimum_demerits) {
                        /*tex

                            Insert a new active node from |best_place[fit_class]|
                            to |cur_p|. When we create an active node, we also
                            create the corresponding passive node.

                        */
                        q = new_node(passive_node, 0);
                        vlink(q) = passive;
                        passive = q;
                        cur_break(q) = cur_p;
                        incr(pass_number);
                        serial(q) = pass_number;
                        prev_break(q) = best_place[fit_class];
                        /*tex

                            Here we keep track of the subparagraph penalties in
                            the break nodes.

                        */
                        passive_pen_inter(q) = internal_pen_inter;
                        passive_pen_broken(q) = internal_pen_broken;
                        passive_last_left_box(q) = internal_left_box;
                        passive_last_left_box_width(q) =
                            internal_left_box_width;
                        if (prev_break(q) != null) {
                            passive_left_box(q) = passive_last_left_box(prev_break(q));
                            passive_left_box_width(q) = passive_last_left_box_width(prev_break(q));
                        } else {
                            passive_left_box(q) = init_internal_left_box;
                            passive_left_box_width(q) = init_internal_left_box_width;
                        }
                        passive_right_box(q) = internal_right_box;
                        passive_right_box_width(q) = internal_right_box_width;
                        q = new_node(break_type, fit_class);
                        break_node(q) = passive;
                        line_number(q) = best_pl_line[fit_class] + 1;
                        total_demerits(q) = minimal_demerits[fit_class];
                        if (do_last_line_fit) {
                            /*tex

                                Store additional data in the new active node.
                                Here we save these data in the active node
                                representing a potential line break.

                            */
                            active_short(q) = best_pl_short[fit_class];
                            active_glue(q) = best_pl_glue[fit_class];
                        }
                        vlink(q) = r;
                        vlink(prev_r) = q;
                        prev_r = q;
                        if (tracing_paragraphs > 0)
                            print_break_node(q, fit_class, break_type, cur_p);
                    }
                    minimal_demerits[fit_class] = awful_bad;
                }
                minimum_demerits = awful_bad;
                /*tex

                    Insert a delta node to prepare for the next active node. When
                    the following code is performed, we will have just inserted
                    at least one active node before |r|, so
                    |type(prev_r)<>delta_node|.

                */
                if (r != active) {
                    q = new_node(delta_node, 0);
                    vlink(q) = r;
                    do_all_eight(new_delta_from_break_width);
                    vlink(prev_r) = q;
                    prev_prev_r = prev_r;
                    prev_r = q;
                }
            }
            if (r == active)
                return;
            /*tex

                Compute the new line width. When we come to the following code,
                we have just encountered the first active node~|r| whose
                |line_number| field contains |l|. Thus we want to compute the
                length of the $l\mskip1mu$th line of the current paragraph.
                Furthermore, we want to set |old_l| to the last number in the
                class of line numbers equivalent to~|l|.

            */
            if (l > easy_line) {
                old_l = max_halfword - 1;
                line_width = second_width;
            } else {
                old_l = l;
                if (l > last_special_line) {
                    line_width = second_width;
                } else if (par_shape_ptr == null) {
                    line_width = first_width;
                } else {
                    line_width = varmem[(par_shape_ptr + 2 * l + 1)].cint;
                }
            }
        }
        /*tex

            If a line number class has ended, create new active nodes for the
            best feasible breaks in that class; then |return| if |r=active|,
            otherwise compute the new |line_width|.

            Consider the demerits for a line from |r| to |cur_p|; deactivate node
            |r| if it should no longer be active; then |goto continue| if a line
            from |r| to |cur_p| is infeasible, otherwise record a new feasible
            break.

        */
        artificial_demerits = false;
        shortfall = line_width - cur_active_width[1];
        if (break_node(r) == null)
            shortfall -= init_internal_left_box_width;
        else
            shortfall -= passive_last_left_box_width(break_node(r));
        shortfall -= internal_right_box_width;
        if (protrude_chars > 1) {
            halfword l1, o;
            l1 = (break_node(r) == null) ? first_p : cur_break(break_node(r));
            if (cur_p == null) {
                o = null;
            } else {
                o = alink(cur_p);
                assert(vlink(o) == cur_p);
            }
            /*tex

                The disc could be a SELECT subtype, to we might need to get the
                last character as |pre_break| from either the |pre_break| list
                (if the previous INIT disc was taken), or the |no_break| (sic)
                list (if the previous INIT disc was not taken).

                The last characters (hyphenation character) if these two list
                should always be the same anyway, so we just look at |pre_break|.

                Let's look at the right margin first.

            */
            if ((cur_p != null) && (type(cur_p) == disc_node) && (vlink_pre_break(cur_p) != null)) {
                /*tex a |disc_node| with non-empty |pre_break|, protrude the last char of |pre_break| */
                o = tlink_pre_break(cur_p);
            } else {
                o = find_protchar_right(l1, o);
            }
            /*tex now the left margin */
            if ((l1 != null) && (type(l1) == disc_node) && (vlink_post_break(l1) != null)) {
                /*tex The first char could be a disc! Protrude the first char. */
                l1 = vlink_post_break(l1);
            } else {
                l1 = find_protchar_left(l1, true);
            }
            shortfall += (left_pw(l1) + right_pw(o));
        }
        if (shortfall != 0) {
            margin_kern_stretch = 0;
            margin_kern_shrink = 0;
            if (protrude_chars > 1) {
                /*tex Calculate variations of marginal kerns. */
                lp = last_leftmost_char;
                rp = last_rightmost_char;
                cp = raw_glyph_node();
                if (lp != null) {
                    cal_margin_kern_var(lp);
                }
                if (rp != null) {
                    cal_margin_kern_var(rp);
                }
                flush_node(cp);
            }
            if ((shortfall > 0) && ((total_font_stretch + margin_kern_stretch) > 0)) {
                if ((total_font_stretch + margin_kern_stretch) > shortfall)
                    shortfall = ((total_font_stretch + margin_kern_stretch) / (max_stretch_ratio / cur_font_step)) / 2;
                else
                    shortfall -= (total_font_stretch + margin_kern_stretch);
            } else if ((shortfall < 0) && ((total_font_shrink + margin_kern_shrink) > 0)) {
                if ((total_font_shrink + margin_kern_shrink) > -shortfall)
                    shortfall = -((total_font_shrink + margin_kern_shrink) / (max_shrink_ratio / cur_font_step)) / 2;
                else
                    shortfall += (total_font_shrink + margin_kern_shrink);
            }
        }
        if (shortfall > 0) {
            /*tex

                Set the value of |b| to the badness for stretching the line, and
                compute the corresponding |fit_class|.

                When a line must stretch, the available stretchability can be
                found in the subarray |cur_active_width[2..6]|, in units of
                points, sfi, fil, fill and filll.

                The present section is part of \TeX's inner loop, and it is most
                often performed when the badness is infinite; therefore it is
                worth while to make a quick test for large width excess and small
                stretchability, before calling the |badness| subroutine.

            */
            if ((cur_active_width[3] != 0) || (cur_active_width[4] != 0) ||
                (cur_active_width[5] != 0) || (cur_active_width[6] != 0)) {
                if (do_last_line_fit) {
                    if (cur_p == null) {
                        /*tex

                            The last line of a paragraph. Perform computations
                            for last line and |goto found|.

                            Here we compute the adjustment |g| and badness |b|
                            for a line from |r| to the end of the paragraph. When
                            any of the criteria for adjustment is violated we
                            fall through to the normal algorithm.

                            The last line must be too short, and have infinite
                            stretch entirely due to |par_fill_skip|.

                        */
                        if ((active_short(r) == 0) || (active_glue(r) <= 0))
                            /*tex

                                Previous line was neither stretched nor shrunk,
                                or was infinitely bad.

                            */
                            goto NOT_FOUND;
                        if ((cur_active_width[3] != fill_width[0]) || (cur_active_width[4] != fill_width[1]) ||
                            (cur_active_width[5] != fill_width[2]) || (cur_active_width[6] != fill_width[3]))
                            /*tex

                                Infinite stretch of this line not entirely due to |par_fill_skip|.

                            */
                            goto NOT_FOUND;
                        if (active_short(r) > 0)
                            g = cur_active_width[2];
                        else
                            g = cur_active_width[7];
                        if (g <= 0)
                            /*tex No finite stretch resp.\ no shrink. */
                            goto NOT_FOUND;
                        arith_error = false;
                        g = fract(g, active_short(r), active_glue(r),
                                  max_dimen);
                        if (last_line_fit < 1000)
                            g = fract(g, last_line_fit, 1000, max_dimen);
                        if (arith_error) {
                            if (active_short(r) > 0)
                                g = max_dimen;
                            else
                                g = -max_dimen;
                        }
                        if (g > 0) {
                            /*tex

                                Set the value of |b| to the badness of the last
                                line for stretching, compute the corresponding
                                |fit_class, and |goto found|. These badness
                                computations are rather similar to those of the
                                standard algorithm, with the adjustment amount
                                |g| replacing the |shortfall|.

                            */
                            if (g > shortfall)
                                g = shortfall;
                            if (g > 7230584) {
                                if (cur_active_width[2] < 1663497) {
                                    b = inf_bad;
                                    fit_class = very_loose_fit;
                                    goto FOUND;
                                }
                            }
                            b = badness(g, cur_active_width[2]);
                            if (b > 99) {
                                fit_class = very_loose_fit;
                            } else if (b > 12) {
                                fit_class = loose_fit;
                            } else {
                                fit_class = decent_fit;
                            }
                            goto FOUND;
                        } else if (g < 0) {
                            /*tex

                                Set the value of |b| to the badness of the last
                                line for shrinking, compute the corresponding
                                |fit_class, and |goto found||.

                            */
                            if (-g > cur_active_width[7])
                                g = -cur_active_width[7];
                            b = badness(-g, cur_active_width[7]);
                            if (b > 12)
                                fit_class = tight_fit;
                            else
                                fit_class = decent_fit;
                            goto FOUND;
                        }
                    }
                  NOT_FOUND:
                    shortfall = 0;
                }
                b = 0;
                /*tex Infinite stretch. */
                fit_class = decent_fit;
            } else if (shortfall > 7230584 && cur_active_width[2] < 1663497) {
                b = inf_bad;
                fit_class = very_loose_fit;
            } else {
                b = badness(shortfall, cur_active_width[2]);
                if (b > 99) {
                    fit_class = very_loose_fit;
                } else if (b > 12) {
                    fit_class = loose_fit;
                } else {
                    fit_class = decent_fit;
                }
            }
        } else {
            /*tex

                Set the value of |b| to the badness for shrinking the line, and
                compute the corresponding |fit_class|. Shrinkability is never
                infinite in a paragraph; we can shrink the line from |r| to
                |cur_p| by at most |cur_active_width[7]|.

            */
            if (-shortfall > cur_active_width[7])
                b = inf_bad + 1;
            else
                b = badness(-shortfall, cur_active_width[7]);
            if (b > 12)
                fit_class = tight_fit;
            else
                fit_class = decent_fit;
        }
        if (do_last_line_fit) {
            /*tex Adjust the additional data for last line; */
            if (cur_p == null)
                shortfall = 0;
            if (shortfall > 0) {
                g = cur_active_width[2];
            } else if (shortfall < 0) {
                g = cur_active_width[7];
            } else {
                g = 0;
            }
        }
      FOUND:
        if ((b > inf_bad) || (pi == eject_penalty)) {
            /*tex

                Prepare to deactivate node~|r|, and |goto deactivate| unless
                there is a reason to consider lines of text from |r| to |cur_p|.
                During the final pass, we dare not lose all active nodes, lest we
                lose touch with the line breaks already found. The code shown
                here makes sure that such a catastrophe does not happen, by
                permitting overfull boxes as a last resort. This particular part
                of \TeX\ was a source of several subtle bugs before the correct
                program logic was finally discovered; readers who seek to
                ``improve'' \TeX\ should therefore think thrice before daring to
                make any changes here.

            */
            if (final_pass && (minimum_demerits == awful_bad) &&
                (vlink(r) == active) && (prev_r == active)) {
                /*tex Set demerits zero, this break is forced. */
                artificial_demerits = true;
            } else if (b > threshold) {
                goto DEACTIVATE;
            }
            node_r_stays_active = false;
        } else {
            prev_r = r;
            if (b > threshold)
                continue;
            node_r_stays_active = true;
        }
        /*tex

            Record a new feasible break. When we get to this part of the code,
            the line from |r| to |cur_p| is feasible, its badness is~|b|, and its
            fitness classification is |fit_class|. We don't want to make an
            active node for this break yet, but we will compute the total
            demerits and record them in the |minimal_demerits| array, if such a
            break is the current champion among all ways to get to |cur_p| in a
            given line-number class and fitness class.

        */
        if (artificial_demerits) {
            d = 0;
        } else {
            /*tex Compute the demerits, |d|, from |r| to |cur_p|. */
            d = line_penalty + b;
            if (abs(d) >= 10000)
                d = 100000000;
            else
                d = d * d;
            if (pi != 0) {
                if (pi > 0) {
                    d += (pi * pi);
                } else if (pi > eject_penalty) {
                    d -= (pi * pi);
                }
            }
            if ((break_type == hyphenated_node) && (type(r) == hyphenated_node)) {
                if (cur_p != null)
                    d += double_hyphen_demerits;
                else
                    d += final_hyphen_demerits;
            }
            if (abs(fit_class - fitness(r)) > 1)
                d = d + adj_demerits;
        }
        if (tracing_paragraphs > 0) {
            print_feasible_break(cur_p, r, b, pi, d, artificial_demerits);
        }
        /*tex This is the minimum total demerits from the beginning to |cur_p| via |r|. */
        d += total_demerits(r);
        if (d <= minimal_demerits[fit_class]) {
            minimal_demerits[fit_class] = d;
            best_place[fit_class] = break_node(r);
            best_pl_line[fit_class] = l;
            if (do_last_line_fit) {
                /*tex

                    Store additional data for this feasible break. For each
                    feasible break we record the shortfall and glue stretch or
                    shrink (or adjustment).

                */
                best_pl_short[fit_class] = shortfall;
                best_pl_glue[fit_class] = g;
            }
            if (d < minimum_demerits)
                minimum_demerits = d;
        }
        /*tex Record a new feasible break. */
        if (node_r_stays_active) {
            /*tex |prev_r| has been set to |r|. */
            continue;
        }
      DEACTIVATE:
        /*tex

            Deactivate node |r|. When an active node disappears, we must delete
            an adjacent delta node if the active node was at the beginning or the
            end of the active list, or if it was surrounded by delta nodes. We
            also must preserve the property that |cur_active_width| represents
            the length of material from |vlink(prev_r)| to~|cur_p|.

        */
        vlink(prev_r) = vlink(r);
        flush_node(r);
        if (prev_r == active) {
            /*tex

                Update the active widths, since the first active node has been
                deleted. The following code uses the fact that
                |type(active)<>delta_node|. If the active list has just become
                empty, we do not need to update the |active_width| array, since
                it will be initialized when an active node is next inserted.

            */
            r = vlink(active);
            if (type(r) == delta_node) {
                do_all_eight(update_active);
                do_all_eight(copy_to_cur_active);
                vlink(active) = vlink(r);
                flush_node(r);
            }
        } else if (type(prev_r) == delta_node) {
            r = vlink(prev_r);
            if (r == active) {
                do_all_eight(downdate_width);
                vlink(prev_prev_r) = active;
                flush_node(prev_r);
                prev_r = prev_prev_r;
            } else if (type(r) == delta_node) {
                do_all_eight(update_width);
                do_all_eight(combine_two_deltas);
                vlink(prev_r) = vlink(r);
                flush_node(r);
            }
        }
    }
}

void ext_do_line_break(
    int paragraph_dir,
    int pretolerance,
    int tracing_paragraphs,
    int tolerance,
    scaled emergency_stretch,
    int looseness,
    int adjust_spacing,
    halfword par_shape_ptr,
    int adj_demerits,
    int protrude_chars,
    int line_penalty,
    int last_line_fit,
    int double_hyphen_demerits,
    int final_hyphen_demerits,
    int hang_indent,
    int hsize,
    int hang_after,
    halfword left_skip,
    halfword right_skip,
    halfword inter_line_penalties_ptr,
    int inter_line_penalty,
    int club_penalty,
    halfword club_penalties_ptr,
    halfword widow_penalties_ptr,
    int widow_penalty,
    int broken_penalty,
    halfword final_par_glue
)
{
    /*tex Miscellaneous nodes of temporary interest. */
    halfword cur_p, q, r, s;
    int line_break_dir = paragraph_dir;
    /*tex Get ready to start */
    minimum_demerits = awful_bad;
    minimal_demerits[tight_fit] = awful_bad;
    minimal_demerits[decent_fit] = awful_bad;
    minimal_demerits[loose_fit] = awful_bad;
    minimal_demerits[very_loose_fit] = awful_bad;
    fewest_demerits = 0;
    actual_looseness = 0;
    /*tex

        We compute the values of |easy_line| and the other local variables
        relating to line length when the |line_break| procedure is initializing
        itself.

    */
    if (par_shape_ptr == null) {
        if (hang_indent == 0) {
            last_special_line = 0;
            second_width = hsize;
            second_indent = 0;
        } else {
            halfword used_hang_indent = swap_hang_indent(hang_indent);
            /*tex

                Set line length parameters in preparation for hanging
                indentation. We compute the values of |easy_line| and the other
                local variables relating to line length when the |line_break|
                procedure is initializing itself.

            */
            last_special_line = abs(hang_after);
            if (hang_after < 0) {
                first_width = hsize - abs(used_hang_indent);
                if (used_hang_indent >= 0)
                    first_indent = used_hang_indent;
                else
                    first_indent = 0;
                second_width = hsize;
                second_indent = 0;
            } else {
                first_width = hsize;
                first_indent = 0;
                second_width = hsize - abs(used_hang_indent);
                if (used_hang_indent >= 0)
                    second_indent = used_hang_indent;
                else
                    second_indent = 0;
            }
        }
    } else {
        last_special_line = vinfo(par_shape_ptr + 1) - 1;
        second_indent = varmem[(par_shape_ptr + 2 * (last_special_line + 1))].cint;
        second_width = varmem[(par_shape_ptr + 2 * (last_special_line + 1) + 1)].cint;
        second_indent = swap_parshape_indent(second_indent,second_width);
    }
    if (looseness == 0)
        easy_line = last_special_line;
    else
        easy_line = max_halfword;
    no_shrink_error_yet = true;
    check_shrinkage(left_skip);
    check_shrinkage(right_skip);
    q = left_skip;
    r = right_skip;
    background[1] = width(q) + width(r);
    background[2] = 0;
    background[3] = 0;
    background[4] = 0;
    background[5] = 0;
    background[6] = 0;
    background[2 + stretch_order(q)] = stretch(q);
    background[2 + stretch_order(r)] += stretch(r);
    background[7] = shrink(q) + shrink(r);
    if (adjust_spacing > 1) {
        background[8] = 0;
        background[9] = 0;
        max_stretch_ratio = -1;
        max_shrink_ratio = -1;
        cur_font_step = -1;
        set_prev_char_p(null);
    }
    /*tex

        Check for special treatment of last line of paragraph. The new algorithm
        for the last line requires that the stretchability |par_fill_skip| is
        infinite and the stretchability of |left_skip| plus |right_skip| is
        finite.

    */
    do_last_line_fit = false;
    if (last_line_fit > 0) {
        q = last_line_fill;
        if ((stretch(q) > 0) && (stretch_order(q) > normal)) {
            if ((background[3] == 0) && (background[4] == 0) && (background[5] == 0) && (background[6] == 0)) {
                do_last_line_fit = true;
                fill_width[0] = 0;
                fill_width[1] = 0;
                fill_width[2] = 0;
                fill_width[3] = 0;
                fill_width[stretch_order(q) - 1] = stretch(q);
            }
        }
    }
    /*tex Initialize |dir_ptr| for |line_break|. */
    if (dir_ptr != null) {
        flush_node_list(dir_ptr);
        dir_ptr = null;
    }
    /*tex Find optimal breakpoints. */
    threshold = pretolerance;
    if (threshold >= 0) {
        if (tracing_paragraphs > 0) {
            begin_diagnostic();
            tprint_nl("@firstpass");
        }
        second_pass = false;
        final_pass = false;
    } else {
        threshold = tolerance;
        second_pass = true;
        final_pass = (emergency_stretch <= 0);
        if (tracing_paragraphs > 0)
            begin_diagnostic();
    }
    while (1) {
        halfword first_p;
        halfword nest_stack[10];
        int nest_index = 0;
        if (threshold > inf_bad)
            threshold = inf_bad;
        /*tex Create an active breakpoint representing the beginning of the paragraph. */
        q = new_node(unhyphenated_node, decent_fit);
        vlink(q) = active;
        break_node(q) = null;
        line_number(q) = cur_list.pg_field + 1;
        total_demerits(q) = 0;
        active_short(q) = 0;
        active_glue(q) = 0;
        vlink(active) = q;
        do_all_eight(store_background);
        passive = null;
        printed_node = temp_head;
        pass_number = 0;
        font_in_short_display = null_font;
        /*tex Create an active breakpoint representing the beginning of the paragraph. */
        auto_breaking = true;
        cur_p = vlink(temp_head);
        /*tex Initialize with first |local_paragraph| node. */
        if ((cur_p != null) && (type(cur_p) == local_par_node)) {
            /*tex This used to be an assert, but may as well force it. */
            alink(cur_p) = temp_head;
            internal_pen_inter = local_pen_inter(cur_p);
            internal_pen_broken = local_pen_broken(cur_p);
            init_internal_left_box = local_box_left(cur_p);
            init_internal_left_box_width = local_box_left_width(cur_p);
            internal_left_box = init_internal_left_box;
            internal_left_box_width = init_internal_left_box_width;
            internal_right_box = local_box_right(cur_p);
            internal_right_box_width = local_box_right_width(cur_p);
        } else {
            internal_pen_inter = 0;
            internal_pen_broken = 0;
            init_internal_left_box = null;
            init_internal_left_box_width = 0;
            internal_left_box = init_internal_left_box;
            internal_left_box_width = init_internal_left_box_width;
            internal_right_box = null;
            internal_right_box_width = 0;
        }
        /*tex Initialize with first |local_paragraph| node. */
        set_prev_char_p(null);
        first_p = cur_p;
        /*tex

            To access the first node of paragraph as the first active node has
            |break_node=null|.

        */
        while ((cur_p != null) && (vlink(active) != active)) {
            /*tex

                |try_break| if |cur_p| is a legal breakpoint; on the 2nd pass,
                also look at |disc_node|s.

            */
            while (is_char_node(cur_p)) {
                /*tex

                    Advance |cur_p| to the node following the present string of
                    characters. The code that passes over the characters of words
                    in a paragraph is part of \TeX's inner loop, so it has been
                    streamlined for speed. We use the fact that
                    `\.{\\parfillskip}' glue appears at the end of each
                    paragraph; it is therefore unnecessary to check if
                    |vlink(cur_p)=null| when |cur_p| is a character node.

                */
                active_width[1] += pack_width(line_break_dir, dir_TRT, cur_p, true);
                if ((adjust_spacing > 1) && check_expand_pars(font(cur_p))) {
                    set_prev_char_p(cur_p);
                    add_char_stretch(active_width[8], cur_p);
                    add_char_shrink(active_width[9], cur_p);
                }
                cur_p = vlink(cur_p);
                while (cur_p == null && nest_index > 0) {
                    cur_p = nest_stack[--nest_index];
                }
            }
            if (cur_p == null) {
                normal_error("linebreak","invalid list tail, probably missing glue");
            }
            /*tex

                Determine legal breaks: As we move through the hlist, we need to
                keep the |active_width| array up to date, so that the badness of
                individual lines is readily calculated by |try_break|. It is
                convenient to use the short name |active_width[1]| for the
                component of active width that represents real width as opposed
                to glue.

            */
            switch (type(cur_p)) {
                case hlist_node:
                case vlist_node:
                    active_width[1] += pack_width(line_break_dir, box_dir(cur_p), cur_p, false);
                    break;
                case rule_node:
                    active_width[1] += width(cur_p);
                    break;
                case dir_node:
                    /*tex Adjust the dir stack for the |line_break| routine. */
                    if (subtype(cur_p) == normal_dir) {
                        line_break_dir = dir_dir(cur_p);
                        /* Adds to |dir_ptr|. */
                        push_dir_node(dir_ptr,cur_p);
                    } else {
                        pop_dir_node(dir_ptr);
                        if (dir_ptr != null) {
                            line_break_dir = dir_dir(dir_ptr);
                        }
                    }
                    break;
                case local_par_node:
                    /*tex Advance past a |local_paragraph| node. */
                    internal_pen_inter = local_pen_inter(cur_p);
                    internal_pen_broken = local_pen_broken(cur_p);
                    internal_left_box = local_box_left(cur_p);
                    internal_left_box_width = local_box_left_width(cur_p);
                    internal_right_box = local_box_right(cur_p);
                    internal_right_box_width = local_box_right_width(cur_p);
                    break;
                case math_node:
                    auto_breaking = (subtype(cur_p) == after);
                    /*tex begin mathskip code */
                    if (glue_is_zero(cur_p) || ignore_math_skip(cur_p)) {
                        kern_break();
                        break;
                    } else {
                        /*tex fall through */
                    }
                    /*tex end mathskip code */
                case glue_node:
                    /*tex

                        If node |cur_p| is a legal breakpoint, call |try_break|;
                        then update the active widths by including the glue in
                        |glue_ptr(cur_p)|.

                        When node |cur_p| is a glue node, we look at the previous
                        to see whether or not a breakpoint is legal at |cur_p|,
                        as explained above.

                        We only break after certain nodes (see texnodes.h), a
                        font related kern and a dir node when
                        |\breakafterdirmode=1|.

                    */
                    if (auto_breaking) {
                        halfword prev_p = alink(cur_p);
                        if (prev_p != temp_head && (is_char_node(prev_p)
                             || precedes_break(prev_p) || precedes_kern(prev_p) || precedes_dir(prev_p))) {
                            ext_try_break(
                                0,
                                unhyphenated_node,
                                line_break_dir,
                                adjust_spacing,
                                par_shape_ptr,
                                adj_demerits,
                                tracing_paragraphs,
                                protrude_chars,
                                line_penalty,
                                last_line_fit,
                                double_hyphen_demerits,
                                final_hyphen_demerits,
                                first_p,
                                cur_p
                            );
                        }
                    }
                    check_shrinkage(cur_p);
                    active_width[1] += width(cur_p);
                    active_width[2 + stretch_order(cur_p)] += stretch(cur_p);
                    active_width[7] += shrink(cur_p);
                    break;
                case kern_node:
                    if (subtype(cur_p) == explicit_kern || subtype(cur_p) == italic_kern) {
                        kern_break();
                    } else {
                        active_width[1] += width(cur_p);
                        if ((adjust_spacing == 2) && (subtype(cur_p) == normal)) {
                            add_kern_stretch(active_width[8], cur_p);
                            add_kern_shrink(active_width[9], cur_p);
                        }
                    }
                    break;
                case disc_node:
                    /*tex

                        |select_disc|s are handled by the leading |init_disc|.

                    */
                    if (subtype(cur_p) == select_disc)
                        break;
                    /*tex

                        Try to break after a discretionary fragment, then |goto
                        done5|. The following code knows that discretionary texts
                        contain only character nodes, kern nodes, box nodes, and
                        rule nodes. This branch differs a bit from older engines
                        because in \LUATEX\ we already have hyphenated the list.
                        This means that we need to skip automatic disc nodes. Of
                        better, we need to treat discretionaries and explicit
                        hyphens always, even in the first pass.

                    */
                    if (second_pass || subtype(cur_p) <= automatic_disc) {
                        int actual_penalty = (int) disc_penalty(cur_p);
                        s = vlink_pre_break(cur_p);
                        do_one_seven_eight(reset_disc_width);
                        if (s == null) {
                            /*tex trivial pre-break */
                            ext_try_break(actual_penalty, hyphenated_node,
                                          line_break_dir, adjust_spacing,
                                          par_shape_ptr, adj_demerits,
                                          tracing_paragraphs, protrude_chars,
                                          line_penalty, last_line_fit,
                                          double_hyphen_demerits,
                                          final_hyphen_demerits, first_p, cur_p);
                        } else {
                            add_to_widths(s, line_break_dir, adjust_spacing, disc_width);
                            do_one_seven_eight(add_disc_width_to_active_width);
                            ext_try_break(actual_penalty, hyphenated_node,
                                          line_break_dir, adjust_spacing,
                                          par_shape_ptr, adj_demerits,
                                          tracing_paragraphs, protrude_chars,
                                          line_penalty, last_line_fit,
                                          double_hyphen_demerits,
                                          final_hyphen_demerits, first_p, cur_p);
                            if (subtype(cur_p) == init_disc) {
                                /*tex

                                    We should at two break points after the one
                                    we added above:

                                    \startitemize[n]
                                        \startitem
                                            which does a possible break in INIT's
                                            |post_break|
                                        \stopitem
                                        \startitem
                                            which means the |no_break| actually
                                            was broken just a character later
                                        \stopitem
                                    \stopitemize

                                    Do the select-0 case |f-f-i|:

                                */
                                s = vlink_pre_break(vlink(cur_p));
                                add_to_widths(s, line_break_dir, adjust_spacing, disc_width);
                                ext_try_break(actual_penalty, hyphenated_node,
                                              line_break_dir, adjust_spacing,
                                              par_shape_ptr, adj_demerits,
                                              tracing_paragraphs,
                                              protrude_chars, line_penalty,
                                              last_line_fit, double_hyphen_demerits,
                                              final_hyphen_demerits, first_p,
                                              vlink(cur_p));
                                /*tex This does not work. */
#if 0
                                /*tex Go back to the starting situation. */
                                do_one_seven_eight(sub_disc_width_from_active_width);
                                do_one_seven_eight(reset_disc_width);
                                /*tex Add select |no_break| to |active_width|. */
                                s = vlink_no_break(vlink(cur_p));
                                add_to_widths(s, line_break_dir, adjust_spacing, disc_width);
                                ext_try_break(actual_penalty, hyphenated_node,
                                              line_break_dir, adjust_spacing,
                                              par_shape_ptr, adj_demerits,
                                              tracing_paragraphs,
                                              protrude_chars, line_penalty,
                                              last_line_fit, double_hyphen_demerits,
                                              final_hyphen_demerits, first_p,
                                              vlink(cur_p));
#endif
                            }
                            do_one_seven_eight(sub_disc_width_from_active_width);
                        }
                    }
                    s = vlink_no_break(cur_p);
                    add_to_widths(s, line_break_dir, adjust_spacing, active_width);
                    break;
                case penalty_node:
                    ext_try_break(penalty(cur_p), unhyphenated_node, line_break_dir,
                                  adjust_spacing, par_shape_ptr, adj_demerits,
                                  tracing_paragraphs, protrude_chars,
                                  line_penalty, last_line_fit,
                                  double_hyphen_demerits, final_hyphen_demerits,
                                  first_p, cur_p);
                    break;
                case boundary_node:
                case whatsit_node:
                    /*tex Advance past a whatsit node in the |line_break| loop. */
                case mark_node:
                case ins_node:
                case adjust_node:
                    break;
                case glue_spec_node:
                    normal_warning("parbuilder","found a glue_spec in a paragraph");
                    break;
                default:
                    formatted_error("parbuilder","weird node %d in paragraph",type(cur_p));
            }
            cur_p = vlink(cur_p);
            while (cur_p == null && nest_index > 0) {
                cur_p = nest_stack[--nest_index];
            }
        }
        if (cur_p == null) {
            /*tex

                Try the final line break at the end of the paragraph, and |goto
                done| if the desired breakpoints have been found.

                The forced line break at the paragraph's end will reduce the list
                of breakpoints so that all active nodes represent breaks at
                |cur_p=null|. On the first pass, we insist on finding an active
                node that has the correct ``looseness.'' On the final pass, there
                will be at least one active node, and we will match the desired
                looseness as well as we can.

                The global variable |best_bet| will be set to the active node for
                the best way to break the paragraph, and a few other variables
                are used to help determine what is best.

            */
            ext_try_break(eject_penalty, hyphenated_node, line_break_dir,
                          adjust_spacing, par_shape_ptr, adj_demerits,
                          tracing_paragraphs, protrude_chars, line_penalty,
                          last_line_fit, double_hyphen_demerits,
                          final_hyphen_demerits, first_p, cur_p);
            if (vlink(active) != active) {
                /*tex Find an active node with fewest demerits; */
                r = vlink(active);
                fewest_demerits = awful_bad;
                do {
                    if (type(r) != delta_node) {
                        if (total_demerits(r) < fewest_demerits) {
                            fewest_demerits = total_demerits(r);
                            best_bet = r;
                        }
                    }
                    r = vlink(r);
                } while (r != active);
                best_line = line_number(best_bet);
                /*tex
                    Find an active node with fewest demerits;
                */
                if (looseness == 0)
                    goto DONE;
                /*tex

                    Find the best active node for the desired looseness;

                    The adjustment for a desired looseness is a slightly more
                    complicated version of the loop just considered. Note that if
                    a paragraph is broken into segments by displayed equations,
                    each segment will be subject to the looseness calculation,
                    independently of the other segments.

                */
                r = vlink(active);
                actual_looseness = 0;
                do {
                    if (type(r) != delta_node) {
                        line_diff = line_number(r) - best_line;
                        if (((line_diff < actual_looseness)
                             && (looseness <= line_diff))
                            || ((line_diff > actual_looseness)
                                && (looseness >= line_diff))) {
                            best_bet = r;
                            actual_looseness = line_diff;
                            fewest_demerits = total_demerits(r);
                        } else if ((line_diff == actual_looseness) &&
                                   (total_demerits(r) < fewest_demerits)) {
                            best_bet = r;
                            fewest_demerits = total_demerits(r);
                        }
                    }
                    r = vlink(r);
                } while (r != active);
                best_line = line_number(best_bet);
                /*tex
                    Find the best active node for the desired looseness.
                */
                if ((actual_looseness == looseness) || final_pass)
                    goto DONE;
            }
        }
        /*tex Clean up the memory by removing the break nodes. */
        clean_up_the_memory();
        /*tex Clean up the memory by removing the break nodes. */
        if (!second_pass) {
            if (tracing_paragraphs > 0)
                tprint_nl("@secondpass");
            threshold = tolerance;
            second_pass = true;
            final_pass = (emergency_stretch <= 0);
        } else {
            /*tex If at first you do not succeed, then: */
            if (tracing_paragraphs > 0)
                tprint_nl("@emergencypass");
            background[2] += emergency_stretch;
            final_pass = true;
        }
    }

  DONE:
    if (tracing_paragraphs > 0) {
        end_diagnostic(true);
        normalize_selector();
    }
    if (do_last_line_fit) {
        /*tex
            Adjust the final line of the paragraph; here we either reset
            |do_last_line_fit| or adjust the |par_fill_skip| glue.
        */
        if (active_short(best_bet) == 0) {
            do_last_line_fit = false;
        } else {
            width(last_line_fill) += (active_short(best_bet) - active_glue(best_bet));
            stretch(last_line_fill) = 0;
        }
    }
    /*tex
        Break the paragraph at the chosen. Once the best sequence of
        breakpoints has been found (hurray), we call on the procedure
        |post_line_break| to finish the remainder of the work. By introducing
        this subprocedure, we are able to keep |line_break| from getting
        extremely long.

        the first thing |ext_post_line_break| does is reset |dir_ptr|.

    */
    flush_node_list(dir_ptr);
    dir_ptr = null;
    ext_post_line_break(paragraph_dir,
                        right_skip,
                        left_skip,
                        protrude_chars,
                        par_shape_ptr,
                        adjust_spacing,
                        inter_line_penalties_par_ptr,
                        inter_line_penalty,
                        club_penalty,
                        club_penalties_ptr,
                        widow_penalties_ptr,
                        widow_penalty,
                        broken_penalty,
                        final_par_glue,
                        best_bet,
                        last_special_line,
                        second_width,
                        second_indent, first_width, first_indent, best_line);
    /*tex

        Clean up the memory by removing the break nodes.

    */
    clean_up_the_memory();
}

void get_linebreak_info (int *f, int *a)
{
    *f = fewest_demerits;
    *a = actual_looseness;
}
