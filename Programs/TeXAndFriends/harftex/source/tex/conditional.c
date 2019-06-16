/*

conditional.w

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

/*tex

@* We consider now the way \TeX\ handles various kinds of \.{\\if} commands.

Conditions can be inside conditions, and this nesting has a stack that is
independent of the |save_stack|.

Four global variables represent the top of the condition stack: |cond_ptr| points
to pushed-down entries, if any; |if_limit| specifies the largest code of a
|fi_or_else| command that is syntactically legal; |cur_if| is the name of the
current type of conditional; and |if_line| is the line number at which it began.

If no conditions are currently in progress, the condition stack has the special
state |cond_ptr=null|, |if_limit=normal|, |cur_if=0|, |if_line=0|. Otherwise
|cond_ptr| points to a two-word node; the |type|, |subtype|, and |link| fields of
the first word contain |if_limit|, |cur_if|, and |cond_ptr| at the next level,
and the second word contains the corresponding |if_line|.

In |cond_ptr| we keep track of the top of the condition stack while |if_limit|
holds the upper bound on |fi_or_else| codes. The type of conditional being worked
on is stored in cur_if and |if_line| keeps track of the line where that
conditional began. When we skip conditional text, |skip_line| keeps track of the
line number where skipping began, for use in error messages.

*/

halfword cond_ptr;
int if_limit, cur_if, if_line, skip_line;

/*tex

Here is a procedure that ignores text until coming to an \.{\\or}, \.{\\else}, or
\.{\\fi} at level zero of $\.{\\if}\ldots\.{\\fi}$ nesting. After it has acted,
|cur_chr| will indicate the token that was found, but |cur_tok| will not be set
(because this makes the procedure run faster).

With |l| we keep track of the level of $\.{\\if}\ldots\.{\\fi}$ nesting and
|scanner_status| let us return to the entry status.

*/

void pass_text(void)
{
    int l = 0;
    int save_scanner_status = scanner_status;
    scanner_status = skipping;
    skip_line = line;
    while (1) {
        get_next();
        if (cur_cmd == fi_or_else_cmd) {
            if (l == 0)
                break;
            if (cur_chr == fi_code)
                decr(l);
        } else if (cur_cmd == if_test_cmd) {
            incr(l);
        }
    }
    scanner_status = save_scanner_status;
    if (tracing_ifs_par > 0)
        show_cur_cmd_chr();
}

/*tex

When we begin to process a new \.{\\if}, we set |if_limit:=if_code|; then if\/
\.{\\or} or \.{\\else} or \.{\\fi} occurs before the current \.{\\if} condition
has been evaluated, \.{\\relax} will be inserted. For example, a sequence of
commands like `\.{\\ifvoid1\\else...\\fi}' would otherwise require something
after the `\.1'.

*/

void push_condition_stack(void)
{
    halfword p = new_node(if_node, 0);
    vlink(p) = cond_ptr;
    if_limit_type(p) = (quarterword) if_limit;
    if_limit_subtype(p) = (quarterword) cur_if;
    if_line_field(p) = if_line;
    cond_ptr = p;
    cur_if = cur_chr;
    if_limit = if_code;
    if_line = line;
}

void pop_condition_stack(void)
{
    halfword p;
    if (if_stack[in_open] == cond_ptr) {
        /*tex Conditionals are possibly not properly nested with files. */
        if_warning();
    }
    p = cond_ptr;
    if_line = if_line_field(p);
    cur_if = if_limit_subtype(p);
    if_limit = if_limit_type(p);
    cond_ptr = vlink(p);
    flush_node(p);
}

/*tex

Here's a procedure that changes the |if_limit| code corresponding to a given
value of |cond_ptr|.

*/

void change_if_limit(int l, halfword p)
{
    if (p == cond_ptr) {
        if_limit = l;
    } else {
        halfword q = cond_ptr;
        while (1) {
            if (q == null)
                confusion("if");
            if (vlink(q) == p) {
                if_limit_type(q) = (quarterword) l;
                return;
            }
            q = vlink(q);
        }
    }
}

/*tex

The conditional \.{\\ifcsname} is equivalent to \.{\\expandafter}
\.{\\expandafter} \.{\\ifdefined} \.{\\csname}, except that no new control
sequence will be entered into the hash table (once all tokens preceding the
mandatory \.{\\endcsname} have been expanded).

*/

static halfword last_tested_cs ;

static boolean test_for_cs(void)
{
    /*tex Is the condition true? */
    boolean b = false;
    /*tex To be tested against the second operand: */
    int m, s;
    /*tex For traversing token lists in \.{\\ifx} tests: */
    halfword q;
    halfword n = get_avail();
    /*tex Head of the list of characters: */
    halfword p = n;
    is_in_csname += 1;
    while (1) {
        get_x_token();
        if (cur_cs != 0)
            break;
        store_new_token(cur_tok);
    }
    if (cur_cmd != end_cs_name_cmd) {
        last_tested_cs = null_cs;
        if (suppress_ifcsname_error_par) {
            do {
                get_x_token();
            } while (cur_cmd != end_cs_name_cmd);
            flush_list(n);
            is_in_csname -= 1;
            return b;
        } else {
            complain_missing_csname();
        }
    }
    /*tex Look up the characters of list |n| in the hash table, and set |cur_cs|. */
    m = first;
    p = token_link(n);
    while (p != null) {
        if (m >= max_buf_stack) {
            max_buf_stack = m + 4;
            if (max_buf_stack >= buf_size)
                check_buffer_overflow(max_buf_stack);
        }
        s = token_chr(token_info(p));
        if (s <= 0x7F) {
            buffer[m++] = (packed_ASCII_code) s;
        } else if (s <= 0x7FF) {
            buffer[m++] = (packed_ASCII_code) (0xC0 + s / 0x40);
            buffer[m++] = (packed_ASCII_code) (0x80 + s % 0x40);
        } else if (s <= 0xFFFF) {
            buffer[m++] = (packed_ASCII_code) (0xE0 + s / 0x1000);
            buffer[m++] = (packed_ASCII_code) (0x80 + (s % 0x1000) / 0x40);
            buffer[m++] = (packed_ASCII_code) (0x80 + (s % 0x1000) % 0x40);
        } else {
            buffer[m++] = (packed_ASCII_code) (0xF0 + s / 0x40000);
            buffer[m++] = (packed_ASCII_code) (0x80 + (s % 0x40000) / 0x1000);
            buffer[m++] = (packed_ASCII_code) (0x80 + ((s % 0x40000) % 0x1000) / 0x40);
            buffer[m++] = (packed_ASCII_code) (0x80 + ((s % 0x40000) % 0x1000) % 0x40);
        }
        p = token_link(p);
    }
    if (m > first) {
        /*tex |no_new_control_sequence| is |true| */
        cur_cs = id_lookup(first, m - first);
    } else if (m == first) {
        /*tex the list is empty */
        cur_cs = null_cs;
    }
    b = (eq_type(cur_cs) != undefined_cs_cmd);
    flush_list(n);
    last_cs_name = cur_cs;
    is_in_csname -= 1;
    return b;
}

/*tex

An active character will be treated as category 13 following \.{\\if\\noexpand}
or following \.{\\ifcat\\noexpand}.

*/

#define get_x_token_or_active_char() do { \
    get_x_token(); \
    if (cur_cmd==relax_cmd && cur_chr==no_expand_flag) { \
        if (is_active_cs(cs_text(cur_cs))) { \
            cur_cmd=active_char_cmd; \
            cur_chr=active_cs_value(cs_text(cur_tok-cs_token_flag)); \
        } \
    } \
} while (0)

/*tex

A condition is started when the |expand| procedure encounters an |if_test|
command; in that case |expand| reduces to |conditional|, which is a recursive
procedure. @^recursion@>

*/

void conditional(void)
{
    /*tex Is the condition true? */
    boolean b = false;
    /*tex The relation to be evaluated: */
    int r;
    /*tex To be tested against the second operand: */
    int m, n;
    /*tex For traversing token lists in \.{\\ifx} tests: */
    halfword p, q;
    /*tex The |scanner_status| upon entry: */
    int save_scanner_status;
    /*tex The |cond_ptr| corresponding to this conditional: */
    halfword save_cond_ptr;
    /*tex The type of this conditional: */
    int this_if;
    /*tex Was this \.{\\if} preceded by \.{\\unless}? */
    boolean is_unless;
    if ((tracing_ifs_par > 0) && (tracing_commands_par <= 1)) {
        show_cur_cmd_chr();
    }
    push_condition_stack();
    save_cond_ptr = cond_ptr;
    is_unless = (cur_chr >= unless_code);
    this_if = cur_chr % unless_code;
    /*tex Either process \.{\\ifcase} or set |b| to the value of a boolean condition. */
    switch (this_if) {
        case if_char_code:
        case if_cat_code:
            /*tex Test if two characters match. */
            get_x_token_or_active_char();
            if ((cur_cmd > active_char_cmd) || (cur_chr > biggest_char)) {
                /*tex It's not a character. */
                m = relax_cmd;
                n = too_big_char;
            } else {
                m = cur_cmd;
                n = cur_chr;
            }
            get_x_token_or_active_char();
            if ((cur_cmd > active_char_cmd) || (cur_chr > biggest_char)) {
                cur_cmd = relax_cmd;
                cur_chr = too_big_char;
            }
            if (this_if == if_char_code)
                b = (n == cur_chr);
            else
                b = (m == cur_cmd);
            break;
        case if_int_code:
        case if_dim_code:
        case if_abs_dim_code:
        case if_abs_num_code:
            /*tex
                Test the relation between integers or dimensions. Here we use the fact
                that |<|, |=|, and |>| are consecutive ASCII codes.
            */
            if (this_if == if_int_code || this_if == if_abs_num_code)
                scan_int();
            else
                scan_normal_dimen();
            n = cur_val;
            if ((n < 0) && (this_if == if_abs_dim_code || this_if == if_abs_num_code))
                negate(n);
            /*tex Get the next non-blank non-call... */
            do {
                get_x_token();
            } while (cur_cmd == spacer_cmd);
            r = cur_tok - other_token;
            if ((r < '<') || (r > '>')) {
                print_err("Missing = inserted for ");
                print_cmd_chr(if_test_cmd, this_if);
                help1("I was expecting to see `<', `=', or `>'. Didn't.");
                back_error();
                r = '=';
            }
            if (this_if == if_int_code || this_if == if_abs_num_code)
                scan_int();
            else
                scan_normal_dimen();
            if ((cur_val < 0) && (this_if == if_abs_dim_code || this_if == if_abs_num_code))
                negate(cur_val);
            switch (r) {
                case '<':
                    b = (n < cur_val);
                    break;
                case '=':
                    b = (n == cur_val);
                    break;
                case '>':
                    b = (n > cur_val);
                    break;
                default:
                    /*tex This can't happen. */
                    b = false;
                    break;
            }
            break;
        case if_odd_code:
            /*tex Test if an integer is odd. */
            scan_int();
            b = odd(cur_val);
            break;
        case if_vmode_code:
            b = (abs(cur_list.mode_field) == vmode);
            break;
        case if_hmode_code:
            b = (abs(cur_list.mode_field) == hmode);
            break;
        case if_mmode_code:
            b = (abs(cur_list.mode_field) == mmode);
            break;
        case if_inner_code:
            b = (cur_list.mode_field < 0);
            break;
        case if_void_code:
            scan_register_num();
            p = box(cur_val);
            b = (p == null);
            break;
        case if_hbox_code:
            scan_register_num();
            p = box(cur_val);
            b = (p != null) && (type(p) == hlist_node);
            break;
        case if_vbox_code:
            scan_register_num();
            p = box(cur_val);
            b = (p != null) && (type(p) == vlist_node);
            break;
        case if_x_code:
            /*tex
                Test if two tokens match. Note that `\.{\\ifx}' will declare two
                macros different if one is \\{long} or \\{outer} and the other
                isn't, even though the texts of the macros are the same.

                We need to reset |scanner_status|, since \.{\\outer} control
                sequences are allowed, but we might be scanning a macro
                definition or preamble.
             */
            save_scanner_status = scanner_status;
            scanner_status = normal;
            get_next();
            n = cur_cs;
            p = cur_cmd;
            q = cur_chr;
            get_next();
            if (cur_cmd != p) {
                b = false;
            } else if (cur_cmd < call_cmd) {
                b = (cur_chr == q);
            } else {
                /*tex
                    Test if two macro texts match. Note also that `\.{\\ifx}'
                    decides that macros \.{\\a} and \.{\\b} are different in
                    examples like this:

                    $$\vbox{\halign{\.{#}\hfil&\qquad\.{#}\hfil\cr
                    {}\\def\\a\{\\c\}&
                    {}\\def\\c\{\}\cr
                    {}\\def\\b\{\\d\}&
                    {}\\def\\d\{\}\cr}}$$
                */
                p = token_link(cur_chr);
                /*tex Omit reference counts. */
                q = token_link(equiv(n));
                if (p == q) {
                    b = true;
                } else {
                    while ((p != null) && (q != null)) {
                        if (token_info(p) != token_info(q)) {
                            p = null;
                            break;
                        } else {
                            p = token_link(p);
                            q = token_link(q);
                        }
                    }
                    b = ((p == null) && (q == null));
                }
            }
            scanner_status = save_scanner_status;
            break;
        case if_eof_code:
            scan_four_bit_int();
            b = (read_open[cur_val] == closed);
            break;
        case if_true_code:
            b = true;
            break;
        case if_false_code:
            b = false;
            break;
        case if_case_code:
            /*tex Select the appropriate case and |return| or |goto common_ending|. */
            scan_int();
            /*tex |n| is the number of cases to pass. */
            n = cur_val;
            if (tracing_commands_par > 1) {
                begin_diagnostic();
                tprint("{case ");
                print_int(n);
                print_char('}');
                end_diagnostic(false);
            }
            while (n != 0) {
                pass_text();
                if (cond_ptr == save_cond_ptr) {
                    if (cur_chr == or_code)
                        decr(n);
                    else
                        goto COMMON_ENDING;
                } else if (cur_chr == fi_code) {
                    pop_condition_stack();
                }
            }
            change_if_limit(or_code, save_cond_ptr);
            /*tex Wait for \.{\\or}, \.{\\else}, or \.{\\fi}. */
            return;
            break;
        case if_primitive_code:
            save_scanner_status = scanner_status;
            scanner_status = normal;
            get_next();
            scanner_status = save_scanner_status;
            m = prim_lookup(cs_text(cur_cs));
            b = ((cur_cmd != undefined_cs_cmd) &&
                 (m != undefined_primitive) &&
                 (cur_cmd == get_prim_eq_type(m)) &&
                 (cur_chr == get_prim_equiv(m)));
            break;
        case if_def_code:
            /*tex
                The conditional \.{\\ifdefined} tests if a control sequence is
                defined. We need to reset |scanner_status|, since \.{\\outer}
                control sequences are allowed, but we might be scanning a macro
                definition or preamble.
            */
            save_scanner_status = scanner_status;
            scanner_status = normal;
            get_next();
            b = (cur_cmd != undefined_cs_cmd);
            scanner_status = save_scanner_status;
            break;
        case if_cs_code:
            b = test_for_cs();
            break;
        case if_in_csname_code:
            b = is_in_csname;
            break;
        case if_font_char_code:
            /*tex
                The conditional \.{\\iffontchar} tests the existence of a
                character in a font.
            */
            scan_font_ident();
            n = cur_val;
            scan_char_num();
            b = char_exists(n, cur_val);
            break;
        default:
            /*tex there are no other cases, but we need to please |-Wall|. */
            b = false;
    }
    if (is_unless)
        b = !b;
    if (tracing_commands_par > 1) {
        /*tex Display the value of |b|. */
        begin_diagnostic();
        if (b)
            tprint("{true}");
        else
            tprint("{false}");
        end_diagnostic(false);
    }
    if (b) {
        change_if_limit(else_code, save_cond_ptr);
        /*tex Wait for \.{\\else} or \.{\\fi}. */
        return;
    }
    /*tex
        Skip to \.{\\else} or \.{\\fi}, then |goto common_ending|. In a
        construction like `\.{\\if\\iftrue abc\\else d\\fi}', the first
        \.{\\else} that we come to after learning that the \.{\\if} is false is
        not the \.{\\else} we're looking for. Hence the following curious logic
        is needed.
     */
    while (1) {
        pass_text();
        if (cond_ptr == save_cond_ptr) {
            if (cur_chr != or_code)
                goto COMMON_ENDING;
            print_err("Extra \\or");
            help1(
                "I'm ignoring this; it doesn't match any \\if."
            );
            error();
        } else if (cur_chr == fi_code) {
            pop_condition_stack();
        }
    }
  COMMON_ENDING:
    if (cur_chr == fi_code) {
        pop_condition_stack();
    } else {
        /*tex Wait for \.{\\fi}. */
        if_limit = fi_code;
    }
}
