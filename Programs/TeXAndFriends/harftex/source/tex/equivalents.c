/*

equivalents.w

Copyright 2009-2010 Taco Hoekwater <taco@@luatex.org>

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

halfword last_cs_name = null_cs;

/*tex |eqtb[p]| has just been restored or retained. */

static void diagnostic_trace(halfword p, const char *s)
{
    begin_diagnostic();
    print_char('{');
    tprint(s);
    print_char(' ');
    show_eqtb(p);
    print_char('}');
    end_diagnostic(false);
}

void show_eqtb_meaning(halfword n);

/*tex

Now that we have studied the data structures for \TeX's semantic routines, we
ought to consider the data structures used by its syntactic routines. In other
words, our next concern will be the tables that \TeX\ looks at when it is
scanning what the user has written.

The biggest and most important such table is called |eqtb|. It holds the current
``equivalents'' of things; i.e., it explains what things mean or what their
current values are, for all quantities that are subject to the nesting structure
provided by \TeX's grouping mechanism. There are six parts to |eqtb|:

1) |eqtb[null_cs]| holds the current equivalent of the zero-length control
sequence.

2) |eqtb[hash_base..(glue_base-1)]| holds the current equivalents of single- and
multiletter control sequences.

3) |eqtb[glue_base..(local_base-1)]| holds the current equivalents of glue
parameters like the current baselineskip.

4) |eqtb[local_base..(int_base-1)]| holds the current equivalents of local
halfword quantities like the current box registers, the current ``catcodes,'' the
current font, and a pointer to the current paragraph shape.

5) |eqtb[int_base..(dimen_base-1)]| holds the current equivalents of fullword
integer parameters like the current hyphenation penalty.

6) |eqtb[dimen_base..eqtb_size]| holds the current equivalents of fullword
dimension parameters like the current hsize or amount of hanging indentation.

Note that, for example, the current amount of baselineskip glue is determined by
the setting of a particular location in region~3 of |eqtb|, while the current
meaning of the control sequence `\.{\\baselineskip}' (which might have been
changed by \.{\\def} or \.{\\let}) appears in region~2.

The last two regions of |eqtb| have fullword values instead of the three fields
|eq_level|, |eq_type|, and |equiv|. An |eq_type| is unnecessary, but \TeX\ needs
to store the |eq_level| information in another array called |xeq_level|.

*/

memory_word *eqtb;
halfword eqtb_top;
quarterword xeq_level[(eqtb_size + 1)];

void initialize_equivalents(void)
{
    int k;
    for (k = int_base; k <= eqtb_size; k++)
        xeq_level[k] = level_one;
}

/*tex

The nested structure provided by `$\.{\char'173}\ldots\.{\char'175}$' groups in
\TeX\ means that |eqtb| entries valid in outer groups should be saved and
restored later if they are overridden inside the braces. When a new |eqtb| value
is being assigned, the program therefore checks to see if the previous entry
belongs to an outer level. In such a case, the old value is placed on the
|save_stack| just before the new value enters |eqtb|. At the end of a grouping
level, i.e., when the right brace is sensed, the |save_stack| is used to restore
the outer values, and the inner ones are destroyed.

Entries on the |save_stack| are of type |save_record|. The top item on this stack
is |save_stack[p]|, where |p=save_ptr-1|; it contains three fields called
|save_type|, |save_level|, and |save_value|, and it is interpreted in one of four
ways:

1) If |save_type(p)=restore_old_value|, then |save_value(p)| is a location in
|eqtb| whose current value should be destroyed at the end of the current group
and replaced by |save_word(p-1)| (|save_type(p-1)==saved_eqtb|). Furthermore if
|save_value(p)>=int_base|, then |save_level(p)| should replace the corresponding
entry in |xeq_level| (if |save_value(p)<int_base|, then the level is part of
|save_word(p-1)|).

2) If |save_type(p)=restore_zero|, then |save_value(p)| is a location in |eqtb|
whose current value should be destroyed at the end of the current group, when it
should be replaced by the current value of |eqtb[undefined_control_sequence]|.

3) If |save_type(p)=insert_token|, then |save_value(p)| is a token that should be
inserted into \TeX's input when the current group ends.

4) If |save_type(p)=level_boundary|, then |save_level(p)| is a code explaining
what kind of group we were previously in, and |save_value(p)| points to the level
boundary word at the bottom of the entries for that group. Furthermore,
|save_value(p-1)| contains the source line number at which the current level of
grouping was entered, this field has itself a type: |save_type(p-1)==saved_line|.

Besides this `official' use, various subroutines push temporary variables on the
save stack when it is handy to do so. These all have an explicit |save_type|, and
they are:

|saved_adjust| signifies an adjustment is beging scanned,
|saved_insert| an insertion is being scanned,
|saved_disc| the \.{\\discretionary} sublist we are working on right now,
|saved_boxtype| whether a \.{\\localbox} is \.{\\left} or \.{\\right},
|saved_textdir| a text direction to be restored,
|saved_eqno| diffentiates between \.{\\eqno} and \.{\\leqno},
|saved_choices| the \.{\\mathchoices} sublist we are working on right now,
|saved_math| and interrupted math list,
|saved_boxcontext| the box context value,
|saved_boxspec| the box \.{to} or \.{spread} specification,
|saved_boxdir| the box \.{dir} specification,
|saved_boxattr| the box \.{attr} specification,
|saved_boxpack| the box \.{pack} specification.

The global variable |cur_group| keeps track of what sort of group we are
currently in. Another global variable, |cur_boundary|, points to the topmost
|level_boundary| word. And |cur_level| is the current depth of nesting. The
routines are designed to preserve the condition that no entry in the |save_stack|
or in |eqtb| ever has a level greater than |cur_level|.

*/

save_record *save_stack;
int save_ptr;                        /* first unused entry on the save stack */
int max_save_stack;                  /* maximum usage of save stack */
quarterword cur_level = level_one;   /* current nesting level for groups */
group_code cur_group = bottom_level; /* current group type */
int cur_boundary;                    /* where the current level begins */

/*tex

At this time it might be a good idea for the reader to review the introduction to
|eqtb| that was given above just before the long lists of parameter names. Recall
that the ``outer level'' of the program is |level_one|, since undefined control
sequences are assumed to be ``defined'' at |level_zero|.

The following macro is used to test if there is room for up to eight more entries
on |save_stack|. By making a conservative test like this, we can get by with
testing for overflow in only a few places.

*/

#define check_full_save_stack() do { \
    if (save_ptr>max_save_stack) { \
        max_save_stack=save_ptr; \
        if (max_save_stack>save_size-8) \
            overflow("save size",(unsigned)save_size); \
    } \
} while (0)

/*tex

Procedure |new_save_level| is called when a group begins. The argument is a group
identification code like `|hbox_group|'. After calling this routine, it is safe
to put six more entries on |save_stack|.

In some cases integer-valued items are placed onto the |save_stack| just below a
|level_boundary| word, because this is a convenient place to keep information
that is supposed to ``pop up'' just when the group has finished. For example,
when `\.{\\hbox to 100pt}' is being treated, the 100pt dimension is stored on
|save_stack| just before |new_save_level| is called.

*/

void new_save_level(group_code c)
{   /*tex We begin a new level of grouping. */
    check_full_save_stack();
    set_saved_record(0, saved_line, 0, line);
    incr(save_ptr);
    save_type(save_ptr) = level_boundary;
    save_level(save_ptr) = cur_group;
    save_value(save_ptr) = cur_boundary;
    if (cur_level == max_quarterword) {
        overflow("grouping levels", max_quarterword - min_quarterword);
    }
    /*tex We quit if |(cur_level+1)| is too big to be stored in |eqtb|. */
    cur_boundary = save_ptr;
    cur_group = c;
    if (tracing_groups_par > 0)
        group_trace(false);
    incr(cur_level);
    incr(save_ptr);
}

static const char *save_stack_type(int v)
{
    const char *s = "";
    switch (save_type(v)) {
        case restore_old_value: s = "restore_old_value"; break;
        case restore_zero:      s = "restore_zero";      break;
        case insert_token:      s = "insert_token";      break;
        case level_boundary:    s = "level_boundary";    break;
        case saved_line:        s = "saved_line";        break;
        case saved_adjust:      s = "saved_adjust";      break;
        case saved_insert:      s = "saved_insert";      break;
        case saved_disc:        s = "saved_disc";        break;
        case saved_boxtype:     s = "saved_boxtype";     break;
        case saved_textdir:     s = "saved_textdir";     break;
        case saved_eqno:        s = "saved_eqno";        break;
        case saved_choices:     s = "saved_choices";     break;
        case saved_math:        s = "saved_math";        break;
        case saved_boxcontext:  s = "saved_boxcontext";  break;
        case saved_boxspec:     s = "saved_boxspec";     break;
        case saved_boxdir:      s = "saved_boxdir";      break;
        case saved_boxattr:     s = "saved_boxattr";     break;
        case saved_boxpack:     s = "saved_boxpack";     break;
        case saved_attrlist:    s = "saved_attrlist";    break;
        case saved_eqtb:        s = "saved_eqtb";        break;
        default: break;
    }
    return s;
}

void print_save_stack(void)
{
    int i;
    begin_diagnostic();
    selector = term_and_log;
    print_ln();
    for (i = (save_ptr - 1); i >= 0; i--) {
        tprint("save_stack[");
        if (i < 100)
            print_char(' ');
        if (i < 10)
            print_char(' ');
        print_int(i);
        tprint("]: ");
        tprint(save_stack_type(i));
        switch (save_type(i)) {
            case restore_old_value:
                tprint(", ");
                show_eqtb_meaning(save_value(i));
                tprint("=");
                if (save_value(i) >= int_base) {
                    print_int(save_word(i - 1).cint);
                } else {
                    print_int(eq_type_field(save_word(i - 1)));
                    print_char(',');
                    print_int(equiv_field(save_word(i - 1)));
                }
                i--;
                break;
            case restore_zero:
                tprint(", ");
                show_eqtb_meaning(save_value(i));
                break;
            case insert_token:
                tprint(", ");
                {
                    halfword p = get_avail();
                    set_token_info(p, save_value(i));
                    show_token_list(p, null, 1);
                    free_avail(p);
                }
                break;
            case level_boundary:
                tprint(", old group=");
                print_int(save_level(i));
                tprint(", boundary = ");
                print_int(save_value(i));
                tprint(", line = ");
                print_int(save_value(i - 1));
                i--;
                break;
            case saved_adjust:
                tprint(", ");
                /*tex vadjust vs vadjust pre */
                print_int(save_level(i));
                break;
            case saved_insert:
                tprint(", ");
                /*tex insert number */
                print_int(save_value(i));
                break;
            case saved_boxtype:
                /*tex \.{\\localleftbox} vs \.{\\localrightbox} */
                tprint(", ");
                print_int(save_value(i));
                break;
            case saved_eqno:
                /*tex \.{\\eqno} vs \.{\\leqno} */
                tprint(", ");
                print_int(save_value(i));
                break;
            case saved_disc:
            case saved_choices:
                tprint(", ");
                print_int(save_value(i));
                break;
            case saved_math:
                tprint(", listptr=");
                print_int(save_value(i));
                break;
            case saved_boxcontext:
                tprint(", ");
                print_int(save_value(i));
                break;
            case saved_boxspec:
                tprint(", spec=");
                print_int(save_level(i));
                tprint(", dimen=");
                print_int(save_value(i));
                break;
            case saved_textdir:
            case saved_boxdir:
                tprint(", ");
                print_dir_text(save_value(i));
                break;
            case saved_boxattr:
            case saved_boxpack:
            case saved_attrlist:
                tprint(", ");
                print_int(save_value(i));
                break;
            case saved_line:
            case saved_eqtb:
                break;
            default:
                break;
        }
        print_ln();
    }
    end_diagnostic(true);
}

/*tex

The \.{\\showgroups} command displays all currently active grouping levels.

The modifications of \TeX\ required for the display produced by the
|show_save_groups| procedure were first discussed by Donald~E. Knuth in {\sl
TUGboat\/} {\bf 11}, 165--170 and 499--511, 1990. @^Knuth, Donald Ervin@>

In order to understand a group type we also have to know its mode. Since
unrestricted horizontal modes are not associated with grouping, they are skipped
when traversing the semantic nest.

*/

void show_save_groups(void)
{
    int p = nest_ptr;
    int m;
    save_pointer v = save_ptr;
    quarterword l = cur_level;
    group_code c = cur_group;
    int a = 1; /* to keep track of alignments */
    int i;
    quarterword j;
    const char *s = NULL;
    save_ptr = cur_boundary;
    decr(cur_level);
    tprint_nl("");
    print_ln();
    while (1) {
        tprint_nl("### ");
        print_group(true);
        if (cur_group == bottom_level)
            goto DONE;
        do {
            m = nest[p].mode_field;
            if (p > 0)
                decr(p);
            else
                m = vmode;
        } while (m == hmode);
        tprint(" (");
        switch (cur_group) {
            case simple_group:
                incr(p);
                goto FOUND2;
                break;
            case hbox_group:
            case adjusted_hbox_group:
                s = "hbox";
                break;
            case vbox_group:
                s = "vbox";
                break;
            case vtop_group:
                s = "vtop";
                break;
            case align_group:
                if (a == 0) {
                    if (m == -vmode)
                        s = "halign";
                    else
                        s = "valign";
                    a = 1;
                    goto FOUND1;
                } else {
                    if (a == 1)
                        tprint("align entry");
                    else
                        tprint_esc("cr");
                    if (p >= a)
                        p = p - a;
                    a = 0;
                    goto FOUND;
                }
                break;
            case no_align_group:
                incr(p);
                a = -1;
                tprint_esc("noalign");
                goto FOUND2;
                break;
            case output_group:
                tprint_esc("output");
                goto FOUND;
                break;
            case math_group:
                goto FOUND2;
                break;
            case disc_group:
                tprint_esc("discretionary");
                for (i = 1; i < 3; i++) {
                    if (i <= saved_value(-2)) {
                        tprint("{}");
                    }
                }
                goto FOUND2;
                break;
            case math_choice_group:
                tprint_esc("mathchoice");
                for (i = 1; i < 4; i++) {
                    /*tex A different offset because |-2==saved_textdir|: */
                    if (i <= saved_value(-3)) {
                        tprint("{}");
                    }
                }
                goto FOUND2;
                break;
            case insert_group:
                if (saved_type(-1) == saved_adjust) {
                    tprint_esc("vadjust");
                    if (saved_level(-1) != 0)
                        tprint(" pre");
                } else {
                    tprint_esc("insert");
                    print_int(saved_value(-1));
                }
                goto FOUND2;
                break;
            case vcenter_group:
                s = "vcenter";
                goto FOUND1;
                break;
            case semi_simple_group:
                incr(p);
                tprint_esc("begingroup");
                goto FOUND;
                break;
            case math_shift_group:
                if (m == mmode) {
                    print_char('$');
                } else if (nest[p].mode_field == mmode) {
                    print_cmd_chr(eq_no_cmd, saved_value(-2));
                    goto FOUND;
                }
                print_char('$');
                goto FOUND;
                break;
            case math_left_group:
                if (subtype(nest[p + 1].eTeX_aux_field) == left_noad_side)
                    tprint_esc("left");
                else
                    tprint_esc("middle");
                goto FOUND;
                break;
            default:
                confusion("showgroups");
                break;
        }
        /*tex Show the box context */
        i = saved_value(-5);
        if (i != 0) {
            if (i < box_flag) {
                if (abs(nest[p].mode_field) == vmode)
                    j = hmove_cmd;
                else
                    j = vmove_cmd;
                if (i > 0)
                    print_cmd_chr(j, 0);
                else
                    print_cmd_chr(j, 1);
                print_scaled(abs(i));
                tprint("pt");
            } else if (i < ship_out_flag) {
                if (i >= global_box_flag) {
                    tprint_esc("global");
                    i = i - (global_box_flag - box_flag);
                }
                tprint_esc("setbox");
                print_int(i - box_flag);
                print_char('=');
            } else {
                print_cmd_chr(leader_ship_cmd, i - (leader_flag - a_leaders));
            }
        }
      FOUND1:
        tprint_esc(s);
        /*tex Show the box packaging info. The offsets may vary. */
        {
            int ii = -1;
            while (saved_type(ii) != saved_boxspec)
                ii--;
            if (saved_value(ii) != 0) {
                print_char(' ');
                if (saved_level(ii) == exactly)
                    tprint("to");
                else
                    tprint("spread");
                print_scaled(saved_value(ii));
                tprint("pt");
            }
        }
      FOUND2:
        print_char('{');
      FOUND:
        print_char(')');
        decr(cur_level);
        cur_group = save_level(save_ptr);
        save_ptr = save_value(save_ptr);
    }
  DONE:
    save_ptr = v;
    cur_level = l;
    cur_group = c;
}

/*tex

Just before an entry of |eqtb| is changed, the following procedure should be
called to update the other data structures properly. It is important to keep in
mind that reference counts in |mem| include references from within |save_stack|,
so these counts must be handled carefully. @^reference counts@>

We don't need to destroy when an assignment has the same node:

*/

void eq_destroy(memory_word w)
{
    halfword q;
    switch (eq_type_field(w)) {
        case call_cmd:
        case long_call_cmd:
        case outer_call_cmd:
        case long_outer_call_cmd:
            delete_token_ref(equiv_field(w));
            break;
        case glue_ref_cmd:
            flush_node(equiv_field(w));
            break;
        case shape_ref_cmd:
            q = equiv_field(w);
            if (q != null) {
                /*tex
                    We need to free a \.{\\parshape} block. Such a block is
                    |2n+1| words long, where |n=vinfo(q)|. It happens in the
                    flush function.
                */
                flush_node(q);
            }
            break;
        case box_ref_cmd:
            flush_node_list(equiv_field(w));
            break;
        default:
            break;
    }
}

/*tex

To save a value of |eqtb[p]| that was established at level |l|, we can use the
following subroutine.

*/
void eq_save(halfword p, quarterword l)
{
    check_full_save_stack();
    if (l == level_zero) {
        save_type(save_ptr) = restore_zero;
    } else {
        save_word(save_ptr) = eqtb[p];
        save_type(save_ptr) = saved_eqtb;
        incr(save_ptr);
        save_type(save_ptr) = restore_old_value;
    }
    save_level(save_ptr) = l;
    save_value(save_ptr) = p;
    incr(save_ptr);
}

/*tex

The procedure |eq_define| defines an |eqtb| entry having specified |eq_type| and
|equiv| fields, and saves the former value if appropriate. This procedure is used
only for entries in the first four regions of |eqtb|, i.e., only for entries that
have |eq_type| and |equiv| fields. After calling this routine, it is safe to put
four more entries on |save_stack|, provided that there was room for four more
entries before the call, since |eq_save| makes the necessary test.

*/

void eq_define(halfword p, quarterword t, halfword e)
{
    boolean trace = tracing_assigns_par > 0;
    if ((eq_type(p) == t) && (equiv(p) == e)) {
        if (trace)
            diagnostic_trace(p, "reassigning");
        eq_destroy(eqtb[p]);
        return;
    }
    if (trace)
        diagnostic_trace(p, "changing");
    if (eq_level(p) == cur_level)
        eq_destroy(eqtb[p]);
    else if (cur_level > level_one)
        eq_save(p, eq_level(p));
    set_eq_level(p, cur_level);
    set_eq_type(p, t);
    set_equiv(p, e);
    if (trace)
        diagnostic_trace(p, "into");
}

/*tex

The counterpart of |eq_define| for the remaining (fullword) positions in |eqtb|
is called |eq_word_define|. Since |xeq_level[p]>=level_one| for all |p|, a
`|restore_zero|' will never be used in this case.

*/

void eq_word_define(halfword p, int w)
{
    boolean trace = tracing_assigns_par > 0;
    if (eqtb[p].cint == w) {
        if (trace)
            diagnostic_trace(p, "reassigning");
        return;
    }
    if (trace)
        diagnostic_trace(p, "changing");
    if (xeq_level[p] != cur_level) {
        eq_save(p, xeq_level[p]);
        xeq_level[p] = cur_level;
    }
    eqtb[p].cint = w;
    if (trace)
        diagnostic_trace(p, "into");
}

/*tex

The |eq_define| and |eq_word_define| routines take care of local definitions.
@^global definitions@> Global definitions are done in almost the same way, but
there is no need to save old values, and the new value is associated with
|level_one|.

*/

void geq_define(halfword p, quarterword t, halfword e)
{
    boolean trace = tracing_assigns_par > 0;
    if (trace)
        diagnostic_trace(p, "globally changing");
    eq_destroy(eqtb[p]);
    set_eq_level(p, level_one);
    set_eq_type(p, t);
    set_equiv(p, e);
    if (trace)
        diagnostic_trace(p, "into");
}

void geq_word_define(halfword p, int w)
{
    boolean trace = tracing_assigns_par > 0;
    if (trace)
        diagnostic_trace(p, "globally changing");
    eqtb[p].cint = w;
    xeq_level[p] = level_one;
    if (trace)
        diagnostic_trace(p, "into");
}

/*tex

Subroutine |save_for_after| puts a token on the stack for save-keeping.

*/

void save_for_after(halfword t)
{
    if (cur_level > level_one) {
        check_full_save_stack();
        save_type(save_ptr) = insert_token;
        save_level(save_ptr) = level_zero;
        save_value(save_ptr) = t;
        incr(save_ptr);
    }
}

/*tex

The |unsave| routine goes the other way, taking items off of |save_stack|. This
routine takes care of restoration when a level ends; everything belonging to the
topmost group is cleared off of the save stack.

*/

void unsave(void)
{
    halfword p;
    quarterword l = level_one;
    /*tex Variable |a| registers if we already have processed an \.{\\aftergroup}. */
    boolean a = false;
    unsave_math_codes(cur_level);
    unsave_cat_codes(cat_code_table_par, cur_level);
    unsave_text_codes(cur_level);
    unsave_math_data(cur_level);
    if (cur_level > level_one) {
        boolean trace = tracing_restores_par > 0;
        decr(cur_level);
        /*tex Clear off top level from |save_stack|. */
        while (true) {
            decr(save_ptr);
            if (save_type(save_ptr) == level_boundary)
                break;
            p = save_value(save_ptr);
            if (save_type(save_ptr) == insert_token) {
                reinsert_token(a, p);
                /*tex always |true| as we are always in \ETEX\ now. */
                a = true;
            } else {
                if (save_type(save_ptr) == restore_old_value) {
                    l = save_level(save_ptr);
                    decr(save_ptr);
                } else {
                    save_word(save_ptr) = eqtb[undefined_control_sequence];
                }
                /*tex
                    Store |save_stack[save_ptr]| in |eqtb[p]|, unless |eqtb[p]|
                    holds a global value A global definition, which sets the
                    level to |level_one|, will not be undone by |unsave|. If at
                    least one global definition of |eqtb[p]| has been carried out
                    within the group that just ended, the last such definition
                    will therefore survive.
                */
                if (p < int_base || p > eqtb_size) {
                    if (eq_level(p) == level_one) {
                        /*tex Destroy the saved value: */
                        eq_destroy(save_word(save_ptr));
                        if (trace)
                            diagnostic_trace(p, "retaining");
                    } else {
                        /*tex Destroy the current value: */
                        eq_destroy(eqtb[p]);
                        /*tex Restore the saved value: */
                        eqtb[p] = save_word(save_ptr);
                        if (trace)
                            diagnostic_trace(p, "restoring");
                    }
                } else if (xeq_level[p] != level_one) {
                    eqtb[p] = save_word(save_ptr);
                    xeq_level[p] = l;
                    if (trace)
                        diagnostic_trace(p, "restoring");
                } else {
                    if (trace)
                        diagnostic_trace(p, "retaining");
                }
            }
        }

        if (tracing_groups_par > 0) {
            group_trace(true);
        }
        if (grp_stack[in_open] == cur_boundary) {
            /*tex Groups are possibly not properly nested with files. */
            group_warning();
        }
        cur_group = save_level(save_ptr);
        cur_boundary = save_value(save_ptr);
        decr(save_ptr);
    } else {
        /*tex |unsave| is not used when |cur_group=bottom_level| */
        confusion("curlevel");
    }
    attr_list_cache = cache_disabled;
}

/*tex

Most of the parameters kept in |eqtb| can be changed freely, but there's an
exception: The magnification should not be used with two different values during
any \TeX\ job, since a single magnification is applied to an entire run. The
global variable |mag_set| is set to the current magnification whenever it becomes
necessary to ``freeze'' it at a particular value.

The |prepare_mag| subroutine is called whenever \TeX\ wants to use |mag| for
magnification. If nonzero, this magnification should be used henceforth. We might
drop magnifaction at some point.

*/

int mag_set = 0;

void prepare_mag(void)
{
    if ((mag_set > 0) && (mag_par != mag_set)) {
        print_err("Incompatible magnification (");
        print_int(mag_par);
        tprint(");");
        tprint_nl(" the previous value will be retained");
        help2(
            "I can handle only one magnification ratio per job. So I've",
            "reverted to the magnification you used earlier on this run."
        );
        int_error(mag_set);
        geq_word_define(int_base + mag_code, mag_set);  /* |mag:=mag_set| */
    }
    if ((mag_par <= 0) || (mag_par > 32768)) {
        print_err("Illegal magnification has been changed to 1000");
        help1(
            "The magnification ratio must be between 1 and 32768."
        );
        int_error(mag_par);
        geq_word_define(int_base + mag_code, 1000);
    }
    if ((mag_set == 0) && (mag_par != mag_set)) {
        if (mag_par != 1000)
            one_true_inch = xn_over_d(one_hundred_inch, 10, mag_par);
        else
            one_true_inch = one_inch;
    }
    mag_set = mag_par;
}

/*tex

Let's pause a moment now and try to look at the Big Picture. The \TeX\ program
consists of three main parts: syntactic routines, semantic routines, and output
routines. The chief purpose of the syntactic routines is to deliver the user's
input to the semantic routines, one token at a time. The semantic routines act as
an interpreter responding to these tokens, which may be regarded as commands. And
the output routines are periodically called on to convert box-and-glue lists into
a compact set of instructions that will be sent to a typesetter. We have
discussed the basic data structures and utility routines of \TeX, so we are good
and ready to plunge into the real activity by considering the syntactic routines.

Our current goal is to come to grips with the |get_next| procedure, which is the
keystone of \TeX's input mechanism. Each call of |get_next| sets the value of
three variables |cur_cmd|, |cur_chr|, and |cur_cs|, representing the next input
token.

$$
    \vbox{\halign{#\hfil\cr
        \hbox{|cur_cmd| denotes a command code from the long list of codes given above;}\cr
        \hbox{|cur_chr| denotes a character code or other modifier of the command code;}\cr
        \hbox{|cur_cs| is the |eqtb| location of the current control sequence,}\cr
        \hbox{\qquad if the current token was a control sequence, otherwise it's zero.}\cr}}
$$

Underlying this external behavior of |get_next| is all the machinery necessary to
convert from character files to tokens. At a given time we may be only partially
finished with the reading of several files (for which \.{\\input} was specified),
and partially finished with the expansion of some user-defined macros and/or some
macro parameters, and partially finished with the generation of some text in a
template for \.{\\halign}, and so on. When reading a character file, special
characters must be classified as math delimiters, etc.; comments and extra blank
spaces must be removed, paragraphs must be recognized, and control sequences must
be found in the hash table. Furthermore there are occasions in which the scanning
routines have looked ahead for a word like `\.{plus}' but only part of that word
was found, hence a few characters must be put back into the input and scanned
again.

To handle these situations, which might all be present simultaneously, \TeX\ uses
various stacks that hold information about the incomplete activities, and there
is a finite state control for each level of the input mechanism. These stacks
record the current state of an implicitly recursive process, but the |get_next|
procedure is not recursive. Therefore it will not be difficult to translate these
algorithms into low-level languages that do not support recursion.

In general, |cur_cmd| is the current command as set by |get_next|, while
|cur_chr| is the operand of the current command. The control sequence found here
is registsred in |cur_cs| and is zero if none found. The |cur_tok| variable
contains the packed representative of |cur_cmd| and |cur_chr| and like the other
ones is global.

*/

int cur_cmd;
halfword cur_chr;
halfword cur_cs;
halfword cur_tok;

/*tex

Here is a procedure that displays the current command. The variable |n| holds the
level of \.{\\if...\\fi} nesting and |l| the line where \.{\\if} started.

*/

void show_cur_cmd_chr(void)
{
    int n, l;
    halfword p;
    begin_diagnostic();
    tprint_nl("{");
    if (mode_par != shown_mode) {
        print_mode(mode_par);
        tprint(": ");
        shown_mode = mode_par;
    }
    print_cmd_chr((quarterword) cur_cmd, cur_chr);
    if (tracing_ifs_par > 0) {
        if (cur_cmd >= if_test_cmd) {
            if (cur_cmd <= fi_or_else_cmd) {
                tprint(": ");
                if (cur_cmd == fi_or_else_cmd) {
                    print_cmd_chr(if_test_cmd, cur_if);
                    print_char(' ');
                    n = 0;
                    l = if_line;
                } else {
                    n = 1;
                    l = line;
                }
                p = cond_ptr;
                while (p != null) {
                    incr(n);
                    p = vlink(p);
                }
                tprint("(level ");
                print_int(n);
                print_char(')');
                print_if_line(l);
            }
        }
    }
    print_char('}');
    end_diagnostic(false);
}

/*tex

Here is a procedure that displays the contents of |eqtb[n]| symbolically.

*/

void show_eqtb(halfword n)
{
    if (n < null_cs) {
        /*tex
            This can't happen in a pure \TEX\ run, but careless usage of tokens
            at the \LUA\ end can make you end up here.
        */
        tprint("? bad token, case 1: ");
        print_int(n);
    } else if ((n < glue_base) || ((n > eqtb_size) && (n <= eqtb_top))) {
        /*tex
            This routine show the current meaning of |eqtb| entry |n| in region 1 or 2.
            Similar routines for the other regions will appear below.
        */
        sprint_cs(n);
        print_char('=');
        print_cmd_chr(eq_type(n), equiv(n));
        if (eq_type(n) >= call_cmd) {
            print_char(':');
            show_token_list(token_link(equiv(n)), null, 32);
        }
    } else if (n < local_base) {
        /*tex
            Here we show equivalent |n| in region 3. All glue parameters and registers
            are initially `\.{0pt plus0pt minus0pt}'.
        */
        if (n < skip_base) {
            if (n < glue_base + thin_mu_skip_code)
                print_cmd_chr(assign_glue_cmd, n);
            else
                print_cmd_chr(assign_mu_glue_cmd, n);
            print_char('=');
            if (n < glue_base + thin_mu_skip_code)
                print_spec(equiv(n), "pt");
            else
                print_spec(equiv(n), "mu");
        } else if (n < mu_skip_base) {
            tprint_esc("skip");
            print_int(n - skip_base);
            print_char('=');
            print_spec(equiv(n), "pt");
        } else {
            tprint_esc("muskip");
            print_int(n - mu_skip_base);
            print_char('=');
            print_spec(equiv(n), "mu");
        }

    } else if (n < int_base) {
        /*tex
            We're now at equivalent |n| in region 4. First we initialize most
            things to null or undefined values. An undefined font is represented
            by the internal code |font_base|.

            However, the character code tables are given initial values based on
            the conventional interpretation of ASCII code. These initial values
            should not be changed when \TeX\ is adapted for use with non-English
            languages; all changes to the initialization conventions should be
            made in format packages, not in \TeX\ itself, so that global
            interchange of formats is possible.
        */
        if ((n == par_shape_loc) || ((n >= etex_pen_base) && (n < etex_pens))) {
            if (n == par_shape_loc)
                print_cmd_chr(set_tex_shape_cmd, n);
            else
                print_cmd_chr(set_etex_shape_cmd, n);
            print_char('=');
            if (equiv(n) == null) {
                print_char('0');
            } else if (n > par_shape_loc) {
                print_int(penalty(equiv(n)));
                print_char(' ');
                print_int(penalty(equiv(n) + 1));
                if (penalty(equiv(n)) > 1)
                    tprint_esc("ETC.");
            } else {
                print_int(vinfo(par_shape_par_ptr + 1));
            }
        } else if (n < toks_base) {
            print_cmd_chr(assign_toks_cmd, n);
            print_char('=');
            if (equiv(n) != null)
                show_token_list(token_link(equiv(n)), null, 32);
        } else if (n < box_base) {
            tprint_esc("toks");
            print_int(n - toks_base);
            print_char('=');
            if (equiv(n) != null)
                show_token_list(token_link(equiv(n)), null, 32);
        } else if (n < cur_font_loc) {
            tprint_esc("box");
            print_int(n - box_base);
            print_char('=');
            if (equiv(n) == null) {
                tprint("void");
            } else {
                depth_threshold = 0;
                breadth_max = 1;
                show_node_list(equiv(n));
            }
        } else if (n == cur_font_loc) {
            /*tex
                Let's show the font identifier in |eqtb[n]|, that's
                |font_id_text(equiv(n))|
            */
            tprint("current font");
            print_char('=');
            print_esc(hash[font_id_base + equiv(n)].rh);
        }
    } else if (n < dimen_base) {
        /*tex Show equivalent |n| in region 5: */
        if (n < dir_base) {
            print_cmd_chr(assign_int_cmd, n);
            print_char('=');
            print_int(eqtb[n].cint);
        } else if (n < count_base) {
            print_cmd_chr(assign_dir_cmd, n);
            print_char(' ');
            print_dir_par(eqtb[n].cint);
        } else if (n < attribute_base) {
            tprint_esc("count");
            print_int(n - count_base);
            print_char('=');
            print_int(eqtb[n].cint);
        } else if (n < del_code_base) {
            tprint_esc("attribute");
            print_int(n - attribute_base);
            print_char('=');
            print_int(eqtb[n].cint);
        }
    } else if (n <= eqtb_size) {
        /*tex Show equivalent |n| in region 6: */
        if (n < scaled_base) {
            print_cmd_chr(assign_dimen_cmd, n);
        } else {
            tprint_esc("dimen");
            print_int(n - scaled_base);
        }
        print_char('=');
        print_scaled(eqtb[n].cint);
        tprint("pt");
    } else {
        /*tex This can't happen unless you messed up at the \LUA\ end. */
        tprint("? bad token, case 2: ");
        print_int(n);
    }
}

void show_eqtb_meaning(halfword n)
{
    if (n < null_cs) {
        /*tex This can't happen. */
        print_char('?');
    } else if ((n < glue_base) || ((n > eqtb_size) && (n <= eqtb_top))) {
        /*tex
            Here is a routine that displays the current meaning of an |eqtb|
            entry in region 1 or~2. Similar routines for the other regions will
            appear below.
        */
        sprint_cs(n);
    } else if (n < local_base) {
        /*
            Show equivalent |n| in region 3. All glue parameters and registers
            are initially `\.{0pt plus0pt minus0pt}'.
        */
        if (n < skip_base) {
            if (n < glue_base + thin_mu_skip_code)
                print_cmd_chr(assign_glue_cmd, n);
            else
                print_cmd_chr(assign_mu_glue_cmd, n);
        } else if (n < mu_skip_base) {
            tprint_esc("skip");
            print_int(n - skip_base);
        } else {
            tprint_esc("muskip");
            print_int(n - mu_skip_base);
        }
    } else if (n < int_base) {
        /*tex
            Show equivalent |n| in region 4. We initialize most things to null or
            undefined values. An undefined font is represented by the internal
            code |font_base|. However, the character code tables are given
            initial values based on the conventional interpretation of ASCII
            code. These initial values should not be changed when \TeX\ is
            adapted for use with non-English languages; all changes to the
            initialization conventions should be made in format packages, not in
            \TeX\ itself, so that global interchange of formats is possible.
        */
        if ((n == par_shape_loc) || ((n >= etex_pen_base) && (n < etex_pens))) {
            if (n == par_shape_loc)
                print_cmd_chr(set_tex_shape_cmd, n);
            else
                print_cmd_chr(set_etex_shape_cmd, n);
        } else if (n < toks_base) {
            print_cmd_chr(assign_toks_cmd, n);
        } else if (n < box_base) {
            tprint_esc("toks");
            print_int(n - toks_base);
        } else if (n < cur_font_loc) {
            tprint_esc("box");
            print_int(n - box_base);
        } else if (n == cur_font_loc) {
            /*tex Show the font identifier in |eqtb[n]|. */
            tprint("current font");
        }
    } else if (n < dimen_base) {
        /*tex Show equivalent |n| in region 5. */
        if (n < dir_base) {
            print_cmd_chr(assign_int_cmd, n);
        } else if (n < count_base) {
            print_cmd_chr(assign_dir_cmd, n);
        } else if (n < attribute_base) {
            tprint_esc("count");
            print_int(n - count_base);
        } else if (n < del_code_base) {
            tprint_esc("attribute");
            print_int(n - attribute_base);
        }
    } else if (n <= eqtb_size) {
        /*tex Show equivalent |n| in region 6. */
        if (n < scaled_base) {
            print_cmd_chr(assign_dimen_cmd, n);
        } else {
            tprint_esc("dimen");
            print_int(n - scaled_base);
        }
    } else {
        /*tex This can't happen either. */
        print_char('?');
    }
}
