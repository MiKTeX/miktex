% scanning.w
%
% Copyright 2009-2012 Taco Hoekwater <taco@@luatex.org>
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
#define prev_depth cur_list.prev_depth_field
#define space_factor cur_list.space_factor_field
#define par_shape_ptr  equiv(par_shape_loc)
#define font_id_text(A) cs_text(font_id_base+(A))

#define attribute(A) eqtb[attribute_base+(A)].hh.rh
#define dimen(A) eqtb[scaled_base+(A)].hh.rh
#undef skip
#define skip(A) eqtb[skip_base+(A)].hh.rh
#define mu_skip(A) eqtb[mu_skip_base+(A)].hh.rh
#define count(A) eqtb[count_base+(A)].hh.rh
#define box(A) equiv(box_base+(A))

static void scan_expr(void);

@ Let's turn now to some procedures that \TeX\ calls upon frequently to digest
certain kinds of patterns in the input. Most of these are quite simple;
some are quite elaborate. Almost all of the routines call |get_x_token|,
which can cause them to be invoked recursively.

The |scan_left_brace| routine is called when a left brace is supposed to be
the next non-blank token. (The term ``left brace'' means, more precisely,
a character whose catcode is |left_brace|.) \TeX\ allows \.{\\relax} to
appear before the |left_brace|.

@c
void scan_left_brace(void)
{                               /* reads a mandatory |left_brace| */
    /* Get the next non-blank non-relax non-call token */
    do {
        get_x_token();
    } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));

    if (cur_cmd != left_brace_cmd) {
        print_err("Missing { inserted");
        help4("A left brace was mandatory here, so I've put one in.",
              "You might want to delete and/or insert some corrections",
              "so that I will find a matching right brace soon.",
              "If you're confused by all this, try typing `I}' now.");
        back_error();
        cur_tok = left_brace_token + '{';
        cur_cmd = left_brace_cmd;
        cur_chr = '{';
        incr(align_state);
    }
}

@ The |scan_optional_equals| routine looks for an optional `\.=' sign preceded
by optional spaces; `\.{\\relax}' is not ignored here.

@c
void scan_optional_equals(void)
{
    /* Get the next non-blank non-call token */
    do {
        get_x_token();
    } while (cur_cmd == spacer_cmd);
    if (cur_tok != other_token + '=')
        back_input();
}

@ Here is a procedure that sounds an alarm when mu and non-mu units
are being switched.

@c
static void mu_error(void)
{
    print_err("Incompatible glue units");
    help1("I'm going to assume that 1mu=1pt when they're mixed.");
    error();
}

@ The next routine `|scan_something_internal|' is used to fetch internal
numeric quantities like `\.{\\hsize}', and also to handle the `\.{\\the}'
when expanding constructions like `\.{\\the\\toks0}' and
`\.{\\the\\baselineskip}'. Soon we will be considering the |scan_int|
procedure, which calls |scan_something_internal|; on the other hand,
|scan_something_internal| also calls |scan_int|, for constructions like
`\.{\\catcode\`\\\$}' or `\.{\\fontdimen} \.3 \.{\\ff}'. So we
have to declare |scan_int| as a |forward| procedure. A few other
procedures are also declared at this point.

\TeX\ doesn't know exactly what to expect when
|scan_something_internal| begins.  For example, an integer or
dimension or glue value could occur immediately after `\.{\\hskip}';
and one can even say \.{\\the} with respect to token lists in
constructions like `\.{\\xdef\\o\{\\the\\output\}}'.  On the other
hand, only integers are allowed after a construction like
`\.{\\count}'. To handle the various possibilities,
|scan_something_internal| has a |level| parameter, which tells the
``highest'' kind of quantity that |scan_something_internal| is allowed
to produce. Eight levels are distinguished, namely |int_val|,
|attr_val|, |dimen_val|, |glue_val|, |mu_val|, |dir_val|, |ident_val|,
and |tok_val|.

The output of |scan_something_internal| (and of the other routines
|scan_int|, |scan_dimen|, and |scan_glue| below) is put into the global
variable |cur_val|, and its level is put into |cur_val_level|. The highest
values of |cur_val_level| are special: |mu_val| is used only when
|cur_val| points to something in a ``muskip'' register, or to one of the
three parameters \.{\\thinmuskip}, \.{\\medmuskip}, \.{\\thickmuskip};
|ident_val| is used only when |cur_val| points to a font identifier;
|tok_val| is used only when |cur_val| points to |null| or to the reference
count of a token list. The last two cases are allowed only when
|scan_something_internal| is called with |level=tok_val|.

If the output is glue, |cur_val| will point to a glue specification, and
the reference count of that glue will have been updated to reflect this
reference; if the output is a nonempty token list, |cur_val| will point to
its reference count, but in this case the count will not have been updated.
Otherwise |cur_val| will contain the integer or scaled value in question.

@c
int cur_val;                    /* value returned by numeric scanners */
int cur_val1;                   /* delcodes are sometimes 51 digits */
int cur_val_level;              /* the ``level'' of this value */

#define scanned_result(A,B) do { \
    cur_val=A; \
    cur_val_level=B; \
} while (0)

@ When a |glue_val| changes to a |dimen_val|, we use the width component
of the glue; there is no need to decrease the reference count, since it
has not yet been increased.  When a |dimen_val| changes to an |int_val|,
we use scaled points so that the value doesn't actually change. And when a
|mu_val| changes to a |glue_val|, the value doesn't change either.

@c
static void downgrade_cur_val(boolean delete_glue)
{
    halfword m;
    if (cur_val_level == glue_val_level) {
        m = cur_val;
        cur_val = width(m);
        if (delete_glue)
            flush_node(m);
    } else if (cur_val_level == mu_val_level) {
        mu_error();
    }
    decr(cur_val_level);
}

static void negate_cur_val(boolean delete_glue)
{
    halfword m;
    if (cur_val_level >= glue_val_level) {
        m = cur_val;
        cur_val = new_spec(m);
        if (delete_glue)
            flush_node(m);
        /* Negate all three glue components of |cur_val| */
        negate(width(cur_val));
        negate(stretch(cur_val));
        negate(shrink(cur_val));

    } else {
        negate(cur_val);
    }
}

@ Some of the internal items can be fetched both routines,
and these have been split off into the next routine, that
returns true if the command code was understood

@c
static boolean short_scan_something_internal(int cmd, int chr, int level,
                                             boolean negative)
{
    halfword m;                 /* |chr_code| part of the operand token */
    halfword q;                 /* general purpose index */
    int p;                      /* index into |nest| */
    int save_cur_chr;
    boolean succeeded = true;
    m = chr;
    switch (cmd) {
    case assign_toks_cmd:
        scanned_result(equiv(m), tok_val_level);
        break;
    case assign_int_cmd:
        scanned_result(eqtb[m].cint, int_val_level);
        break;
    case assign_attr_cmd:
        scanned_result(eqtb[m].cint, int_val_level);
        break;
    case assign_dir_cmd:
        scanned_result(eqtb[m].cint, dir_val_level);
        break;
    case assign_dimen_cmd:
        scanned_result(eqtb[m].cint, dimen_val_level);
        break;
    case assign_glue_cmd:
        scanned_result(equiv(m), glue_val_level);
// cur_val = new_spec(cur_val);
        break;
    case assign_mu_glue_cmd:
        scanned_result(equiv(m), mu_val_level);
// cur_val = new_spec(cur_val);
        break;
    case math_style_cmd:
        scanned_result(m, int_val_level);
        break;
    case set_aux_cmd:
        /* Fetch the |space_factor| or the |prev_depth| */
        if (abs(cur_list.mode_field) != m) {
            print_err("Improper ");
            print_cmd_chr(set_aux_cmd, m);
            help4("You can refer to \\spacefactor only in horizontal mode;",
                  "you can refer to \\prevdepth only in vertical mode; and",
                  "neither of these is meaningful inside \\write. So",
                  "I'm forgetting what you said and using zero instead.");
            error();
            if (level != tok_val_level)
                scanned_result(0, dimen_val_level);
            else
                scanned_result(0, int_val_level);
        } else if (m == vmode) {
            scanned_result(prev_depth, dimen_val_level);
        } else {
            scanned_result(space_factor, int_val_level);
        }
        break;
    case set_prev_graf_cmd:
        /* Fetch the |prev_graf| */
        if (cur_list.mode_field == 0) {
            scanned_result(0, int_val_level);   /* |prev_graf=0| within \.{\\write} */
        } else {
            p = nest_ptr;
            while (abs(nest[p].mode_field) != vmode)
                decr(p);
            scanned_result(nest[p].pg_field, int_val_level);
        }
        break;
    case set_page_int_cmd:
        /* Fetch the |dead_cycles| or the |insert_penalties| */
        if (m == 0)
            cur_val = dead_cycles;
        else if (m == 2)
            cur_val = interaction;      /* interactionmode */
        else
            cur_val = insert_penalties;
        cur_val_level = int_val_level;
        break;
    case set_page_dimen_cmd:
        /* Fetch something on the |page_so_far| */
        if ((page_contents == empty) && (!output_active)) {
            if (m == 0)
                cur_val = max_dimen;
            else
                cur_val = 0;
        } else {
            cur_val = page_so_far[m];
        }
        cur_val_level = dimen_val_level;
        break;
    case set_tex_shape_cmd:
        /* Fetch the |par_shape| size */
        if (par_shape_ptr == null)
            cur_val = 0;
        else
            cur_val = vinfo(par_shape_ptr + 1);
        cur_val_level = int_val_level;
        break;
    case set_etex_shape_cmd:
        /* Fetch a penalties array element */
        scan_int();
        if ((equiv(m) == null) || (cur_val < 0)) {
            cur_val = 0;
        } else {
            if (cur_val > penalty(equiv(m)))
                cur_val = penalty(equiv(m));
            cur_val = penalty(equiv(m) + cur_val);
        }
        cur_val_level = int_val_level;
        break;
    case char_given_cmd:
    case math_given_cmd:
    case xmath_given_cmd:
        scanned_result(cur_chr, int_val_level);
        break;
    case last_item_cmd:
        /* Because the items in this case directly refer to |cur_chr|,
           it needs to be saved and restored */
        save_cur_chr = cur_chr;
        cur_chr = chr;
        /* Fetch an item in the current node, if appropriate */
        /* Here is where \.{\\lastpenalty}, \.{\\lastkern}, and \.{\\lastskip} are
           implemented. The reference count for \.{\\lastskip} will be updated later.

           We also handle \.{\\inputlineno} and \.{\\badness} here, because they are
           legal in similar contexts. */

        if (m >= input_line_no_code) {
            if (m >= eTeX_glue) {
                /* Process an expression and |return| */
                if (m < eTeX_mu) {
                    switch (m) {
                    case mu_to_glue_code:
                        scan_mu_glue();
                        break;
                    };          /* there are no other cases */
                    cur_val_level = glue_val_level;
                } else if (m < eTeX_expr) {
                    switch (m) {
                    case glue_to_mu_code:
                        scan_normal_glue();
                        break;
                    }           /* there are no other cases */
                    cur_val_level = mu_val_level;
                } else {
                    cur_val_level = m - eTeX_expr + int_val_level;
                    scan_expr();
                }
                /* This code for reducing |cur_val_level| and\slash or negating the
                   result is similar to the one for all the other cases of
                   |scan_something_internal|, with the difference that |scan_expr| has
                   already increased the reference count of a glue specification.
                 */
                while (cur_val_level > level) {
                    downgrade_cur_val(true);
                }
                if (negative) {
                    negate_cur_val(true);
                }
                return succeeded;

            } else if (m >= eTeX_dim) {
                switch (m) {
                case font_char_wd_code:
                case font_char_ht_code:
                case font_char_dp_code:
                case font_char_ic_code:
                    scan_font_ident();
                    q = cur_val;
                    scan_char_num();
                    if (char_exists(q, cur_val)) {
                        switch (m) {
                        case font_char_wd_code:
                            cur_val = char_width(q, cur_val);
                            break;
                        case font_char_ht_code:
                            cur_val = char_height(q, cur_val);
                            break;
                        case font_char_dp_code:
                            cur_val = char_depth(q, cur_val);
                            break;
                        case font_char_ic_code:
                            cur_val = char_italic(q, cur_val);
                            break;
                        }       /* there are no other cases */
                    } else {
                        cur_val = 0;
                    }
                    break;
                case par_shape_length_code:
                case par_shape_indent_code:
                case par_shape_dimen_code:
                    q = cur_chr - par_shape_length_code;
                    scan_int();
                    if ((par_shape_ptr == null) || (cur_val <= 0)) {
                        cur_val = 0;
                    } else {
                        if (q == 2) {
                            q = cur_val % 2;
                            cur_val = (cur_val + q) / 2;
                        }
                        if (cur_val > vinfo(par_shape_ptr + 1))
                            cur_val = vinfo(par_shape_ptr + 1);
                        cur_val =
                            varmem[par_shape_ptr + 2 * cur_val - q + 1].cint;
                    }
                    cur_val_level = dimen_val_level;
                    break;
                case glue_stretch_code:
                case glue_shrink_code:
                    scan_normal_glue();
                    q = cur_val;
                    if (m == glue_stretch_code)
                        cur_val = stretch(q);
                    else
                        cur_val = shrink(q);
                    flush_node(q);
                    break;
                }               /* there are no other cases */
                cur_val_level = dimen_val_level;
            } else {
                switch (m) {
                case input_line_no_code:
                    cur_val = line;
                    break;
                case badness_code:
                    cur_val = last_badness;
                    break;
                case luatex_version_code:
                    cur_val = get_luatexversion();
                    break;
                case last_saved_box_resource_index_code:
                    cur_val = last_saved_box_index;
                    break;
                case last_saved_image_resource_index_code:
                    cur_val = last_saved_image_index;
                    break;
                case last_saved_image_resource_pages_code:
                    cur_val = last_saved_image_pages;
                    break;
                case last_x_pos_code:
                    cur_val = last_position.h;
                    break;
                case last_y_pos_code:
                    cur_val = last_position.v;
                    break;
                case random_seed_code:
                    cur_val = random_seed;
                    break;
                case eTeX_version_code:
                    cur_val = eTeX_version;
                    break;
                case eTeX_minor_version_code:
                    cur_val = eTeX_minor_version;
                    break;
                case current_group_level_code:
                    cur_val = cur_level - level_one;
                    break;
                case current_group_type_code:
                    cur_val = cur_group;
                    break;
                case current_if_level_code:
                    q = cond_ptr;
                    cur_val = 0;
                    while (q != null) {
                        incr(cur_val);
                        q = vlink(q);
                    }
                    break;
                case current_if_type_code:
                    if (cond_ptr == null)
                        cur_val = 0;
                    else if (cur_if < unless_code)
                        cur_val = cur_if + 1;
                    else
                        cur_val = -(cur_if - unless_code + 1);
                    break;
                case current_if_branch_code:
                    if ((if_limit == or_code) || (if_limit == else_code))
                        cur_val = 1;
                    else if (if_limit == fi_code)
                        cur_val = -1;
                    else
                        cur_val = 0;
                    break;
                case glue_stretch_order_code:
                case glue_shrink_order_code:
                    scan_normal_glue();
                    q = cur_val;
                    if (m == glue_stretch_order_code)
                        cur_val = stretch_order(q);
                    else
                        cur_val = shrink_order(q);
                    flush_node(q);
                    break;
                }               /* there are no other cases */
                cur_val_level = int_val_level;
            }
        } else {
            if (cur_chr == glue_val_level)
                cur_val = zero_glue;
            else
                cur_val = 0;
            if (cur_chr == last_node_type_code) {
                cur_val_level = int_val_level;
                if ((cur_list.tail_field == cur_list.head_field)
                    || (cur_list.mode_field == 0))
                    cur_val = -1;
            } else {
                cur_val_level = cur_chr;        /* assumes identical values */
            }
            if ((cur_list.tail_field != contrib_head) &&
                !is_char_node(cur_list.tail_field) &&
                (cur_list.mode_field != 0)) {
                switch (cur_chr) {
                case lastpenalty_code:
                    if (type(cur_list.tail_field) == penalty_node)
                        cur_val = penalty(cur_list.tail_field);
                    break;
                case lastkern_code:
                    if (type(cur_list.tail_field) == kern_node)
                        cur_val = width(cur_list.tail_field);
                    break;
                case lastskip_code:
                    if (type(cur_list.tail_field) == glue_node)
                        cur_val = new_glue(cur_list.tail_field);
                    if (subtype(cur_list.tail_field) == mu_glue)
                        cur_val_level = mu_val_level;
                    break;
                case last_node_type_code:
                    cur_val = visible_last_node_type(cur_list.tail_field);
                    break;
                }               /* there are no other cases */
            } else if ((cur_list.mode_field == vmode)
                       && (cur_list.tail_field == cur_list.head_field)) {
                switch (cur_chr) {
                case lastpenalty_code:
                    cur_val = last_penalty;
                    break;
                case lastkern_code:
                    cur_val = last_kern;
                    break;
                case lastskip_code:
                    if (last_glue != max_halfword)
                        cur_val = last_glue; /* maybe new_glue */ 
                    break;
                case last_node_type_code:
                    cur_val = last_node_type;
                    break;
                }               /* there are no other cases */
            }
        }
        cur_chr = save_cur_chr;
        break;
    default:
        succeeded = false;
    }
    if (succeeded) {
        while (cur_val_level > level) {
            /* Convert |cur_val| to a lower level */
            downgrade_cur_val(false);
        }
        /* Fix the reference count, if any, and negate |cur_val| if |negative| */
        /* If |cur_val| points to a glue specification at this point, the reference
           count for the glue does not yet include the reference by |cur_val|.
           If |negative| is |true|, |cur_val_level| is known to be |<=mu_val|.
         */
        if (negative) {
            negate_cur_val(false);
        } else if ((cur_val_level >= glue_val_level) && (cur_val_level <= mu_val_level)) {
			cur_val = new_glue(cur_val);
        }
    }
    return succeeded;
}

@ First, here is a short routine that is called from lua code. All
the  real work is delegated to |short_scan_something_internal| that
is shared between this routine and |scan_something_internal|.

@c
void scan_something_simple(halfword cmd, halfword subitem)
{
    /* negative is never true */
    if (!short_scan_something_internal(cmd, subitem, tok_val_level, false)) {
        /* Complain that |texlib| can not do this; give zero result */
        print_err("You can't use `");
        print_cmd_chr((quarterword) cmd, subitem);
        tprint("' as tex library index");
        help1("I'm forgetting what you said and using zero instead.");
        error();
        scanned_result(0, int_val_level);
    }
}

@ OK, we're ready for |scan_something_internal| itself. A second parameter,
|negative|, is set |true| if the value that is found should be negated.
It is assumed that |cur_cmd| and |cur_chr| represent the first token of
the internal quantity to be scanned; an error will be signalled if
|cur_cmd<min_internal| or |cur_cmd>max_internal|.

@c
void scan_something_internal(int level, boolean negative)
{
    /* fetch an internal parameter */
    halfword m;                 /* |chr_code| part of the operand token */
    int n, k;                   /* accumulators */
  RESTART:
    m = cur_chr;
    if (!short_scan_something_internal(cur_cmd, cur_chr, level, negative)) {
        switch (cur_cmd) {
        case def_char_code_cmd:
            /* Fetch a character code from some table */
            scan_char_num();
            if (m == math_code_base) {
                cur_val1 = get_math_code_num(cur_val);
                scanned_result(cur_val1, int_val_level);
            } else if (m == lc_code_base) {
                cur_val1 = get_lc_code(cur_val);
                scanned_result(cur_val1, int_val_level);
            } else if (m == uc_code_base) {
                cur_val1 = get_uc_code(cur_val);
                scanned_result(cur_val1, int_val_level);
            } else if (m == sf_code_base) {
                cur_val1 = get_sf_code(cur_val);
                scanned_result(cur_val1, int_val_level);
            } else if (m == cat_code_base) {
                cur_val1 = get_cat_code(int_par(cat_code_table_code), cur_val);
                scanned_result(cur_val1, int_val_level);
            } else {
                confusion("def_char");
            }
            break;
        case def_del_code_cmd:
        case extdef_del_code_cmd: /* bonus */
            /* Fetch a character code from some table */
            scan_char_num();
            cur_val1 = get_del_code_num(cur_val);
            scanned_result(cur_val1, int_val_level);
            break;
        case extdef_math_code_cmd:
            /* Fetch an extended math code table value */
            scan_char_num();
            cur_val1 = get_math_code_num(cur_val);
            scanned_result(cur_val1, int_val_level);
            break;
        case toks_register_cmd:
        case set_font_cmd:
        case def_font_cmd:
        case letterspace_font_cmd:
        case copy_font_cmd:
            /* Fetch a token list or font identifier, provided that |level=tok_val| */
            if (level != tok_val_level) {
                print_err("Missing number, treated as zero");
                help3("A number should have been here; I inserted `0'.",
                      "(If you can't figure out why I needed to see a number,",
                      "look up `weird error' in the index to The TeXbook.)");
                back_error();
                scanned_result(0, dimen_val_level);
            } else if (cur_cmd == toks_register_cmd) {
                scan_register_num();
                m = toks_base + cur_val;
                scanned_result(equiv(m), tok_val_level);
            } else {
                back_input();
                scan_font_ident();
                scanned_result(font_id_base + cur_val, ident_val_level);
            }
            break;
        case set_font_id_cmd:
            scan_int();
            scanned_result(font_id_base + cur_val, ident_val_level);
            break;
        case def_family_cmd:
            /* Fetch a math font identifier */
            scan_char_num();
            cur_val1 = fam_fnt(cur_val, m);
            scanned_result(font_id_base + cur_val1, ident_val_level);
            break;
        case set_math_param_cmd:
            /* Fetch a math param */
            cur_val1 = cur_chr;
            get_token();
            if (cur_cmd != math_style_cmd) {
                print_err("Missing math style, treated as \\displaystyle");
                help1
                    ("A style should have been here; I inserted `\\displaystyle'.");
                cur_val = display_style;
                back_error();
            } else {
                cur_val = cur_chr;
            }
            if (cur_val1 < math_param_first_mu_glue) {
                if (cur_val1 == math_param_radical_degree_raise) {
                    cur_val1 = get_math_param(cur_val1, cur_chr);
                    scanned_result(cur_val1, int_val_level);
                } else {
                    cur_val1 = get_math_param(cur_val1, cur_chr);
                    scanned_result(cur_val1, dimen_val_level);
                }
            } else {
                cur_val1 = get_math_param(cur_val1, cur_chr);
                if (cur_val1 == thin_mu_skip_code)
                    cur_val1 = glue_par(thin_mu_skip_code);
                else if (cur_val1 == med_mu_skip_code)
                    cur_val1 = glue_par(med_mu_skip_code);
                else if (cur_val1 == thick_mu_skip_code)
                    cur_val1 = glue_par(thick_mu_skip_code);
                scanned_result(cur_val1, mu_val_level);
            }
            break;
        case assign_box_dir_cmd:
            scan_register_num();
            m = cur_val;
            if (box(m) != null)
                cur_val = box_dir(box(m));
            else
                cur_val = 0;
            cur_val_level = dir_val_level;
            break;
        case set_box_dimen_cmd:
            /* Fetch a box dimension */
            scan_register_num();
            if (box(cur_val) == null)
                cur_val = 0;
            else
                cur_val = varmem[box(cur_val) + m].cint;
            cur_val_level = dimen_val_level;
            break;
        case assign_font_dimen_cmd:
            /* Fetch a font dimension */
            get_font_dimen();
            break;
        case assign_font_int_cmd:
            /* Fetch a font integer */
            scan_font_ident();
            if (m == 0) {
                scanned_result(hyphen_char(cur_val), int_val_level);
            } else if (m == 1) {
                scanned_result(skew_char(cur_val), int_val_level);
            } else if (m == no_lig_code) {
                scanned_result(test_no_ligatures(cur_val), int_val_level);
            } else {
                n = cur_val;
                scan_char_num();
                k = cur_val;
                switch (m) {
                case lp_code_base:
                    scanned_result(get_lp_code(n, k), int_val_level);
                    break;
                case rp_code_base:
                    scanned_result(get_rp_code(n, k), int_val_level);
                    break;
                case ef_code_base:
                    scanned_result(get_ef_code(n, k), int_val_level);
                    break;
                case tag_code:
                    scanned_result(get_tag_code(n, k), int_val_level);
                    break;
                }
            }
            break;
        case register_cmd:
            /* Fetch a register */
            scan_register_num();
            switch (m) {
            case int_val_level:
                cur_val = count(cur_val);
                break;
            case attr_val_level:
                cur_val = attribute(cur_val);
                break;
            case dimen_val_level:
                cur_val = dimen(cur_val);
                break;
            case glue_val_level:
                cur_val = skip(cur_val);
                break;
            case mu_val_level:
                cur_val = mu_skip(cur_val);
                break;
            }                   /* there are no other cases */
            cur_val_level = m;
            break;
        case ignore_spaces_cmd:        /* trap unexpandable primitives */
            if (cur_chr == 1) {
                /* Reset |cur_tok| for unexpandable primitives, goto restart */
                /* This block deals with unexpandable \.{\\primitive} appearing at a spot where
                   an integer or an internal values should have been found. It fetches the
                   next token then resets |cur_cmd|, |cur_cs|, and |cur_tok|, based on the
                   primitive value of that token. No expansion takes place, because the
                   next token may be all sorts of things. This could trigger further
                   expansion creating new errors.
                 */
                get_token();
                cur_cs = prim_lookup(cs_text(cur_cs));
                if (cur_cs != undefined_primitive) {
                    cur_cmd = get_prim_eq_type(cur_cs);
                    cur_chr = get_prim_equiv(cur_cs);
                    cur_tok = token_val(cur_cmd, cur_chr);
                } else {
                    cur_cmd = relax_cmd;
                    cur_chr = 0;
                    cur_tok = cs_token_flag + frozen_relax;
                    cur_cs = frozen_relax;
                }
                goto RESTART;
            }
            break;
        case hyph_data_cmd:
            switch (cur_chr) {
                case 0:
                case 1:
                    goto DEFAULT;
                    break;
                case 2:
                    cur_val = get_pre_hyphen_char(int_par(language_code));
                    cur_val_level = int_val_level;
                    break;
                case 3:
                    cur_val = get_post_hyphen_char(int_par(language_code));
                    cur_val_level = int_val_level;
                    break;
                case 4:
                    cur_val = get_pre_exhyphen_char(int_par(language_code));
                    cur_val_level = int_val_level;
                    break;
                case 5:
                    cur_val = get_post_exhyphen_char(int_par(language_code));
                    cur_val_level = int_val_level;
                    break;
                case 6:
                    cur_val = get_hyphenation_min(int_par(language_code));
                    cur_val_level = int_val_level;
                    break;
                case 7:
                    scan_int();
                    cur_val = get_hj_code(int_par(language_code),cur_val);
                    cur_val_level = int_val_level;
                    break;
            }
            break;
        default:
            DEFAULT:
            /* Complain that \.{\\the} can not do this; give zero result */
            print_err("You can't use `");
            print_cmd_chr((quarterword) cur_cmd, cur_chr);
            tprint("' after \\the");
            help1("I'm forgetting what you said and using zero instead.");
            error();
            if (level != tok_val_level)
                scanned_result(0, dimen_val_level);
            else
                scanned_result(0, int_val_level);
            break;
        }
        while (cur_val_level > level) {
            /* Convert |cur_val| to a lower level */
            downgrade_cur_val(false);
        }
        /* Fix the reference count, if any, and negate |cur_val| if |negative| */
        /* If |cur_val| points to a glue specification at this point, the reference
           count for the glue does not yet include the reference by |cur_val|.
           If |negative| is |true|, |cur_val_level| is known to be |<=mu_val|.
         */
        if (negative) {
            negate_cur_val(false);
        } else if ((cur_val_level >= glue_val_level) && (cur_val_level <= mu_val_level)) {
			cur_val = new_glue(cur_val);
        }
    }
}

@ It is nice to have routines that say what they do, so the original
|scan_eight_bit_int| is superceded by |scan_register_num| and
|scan_mark_num|. It may become split up even further in the future.

Many of the |restricted classes| routines are the essentially
the same except for the upper limit and the error message, so it makes
sense to combine these all into one function.

@c
void scan_limited_int(int max, const char *name)
{
    char hlp[80];
    scan_int();
    if ((cur_val < 0) || (cur_val > max)) {
        if (name == NULL) {
            snprintf(hlp, 80,
                     "Since I expected to read a number between 0 and %d,",
                     max);
            print_err("Bad number");
        } else {
            char msg[80];
            snprintf(hlp, 80, "A %s must be between 0 and %d.", name, max);
            snprintf(msg, 80, "Bad %s", name);
            print_err(msg);
        }
        help2(hlp, "I changed this one to zero.");
        int_error(cur_val);
        cur_val = 0;
    }
}

@ @c
void scan_fifteen_bit_int(void)
{
    scan_real_fifteen_bit_int();
    cur_val = ((cur_val / 0x1000) * 0x1000000) +
        (((cur_val % 0x1000) / 0x100) * 0x10000) + (cur_val % 0x100);
}

@ @c
void scan_fifty_one_bit_int(void)
{
    int iiii;
    scan_int();
    if ((cur_val < 0) || (cur_val > 0777777777)) {
        print_err("Bad delimiter code");
        help2
            ("A numeric delimiter (first part) must be between 0 and 2^{27}-1.",
             "I changed this one to zero.");
        int_error(cur_val);
        cur_val = 0;
    }
    iiii = cur_val;
    scan_int();
    if ((cur_val < 0) || (cur_val > 0xFFFFFF)) {
        print_err("Bad delimiter code");
        help2
            ("A numeric delimiter (second part) must be between 0 and 2^{24}-1.",
             "I changed this one to zero.");
        int_error(cur_val);
        cur_val = 0;
    }
    cur_val1 = cur_val;
    cur_val = iiii;
}

@ An integer number can be preceded by any number of spaces and `\.+' or
`\.-' signs. Then comes either a decimal constant (i.e., radix 10), an
octal constant (i.e., radix 8, preceded by~'), a hexadecimal constant
(radix 16, preceded by~"), an alphabetic constant (preceded by~`), or
an internal variable. After scanning is complete,
|cur_val| will contain the answer, which must be at most
$2^{31}-1=2147483647$ in absolute value. The value of |radix| is set to
10, 8, or 16 in the cases of decimal, octal, or hexadecimal constants,
otherwise |radix| is set to zero. An optional space follows a constant.

@c
int radix;                      /* |scan_int| sets this to 8, 10, 16, or zero */

@ The |scan_int| routine is used also to scan the integer part of a
  fraction; for example, the `\.3' in `\.{3.14159}' will be found by
  |scan_int|. The |scan_dimen| routine assumes that |cur_tok=point_token|
  after the integer part of such a fraction has been scanned by |scan_int|,
  and that the decimal point has been backed up to be scanned again.

@c
void scan_int(void)
{                               /* sets |cur_val| to an integer */
    boolean negative;           /* should the answer be negated? */
    int m;                      /* |$2^{31}$ / radix|, the threshold of danger */
    int d;                      /* the digit just scanned */
    boolean vacuous;            /* have no digits appeared? */
    boolean OK_so_far;          /* has an error message been issued? */
    radix = 0;
    OK_so_far = true;
    /* Get the next non-blank non-sign token; set |negative| appropriately */
    negative = false;
    do {
        /* Get the next non-blank non-call token */
        do {
            get_x_token();
        } while (cur_cmd == spacer_cmd);
        if (cur_tok == other_token + '-') {
            negative = !negative;
            cur_tok = other_token + '+';
        }
    } while (cur_tok == other_token + '+');

  RESTART:
    if (cur_tok == alpha_token) {
        /* Scan an alphabetic character code into |cur_val| */
        /* A space is ignored after an alphabetic character constant, so that
           such constants behave like numeric ones. */
        get_token();            /* suppress macro expansion */
        if (cur_tok < cs_token_flag) {
            cur_val = cur_chr;
            if (cur_cmd <= right_brace_cmd) {
                if (cur_cmd == right_brace_cmd)
                    incr(align_state);
                else
                    decr(align_state);
            }
        } else {                /* the value of a csname in this context is its name */
            str_number txt = cs_text(cur_tok - cs_token_flag);
            if (is_active_cs(txt))
                cur_val = active_cs_value(txt);
            else if (single_letter(txt))
                cur_val = pool_to_unichar(str_string(txt));
            else
                cur_val = (biggest_char + 1);
        }
        if (cur_val > biggest_char) {
            print_err("Improper alphabetic constant");
            help2("A one-character control sequence belongs after a ` mark.",
                  "So I'm essentially inserting \\0 here.");
            cur_val = '0';
            back_error();
        } else {
            /* Scan an optional space */
            get_x_token();
            if (cur_cmd != spacer_cmd)
                back_input();
        }

    } else if (cur_tok == cs_token_flag + frozen_primitive) {
        /* Reset |cur_tok| for unexpandable primitives, goto restart */
        /* This block deals with unexpandable \.{\\primitive} appearing at a spot where
           an integer or an internal values should have been found. It fetches the
           next token then resets |cur_cmd|, |cur_cs|, and |cur_tok|, based on the
           primitive value of that token. No expansion takes place, because the
           next token may be all sorts of things. This could trigger further
           expansion creating new errors.
         */
        get_token();
        cur_cs = prim_lookup(cs_text(cur_cs));
        if (cur_cs != undefined_primitive) {
            cur_cmd = get_prim_eq_type(cur_cs);
            cur_chr = get_prim_equiv(cur_cs);
            cur_tok = token_val(cur_cmd, cur_chr);
        } else {
            cur_cmd = relax_cmd;
            cur_chr = 0;
            cur_tok = cs_token_flag + frozen_relax;
            cur_cs = frozen_relax;
        }
        goto RESTART;
    } else if (cur_cmd == math_style_cmd) {
        cur_val = cur_chr;
    } else if ((cur_cmd >= min_internal_cmd) && (cur_cmd <= max_internal_cmd)) {
        scan_something_internal(int_val_level, false);
    } else {
        /* Scan a numeric constant */
        radix = 10;
        m = 214748364;
        if (cur_tok == octal_token) {
            radix = 8;
            m = 02000000000;
            get_x_token();
        } else if (cur_tok == hex_token) {
            radix = 16;
            m = 01000000000;
            get_x_token();
        }
        vacuous = true;
        cur_val = 0;
        /* Accumulate the constant until |cur_tok| is not a suitable digit */
        while (1) {
            if ((cur_tok < zero_token + radix) && (cur_tok >= zero_token)
                && (cur_tok <= zero_token + 9)) {
                d = cur_tok - zero_token;
            } else if (radix == 16) {
                if ((cur_tok <= A_token + 5) && (cur_tok >= A_token)) {
                    d = cur_tok - A_token + 10;
                } else if ((cur_tok <= other_A_token + 5)
                           && (cur_tok >= other_A_token)) {
                    d = cur_tok - other_A_token + 10;
                } else {
                    break;
                }
            } else {
                break;
            }
            vacuous = false;
            if ((cur_val >= m) && ((cur_val > m) || (d > 7) || (radix != 10))) {
                if (OK_so_far) {
                    print_err("Number too big");
                    help2
                        ("I can only go up to 2147483647='17777777777=\"7FFFFFFF,",
                         "so I'm using that number instead of yours.");
                    error();
                    cur_val = infinity;
                    OK_so_far = false;
                }
            } else {
                cur_val = cur_val * radix + d;
            }
            get_x_token();
        }
        if (vacuous) {
            /* Express astonishment that no number was here */
            print_err("Missing number, treated as zero");
            help3("A number should have been here; I inserted `0'.",
                  "(If you can't figure out why I needed to see a number,",
                  "look up `weird error' in the index to The TeXbook.)");
            back_error();
        } else if (cur_cmd != spacer_cmd) {
            back_input();
        }
    }
    if (negative)
        negate(cur_val);
}

@ The following code is executed when |scan_something_internal| was
called asking for |mu_val|, when we really wanted a ``mudimen'' instead
of ``muglue.''

@c
static void coerce_glue(void)
{
    int v;
    if (cur_val_level >= glue_val_level) {
        v = width(cur_val);
        flush_node(cur_val);
        cur_val = v;
    }
}

@ The |scan_dimen| routine is similar to |scan_int|, but it sets |cur_val| to
a |scaled| value, i.e., an integral number of sp. One of its main tasks
is therefore to interpret the abbreviations for various kinds of units and
to convert measurements to scaled points.

There are three parameters: |mu| is |true| if the finite units must be
`\.{mu}', while |mu| is |false| if `\.{mu}' units are disallowed;
|inf| is |true| if the infinite units `\.{fil}', `\.{fill}', `\.{filll}'
are permitted; and |shortcut| is |true| if |cur_val| already contains
an integer and only the units need to be considered.

The order of infinity that was found in the case of infinite glue is returned
in the global variable |cur_order|.

@c
int cur_order;                  /* order of infinity found by |scan_dimen| */


@ Constructions like `\.{-\'77 pt}' are legal dimensions, so |scan_dimen|
may begin with |scan_int|. This explains why it is convenient to use
|scan_int| also for the integer part of a decimal fraction.

Several branches of |scan_dimen| work with |cur_val| as an integer and
with an auxiliary fraction |f|, so that the actual quantity of interest is
$|cur_val|+|f|/2^{16}$. At the end of the routine, this ``unpacked''
representation is put into the single word |cur_val|, which suddenly
switches significance from |integer| to |scaled|.

@
The necessary conversion factors can all be specified exactly as
fractions whose numerator and denominator add to 32768 or less.
According to the definitions here, $\rm2660\,dd\approx1000.33297\,mm$;
this agrees well with the value $\rm1000.333\,mm$ cited by Bosshard
\^{Bosshard, Hans Rudolf}
in {\sl Technische Grundlagen zur Satzherstellung\/} (Bern, 1980).
The Didot point has been newly standardized in 1978;
it's now exactly $\rm 1\,nd=0.375\,mm$.
Conversion uses the equation $0.375=21681/20320/72.27\cdot25.4$.
The new Cicero follows the new Didot point; $\rm 1\,nc=12\,nd$.
These would lead to the ratios $21681/20320$ and $65043/5080$,
respectively.
The closest approximations supported by the algorithm would be
$11183/10481$ and $1370/107$.  In order to maintain the
relation $\rm 1\,nc=12\,nd$, we pick the ratio $685/642$ for
$\rm nd$, however.

@c

static void scan_dimen_mu_error(void) {
    print_err("Illegal unit of measure (mu inserted)");
    help4("The unit of measurement in math glue must be mu.",
          "To recover gracefully from this error, it's best to",
          "delete the erroneous units; e.g., type `2' to delete",
          "two letters. (See Chapter 27 of The TeXbook.)");
    error();
}

static void scan_dimen_unknown_unit_error(void) {
    print_err("Illegal unit of measure (pt inserted)");
    help6("Dimensions can be in units of em, ex, in, pt, pc,",
          "cm, mm, dd, cc, nd, nc, bp, or sp; but yours is a new one!",
          "I'll assume that you meant to say pt, for printer's points.",
          "To recover gracefully from this error, it's best to",
          "delete the erroneous units; e.g., type `2' to delete",
          "two letters. (See Chapter 27 of The TeXbook.)");
    error();
}

static void scan_dimen_out_of_range_error(void) {
    print_err("Dimension too large");
    help2("I can't work with sizes bigger than about 19 feet.",
          "Continue and I'll use the largest value I can.");
    error();
}

#define set_conversion(A,B) do { num=(A); denom=(B); } while(0)

/*
    This function sets |cur_val| to a dimension. It could be optimized a bit
    more (but not now, something for luatex > 1).
*/

void scan_dimen(boolean mu, boolean inf, boolean shortcut)
{
    boolean negative; /* should the answer be negated? */
    int f = 0;        /* numerator of a fraction whose denominator is $2^{16}$ */
    int num, denom;   /* conversion ratio for the scanned units */
    halfword q;       /* top of decimal digit stack */
    scaled v;         /* an internal dimension */
    int save_cur_val; /* temporary storage of |cur_val| */
    arith_error = false;
    cur_order = normal;
    negative = false;
    if (!shortcut) {
        /* Get the next non-blank non-sign... */
        do {
            /* Get the next non-blank non-call token */
            do {
                get_x_token();
            } while (cur_cmd == spacer_cmd);
            if (cur_tok == other_token + '-') {
                negative = !negative;
                cur_tok = other_token + '+';
            }
        } while (cur_tok == other_token + '+');
        if ((cur_cmd >= min_internal_cmd) && (cur_cmd <= max_internal_cmd)) {
            /* Fetch an internal dimension and |goto attach_sign|, or fetch an internal integer */
            if (mu) {
                scan_something_internal(mu_val_level, false);
                coerce_glue();
                if (cur_val_level == mu_val_level) {
                    goto ATTACH_SIGN;
                } else if (cur_val_level != int_val_level) {
                    mu_error();
                }
            } else {
                scan_something_internal(dimen_val_level, false);
                if (cur_val_level == dimen_val_level) {
                    goto ATTACH_SIGN;
                }
            }
        } else {
            back_input();
            if (cur_tok == continental_point_token) {
                cur_tok = point_token;
            }
            if (cur_tok != point_token) {
                scan_int();
            } else {
                radix = 10;
                cur_val = 0;
            }
            if (cur_tok == continental_point_token) {
                cur_tok = point_token;
            }
            if ((radix == 10) && (cur_tok == point_token)) {
                /*
                    Scan decimal fraction. When the following code is executed, we have
                    |cur_tok=point_token|, but this token has been backed up using |back_input|;
                    we must first discard it. It turns out that a decimal point all by itself
                    is equivalent to `\.{0.0}'. Let's hope people don't use that fact.
                */
                int k = 0;
                halfword p = null;
                int kk;
                get_token(); /* |point_token| is being re-scanned */
                while (1) {
                    get_x_token();
                    if ((cur_tok > zero_token + 9) || (cur_tok < zero_token))
                        break;
                    if (k < 17) {
                        /* digits for |k>=17| cannot affect the result */
                        q = get_avail();
                        set_token_link(q, p);
                        set_token_info(q, cur_tok - zero_token);
                        p = q;
                        incr(k);
                    }
                }
                for (kk = k; kk >= 1; kk--) {
                    dig[kk - 1] = token_info(p);
                    q = p;
                    p = token_link(p);
                    free_avail(q);
                }
                f = round_decimals(k);
                if (cur_cmd != spacer_cmd) {
                    back_input();
                }
            }
        }
    }
    if (cur_val < 0) {
        /* in this case |f=0| */
        negative = !negative;
        negate(cur_val);
    }
    /*
        Scan units and set |cur_val| to $x\cdot(|cur_val|+f/2^{16})$, where there
        are |x| sp per unit; |goto attach_sign| if the units are internal. Now comes
        the harder part: At this point in the program, |cur_val| is a nonnegative
        integer and $f/2^{16}$ is a nonnegative fraction less than 1; we want to
        multiply the sum of these two quantities by the appropriate factor, based
        on the specified units, in order to produce a |scaled| result, and we want
        to do the calculation with fixed point arithmetic that does not overflow.
    */
    if (inf) {
        /*
            Scan for (f)\.{fil} units; |goto attach_fraction| if found. In traditional
            \TeX, a specification like `\.{filllll}' or `\.{fill L L L}' will lead to
            two error messages (one for each additional keyword \.{"l"}). Not so for
            \LuaTeX, it just parses the construct in reverse.

             if (scan_keyword("filll")) {
                 cur_order = filll;
                 goto ATTACH_FRACTION;
             } else if (scan_keyword("fill")) {
                 cur_order = fill;
                 goto ATTACH_FRACTION;
             } else if (scan_keyword("fil")) {
                 cur_order = fil;
                 goto ATTACH_FRACTION;
             } else if (scan_keyword("fi")) {
                 cur_order = sfi;
                 goto ATTACH_FRACTION;
             }

            But ... it failed in alignments so now we do this. And, as we support an extra
            l we don't issue an error message (we didn't do that anyway).
         */
         if (scan_keyword("fi")) {
             cur_order = sfi;
             if (scan_keyword("l")) {
                cur_order = fil;
                if (scan_keyword("l")) {
                    cur_order = fill;
                    if (scan_keyword("l")) {
                        cur_order = filll;
                    }
                }
            }
            goto ATTACH_FRACTION;
         }
    }
    /*
        Scan for (u)units that are internal dimensions; |goto attach_sign| with
        |cur_val| set if found
    */
    save_cur_val = cur_val;
    /* Get the next non-blank non-call... a pitty if just backed up the input */
    do {
        get_x_token();
    } while (cur_cmd == spacer_cmd);

    if ((cur_cmd < min_internal_cmd) || (cur_cmd > max_internal_cmd)) {
        back_input();
    } else {
        /* math_given_cmd xmath_given_cmd last_item_cmd */
        if (mu) {
            scan_something_internal(mu_val_level, false);
            coerce_glue();
            if (cur_val_level != mu_val_level) {
                mu_error();
            }
        } else {
            scan_something_internal(dimen_val_level, false);
        }
        v = cur_val;
        goto FOUND;
    }
    /* bah ... true forces to split the unit scanner */
    if (mu) {
        /* Scan for (m)\.{mu} units and |goto attach_fraction| */
        if (! scan_keyword("mu")) {
            scan_dimen_mu_error();
        }
        goto ATTACH_FRACTION;
    } else if (scan_keyword("em")) {
        v = quad(get_cur_font());
    } else if (scan_keyword("ex")) {
        v = x_height(get_cur_font());
    } else if (scan_keyword("px")) {
        v = dimen_par(px_dimen_code);
    } else {
        goto PICKUP_UNIT;
    }
    /* Scan an optional space (after em, ex or px) */
    get_x_token();
    if (cur_cmd != spacer_cmd) {
        back_input();
    }
  FOUND:
    cur_val = nx_plus_y(save_cur_val, v, xn_over_d(v, f, 0200000));
    goto ATTACH_SIGN;
    /*
        Scan for (a)all other units and adjust |cur_val| and |f| accordingly;
        |goto done| in the case of scaled points
    */
  PICKUP_UNIT:
    if (scan_keyword("pt")) {
        goto ATTACH_FRACTION;   /* the easy case */
    } else if (scan_keyword("mm")) {
        set_conversion(7227, 2540);
        goto SCALE_VALUE;
    } else if (scan_keyword("cm")) {
        set_conversion(7227, 254);
        goto SCALE_VALUE;
    } else if (scan_keyword("sp")) {
        goto DONE;
    } else if (scan_keyword("bp")) {
        set_conversion(7227, 7200);
        goto SCALE_VALUE;
    } else if (scan_keyword("in")) {
        set_conversion(7227, 100);
        goto SCALE_VALUE;
    } else if (scan_keyword("dd")) {
        set_conversion(1238, 1157);
        goto SCALE_VALUE;
    } else if (scan_keyword("cc")) {
        set_conversion(14856, 1157);
        goto SCALE_VALUE;
    } else if (scan_keyword("pc")) {
        set_conversion(12, 1);
        goto SCALE_VALUE;
    } else if (scan_keyword("nd")) {
        set_conversion(685, 642);
        goto SCALE_VALUE;
    } else if (scan_keyword("nc")) {
        set_conversion(1370, 107);
        goto SCALE_VALUE;
    } else if (scan_keyword("true")) {
        /* Adjust (f)for the magnification ratio */
        if (output_mode_used == OMODE_DVI) {
            prepare_mag();
            if (int_par(mag_code) != 1000) {
                cur_val = xn_over_d(cur_val, 1000, int_par(mag_code));
                f = (1000 * f + 0200000 * tex_remainder) / int_par(mag_code);
                cur_val = cur_val + (f / 0200000);
                f = f % 0200000;
            }
        }
        goto PICKUP_UNIT;
    } else {
        /* Complain about unknown unit and |goto done2| */
        scan_dimen_unknown_unit_error();
        goto BAD_NEWS;
    }
  SCALE_VALUE:
    cur_val = xn_over_d(cur_val, num, denom);
    f = (num * f + 0200000 * tex_remainder) / denom;
    cur_val = cur_val + (f / 0200000);
    f = f % 0200000;
  BAD_NEWS:
  ATTACH_FRACTION:
    if (cur_val >= 040000) {
        arith_error = true;
    } else {
        cur_val = cur_val * unity + f;
    }
  DONE:
    /* Scan an optional space */ /* happens too often */
    get_x_token();
    if (cur_cmd != spacer_cmd) {
        back_input();
    }
  ATTACH_SIGN:
    if (arith_error || (abs(cur_val) >= 010000000000)) {
        /* Report that this dimension is out of range */
        scan_dimen_out_of_range_error();
        cur_val = max_dimen;
        arith_error = false;
    }
    if (negative) {
        negate(cur_val);
    }
}

@ The final member of \TeX's value-scanning trio is |scan_glue|, which
makes |cur_val| point to a glue specification. The reference count of that
glue spec will take account of the fact that |cur_val| is pointing to~it.

The |level| parameter should be either |glue_val| or |mu_val|.

Since |scan_dimen| was so much more complex than |scan_int|, we might expect
|scan_glue| to be even worse. But fortunately, it is very simple, since
most of the work has already been done.

@c
void scan_glue(int level)
{
    boolean negative = false;             /* should the answer be negated? */
    halfword q = null;                    /* new glue specification */
    boolean mu = (level == mu_val_level); /* does |level=mu_val|? */
    /* Get the next non-blank non-sign ... */
    do {
        /* Get the next non-blank non-call token */
        do {
            get_x_token();
        } while (cur_cmd == spacer_cmd);
        if (cur_tok == other_token + '-') {
            negative = !negative;
            cur_tok = other_token + '+';
        }
    } while (cur_tok == other_token + '+');
    if ((cur_cmd >= min_internal_cmd) && (cur_cmd <= max_internal_cmd)) {
        scan_something_internal(level, negative);
        if (cur_val_level >= glue_val_level) {
            if (cur_val_level != level)
                mu_error();
            return;
        }
        if (cur_val_level == int_val_level)
            scan_dimen(mu, false, true);
        else if (level == mu_val_level)
            mu_error();
    } else {
        back_input();
        scan_dimen(mu, false, false);
        if (negative)
            negate(cur_val);
    }
    /*
        Create a new glue specification whose width is |cur_val|; scan for its
        stretch and shrink components.
    */
    q = new_spec(zero_glue);
    width(q) = cur_val;
    if (scan_keyword("plus")) {
        scan_dimen(mu, true, false);
        stretch(q) = cur_val;
        stretch_order(q) = (quarterword) cur_order;
    }
    if (scan_keyword("minus")) {
        scan_dimen(mu, true, false);
        shrink(q) = cur_val;
        shrink_order(q) = (quarterword) cur_order;
    }
    cur_val = q;
}

@ This is an omega routine
@c
void scan_scaled(void)
{                               /* sets |cur_val| to a scaled value */
    boolean negative;           /* should the answer be negated? */
    int f;                      /* numerator of a fraction whose denominator is $2^{16}$ */
    int k, kk;                  /* number of digits in a decimal fraction */
    halfword p, q;              /* top of decimal digit stack */
    f = 0;
    arith_error = false;
    negative = false;
    /* Get the next non-blank non-sign... */
    do {
        /* Get the next non-blank non-call token */
        do {
            get_x_token();
        } while (cur_cmd == spacer_cmd);
        if (cur_tok == other_token + '-') {
            negative = !negative;
            cur_tok = other_token + '+';
        }
    } while (cur_tok == other_token + '+');

    back_input();
    if (cur_tok == continental_point_token)
        cur_tok = point_token;
    if (cur_tok != point_token) {
        scan_int();
    } else {
        radix = 10;
        cur_val = 0;
    }
    if (cur_tok == continental_point_token)
        cur_tok = point_token;
    if ((radix == 10) && (cur_tok == point_token)) {
        /* Scan decimal fraction */
        /* TODO: merge this with the same block in |scan_dimen| */
        /* When the following code is executed, we have |cur_tok=point_token|, but this
           token has been backed up using |back_input|; we must first discard it.

           It turns out that a decimal point all by itself is equivalent to `\.{0.0}'.
           Let's hope people don't use that fact. */

        k = 0;
        p = null;
        get_token();            /* |point_token| is being re-scanned */
        while (1) {
            get_x_token();
            if ((cur_tok > zero_token + 9) || (cur_tok < zero_token))
                break;
            if (k < 17) {       /* digits for |k>=17| cannot affect the result */
                q = get_avail();
                set_token_link(q, p);
                set_token_info(q, cur_tok - zero_token);
                p = q;
                incr(k);
            }
        }
        for (kk = k; kk >= 1; kk--) {
            dig[kk - 1] = token_info(p);
            q = p;
            p = token_link(p);
            free_avail(q);
        }
        f = round_decimals(k);
        if (cur_cmd != spacer_cmd)
            back_input();

    }
    if (cur_val < 0) {          /* in this case |f=0| */
        negative = !negative;
        negate(cur_val);
    }
    if (cur_val > 040000)
        arith_error = true;
    else
        cur_val = cur_val * unity + f;
    if (arith_error || (abs(cur_val) >= 010000000000)) {
        print_err("Stack number too large");
        error();
    }
    if (negative)
        negate(cur_val);
}

@ This procedure is supposed to scan something like `\.{\\skip\\count12}',
i.e., whatever can follow `\.{\\the}', and it constructs a token list
containing something like `\.{-3.0pt minus 0.5fill}'.

@c
halfword the_toks(void)
{
    int old_setting;            /* holds |selector| setting */
    halfword p, q, r;           /* used for copying a token list */
    int c;                      /* value of |cur_chr| */
    str_number s;
    halfword retval;
    /* Handle \.{\\unexpanded} or \.{\\detokenize} and |return| */
    if (odd(cur_chr)) {
        c = cur_chr;
        scan_general_text();
        if (c == 1) {
            return cur_val;
        } else {
            old_setting = selector;
            selector = new_string;
            p = get_avail();
            set_token_link(p, token_link(temp_token_head));
            token_show(p);
            flush_list(p);
            selector = old_setting;
            s = make_string();
            retval = str_toks(str_lstring(s));
            flush_str(s);
            return retval;
        }
    }
    get_x_token();
    scan_something_internal(tok_val_level, false);
    if (cur_val_level >= ident_val_level) {
        /* Copy the token list */
        p = temp_token_head;
        set_token_link(p, null);
        if (cur_val_level == ident_val_level) {
            store_new_token(cs_token_flag + cur_val);
        } else if (cur_val != null) {
            r = token_link(cur_val);    /* do not copy the reference count */
            while (r != null) {
                fast_store_new_token(token_info(r));
                r = token_link(r);
            }
        }
        return p;
    } else {
        old_setting = selector;
        selector = new_string;
        switch (cur_val_level) {
        case int_val_level:
            print_int(cur_val);
            break;
        case attr_val_level:
            print_int(cur_val);
            break;
        case dir_val_level:
            print_dir(cur_val);
            break;
        case dimen_val_level:
            print_scaled(cur_val);
            tprint("pt");
            break;
        case glue_val_level:
            print_spec(cur_val, "pt");
            flush_node(cur_val);
            break;
        case mu_val_level:
            print_spec(cur_val, "mu");
            flush_node(cur_val);
            break;
        }                       /* there are no other cases */
        selector = old_setting;
        s = make_string();
        retval = str_toks(str_lstring(s));
        flush_str(s);
        return retval;
    }
}

@ @c
str_number the_scanned_result(void)
{
    int old_setting;            /* holds |selector| setting */
    str_number r;               /* return value * */
    old_setting = selector;
    selector = new_string;
    if (cur_val_level >= ident_val_level) {
        if (cur_val != null) {
            show_token_list(token_link(cur_val), null, -1);
            r = make_string();
        } else {
            r = get_nullstr();
        }
    } else {
        switch (cur_val_level) {
        case int_val_level:
            print_int(cur_val);
            break;
        case attr_val_level:
            print_int(cur_val);
            break;
        case dir_val_level:
            print_dir(cur_val);
            break;
        case dimen_val_level:
            print_scaled(cur_val);
            tprint("pt");
            break;
        case glue_val_level:
            print_spec(cur_val, "pt");
            flush_node(cur_val);
            break;
        case mu_val_level:
            print_spec(cur_val, "mu");
            flush_node(cur_val);
            break;
        }                       /* there are no other cases */
        r = make_string();
    }
    selector = old_setting;
    return r;
}

@ The following routine is used to implement `\.{\\fontdimen} |n| |f|'.
The boolean parameter |writing| is set |true| if the calling program
intends to change the parameter value.

@c
static void font_param_error(int f)
{
    print_err("Font ");
    print_esc(font_id_text(f));
    tprint(" has only ");
    print_int(font_params(f));
    tprint(" fontdimen parameters");
    help2("To increase the number of font parameters, you must",
          "use \\fontdimen immediately after the \\font is loaded.");
    error();
}

void set_font_dimen(void)
{
    internal_font_number f;
    int n;                      /* the parameter number */
    scan_int();
    n = cur_val;
    scan_font_ident();
    f = cur_val;
    if (n <= 0) {
        font_param_error(f);
    } else {
        if (n > font_params(f)) {
            if (font_used(f)) {
                font_param_error(f);
            } else {
                /* Increase the number of parameters in the font */
                do {
                    set_font_param(f, (font_params(f) + 1), 0);
                } while (n != font_params(f));
            }
        }
    }
    scan_optional_equals();
    scan_normal_dimen();
    set_font_param(f, n, cur_val);
}

void get_font_dimen(void)
{
    internal_font_number f;
    int n;                      /* the parameter number */
    scan_int();
    n = cur_val;
    scan_font_ident();
    f = cur_val;
    cur_val = 0;                /* initialize return value */
    if (n <= 0) {
        font_param_error(f);
        goto EXIT;
    } else {
        if (n > font_params(f)) {
            if (font_used(f)) {
                font_param_error(f);
                goto EXIT;
            } else {
                /* Increase the number of parameters in the font */
                do {
                    set_font_param(f, (font_params(f) + 1), 0);
                } while (n != font_params(f));

            }
        }
    }
    cur_val = font_param(f, n);
  EXIT:
    scanned_result(cur_val, dimen_val_level);
}

@ Here's a similar procedure that returns a pointer to a rule node. This
routine is called just after \TeX\ has seen \.{\\hrule} or \.{\\vrule};
therefore |cur_cmd| will be either |hrule| or |vrule|. The idea is to store
the default rule dimensions in the node, then to override them if
`\.{height}' or `\.{width}' or `\.{depth}' specifications are
found (in any order).

@c
halfword scan_rule_spec(void)
{
    /* |width|, |depth|, and |height| all equal |null_flag| now */
    halfword q;
    if (cur_cmd == no_vrule_cmd) {
        q = new_rule(empty_rule);
        cur_cmd = vrule_cmd;
    } else if (cur_cmd == no_hrule_cmd) {
        q = new_rule(empty_rule);
        cur_cmd = hrule_cmd;
    } else {
        q = new_rule(normal_rule);
    }
    if (cur_cmd == vrule_cmd) {
        width(q) = default_rule;
        rule_dir(q) = body_direction;
    } else {
        height(q) = default_rule;
        depth(q) = 0;
        rule_dir(q) = text_direction;
    }
  RESWITCH:
    if (scan_keyword("width")) {
        scan_normal_dimen();
        width(q) = cur_val;
        goto RESWITCH;
    }
    if (scan_keyword("height")) {
        scan_normal_dimen();
        height(q) = cur_val;
        goto RESWITCH;
    }
    if (scan_keyword("depth")) {
        scan_normal_dimen();
        depth(q) = cur_val;
        goto RESWITCH;
    }
    return q;
}


@ Declare procedures that scan font-related stuff

@c
void scan_font_ident(void)
{
    internal_font_number f;
    halfword m;
    /* Get the next non-blank non-call... */
    do {
        get_x_token();
    } while (cur_cmd == spacer_cmd);

    if ((cur_cmd == def_font_cmd) || (cur_cmd == letterspace_font_cmd) || (cur_cmd == copy_font_cmd)) {
        f = get_cur_font();
    } else if (cur_cmd == set_font_cmd) {
        f = cur_chr;
        set_font_touched(f, 1);
    } else if (cur_cmd == def_family_cmd) {
        m = cur_chr;
        scan_math_family_int();
        f = fam_fnt(cur_val, m);
        set_font_touched(f, 1);
    } else {
        print_err("Missing font identifier");
        help2("I was looking for a control sequence whose",
              "current meaning has been defined by \\font.");
        back_error();
        f = null_font;
    }
    cur_val = f;
}

@ The |scan_general_text| procedure is much like |scan_toks(false,false)|,
but will be invoked via |expand|, i.e., recursively.

The token list (balanced text) created by |scan_general_text| begins
at |link(temp_token_head)| and ends at |cur_val|.  (If |cur_val=temp_token_head|,
the list is empty.)

@c
void scan_general_text(void)
{
    int s;                      /* to save |scanner_status| */
    halfword w;                 /* to save |warning_index| */
    halfword d;                 /* to save |def_ref| */
    halfword p;                 /* tail of the token list being built */
    halfword q;                 /* new node being added to the token list via |store_new_token| */
    halfword unbalance;         /* number of unmatched left braces */
    s = scanner_status;
    w = warning_index;
    d = def_ref;
    scanner_status = absorbing;
    warning_index = cur_cs;
    p = get_avail();
    def_ref = p;
    set_token_ref_count(def_ref, 0);
    p = def_ref;
    scan_left_brace();          /* remove the compulsory left brace */
    unbalance = 1;
    while (1) {
        get_token();
        if (cur_tok < right_brace_limit) {
            if (cur_cmd < right_brace_cmd) {
                incr(unbalance);
            } else {
                decr(unbalance);
                if (unbalance == 0)
                    break;
            }
        }
        store_new_token(cur_tok);
    }
    q = token_link(def_ref);
    free_avail(def_ref);        /* discard reference count */
    if (q == null)
        cur_val = temp_token_head;
    else
        cur_val = p;
    set_token_link(temp_token_head, q);
    scanner_status = s;
    warning_index = w;
    def_ref = d;
}

@ The |get_x_or_protected| procedure is like |get_x_token| except that
protected macros are not expanded.

@c
void get_x_or_protected(void)
{                               /* sets |cur_cmd|, |cur_chr|, |cur_tok|,
                                   and expands non-protected macros */
    while (1) {
        get_token();
        if (cur_cmd <= max_command_cmd)
            return;
        if ((cur_cmd >= call_cmd) && (cur_cmd < end_template_cmd)) {
            if (token_info(token_link(cur_chr)) == protected_token)
                return;
        }
        expand();
    }
}

@ |scan_toks|. This function returns a pointer to the tail of a new token
list, and it also makes |def_ref| point to the reference count at the
head of that list.

There are two boolean parameters, |macro_def| and |xpand|. If |macro_def|
is true, the goal is to create the token list for a macro definition;
otherwise the goal is to create the token list for some other \TeX\
primitive: \.{\\mark}, \.{\\output}, \.{\\everypar}, \.{\\lowercase},
\.{\\uppercase}, \.{\\message}, \.{\\errmessage}, \.{\\write}, or
\.{\\special}. In the latter cases a left brace must be scanned next; this
left brace will not be part of the token list, nor will the matching right
brace that comes at the end. If |xpand| is false, the token list will
simply be copied from the input using |get_token|. Otherwise all expandable
tokens will be expanded until unexpandable tokens are left, except that
the results of expanding `\.{\\the}' are not expanded further.
If both |macro_def| and |xpand| are true, the expansion applies
only to the macro body (i.e., to the material following the first
|left_brace| character).

The value of |cur_cs| when |scan_toks| begins should be the |eqtb|
address of the control sequence to display in ``runaway'' error
messages.

@c
halfword scan_toks(boolean macro_def, boolean xpand)
{
    halfword t;                 /* token representing the highest parameter number */
    halfword s;                 /* saved token */
    halfword p;                 /* tail of the token list being built */
    halfword q;                 /* new node being added to the token list via |store_new_token| */
    halfword unbalance;         /* number of unmatched left braces */
    halfword hash_brace;        /* possible `\.{\#\{}' token */
    if (macro_def)
        scanner_status = defining;
    else
        scanner_status = absorbing;
    warning_index = cur_cs;
    p = get_avail();
    def_ref = p;
    set_token_ref_count(def_ref, 0);
    p = def_ref;
    hash_brace = 0;
    t = zero_token;
    if (macro_def) {
        /* Scan and build the parameter part of the macro definition */
        while (1) {
            get_token();        /* set |cur_cmd|, |cur_chr|, |cur_tok| */
            if (cur_tok < right_brace_limit)
                break;
            if (cur_cmd == mac_param_cmd) {
                /* If the next character is a parameter number, make |cur_tok|
                   a |match| token; but if it is a left brace, store
                   `|left_brace|, |end_match|', set |hash_brace|, and |goto done|;
                 */
                s = match_token + cur_chr;
                get_token();
                if (cur_cmd == left_brace_cmd) {
                    hash_brace = cur_tok;
                    store_new_token(cur_tok);
                    store_new_token(end_match_token);
                    goto DONE;
                }
                if (t == zero_token + 9) {
                    print_err("You already have nine parameters");
                    help1("I'm going to ignore the # sign you just used.");
                    error();
                } else {
                    incr(t);
                    if (cur_tok != t) {
                        print_err("Parameters must be numbered consecutively");
                        help2
                            ("I've inserted the digit you should have used after the #.",
                             "Type `1' to delete what you did use.");
                        back_error();
                    }
                    cur_tok = s;
                }
            }
            store_new_token(cur_tok);
        }
        store_new_token(end_match_token);
        if (cur_cmd == right_brace_cmd) {
            /* Express shock at the missing left brace; |goto found| */
            print_err("Missing { inserted");
            incr(align_state);
            help2
                ("Where was the left brace? You said something like `\\def\\a}',",
                 "which I'm going to interpret as `\\def\\a{}'.");
            error();
            goto FOUND;
        }

    } else {
        scan_left_brace();      /* remove the compulsory left brace */
    }
  DONE:
    /* Scan and build the body of the token list; |goto found| when finished */
    unbalance = 1;
    while (1) {
        if (xpand) {
            /* Expand the next part of the input */
            /* Here we insert an entire token list created by |the_toks| without
               expanding it further. */
            while (1) {
                get_next();
                if (cur_cmd >= call_cmd) {
                    if (token_info(token_link(cur_chr)) == protected_token) {
                        cur_cmd = relax_cmd;
                        cur_chr = no_expand_flag;
                    }
                }
                if (cur_cmd <= max_command_cmd)
                    break;
                if (cur_cmd != the_cmd) {
                    expand();
                } else {
                    q = the_toks();
                    if (token_link(temp_token_head) != null) {
                        set_token_link(p, token_link(temp_token_head));
                        p = q;
                    }
                }
            }
            x_token();

        } else {
            get_token();
        }
        if (cur_tok < right_brace_limit) {
            if (cur_cmd < right_brace_cmd) {
                incr(unbalance);
            } else {
                decr(unbalance);
                if (unbalance == 0)
                    goto FOUND;
            }
        } else if (cur_cmd == mac_param_cmd) {
            if (macro_def) {
                /* Look for parameter number or \.{\#\#} */
                s = cur_tok;
                if (xpand)
                    get_x_token();
                else
                    get_token();
                if (cur_cmd != mac_param_cmd) {
                    if ((cur_tok <= zero_token) || (cur_tok > t)) {
                        print_err("Illegal parameter number in definition of ");
                        sprint_cs(warning_index);
                        help3("You meant to type ## instead of #, right?",
                              "Or maybe a } was forgotten somewhere earlier, and things",
                              "are all screwed up? I'm going to assume that you meant ##.");
                        back_error();
                        cur_tok = s;
                    } else {
                        cur_tok = out_param_token - '0' + cur_chr;
                    }
                }
            }
        }
        store_new_token(cur_tok);
    }
  FOUND:
    scanner_status = normal;
    if (hash_brace != 0)
        store_new_token(hash_brace);
    return p;
}

@ Here we declare two trivial procedures in order to avoid mutually
recursive procedures with parameters.

@c
void scan_normal_glue(void)
{
    scan_glue(glue_val_level);
}

void scan_mu_glue(void)
{
    scan_glue(mu_val_level);
}

@ The |scan_expr| procedure scans and evaluates an expression.

@ Evaluating an expression is a recursive process:  When the left
parenthesis of a subexpression is scanned we descend to the next level
of recursion; the previous level is resumed with the matching right
parenthesis.

@c
typedef enum {
    expr_none = 0,              /* \.( seen, or \.( $\langle\it expr\rangle$ \.) seen */
    expr_add = 1,               /* \.( $\langle\it expr\rangle$ \.+ seen */
    expr_sub = 2,               /* \.( $\langle\it expr\rangle$ \.- seen */
    expr_mult = 3,              /* $\langle\it term\rangle$ \.* seen */
    expr_div = 4,               /* $\langle\it term\rangle$ \./ seen */
    expr_scale = 5,             /* $\langle\it term\rangle$ \.*  $\langle\it factor\rangle$ \./ seen */
} expression_states;


@  We want to make sure that each term and (intermediate) result is in
  the proper range.  Integer values must not exceed |infinity|
  ($2^{31}-1$) in absolute value, dimensions must not exceed |max_dimen|
  ($2^{30}-1$).  We avoid the absolute value of an integer, because this
  might fail for the value $-2^{31}$ using 32-bit arithmetic.

@   clear a number or dimension and set |arith_error|

@c
#define num_error(A) do { \
    arith_error=true; \
    A=0; \
} while (0)

@   clear a glue spec and set |arith_error|

@c
#define glue_error(A) do { \
    arith_error=true; \
    reset_glue_to_zero(A); \
} while (0)

#define normalize_glue(A) do { \
    if (stretch(A)==0) stretch_order(A)=normal; \
    if (shrink(A)==0) shrink_order(A)=normal; \
} while (0)

@ Parenthesized subexpressions can be inside expressions, and this
nesting has a stack.  Seven local variables represent the top of the
expression stack:  |p| points to pushed-down entries, if any; |l|
specifies the type of expression currently beeing evaluated; |e| is the
expression so far and |r| is the state of its evaluation; |t| is the
term so far and |s| is the state of its evaluation; finally |n| is the
numerator for a combined multiplication and division, if any.

@c
#define expr_type(A) type((A)+1)
#define expr_state(A) subtype((A)+1)
#define expr_e_field(A) vlink((A)+1)    /* saved expression so far */
#define expr_t_field(A) vlink((A)+2)    /* saved term so far */
#define expr_n_field(A) vinfo((A)+2)    /* saved numerator */

#define expr_add_sub(A,B,C) add_or_sub((A),(B),(C),(r==expr_sub))
#define expr_a(A,B) expr_add_sub((A),(B),max_dimen)

@
  The function |add_or_sub(x,y,max_answer,negative)| computes the sum
  (for |negative=false|) or difference (for |negative=true|) of |x| and
  |y|, provided the absolute value of the result does not exceed
  |max_answer|.

@c
inline static int add_or_sub(int x, int y, int max_answer, boolean negative)
{
    int a;                      /* the answer */
    if (negative)
        negate(y);
    if (x >= 0) {
        if (y <= max_answer - x)
            a = x + y;
        else
            num_error(a);
    } else if (y >= -max_answer - x) {
        a = x + y;
    } else {
        num_error(a);
    }
    return a;
}

#define expr_m(A) A = nx_plus_y((A),f,0)
#define expr_d(A) A=quotient((A),f)

@ The function |quotient(n,d)| computes the rounded quotient
$q=\lfloor n/d+{1\over2}\rfloor$, when $n$ and $d$ are positive.

@c
inline static int quotient(int n, int d)
{
    boolean negative;           /* should the answer be negated? */
    int a;                      /* the answer */
    if (d == 0) {
        num_error(a);
    } else {
        if (d > 0) {
            negative = false;
        } else {
            negate(d);
            negative = true;
        }
        if (n < 0) {
            negate(n);
            negative = !negative;
        }
        a = n / d;
        n = n - a * d;
        d = n - d;              /* avoid certain compiler optimizations! */
        if (d + n >= 0)
            incr(a);
        if (negative)
            negate(a);
    }
    return a;
}

#define expr_s(A) A=fract((A),n,f,max_dimen)

@ Finally, the function |fract(x,n,d,max_answer)| computes the integer
$q=\lfloor xn/d+{1\over2}\rfloor$, when $x$, $n$, and $d$ are positive
and the result does not exceed |max_answer|.  We can't use floating
point arithmetic since the routine must produce identical results in all
cases; and it would be too dangerous to multiply by~|n| and then divide
by~|d|, in separate operations, since overflow might well occur.  Hence
this subroutine simulates double precision arithmetic, somewhat
analogous to Metafont's |make_fraction| and |take_fraction| routines.

@c
int fract(int x, int n, int d, int max_answer)
{
    boolean negative;           /* should the answer be negated? */
    int a;                      /* the answer */
    int f;                      /* a proper fraction */
    int h;                      /* smallest integer such that |2*h>=d| */
    int r;                      /* intermediate remainder */
    int t;                      /* temp variable */
    if (d == 0)
        goto TOO_BIG;
    a = 0;
    if (d > 0) {
        negative = false;
    } else {
        negate(d);
        negative = true;
    }
    if (x < 0) {
        negate(x);
        negative = !negative;
    } else if (x == 0) {
        goto DONE;
    }
    if (n < 0) {
        negate(n);
        negative = !negative;
    }
    t = n / d;
    if (t > max_answer / x)
        goto TOO_BIG;
    a = t * x;
    n = n - t * d;
    if (n == 0)
        goto FOUND;
    t = x / d;
    if (t > (max_answer - a) / n)
        goto TOO_BIG;
    a = a + t * n;
    x = x - t * d;
    if (x == 0)
        goto FOUND;
    if (x < n) {
        t = x;
        x = n;
        n = t;
    }
    /* now |0<n<=x<d| */
    /* Compute $f=\lfloor xn/d+{1\over2}\rfloor$; */
    /* The loop here preserves the following invariant relations
       between |f|, |x|, |n|, and~|r|:
       (i)~$f+\lfloor(xn+(r+d))/d\rfloor=\lfloor x_0n_0/d+{1\over2}\rfloor$;
       (ii)~|-d<=r<0<n<=x<d|, where $x_0$, $n_0$ are the original values of~$x$
       and $n$. */
    /* Notice that the computation specifies |(x-d)+x| instead of |(x+x)-d|,
       because the latter could overflow. */
    f = 0;
    r = (d / 2) - d;
    h = -r;
    while (1) {
        if (odd(n)) {
            r = r + x;
            if (r >= 0) {
                r = r - d;
                incr(f);
            }
        }
        n = n / 2;
        if (n == 0)
            break;
        if (x < h) {
            x = x + x;
        } else {
            t = x - d;
            x = t + x;
            f = f + n;
            if (x < n) {
                if (x == 0)
                    break;
                t = x;
                x = n;
                n = t;
            }
        }
    }

    if (f > (max_answer - a))
        goto TOO_BIG;
    a = a + f;
  FOUND:
    if (negative)
        negate(a);
    goto DONE;
  TOO_BIG:
    num_error(a);
  DONE:
    return a;
}

@ @c
static void scan_expr(void)
{                               /* scans and evaluates an expression */
    boolean a, b;               /* saved values of |arith_error| */
    int l;                      /* type of expression */
    int r;                      /* state of expression so far */
    int s;                      /* state of term so far */
    int o;                      /* next operation or type of next factor */
    int e;                      /* expression so far */
    int t;                      /* term so far */
    int f;                      /* current factor */
    int n;                      /* numerator of combined multiplication and division */
    halfword p;                 /* top of expression stack */
    halfword q;                 /* for stack manipulations */
    l = cur_val_level;
    a = arith_error;
    b = false;
    p = null;
    /* Scan and evaluate an expression |e| of type |l| */
  RESTART:
    r = expr_none;
    e = 0;
    s = expr_none;
    t = 0;
    n = 0;
  CONTINUE:
    if (s == expr_none)
        o = l;
    else
        o = int_val_level;
    /* Scan a factor |f| of type |o| or start a subexpression */
    /* Get the next non-blank non-call token */
    do {
        get_x_token();
    } while (cur_cmd == spacer_cmd);

    if (cur_tok == other_token + '(') {
        /* Push the expression stack and |goto restart| */
        q = new_node(expr_node, 0);
        vlink(q) = p;
        expr_type(q) = (quarterword) l;
        expr_state(q) = (quarterword) (4 * s + r);
        expr_e_field(q) = e;
        expr_t_field(q) = t;
        expr_n_field(q) = n;
        p = q;
        l = o;
        goto RESTART;
    }
    back_input();
    if ((o == int_val_level) || (o == attr_val_level))
        scan_int();
    else if (o == dimen_val_level)
        scan_normal_dimen();
    else if (o == glue_val_level)
        scan_normal_glue();
    else
        scan_mu_glue();
    f = cur_val;

  FOUND:
    /* Scan the next operator and set |o| */
    /* Get the next non-blank non-call token */
    do {
        get_x_token();
    } while (cur_cmd == spacer_cmd);

    if (cur_tok == other_token + '+') {
        o = expr_add;
    } else if (cur_tok == other_token + '-') {
        o = expr_sub;
    } else if (cur_tok == other_token + '*') {
        o = expr_mult;
    } else if (cur_tok == other_token + '/') {
        o = expr_div;
    } else {
        o = expr_none;
        if (p == null) {
            if (cur_cmd != relax_cmd)
                back_input();
        } else if (cur_tok != other_token + ')') {
            print_err("Missing ) inserted for expression");
            help1("I was expecting to see `+', `-', `*', `/', or `)'. Didn't.");
            back_error();
        }
    }

    arith_error = b;
    /* Make sure that |f| is in the proper range */
    if (((l == int_val_level) || (l == attr_val_level)) || (s > expr_sub)) {
        if ((f > infinity) || (f < -infinity))
            num_error(f);
    } else if (l == dimen_val_level) {
        if (abs(f) > max_dimen)
            num_error(f);
    } else {
        if ((abs(width(f)) > max_dimen) || (abs(stretch(f)) > max_dimen) || (abs(shrink(f)) > max_dimen))
            glue_error(f);
    }

    switch (s) {
        /* Cases for evaluation of the current term */
    case expr_none:
        /*
           Applying the factor |f| to the partial term |t| (with the operator
           |s|) is delayed until the next operator |o| has been scanned.  Here we
           handle the first factor of a partial term.  A glue spec has to be copied
           unless the next operator is a right parenthesis; this allows us later on
           to simply modify the glue components.
         */
        t = f;
        if ((l >= glue_val_level) && (o != expr_none)) {
	        /* do we really need to copy here ? */ 
            t = new_spec(f);
            flush_node(f);
            normalize_glue(t);
        } else {
            t = f;
        }
        break;
    case expr_mult:
        /* If a multiplication is followed by a division, the two operations are
           combined into a `scaling' operation.  Otherwise the term |t| is
           multiplied by the factor |f|. */
        if (o == expr_div) {
            n = f;
            o = expr_scale;
        } else if ((l == int_val_level) || (l == attr_val_level)) {
            t = mult_integers(t, f);
        } else if (l == dimen_val_level) {
            expr_m(t);
        } else {
            expr_m(width(t));
            expr_m(stretch(t));
            expr_m(shrink(t));
        }
        break;
    case expr_div:
        /* Here we divide the term |t| by the factor |f| */
        if (l < glue_val_level) {
            expr_d(t);
        } else {
            expr_d(width(t));
            expr_d(stretch(t));
            expr_d(shrink(t));
        }
        break;
    case expr_scale:
        /* Here the term |t| is multiplied by the quotient $n/f$. */
        if ((l == int_val_level) || (l == attr_val_level)) {
            t = fract(t, n, f, infinity);
        } else if (l == dimen_val_level) {
            expr_s(t);
        } else {
            expr_s(width(t));
            expr_s(stretch(t));
            expr_s(shrink(t));
        }
        break;
    }                           /* there are no other cases */
    if (o > expr_sub) {
        s = o;
    } else {
        /* Evaluate the current expression */
        /* When a term |t| has been completed it is copied to, added to, or
           subtracted from the expression |e|. */
        s = expr_none;
        if (r == expr_none) {
            e = t;
        } else if ((l == int_val_level) || (l == attr_val_level)) {
            e = expr_add_sub(e, t, infinity);
        } else if (l == dimen_val_level) {
            e = expr_a(e, t);
        } else {
            /* Compute the sum or difference of two glue specs */
            /* We know that |stretch_order(e)>normal| implies |stretch(e)<>0| and
               |shrink_order(e)>normal| implies |shrink(e)<>0|. */
            width(e) = expr_a(width(e), width(t));
            if (stretch_order(e) == stretch_order(t)) {
                stretch(e) = expr_a(stretch(e), stretch(t));
            } else if ((stretch_order(e) < stretch_order(t)) && (stretch(t) != 0)) {
                stretch(e) = stretch(t);
                stretch_order(e) = stretch_order(t);
            }
            if (shrink_order(e) == shrink_order(t)) {
                shrink(e) = expr_a(shrink(e), shrink(t));
            } else if ((shrink_order(e) < shrink_order(t)) && (shrink(t) != 0)) {
                shrink(e) = shrink(t);
                shrink_order(e) = shrink_order(t);
            }
            flush_node(t);
            normalize_glue(e);
        }
        r = o;
    }
    b = arith_error;
    if (o != expr_none)
        goto CONTINUE;
    if (p != null) {
        /* Pop the expression stack and |goto found| */
        f = e;
        q = p;
        e = expr_e_field(q);
        t = expr_t_field(q);
        n = expr_n_field(q);
        s = expr_state(q) / 4;
        r = expr_state(q) % 4;
        l = expr_type(q);
        p = vlink(q);
        flush_node(q);
        goto FOUND;
    }

    if (b) {
        print_err("Arithmetic overflow");
        help2("I can't evaluate this expression,",
              "since the result is out of range.");
        error();
        if (l >= glue_val_level) {
            reset_glue_to_zero(e);
        } else {
            e = 0;
        }
    }
    arith_error = a;
    cur_val = e;
    cur_val_level = l;
}
