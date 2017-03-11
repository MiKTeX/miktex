% texmath.w
%
% Copyright 2008-2010 Taco Hoekwater <taco@@luatex.org>
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
#define mode     mode_par
#define tail     tail_par
#define head     head_par
#define dir_save dirs_par

/*

    \mathdisplayskipmode

    tex normally always inserts before and only after when larger than zero

    0 = normal tex
    1 = always
    2 = non-zero
    3 = ignore

*/

@ TODO: not sure if this is the right order
@c
#define back_error(A,B) do {                    \
    OK_to_interrupt=false;                      \
    back_input();                               \
    OK_to_interrupt=true;                       \
    tex_error(A,B);                             \
  } while (0)

@ @c
int scan_math(pointer, int);
int scan_math_style(pointer, int);
pointer fin_mlist(pointer);

@ When \TeX\ reads a formula that is enclosed between \.\$'s, it constructs an
{\sl mlist}, which is essentially a tree structure representing that
formula.  An mlist is a linear sequence of items, but we can regard it as
a tree structure because mlists can appear within mlists. For example, many
of the entries can be subscripted or superscripted, and such ``scripts''
are mlists in their own right.

An entire formula is parsed into such a tree before any of the actual
typesetting is done, because the current style of type is usually not
known until the formula has been fully scanned. For example, when the
formula `\.{\$a+b \\over c+d\$}' is being read, there is no way to tell
that `\.{a+b}' will be in script size until `\.{\\over}' has appeared.

During the scanning process, each element of the mlist being built is
classified as a relation, a binary operator, an open parenthesis, etc.,
or as a construct like `\.{\\sqrt}' that must be built up. This classification
appears in the mlist data structure.

After a formula has been fully scanned, the mlist is converted to an hlist
so that it can be incorporated into the surrounding text. This conversion is
controlled by a recursive procedure that decides all of the appropriate
styles by a ``top-down'' process starting at the outermost level and working
in towards the subformulas. The formula is ultimately pasted together using
combinations of horizontal and vertical boxes, with glue and penalty nodes
inserted as necessary.

An mlist is represented internally as a linked list consisting chiefly
of ``noads'' (pronounced ``no-adds''), to distinguish them from the somewhat
similar ``nodes'' in hlists and vlists. Certain kinds of ordinary nodes are
allowed to appear in mlists together with the noads; \TeX\ tells the difference
by means of the |type| field, since a noad's |type| is always greater than
that of a node. An mlist does not contain character nodes, hlist nodes, vlist
nodes, math nodes or unset nodes; in particular, each mlist item appears in the
variable-size part of |mem|, so the |type| field is always present.

Each noad is five or more words long. The first word contains the
|type| and |subtype| and |link| fields that are already so familiar to
us; the second contains the attribute list pointer, and the third,
fourth an fifth words are called the noad's |nucleus|, |subscr|, and
|supscr| fields. (This use of a combined attribute list is temporary.
Eventually, each of fields need their own list)

Consider, for example, the simple formula `\.{\$x\^2\$}', which would be
parsed into an mlist containing a single element called an |ord_noad|.
The |nucleus| of this noad is a representation of `\.x', the |subscr| is
empty, and the |supscr| is a representation of `\.2'.

The |nucleus|, |subscr|, and |supscr| fields are further broken into
subfields. If |p| points to a noad, and if |q| is one of its principal
fields (e.g., |q=subscr(p)|), |q=null| indicates a field with no value (the
corresponding attribute of noad |p| is not present). Otherwise, there are
several possibilities for the subfields, depending on the |type| of |q|.

\yskip\hang|type(q)=math_char_node| means that |math_fam(q)| refers to one of
the sixteen font families, and |character(q)| is the number of a character
within a font of that family, as in a character node.

\yskip\hang|type(q)=math_text_char_node| is similar, but the character is
unsubscripted and unsuperscripted and it is followed immediately by another
character from the same font. (This |type| setting appears only
briefly during the processing; it is used to suppress unwanted italic
corrections.)

\yskip\hang|type(q)=sub_box_node| means that |math_list(q)| points to a box
node (either an |hlist_node| or a |vlist_node|) that should be used as the
value of the field.  The |shift_amount| in the subsidiary box node is the
amount by which that box will be shifted downward.

\yskip\hang|type(q)=sub_mlist_node| means that |math_list(q)| points to
an mlist; the mlist must be converted to an hlist in order to obtain
the value of this field.

\yskip\noindent In the latter case, we might have |math_list(q)=null|. This
is not the same as |q=null|; for example, `\.{\$P\_\{\}\$}'
and `\.{\$P\$}' produce different results (the former will not have the
``italic correction'' added to the width of |P|, but the ``script skip''
will be added).

@c
static void unsave_math(void)
{
    unsave();
    decr(save_ptr);
    flush_node_list(text_dir_ptr);
    assert(saved_type(0) == saved_textdir);
    text_dir_ptr = saved_value(0);
}

@ Sometimes it is necessary to destroy an mlist. The following
subroutine empties the current list, assuming that |abs(mode)=mmode|.

@c
void flush_math(void)
{
    flush_node_list(vlink(head));
    flush_node_list(incompleat_noad_par);
    vlink(head) = null;
    tail = head;
    incompleat_noad_par = null;
}

@ Before we can do anything in math mode, we need fonts.

@c
#define MATHFONTSTACK  8
#define MATHFONTDEFAULT 0       /* == nullfont */

static sa_tree math_fam_head = NULL;

@ @c
int fam_fnt(int fam_id, int size_id)
{
    int n = fam_id + (256 * size_id);
    return (int) get_sa_item(math_fam_head, n).int_value;
}

void def_fam_fnt(int fam_id, int size_id, int f, int lvl)
{
    int n = fam_id + (256 * size_id);
    sa_tree_item sa_value = { 0 };
    sa_value.int_value = f;
    set_sa_item(math_fam_head, n, sa_value, lvl);
    fixup_math_parameters(fam_id, size_id, f, lvl);
    if (tracing_assigns_par > 1) {
        begin_diagnostic();
        tprint("{assigning");
        print_char(' ');
        print_cmd_chr(def_family_cmd, size_id);
        print_int(fam_id);
        print_char('=');
        print_font_identifier(fam_fnt(fam_id, size_id));
        print_char('}');
        end_diagnostic(false);
    }
}

@ @c
static void unsave_math_fam_data(int gl)
{
    sa_stack_item st;
    if (math_fam_head->stack == NULL)
        return;
    while (math_fam_head->stack_ptr > 0 &&
           abs(math_fam_head->stack[math_fam_head->stack_ptr].level)
           >= (int) gl) {
        st = math_fam_head->stack[math_fam_head->stack_ptr];
        if (st.level > 0) {
            rawset_sa_item(math_fam_head, st.code, st.value);
            /* now do a trace message, if requested */
            if (tracing_restores_par > 1) {
                int size_id = st.code / 256;
                int fam_id = st.code % 256;
                begin_diagnostic();
                tprint("{restoring");
                print_char(' ');
                print_cmd_chr(def_family_cmd, size_id);
                print_int(fam_id);
                print_char('=');
                print_font_identifier(fam_fnt(fam_id, size_id));
                print_char('}');
                end_diagnostic(false);
            }
        }
        (math_fam_head->stack_ptr)--;
    }
}

@ and parameters

@c
#define MATHPARAMSTACK  8
#define MATHPARAMDEFAULT undefined_math_parameter

static sa_tree math_param_head = NULL;

@ @c
void def_math_param(int param_id, int style_id, scaled value, int lvl)
{
    int n = param_id + (256 * style_id);
    sa_tree_item sa_value = { 0 };
    sa_value.int_value = (int) value;
    set_sa_item(math_param_head, n, sa_value, lvl);
    if (tracing_assigns_par > 1) {
        begin_diagnostic();
        tprint("{assigning");
        print_char(' ');
        print_cmd_chr(set_math_param_cmd, param_id);
        print_cmd_chr(math_style_cmd, style_id);
        print_char('=');
        print_int(value);
        print_char('}');
        end_diagnostic(false);
    }
}

scaled get_math_param(int param_id, int style_id)
{
    int n = param_id + (256 * style_id);
    return (scaled) get_sa_item(math_param_head, n).int_value;
}

@ @c
static void unsave_math_param_data(int gl)
{
    sa_stack_item st;
    if (math_param_head->stack == NULL)
        return;
    while (math_param_head->stack_ptr > 0 &&
           abs(math_param_head->stack[math_param_head->stack_ptr].level)
           >= (int) gl) {
        st = math_param_head->stack[math_param_head->stack_ptr];
        if (st.level > 0) {
            rawset_sa_item(math_param_head, st.code, st.value);
            /* now do a trace message, if requested */
            if (tracing_restores_par > 1) {
                int param_id = st.code % 256;
                int style_id = st.code / 256;
                begin_diagnostic();
                tprint("{restoring");
                print_char(' ');
                print_cmd_chr(set_math_param_cmd, param_id);
                print_cmd_chr(math_style_cmd, style_id);
                print_char('=');
                print_int(get_math_param(param_id, style_id));
                print_char('}');
                end_diagnostic(false);
            }
        }
        (math_param_head->stack_ptr)--;
    }
}

@ saving and unsaving of both

@c
void unsave_math_data(int gl)
{
    unsave_math_fam_data(gl);
    unsave_math_param_data(gl);
}

@ Dumping and undumping
@c
void dump_math_data(void)
{
    sa_tree_item sa_value = { 0 };
    if (math_fam_head == NULL) {
        sa_value.int_value = MATHFONTDEFAULT;
        math_fam_head = new_sa_tree(MATHFONTSTACK, 1, sa_value);
    }
    dump_sa_tree(math_fam_head, "mathfonts");
    if (math_param_head == NULL) {
        sa_value.int_value = MATHPARAMDEFAULT;
        math_param_head = new_sa_tree(MATHPARAMSTACK, 1, sa_value);
    }
    dump_sa_tree(math_param_head, "mathparameters");
}

void undump_math_data(void)
{
    math_fam_head = undump_sa_tree("mathfonts");
    math_param_head = undump_sa_tree("mathparameters");
}

@ @c
void initialize_math(void)
{
    sa_tree_item sa_value = { 0 };
    if (math_fam_head == NULL) {
        sa_value.int_value = MATHFONTDEFAULT;
        math_fam_head = new_sa_tree(MATHFONTSTACK, 1, sa_value);
    }
    if (math_param_head == NULL) {
        sa_value.int_value = MATHPARAMDEFAULT;
        math_param_head = new_sa_tree(MATHPARAMSTACK, 1, sa_value);
        initialize_math_spacing();
    }
    return;
}

@ Each portion of a formula is classified as Ord, Op, Bin, Rel, Ope,
Clo, Pun, or Inn, for purposes of spacing and line breaking. An
|ord_noad|, |op_noad|, |bin_noad|, |rel_noad|, |open_noad|, |close_noad|,
|punct_noad|, or |inner_noad| is used to represent portions of the various
types. For example, an `\.=' sign in a formula leads to the creation of a
|rel_noad| whose |nucleus| field is a representation of an equals sign
(usually |fam=0|, |character=075|).  A formula preceded by \.{\\mathrel}
also results in a |rel_noad|.  When a |rel_noad| is followed by an
|op_noad|, say, and possibly separated by one or more ordinary nodes (not
noads), \TeX\ will insert a penalty node (with the current |rel_penalty|)
just after the formula that corresponds to the |rel_noad|, unless there
already was a penalty immediately following; and a ``thick space'' will be
inserted just before the formula that corresponds to the |op_noad|.

A noad of type |ord_noad|, |op_noad|, \dots, |inner_noad| usually
has a |subtype=normal|. The only exception is that an |op_noad| might
have |subtype=limits| or |no_limits|, if the normal positioning of
limits has been overridden for this operator.

A |radical_noad| also has a |left_delimiter| field, which usually
represents a square root sign.

A |fraction_noad| has a |right_delimiter| field as well as a |left_delimiter|.

Delimiter fields have four subfields
called |small_fam|, |small_char|, |large_fam|, |large_char|. These subfields
represent variable-size delimiters by giving the ``small'' and ``large''
starting characters, as explained in Chapter~17 of {\sl The \TeX book}.
@:TeXbook}{\sl The \TeX book@>

A |fraction_noad| is actually quite different from all other noads.
It has |thickness|, |denominator|, and |numerator| fields instead of
|nucleus|, |subscr|, and |supscr|. The |thickness| is a scaled value
that tells how thick to make a fraction rule; however, the special
value |default_code| is used to stand for the
|default_rule_thickness| of the current size. The |numerator| and
|denominator| point to mlists that define a fraction; we always have
$$\hbox{|type(numerator)=type(denominator)=sub_mlist|}.$$ The
|left_delimiter| and |right_delimiter| fields specify delimiters that will
be placed at the left and right of the fraction. In this way, a
|fraction_noad| is able to represent all of \TeX's operators \.{\\over},
\.{\\atop}, \.{\\above}, \.{\\overwithdelims}, \.{\\atopwithdelims}, and
 \.{\\abovewithdelims}.

@ The |new_noad| function creates an |ord_noad| that is completely null

@c
pointer new_noad(void)
{
    pointer p;
    p = new_node(simple_noad, ord_noad_type);
    /* all noad fields are zero after this */
    return p;
}

@ @c
pointer new_sub_box(pointer curbox)
{
    pointer p, q;
    p = new_noad();
    q = new_node(sub_box_node, 0);
    nucleus(p) = q;
    math_list(nucleus(p)) = curbox;
    return p;
}

@ A few more kinds of noads will complete the set: An |under_noad| has its
nucleus underlined; an |over_noad| has it overlined. An |accent_noad| places
an accent over its nucleus; the accent character appears as
|math_fam(accent_chr(p))| and |math_character(accent_chr(p))|. A |vcenter_noad|
centers its nucleus vertically with respect to the axis of the formula;
in such noads we always have |type(nucleus(p))=sub_box|.

And finally, we have the |fence_noad| type, to implement
\TeX's \.{\\left} and \.{\\right} as well as eTeX's \.{\\middle}.
The |nucleus| of such noads is
replaced by a |delimiter| field; thus, for example, `\.{\\left(}' produces
a |fence_noad| such that |delimiter(p)| holds the family and character
codes for all left parentheses. A |fence_noad| of subtype |left_noad_side|
never appears in an mlist except as the first element, and a |fence_noad|
with subtype |right_noad_side| never appears in an mlist
except as the last element; furthermore, we either have both a |left_noad_side|
and a |right_noad_side|, or neither one is present.

@ Math formulas can also contain instructions like \.{\\textstyle} that
override \TeX's normal style rules. A |style_node| is inserted into the
data structure to record such instructions; it is three words long, so it
is considered a node instead of a noad. The |subtype| is either |display_style|
or |text_style| or |script_style| or |script_script_style|. The
second and third words of a |style_node| are not used, but they are
present because a |choice_node| is converted to a |style_node|.

\TeX\ uses even numbers 0, 2, 4, 6 to encode the basic styles
|display_style|, \dots, |script_script_style|, and adds~1 to get the
``cramped'' versions of these styles. This gives a numerical order that
is backwards from the convention of Appendix~G in {\sl The \TeX book\/};
i.e., a smaller style has a larger numerical value.
@:TeXbook}{\sl The \TeX book@>

@c
const char *math_style_names[] = {
    "display", "crampeddisplay",
    "text", "crampedtext",
    "script", "crampedscript",
    "scriptscript", "crampedscriptscript",
    NULL
};

const char *math_param_names[] = {
    "quad", "axis", "operatorsize",
    "overbarkern", "overbarrule", "overbarvgap",
    "underbarkern", "underbarrule", "underbarvgap",
    "radicalkern", "radicalrule", "radicalvgap",
    "radicaldegreebefore", "radicaldegreeafter", "radicaldegreeraise",
    "stackvgap", "stacknumup", "stackdenomdown",
    "fractionrule", "fractionnumvgap", "fractionnumup",
    "fractiondenomvgap", "fractiondenomdown", "fractiondelsize",
    "limitabovevgap", "limitabovebgap", "limitabovekern",
    "limitbelowvgap", "limitbelowbgap", "limitbelowkern",
    "nolimitsubfactor", "nolimitsupfactor", /* bonus */
    "underdelimitervgap", "underdelimiterbgap",
    "overdelimitervgap", "overdelimiterbgap",
    "subshiftdrop", "supshiftdrop", "subshiftdown",
    "subsupshiftdown", "subtopmax", "supshiftup",
    "supbottommin", "supsubbottommax", "subsupvgap",
    "spaceafterscript", "connectoroverlapmin",
    "ordordspacing", "ordopspacing", "ordbinspacing", "ordrelspacing",
    "ordopenspacing", "ordclosespacing", "ordpunctspacing", "ordinnerspacing",
    "opordspacing", "opopspacing", "opbinspacing", "oprelspacing",
    "opopenspacing", "opclosespacing", "oppunctspacing", "opinnerspacing",
    "binordspacing", "binopspacing", "binbinspacing", "binrelspacing",
    "binopenspacing", "binclosespacing", "binpunctspacing", "bininnerspacing",
    "relordspacing", "relopspacing", "relbinspacing", "relrelspacing",
    "relopenspacing", "relclosespacing", "relpunctspacing", "relinnerspacing",
    "openordspacing", "openopspacing", "openbinspacing", "openrelspacing",
    "openopenspacing", "openclosespacing", "openpunctspacing",
    "openinnerspacing",
    "closeordspacing", "closeopspacing", "closebinspacing", "closerelspacing",
    "closeopenspacing", "closeclosespacing", "closepunctspacing",
    "closeinnerspacing",
    "punctordspacing", "punctopspacing", "punctbinspacing", "punctrelspacing",
    "punctopenspacing", "punctclosespacing", "punctpunctspacing",
    "punctinnerspacing",
    "innerordspacing", "inneropspacing", "innerbinspacing", "innerrelspacing",
    "inneropenspacing", "innerclosespacing", "innerpunctspacing",
    "innerinnerspacing",
    NULL
};

@ @c
pointer new_style(small_number s)
{                               /* create a style node */
    m_style = s;
    return new_node(style_node, s);
}

@ Finally, the \.{\\mathchoice} primitive creates a |choice_node|, which
has special subfields |display_mlist|, |text_mlist|, |script_mlist|,
and |script_script_mlist| pointing to the mlists for each style.

@c
static pointer new_choice(void)
{                               /* create a choice node */
    return new_node(choice_node, 0);    /* the |subtype| is not used */
}

@ Let's consider now the previously unwritten part of |show_node_list|
that displays the things that can only be present in mlists; this
program illustrates how to access the data structures just defined.

In the context of the following program, |p| points to a node or noad that
should be displayed, and the current string contains the ``recursion history''
that leads to this point. The recursion history consists of a dot for each
outer level in which |p| is subsidiary to some node, or in which |p| is
subsidiary to the |nucleus| field of some noad; the dot is replaced by
`\.\_' or `\.\^' or `\./' or `\.\\' if |p| is descended from the |subscr|
or |supscr| or |denominator| or |numerator| fields of noads. For example,
the current string would be `\.{.\^.\_/}' if |p| points to the |ord_noad| for
|x| in the (ridiculous) formula
`\.{\$\\sqrt\{a\^\{\\mathinner\{b\_\{c\\over x+y\}\}\}\}\$}'.

@c
void display_normal_noad(pointer p);    /* forward */
void display_fence_noad(pointer p);     /* forward */
void display_fraction_noad(pointer p);  /* forward */

void show_math_node(pointer p)
{
    switch (type(p)) {
    case style_node:
        print_cmd_chr(math_style_cmd, subtype(p));
        break;
    case choice_node:
        tprint_esc("mathchoice");
        append_char('D');
        show_node_list(display_mlist(p));
        flush_char();
        append_char('T');
        show_node_list(text_mlist(p));
        flush_char();
        append_char('S');
        show_node_list(script_mlist(p));
        flush_char();
        append_char('s');
        show_node_list(script_script_mlist(p));
        flush_char();
        break;
    case simple_noad:
    case radical_noad:
    case accent_noad:
        display_normal_noad(p);
        break;
    case fence_noad:
        display_fence_noad(p);
        break;
    case fraction_noad:
        display_fraction_noad(p);
        break;
    default:
        tprint("Unknown node type!");
        break;
    }
}

@ Here are some simple routines used in the display of noads.

@c
static void print_fam_and_char(pointer p)
{                               /* prints family and character */
    tprint_esc("fam");
    print_int(math_fam(p));
    print_char(' ');
    print(math_character(p));
}

@ @c
static void print_delimiter(pointer p)
{
    int a;
    if (delimiteroptionset(p)) {
        tprint(" [ ");
        if (delimiteraxis(p))
            tprint("axis ");
        if (delimiternoaxis(p))
            tprint("noaxis ");
        if (delimiterexact(p))
            tprint("exact ");
        tprint("]");
    }
    if (delimiterheight(p)) {
        tprint("height=");
        print_scaled(delimiterheight(p));
        tprint(" ");
    }
    if (delimiterdepth(p)) {
        tprint("depth=");
        print_scaled(delimiterdepth(p));
        tprint(" ");
    }
    if (delimiterclass(p)) {
        tprint("class=");
        print_int(delimiterclass(p));
        tprint(" ");
    }
    if (small_fam(p) < 0) {
        print_int(-1);          /* this should never happen */
    } else if (small_fam(p) < 16 && large_fam(p) < 16 &&
               small_char(p) < 256 && large_char(p) < 256) {
        /* traditional tex style */
        a = small_fam(p) * 256 + small_char(p);
        a = a * 0x1000 + large_fam(p) * 256 + large_char(p);
        print_hex(a);
    } else if ((large_fam(p) == 0 && large_char(p) == 0) ||
               small_char(p) > 65535 || large_char(p) > 65535) {
        /* modern xetex/luatex style */
        print_hex(small_fam(p));
        print_hex(small_char(p));
    }
}

@ The next subroutine will descend to another level of recursion when a
subsidiary mlist needs to be displayed. The parameter |c| indicates what
character is to become part of the recursion history. An empty mlist is
distinguished from a missing field, because these are not equivalent
(as explained above).
@^recursion@>

@c
static void print_subsidiary_data(pointer p, ASCII_code c)
{                               /* display a noad field */
    if ((int) cur_length >= depth_threshold) {
        if (p != null)
            tprint(" []");
    } else {
        append_char(c);         /* include |c| in the recursion history */
        if (p != null) {
            switch (type(p)) {
            case math_char_node:
                print_ln();
                print_current_string();
                print_fam_and_char(p);
                break;
            case sub_box_node:
                show_node_list(math_list(p));
                break;
            case sub_mlist_node:
                if (math_list(p) == null) {
                    print_ln();
                    print_current_string();
                    tprint("{}");
                } else {
                    show_node_list(math_list(p));
                }
                break;
            }
        }
        flush_char();           /* remove |c| from the recursion history */
    }
}

@ @c
void display_normal_noad(pointer p)
{
    switch (type(p)) {
    case simple_noad:
        switch (subtype(p)) {
        case ord_noad_type:
            tprint_esc("mathord");
            break;
        case op_noad_type_normal:
        case op_noad_type_limits:
        case op_noad_type_no_limits:
            tprint_esc("mathop");
            if (subtype(p) == op_noad_type_limits)
                tprint_esc("limits");
            else if (subtype(p) == op_noad_type_no_limits)
                tprint_esc("nolimits");
            break;
        case bin_noad_type:
            tprint_esc("mathbin");
            break;
        case rel_noad_type:
            tprint_esc("mathrel");
            break;
        case open_noad_type:
            tprint_esc("mathopen");
            break;
        case close_noad_type:
            tprint_esc("mathclose");
            break;
        case punct_noad_type:
            tprint_esc("mathpunct");
            break;
        case inner_noad_type:
            tprint_esc("mathinner");
            break;
        case over_noad_type:
            tprint_esc("overline");
            break;
        case under_noad_type:
            tprint_esc("underline");
            break;
        case vcenter_noad_type:
            tprint_esc("vcenter");
            break;
        default:
            tprint("<unknown noad type!>");
            break;
        }
        break;
    case radical_noad:
        if (subtype(p) == 6)
            tprint_esc("Udelimiterover");
        else if (subtype(p) == 5)
            tprint_esc("Udelimiterunder");
        else if (subtype(p) == 4)
            tprint_esc("Uoverdelimiter");
        else if (subtype(p) == 3)
            tprint_esc("Uunderdelimiter");
        else if (subtype(p) == 2)
            tprint_esc("Uroot");
        else
            tprint_esc("radical");
        print_delimiter(left_delimiter(p));
        if (degree(p) != null) {
            print_subsidiary_data(degree(p), '/');
        }
        if (radicalwidth(p)) {
            tprint("width=");
            print_scaled(radicalwidth(p));
            tprint(" ");
        }
        if (radicaloptionset(p)) {
            tprint(" [ ");
            if (radicalexact(p))
                tprint("exact ");
            if (radicalleft(p))
                tprint("left ");
            if (radicalmiddle(p))
                tprint("middle ");
            if (radicalright(p))
                tprint("right ");
            tprint("]");
        }
        break;
    case accent_noad:
       if (top_accent_chr(p) != null) {
           if (bot_accent_chr(p) != null) {
               tprint_esc("Umathaccent both");
           } else {
               tprint_esc("Umathaccent");
           }
        } else if (bot_accent_chr(p) != null) {
            tprint_esc("Umathaccent bottom");
        } else {
            tprint_esc("Umathaccent overlay");
        }
        if (accentfraction(p)) {
            tprint(" fraction=");
            print_int(accentfraction(p));
            tprint(" ");
        }
        switch (subtype(p)) {
            case 0:
                if (top_accent_chr(p) != null) {
                    if (bot_accent_chr(p) != null) {
                        print_fam_and_char(top_accent_chr(p));
                        print_fam_and_char(bot_accent_chr(p));
                    } else {
                        print_fam_and_char(top_accent_chr(p));
                    }
                } else if (bot_accent_chr(p) != null) {
                    print_fam_and_char(bot_accent_chr(p));
                } else {
                    print_fam_and_char(overlay_accent_chr(p));
                }
                break;
            case 1:
                if (top_accent_chr(p) != null) {
                    tprint(" fixed ");
                    print_fam_and_char(top_accent_chr(p));
                    if (bot_accent_chr(p) != null) {
                        print_fam_and_char(bot_accent_chr(p));
                    }
                } else {
                    confusion("display_accent_noad");
                }
                break;
            case 2:
                if (bot_accent_chr(p) != null) {
                    if (top_accent_chr(p) != null) {
                        print_fam_and_char(top_accent_chr(p));
                    }
                    tprint(" fixed ");
                    print_fam_and_char(bot_accent_chr(p));
                } else{
                    confusion("display_accent_noad");
                }
                break;
            case 3:
                if (top_accent_chr(p) != null && bot_accent_chr(p) != null) {
                    tprint(" fixed ");
                    print_fam_and_char(top_accent_chr(p));
                    tprint(" fixed ");
                    print_fam_and_char(bot_accent_chr(p));
                } else {
                    confusion("display_accent_noad");
                }
                break;
            }
        break;
    }
    print_subsidiary_data(nucleus(p), '.');
    print_subsidiary_data(supscr(p), '^');
    print_subsidiary_data(subscr(p), '_');
}

@ @c
void display_fence_noad(pointer p)
{
    if (subtype(p) == right_noad_side)
        tprint_esc("right");
    else if (subtype(p) == left_noad_side)
        tprint_esc("left");
    else
        tprint_esc("middle");
    print_delimiter(delimiter(p));
}

@ @c
void display_fraction_noad(pointer p)
{
    tprint_esc("fraction, thickness ");
    if (thickness(p) == default_code)
        tprint("= default");
    else
        print_scaled(thickness(p));
    if ((left_delimiter(p) != null) &&
        ((small_fam(left_delimiter(p)) != 0) ||
         (small_char(left_delimiter(p)) != 0) ||
         (large_fam(left_delimiter(p)) != 0) ||
         (large_char(left_delimiter(p)) != 0))) {
        tprint(", left-delimiter ");
        print_delimiter(left_delimiter(p));
    }
    if ((right_delimiter(p) != null) &&
        ((small_fam(right_delimiter(p)) != 0) ||
         (small_char(right_delimiter(p)) != 0) ||
         (large_fam(right_delimiter(p)) != 0) ||
         (large_char(right_delimiter(p)) != 0))) {
        tprint(", right-delimiter ");
        print_delimiter(right_delimiter(p));
    }
    print_subsidiary_data(numerator(p), '\\');
    print_subsidiary_data(denominator(p), '/');
}

@ The routines that \TeX\ uses to create mlists are similar to those we have
just seen for the generation of hlists and vlists. But it is necessary to
make ``noads'' as well as nodes, so the reader should review the
discussion of math mode data structures before trying to make sense out of
the following program.

Here is a little routine that needs to be done whenever a subformula
is about to be processed. The parameter is a code like |math_group|.

@c
static void new_save_level_math(group_code c)
{
    set_saved_record(0, saved_textdir, 0, text_dir_ptr);
    text_dir_ptr = new_dir(math_direction_par);
    incr(save_ptr);
    new_save_level(c);
    eq_word_define(int_base + body_direction_code, math_direction_par);
    eq_word_define(int_base + par_direction_code, math_direction_par);
    eq_word_define(int_base + text_direction_code, math_direction_par);
}

@ @c
static void push_math(group_code c, int mstyle)
{
    if (math_direction_par != text_direction_par)
        dir_math_save = true;
    push_nest();
    mode = -mmode;
    incompleat_noad_par = null;
    m_style = mstyle;
    new_save_level_math(c);
}

@ @c
static void enter_ordinary_math(void)
{
    push_math(math_shift_group, text_style);
    eq_word_define(int_base + cur_fam_code, -1);
    if (every_math_par != null)
        begin_token_list(every_math_par, every_math_text);
}

@ @c
void enter_display_math(void);

@ We get into math mode from horizontal mode when a `\.\$' (i.e., a
|math_shift| character) is scanned. We must check to see whether this
`\.\$' is immediately followed by another, in case display math mode is
called for.

@c
void init_math(void)
{
    if (cur_cmd == math_shift_cmd) {
        get_token();            /* |get_x_token| would fail on \.{\\ifmmode}\thinspace! */
        if ((cur_cmd == math_shift_cmd) && (mode > 0)) {
            enter_display_math();
        } else {
            back_input();
            enter_ordinary_math();
        }
    } else if (cur_cmd == math_shift_cs_cmd && cur_chr == display_style && (mode > 0)) {
        enter_display_math();
    } else if (cur_cmd == math_shift_cs_cmd && cur_chr == text_style) {
        enter_ordinary_math();
    } else {
        you_cant();
    }
}

@ We get into ordinary math mode from display math mode when `\.{\\eqno}' or
`\.{\\leqno}' appears. In such cases |cur_chr| will be 0 or~1, respectively;
the value of |cur_chr| is placed onto |save_stack| for safe keeping.

@ When \TeX\ is in display math mode, |cur_group=math_shift_group|,
so it is not necessary for the |start_eq_no| procedure to test for
this condition.

@c
void start_eq_no(void)
{
    set_saved_record(0, saved_eqno, 0, cur_chr);
    incr(save_ptr);
    enter_ordinary_math();
}

@ Subformulas of math formulas cause a new level of math mode to be entered,
on the semantic nest as well as the save stack. These subformulas arise in
several ways: (1)~A left brace by itself indicates the beginning of a
subformula that will be put into a box, thereby freezing its glue and
preventing line breaks. (2)~A subscript or superscript is treated as a
subformula if it is not a single character; the same applies to
the nucleus of things like \.{\\underline}. (3)~The \.{\\left} primitive
initiates a subformula that will be terminated by a matching \.{\\right}.
The group codes placed on |save_stack| in these three cases are
|math_group|, |math_group|, and |math_left_group|, respectively.

Here is the code that handles case (1); the other cases are not quite as
trivial, so we shall consider them later.

@c
void math_left_brace(void)
{
    pointer q;
    tail_append(new_noad());
    q = new_node(math_char_node, 0);
    nucleus(tail) = q;
    back_input();
    (void) scan_math(nucleus(tail), m_style);
}

@ If the inline directions of \.{\\pardir} and \.{\\mathdir} are
opposite, then this function will return true. Discovering that fact
is somewhat odd because it needs traversal of the |save_stack|.
The occurance of displayed equations is weird enough that this is
probably still better than having yet another field in the |input_stack|
structures.

None of this makes much sense if the inline direction of either one of
\.{\\pardir} or \.{\\mathdir} is vertical, but in that case the current
math machinery is ill suited anyway so I do not bother to test that.

@c
static boolean math_and_text_reversed_p(void)
{
    int i = save_ptr - 1;
    while (save_type(i) != level_boundary)
        i--;
    while (i < save_ptr) {
        if (save_type(i) == restore_old_value &&
            save_value(i) == int_base + par_direction_code) {
            if (textdir_opposite(math_direction_par, save_value(i - 1)))
                return true;
        }
        i++;
    }
    return false;
}

@ When we enter display math mode, we need to call |line_break| to
process the partial paragraph that has just been interrupted by the
display. Then we can set the proper values of |display_width| and
|display_indent| and |pre_display_size|.

@c
void enter_display_math(void)
{
    scaled w;                   /* new or partial |pre_display_size| */
    scaled l;                   /* new |display_width| */
    scaled s;                   /* new |display_indent| */
    pointer p;
    int n;                      /* scope of paragraph shape specification */
    if (head == tail ||         /* `\.{\\noindent\$\$}' or `\.{\$\${ }\$\$}' */
        (vlink(head) == tail && /* the 2nd of \.{\$\${ }\$\$} \.{\$\${ }\$\$} */
         type(tail) == local_par_node && vlink(tail) == null)) {
        if (vlink(head) == tail) {
            /* bug \#270: |resume_after_display| inserts a |local_par_node|, but if
               there is another display immediately following, we have to get rid
               of that node */
            flush_node(tail);
        }
        pop_nest();
        w = -max_dimen;
    } else {
        line_break(true, math_shift_group);
        w = actual_box_width(just_box, x_over_n(quad(get_cur_font()),1000) * math_pre_display_gap_factor_par);
    }
    /* now we are in vertical mode, working on the list that will contain the display */
    /* A displayed equation is considered to be three lines long, so we
       calculate the length and offset of line number |prev_graf+2|. */
    if (par_shape_par_ptr == null) {
        if ((hang_indent_par != 0) && (((hang_after_par >= 0) && (prev_graf_par + 2 > hang_after_par)) || (prev_graf_par + 1 < -hang_after_par))) {
            halfword used_hang_indent = swap_hang_indent(hang_indent_par);
            l = hsize_par - abs(used_hang_indent);
            if (used_hang_indent > 0)
                s = used_hang_indent;
            else
                s = 0;
        } else {
            l = hsize_par;
            s = 0;
        }
    } else {
        n = vinfo(par_shape_par_ptr + 1);
        if (prev_graf_par + 2 >= n)
            p = par_shape_par_ptr + 2 * n + 1;
        else
            p = par_shape_par_ptr + 2 * (prev_graf_par + 2) + 1;
        s = varmem[(p - 1)].cint;
        l = varmem[p].cint;
        s = swap_parshape_indent(s,l);
    }

    push_math(math_shift_group, display_style);
    mode = mmode;
    eq_word_define(int_base + cur_fam_code, -1);
    eq_word_define(dimen_base + pre_display_size_code, w);
    eq_word_define(dimen_base + display_width_code, l);
    eq_word_define(dimen_base + display_indent_code, s);
    eq_word_define(int_base + pre_display_direction_code, (math_and_text_reversed_p() ? -1 : 0));
    if (every_display_par != null)
        begin_token_list(every_display_par, every_display_text);
    if (nest_ptr == 1) {
        checked_page_filter(before_display);
        build_page();
    }
}

@ The next routine parses all variations of a delimiter code. The |extcode|
 tells what syntax form to use (\TeX, XeTeX, XeTeXnum, ...) , the
 |doclass| tells whether or not read a math class also (for \.{\\delimiter} c.s.).
 (the class is passed on for conversion to \.{\\mathchar}).

@c
static delcodeval do_scan_extdef_del_code(int extcode, boolean doclass)
{
    const char *hlp[] = {
        "I'm going to use 0 instead of that illegal code value.",
        NULL
    };
    delcodeval d;
    int mcls = 0, msfam = 0, mschr = 0, mlfam = 0, mlchr = 0;
    if (extcode == tex_mathcode) {      /* \.{\\delcode}, this is the easiest */
        scan_int();
        /*  "MFCCFCC or "FCCFCC */
        if (doclass) {
            mcls = (cur_val / 0x1000000);
            cur_val = (cur_val & 0xFFFFFF);
        }
        if (cur_val > 0xFFFFFF) {
            tex_error("Invalid delimiter code", hlp);
            cur_val = 0;
        }
        msfam = (cur_val / 0x100000);
        mschr = (cur_val % 0x100000) / 0x1000;
        mlfam = (cur_val & 0xFFF) / 0x100;
        mlchr = (cur_val % 0x100);
    } else if (extcode == umath_mathcode) {     /* \.{\\Udelcode} */
        /* <0-7>,<0-0xFF>,<0-0x10FFFF>  or <0-0xFF>,<0-0x10FFFF> */
        if (doclass) {
            scan_int();
            mcls = cur_val;
        }
        scan_int();
        msfam = cur_val;
        scan_char_num();
        mschr = cur_val;
        if (msfam < 0 || msfam > 255) {
            tex_error("Invalid delimiter code", hlp);
            msfam = 0;
            mschr = 0;
        }
        mlfam = 0;
        mlchr = 0;
    } else if (extcode == umathnum_mathcode) {  /* \.{\\Udelcodenum} */
        /* "FF<21bits> */
        /* the largest numeric value is $2^29-1$, but
           the top of bit 21 can't be used as it contains invalid USV's
         */
        if (doclass) {          /* such a primitive doesn't exist */
            confusion("umathnum_mathcode");
        }
        scan_int();
        msfam = (cur_val / 0x200000);
        mschr = cur_val & 0x1FFFFF;
        if (msfam < 0 || msfam > 255 || mschr > 0x10FFFF) {
            tex_error("Invalid delimiter code", hlp);
            msfam = 0;
            mschr = 0;
        }
        mlfam = 0;
        mlchr = 0;
    } else {
        /* something's gone wrong */
        confusion("unknown_extcode");
    }
    d.class_value = mcls;
    d.small_family_value = msfam;
    d.small_character_value = mschr;
    d.large_family_value = mlfam;
    d.large_character_value = mlchr;
    return d;
}

@ @c
void scan_extdef_del_code(int level, int extcode)
{
    delcodeval d;
    int p;
    scan_char_num();
    p = cur_val;
    scan_optional_equals();
    d = do_scan_extdef_del_code(extcode, false);
    set_del_code(p, d.small_family_value, d.small_character_value,
                 d.large_family_value, d.large_character_value,
                 (quarterword) (level));
}

@ @c
mathcodeval scan_mathchar(int extcode)
{
    char errstr[255] = { 0 };
    const char *hlp[] = {
        "I'm going to use 0 instead of that illegal code value.",
        NULL
    };
    mathcodeval d;
    int mcls = 0, mfam = 0, mchr = 0;
    if (extcode == tex_mathcode) {      /* \.{\\mathcode} */
        /* "TFCC */
        scan_int();
        if (cur_val > 0x8000) {
            /*
                tex_error("Invalid math code", hlp);
                cur_val = 0;
            */
            /* needed for latex: fallback to umathnum_mathcode */
            mfam = (cur_val / 0x200000) & 0x7FF;
            mcls = mfam % 0x08;
            mfam = mfam / 0x08;
            mchr = cur_val & 0x1FFFFF;
            if (mchr > 0x10FFFF) {
                tex_error("Invalid math code during > 0x8000 mathcode fallback", hlp);
                mcls = 0;
                mfam = 0;
                mchr = 0;
            }
        } else {
            if (cur_val < 0) {
                snprintf(errstr, 255, "Bad mathchar (%d)", (int)cur_val);
                tex_error(errstr, hlp);
                cur_val = 0;
            }
            mcls = (cur_val / 0x1000);
            mfam = ((cur_val % 0x1000) / 0x100);
            mchr = (cur_val % 0x100);
        }
    } else if (extcode == umath_mathcode) {
        /* <0-0x7> <0-0xFF> <0-0x10FFFF> */
        scan_int();
        mcls = cur_val;
        scan_int();
        mfam = cur_val;
        scan_char_num();
        mchr = cur_val;
        if (mcls < 0 || mcls > 7 || mfam > 255) {
            tex_error("Invalid math code", hlp);
            mchr = 0;
            mfam = 0;
            mcls = 0;
        }
    } else if (extcode == umathnum_mathcode) {
        /* "FFT<21bits> */
        /* the largest numeric value is $2^32-1$, but
           the top of bit 21 can't be used as it contains invalid USV's
         */
        /* Note: |scan_int| won't accept families 128-255 because these use bit 32 */
        scan_int();
        mfam = (cur_val / 0x200000) & 0x7FF;
        mcls = mfam % 0x08;
        mfam = mfam / 0x08;
        mchr = cur_val & 0x1FFFFF;
        if (mchr > 0x10FFFF) {
            tex_error("Invalid math code", hlp);
            mcls = 0;
            mfam = 0;
            mchr = 0;
        }
    } else {
        /* something's gone wrong */
        confusion("unknown_extcode");
    }
    d.class_value = mcls;
    d.family_value = mfam;
    d.character_value = mchr;
    return d;
}

@ @c
void scan_extdef_math_code(int level, int extcode)
{
    mathcodeval d;
    int p;
    scan_char_num();
    p = cur_val;
    scan_optional_equals();
    d = scan_mathchar(extcode);
    set_math_code(p, d.class_value,
                  d.family_value, d.character_value, (quarterword) (level));
}

@ this reads in a delcode when actually a mathcode is needed
@c
mathcodeval scan_delimiter_as_mathchar(int extcode)
{
    delcodeval dval;
    mathcodeval mval;
    dval = do_scan_extdef_del_code(extcode, true);
    mval.class_value = dval.class_value;
    mval.family_value = dval.small_family_value;
    mval.character_value = dval.small_character_value;
    return mval;
}

@ Recall that the |nucleus|, |subscr|, and |supscr| fields in a noad
are broken down into subfields called |type| and either |math_list| or
|(math_fam,math_character)|. The job of |scan_math| is to figure out
what to place in one of these principal fields; it looks at the
subformula that comes next in the input, and places an encoding of
that subformula into a given word of |mem|.

@c
#define get_next_nb_nr() do { get_x_token(); } while (cur_cmd==spacer_cmd||cur_cmd==relax_cmd)

int scan_math_style(pointer p, int mstyle)
{
    get_next_nb_nr();
    back_input();
    scan_left_brace();
    set_saved_record(0, saved_math, 0, p);
    incr(save_ptr);
    push_math(math_group, mstyle);
    return 1;
}

int scan_math(pointer p, int mstyle)
{
    /* label restart,reswitch,exit; */
    mathcodeval mval = { 0, 0, 0 };
    assert(p != null);
  RESTART:
    get_next_nb_nr();
  RESWITCH:
    switch (cur_cmd) {
    case letter_cmd:
    case other_char_cmd:
    case char_given_cmd:
        mval = get_math_code(cur_chr);
        if (mval.class_value == 8) {
            /* An active character that is an |outer_call| is allowed here */
            cur_cs = active_to_cs(cur_chr, true);
            cur_cmd = eq_type(cur_cs);
            cur_chr = equiv(cur_cs);
            x_token();
            back_input();
            goto RESTART;
        }
        break;
    case char_num_cmd:
        scan_char_num();
        cur_chr = cur_val;
        cur_cmd = char_given_cmd;
        goto RESWITCH;
        break;
    case math_char_num_cmd:
        if (cur_chr == 0)
            mval = scan_mathchar(tex_mathcode);
        else if (cur_chr == 1)
            mval = scan_mathchar(umath_mathcode);
        else if (cur_chr == 2)
            mval = scan_mathchar(umathnum_mathcode);
        else
            confusion("scan_math");
        break;
    case math_given_cmd:
        mval = mathchar_from_integer(cur_chr, tex_mathcode);
        break;
    case xmath_given_cmd:
        mval = mathchar_from_integer(cur_chr, umath_mathcode);
        break;
    case delim_num_cmd:
        if (cur_chr == 0)
            mval = scan_delimiter_as_mathchar(tex_mathcode);
        else if (cur_chr == 1)
            mval = scan_delimiter_as_mathchar(umath_mathcode);
        else
            confusion("scan_math");
        break;
    default:
        /* The pointer |p| is placed on |save_stack| while a complex subformula
           is being scanned. */
        back_input();
        scan_left_brace();
        set_saved_record(0, saved_math, 0, p);
        incr(save_ptr);
        push_math(math_group, mstyle);
        return 1;
    }
    type(p) = math_char_node;
    math_character(p) = mval.character_value;
    if ((mval.class_value == math_use_current_family_code) && cur_fam_par_in_range)
        math_fam(p) = cur_fam_par;
    else
        math_fam(p) = mval.family_value;
    return 0;
}

@ The |set_math_char| procedure creates a new noad appropriate to a given
math code, and appends it to the current mlist. However, if the math code
is sufficiently large, the |cur_chr| is treated as an active character and
nothing is appended.

@c
void set_math_char(mathcodeval mval)
{
    pointer p;                  /* the new noad */
    if (mval.class_value == 8) {
        /* An active character that is an |outer_call| is allowed here */
        cur_cs = active_to_cs(cur_chr, true);
        cur_cmd = eq_type(cur_cs);
        cur_chr = equiv(cur_cs);
        x_token();
        back_input();
    } else {
        pointer q;
        p = new_noad();
        q = new_node(math_char_node, 0);
        nucleus(p) = q;
        math_character(nucleus(p)) = mval.character_value;
        math_fam(nucleus(p)) = mval.family_value;
        if (mval.class_value == math_use_current_family_code) {
            if (cur_fam_par_in_range)
                math_fam(nucleus(p)) = cur_fam_par;
            subtype(p) = ord_noad_type;
        } else {
            switch (mval.class_value) {
                  /* *INDENT-OFF* */
                case 0: subtype(p) = ord_noad_type; break;
                case 1: subtype(p) = op_noad_type_normal; break;
                case 2: subtype(p) = bin_noad_type; break;
                case 3: subtype(p) = rel_noad_type; break;
                case 4: subtype(p) = open_noad_type; break;
                case 5: subtype(p) = close_noad_type; break;
                case 6: subtype(p) = punct_noad_type; break;
                  /* *INDENT-ON* */
            }
        }
        vlink(tail) = p;
        tail = p;
    }
}

@ The |math_char_in_text| procedure creates a new node representing a math char
in text code, and appends it to the current list. However, if the math code
is sufficiently large, the |cur_chr| is treated as an active character and
nothing is appended.

@c
void math_char_in_text(mathcodeval mval)
{
    pointer p;                  /* the new node */
    if (mval.class_value == 8) {
        /* An active character that is an |outer_call| is allowed here */
        cur_cs = active_to_cs(cur_chr, true);
        cur_cmd = eq_type(cur_cs);
        cur_chr = equiv(cur_cs);
        x_token();
        back_input();
    } else {
        p = new_char(fam_fnt(mval.family_value, text_size), mval.character_value);
        vlink(tail) = p;
        tail = p;
    }
}

@ @c
void math_math_comp(void)
{
    pointer q;
    tail_append(new_noad());
    subtype(tail) = (quarterword) cur_chr;
    q = new_node(math_char_node, 0);
    nucleus(tail) = q;
    if (cur_chr == over_noad_type)
        (void) scan_math(nucleus(tail), cramped_style(m_style));
    else
        (void) scan_math(nucleus(tail), m_style);
}

@ @c
void math_limit_switch(void)
{
    const char *hlp[] = {
        "I'm ignoring this misplaced \\limits or \\nolimits command.",
        NULL
    };
    if (head != tail) {
         if (type(tail) == simple_noad &&
             (subtype(tail) == op_noad_type_normal ||
              subtype(tail) == op_noad_type_limits ||
              subtype(tail) == op_noad_type_no_limits)) {
            subtype(tail) = (quarterword) cur_chr;
            return;
        }
    }
    tex_error("Limit controls must follow a math operator", hlp);
}

@ Delimiter fields of noads are filled in by the |scan_delimiter| routine.
The first parameter of this procedure is the |mem| address where the
delimiter is to be placed; the second tells if this delimiter follows
\.{\\radical} or not.

@c
static void scan_delimiter(pointer p, int r)
{
    delcodeval dval = { 0, 0, 0, 0, 0 };
    if (r == tex_mathcode) {    /* \.{\\radical} */
        dval = do_scan_extdef_del_code(tex_mathcode, true);
    } else if (r == umath_mathcode) {   /* \.{\\Uradical} */
        dval = do_scan_extdef_del_code(umath_mathcode, false);
    } else if (r == no_mathcode) {
        get_next_nb_nr();
        switch (cur_cmd) {
        case letter_cmd:
        case other_char_cmd:
            dval = get_del_code(cur_chr);
            break;
        case delim_num_cmd:
            if (cur_chr == 0)   /* \.{\\delimiter} */
                dval = do_scan_extdef_del_code(tex_mathcode, true);
            else if (cur_chr == 1)      /* \.{\\Udelimiter} */
                dval = do_scan_extdef_del_code(umath_mathcode, true);
            else
                confusion("scan_delimiter1");
            break;
        default:
            dval.small_family_value = -1;
            break;
        }
    } else {
        confusion("scan_delimiter2");
    }
    if (p == null)
        return;
    if (dval.small_family_value < 0) {
        const char *hlp[] = {
            "I was expecting to see something like `(' or `\\{' or",
            "`\\}' here. If you typed, e.g., `{' instead of `\\{', you",
            "should probably delete the `{' by typing `1' now, so that",
            "braces don't get unbalanced. Otherwise just proceed",
            "Acceptable delimiters are characters whose \\delcode is",
            "nonnegative, or you can use `\\delimiter <delimiter code>'.",
            NULL
        };
        back_error("Missing delimiter (. inserted)", hlp);
        small_fam(p) = 0;
        small_char(p) = 0;
        large_fam(p) = 0;
        large_char(p) = 0;
    } else {
        small_fam(p) = dval.small_family_value;
        small_char(p) = dval.small_character_value;
        large_fam(p) = dval.large_family_value;
        large_char(p) = dval.large_character_value;
    }
    return;
}

@ @c
void math_radical(void)
{
    halfword q;
    int chr_code = cur_chr;
    halfword options = 0;
    tail_append(new_node(radical_noad, chr_code));
    q = new_node(delim_node, 0);
    left_delimiter(tail) = q;
    while (1) {
        if (scan_keyword("width")) {
            scan_dimen(false,false,false);
            radicalwidth(tail) = cur_val ;
        } else if (scan_keyword("left")) {
            options = options | noad_option_left ;
        } else if (scan_keyword("middle")) {
            options = options | noad_option_middle ;
        } else if (scan_keyword("right")) {
            options = options | noad_option_right ;
        } else {
            break;
        }
    }
    radicaloptions(tail) = options;
    if (chr_code == 0)          /* \.{\\radical} */
        scan_delimiter(left_delimiter(tail), tex_mathcode);
    else if (chr_code == 1)     /* \.{\\Uradical} */
        scan_delimiter(left_delimiter(tail), umath_mathcode);
    else if (chr_code == 2)     /* \.{\\Uroot} */
        scan_delimiter(left_delimiter(tail), umath_mathcode);
    else if (chr_code == 3)     /* \.{\\Uunderdelimiter} */
        scan_delimiter(left_delimiter(tail), umath_mathcode);
    else if (chr_code == 4)     /* \.{\\Uoverdelimiter} */
        scan_delimiter(left_delimiter(tail), umath_mathcode);
    else if (chr_code == 5)     /* \.{\\Udelimiterunder} */
        scan_delimiter(left_delimiter(tail), umath_mathcode);
    else if (chr_code == 6)     /* \.{\\Udelimiterover} */
        scan_delimiter(left_delimiter(tail), umath_mathcode);
    else if (chr_code == 7)     /* \.{\\Uhextensible} */
        scan_delimiter(left_delimiter(tail), umath_mathcode);
    else
        confusion("math_radical");
    if (chr_code == 7) {
        q = new_node(sub_box_node, 0); /* type will change */
        nucleus(tail) = q;
        return;
    } else if (chr_code == 2) {
        /* the trick with the |vlink(q)| is used by |scan_math|
           to decide whether it needs to go on */
        q = new_node(math_char_node, 0);
        vlink(q) = tail;
        degree(tail) = q;
        if (!scan_math(degree(tail), sup_sup_style(m_style))) {
            vlink(degree(tail)) = null;
            q = new_node(math_char_node, 0);
            nucleus(tail) = q;
            (void) scan_math(nucleus(tail), cramped_style(m_style));
        }
    } else {
        q = new_node(math_char_node, 0);
        nucleus(tail) = q;
        (void) scan_math(nucleus(tail), cramped_style(m_style));
    }
}

@ @c
void math_ac(void)
{
    halfword q;
    mathcodeval t = { 0, 0, 0 };
    mathcodeval b = { 0, 0, 0 };
    mathcodeval o = { 0, 0, 0 };
    if (cur_cmd == accent_cmd) {
        const char *hlp[] = {
            "I'm changing \\accent to \\mathaccent here; wish me luck.",
            "(Accents are not the same in formulas as they are in text.)",
            NULL
        };
        tex_error("Please use \\mathaccent for accents in math mode", hlp);
    }
    tail_append(new_node(accent_noad, 0));
    if (cur_chr == 0) {         /* \.{\\mathaccent} */
        t = scan_mathchar(tex_mathcode);
    } else if (cur_chr == 1) {  /* \.{\\Umathaccent} */
        if (scan_keyword("fixed")) {
            /* top */
            subtype(tail) = 1;
            t = scan_mathchar(umath_mathcode);
        } else if (scan_keyword("both")) {
            /* top bottom */
            if (scan_keyword("fixed")) {
                subtype(tail) = 1;
            }
            t = scan_mathchar(umath_mathcode);
            if (scan_keyword("fixed")) {
                subtype(tail) += 2;
            }
            b = scan_mathchar(umath_mathcode);
        } else if (scan_keyword("bottom")) {
            /* bottom */
            if (scan_keyword("fixed")) {
                subtype(tail) = 2;
            }
            b = scan_mathchar(umath_mathcode);
        } else if (scan_keyword("top")) {
            /* top */
            if (scan_keyword("fixed")) {
                subtype(tail) = 1;
            }
            t = scan_mathchar(umath_mathcode);
        } else if (scan_keyword("overlay")) {
            /* overlay */
            if (scan_keyword("fixed")) {
                subtype(tail) = 1;
            }
            o = scan_mathchar(umath_mathcode);
        } else {
            /* top */
            t = scan_mathchar(umath_mathcode);
        }
        if (scan_keyword("fraction")) {
            scan_int();
            accentfraction(tail) = cur_val;
        }
    } else {
        confusion("mathaccent");
    }
    if (!(t.character_value == 0 && t.family_value == 0)) {
        q = new_node(math_char_node, 0);
        top_accent_chr(tail) = q;
        math_character(top_accent_chr(tail)) = t.character_value;
        if ((t.class_value == math_use_current_family_code) && cur_fam_par_in_range)
            math_fam(top_accent_chr(tail)) = cur_fam_par;
        else
            math_fam(top_accent_chr(tail)) = t.family_value;
    }
    if (!(b.character_value == 0 && b.family_value == 0)) {
        q = new_node(math_char_node, 0);
        bot_accent_chr(tail) = q;
        math_character(bot_accent_chr(tail)) = b.character_value;
        if ((b.class_value == math_use_current_family_code) && cur_fam_par_in_range)
            math_fam(bot_accent_chr(tail)) = cur_fam_par;
        else
            math_fam(bot_accent_chr(tail)) = b.family_value;
    }
    if (!(o.character_value == 0 && o.family_value == 0)) {
        q = new_node(math_char_node, 0);
        overlay_accent_chr(tail) = q;
        math_character(overlay_accent_chr(tail)) = o.character_value;
        if ((o.class_value == math_use_current_family_code) && cur_fam_par_in_range)
            math_fam(overlay_accent_chr(tail)) = cur_fam_par;
        else
            math_fam(overlay_accent_chr(tail)) = o.family_value;
    }
    q = new_node(math_char_node, 0);
    nucleus(tail) = q;
    (void) scan_math(nucleus(tail), cramped_style(m_style));
}

@ @c
pointer math_vcenter_group(pointer p)
{
    pointer q, r;
    q = new_noad();
    subtype(q) = vcenter_noad_type;
    r = new_node(sub_box_node, 0);
    nucleus(q) = r;
    math_list(nucleus(q)) = p;
    return q;
}

@ The routine that scans the four mlists of a \.{\\mathchoice} is very
much like the routine that builds discretionary nodes.

@c
void append_choices(void)
{
    tail_append(new_choice());
    incr(save_ptr);
    set_saved_record(-1, saved_choices, 0, 0);
    push_math(math_choice_group, display_style);
    scan_left_brace();
}

@ @c
void build_choices(void)
{
    pointer p;                  /* the current mlist */
    int prev_style;
    prev_style = m_style;
    unsave_math();
    p = fin_mlist(null);
    assert(saved_type(-1) == saved_choices);
    switch (saved_value(-1)) {
    case 0:
        display_mlist(tail) = p;
        break;
    case 1:
        text_mlist(tail) = p;
        break;
    case 2:
        script_mlist(tail) = p;
        break;
    case 3:
        script_script_mlist(tail) = p;
        decr(save_ptr);
        return;
        break;
    }                           /* there are no other cases */
    set_saved_record(-1, saved_choices, 0, (saved_value(-1) + 1));
    push_math(math_choice_group, (prev_style + 2));
    scan_left_brace();
}

@ Subscripts and superscripts are attached to the previous nucleus by the
action procedure called |sub_sup|.

@c
void sub_sup(void)
{
    pointer q;
    if (tail == head || (!scripts_allowed(tail))) {
        tail_append(new_noad());
        q = new_node(sub_mlist_node, 0);
        nucleus(tail) = q;
    }
    if (cur_cmd == sup_mark_cmd || cur_chr == sup_mark_cmd) {   /* |super_sub_script| */
        if (supscr(tail) != null) {
            const char *hlp[] = {
                "I treat `x^1^2' essentially like `x^1{}^2'.", NULL
            };
            tail_append(new_noad());
            q = new_node(sub_mlist_node, 0);
            nucleus(tail) = q;
            tex_error("Double superscript", hlp);
        }
        q = new_node(math_char_node, 0);
        supscr(tail) = q;
        (void) scan_math(supscr(tail), sup_style(m_style));
    } else if (cur_cmd == sub_mark_cmd || cur_chr == sub_mark_cmd) {
        if (subscr(tail) != null) {
            const char *hlp[] = {
                "I treat `x_1_2' essentially like `x_1{}_2'.", NULL
            };
            tail_append(new_noad());
            q = new_node(sub_mlist_node, 0);
            nucleus(tail) = q;
            tex_error("Double subscript", hlp);
        }
        q = new_node(math_char_node, 0);
        subscr(tail) = q;
        (void) scan_math(subscr(tail), sub_style(m_style));
    }
}

@ An operation like `\.{\\over}' causes the current mlist to go into a
state of suspended animation: |incompleat_noad| points to a |fraction_noad|
that contains the mlist-so-far as its numerator, while the denominator
is yet to come. Finally when the mlist is finished, the denominator will
go into the incompleat fraction noad, and that noad will become the
whole formula, unless it is surrounded by `\.{\\left}' and `\.{\\right}'
delimiters.

@c
void math_fraction(void)
{
    halfword c;                 /* the type of generalized fraction we are scanning */
    pointer q;
    halfword options = 0;
    halfword temp_value;
    c = cur_chr;
    if (incompleat_noad_par != null) {
        const char *hlp[] = {
            "I'm ignoring this fraction specification, since I don't",
            "know whether a construction like `x \\over y \\over z'",
            "means `{x \\over y} \\over z' or `x \\over {y \\over z}'.",
            NULL
        };
        if (c >= delimited_code) {
            scan_delimiter(null, no_mathcode);
            scan_delimiter(null, no_mathcode);
        }
        if ((c % delimited_code) == above_code)
            scan_normal_dimen();
        tex_error("Ambiguous; you need another { and }", hlp);
    } else {
        incompleat_noad_par = new_node(fraction_noad, 0);
        temp_value = new_node(sub_mlist_node, 0);
        numerator(incompleat_noad_par) = temp_value;
        math_list(numerator(incompleat_noad_par)) = vlink(head);
        vlink(head) = null;
        tail = head;
        m_style = cramped_style(m_style);

        if ((c % delimited_code) == skewed_code) {
            q = new_node(delim_node, 0);
            middle_delimiter(incompleat_noad_par) = q;
            scan_delimiter(middle_delimiter(incompleat_noad_par), no_mathcode);
        }
        if (c >= delimited_code) {
            q = new_node(delim_node, 0);
            left_delimiter(incompleat_noad_par) = q;
            q = new_node(delim_node, 0);
            right_delimiter(incompleat_noad_par) = q;
            scan_delimiter(left_delimiter(incompleat_noad_par), no_mathcode);
            scan_delimiter(right_delimiter(incompleat_noad_par), no_mathcode);
        }
        switch (c % delimited_code) {
            case above_code:
                while (1) {
                    if (scan_keyword("exact")) {
                        options = options | noad_option_exact ;
                    } else {
                        break;
                    }
                }
                fractionoptions(incompleat_noad_par) = options;
                scan_normal_dimen();
                thickness(incompleat_noad_par) = cur_val;
                break;
            case over_code:
                thickness(incompleat_noad_par) = default_code;
                break;
            case atop_code:
                thickness(incompleat_noad_par) = 0;
                break;
            case skewed_code:
                while (1) {
                    if (scan_keyword("exact")) {
                        options = options | noad_option_exact ;
                    } else if (scan_keyword("noaxis")) {
                        options = options | noad_option_no_axis ;
                    } else {
                        break;
                    }
                }
                fractionoptions(incompleat_noad_par) = options;
                thickness(incompleat_noad_par) = 0;
                break;
        }
    }
}

@ At the end of a math formula or subformula, the |fin_mlist| routine is
called upon to return a pointer to the newly completed mlist, and to
pop the nest back to the enclosing semantic level. The parameter to
|fin_mlist|, if not null, points to a |fence_noad| that ends the
current mlist; this |fence_noad| has not yet been appended.

@c
pointer fin_mlist(pointer p)
{
    pointer q;                  /* the mlist to return */
    if (incompleat_noad_par != null) {
        if (denominator(incompleat_noad_par) != null) {
            type(denominator(incompleat_noad_par)) = sub_mlist_node;
        } else {
            q = new_node(sub_mlist_node, 0);
            denominator(incompleat_noad_par) = q;
        }
        math_list(denominator(incompleat_noad_par)) = vlink(head);
        if (p == null) {
            q = incompleat_noad_par;
        } else {
            q = math_list(numerator(incompleat_noad_par));
            if ((type(q) != fence_noad) || (subtype(q) != left_noad_side)
                || (delim_par == null))
                confusion("right");     /* this can't happen */
            math_list(numerator(incompleat_noad_par)) = vlink(delim_par);
            vlink(delim_par) = incompleat_noad_par;
            vlink(incompleat_noad_par) = p;
        }
    } else {
        vlink(tail) = p;
        q = vlink(head);
    }
    pop_nest();
    return q;
}

@ Now at last we're ready to see what happens when a right brace occurs
in a math formula. Two special cases are simplified here: Braces are effectively
removed when they surround a single Ord without sub/superscripts, or when they
surround an accent that is the nucleus of an Ord atom.

@c
void close_math_group(pointer p)
{
    int old_style = m_style;
    unsave_math();

    decr(save_ptr);
    assert(saved_type(0) == saved_math);
    type(saved_value(0)) = sub_mlist_node;
    p = fin_mlist(null);
    math_list(saved_value(0)) = p;
    if (p != null) {
        if (vlink(p) == null) {
            if (type(p) == simple_noad && subtype(p) == ord_noad_type) {
                if (subscr(p) == null && supscr(p) == null) {
                    type(saved_value(0)) = type(nucleus(p));
                    if (type(nucleus(p)) == math_char_node) {
                        math_fam(saved_value(0)) = math_fam(nucleus(p));
                        math_character(saved_value(0)) =
                            math_character(nucleus(p));
                    } else {
                        math_list(saved_value(0)) = math_list(nucleus(p));
                        math_list(nucleus(p)) = null;
                    }
                    delete_attribute_ref(node_attr(saved_value(0)));
                    node_attr(saved_value(0)) = node_attr(nucleus(p));
                    node_attr(nucleus(p)) = null;
                    flush_node(p);
                }
            } else if (type(p) == accent_noad) {
                if (saved_value(0) == nucleus(tail)) {
                    if (type(tail) == simple_noad
                        && subtype(tail) == ord_noad_type) {
                        pointer q = head;
                        while (vlink(q) != tail)
                            q = vlink(q);
                        vlink(q) = p;
                        nucleus(tail) = null;
                        subscr(tail) = null;
                        supscr(tail) = null;
                        delete_attribute_ref(node_attr(p));
                        node_attr(p) = node_attr(tail);
                        node_attr(tail) = null;
                        flush_node(tail);
                        tail = p;
                    }
                }
            }
        }
    }
    if (vlink(saved_value(0)) > 0) {
        pointer q;
        q = new_node(math_char_node, 0);
        nucleus(vlink(saved_value(0))) = q;
        vlink(saved_value(0)) = null;
        saved_value(0) = q;
        (void) scan_math(saved_value(0), old_style);
        /* restart */
    }
}

@ We have dealt with all constructions of math mode except `\.{\\left}' and
`\.{\\right}', so the picture is completed by the following sections of
the program. The |middle| feature of eTeX allows one ore several \.{\\middle}
delimiters to appear between \.{\\left} and \.{\\right}.

@c
void math_left_right(void)
{
    halfword t;      /* |left_noad_side| .. |right_noad_side| */
    pointer p;       /* new noad */
    pointer q;       /* resulting mlist */
    pointer r;       /* temporary */
    halfword ht = 0;
    halfword dp = 0;
    halfword options = 0;
    halfword type = -1 ;
    t = cur_chr;

    if (t > 10) {
        /* we have \Uleft \Uright \Umiddle */
        t = t - 10;
        while (1) {
            if (scan_keyword("height")) {
                scan_dimen(false,false,false);
                ht = cur_val ;
            } else if (scan_keyword("depth")) {
                scan_dimen(false,false,false);
                dp = cur_val ;
            } else if (scan_keyword("axis")) {
                options = options | noad_option_axis ;
            } else if (scan_keyword("noaxis")) {
                options = options | noad_option_no_axis ;
            } else if (scan_keyword("exact")) {
                options = options | noad_option_exact ;
            } else if (scan_keyword("class")) {
                scan_int();
                type = cur_val ;
            } else {
                break;
            }
        }
    }

    if ((t != no_noad_side) && (t != left_noad_side) && (cur_group != math_left_group)) {
        if (cur_group == math_shift_group) {
            scan_delimiter(null, no_mathcode);
            if (t == middle_noad_side) {
                const char *hlp[] = {
                    "I'm ignoring a \\middle that had no matching \\left.",
                    NULL
                };
                tex_error("Extra \\middle", hlp);
            } else {
                const char *hlp[] = {
                    "I'm ignoring a \\right that had no matching \\left.",
                    NULL
                };
                tex_error("Extra \\right", hlp);
            }
        } else {
            off_save();
        }
    } else {
        p = new_noad();
        type(p) = fence_noad;
        subtype(p) = (quarterword) t;
        r = new_node(delim_node, 0);
        delimiter(p) = r;

        delimiterheight(p) = ht;
        delimiterdepth(p) = dp;
        delimiteroptions(p) = options;
        delimiterclass(p) = type;
        delimiteritalic(p) = 0;

        scan_delimiter(delimiter(p), no_mathcode);

        if (t == no_noad_side) {
            tail_append(new_noad());
            subtype(tail) = inner_noad_type;
            r = new_node(sub_mlist_node, 0);
            nucleus(tail) = r;
            math_list(nucleus(tail)) = p;
            return ;
        }

        if (t == left_noad_side) {
            q = p;
        } else {
            q = fin_mlist(p);
            unsave_math();
        }
        if (t != right_noad_side) {
            push_math(math_left_group, m_style);
            vlink(head) = q;
            tail = p;
            delim_par = p;
        } else {
            tail_append(new_noad());
            subtype(tail) = inner_noad_type;
            r = new_node(sub_mlist_node, 0);
            nucleus(tail) = r;
            math_list(nucleus(tail)) = q;
        }
    }
}

@ \TeX\ gets to the following part of the program when
the first `\.\$' ending a display has been scanned.

@c
static void check_second_math_shift(void)
{
    get_x_token();
    if (cur_cmd != math_shift_cmd) {
        const char *hlp[] = {
            "The `$' that I just saw supposedly matches a previous `$$'.",
            "So I shall assume that you typed `$$' both times.",
            NULL
        };
        back_error("Display math should end with $$", hlp);
    }
}

static void check_display_math_end(void)
{
    if (cur_chr != cramped_display_style) {
        const char *hlp[] = {
            "I shall assume that you typed that.",
            NULL
        };
        tex_error("Display math should end with \\Ustopdisplaymath", hlp);
    }
}

static void check_inline_math_end(void)
{
    if (cur_chr != cramped_text_style) {
        const char *hlp[] = {
            "I shall assume that you typed that.",
            NULL
        };
        tex_error("Inline math should end with \\Ustopmath", hlp);
    }
}

@ @c
static void resume_after_display(void)
{
    if (cur_group != math_shift_group)
        confusion("display");
    unsave_math();
    prev_graf_par = prev_graf_par + 3;
    push_nest();
    mode = hmode;
    space_factor_par = 1000;
    /* this needs to be intercepted in the display math start ! */
    tail_append(make_local_par_node(penalty_par_code));
    get_x_token();
    if (cur_cmd != spacer_cmd)
        back_input();
    if (nest_ptr == 1) {
        normal_page_filter(after_display);
        build_page();
    }
}

@  The fussiest part of math mode processing occurs when a displayed formula is
being centered and placed with an optional equation number.

At this time we are in vertical mode (or internal vertical mode).

  |p| points to the mlist for the formula.
  |a| is either |null| or it points to a box containing the equation number.
  |l| is true if there was an \.{\\leqno}/ (so |a| is a horizontal box).

@c
#define inject_display_skip_before(g) \
    switch (display_skip_mode_par) { \
        case 0 : /* normal tex */ \
            tail_append(new_param_glue(g)); \
            break;\
        case 1 : /* always */ \
            tail_append(new_param_glue(g)); \
            break; \
        case 2 : /* non-zero */ \
            if (g != 0 && ! glue_is_zero(glue_par(g))) \
                tail_append(new_param_glue(g)); \
            break; \
        case 3: /* ignore */ \
            break; \
    }

#define inject_display_skip_after(g) \
    switch (display_skip_mode_par) { \
        case 0 : /* normal tex */ \
            if (g != 0 && glue_is_positive(glue_par(g))) \
                tail_append(new_param_glue(g)); \
            break; \
        case 1 : /* always */ \
            tail_append(new_param_glue(g)); \
            break; \
        case 2 : /* non-zero */ \
            if (g != 0 && ! glue_is_zero(glue_par(g))) \
                tail_append(new_param_glue(g)); \
            break; \
        case 3: /* ignore */ \
            break; \
    }

static void finish_displayed_math(boolean l, pointer eqno_box, pointer p)
{
    pointer eq_box;             /* box containing the equation */
    scaled eq_w;                /* width of the equation */
    scaled line_w;              /* width of the line */
    scaled eqno_w;              /* width of equation number */
    scaled eqno_w2;             /* width of equation number plus space to separate from equation */
    scaled line_s;              /* move the line right this much */
    scaled d;                   /* displacement of equation in the line */
    small_number g1, g2;        /* glue parameter codes for before and after */
    pointer r,s;                /* kern nodes used to position the display */
    pointer t;                  /* tail of adjustment list */
    pointer pre_t;              /* tail of pre-adjustment list */
    boolean swap_dir;           /* true if the math and surrounding text dirs are opposed */
    scaled eqno_width;
    swap_dir = (pre_display_direction_par < 0 ? true : false );
    if (eqno_box != null && swap_dir)
        l = !l;
    adjust_tail = adjust_head;
    pre_adjust_tail = pre_adjust_head;
    eq_box = hpack(p, 0, additional, -1);
    subtype(eq_box) = equation_list; /* new */
    build_attribute_list(eq_box);
    p = list_ptr(eq_box);
    t = adjust_tail;
    adjust_tail = null;
    pre_t = pre_adjust_tail;
    pre_adjust_tail = null;
    eq_w = width(eq_box);
    line_w = display_width_par;
    line_s = display_indent_par;
    if (eqno_box == null) {
        eqno_w = 0;
        eqno_width = 0;
        eqno_w2 = 0;
    } else {
        eqno_w = width(eqno_box);
        eqno_width = eqno_w;
        eqno_w2 = eqno_w + round_xn_over_d(math_eqno_gap_step_par, get_math_quad_style(text_style), 1000);
        subtype(eqno_box) = equation_number_list; /* new */
     /* build_attribute_list(eqno_box); */ /* probably already set */
   }
    if (eq_w + eqno_w2 > line_w) {
        /* The user can force the equation number to go on a separate line
           by causing its width to be zero. */
        if ((eqno_w != 0) && ((eq_w - total_shrink[normal] + eqno_w2 <= line_w)
                || (total_shrink[sfi] != 0)
                || (total_shrink[fil] != 0)
                || (total_shrink[fill] != 0)
                || (total_shrink[filll] != 0))) {
            list_ptr(eq_box) = null;
            flush_node(eq_box);
            eq_box = hpack(p, line_w - eqno_w2, exactly, -1);
            subtype(eq_box) = equation_list; /* new */
            build_attribute_list(eq_box);
        } else {
            eqno_w = 0;
            if (eq_w > line_w) {
                list_ptr(eq_box) = null;
                flush_node(eq_box);
                eq_box = hpack(p, line_w, exactly, -1);
                subtype(eq_box) = equation_list; /* new */
                build_attribute_list(eq_box);
            }
        }
        eq_w = width(eq_box);
    }
    /* We try first to center the display without regard to the existence of
       the equation number. If that would make it too close (where ``too close''
       means that the space between display and equation number is less than the
       width of the equation number), we either center it in the remaining space
       or move it as far from the equation number as possible. The latter alternative
       is taken only if the display begins with glue, since we assume that the
       user put glue there to control the spacing precisely.
     */
    d = half(line_w - eq_w);
    if ((eqno_w > 0) && (d < 2 * eqno_w)) {     /* too close */
        d = half(line_w - eq_w - eqno_w);
        if (p != null)
            if (!is_char_node(p))
                if (type(p) == glue_node)
                    d = 0;
    }

    tail_append(new_penalty(pre_display_penalty_par,after_display_penalty));
    if ((d + line_s <= pre_display_size_par) || l) {        /* not enough clearance */
        g1 = above_display_skip_code;
        g2 = below_display_skip_code;
    } else {
        g1 = above_display_short_skip_code;
        g2 = below_display_short_skip_code;
    }

    /* If the equation number is set on a line by itself, either before or
       after the formula, we append an infinite penalty so that no page break will
       separate the display from its number; and we use the same size and
       displacement for all three potential lines of the display, even though
       `\.{\\parshape}' may specify them differently.
     */
     /* \.{\\leqno} on a forced single line due to |width=0| */
     /* it follows that |type(a)=hlist_node| */

    if (eqno_box && l && (eqno_w == 0)) {
     /* if (math_direction_par==dir_TLT) { */
            shift_amount(eqno_box) = 0;
     /* } else {                       */
     /* }                              */
        append_to_vlist(eqno_box,lua_key_index(equation_number));
        tail_append(new_penalty(inf_penalty,equation_number_penalty));
    } else {
        inject_display_skip_before(g1);
    }

    if (eqno_w != 0) {
        r = new_kern(line_w - eq_w - eqno_w - d);
        if (l) {
            if (swap_dir) {
                if (math_direction_par==dir_TLT) {
                    /* TRT + TLT + \eqno,    (swap_dir=true,  math_direction_par=TLT, l=true)  */
#ifdef DEBUG
        fprintf(stderr, "\nDEBUG: CASE 1\n");
#endif
                    s = new_kern(width(r) + eqno_w);
                    try_couple_nodes(eqno_box,r);
                    try_couple_nodes(r,eq_box);
                    try_couple_nodes(eq_box,s);
                } else {
                    /* TLT + TRT + \eqno,    (swap_dir=true,  math_direction_par=TRT, l=true) */
#ifdef DEBUG
        fprintf(stderr, "\nDEBUG: CASE 2\n");
#endif
                    try_couple_nodes(eqno_box,r);
                    try_couple_nodes(r,eq_box);
                }
            } else {
                if (math_direction_par==dir_TLT) {
                    /* TLT + TLT + \leqno,   (swap_dir=false, math_direction_par=TLT, l=true) */ /* OK */
#ifdef DEBUG
        fprintf(stderr, "\nDEBUG: CASE 3\n");
#endif
                    s = new_kern(width(r) + eqno_w);
                } else {
                    /* TRT + TRT + \leqno,    (swap_dir=false, math_direction_par=TRT, l=true) */
#ifdef DEBUG
        fprintf(stderr, "\nDEBUG: CASE 4\n");
#endif
                    s = new_kern(width(r));
                }
                try_couple_nodes(eqno_box,r);
                try_couple_nodes(r,eq_box);
                try_couple_nodes(eq_box,s);
            }
            eq_box = eqno_box;
        } else {
            if (swap_dir) {
                if (math_direction_par==dir_TLT) {
                    /* TRT + TLT + \leqno,   (swap_dir=true,  math_direction_par=TLT, l=false) */
#ifdef DEBUG
        fprintf(stderr, "\nDEBUG: CASE 5\n");
#endif
                } else {
                    /* TLT + TRT + \leqno,   (swap_dir=true,  math_direction_par=TRT, l=false) */
#ifdef DEBUG
        fprintf(stderr, "\nDEBUG: CASE 6\n");
#endif
                }
                try_couple_nodes(eq_box,r);
                try_couple_nodes(r,eqno_box);
            } else {
                if (math_direction_par==dir_TLT) {
                    /*  TLT + TLT + \eqno,    (swap_dir=false, math_direction_par=TLT, l=false) */ /* OK */
#ifdef DEBUG
        fprintf(stderr, "\nDEBUG: CASE 7\n");
#endif
                    s = new_kern(d);
                } else {
                    /* TRT + TRT + \eqno,   (swap_dir=false, math_direction_par=TRT, l=false) */
#ifdef DEBUG
        fprintf(stderr, "\nDEBUG: CASE 8\n");
#endif
                    s = new_kern(width(r) + eqno_w);
                }
                try_couple_nodes(s,eq_box);
                try_couple_nodes(eq_box,r);
                try_couple_nodes(r,eqno_box);
                eq_box = s;
            }
        }
        eq_box = hpack(eq_box, 0, additional, -1);
        subtype(eq_box) = equation_list; /* new */
        build_attribute_list(eq_box);
        shift_amount(eq_box) = line_s;
    } else {
        shift_amount(eq_box) = line_s + d;
    }
/* check for prev: */
    append_to_vlist(eq_box,lua_key_index(equation));

    if ((eqno_box != null) && (eqno_w == 0) && !l) {
        tail_append(new_penalty(inf_penalty,equation_number_penalty));
     /* if (math_direction_par==dir_TLT) { */
            shift_amount(eqno_box) = line_s + line_w - eqno_width ;
     /* } else {                       */
     /* }                              */
        append_to_vlist(eqno_box,lua_key_index(equation_number));
        g2 = 0;
    }
    if (t != adjust_head) {     /* migrating material comes after equation number */
        vlink(tail) = vlink(adjust_head);
        /* needs testing */
        alink(adjust_tail) = alink(tail);
        tail = t;
    }
    if (pre_t != pre_adjust_head) {
        vlink(tail) = vlink(pre_adjust_head);
        /* needs testing */
        alink(pre_adjust_tail) = alink(tail);
        tail = pre_t;
    }
    tail_append(new_penalty(post_display_penalty_par,after_display_penalty));
    inject_display_skip_after(g2);
    resume_after_display();
}

@ @c
void after_math(void)
{
    int m;                      /* |mmode| or |-mmode| */
    pointer p;                  /* the formula */
    pointer a = null;           /* box containing equation number */
    boolean l = false;          /* `\.{\\leqno}' instead of `\.{\\eqno}' */
    m = mode;
    p = fin_mlist(null);        /* this pops the nest */
    if (cur_cmd == math_shift_cs_cmd &&
        (cur_chr == text_style || cur_chr == display_style)) {
        you_cant();
    }
    if (mode == -m) {           /* end of equation number */
        if (cur_cmd == math_shift_cmd) {
            check_second_math_shift();
        } else {
            check_display_math_end();
        }
        run_mlist_to_hlist(p, false, text_style);
        a = hpack(vlink(temp_head), 0, additional, -1);
        build_attribute_list(a);
        unsave_math();
        decr(save_ptr);         /* now |cur_group=math_shift_group| */
        assert(saved_type(0) == saved_eqno);
        if (saved_value(0) == 1)
            l = true;
        m = mode;
        p = fin_mlist(null);

    }
    if (m < 0) {
        /* The |unsave| is done after everything else here; hence an appearance of
           `\.{\\mathsurround}' inside of `\.{\$...\$}' affects the spacing at these
           particular \.\$'s. This is consistent with the conventions of
           `\.{\$\$...\$\$}', since `\.{\\abovedisplayskip}' inside a display affects the
           space above that display.
         */
        if (cur_cmd == math_shift_cs_cmd) {
            check_inline_math_end();
        }
        tail_append(new_math(math_surround_par, before));
        /* begin mathskip code */
        switch (math_skip_mode) {
            case 0 :
                /* obey mathsurround when zero glue */
                if (! glue_is_zero(math_skip_par)) {
                    copy_glue_values(tail,math_skip_par);
                    surround(tail) = 0;
                }
                break ;
            case 1 :
                /* always left */
            case 3 :
                /* always both */
            case 6 :
                /* only when skip */
                copy_glue_values(tail,math_skip_par);
                surround(tail) = 0;
                break ;
            case 2 :
                /* only right */
                surround(tail) = 0;
                break ;
            case 4 :
                /* ignore, obey marthsurround */
                break ;
            case 5:
                /* all spacing disabled */
                surround(tail) = 0;
                break ;
        }
        /* end mathskip code */
        if (dir_math_save) {
            tail_append(new_dir(math_direction_par));
        }
        run_mlist_to_hlist(p, (mode > 0), text_style);
        vlink(tail) = vlink(temp_head);
        while (vlink(tail) != null) {
            tail = vlink(tail);
        }
        if (dir_math_save) {
            tail_append(new_dir(math_direction_par - dir_swap));
        }
        dir_math_save = false;
        tail_append(new_math(math_surround_par, after));
        /* begin mathskip code */
        switch (math_skip_mode) {
            case 0 :
                /* obey mathsurround when zero glue */
                if (! glue_is_zero(math_skip_par)) {
                    copy_glue_values(tail,math_skip_par);
                    surround(tail) = 0;
                }
                break ;
            case 2 :
                /* always right */
            case 3 :
                /* always both */
            case 6 :
                /* only when skip */
                copy_glue_values(tail,math_skip_par);
                surround(tail) = 0;
                break ;
            case 1 :
                /* only left */
                surround(tail) = 0;
                break ;
            case 4 :
                /* ignore, obey marthsurround */
                break ;
            case 5:
                /* all spacing disabled */
                surround(tail) = 0;
                break ;
        }
        /* end mathskip code */
        space_factor_par = 1000;
        unsave_math();
    } else {
        if (a == null) {
            if (cur_cmd == math_shift_cmd) {
                check_second_math_shift();
            } else {
                check_display_math_end();
            }
        }
        run_mlist_to_hlist(p, false, display_style);
        finish_displayed_math(l, a, vlink(temp_head));
    }
}

@ When \.{\\halign} appears in a display, the alignment routines operate
essentially as they do in vertical mode. Then the following program is
activated, with |p| and |q| pointing to the beginning and end of the
resulting list, and with |aux_save| holding the |prev_depth| value.

@c
void finish_display_alignment(pointer p, pointer q, halfword saved_prevdepth)
{
    do_assignments();
    if (cur_cmd == math_shift_cmd) {
        check_second_math_shift();
    } else {
        check_display_math_end();
    }
    pop_nest();
    tail_append(new_penalty(pre_display_penalty_par,before_display_penalty));
    inject_display_skip_before(above_display_skip_code);
    vlink(tail) = p;
    if (p != null)
        tail = q;
    tail_append(new_penalty(post_display_penalty_par,after_display_penalty));
    inject_display_skip_after(below_display_skip_code);
    cur_list.prev_depth_field = saved_prevdepth;
    resume_after_display();
}

@ Interface to \.{\\Umath} and \.{\\mathstyle}

@c
void setup_math_style(void)
{
    pointer q;
    tail_append(new_noad());
    q = new_node(math_char_node, 0);
    nucleus(tail) = q;
    (void) scan_math_style(nucleus(tail), num_style(m_style));
}

@ @c
void print_math_style(void)
{
    if (abs(mode) == mmode)
        print_int(m_style);
    else
        print_int(-1);
}
