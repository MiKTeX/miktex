/*

maincontrol.w

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
#include "lua/luatex-api.h"

#define mode     mode_par
#define tail     tail_par
#define head     head_par
#define dir_save dirs_par

/*tex

We come now to the |main_control| routine, which contains the master switch that
causes all the various pieces of \TeX\ to do their things, in the right order.

In a sense, this is the grand climax of the program: It applies all the tools
that we have worked so hard to construct. In another sense, this is the messiest
part of the program: It necessarily refers to other pieces of code all over the
place, so that a person can't fully understand what is going on without paging
back and forth to be reminded of conventions that are defined elsewhere. We are
now at the hub of the web, the central nervous system that touches most of the
other parts and ties them together. @^brain@>

The structure of |main_control| itself is quite simple. There's a label called
|big_switch|, at which point the next token of input is fetched using
|get_x_token|. Then the program branches at high speed into one of about 100
possible directions, based on the value of the current mode and the newly fetched
command code; the sum |abs(mode)+cur_cmd| indicates what to do next. For example,
the case `|vmode+letter|' arises when a letter occurs in vertical mode (or
internal vertical mode); this case leads to instructions that initialize a new
paragraph and enter horizontal mode.

The big |case| statement that contains this multiway switch has been labeled
|reswitch|, so that the program can |goto reswitch| when the next token has
already been fetched. Most of the cases are quite short; they call an ``action
procedure'' that does the work for that case, and then they either |goto
reswitch| or they ``fall through'' to the end of the |case| statement, which
returns control back to |big_switch|. Thus, |main_control| is not an extremely
large procedure, in spite of the multiplicity of things it must do; it is small
enough to be handled by PASCAL compilers that put severe restrictions on
procedure size. @!@^action procedure@>

One case is singled out for special treatment, because it accounts for most of
\TeX's activities in typical applications. The process of reading simple text and
converting it into |char_node| records, while looking for ligatures and kerns, is
part of \TeX's ``inner loop''; the whole program runs efficiently when its inner
loop is fast, so this part has been written with particular care.

We leave the |space_factor| unchanged if |sf_code(cur_chr)=0|; otherwise we set
it equal to |sf_code(cur_chr)|, except that it should never change from a value
less than 1000 to a value exceeding 1000. The most common case is
|sf_code(cur_chr)=1000|, so we want that case to be fast.

*/

void adjust_space_factor(void)
{
    halfword s = get_sf_code(cur_chr);
    if (s == 1000) {
        space_factor_par = 1000;
    } else if (s < 1000) {
        if (s > 0)
            space_factor_par = s;
    } else if (space_factor_par < 1000) {
        space_factor_par = 1000;
    } else {
        space_factor_par = s;
    }
}

/*tex

To handle the execution state of |main_control|'s eternal loop, an extra global
variable is used, along with a macro to define its values.

*/

#define goto_next 0
#define goto_skip_token 1
#define goto_return 2

static int main_control_state;
static int local_level = 0;

/*tex

@* Main control helpers.

Here are all the functions that are called from |main_control| that
are not already defined elsewhere. For the moment, this list simply
in the order that the appear in |init_main_control|, below.

*/

static void run_char_num (void) {
    scan_char_num();
    cur_chr = cur_val;
    adjust_space_factor();
    tail_append(new_char(cur_font_par, cur_chr));
}

static void run_char (void) {
    adjust_space_factor();
    tail_append(new_char(cur_font_par, cur_chr));
}

static void run_node (void) {
    halfword n = cur_chr;
    if (copy_lua_input_nodes_par) {
        n = copy_node_list(n);
    }
    tail_append(n);
    if (nodetype_has_attributes(type(n)) && node_attr(n) == null) {
        build_attribute_list(n);
    }
    while (vlink(n) != null) {
        n = vlink(n);
        tail_append(n);
        if (nodetype_has_attributes(type(n)) && node_attr(n) == null) {
            build_attribute_list(n);
        }
    }
}

static void run_lua_call(void) {
    if (cur_chr <= 0) {
        normal_error("luacall", "invalid number");
    } else {
        str_number u = save_cur_string();
        luacstrings = 0;
        luafunctioncall(cur_chr);
        restore_cur_string(u);
        if (luacstrings > 0)
            lua_string_start();
    }
}

/*tex

The occurrence of blank spaces is almost part of \TeX's inner loop, since we
usually encounter about one space for every five non-blank characters. Therefore
|main_control| gives second-highest priority to ordinary spaces.

When a glue parameter like \.{\\spaceskip} is set to `\.{0pt}', we will see to it
later that the corresponding glue specification is precisely |zero_glue|, not
merely a pointer to some specification that happens to be full of zeroes.
Therefore it is simple to test whether a glue parameter is zero or~not.

*/

static void run_app_space (void) {
    halfword p; /* was a global temp_ptr */
    int method = disable_space_par ;
    if (method == 1) {
        /*tex Don't inject anything, not even zero skip. */
    } else if (method == 2) {
        p = new_glue(zero_glue);
        couple_nodes(tail,p);
        tail = p;
    } else if ((abs(mode) + cur_cmd == hmode + spacer_cmd) && (!(space_factor_par == 1000))) {
        app_space();
    } else {
        /*tex Append a normal inter-word space to the current list. */
        if (glue_is_zero(space_skip_par)) {
            /*tex Find the glue specification for text spaces in the current font. */
            p = new_glue(zero_glue);
            width(p) = space(cur_font_par);
            stretch(p) = space_stretch(cur_font_par);
            shrink(p) = space_shrink(cur_font_par);

        } else {
            p = new_param_glue(space_skip_code);
        }
        /*tex So from now we have a subtype with spaces: */
        subtype(p) = space_skip_code + 1 ;
        couple_nodes(tail,p);
        tail = p;
    }
}

/*tex

Append a |boundary_node|

*/

static void run_boundary (void) {
    halfword n ;
    n = new_node(boundary_node,cur_chr);
    if ((cur_chr == 1) || (cur_chr == 2) ) {
        /*tex We expect a user boundary or protrusion boundary. */
        scan_int();
        boundary_value(n) = cur_val;
    }
    couple_nodes(tail, n);
    tail = n;
}

static void run_char_ghost (void) {
    int t;
    t = cur_chr;
    get_x_token();
    if ((cur_cmd == letter_cmd) || (cur_cmd == other_char_cmd)
        || (cur_cmd == char_given_cmd) || (cur_cmd == char_num_cmd)) {
        halfword p = new_glyph(get_cur_font(), cur_chr);
        if (t == 0) {
            set_is_leftghost(p);
        } else {
            set_is_rightghost(p);
        }
        tail_append(p);
    }
}

static void run_relax (void) {
    return;
}

/*tex

|ignore_spaces| is a special case: after it has acted, |get_x_token| has already
fetched the next token from the input, so that operation in |main_control| should
be skipped.

*/

static void run_ignore_spaces (void) {
    if (cur_chr == 0) {
        /*tex Get the next non-blank non-call... */
        do {
            get_x_token();
        } while (cur_cmd == spacer_cmd);
        main_control_state = goto_skip_token;
    } else {
        int t = scanner_status;
        scanner_status = normal;
        get_next();
        scanner_status = t;
        cur_cs = prim_lookup(cs_text(cur_cs));
        if (cur_cs != undefined_primitive) {
            cur_cmd = get_prim_eq_type(cur_cs);
            cur_chr = get_prim_equiv(cur_cs);
            cur_tok = (cur_cmd * STRING_OFFSET) + cur_chr;
            main_control_state = goto_skip_token;
        }
    }
}

/*tex

|stop| is the second special case. We want |main_control| to return to its caller
if there is nothing left to do.

*/

static void run_stop (void) {
    if (its_all_over()) {
        /*tex this is the only way out */
        main_control_state= goto_return;
    }
}

static void run_non_math_math (void) {
    back_input();
    new_graf(true);
}

/*tex

    We build up an argument to |set_math_char|:

*/

static void run_math_char_num (void) {
    mathcodeval mval;
    if (cur_chr == 0)
        mval = scan_mathchar(tex_mathcode);
    else if (cur_chr == 1)
        mval = scan_mathchar(umath_mathcode);
    else
        mval = scan_mathchar(umathnum_mathcode);
    math_char_in_text(mval);
}

/*tex

    We build up an argument to |set_math_char|:
*/

static void run_math_given (void) {
    mathcodeval mval;
    mval = mathchar_from_integer(cur_chr, tex_mathcode);
    math_char_in_text(mval);
}

/*tex

    We build up an argument to |set_math_char| the \LUATEX\ way:
*/

static void run_xmath_given (void) {
    mathcodeval mval;
    mval = mathchar_from_integer(cur_chr, umath_mathcode);
    math_char_in_text(mval);
}

/*tex

The most important parts of |main_control| are concerned with \TeX's chief
mission of box-making. We need to control the activities that put entries on
vlists and hlists, as well as the activities that convert those lists into boxes.
All of the necessary machinery has already been developed; it remains for us to
``push the buttons'' at the right times.

As an introduction to these routines, let's consider one of the simplest cases:
What happens when `\.{\\hrule}' occurs in vertical mode, or `\.{\\vrule}' in
horizontal mode or math mode? The code in |main_control| is short, since the
|scan_rule_spec| routine already does most of what is required; thus, there is no
need for a special action procedure.

Note that baselineskip calculations are disabled after a rule in vertical mode,
by setting |prev_depth:=ignore_depth|.

*/

static void run_rule (void) {
    tail_append(scan_rule_spec());
    if (abs(mode) == vmode)
        prev_depth_par = ignore_depth;
    else if (abs(mode) == hmode)
        space_factor_par = 1000;
}

/*tex

Many of the actions related to box-making are triggered by the appearance of
braces in the input. For example, when the user says `\.{\\hbox} \.{to}
\.{100pt\{$\langle\,\hbox{hlist}\,\rangle$\}}' in vertical mode, the information
about the box size (100pt, |exactly|) is put onto |save_stack| with a level
boundary word just above it, and |cur_group:=adjusted_hbox_group|; \TeX\ enters
restricted horizontal mode to process the hlist. The right brace eventually
causes |save_stack| to be restored to its former state, at which time the
information about the box size (100pt, |exactly|) is available once again; a box
is packaged and we leave restricted horizontal mode, appending the new box to the
current list of the enclosing mode (in this case to the current list of vertical
mode), followed by any vertical adjustments that were removed from the box by
|hpack|.

The next few sections of the program are therefore concerned with the treatment
of left and right curly braces.

If a left brace occurs in the middle of a page or paragraph, it simply introduces
a new level of grouping, and the matching right brace will not have such a
drastic effect. Such grouping affects neither the mode nor the current list.

*/

static void run_left_brace (void) {
    new_save_level(simple_group);
    eq_word_define(int_base + no_local_whatsits_code, 0);
    eq_word_define(int_base + no_local_dirs_code, 0);
}

static void run_begin_group (void) {
    new_save_level(semi_simple_group);
    eq_word_define(int_base + no_local_whatsits_code, 0);
    eq_word_define(int_base + no_local_dirs_code, 0);
}

static void run_end_group (void) {
    if (cur_group == semi_simple_group) {
        fixup_directions();
    } else {
        off_save();
    }
}

/*tex

Constructions that require a box are started by calling |scan_box| with a
specified context code. The |scan_box| routine verifies that a |make_box| command
comes next and then it calls |begin_box|.

*/

static void run_move (void) {
    int t = cur_chr;
    scan_normal_dimen();
    if (t == 0)
        scan_box(cur_val);
    else
        scan_box(-cur_val);
}

static void run_leader_ship (void) {
    scan_box(leader_flag - a_leaders + cur_chr);
}

static void run_make_box (void) {
    begin_box(0);
}

static void run_box_dir (void) {
    scan_register_num();
    cur_box = box(cur_val);
    scan_optional_equals();
    scan_direction();
    if (cur_box != null)
        box_dir(cur_box) = cur_val;
}

static void run_box_direction (void) {
    scan_register_num();
    cur_box = box(cur_val);
    scan_optional_equals();
    scan_int();
    check_dir_value(cur_val);
    if (cur_box != null)
        box_dir(cur_box) = cur_val;
}

/*tex

There is a really small patch to add a new primitive called \.{\\quitvmode}. In
vertical modes, it is identical to \.{\\indent}, but in horizontal and math modes
it is really a no-op (as opposed to \.{\\indent}, which executes the
|indent_in_hmode| procedure).

A paragraph begins when horizontal-mode material occurs in vertical mode, or when
the paragraph is explicitly started by `\.{\\quitvmode}', `\.{\\indent}' or
`\.{\\noindent}'.

*/

static void run_start_par_vmode (void) {
    new_graf((cur_chr > 0));
}

static void run_start_par (void) {
   if (cur_chr != 2)
       indent_in_hmode();
}

static void run_new_graf (void) {
   back_input();
   new_graf(true);
}

/*tex

A paragraph ends when a |par_end| command is sensed, or when we are in horizontal
mode when reaching the right brace of vertical-mode routines like \.{\\vbox},
\.{\\insert}, or \.{\\output}.

*/

static void run_par_end_vmode (void) {
    normal_paragraph();
    if (mode > 0) {
        checked_page_filter(vmode_par);
        build_page();
    }
}

static void run_par_end_hmode (void) {
    if (align_state < 0) {
        /*tex This tries to recover from an alignment that didn't end properly. */
        off_save();
    }
    /* This takes us to the enclosing mode, if |mode>0|. */
    end_graf(bottom_level);
    if (mode == vmode) {
        checked_page_filter(hmode_par);
        build_page();
    }
}

static void append_italic_correction_mmode (void) {
    tail_append(new_kern(0));
}

static void run_local_box (void) {
    append_local_box(cur_chr);
}

static void run_halign_mmode (void) {
    if (privileged()) {
        if (cur_group == math_shift_group)
            init_align();
        else
            off_save();
    }
}

static void run_eq_no (void) {
    if (privileged()) {
        if (cur_group == math_shift_group)
            start_eq_no();
        else
            off_save();
    }
}

static void run_letter_mmode (void) {
   set_math_char(get_math_code(cur_chr));
}

static void run_char_num_mmode (void) {
    scan_char_num();
    cur_chr = cur_val;
    set_math_char(get_math_code(cur_chr));
}

static void run_math_char_num_mmode (void) {
    mathcodeval mval;
    if (cur_chr == 0)
        mval = scan_mathchar(tex_mathcode);
    else if (cur_chr == 1)
        mval = scan_mathchar(umath_mathcode);
    else
        mval = scan_mathchar(umathnum_mathcode);
    set_math_char(mval);
}

static void run_math_given_mmode (void) {
    mathcodeval mval;
    mval = mathchar_from_integer(cur_chr, tex_mathcode);
    set_math_char(mval);
}

static void run_xmath_given_mmode (void) {
    mathcodeval mval;
    mval = mathchar_from_integer(cur_chr, umath_mathcode);
    set_math_char(mval);
}

static void run_delim_num (void) {
    mathcodeval mval;
    if (cur_chr == 0)
        mval = scan_delimiter_as_mathchar(tex_mathcode);
    else
        mval = scan_delimiter_as_mathchar(umath_mathcode);
    set_math_char(mval);
}

static void run_vcenter (void) {
    scan_spec(vcenter_group);
    normal_paragraph();
    push_nest();
    mode = -vmode;
    prev_depth_par = ignore_depth;
    if (every_vbox_par != null)
        begin_token_list(every_vbox_par, every_vbox_text);
}

static void run_math_style (void) {
    tail_append(new_style((small_number) cur_chr));
}

static void run_non_script (void) {
    tail_append(new_glue(zero_glue));
    subtype(tail) = cond_math_glue;
}

static void run_math_choice (void) {
    if (cur_chr == 0)
        append_choices();
    else
        setup_math_style();
}

static void run_math_shift (void) {
    if (cur_group == math_shift_group)
        after_math();
    else
        off_save();
}

static void run_after_assignment (void) {
    get_token();
    after_token = cur_tok;
}

static void run_after_group (void) {
    get_token();
    save_for_after(cur_tok);
}

static void run_extension (void) {
    do_extension(0);
}

static void run_normal (void) {
{
    switch (cur_chr) {
        case save_pos_code:
            new_whatsit(save_pos_node);
            break;
        case save_cat_code_table_code:
            scan_int();
            if ((cur_val < 0) || (cur_val > 0x7FFF)) {
                print_err("Invalid \\catcode table");
                help1(
                    "All \\catcode table ids must be between 0 and 0x7FFF"
                );
                error();
            } else {
                if (cur_val == cat_code_table_par) {
                    print_err("Invalid \\catcode table");
                    help1(
                        "You cannot overwrite the current \\catcode table"
                    );
                    error();
                } else {
                    copy_cat_codes(cat_code_table_par, cur_val);
                }
            }
            break;
        case init_cat_code_table_code:
            scan_int();
            if ((cur_val < 0) || (cur_val > 0x7FFF)) {
                print_err("Invalid \\catcode table");
                help1(
                    "All \\catcode table ids must be between 0 and 0x7FFF"
                );
                error();
            } else {
                if (cur_val == cat_code_table_par) {
                    print_err("Invalid \\catcode table");
                    help1(
                        "You cannot overwrite the current \\catcode table"
                    );
                    error();
                } else {
                    initex_cat_codes(cur_val);
                }
            }
            break;
        case set_random_seed_code:
            /*  Negative random seed values are silently converted to positive ones */
            scan_int();
            if (cur_val < 0)
                negate(cur_val);
            random_seed = cur_val;
            init_randoms(random_seed);
            break;
        case late_lua_code:
            new_whatsit(late_lua_node); /* type == normal */
            late_lua_name(tail) = scan_lua_state();
            (void) scan_toks(false, false);
            late_lua_data(tail) = def_ref;
            break;
        case late_lua_call_code:
            new_whatsit(late_lua_node);
            late_lua_type(tail) = lua_refid_call;
            scan_int();
            late_lua_data(tail) = cur_val;
            break;
        case expand_font_code:
            read_expand_font();
            break;
        default:
            confusion("int1");
            break;
        }
    }
}

/*tex

This is experimental and not used for production, only for testing and writing
macros (some options stay).

*/

#define mathoption_set_int(A) \
    scan_int(); \
    word_define(mathoption_int_base+A, cur_val);

static void run_option(void) {
    int a = 0 ;
    switch (cur_chr) {
        case math_option_code:
            if (scan_keyword("old")) {
                mathoption_set_int(c_mathoption_old_code);
            /*
            } else if (scan_keyword("umathcodemeaning")) {
                mathoption_set_int(c_mathoption_umathcode_meaning_code);
            */
            } else {
                normal_warning("mathoption","unknown key");
            }
            break;
        default:
            /* harmless */
            break;
    }
}

static void lua_function_call(void) {
    scan_int();
    if (cur_val <= 0) {
        normal_error("luafunctioncall", "invalid number");
    } else {
        str_number u = save_cur_string();
        luacstrings = 0;
        luafunctioncall(cur_val);
        restore_cur_string(u);
        if (luacstrings > 0)
            lua_string_start();
    }
}

static void lua_bytecode_call(void) {
    scan_int();
    if (cur_val < 0 || cur_val > 65535) {
        normal_error("luabytecodecall", "invalid number");
    } else {
        str_number u = save_cur_string();
        luacstrings = 0;
        luabytecodecall(cur_val);
        restore_cur_string(u);
        if (luacstrings > 0)
            lua_string_start();
    }
}

/*tex

For mode-independent commands, the following macro is useful.

Also, there is a list of cases where the user has probably gotten into or out of
math mode by mistake. \TeX\ will insert a dollar sign and rescan the current
token, and it makes sense ot have a macro for that as well.

*/

#define any_mode(A,B) jump_table[vmode+(A)]=B; jump_table[hmode+(A)]=B; jump_table[mmode+(A)]=B
#define non_math(A,B) jump_table[vmode+(A)]=B; jump_table[hmode+(A)]=B;

/*tex

The |main_control| uses a jump table, and |init_main_control| sets that table up.

*/

typedef void (*main_control_function) (void);

main_control_function *jump_table;

static void init_main_control (void) {
    jump_table = xmalloc((mmode+max_command_cmd+1) * sizeof(main_control_function)) ;

    jump_table[hmode + char_num_cmd] = run_char_num;
    jump_table[hmode + letter_cmd] = run_char;
    jump_table[hmode + other_char_cmd] = run_char;
    jump_table[hmode + char_given_cmd] = run_char;
    jump_table[hmode + spacer_cmd] = run_app_space;
    jump_table[hmode + ex_space_cmd] = run_app_space;
    jump_table[mmode + ex_space_cmd] = run_app_space;
    jump_table[hmode + boundary_cmd] = run_boundary;
    jump_table[hmode + char_ghost_cmd] = run_char_ghost;
    jump_table[mmode + char_ghost_cmd] = run_char_ghost;
    any_mode(relax_cmd, run_relax);
    jump_table[vmode + spacer_cmd] = run_relax;
    jump_table[mmode + spacer_cmd] = run_relax;
    jump_table[mmode + boundary_cmd] = run_relax;
    any_mode(ignore_spaces_cmd,run_ignore_spaces);
    jump_table[vmode + stop_cmd] = run_stop;
    jump_table[vmode + math_char_num_cmd] = run_non_math_math;
    jump_table[vmode + math_given_cmd] = run_non_math_math;
    jump_table[vmode + xmath_given_cmd] = run_non_math_math;
    jump_table[hmode + math_char_num_cmd] = run_math_char_num;
    jump_table[hmode + math_given_cmd] = run_math_given;
    jump_table[hmode + xmath_given_cmd] = run_xmath_given;

    jump_table[vmode + vmove_cmd] = report_illegal_case;
    jump_table[hmode + hmove_cmd] = report_illegal_case;
    jump_table[mmode + hmove_cmd] = report_illegal_case;
    any_mode(last_item_cmd, report_illegal_case);
    jump_table[vmode + vadjust_cmd] = report_illegal_case;
    jump_table[vmode + ital_corr_cmd] = report_illegal_case;
    non_math(eq_no_cmd,report_illegal_case);
    any_mode(mac_param_cmd,report_illegal_case);

    non_math(sup_mark_cmd, insert_dollar_sign);
    non_math(sub_mark_cmd, insert_dollar_sign);
    non_math(super_sub_script_cmd, insert_dollar_sign);
    non_math(no_super_sub_script_cmd, insert_dollar_sign);
    non_math(math_comp_cmd, insert_dollar_sign);
    non_math(delim_num_cmd, insert_dollar_sign);
    non_math(left_right_cmd, insert_dollar_sign);
    non_math(above_cmd, insert_dollar_sign);
    non_math(radical_cmd, insert_dollar_sign);
    non_math(math_style_cmd, insert_dollar_sign);
    non_math(math_choice_cmd, insert_dollar_sign);
    non_math(vcenter_cmd, insert_dollar_sign);
    non_math(non_script_cmd, insert_dollar_sign);
    non_math(mkern_cmd, insert_dollar_sign);
    non_math(limit_switch_cmd, insert_dollar_sign);
    non_math(mskip_cmd, insert_dollar_sign);
    non_math(math_accent_cmd, insert_dollar_sign);
    jump_table[mmode + endv_cmd] =  insert_dollar_sign;
    jump_table[mmode + par_end_cmd] =  insert_dollar_sign_par_end;
    jump_table[mmode + stop_cmd] =  insert_dollar_sign;
    jump_table[mmode + vskip_cmd] =  insert_dollar_sign;
    jump_table[mmode + un_vbox_cmd] =  insert_dollar_sign;
    jump_table[mmode + valign_cmd] =  insert_dollar_sign;
    jump_table[mmode + hrule_cmd] =  insert_dollar_sign;
    jump_table[mmode + no_hrule_cmd] =  insert_dollar_sign;
    jump_table[vmode + hrule_cmd] = run_rule;
    jump_table[vmode + no_hrule_cmd] = run_rule;
    jump_table[hmode + vrule_cmd] = run_rule;
    jump_table[hmode + no_vrule_cmd] = run_rule;
    jump_table[mmode + vrule_cmd] = run_rule;
    jump_table[mmode + no_vrule_cmd] = run_rule;
    jump_table[vmode + vskip_cmd] = append_glue;
    jump_table[hmode + hskip_cmd] = append_glue;
    jump_table[mmode + hskip_cmd] = append_glue;
    jump_table[mmode + mskip_cmd] = append_glue;
    any_mode(kern_cmd, append_kern);
    jump_table[mmode + mkern_cmd] = append_kern;
    non_math(left_brace_cmd, run_left_brace);
    any_mode(begin_group_cmd,run_begin_group);
    any_mode(end_group_cmd, run_end_group);
    any_mode(right_brace_cmd, handle_right_brace);
    jump_table[vmode + hmove_cmd] = run_move;
    jump_table[hmode + vmove_cmd] = run_move;
    jump_table[mmode + vmove_cmd] = run_move;
    any_mode(leader_ship_cmd, run_leader_ship);
    any_mode(make_box_cmd, run_make_box);
    any_mode(assign_box_dir_cmd, run_box_dir);
    any_mode(assign_box_direction_cmd, run_box_direction);
    jump_table[vmode + start_par_cmd] = run_start_par_vmode;
    jump_table[hmode + start_par_cmd] = run_start_par;
    jump_table[mmode + start_par_cmd] = run_start_par;
    jump_table[vmode + letter_cmd] = run_new_graf;
    jump_table[vmode + other_char_cmd] = run_new_graf;
    jump_table[vmode + char_num_cmd] = run_new_graf;
    jump_table[vmode + char_given_cmd] = run_new_graf;
    jump_table[vmode + char_ghost_cmd] = run_new_graf;
    jump_table[vmode + math_shift_cmd] = run_new_graf;
    jump_table[vmode + math_shift_cs_cmd] = run_new_graf;
    jump_table[vmode + un_hbox_cmd] = run_new_graf;
    jump_table[vmode + vrule_cmd] = run_new_graf;
    jump_table[vmode + no_vrule_cmd] = run_new_graf;
    jump_table[vmode + accent_cmd] = run_new_graf;
    jump_table[vmode + discretionary_cmd] = run_new_graf;
    jump_table[vmode + hskip_cmd] = run_new_graf;
    jump_table[vmode + valign_cmd] = run_new_graf;
    jump_table[vmode + ex_space_cmd] = run_new_graf;
    jump_table[vmode + boundary_cmd] = run_new_graf;
    jump_table[vmode + par_end_cmd] = run_par_end_vmode;
    jump_table[hmode + par_end_cmd] = run_par_end_hmode;
    jump_table[hmode + stop_cmd] = head_for_vmode;
    jump_table[hmode + vskip_cmd] = head_for_vmode;
    jump_table[hmode + hrule_cmd] = head_for_vmode;
    jump_table[hmode + no_hrule_cmd] = head_for_vmode;
    jump_table[hmode + un_vbox_cmd] = head_for_vmode;
    jump_table[hmode + halign_cmd] = head_for_vmode;
    any_mode(insert_cmd,begin_insert_or_adjust);
    jump_table[hmode + vadjust_cmd] = begin_insert_or_adjust;
    jump_table[mmode + vadjust_cmd] = begin_insert_or_adjust;
    any_mode(mark_cmd, handle_mark);
    any_mode(break_penalty_cmd, append_penalty);
    any_mode(remove_item_cmd, delete_last);
    jump_table[vmode + un_vbox_cmd] = unpackage;
    jump_table[hmode + un_hbox_cmd] = unpackage;
    jump_table[mmode + un_hbox_cmd] = unpackage;
    jump_table[hmode + ital_corr_cmd] = append_italic_correction;
    jump_table[mmode + ital_corr_cmd] = append_italic_correction_mmode;
    jump_table[hmode + discretionary_cmd] = append_discretionary;
    jump_table[mmode + discretionary_cmd] = append_discretionary;
    any_mode(assign_local_box_cmd, run_local_box);
    jump_table[hmode + accent_cmd] = make_accent;
    any_mode(car_ret_cmd,align_error);
    any_mode(tab_mark_cmd,align_error);
    any_mode(no_align_cmd,no_align_error);
    any_mode(omit_cmd, omit_error);
    jump_table[vmode + halign_cmd] = init_align;
    jump_table[hmode + valign_cmd] = init_align;
    jump_table[mmode + halign_cmd] = run_halign_mmode;
    jump_table[vmode + endv_cmd] = do_endv;
    jump_table[hmode + endv_cmd] = do_endv;
    any_mode(end_cs_name_cmd, cs_error);
    jump_table[hmode + math_shift_cmd] = init_math;
    jump_table[hmode + math_shift_cs_cmd] = init_math;
    jump_table[mmode + eq_no_cmd] = run_eq_no;
    jump_table[mmode + left_brace_cmd] = math_left_brace;
    jump_table[mmode + letter_cmd] = run_letter_mmode;
    jump_table[mmode + other_char_cmd] = run_letter_mmode;
    jump_table[mmode + char_given_cmd] = run_letter_mmode;
    jump_table[mmode + char_num_cmd] = run_char_num_mmode;
    jump_table[mmode + math_char_num_cmd] = run_math_char_num_mmode;
    jump_table[mmode + math_given_cmd] = run_math_given_mmode;
    jump_table[mmode + xmath_given_cmd] = run_xmath_given_mmode;
    jump_table[mmode + delim_num_cmd] = run_delim_num;
    jump_table[mmode + math_comp_cmd] = math_math_comp;
    jump_table[mmode + limit_switch_cmd] = math_limit_switch;
    jump_table[mmode + radical_cmd] = math_radical;
    jump_table[mmode + accent_cmd] = math_ac;
    jump_table[mmode + math_accent_cmd] = math_ac;
    jump_table[mmode + vcenter_cmd] = run_vcenter;
    jump_table[mmode + math_style_cmd] = run_math_style;
    jump_table[mmode + non_script_cmd] = run_non_script;
    jump_table[mmode + math_choice_cmd] = run_math_choice;
    jump_table[mmode + above_cmd] = math_fraction;
    jump_table[mmode + sub_mark_cmd] = sub_sup;
    jump_table[mmode + sup_mark_cmd] = sub_sup;
    jump_table[mmode + super_sub_script_cmd] = sub_sup;
    jump_table[mmode + no_super_sub_script_cmd] = no_sub_sup;
    jump_table[mmode + left_right_cmd] = math_left_right;
    jump_table[mmode + math_shift_cmd] = run_math_shift;
    jump_table[mmode + math_shift_cs_cmd] = run_math_shift;
    any_mode(toks_register_cmd, prefixed_command);
    any_mode(assign_toks_cmd, prefixed_command);
    any_mode(assign_int_cmd, prefixed_command);
    any_mode(assign_attr_cmd, prefixed_command);
    any_mode(assign_dir_cmd, prefixed_command);
    any_mode(assign_direction_cmd, prefixed_command);
    any_mode(assign_dimen_cmd, prefixed_command);
    any_mode(assign_glue_cmd, prefixed_command);
    any_mode(assign_mu_glue_cmd, prefixed_command);
    any_mode(assign_font_dimen_cmd, prefixed_command);
    any_mode(assign_font_int_cmd, prefixed_command);
    any_mode(set_aux_cmd, prefixed_command);
    any_mode(set_prev_graf_cmd, prefixed_command);
    any_mode(set_page_dimen_cmd, prefixed_command);
    any_mode(set_page_int_cmd, prefixed_command);
    any_mode(set_box_dimen_cmd, prefixed_command);
    any_mode(set_tex_shape_cmd, prefixed_command);
    any_mode(set_etex_shape_cmd, prefixed_command);
    any_mode(def_char_code_cmd, prefixed_command);
    any_mode(def_del_code_cmd, prefixed_command);
    any_mode(extdef_math_code_cmd, prefixed_command);
    any_mode(extdef_del_code_cmd, prefixed_command);
    any_mode(def_family_cmd, prefixed_command);
    any_mode(set_math_param_cmd, prefixed_command);
    any_mode(set_font_cmd, prefixed_command);
    any_mode(def_font_cmd, prefixed_command);
    any_mode(letterspace_font_cmd, prefixed_command);
    any_mode(copy_font_cmd, prefixed_command);
    any_mode(set_font_id_cmd, prefixed_command);
    any_mode(register_cmd, prefixed_command);
    any_mode(advance_cmd, prefixed_command);
    any_mode(multiply_cmd, prefixed_command);
    any_mode(divide_cmd, prefixed_command);
    any_mode(prefix_cmd, prefixed_command);
    any_mode(let_cmd, prefixed_command);
    any_mode(shorthand_def_cmd, prefixed_command);
    any_mode(read_to_cs_cmd, prefixed_command);
    any_mode(def_cmd, prefixed_command);
    any_mode(set_box_cmd, prefixed_command);
    any_mode(hyph_data_cmd, prefixed_command);
    any_mode(set_interaction_cmd, prefixed_command);
    any_mode(after_assignment_cmd,run_after_assignment);
    any_mode(after_group_cmd,run_after_group);
    any_mode(in_stream_cmd,open_or_close_in);
    any_mode(message_cmd,issue_message);
    any_mode(case_shift_cmd, shift_case);
    any_mode(xray_cmd, show_whatever);
    any_mode(normal_cmd, run_normal);
    any_mode(extension_cmd, run_extension);
    any_mode(option_cmd, run_option);

    any_mode(lua_function_call_cmd, lua_function_call);
    any_mode(lua_bytecode_call_cmd, lua_bytecode_call);
    any_mode(def_lua_call_cmd, prefixed_command);
    any_mode(lua_call_cmd, run_lua_call);
 /* any_mode(lua_expandable_call_cmd, run_lua_call); */ /* no! outside jump table anyway, handled in expand() */
    any_mode(node_cmd, run_node);

}

/*tex

    And here is |main_control| itself.  It is quite short nowadays.

*/

void main_control(void)
{
    main_control_state = goto_next;
    init_main_control() ;
    if (equiv(every_job_loc) != null) {
        begin_token_list(equiv(every_job_loc), every_job_text);
    }
    while (1) {
        if (main_control_state == goto_skip_token) {
            main_control_state = goto_next;
        } else {
            get_x_token();
        }
        /*tex
            Give diagnostic information, if requested When a new token has just
            been fetched at |big_switch|, we have an ideal place to monitor
            \TeX's activity.
        */
        if (interrupt != 0 && OK_to_interrupt) {
            back_input();
            check_interrupt();
            continue;
        }
        if (tracing_commands_par > 0) {
            show_cur_cmd_chr();
        }
        /*tex run the command */
        (jump_table[(abs(mode) + cur_cmd)])();
        if (main_control_state == goto_return) {
            return;
        }
    }
    /*tex not reached */
    return;
}

/*tex

We assume a trailing relax: |{...}\relax|, so we don't need a |back_input()| here.

*/

/*int local_level = 0; */

extern void local_control_message(const char *s)
{
    tprint("local control level ");
    print_int(local_level);
    tprint(": ");
    tprint(s);
    tprint_nl("");
}

void local_control(void)
{
    int ll = local_level;
    main_control_state = goto_next;
    local_level += 1;
    while (1) {
        if (main_control_state == goto_skip_token) {
            main_control_state = goto_next;
        } else {
            get_x_token();
        }
        if (interrupt != 0 && OK_to_interrupt) {
            back_input();
            check_interrupt();
            continue;
        }
        if (tracing_commands_par > 0) {
            show_cur_cmd_chr();
        }
        (jump_table[(abs(mode) + cur_cmd)])();
        if (local_level <= ll) {
            main_control_state = goto_next;
            if (tracing_nesting_par > 2) {
                local_control_message("leaving due to level change");
            }
            return ;
        } else if (main_control_state == goto_return) {
            if (tracing_nesting_par > 2) {
                local_control_message("leaving due to triggering");
            }
            return;
        }
    }
    return;
}

void end_local_control(void )
{
    local_level -= 1;
}

/*tex
    We need to go back to the main loop. This is rather nasty and dirty
    and counterintuive code and there might be a cleaner way. Basically
    we trigger the main control state from here.

    \starttyping
     0 0       \directlua{token.scan_list()}\hbox{!}
    -1 0       \setbox0\hbox{x}\directlua{token.scan_list()}\box0
     1 1       \toks0={\directlua{token.scan_list()}\hbox{x}}\directlua{tex.runtoks(0)}
     0 0  1 1  \directlua{tex.box[0]=token.scan_list()}\hbox{x\directlua{node.write(token.scan_list())}\hbox{x}}
     0 0  0 1  \setbox0\hbox{x}\directlua{tex.box[0]=token.scan_list()}\hbox{x\directlua{node.write(token.scan_list())}\box0}
    \stoptyping

    It's rather fragile code so we added some tracing options.

*/

halfword local_scan_box(void)
{
    int old_mode = mode;
    int ll = local_level;
    mode = -hmode;
    scan_box(lua_scan_flag);
    if (local_level == ll) {
        /*tex |\directlua{print(token.scan_list())}\hbox{!}| (n n) */
        if (tracing_nesting_par > 2) {
            local_control_message("entering at end of box scanning");
        }
        local_control();
    } else {
        /*tex |\directlua{print(token.scan_list())}\box0| (n-1 n) */
        /*
            if (tracing_nesting_par > 2) {
                local_control_message("setting level after box scanning");
            }
        */
        local_level = ll;
    }
    mode = old_mode;
    return cur_box;
}

/*tex

    We have an issue with modes when we quit here because we're coming
    from and still staying at the \LUA\ end. So, unless we're already
    nested, we trigger an end_local_level token (an extension code).

*/

static void wrapup_local_scan_box(void)
{
    /*
    if (tracing_nesting_par > 2) {
        local_control_message("leaving box scanner");
    }
    */
    local_level -= 1;
}

int current_local_level(void)
{
    return local_level;
}

void app_space(void)
{                               /* handle spaces when |space_factor<>1000| */
    halfword q;                 /* glue node */
    if ((space_factor_par >= 2000) && (! glue_is_zero(xspace_skip_par))) {
        q = new_param_glue(xspace_skip_code);
        /* so from now we have a subtype with spaces: */
        subtype(q) = xspace_skip_code + 1;
    } else {
        if (!glue_is_zero(space_skip_par)) {
            q = new_glue(space_skip_par);
        } else {
            q = new_glue(zero_glue);
            width(q) = space(cur_font_par);
            stretch(q) = space_stretch(cur_font_par);
            shrink(q) = space_shrink(cur_font_par);
        }
        /* Modify the glue specification in |q| according to the space factor */
        if (space_factor_par >= 2000)
            width(q) = width(q) + extra_space(cur_font_par);
        stretch(q) = xn_over_d(stretch(q), space_factor_par, 1000);
        shrink(q) = xn_over_d(shrink(q), 1000, space_factor_par);

        /* so from now we have a subtype with spaces: */
        subtype(q) = space_skip_code + 1;
    }
    couple_nodes(tail, q);
    tail = q;
}

void insert_dollar_sign(void)
{
    back_input();
    cur_tok = math_shift_token + '$';
    print_err("Missing $ inserted");
    help2(
        "I've inserted a begin-math/end-math symbol since I think",
        "you left one out. Proceed, with fingers crossed."
    );
    ins_error();
}

/*tex

    We can silently ignore  \.{\\par}s in a math formula.

*/

void insert_dollar_sign_par_end(void)
{
    if (!suppress_mathpar_error_par) {
        insert_dollar_sign() ;
    }
}

/*tex

The `|you_cant|' procedure prints a line saying that the current command is
illegal in the current mode; it identifies these things symbolically.

*/

void you_cant(void)
{
    print_err("You can't use `");
    print_cmd_chr((quarterword) cur_cmd, cur_chr);
    print_in_mode(mode);
}

/*tex

When erroneous situations arise, \TeX\ usually issues an error message specific
to the particular error. For example, `\.{\\noalign}' should not appear in any
mode, since it is recognized by the |align_peek| routine in all of its legitimate
appearances; a special error message is given when `\.{\\noalign}' occurs
elsewhere. But sometimes the most appropriate error message is simply that the
user is not allowed to do what he or she has attempted. For example,
`\.{\\moveleft}' is allowed only in vertical mode, and `\.{\\lower}' only in
non-vertical modes. Such cases are enumerated here and in the other sections
referred to under `See also \dots.'

*/

void report_illegal_case(void)
{
    you_cant();
    help4(
        "Sorry, but I'm not programmed to handle this case;",
        "I'll just pretend that you didn''t ask for it.",
        "If you're in the wrong mode, you might be able to",
        "return to the right one by typing `I}' or `I$' or `I\\par'."
    );
    error();
}

/*tex

Some operations are allowed only in privileged modes, i.e., in cases that
|mode>0|. The |privileged| function is used to detect violations of this rule; it
issues an error message and returns |false| if the current |mode| is negative.

*/

boolean privileged(void)
{
    if (mode > 0) {
        return true;
    } else {
        report_illegal_case();
        return false;
    }
}

/*tex

We don't want to leave |main_control| immediately when a |stop| command is
sensed, because it may be necessary to invoke an \.{\\output} routine several
times before things really grind to a halt. (The output routine might even say
`\.{\\gdef\\end\{...\}}', to prolong the life of the job.) Therefore
|its_all_over| is |true| only when the current page and contribution list are
empty, and when the last output was not a ``dead cycle.''

*/


boolean its_all_over(void)
{                               /* do this when \.{\\end} or \.{\\dump} occurs */
    if (privileged()) {
        if ((page_head == page_tail) && (head == tail) && (dead_cycles == 0)) {
            return true;
        }
        back_input();           /* we will try to end again after ejecting residual material */
        tail_append(new_null_box());
        width(tail) = hsize_par;
        tail_append(new_glue(fill_glue));
        tail_append(new_penalty(-010000000000,final_penalty));
        normal_page_filter(end);
        build_page();           /* append \.{\\hbox to \\hsize\{\}\\vfill\\penalty-'10000000000} */
    }
    return false;
}

/*tex

The |hskip| and |vskip| command codes are used for control sequences like
\.{\\hss} and \.{\\vfil} as well as for \.{\\hskip} and \.{\\vskip}. The
difference is in the value of |cur_chr|.

All the work relating to glue creation has been relegated to the following
subroutine. It does not call |build_page|, because it is used in at least one
place where that would be a mistake.

*/

void append_glue(void)
{
    int s = cur_chr;
    switch (s) {
        case fil_code:
            cur_val = new_glue(fil_glue);
            break;
        case fill_code:
            cur_val = new_glue(fill_glue);
            break;
        case ss_code:
            cur_val = new_glue(ss_glue);
            break;
        case fil_neg_code:
            cur_val = new_glue(fil_neg_glue);
            break;
        case skip_code:
            scan_glue(glue_val_level);
            break;
        case mskip_code:
            scan_glue(mu_val_level);
            break;
    }
    /* now |cur_val| points to the glue specification */
    tail_append(new_glue(cur_val));
    flush_node(cur_val);
    if (s > skip_code) {
        subtype(tail) = mu_glue;
    }
}

void append_kern(void)
{
    int s = cur_chr; /* |subtype| of the kern node */
    scan_dimen((s == mu_glue), false, false);
    tail_append(new_kern(cur_val));
    subtype(tail) = (quarterword) s;
}

/*tex

We have to deal with errors in which braces and such things are not properly
nested. Sometimes the user makes an error of commission by inserting an extra
symbol, but sometimes the user makes an error of omission. \TeX\ can't always
tell one from the other, so it makes a guess and tries to avoid getting into a
loop.

The |off_save| routine is called when the current group code is wrong. It tries
to insert something into the user's input that will help clean off the top level.

*/

void off_save(void)
{
    halfword p, q;
    if (cur_group == bottom_level) {
        /*tex Drop current token and complain that it was unmatched */
        print_err("Extra ");
        print_cmd_chr((quarterword) cur_cmd, cur_chr);
        help1(
            "Things are pretty mixed up, but I think the worst is over."
        );
        error();

    } else {
        back_input();
        p = get_avail();
        set_token_link(temp_token_head, p);
        print_err("Missing ");
        /*tex
            Prepare to insert a token that matches |cur_group|, and print what it
            is. At this point, |link(temp_token_head)=p|, a pointer to an empty
            one-word node.
        */
        switch (cur_group) {
            case semi_simple_group:
                set_token_info(p, cs_token_flag + frozen_end_group);
                tprint_esc("endgroup");
                break;
            case math_shift_group:
                set_token_info(p, math_shift_token + '$');
                print_char('$');
                break;
            case math_left_group:
                set_token_info(p, cs_token_flag + frozen_right);
                q = get_avail();
                set_token_link(p, q);
                p = token_link(p);
                set_token_info(p, other_token + '.');
                tprint_esc("right.");
                break;
            default:
                set_token_info(p, right_brace_token + '}');
                print_char('}');
                break;
        }
        tprint(" inserted");
        ins_list(token_link(temp_token_head));
        help5(
            "I've inserted something that you may have forgotten.",
            "(See the <inserted text> above.)",
            "With luck, this will get me unwedged. But if you",
            "really didn't forget anything, try typing `2' now; then",
            "my insertion and my current dilemma will both disappear."
        );
        error();
    }
}
/*tex

The routine for a |right_brace| character branches into many subcases, since a
variety of things may happen, depending on |cur_group|. Some types of groups are
not supposed to be ended by a right brace; error messages are given in hopes of
pinpointing the problem. Most branches of this routine will be filled in later,
when we are ready to understand them; meanwhile, we must prepare ourselves to
deal with such errors.

*/

void handle_right_brace(void)
{
    halfword p, q;              /* for short-term use */
    scaled d;                   /* holds |split_max_depth| in |insert_group| */
    int f;                      /* holds |floating_penalty| in |insert_group| */
    p = null;
    switch (cur_group) {
        case simple_group:
            fixup_directions();
            break;
        case bottom_level:
            print_err("Too many }'s");
            help2(
                "You've closed more groups than you opened.",
                "Such booboos are generally harmless, so keep going."
            );
            error();
            break;
        case semi_simple_group:
        case math_shift_group:
        case math_left_group:
            extra_right_brace();
            break;
        /*tex
            When the right brace occurs at the end of an \.{\\hbox} or
            \.{\\vbox} or \.{\\vtop} construction, the |package| routine
            comes into action. We might also have to finish a paragraph that
            hasn't ended.
        */
        case hbox_group:
            if (fixup_boxes_par) {
                /*tex
                    This is unofficial! Fixing up (also elsewhere) might become default
                    some day but for a while I will test this in ConTeXt.
                */
                fixup_directions_only();
            }
            package(0);
            break;
        case adjusted_hbox_group:
            adjust_tail = adjust_head;
            pre_adjust_tail = pre_adjust_head;
            package(0);
            break;
        case vbox_group:
            end_graf(vbox_group);
            package(0);
            break;
        case vtop_group:
            end_graf(vtop_group);
            package(vtop_code);
            break;
        case insert_group:
            end_graf(insert_group);
            q = new_glue(split_top_skip_par);
            d = split_max_depth_par;
            f = floating_penalty_par;
            unsave();
            save_ptr--;
            /*tex
                Now |saved_value(0)| is the insertion number, or the |vadjust| subtype.
            */
            p = vpack(vlink(head), 0, additional, -1);
            pop_nest();
            if (saved_type(0) == saved_insert) {
                tail_append(new_node(ins_node, saved_value(0)));
                height(tail) = height(p) + depth(p);
                ins_ptr(tail) = list_ptr(p);
                split_top_ptr(tail) = q;
                depth(tail) = d;
                float_cost(tail) = f;
            } else if (saved_type(0) == saved_adjust) {
                tail_append(new_node(adjust_node, saved_value(0)));
                adjust_ptr(tail) = list_ptr(p);
                flush_node(q);
            } else {
                confusion("insert_group");
            }
            list_ptr(p) = null;
            flush_node(p);
            if (nest_ptr == 0) {
                checked_page_filter(insert);
                build_page();
            }
            break;
        case output_group:
            /*tex
                this is needed in case the \.{\\output} executes a \.{\\textdir} command.
            */
            if (dir_level(text_dir_ptr) == cur_level) {
                /*tex Remove from |text_dir_ptr| */
                halfword text_dir_tmp = vlink(text_dir_ptr);
                flush_node(text_dir_ptr);
                text_dir_ptr = text_dir_tmp;
            }
            resume_after_output();
            break;
        case disc_group:
            build_discretionary();
            break;
        case local_box_group:
            build_local_box();
            break;
        case align_group:
            back_input();
            cur_tok = cs_token_flag + frozen_cr;
            print_err("Missing \\cr inserted");
            help1(
                "I'm guessing that you meant to end an alignment here."
            );
            ins_error();
            break;
        case no_align_group:
            end_graf(no_align_group);
            unsave();
            align_peek();
            break;
        case vcenter_group:
            end_graf(vcenter_group);
            finish_vcenter();
            break;
        case math_choice_group:
            build_choices();
            break;
        case math_group:
            close_math_group(p);
            break;
        default:
            confusion("rightbrace");
            break;
    }
}

void extra_right_brace(void)
{
    print_err("Extra }, or forgotten ");
    switch (cur_group) {
        case semi_simple_group:
            tprint_esc("endgroup");
            break;
        case math_shift_group:
            print_char('$');
            break;
        case math_left_group:
            tprint_esc("right");
            break;
    }
    help5(
        "I've deleted a group-closing symbol because it seems to be",
        "spurious, as in `$x}$'. But perhaps the } is legitimate and",
        "you forgot something else, as in `\\hbox{$x}'. In such cases",
        "the way to recover is to insert both the forgotten and the",
        "deleted material, e.g., by typing `I$}'."
    );
    error();
    incr(align_state);
}

/*tex

Here is where we clear the parameters that are supposed to revert to their
default values after every paragraph and when internal vertical mode is entered.

*/

void normal_paragraph(void)
{
    if (looseness_par != 0)
        eq_word_define(int_base + looseness_code, 0);
    if (hang_indent_par != 0)
        eq_word_define(dimen_base + hang_indent_code, 0);
    if (hang_after_par != 1)
        eq_word_define(int_base + hang_after_code, 1);
    if (par_shape_par_ptr != null)
        eq_define(par_shape_loc, shape_ref_cmd, null);
    if (inter_line_penalties_par_ptr != null)
        eq_define(inter_line_penalties_loc, shape_ref_cmd, null);
    if (shape_mode_par > 0)
        eq_word_define(dimen_base + shape_mode_code, 0);
}

/*tex

The global variable |cur_box| will point to a newly-made box. If the box is void,
we will have |cur_box=null|. Otherwise we will have |type(cur_box)=hlist_node| or
|vlist_node| or |rule_node|; the |rule_node| case can occur only with leaders.

*/

halfword cur_box;               /* box to be placed into its context */

/*tex

The |box_end| procedure does the right thing with |cur_box|, if |box_context|
represents the context as explained above.

*/

void box_end(int box_context)
{
    if (box_context < box_flag) {
        /*tex

            Append box |cur_box| to the current list, shifted by |box_context|.
            The global variable |adjust_tail| will be non-null if and only if the
            current box might include adjustments that should be appended to the
            current vertical list.

        */

        if (cur_box != null) {
            shift_amount(cur_box) = box_context;
            if (abs(mode) == vmode) {
                if (pre_adjust_tail != null) {
                    if (pre_adjust_head != pre_adjust_tail)
                        append_list(pre_adjust_head, pre_adjust_tail);
                    pre_adjust_tail = null;
                }
                append_to_vlist(cur_box,lua_key_index(box));
                if (adjust_tail != null) {
                    if (adjust_head != adjust_tail)
                        append_list(adjust_head, adjust_tail);
                    adjust_tail = null;
                }
                if (mode > 0) {
                    checked_page_filter(box);
                    build_page();
                }
            } else {
                if (abs(mode) == hmode)
                    space_factor_par = 1000;
                else
                    cur_box = new_sub_box(cur_box);
                couple_nodes(tail, cur_box);
                tail = cur_box;
            }
        }
    } else if (box_context < ship_out_flag) {
        /*tex
            Store |cur_box| in a box register
        */
        if (box_context < global_box_flag)
            eq_define(box_base + box_context - box_flag, box_ref_cmd, cur_box);
        else
            geq_define(box_base + box_context - global_box_flag, box_ref_cmd, cur_box);
    } else if (box_context == lua_scan_flag) {
        /*tex
            We are done with scanning so let's return to the caller.
        */
        wrapup_local_scan_box();
    } else if (cur_box != null) {
        /*tex
            The leaders contexts come after shipout and luascan contexts.
        */
        /* if (box_context > lua_scan_flag) { */
        if (box_context >= leader_flag) {
            /*tex
                Append a new leader node that uses |cur_box| and get the next
                non-blank non-relax...
            */
            do {
                get_x_token();
            } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));
            if (((cur_cmd == hskip_cmd) && (abs(mode) != vmode)) ||
                ((cur_cmd == vskip_cmd) && (abs(mode) == vmode))) {
                append_glue();
                subtype(tail) = (quarterword) (box_context - (leader_flag - a_leaders));
                leader_ptr(tail) = cur_box;
            } else {
                print_err("Leaders not followed by proper glue");
                help3(
                    "You should say `\\leaders <box or rule><hskip or vskip>'.",
                    "I found the <box or rule>, but there's no suitable",
                    "<hskip or vskip>, so I'm ignoring these leaders."
                );
                back_error();
                flush_node_list(cur_box);
            }
        } else {
            if (box_context != ship_out_flag) {
                normal_error("scanner","shipout expected");
            }
            ship_out(static_pdf, cur_box, SHIPPING_PAGE);
        }
    }
}

/*tex

the next input should specify a box or perhaps a rule

*/

void scan_box(int box_context)
{
    /*tex Get the next non-blank non-relax... */
    do {
        get_x_token();
    } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));
    if (cur_cmd == make_box_cmd) {
        begin_box(box_context);
    } else if ((box_context >= leader_flag) &&
            ((cur_cmd == hrule_cmd) || (cur_cmd == vrule_cmd) ||
             (cur_cmd == no_hrule_cmd) || (cur_cmd == no_vrule_cmd))) {
        cur_box = scan_rule_spec();
        box_end(box_context);
    } else {
        print_err("A <box> was supposed to be here");
        help3(
            "I was expecting to see \\hbox or \\vbox or \\copy or \\box or",
            "something like that. So you might find something missing in",
            "your output. But keep trying; you can fix this later."
        );
        back_error();
        if (box_context == lua_scan_flag) {
            cur_box = null;
            box_end(box_context);
        }
    }
}

void new_graf(boolean indented)
{
    halfword p, q, dir_graf_tmp;
    halfword dir_rover;
    int callback_id;
    if ((mode == vmode) || (head != tail)) {
        tail_append(new_param_glue(par_skip_code));
    }
    callback_id = callback_defined(new_graf_callback);
    if (callback_id > 0) {
        run_callback(callback_id, "db->b", cur_list.mode_field,indented,&indented);
    }
    prev_graf_par = 0;
    push_nest();
    mode = hmode;
    space_factor_par = 1000;
    /*tex  Add local paragraph node */
    tail_append(make_local_par_node(new_graf_par_code));
    if (indented) {
        p = new_null_box();
        box_dir(p) = par_direction_par;
        width(p) = par_indent_par;
        subtype(p) = indent_list;
        q = tail;
        tail_append(p);
    } else {
        q = tail;
    }
    dir_rover = text_dir_ptr;
    while (dir_rover != null) {
        if ((vlink(dir_rover) != null) || (dir_dir(dir_rover) != par_direction_par)) {
            dir_graf_tmp = new_dir(dir_dir(dir_rover));
            try_couple_nodes(dir_graf_tmp,vlink(q));
            couple_nodes(q,dir_graf_tmp);
        }
        dir_rover = vlink(dir_rover);
    }
    q = head;
    while (vlink(q) != null)
        q = vlink(q);
    tail = q;
    if (every_par_par != null)
        begin_token_list(every_par_par, every_par_text);
    if (nest_ptr == 1) {
        checked_page_filter(new_graf);
        /*tex put |par_skip| glue on current page */
        build_page();
    }
}

void indent_in_hmode(void)
{
    halfword p;
    if (cur_chr > 0) {
        /*tex \.{\\indent} */
        p = new_null_box();
        width(p) = par_indent_par;
        if (abs(mode) == hmode)
            space_factor_par = 1000;
        else
            p = new_sub_box(p);
        tail_append(p);
    }
}

void head_for_vmode(void)
{
    if (mode < 0) {
        if ((cur_cmd != hrule_cmd) && (cur_cmd != no_hrule_cmd)) {
            off_save();
        } else {
            print_err("You can't use `\\hrule' here except with leaders");
            help2(
                "To put a horizontal rule in an hbox or an alignment,",
                "you should use \\leaders or \\hrulefill (see The TeXbook)."
            );
            error();
        }
    } else {
        back_input();
        cur_tok = par_token;
        back_input();
        token_type = inserted;
    }
}

/*tex

|dir_save| would have been set by |line_break| by means of |post_line_break|, but
this is not done right now, as it introduces pretty heavy memory leaks. This
means the current code might be wrong in some way that relates to in-paragraph
displays.

*/

void end_graf(int line_break_context)
{
    if (mode == hmode) {
        if ((head == tail) || (vlink(head) == tail)) {
            if (vlink(head) == tail)
                flush_node(vlink(head));
            /*tex |null| paragraphs are ignored, all contain a |local_paragraph| node */
            pop_nest();
        } else {
            line_break(false, line_break_context);
        }
        if (dir_save != null) {
            flush_node_list(dir_save);
            dir_save = null;
        }
        normal_paragraph();
        error_count = 0;
    }
}

void begin_insert_or_adjust(void)
{
    if (cur_cmd != vadjust_cmd) {
        scan_register_num();
        if (cur_val == output_box_par) {
            print_err("You can't \\insert");
            print_int(output_box_par);
            help1(
                "I'm changing to \\insert0; box \\outputbox is special."
            );
            error();
            cur_val = 0;
        }
        set_saved_record(0, saved_insert, 0, cur_val);
    } else if (scan_keyword("pre")) {
        set_saved_record(0, saved_adjust, 0, 1);
    } else {
        set_saved_record(0, saved_adjust, 0, 0);
    }
    save_ptr++;
    new_save_level(insert_group);
    scan_left_brace();
    normal_paragraph();
    push_nest();
    mode = -vmode;
    prev_depth_par = ignore_depth;
}

/*tex

I (TH)'ve renamed the |make_mark| procedure to this, because if the current chr
code is 1, then the actual command was \.{\\clearmarks}, which does not generate
a mark node but instead destroys the current mark tokenlists.

*/

void handle_mark(void)
{
    halfword p;                 /* new node */
    halfword c;                 /* the mark class */
    if (cur_chr == clear_marks_code) {
        scan_mark_num();
        c = cur_val;
        delete_top_mark(c);
        delete_bot_mark(c);
        delete_first_mark(c);
        delete_split_first_mark(c);
        delete_split_bot_mark(c);
    } else {
        if (cur_chr == 0) {
            c = 0;
        } else {
            scan_mark_num();
            c = cur_val;
            if (c > biggest_used_mark)
                biggest_used_mark = c;
        }
        p = scan_toks(false, true);
        p = new_node(mark_node, 0);     /* the |subtype| is not used */
        mark_class(p) = c;
        mark_ptr(p) = def_ref;
        couple_nodes(tail, p);
        tail = p;
    }
}

void append_penalty(void)
{
    scan_int();
    tail_append(new_penalty(cur_val,user_penalty));
    if (mode == vmode) {
        checked_page_filter(penalty);
        build_page();
    }
}

/*tex

When |delete_last| is called, |cur_chr| is the |type| of node that will be
deleted, if present.

The |remove_item| command removes a penalty, kern, or glue node if it appears at
the tail of the current list, using a brute-force linear scan. Like
\.{\\lastbox}, this command is not allowed in vertical mode (except internal
vertical mode), since the current list in vertical mode is sent to the page
builder. But if we happen to be able to implement it in vertical mode, we do.

*/

void delete_last(void)
{
    halfword p, q;              /* run through the current list */
    if ((mode == vmode) && (tail == head)) {
        /*tex
            Apologize for inability to do the operation now, unless \.{\\unskip}
            follows non-glue
        */
        if ((cur_chr != glue_node) || (last_glue != max_halfword)) {
            you_cant();
            if (cur_chr == kern_node) {
                help2(
                    "Sorry...I usually can't take things from the current page.",
                    "Try `I\\kern-\\lastkern' instead."
                );
            } else if (cur_chr != glue_node) {
                help2
                    ("Sorry...I usually can't take things from the current page.",
                     "Perhaps you can make the output routine do it.");
            } else {
                help2
                    ("Sorry...I usually can't take things from the current page.",
                     "Try `I\\vskip-\\lastskip' instead.");
            }
            error();
        }
    } else {
        /*tex Todo: clean this up! */
        if (!is_char_node(tail)) {
            if (type(tail) == cur_chr) {
                q = head;
                do {
                    p = q;
                    if (!is_char_node(q)) {
                        if (type(q) == disc_node) {
                            if (p == tail)
                                return;
                        }
                    }
                    q = vlink(p);
                } while (q != tail);
                vlink(p) = null;
                flush_node_list(tail);
                tail = p;
            }
        }
    }
}

void unpackage(void)
{
    halfword p;                 /* the box */
    halfword r;                 /* to remove marginal kern nodes */
    int c;                      /* should we copy? */
    halfword s;                 /* for varmem assignment */
    if (cur_chr > copy_code) {
        /*tex Handle saved items and |goto done| */
        try_couple_nodes(tail, disc_ptr[cur_chr]);
        disc_ptr[cur_chr] = null;
        goto DONE;
    }
    c = cur_chr;
    scan_register_num();
    p = box(cur_val);
    if (p == null)
        return;
    if ((abs(mode) == mmode)
        || ((abs(mode) == vmode) && (type(p) != vlist_node))
        || ((abs(mode) == hmode) && (type(p) != hlist_node))) {
        print_err("Incompatible list can't be unboxed");
        help3(
            "Sorry, Pandora. (You sneaky devil.)",
            "I refuse to unbox an \\hbox in vertical mode or vice versa.",
            "And I can't open any boxes in math mode."
        );
        error();
        return;
    }
    if (c == copy_code) {
        s = copy_node_list(list_ptr(p));
        try_couple_nodes(tail,s);
    } else {
        try_couple_nodes(tail,list_ptr(p));
        box(cur_val) = null;
        list_ptr(p) = null;
        flush_node(p);
    }
  DONE:
    while (vlink(tail) != null) {
        r = vlink(tail);
        if (!is_char_node(r) && (type(r) == margin_kern_node)) {
            try_couple_nodes(tail,vlink(r));
            flush_node(r);
        }
        tail = vlink(tail);
    }
}

/*tex

Italic corrections are converted to kern nodes when the |ital_corr| command
follows a character. In math mode the same effect is achieved by appending a kern
of zero here, since italic corrections are supplied later.

*/

void append_italic_correction(void)
{
    halfword p;                 /* |char_node| at the tail of the current list */
    internal_font_number f;     /* the font in the |char_node| */
    if (tail != head) {
        if (is_char_node(tail))
            p = tail;
        else
            return;
        f = font(p);
        tail_append(new_kern(char_italic(f, character(p))));
        subtype(tail) = italic_kern;
    }
}

void append_local_box(int kind)
{
    incr(save_ptr);
    set_saved_record(-1, saved_boxtype, 0, kind);
    new_save_level(local_box_group);
    scan_left_brace();
    push_nest();
    mode = -hmode;
    space_factor_par = 1000;
}

/*tex

Discretionary nodes are easy in the common case `\.{\\-}', but in the general
case we must process three braces full of items.

The space factor does not change when we append a discretionary node, but it
starts out as 1000 in the subsidiary lists.

*/

void append_discretionary(void)
{
    int c;
    tail_append(new_disc());
    subtype(tail) = (quarterword) cur_chr;
    if (cur_chr == explicit_disc) {
        /* |\-| */
        c = get_pre_hyphen_char(cur_lang_par);
        if (c > 0) {
            vlink(pre_break(tail)) = new_char(equiv(cur_font_loc), c);
            alink(vlink(pre_break(tail))) = pre_break(tail);
            tlink(pre_break(tail)) = vlink(pre_break(tail));
        }
        c = get_post_hyphen_char(cur_lang_par);
        if (c > 0) {
            vlink(post_break(tail)) = new_char(equiv(cur_font_loc), c);
            alink(vlink(post_break(tail))) = post_break(tail);
            tlink(post_break(tail)) = vlink(post_break(tail));
        }
        set_explicit_disc_penalty(tail);
    } else if (cur_chr == automatic_disc) {
        /*tex As done in hyphenator: */
        c = get_pre_exhyphen_char(cur_lang_par);
        if (c <= 0) {
            c = ex_hyphen_char_par;
        }
        if (c > 0) {
            vlink(pre_break(tail)) = new_char(equiv(cur_font_loc), c);
            alink(vlink(pre_break(tail))) = pre_break(tail);
            tlink(pre_break(tail)) = vlink(pre_break(tail));
        }
        c = get_post_exhyphen_char(cur_lang_par);
        if (c > 0) {
            vlink(post_break(tail)) = new_char(equiv(cur_font_loc), c);
            alink(vlink(post_break(tail))) = post_break(tail);
            tlink(post_break(tail)) = vlink(post_break(tail));
        }
        c = ex_hyphen_char_par;
        if (c > 0) {
            vlink(no_break(tail)) = new_char(equiv(cur_font_loc), c);
            alink(vlink(no_break(tail))) = no_break(tail);
            tlink(no_break(tail)) = vlink(no_break(tail));
        }
        set_automatic_disc_penalty(tail);
    } else {
        /*tex |\discretionary| */
        if (scan_keyword("penalty")) {
            scan_int();
            disc_penalty(tail) = cur_val;
        }
        incr(save_ptr);
        set_saved_record(-1, saved_disc, 0, 0);
        new_save_level(disc_group);
        scan_left_brace();
        push_nest();
        mode = -hmode;
        space_factor_par = 1000;
        /*tex Already preset: |disc_penalty(tail) = hyphen_penalty_par;| */
    }
}

/*tex

The test for |p != null| ensures that empty \.{\\localleftbox} and
\.{\\localrightbox} commands are not applied.

*/

void build_local_box(void)
{
    halfword p;
    int kind;
    unsave();
    assert(saved_type(-1) == saved_boxtype);
    kind = saved_value(-1);
    decr(save_ptr);
    p = vlink(head);
    pop_nest();
    if (p != null) {
        /*tex
            Somehow |filtered_hpack| goes beyond the first node so we loose it.
        */
        /*tex
            There is no need for |new_hyphenation(p, null);| here as we're in
            an |\hbox|.
        */
        (void) new_ligkern(p, null);
        p = lua_hpack_filter(p, 0, additional, local_box_group, -1, null);
        /*tex
            We really need something packed so we play safe! This feature is inherited
            but could have been delegated to a callback anyway.
        */
        p = hpack(p, 0, additional, -1);
    }
    if (kind == 0)
        eq_define(local_left_box_base, box_ref_cmd, p);
    else
        eq_define(local_right_box_base, box_ref_cmd, p);
    if (abs(mode) == hmode) {
        /*tex Add local paragraph node */
        tail_append(make_local_par_node(local_box_par_code));
    }
    eq_word_define(int_base + no_local_whatsits_code, no_local_whatsits_par + 1);
}

/*tex

The three discretionary lists are constructed somewhat as if they were hboxes.
A~subroutine called |build_discretionary| handles the transitions. (This is sort
of fun.)

*/

void build_discretionary(void)
{
    halfword p, q;              /* for link manipulation */
    int n;                      /* length of discretionary list */
    unsave();
    /*tex
        Prune the current list, if necessary, until it contains only |char_node|,
        |kern_node|, |hlist_node|, |vlist_node| and |rule_node| items; set |n| to
        the length of the list, and set |q| to the lists tail. During this loop,
        |p=vlink(q)| and there are |n| items preceding |p|.
    */
    q = head;
    p = vlink(q);
    n = 0;
    while (p != null) {
        if (!is_char_node(p) && type(p) > rule_node && type(p) != kern_node) {
            print_err("Improper discretionary list");
            help1(
                "Discretionary lists must contain only boxes and kerns."
            );
            error();
            begin_diagnostic();
            tprint_nl("The following discretionary sublist has been deleted:");
            show_box(p);
            end_diagnostic(true);
            flush_node_list(p);
            vlink(q) = null;
            break;
        }
        alink(p) = q;
        q = p;
        p = vlink(q);
        incr(n);
    }

    p = vlink(head);
    pop_nest();
    assert(saved_type(-1) == saved_disc);
    switch (saved_value(-1)) {
    case 0:
        if (n > 0) {
            vlink(pre_break(tail)) = p;
            alink(p) = pre_break(tail);
            tlink(pre_break(tail)) = q;
        }
        break;
    case 1:
        if (n > 0) {
            vlink(post_break(tail)) = p;
            alink(p) = post_break(tail);
            tlink(post_break(tail)) = q;
        }
        break;
    case 2:
        /*tex
            Attach list |p| to the current list, and record its length; then
            finish up and |return|
        */
        if ((n > 0) && (abs(mode) == mmode)) {
            print_err("Illegal math \\discretionary");
            help2(
                "Sorry: The third part of a discretionary break must be",
                "empty, in math formulas. I had to delete your third part."
            );
            flush_node_list(p);
            error();
        } else {
            if (n > 0) {
                vlink(no_break(tail)) = p;
                alink(p) = no_break(tail);
                tlink(no_break(tail)) = q;
            }
        }
        decr(save_ptr);
        /*tex There are no other cases. */
        return;
        break;
    }
    set_saved_record(-1, saved_disc, 0, (saved_value(-1) + 1));
    new_save_level(disc_group);
    scan_left_brace();
    push_nest();
    mode = -hmode;
    space_factor_par = 1000;
}

/*tex

The positioning of accents is straightforward but tedious. Given an accent of
width |a|, designed for characters of height |x| and slant |s|; and given a
character of width |w|, height |h|, and slant |t|: We will shift the accent down
by |x-h|, and we will insert kern nodes that have the effect of centering the
accent over the character and shifting the accent to the right by
$\delta={1\over2}(w-a)+h\cdot t-x\cdot s$. If either character is absent from the
font, we will simply use the other, without shifting.

*/

void make_accent(void)
{
    double s, t;                /* amount of slant */
    halfword p, q, r;           /* character, box, and kern nodes */
    internal_font_number f;     /* relevant font */
    scaled a, h, x, w, delta;   /* heights and widths, as explained above */
    scan_char_num();
    f = equiv(cur_font_loc);
    p = new_glyph(f, cur_val);
    if (p != null) {
        x = x_height(f);
        /*tex real division */
        s = float_cast(slant(f)) / float_constant(65536);
        a = glyph_width(p);
        do_assignments();
        /*tex
            Create a character node |q| for the next character, but set |q:=null|
            if problems arise
        */
        q = null;
        f = equiv(cur_font_loc);
        if ((cur_cmd == letter_cmd) ||
            (cur_cmd == other_char_cmd) || (cur_cmd == char_given_cmd)) {
            q = new_glyph(f, cur_chr);
        } else if (cur_cmd == char_num_cmd) {
            scan_char_num();
            q = new_glyph(f, cur_val);
        } else {
            back_input();
        }

        if (q != null) {
            /*tex
                Append the accent with appropriate kerns, then set |p:=q|. The
                kern nodes appended here must be distinguished from other kerns,
                lest they be wiped away by the hyphenation algorithm or by a
                previous line break. The two kerns are computed with
                (machine-dependent) |real| arithmetic, but their sum is
                machine-independent; the net effect is machine-independent,
                because the user cannot remove these nodes nor access them via
                \.{\\lastkern}.
             */
            t = float_cast(slant(f)) / float_constant(65536);   /* real division */
            w = glyph_width(q);
            h = glyph_height(q);
            if (h != x) {
                /*tex the accent must be shifted up or down */
                p = hpack(p, 0, additional, -1);
                shift_amount(p) = x - h;
            }
            /*tex real multiplication */
            delta = round(float_cast(w - a) / float_constant(2) + h * t - x * s);
            r = new_kern(delta);
            subtype(r) = accent_kern;
            couple_nodes(tail, r);
            couple_nodes(r, p);
            tail = new_kern(-a - delta);
            subtype(tail) = accent_kern;
            couple_nodes(p, tail);
            p = q;

        }
        couple_nodes(tail, p);
        tail = p;
        space_factor_par = 1000;
    }
}

/*tex

When `\.{\\cr}' or `\.{\\span}' or a tab mark comes through the scanner into
|main_control|, it might be that the user has foolishly inserted one of them into
something that has nothing to do with alignment. But it is far more likely that a
left brace or right brace has been omitted, since |get_next| takes actions
appropriate to alignment only when `\.{\\cr}' or `\.{\\span}' or tab marks occur
with |align_state=0|. The following program attempts to make an appropriate
recovery.

*/

void align_error(void)
{
    if (abs(align_state) > 2) {
        /*tex
            Express consternation over the fact that no alignment is in progress.
        */
        print_err("Misplaced ");
        print_cmd_chr((quarterword) cur_cmd, cur_chr);
        if (cur_tok == tab_token + '&') {
            help6(
                "I can't figure out why you would want to use a tab mark",
                "here. If you just want an ampersand, the remedy is",
                "simple: Just type `I\\&' now. But if some right brace",
                "up above has ended a previous alignment prematurely,",
                "you're probably due for more error messages, and you",
                "might try typing `S' now just to see what is salvageable."
            );
        } else {
            help5(
                "I can't figure out why you would want to use a tab mark",
                "or \\cr or \\span just now. If something like a right brace",
                "up above has ended a previous alignment prematurely,",
                "you're probably due for more error messages, and you",
                "might try typing `S' now just to see what is salvageable."
            );
        }
        error();

    } else {
        back_input();
        if (align_state < 0) {
            print_err("Missing { inserted");
            incr(align_state);
            cur_tok = left_brace_token + '{';
        } else {
            print_err("Missing } inserted");
            decr(align_state);
            cur_tok = right_brace_token + '}';
        }
        help3(
            "I've put in what seems to be necessary to fix",
            "the current column of the current alignment.",
            "Try to go on, since this might almost work."
        );
        ins_error();
    }
}

/*tex

The help messages here contain a little white lie, since \.{\\noalign} and
\.{\\omit} are allowed also after `\.{\\noalign\{...\}}'.

*/

void no_align_error(void)
{
    print_err("Misplaced \\noalign");
    help2(
        "I expect to see \\noalign only after the \\cr of",
        "an alignment. Proceed, and I'll ignore this case."
    );
    error();
}

void omit_error(void)
{
    print_err("Misplaced \\omit");
    help2(
        "I expect to see \\omit only after tab marks or the \\cr of",
        "an alignment. Proceed, and I'll ignore this case."
    );
    error();
}

/*tex

We've now covered most of the abuses of \.{\\halign} and \.{\\valign}. Let's take
a look at what happens when they are used correctly.

An |align_group| code is supposed to remain on the |save_stack| during an entire
alignment, until |fin_align| removes it.

A devious user might force an |endv| command to occur just about anywhere; we
must defeat such hacks.

*/

void do_endv(void)
{
    base_ptr = input_ptr;
    input_stack[base_ptr] = cur_input;
    while ((input_stack[base_ptr].index_field != v_template) &&
           (input_stack[base_ptr].loc_field == null) &&
           (input_stack[base_ptr].state_field == token_list))
        decr(base_ptr);
    if ((input_stack[base_ptr].index_field != v_template) ||
        (input_stack[base_ptr].loc_field != null) ||
        (input_stack[base_ptr].state_field != token_list))
        fatal_error("(interwoven alignment preambles are not allowed)");
    /*tex interwoven alignment preambles... */
    if (cur_group == align_group) {
        end_graf(align_group);
        if (fin_col())
            fin_row();
    } else {
        off_save();
    }
}

/*tex

Finally, \.{\\endcsname} is not supposed to get through to |main_control|.

*/

void cs_error(void)
{
    print_err("Extra \\endcsname");
    help1(
        "I'm ignoring this, since I wasn't doing a \\csname."
    );
    error();
}

/*tex

Assignments to values in |eqtb| can be global or local. Furthermore, a control
sequence can be defined to be `\.{\\long}', `\.{\\protected}', or `\.{\\outer}',
and it might or might not be expanded. The prefixes `\.{\\global}', `\.{\\long}',
`\.{\\protected}', and `\.{\\outer}' can occur in any order. Therefore we assign
binary numeric codes, making it possible to accumulate the union of all specified
prefixes by adding the corresponding codes. (PASCAL's |set| operations could also
have been used.)

Every prefix, and every command code that might or might not be prefixed, calls
the action procedure |prefixed_command|. This routine accumulates a sequence of
prefixes until coming to a non-prefix, then it carries out the command.

*/

/*tex

If the user says, e.g., `\.{\\global\\global}', the redundancy is silently
accepted. The different types of code values have different legal ranges; the
following program is careful to check each case properly.

*/

#define check_def_code(A) do { \
    if (((cur_val<0)&&(p<(A)))||(cur_val>n)) { \
        print_err("Invalid code ("); \
        print_int(cur_val); \
        if (p<(A)) \
            tprint("), should be in the range 0.."); \
        else \
            tprint("), should be at most "); \
        print_int(n); \
        help1( \
            "I'm going to use 0 instead of that illegal code value." \
        ); \
        error(); \
        cur_val=0; \
    } \
} while (0)

/*
halfword swap_hang_indent(halfword indentation, halfword shape_mode) {
    if (shape_mode == 1 || shape_mode == 3 || shape_mode == -1 || shape_mode == -3) {
        return negate(indentation);
    } else {
        return indentation;
    }
}

halfword swap_parshape_indent(halfword indentation, halfword width, halfword shape_mode) {
    if (shape_mode == 2 || shape_mode == 3 || shape_mode == -2 || shape_mode == -3) {
        return hsize_par - width - indentation;
    } else {
        return indentation;
    }
}

*/

void prefixed_command(void)
{
    int a;                      /* accumulated prefix codes so far */
    internal_font_number f;     /* identifies a font */
    halfword j;                 /* index into a \.{\\parshape} specification */
    halfword p, q;              /* for temporary short-term use */
    int n;                      /* ditto */
    boolean e, check_glue;      /* should a definition be expanded? or was \.{\\let} not done? */
    mathcodeval mval;           /* for handling of \.{\\mathchardef}s */
    a = 0;
    while (cur_cmd == prefix_cmd) {
        if (!odd(a / cur_chr))
            a = a + cur_chr;
        /*tex
            Get the next non-blank non-relax...
        */
        do {
            get_x_token();
        } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));

        if (cur_cmd <= max_non_prefixed_command) {
            /*tex
                Discard erroneous prefixes and |return|
            */
            print_err("You can't use a prefix with `");
            print_cmd_chr((quarterword) cur_cmd, cur_chr);
            print_char('\'');
            help2(
                "I'll pretend you didn't say \\long or \\outer or \\global or",
                "\\protected."
            );
            back_error();
            return;
        }
        if (tracing_commands_par > 2)
            show_cur_cmd_chr();
    }
    /*tex
        Discard the prefixes \.{\\long} and \.{\\outer} if they are irrelevant
    */
    if (a >= 8) {
        j = protected_token;
        a = a - 8;
    } else {
        j = 0;
    }
    if ((cur_cmd != def_cmd) && (cur_cmd != def_lua_call_cmd) && ((a % 4 != 0) || (j != 0))) {
        print_err("You can't use `\\long' or `\\outer' or `\\protected' with `");
        print_cmd_chr((quarterword) cur_cmd, cur_chr);
        print_char('\'');
        help1(
            "I'll pretend you didn't say \\long or \\outer or \\protected here."
        );
        error();
    }
    /*tex
        Adjust for the setting of \.{\\globaldefs}
    */
    if (global_defs_par != 0) {
        if (global_defs_par < 0) {
            if (is_global(a))
                a = a - 4;
        } else {
            if (!is_global(a))
                a = a + 4;
        }
    }
    switch (cur_cmd) {
        case set_font_cmd:
            /*tex
                Here's an example of the way many of the following routines operate.
                (Unfortunately, they aren't all as simple as this.)
            */
            define(cur_font_loc, data_cmd, cur_chr);
            break;
        case def_cmd:
            /*tex
                When a |def| command has been scanned, |cur_chr| is odd if the
                definition is supposed to be global, and |cur_chr>=2| if the
                definition is supposed to be expanded.
            */
            if (odd(cur_chr) && !is_global(a) && (global_defs_par >= 0))
                a = a + 4;
            e = (cur_chr >= 2);
            get_r_token();
            p = cur_cs;
            q = scan_toks(true, e);
            if (j != 0) {
                q = get_avail();
                set_token_info(q, j);
                set_token_link(q, token_link(def_ref));
                set_token_link(def_ref, q);
            }
            define(p, call_cmd + (a % 4), def_ref);
            break;
        case let_cmd:
            n = cur_chr;
            switch (n) {
                case 0:
                    /*tex |glet| */
                    if (!is_global(a) && (global_defs_par >= 0)) {
                        a = a + 4;
                    }
                case 1:
                    /*tex |let| */
                    get_r_token();
                    p = cur_cs;
                    do {
                        get_token();
                    } while (cur_cmd == spacer_cmd);
                    if (cur_tok == other_token + '=') {
                        get_token();
                        if (cur_cmd == spacer_cmd)
                            get_token();
                    }
                    break;
                case 2:
                    /*tex |futurelet| */
                    get_r_token();
                    p = cur_cs;
                    get_token();
                    q = cur_tok;
                    get_token();
                    back_input();
                    cur_tok = q;
                    /*tex
                        We look ahead and then back up. Note that |back_input| doesn't
                        affect |cur_cmd|, |cur_chr|
                    */
                    back_input();
                    break;
                case 3:
                    /*tex |letcharcode| */
                    scan_int();
                    if (cur_val > 0) {
                        /*tex HH: I need to  do a more extensive test later. */
                        p = active_to_cs(cur_val, true);
                        do {
                            get_token();
                        } while (cur_cmd == spacer_cmd);
                        if (cur_tok == other_token + '=') {
                            get_token();
                            if (cur_cmd == spacer_cmd)
                                get_token();
                        }
                    } else {
                        p = null;
                        tex_error("invalid number for \\letcharcode",NULL);
                    }
                    break;
                default:
                    /*tex We please the compiler. */
                    p = null;
                    confusion("let");
                    break;
            }
            if (cur_cmd >= call_cmd)
                add_token_ref(cur_chr);
            define(p, cur_cmd, cur_chr);
            break;
        case shorthand_def_cmd:
            /*tex
                We temporarily define |p| to be |relax|, so that an occurrence of
                |p| while scanning the definition will simply stop the scanning
                instead of producing an ``undefined control sequence'' error or
                expanding the previous meaning. This allows, for instance,
                `\.{\\chardef\\foo=123\\foo}'.
            */
            n = cur_chr;
            get_r_token();
            p = cur_cs;
            define(p, relax_cmd, too_big_char);
            scan_optional_equals();
            switch (n) {
            case char_def_code:
                scan_char_num();
                define(p, char_given_cmd, cur_val);
                break;
            case math_char_def_code:
                mval = scan_mathchar(tex_mathcode);
             /* if (math_umathcode_meaning_par == 1) { */
             /*     cur_val = (mval.class_value + (8 * mval.family_value)) * (65536 * 32) + mval.character_value; */
             /*     define(p, xmath_given_cmd, cur_val); */
             /* } else { */
                    cur_val = (mval.class_value * 16 + mval.family_value) * 256 + mval.character_value;
                    define(p, math_given_cmd, cur_val);
             /* } */
                break;
            case xmath_char_def_code:
                mval = scan_mathchar(umath_mathcode);
                cur_val = (mval.class_value + (8 * mval.family_value)) * (65536 * 32) + mval.character_value;
                define(p, xmath_given_cmd, cur_val);
                break;
            case umath_char_def_code:
                mval = scan_mathchar(umathnum_mathcode);
                cur_val = (mval.class_value + (8 * mval.family_value)) * (65536 * 32) + mval.character_value;
                define(p, xmath_given_cmd, cur_val);
                break;
            default:
                scan_register_num();
                switch (n) {
                case count_def_code:
                    define(p, assign_int_cmd, count_base + cur_val);
                    break;
                case attribute_def_code:
                    define(p, assign_attr_cmd, attribute_base + cur_val);
                    break;
                case dimen_def_code:
                    define(p, assign_dimen_cmd, scaled_base + cur_val);
                    break;
                case skip_def_code:
                    define(p, assign_glue_cmd, skip_base + cur_val);
                    break;
                case mu_skip_def_code:
                    define(p, assign_mu_glue_cmd, mu_skip_base + cur_val);
                    break;
                case toks_def_code:
                    define(p, assign_toks_cmd, toks_base + cur_val);
                    break;
                default:
                    confusion("shorthand_def");
                    break;
                }
                break;
            }
            break;
        case read_to_cs_cmd:
            j = cur_chr;
            scan_int();
            n = cur_val;
            if (!scan_keyword("to")) {
                print_err("Missing `to' inserted");
                help2(
                    "You should have said `\\read<number> to \\cs'.",
                    "I'm going to look for the \\cs now."
                );
                error();
            }
            get_r_token();
            p = cur_cs;
            read_toks(n, p, j);
            define(p, call_cmd, cur_val);
            break;
        case toks_register_cmd:
        case assign_toks_cmd:
            /*tex
                The token-list parameters, \.{\\output} and \.{\\everypar}, etc.,
                receive their values in the following way. (For safety's sake, we
                place an enclosing pair of braces around an \.{\\output} list.)
            */
            q = cur_cs;
            if (cur_cmd == toks_register_cmd) {
                scan_register_num();
                p = toks_base + cur_val;
            } else {
                /*tex |p=every_par_loc| or |output_routine_loc| or \dots */
                p = cur_chr;
            }
            scan_optional_equals();
            /*tex Get the next non-blank non-relax non-call token */
            do {
                get_x_token();
            } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));

            if (cur_cmd != left_brace_cmd) {
                /*tex
                    If the right-hand side is a token parameter or token
                    register, finish the assignment and |goto done|
                */
                if (cur_cmd == toks_register_cmd) {
                    scan_register_num();
                    cur_cmd = assign_toks_cmd;
                    cur_chr = toks_base + cur_val;
                }
                if (cur_cmd == assign_toks_cmd) {
                    q = equiv(cur_chr);
                    if (q == null) {
                        define(p, undefined_cs_cmd, null);
                    } else {
                        add_token_ref(q);
                        define(p, call_cmd, q);
                    }
                    goto DONE;
                }
            }
            back_input();
            cur_cs = q;
            q = scan_toks(false, false);
            if (token_link(def_ref) == null) {      /* empty list: revert to the default */
                define(p, undefined_cs_cmd, null);
                free_avail(def_ref);
            } else {
                if (p == output_routine_loc) {      /* enclose in curlies */
                    p = get_avail();
                    set_token_link(q, p);
                    p = output_routine_loc;
                    q = token_link(q);
                    set_token_info(q, right_brace_token + '}');
                    q = get_avail();
                    set_token_info(q, left_brace_token + '{');
                    set_token_link(q, token_link(def_ref));
                    set_token_link(def_ref, q);
                }
                define(p, call_cmd, def_ref);
            }
            break;
        case assign_int_cmd:
            /*tex Similar routines are used to assign values to the numeric parameters. */
            p = cur_chr;
            scan_optional_equals();
            scan_int();
            assign_internal_value(a, p, cur_val);
            break;
        case assign_attr_cmd:
            p = cur_chr;
            scan_optional_equals();
            scan_int();
            if ((p - attribute_base) > max_used_attr)
                max_used_attr = (p - attribute_base);
            attr_list_cache = cache_disabled;
            word_define(p, cur_val);
            break;
        case assign_direction_cmd:
        case assign_dir_cmd:
            /*tex Assign direction codes. */
            if (cur_cmd == assign_direction_cmd) {
                p = cur_chr;
                scan_optional_equals();
                scan_int();
                check_dir_value(cur_val);
                cur_chr = p;
            } else {
                scan_direction();
            }
            switch (cur_chr) {
                case int_base + page_direction_code:
                    eq_word_define(int_base + page_direction_code, cur_val);
                    break;
                case int_base + body_direction_code:
                    eq_word_define(int_base + body_direction_code, cur_val);
                    break;
                case int_base + par_direction_code:
                    eq_word_define(int_base + par_direction_code, cur_val);
                    break;
                case int_base + math_direction_code:
                    eq_word_define(int_base + math_direction_code, cur_val);
                    break;
                case int_base + text_direction_code:
                case int_base + line_direction_code:
                    /*
                        pre version 0.97 this was a commented section because various tests hint that this
                        is unnecessary and sometimes even produces weird results, like:

                            (\hbox{\textdir TRT ABC\textdir TLT DEF}))

                        becomes

                            (DEFCBA)

                        in the output when we use

                            tail_append(new_dir(text_direction_par)

                        but when we append the reverse of the current it goes better

                    */
                    check_glue = (cur_chr == (int_base + line_direction_code));
                    if (check_glue) {
                        cur_chr = int_base + text_direction_code ;
                    }
                    if (abs(mode) == hmode) {
                        if (no_local_dirs_par > 0) {
                            /*tex |tail| is non zero but we test anyway. */
                            if (check_glue && (tail != null && type(tail) == glue_node))  {
                                halfword prev = alink(tail);
                                halfword dirn = new_dir(text_direction_par);
                                subtype(dirn) = cancel_dir;
                                couple_nodes(prev,dirn);
                                couple_nodes(dirn,tail);
                            } else {
                                tail_append(new_dir(text_direction_par));
                                subtype(tail) = cancel_dir;
                            }
                        } else {
                            /*tex What is the use of nolocaldirs? Maybe we should get rid of it. */
                        }
                        update_text_dir_ptr(cur_val);
                        tail_append(new_dir(cur_val));
                        dir_level(tail) = cur_level;
                    } else {
                        update_text_dir_ptr(cur_val);
                    }
                    eq_word_define(int_base + text_direction_code, cur_val);
                    eq_word_define(int_base + no_local_dirs_code, no_local_dirs_par + 1);
                    break;
                }
            break;
        case assign_dimen_cmd:
            p = cur_chr;
            scan_optional_equals();
            scan_normal_dimen();
            assign_internal_value(a, p, cur_val);
            break;
        case assign_glue_cmd:
        case assign_mu_glue_cmd:
            p = cur_chr;
            n = cur_cmd;
            scan_optional_equals();
            if (n == assign_mu_glue_cmd)
                scan_glue(mu_val_level);
            else
                scan_glue(glue_val_level);
            define(p, glue_ref_cmd, cur_val);
            break;
        case def_char_code_cmd:
        case def_del_code_cmd:
            /*tex Let |n| be the largest legal code value, based on |cur_chr| */
            if (cur_chr == cat_code_base)
                n = max_char_code;
            else if (cur_chr == sf_code_base)
                n = 077777;
            else
                n = biggest_char;
            p = cur_chr;
            if (cur_chr == math_code_base) {
                if (is_global(a))
                    cur_val1 = level_one;
                else
                    cur_val1 = cur_level;
                scan_extdef_math_code(cur_val1, tex_mathcode);
            } else if (cur_chr == lc_code_base) {
                scan_char_num();
                p = cur_val;
                scan_optional_equals();
                scan_int();
                check_def_code(lc_code_base);
                define_lc_code(p, cur_val);
            } else if (cur_chr == uc_code_base) {
                scan_char_num();
                p = cur_val;
                scan_optional_equals();
                scan_int();
                check_def_code(uc_code_base);
                define_uc_code(p, cur_val);
            } else if (cur_chr == sf_code_base) {
                scan_char_num();
                p = cur_val;
                scan_optional_equals();
                scan_int();
                check_def_code(sf_code_base);
                define_sf_code(p, cur_val);
            } else if (cur_chr == cat_code_base) {
                scan_char_num();
                p = cur_val;
                scan_optional_equals();
                scan_int();
                check_def_code(cat_code_base);
                define_cat_code(p, cur_val);
            } else if (cur_chr == del_code_base) {
                if (is_global(a))
                    cur_val1 = level_one;
                else
                    cur_val1 = cur_level;
                scan_extdef_del_code(cur_val1, tex_mathcode);
            }
            break;
        case extdef_math_code_cmd:
        case extdef_del_code_cmd:
            if (is_global(a))
                cur_val1 = level_one;
            else
                cur_val1 = cur_level;
            if (cur_chr == math_code_base)
                scan_extdef_math_code(cur_val1, umath_mathcode);
            else if (cur_chr == math_code_base + 1)
                scan_extdef_math_code(cur_val1, umathnum_mathcode);
            else if (cur_chr == del_code_base)
                scan_extdef_del_code(cur_val1, umath_mathcode);
            else if (cur_chr == del_code_base + 1)
                scan_extdef_del_code(cur_val1, umathnum_mathcode);
            break;
        case def_family_cmd:
            p = cur_chr;
            scan_math_family_int();
            cur_val1 = cur_val;
            scan_optional_equals();
            scan_font_ident();
            define_fam_fnt(cur_val1, p, cur_val);
            break;
        case set_math_param_cmd:
            p = cur_chr;
            get_token();
            if (cur_cmd != math_style_cmd) {
                print_err("Missing math style, treated as \\displaystyle");
                help1(
                    "A style should have been here; I inserted `\\displaystyle'."
                );
                cur_val1 = display_style;
                back_error();
            } else {
                cur_val1 = cur_chr;
            }
            scan_optional_equals();
            if (p < math_param_first_mu_glue) {
                if (p == math_param_radical_degree_raise)
                    scan_int();
                else
                    scan_dimen(false, false, false);
            } else {
                scan_glue(mu_val_level);
                if (cur_val == thin_mu_skip_par)
                    cur_val = thin_mu_skip_code;
                else if (cur_val == med_mu_skip_par)
                    cur_val = med_mu_skip_code;
                else if (cur_val == thick_mu_skip_par)
                    cur_val = thick_mu_skip_code;
            }
            define_math_param(p, cur_val1, cur_val);
            break;
        case register_cmd:
        case advance_cmd:
        case multiply_cmd:
        case divide_cmd:
            do_register_command(a);
            break;
        case set_box_cmd:
            /*tex
                The processing of boxes is somewhat different, because we may
                need to scan and create an entire box before we actually change
                the value of the old one.
            */
            scan_register_num();
            if (is_global(a))
                n = global_box_flag + cur_val;
            else
                n = box_flag + cur_val;
            scan_optional_equals();
            if (set_box_allowed) {
                scan_box(n);
            } else {
                print_err("Improper \\setbox");
                help3(
                    "Sorry, \\setbox is not allowed after \\halign in a display,",
                    "between \\accent and an accented character, or in immediate",
                    "assignments."
                );
                error();
            }
            break;
        case set_aux_cmd:
            /*tex
                The |space_factor| or |prev_depth| settings are changed when a
                |set_aux| command is sensed. Similarly, |prev_graf| is changed in
                the presence of |set_prev_graf|, and |dead_cycles| or
                |insert_penalties| in the presence of |set_page_int|. These
                definitions are always global.
            */
            alter_aux();
            break;
        case set_prev_graf_cmd:
            alter_prev_graf();
            break;
        case set_page_dimen_cmd:
            alter_page_so_far();
            break;
        case set_page_int_cmd:
            alter_integer();
            break;
        case set_box_dimen_cmd:
            /*tex
                When some dimension of a box register is changed, the change
                isn't exactly global; but \TeX\ does not look at the \.{\\global}
                switch.
            */
            alter_box_dimen();
            break;
        case set_tex_shape_cmd:
            q = cur_chr;
            scan_optional_equals();
            scan_int();
            n = cur_val;
            if (n <= 0) {
                p = null;
            } else {
                p = new_node(shape_node, 2 * (n + 1) + 1);
                vinfo(p + 1) = n;
                for (j = 1; j <= n; j++) {
                    scan_normal_dimen();
                    varmem[p + 2 * j].cint = cur_val;       /* indentation */
                    scan_normal_dimen();
                    varmem[p + 2 * j + 1].cint = cur_val;   /* width */
                }
            }
            define(q, shape_ref_cmd, p);
            break;
        case set_etex_shape_cmd:
            q = cur_chr;
            scan_optional_equals();
            scan_int();
            n = cur_val;
            if (n <= 0) {
                p = null;
            } else {
                n = (cur_val / 2) + 1;
                p = new_node(shape_node, 2 * n + 1 + 1);
                vinfo(p + 1) = n;
                n = cur_val;
                varmem[p + 2].cint = n;             /* number of penalties */
                for (j = p + 3; j <= p + n + 2; j++) {
                    scan_int();
                    varmem[j].cint = cur_val;       /* penalty values */
                }
                if (!odd(n))
                    varmem[p + n + 3].cint = 0;     /* unused */
            }
            define(q, shape_ref_cmd, p);
            break;
        case hyph_data_cmd:
            /*tex
                All of \TeX's parameters are kept in |eqtb| except the font
                information, the interaction mode, and the hyphenation tables;
                these are strictly global.
             */
            switch (cur_chr) {
                case 0:
                    new_hyph_exceptions();
                    break;
                case 1:
                    new_patterns();
                    break;
                case 2:
                    new_pre_hyphen_char();
                    break;
                case 3:
                    new_post_hyphen_char();
                    break;
                case 4:
                    new_pre_exhyphen_char();
                    break;
                case 5:
                    new_post_exhyphen_char();
                    break;
                case 6:
                    new_hyphenation_min();
                    break;
                case 7:
                    new_hj_code();
                    break;
            }
            break;
        case assign_font_dimen_cmd:
            set_font_dimen();
            break;
        case assign_font_int_cmd:
            n = cur_chr;
            scan_font_ident();
            f = cur_val;
            if (n == no_lig_code) {
                set_no_ligatures(f);
            } else if (n < lp_code_base) {
                scan_optional_equals();
                scan_int();
                if (n == 0)
                    set_hyphen_char(f, cur_val);
                else
                    set_skew_char(f, cur_val);
            } else {
                scan_char_num();
                p = cur_val;
                scan_optional_equals();
                scan_int();
                switch (n) {
                    case lp_code_base:
                        set_lp_code(f, p, cur_val);
                        break;
                    case rp_code_base:
                        set_rp_code(f, p, cur_val);
                        break;
                    case ef_code_base:
                        set_ef_code(f, p, cur_val);
                        break;
                    case tag_code:
                        set_tag_code(f, p, cur_val);
                        break;
                }
            }
            break;
        case def_font_cmd:
            /*tex Here is where the information for a new font gets loaded. */
            tex_def_font((small_number) a);
            break;
        case def_lua_call_cmd:
            get_r_token();
            p = cur_cs;
            scan_optional_equals();
            scan_int();
            if (j != 0) {
                define(p, lua_call_cmd, cur_val);
            } else {
                define(p, lua_expandable_call_cmd, cur_val);
            }
            break;
        case letterspace_font_cmd:
            new_letterspaced_font((small_number) a);
            break;
        case copy_font_cmd:
            make_font_copy((small_number) a);
            break;
        case set_font_id_cmd:
            scan_int();
            if (is_valid_font(cur_val))
                zset_cur_font(cur_val);
            break ;
        case set_interaction_cmd:
            new_interaction();
            break;
        default:
            confusion("prefix");
            break;
    }
    /*tex End of assignments cases. */
  DONE:
    /*tex Insert a token saved by \.{\\afterassignment}, if any. */
    if (after_token != 0) {
        cur_tok = after_token;
        back_input();
        after_token = 0;
    }
}

void fixup_directions(void)
{
    int temp_no_whatsits = no_local_whatsits_par;
    int temp_no_dirs = no_local_dirs_par;
    int temporary_dir = text_direction_par;
    if (dir_level(text_dir_ptr) == cur_level) {
        /* Remove from |text_dir_ptr|. */
        halfword text_dir_tmp = vlink(text_dir_ptr);
        flush_node(text_dir_ptr);
        text_dir_ptr = text_dir_tmp;
    }
    unsave();
    if (abs(mode) == hmode) {
        if (temp_no_dirs != 0) {
            /* Add local dir node. */
            tail_append(new_dir(text_direction_par));
            dir_dir(tail) = temporary_dir;
            subtype(tail) = cancel_dir;
        }
        if (temp_no_whatsits != 0) {
            /*tex Add local paragraph node. */
            tail_append(make_local_par_node(hmode_par_par_code));
        }
    }
}

/*tex

    This is experimental and needs more checking!

*/

void fixup_directions_only(void)
{
    int temp_no_dirs = no_local_dirs_par;
    int temporary_dir = text_direction_par;
    if (dir_level(text_dir_ptr) == cur_level) {
        /* Remove from |text_dir_ptr|. */
        halfword text_dir_tmp = vlink(text_dir_ptr);
        flush_node(text_dir_ptr);
        text_dir_ptr = text_dir_tmp;
    }
    if (temp_no_dirs != 0) {
        /* Add local dir node. */
        tail_append(new_dir(text_direction_par));
        dir_dir(tail) = temporary_dir;
        subtype(tail) = cancel_dir;
    }
}

/*tex

When a control sequence is to be defined, by \.{\\def} or \.{\\let} or something
similar, the |get_r_token| routine will substitute a special control sequence for
a token that is not redefinable.

*/

void get_r_token(void)
{
  RESTART:
    do {
        get_token();
    } while (cur_tok == space_token);
    if ((cur_cs == 0) || (cur_cs > eqtb_top) ||
        ((cur_cs > frozen_control_sequence) && (cur_cs <= eqtb_size))) {
        print_err("Missing control sequence inserted");
        help5(
            "Please don't say `\\def cs{...}', say `\\def\\cs{...}'.",
            "I've inserted an inaccessible control sequence so that your",
            "definition will be completed without mixing me up too badly.",
            "You can recover graciously from this error, if you're",
            "careful; see exercise 27.2 in The TeXbook."
        );
        if (cur_cs == 0)
            back_input();
        cur_tok = cs_token_flag + frozen_protection;
        ins_error();
        goto RESTART;
    }
}

void assign_internal_value(int a, halfword p, int val)
{
    halfword n;
    if ((p >= int_base) && (p < attribute_base)) {
        switch ((p - int_base)) {
        case cat_code_table_code:
            if (valid_catcode_table(val)) {
                if (val != cat_code_table_par)
                    word_define(p, val);
            } else {
                print_err("Invalid \\catcode table");
                help2(
                    "You can only switch to a \\catcode table that is initialized",
                    "using \\savecatcodetable or \\initcatcodetable, or to table 0"
                );
                error();
            }
            break;
        case output_box_code:
            if ((val > 65535) | (val < 0)) {
                print_err("Invalid \\outputbox");
                help1(
                    "The value for \\outputbox has to be between 0 and 65535."
                );
                error();
            } else {
                word_define(p, val);
            }
            break;
        case new_line_char_code:
            if (val > 127) {
                print_err("Invalid \\newlinechar");
                help2(
                    "The value for \\newlinechar has to be no higher than 127.",
                    "Your invalid assignment will be ignored."
                );
                error();
            } else {
                word_define(p, val);
            }
            break;
        case end_line_char_code:
            if (val > 127) {
                print_err("Invalid \\endlinechar");
                help2(
                    "The value for \\endlinechar has to be no higher than 127.",
                    "Your invalid assignment will be ignored."
                );
                error();
            } else {
                word_define(p, val);
            }
            break;
        case language_code:
            if (val < 0) {
                word_define(int_base + cur_lang_code, -1);
                word_define(p, -1);
            } else if (val > 16383) {
                print_err("Invalid \\language");
                help2(
                    "The absolute value for \\language has to be no higher than 16383.",
                    "Your invalid assignment will be ignored."
                );
                error();
            } else {
                word_define(int_base + cur_lang_code, val);
                word_define(p, val);
            }
            break;
        default:
            word_define(p, val);
            break;
        }
        /*tex
            If we are defining subparagraph penalty levels while we are in hmode,
            then we put out a whatsit immediately, otherwise we leave it alone.
            This mechanism might not be sufficiently powerful, and some other
            algorithm, searching down the stack, might be necessary. Good first
            step.
        */
        if ((abs(mode) == hmode) &&
            ((p == (int_base + local_inter_line_penalty_code)) ||
             (p == (int_base + local_broken_penalty_code)))) {
            /*tex Add local paragraph node */
            tail_append(make_local_par_node(penalty_par_code));
            eq_word_define(int_base + no_local_whatsits_code, no_local_whatsits_par + 1);
        }
    } else if ((p >= dimen_base) && (p <= eqtb_size)) {
        if (p == (dimen_base + page_left_offset_code)) {
            n = val - one_true_inch;
            word_define(dimen_base + h_offset_code, n);
        } else if (p == (dimen_base + h_offset_code)) {
            n = val + one_true_inch;
            word_define(dimen_base + page_left_offset_code, n);
        } else if (p == (dimen_base + page_top_offset_code)) {
            n = val - one_true_inch;
            word_define(dimen_base + v_offset_code, n);
        } else if (p == (dimen_base + v_offset_code)) {
            n = val + one_true_inch;
            word_define(dimen_base + page_top_offset_code, n);
        }
        word_define(p, val);
    } else if ((p >= local_base) && (p < toks_base)) {
        /*tex internal locals */
        define(p, call_cmd, val);
    } else {
        confusion("assign internal value");
    }
}

/*tex

We use the fact that |register| $<$ |advance| $<$ |multiply| $<$ |divide|/ We
compute the register location |l| and its type |p| but |return| if invalid. Here
we use the fact that the consecutive codes |int_val..mu_val| and
|assign_int..assign_mu_glue| correspond to each other nicely.

*/

void do_register_command(int a)
{
    int p;
    halfword q = cur_cmd;
    halfword l = 0;
    if (q != register_cmd) {
        get_x_token();
        if ((cur_cmd >= assign_int_cmd) && (cur_cmd <= assign_mu_glue_cmd)) {
            l = cur_chr;
            p = cur_cmd - assign_int_cmd;
            goto FOUND;
        }
        if (cur_cmd != register_cmd) {
            print_err("You can't use `");
            print_cmd_chr((quarterword) cur_cmd, cur_chr);
            tprint("' after ");
            print_cmd_chr((quarterword) q, 0);
            help1(
                "I'm forgetting what you said and not changing anything."
            );
            error();
            return;
        }
    }
    p = cur_chr;
    scan_register_num();
    if (p == int_val_level)
        l = cur_val + count_base;
    else if (p == attr_val_level)
        l = cur_val + attribute_base;
    else if (p == dimen_val_level)
        l = cur_val + scaled_base;
    else if (p == glue_val_level)
        l = cur_val + skip_base;
    else if (p == mu_val_level)
        l = cur_val + mu_skip_base;
  FOUND:
    if (q == register_cmd) {
        scan_optional_equals();
    } else if (scan_keyword("by")) {
        /*tex optional `\.{by}' */
    }
    arith_error = false;
    if (q < multiply_cmd) {
        /*tex Compute result of |register| or |advance|, put it in |cur_val|. */
        if (p < glue_val_level) {
            if ((p == int_val_level) || (p == attr_val_level))
                scan_int();
            else
                scan_normal_dimen();
            if (q == advance_cmd)
                cur_val = cur_val + eqtb[l].cint;
        } else {
            scan_glue(p);
            if (q == advance_cmd) {
                /* Compute the sum of two glue specs */
                halfword r = equiv(l);
                q = new_spec(cur_val);
                flush_node(cur_val);
                width(q) = width(q) + width(r);
                if (stretch(q) == 0) {
                    stretch_order(q) = normal;
                }
                if (stretch_order(q) == stretch_order(r)) {
                    stretch(q) = stretch(q) + stretch(r);
                } else if ((stretch_order(q) < stretch_order(r)) && (stretch(r) != 0)) {
                    stretch(q) = stretch(r);
                    stretch_order(q) = stretch_order(r);
                }
                if (shrink(q) == 0) {
                    shrink_order(q) = normal;
                }
                if (shrink_order(q) == shrink_order(r)) {
                    shrink(q) = shrink(q) + shrink(r);
                } else if ((shrink_order(q) < shrink_order(r)) && (shrink(r) != 0)) {
                    shrink(q) = shrink(r);
                    shrink_order(q) = shrink_order(r);
                }
                cur_val = q;
            }
        }
    } else {
        /*tex Compute result of |multiply| or |divide|, put it in |cur_val| */
        scan_int();
        if (p < glue_val_level) {
            if (q == multiply_cmd) {
                if ((p == int_val_level) || (p == attr_val_level)) {
                    cur_val = mult_integers(eqtb[l].cint, cur_val);
                } else {
                    cur_val = nx_plus_y(eqtb[l].cint, cur_val, 0);
                }
            } else {
                cur_val = x_over_n(eqtb[l].cint, cur_val);
            }
        } else {
            halfword s = equiv(l);
            halfword r = new_spec(s);
            if (q == multiply_cmd) {
                width(r) = nx_plus_y(width(s), cur_val, 0);
                stretch(r) = nx_plus_y(stretch(s), cur_val, 0);
                shrink(r) = nx_plus_y(shrink(s), cur_val, 0);
            } else {
                width(r) = x_over_n(width(s), cur_val);
                stretch(r) = x_over_n(stretch(s), cur_val);
                shrink(r) = x_over_n(shrink(s), cur_val);
            }
            cur_val = r;
        }
    }
    if (arith_error) {
        print_err("Arithmetic overflow");
        help2(
            "I can't carry out that multiplication or division,",
            "since the result is out of range."
        );
        if (p >= glue_val_level)
            flush_node(cur_val);
        error();
        return;
    }
    if (p < glue_val_level) {
        if (p == attr_val_level) {
            if ((l - attribute_base) > max_used_attr)
                max_used_attr = (l - attribute_base);
            attr_list_cache = cache_disabled;
        }
        if ((p == int_val_level) || (p == dimen_val_level))
            assign_internal_value(a, l, cur_val);
        else
            word_define(l, cur_val);
    } else {
        define(l, glue_ref_cmd, cur_val);
    }
}

void alter_aux(void)
{
    halfword c; /* |hmode| or |vmode| */
    if (cur_chr != abs(mode)) {
        report_illegal_case();
    } else {
        c = cur_chr;
        scan_optional_equals();
        if (c == vmode) {
            scan_normal_dimen();
            prev_depth_par = cur_val;
        } else {
            scan_int();
            if ((cur_val <= 0) || (cur_val > 32767)) {
                print_err("Bad space factor");
                help1(
                    "I allow only values in the range 1..32767 here."
                );
                int_error(cur_val);
            } else {
                space_factor_par = cur_val;
            }
        }
    }
}

void alter_prev_graf(void)
{
    int p = nest_ptr; /* index into |nest| */
    while (abs(nest[p].mode_field) != vmode)
        decr(p);
    scan_optional_equals();
    scan_int();
    if (cur_val < 0) {
        print_err("Bad \\prevgraf");
        help1(
            "I allow only nonnegative values here."
        );
        int_error(cur_val);
    } else {
        nest[p].pg_field = cur_val;
    }
}

void alter_page_so_far(void)
{
    int c = cur_chr;   /* index into |page_so_far| */
    scan_optional_equals();
    scan_normal_dimen();
    page_so_far[c] = cur_val;
}

/*tex
    The value of |c| is 0 for \.{\\deadcycles}, 1 for \.{\\insertpenalties}, etc.
*/

void alter_integer(void)
{
    int c = cur_chr;
    scan_optional_equals();
    scan_int();
    if (c == 0) {
        dead_cycles = cur_val;
    } else if (c == 2) {
        if ((cur_val < batch_mode) || (cur_val > error_stop_mode)) {
            print_err("Bad interaction mode");
            help2(
                "Modes are 0=batch, 1=nonstop, 2=scroll, and",
                "3=errorstop. Proceed, and I'll ignore this case."
            );
            int_error(cur_val);
        } else {
            cur_chr = cur_val;
            new_interaction();
        }
    } else {
        insert_penalties = cur_val;
    }
}

void alter_box_dimen(void)
{
    int c; /* |width_offset| or |height_offset| or |depth_offset| */
    int b; /* box number */
    c = cur_chr;
    scan_register_num();
    b = cur_val;
    scan_optional_equals();
    scan_normal_dimen();
    if (box(b) != null)
        varmem[box(b) + c].cint = cur_val;
}

void new_interaction(void)
{
    print_ln();
    interaction = cur_chr;
    if (interaction == batch_mode)
        kpse_make_tex_discard_errors = 1;
    else
        kpse_make_tex_discard_errors = 0;
    fixup_selector(log_opened_global);
}

/*tex

The \.{\\afterassignment} command puts a token into the global variable
|after_token|. This global variable is examined just after every assignment has
been performed. It's value is zero, or a saved token.

*/

halfword after_token;

/*tex

    Here is a procedure that might be called `Get the next non-blank non-relax
    non-call non-assignment token'.

*/

void do_assignments(void)
{
    while (true) {
        /*tex Get the next non-blank non-relax... */
        do {
            get_x_token();
        } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));
        if (cur_cmd <= max_non_prefixed_command)
            return;
        set_box_allowed = false;
        prefixed_command();
        set_box_allowed = true;
    }
}

/*tex |cur_chr| is 1 for \.{\\openin}, 0 for \.{\\closein}: */

void open_or_close_in(void)
{
    int c = cur_chr;
    int n;
    char *fn;
    scan_four_bit_int();
    n = cur_val;
    if (read_open[n] != closed) {
        lua_a_close_in(read_file[n], (n + 1));
        read_open[n] = closed;
    }
    if (c != 0) {
        scan_optional_equals();
        do {
            get_x_token();
        } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));
        back_input();
        if (cur_cmd != left_brace_cmd) {
            /*tex Set |cur_name| to desired file name. */
            scan_file_name();
            if (cur_ext == get_nullstr())
                cur_ext = maketexstring(".tex");
        } else {
            scan_file_name_toks();
        }
        fn = pack_file_name(cur_name, cur_area, cur_ext);
        if (lua_a_open_in(&(read_file[n]), fn, (n + 1))) {
            read_open[n] = just_open;
        }
    }
}

/*tex
    Has the long \.{\\errmessage} help been used?
*/

boolean long_help_seen;

void issue_message(void)
{
    int old_setting; /* holds |selector| setting */
    int c;           /* identifies \.{\\message} and \.{\\errmessage} */
    str_number s;    /* the message */
    c = cur_chr;
    (void) scan_toks(false, true);
    old_setting = selector;
    selector = new_string;
    token_show(def_ref);
    selector = old_setting;
    flush_list(def_ref);
    str_room(1);
    s = make_string();
    if (c == 0) {
        /*tex Print string |s| on the terminal */
        if (term_offset + (int) str_length(s) > max_print_line - 2)
            print_ln();
        else if ((term_offset > 0) || (file_offset > 0))
            print_char(' ');
        print(s);
        update_terminal();

    } else {
        /*tex
            Print string |s| as an error message. If \.{\\errmessage} occurs
            often in |scroll_mode|, without user-defined \.{\\errhelp}, we don't
            want to give a long help message each time. So we give a verbose
            explanation only once.
        */
        print_err("");
        print(s);
        if (err_help_par != null) {
            use_err_help = true;
        } else if (long_help_seen) {
            help1(
                "(That was another \\errmessage.)"
            );
        } else {
            if (interaction < error_stop_mode)
                long_help_seen = true;
            help4(
                "This error message was generated by an \\errmessage",
                "command, so I can't give any explicit help.",
                "Pretend that you're Hercule Poirot: Examine all clues,",
                "and deduce the truth by order and method."
            );
        }
        error();
        use_err_help = false;

    }
    flush_str(s);
}

/*tex

The |error| routine calls on |give_err_help| if help is requested from the
|err_help| parameter.

*/

void give_err_help(void)
{
    token_show(err_help_par);
}

/*tex

The \.{\\uppercase} and \.{\\lowercase} commands are implemented by building a
token list and then changing the cases of the letters in it.

*/

void shift_case(void)
{
    halfword b;  /* |lc_code_base| or |uc_code_base| */
    halfword p;  /* runs through the token list */
    halfword t;  /* token */
    halfword c;  /* character code */
    halfword i;  /* inbetween */
    b = cur_chr;
    p = scan_toks(false, false);
    p = token_link(def_ref);
    while (p != null) {
        /*tex
            Change the case of the token in |p|, if a change is appropriate. When
            the case of a |chr_code| changes, we don't change the |cmd|. We also
            change active characters.
         */
        t = token_info(p);
        if (t < cs_token_flag) {
            c = t % STRING_OFFSET;
            if (b == uc_code_base)
                i = get_uc_code(c);
            else
                i = get_lc_code(c);
            if (i != 0)
                set_token_info(p, t - c + i);
        } else if (is_active_cs(cs_text(t - cs_token_flag))) {
            c = active_cs_value(cs_text(t - cs_token_flag));
            if (b == uc_code_base)
                i = get_uc_code(c);
            else
                i = get_lc_code(c);
            if (i != 0)
                set_token_info(p, active_to_cs(i, true) + cs_token_flag);
        }
        p = token_link(p);
    }
    back_list(token_link(def_ref));
    free_avail(def_ref);
}

/*tex

We come finally to the last pieces missing from |main_control|, namely the
`\.{\\show}' commands that are useful when debugging.

*/

void show_whatever(void)
{
    halfword p; /* tail of a token list to show */
    int t;      /* type of conditional being shown */
    int m;      /* upper bound on |fi_or_else| codes */
    int l;      /* line where that conditional began */
    int n;      /* level of \.{\\if...\\fi} nesting */
    switch (cur_chr) {
    case show_lists:
        begin_diagnostic();
        show_activities();
        break;
    case show_box_code:
        /*tex Show the current contents of a box. */
        scan_register_num();
        begin_diagnostic();
        tprint_nl("> \\box");
        print_int(cur_val);
        print_char('=');
        if (box(cur_val) == null)
            tprint("void");
        else
            show_box(box(cur_val));
        break;
    case show_code:
        /*tex Show the current meaning of a token, then |goto common_ending|. */
        get_token();
        if (interaction == error_stop_mode)
            wake_up_terminal();
        tprint_nl("> ");
        if (cur_cs != 0) {
            sprint_cs(cur_cs);
            print_char('=');
        }
        print_meaning();
        goto COMMON_ENDING;
        break;
        /*tex Cases for |show_whatever| */
    case show_groups:
        begin_diagnostic();
        show_save_groups();
        break;
    case show_ifs:
        begin_diagnostic();
        tprint_nl("");
        print_ln();
        if (cond_ptr == null) {
            tprint_nl("### ");
            tprint("no active conditionals");
        } else {
            p = cond_ptr;
            n = 0;
            do {
                incr(n);
                p = vlink(p);
            } while (p != null);
            p = cond_ptr;
            t = cur_if;
            l = if_line;
            m = if_limit;
            do {
                tprint_nl("### level ");
                print_int(n);
                tprint(": ");
                print_cmd_chr(if_test_cmd, t);
                if (m == fi_code)
                    tprint_esc("else");
                print_if_line(l);
                decr(n);
                t = if_limit_subtype(p);
                l = if_line_field(p);
                m = if_limit_type(p);
                p = vlink(p);
            } while (p != null);
        }
        break;
    default:
        /*tex
            Show the current value of some parameter or register, then |goto
            common_ending|.
        */
        p = the_toks();
        if (interaction == error_stop_mode)
            wake_up_terminal();
        tprint_nl("> ");
        token_show(temp_token_head);
        flush_list(token_link(temp_token_head));
        goto COMMON_ENDING;
        break;
    }
    /*tex Complete a potentially long \.{\\show} command: */
    end_diagnostic(true);
    print_err("OK");
    if (selector == term_and_log) {
        if (tracing_online_par <= 0) {
            selector = term_only;
            tprint(" (see the transcript file)");
            selector = term_and_log;
        }
    }
  COMMON_ENDING:
    if (interaction < error_stop_mode) {
        help0();
        decr(error_count);
    } else if (tracing_online_par > 0) {
        help3(
            "This isn't an error message; I'm just \\showing something.",
            "Type `I\\show...' to show more (e.g., \\show\\cs,",
            "\\showthe\\count10, \\showbox255, \\showlists)."
        );
    } else {
        help5(
            "This isn't an error message; I'm just \\showing something.",
            "Type `I\\show...' to show more (e.g., \\show\\cs,",
            "\\showthe\\count10, \\showbox255, \\showlists).",
            "And type `I\\tracingonline=1\\show...' to show boxes and",
            "lists on your terminal as well as in the transcript file."
        );
    }
    error();
}

/*tex
    This procedure gets things started properly:
*/

void initialize(void)
{
    int k; /* index into |mem|, |eqtb|, etc. */
    /*tex
        Initialize whatever \TeX\ might access and set initial values of key
        variables
    */
    initialize_errors();
    initialize_arithmetic();
    max_used_attr = -1;
    attr_list_cache = cache_disabled;
    initialize_nesting();
    /*tex Start a new current page: */
    page_contents = empty;
    page_tail = page_head;
#if 0
    vlink(page_head) = null;
#endif
    last_glue = max_halfword;
    last_penalty = 0;
    last_kern = 0;
    last_node_type = -1;
    page_depth = 0;
    page_max_depth = 0;

    initialize_equivalents();
    no_new_control_sequence = true; /* new identifiers are usually forbidden */
    init_primitives();

    mag_set = 0;
    initialize_marks();
    initialize_read();

    static_pdf = init_pdf_struct(static_pdf); /* should be init_backend() */

    format_ident = 0;
    format_name = get_nullstr();
    initialize_directions();
    initialize_write_files();
    seconds_and_micros(epochseconds, microseconds);
    initialize_start_time(static_pdf);

    edit_name_start = 0;
    stop_at_space = true;

    if (ini_version) {
        /*tex Initialize table entries (done by \.{INITEX} only). */
        init_node_mem(500);
        initialize_tokens();
        /*tex Initialize the special list heads and constant nodes. */
        initialize_alignments();
        initialize_buildpage();

        initialize_active();

        set_eq_type(undefined_control_sequence, undefined_cs_cmd);
        set_equiv(undefined_control_sequence, null);
        set_eq_level(undefined_control_sequence, level_zero);
        for (k = null_cs; k <= (eqtb_top - 1); k++)
            eqtb[k] = eqtb[undefined_control_sequence];
        set_equiv(glue_base, zero_glue);
        set_eq_level(glue_base, level_one);
        set_eq_type(glue_base, glue_ref_cmd);
        for (k = glue_base + 1; k <= local_base - 1; k++) {
            eqtb[k] = eqtb[glue_base];
        }
        par_shape_par_ptr = null;
        set_eq_type(par_shape_loc, shape_ref_cmd);
        set_eq_level(par_shape_loc, level_one);
        for (k = etex_pen_base; k <= (etex_pens - 1); k++)
            eqtb[k] = eqtb[par_shape_loc];
        for (k = output_routine_loc; k <= toks_base + biggest_reg; k++)
            eqtb[k] = eqtb[undefined_control_sequence];
        box(0) = null;
        set_eq_type(box_base, box_ref_cmd);
        set_eq_level(box_base, level_one);
        for (k = box_base + 1; k <= (box_base + biggest_reg); k++)
            eqtb[k] = eqtb[box_base];
        cur_font_par = null_font;
        set_eq_type(cur_font_loc, data_cmd);
        set_eq_level(cur_font_loc, level_one);
        set_equiv(cat_code_base, 0);
        set_eq_type(cat_code_base, data_cmd);
        set_eq_level(cat_code_base, level_one);
        eqtb[internal_math_param_base] = eqtb[cat_code_base];
        eqtb[lc_code_base] = eqtb[cat_code_base];
        eqtb[uc_code_base] = eqtb[cat_code_base];
        eqtb[sf_code_base] = eqtb[cat_code_base];
        eqtb[math_code_base] = eqtb[cat_code_base];
        cat_code_table_par = 0;
        initialize_math_codes();
        initialize_text_codes();
        initex_cat_codes(0);
        for (k = '0'; k <= '9'; k++)
            set_math_code(k, math_use_current_family_code, 0, k, level_one);
        for (k = 'A'; k <= 'Z'; k++) {
            set_math_code(k, math_use_current_family_code, 1, k, level_one);
            set_math_code((k + 32), math_use_current_family_code, 1, (k + 32), level_one);
            set_lc_code(k, k + 32, level_one);
            set_lc_code(k + 32, k + 32, level_one);
            set_uc_code(k, k, level_one);
            set_uc_code(k + 32, k, level_one);
            set_sf_code(k, 999, level_one);
        }
        for (k = int_base; k <= attribute_base - 1; k++)
            eqtb[k].cint = 0;
        for (k = attribute_base; k <= del_code_base - 1; k++)
            eqtb[k].cint = UNUSED_ATTRIBUTE;
        mag_par = 1000;
        tolerance_par = 10000;
        hang_after_par = 1;
        max_dead_cycles_par = 25;
        math_pre_display_gap_factor_par = 2000;
        pre_bin_op_penalty_par = inf_penalty;
        math_script_box_mode_par = 1;
        math_script_char_mode_par = 1;
        pre_rel_penalty_par = inf_penalty;
        compound_hyphen_mode_par = 1;
        escape_char_par = '\\';
        end_line_char_par = carriage_return;
        set_del_code('.', 0, 0, 0, 0, level_one); /* this null delimiter is used in error recovery */
        ex_hyphen_char_par = '-';
        output_box_par = 255;
        for (k = dimen_base; k <= eqtb_size; k++)
            eqtb[k].cint = 0;
        page_left_offset_par = one_inch;
        page_top_offset_par = one_inch;
        page_right_offset_par = one_inch;
        page_bottom_offset_par = one_inch;
        ini_init_primitives();
        hash_used = frozen_control_sequence;
        hash_high = 0;
        cs_count = 0;
        set_eq_type(frozen_dont_expand, dont_expand_cmd);
        cs_text(frozen_dont_expand) = maketexstring("notexpanded:");
        set_eq_type(frozen_primitive, ignore_spaces_cmd);
        set_equiv(frozen_primitive, 1);
        set_eq_level(frozen_primitive, level_one);
        cs_text(frozen_primitive) = maketexstring("primitive");
        create_null_font();
        font_bytes = 0;
        px_dimen_par = one_bp;
        math_eqno_gap_step_par = 1000 ;
        math_flatten_mode_par = 1; /* ord */
        cs_text(frozen_protection) = maketexstring("inaccessible");
        format_ident = maketexstring(" (INITEX)");
        cs_text(end_write) = maketexstring("endwrite");
        set_eq_level(end_write, level_one);
        set_eq_type(end_write, outer_call_cmd);
        set_equiv(end_write, null);
        /*tex Bah, this is a bad place do do this. */
        set_pdf_major_version(1);
        set_pdf_minor_version(0);
    }
    synctexoffset = int_base + synctex_code;
}
