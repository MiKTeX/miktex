% expand.w
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

@ Only a dozen or so command codes |>max_command| can possibly be returned by
|get_next|; in increasing order, they are |undefined_cs|, |expand_after|,
|no_expand|, |input|, |if_test|, |fi_or_else|, |cs_name|, |convert|, |the|,
|top_bot_mark|, |call|, |long_call|, |outer_call|, |long_outer_call|, and
|end_template|.{\emergencystretch=40pt\par}

Sometimes, recursive calls to the following |expand| routine may
cause exhaustion of the run-time calling stack, resulting in
forced execution stops by the operating system. To diminish the chance
of this happening, a counter is used to keep track of the recursion
depth, in conjunction with a constant called |expand_depth|.

Note that this does not catch all possible infinite recursion loops,
just the ones that exhaust the application calling stack. The
actual maximum value of |expand_depth| is outside of our control, but
the initial setting of |100| should be enough to prevent problems.
@^system dependencies@>

@c
static int expand_depth_count = 0;


@ The |expand| subroutine is used when |cur_cmd>max_command|. It removes a
``call'' or a conditional or one of the other special operations just
listed.  It follows that |expand| might invoke itself recursively. In all
cases, |expand| destroys the current token, but it sets things up so that
the next |get_next| will deliver the appropriate next token. The value of
|cur_tok| need not be known when |expand| is called.

Since several of the basic scanning routines communicate via global variables,
their values are saved as local variables of |expand| so that
recursive calls don't invalidate them.
@^recursion@>

@c
int is_in_csname = 0;

@ @c
void expand(void)
{
    halfword t;                 /* token that is being ``expanded after'' */
    halfword p;                 /* for list manipulation */
    halfword cur_ptr;           /* for a local token list pointer */
    int cv_backup;              /* to save the global quantity |cur_val| */
    int cvl_backup, radix_backup, co_backup;    /* to save |cur_val_level|, etc. */
    halfword backup_backup;     /* to save |link(backup_head)| */
    int save_scanner_status;    /* temporary storage of |scanner_status| */
    incr(expand_depth_count);
    if (expand_depth_count >= expand_depth)
        overflow("expansion depth", (unsigned) expand_depth);
    cv_backup = cur_val;
    cvl_backup = cur_val_level;
    radix_backup = radix;
    co_backup = cur_order;
    backup_backup = token_link(backup_head);
  RESWITCH:
    if (cur_cmd < call_cmd) {
        /* Expand a nonmacro */
        if (int_par(tracing_commands_code) > 1)
            show_cur_cmd_chr();
        switch (cur_cmd) {
        case top_bot_mark_cmd:
            /* Insert the appropriate mark text into the scanner */
            t = cur_chr % marks_code;
            if (cur_chr >= marks_code)
                scan_mark_num();
            else
                cur_val = 0;
            switch (t) {
            case first_mark_code:
                cur_ptr = first_mark(cur_val);
                break;
            case bot_mark_code:
                cur_ptr = bot_mark(cur_val);
                break;
            case split_first_mark_code:
                cur_ptr = split_first_mark(cur_val);
                break;
            case split_bot_mark_code:
                cur_ptr = split_bot_mark(cur_val);
                break;
            default:
                cur_ptr = top_mark(cur_val);
                break;
            }
            if (cur_ptr != null)
                begin_token_list(cur_ptr, mark_text);
            break;
        case expand_after_cmd:
            if (cur_chr == 0) {
                /* Expand the token after the next token */
                /* It takes only a little shuffling to do what \TeX\ calls \.{\\expandafter}. */
                get_token();
                t = cur_tok;
                get_token();
                if (cur_cmd > max_command_cmd)
                    expand();
                else
                    back_input();
                cur_tok = t;
                back_input();

            } else {            /* \\unless */
                /* Negate a boolean conditional and |goto reswitch| */
                /* The result of a boolean condition is reversed when the conditional is
                   preceded by \.{\\unless}. */
                get_token();
                if ((cur_cmd == if_test_cmd) && (cur_chr != if_case_code)) {
                    cur_chr = cur_chr + unless_code;
                    goto RESWITCH;
                }
                print_err("You can't use `\\unless' before `");
                print_cmd_chr((quarterword) cur_cmd, cur_chr);
                print_char('\'');
                help1("Continue, and I'll forget that it ever happened.");
                back_error();
            }
            break;
        case no_expand_cmd:
            if (cur_chr == 0) {
                /* Suppress expansion of the next token */
                /* The implementation of \.{\\noexpand} is a bit trickier, because it is
                   necessary to insert a special `|dont_expand|' marker into \TeX's reading
                   mechanism.  This special marker is processed by |get_next|, but it does
                   not slow down the inner loop.

                   Since \.{\\outer} macros might arise here, we must also
                   clear the |scanner_status| temporarily.
                 */

                save_scanner_status = scanner_status;
                scanner_status = normal;
                get_token();
                scanner_status = save_scanner_status;
                t = cur_tok;
                back_input();   /* now |start| and |loc| point to the backed-up token |t| */
                if (t >= cs_token_flag) {
                    p = get_avail();
                    set_token_info(p, cs_token_flag + frozen_dont_expand);
                    set_token_link(p, iloc);
                    istart = p;
                    iloc = p;
                }

            } else {
                /* Implement \.{\\primitive} */
                /*
                   The \.{\\primitive} handling. If the primitive meaning of the next
                   token is an expandable command, it suffices to replace the current
                   token with the primitive one and restart |expand|.

                   Otherwise, the token we just read has to be pushed back, as well
                   as a token matching the internal form of \.{\\primitive}, that is
                   sneaked in as an alternate form of |ignore_spaces|.

                   An implementation problem surfaces: There really is no |cur_cs|
                   attached to the inserted primitive command, so it is safer to set
                   |cur_cs| to zero.  |cur_tok| has a similar problem. And for the
                   non-expanded branch, simply pushing back a token that matches the
                   correct internal command does not work, because that approach would
                   not survive roundtripping to a temporary file or even a token list.

                   In a next version, it would be smart to create |frozen_| versions of
                   all the primitives.  Then, this problem would not happen, at the
                   expense of a few hundred extra control sequences.
                 */
                save_scanner_status = scanner_status;
                scanner_status = normal;
                get_token();
                scanner_status = save_scanner_status;
                cur_cs = prim_lookup(cs_text(cur_cs));
                if (cur_cs != undefined_primitive) {
                    t = get_prim_eq_type(cur_cs);
                    if (t > max_command_cmd) {
                        cur_cmd = t;
                        cur_chr = get_prim_equiv(cur_cs);
                        cur_tok = token_val(cur_cmd, cur_chr);
                        cur_cs = 0;
                        goto RESWITCH;
                    } else {
                        back_input();   /*  now |loc| and |start| point to a one-item list */
                        p = get_avail();
                        set_token_info(p, cs_token_flag + frozen_primitive);
                        set_token_link(p, iloc);
                        iloc = p;
                        istart = p;
                    }
                } else {
                    print_err("Missing primitive name");
                    help2
                        ("The control sequence marked <to be read again> does not",
                         "represent any known primitive.");
                    back_error();
                }

            }
            break;
        case cs_name_cmd:
            /* Manufacture a control sequence name; */
            if (cur_chr == 0) {
                manufacture_csname(0);
            } else if (cur_chr == 1) {
                inject_last_tested_cs();
            } else {
                manufacture_csname(1);
            }
            break;
        case convert_cmd:
            conv_toks();        /* this procedure is discussed in Part 27 below */
            break;
        case the_cmd:
            ins_the_toks();     /* this procedure is discussed in Part 27 below */
            break;
        case combine_toks_cmd:
            combine_the_toks(cur_chr);
            break;
        case if_test_cmd:
            conditional();      /* this procedure is discussed in Part 28 below */
            break;
        case fi_or_else_cmd:
            /* Terminate the current conditional and skip to \.{\\fi} */
            /* The processing of conditionals is complete except for the following
               code, which is actually part of |expand|. It comes into play when
               \.{\\or}, \.{\\else}, or \.{\\fi} is scanned. */

            if (int_par(tracing_ifs_code) > 0)
                if (int_par(tracing_commands_code) <= 1)
                    show_cur_cmd_chr();
            if (cur_chr > if_limit) {
                if (if_limit == if_code) {
                    insert_relax();     /*  condition not yet evaluated */
                } else {
                    print_err("Extra ");
                    print_cmd_chr(fi_or_else_cmd, cur_chr);
                    help1("I'm ignoring this; it doesn't match any \\if.");
                    error();
                }
            } else {
                while (cur_chr != fi_code)
                    pass_text();        /* skip to \.{\\fi} */
                pop_condition_stack();
            }

            break;
        case input_cmd:
            /* Initiate or terminate input from a file */
            if (cur_chr == 1)
                force_eof = true;
            else if (cur_chr == 2)
                pseudo_start();
            else if (cur_chr == 3) {
                pseudo_start();
                iname = 19;
            } else if (name_in_progress)
                insert_relax();
            else
                start_input();
            break;
        case variable_cmd:
            do_variable();
            break;
        case feedback_cmd:
            do_feedback();
            break;
        default:
            /* Complain about an undefined macro */
            print_err("Undefined control sequence");
            help5("The control sequence at the end of the top line",
                  "of your error message was never \\def'ed. If you have",
                  "misspelled it (e.g., `\\hobx'), type `I' and the correct",
                  "spelling (e.g., `I\\hbox'). Otherwise just continue,",
                  "and I'll forget about whatever was undefined.");
            error();
            break;
        }
    } else if (cur_cmd < end_template_cmd) {
        macro_call();
    } else {
        /* Insert a token containing |frozen_endv| */
        /* An |end_template| command is effectively changed to an |endv| command
           by the following code. (The reason for this is discussed below; the
           |frozen_end_template| at the end of the template has passed the
           |check_outer_validity| test, so its mission of error detection has been
           accomplished.)
         */
        cur_tok = cs_token_flag + frozen_endv;
        back_input();

    }
    cur_val = cv_backup;
    cur_val_level = cvl_backup;
    radix = radix_backup;
    cur_order = co_backup;
    set_token_link(backup_head, backup_backup);
    decr(expand_depth_count);
}

@ @c
void complain_missing_csname(void)
{
    print_err("Missing \\endcsname inserted");
    help2("The control sequence marked <to be read again> should",
          "not appear between \\csname and \\endcsname.");
    back_error();
}

@ @c
void manufacture_csname(boolean use)
{
    halfword p, q, r;
    lstring *ss;
    r = get_avail();
    p = r;                      /* head of the list of characters */
    is_in_csname += 1;
    do {
        get_x_token();
        if (cur_cs == 0)
            store_new_token(cur_tok);
    } while (cur_cs == 0);
    if (cur_cmd != end_cs_name_cmd) {
        /* Complain about missing \.{\\endcsname} */
        complain_missing_csname();
    }
    /* Look up the characters of list |r| in the hash table, and set |cur_cs| */
    ss = tokenlist_to_lstring(r, true);
    is_in_csname -= 1;
    if (use) {
        if (ss->l > 0) {
            cur_cs = string_lookup((char *) ss->s, ss->l);
        } else {
            cur_cs = null_cs;
        }
        last_cs_name = cur_cs ;
        free_lstring(ss);
        flush_list(r);
        if (cur_cs == null_cs) {
            /* skip */
        } else if (eq_type(cur_cs) == undefined_cs_cmd) {
            /* skip */
        } else {
            cur_tok = cur_cs + cs_token_flag;
            back_input();
        }
    } else {
        if (ss->l > 0) {
            no_new_control_sequence = false;
            cur_cs = string_lookup((char *) ss->s, ss->l);
            no_new_control_sequence = true;
        } else {
            cur_cs = null_cs;       /* the list is empty */
        }
        last_cs_name = cur_cs ;
        free_lstring(ss);
        flush_list(r);
        if (eq_type(cur_cs) == undefined_cs_cmd) {
            eq_define(cur_cs, relax_cmd, too_big_char);     /* N.B.: The |save_stack| might change */
        };                          /* the control sequence will now match `\.{\\relax}' */
        cur_tok = cur_cs + cs_token_flag;
        back_input();
    }
}

void inject_last_tested_cs(void)
{
    if (last_cs_name != null_cs) {
        cur_cs = last_cs_name;
        cur_tok = last_cs_name + cs_token_flag;
        back_input();
    }
}

@ Sometimes the expansion looks too far ahead, so we want to insert
a harmless \.{\\relax} into the user's input.

@c
void insert_relax(void)
{
    cur_tok = cs_token_flag + cur_cs;
    back_input();
    cur_tok = cs_token_flag + frozen_relax;
    back_input();
    token_type = inserted;
}


@ Here is a recursive procedure that is \TeX's usual way to get the
next token of input. It has been slightly optimized to take account of
common cases.

@c
void get_x_token(void)
{                               /* sets |cur_cmd|, |cur_chr|, |cur_tok|,  and expands macros */
  RESTART:
    get_next();
    if (cur_cmd <= max_command_cmd)
        goto DONE;
    if (cur_cmd >= call_cmd) {
        if (cur_cmd < end_template_cmd) {
            macro_call();
        } else {
            cur_cs = frozen_endv;
            cur_cmd = endv_cmd;
            goto DONE;          /* |cur_chr=null_list| */
        }
    } else {
        expand();
    }
    goto RESTART;
  DONE:
    if (cur_cs == 0)
        cur_tok = token_val(cur_cmd, cur_chr);
    else
        cur_tok = cs_token_flag + cur_cs;
}


@ The |get_x_token| procedure is equivalent to two consecutive
procedure calls: |get_next; x_token|.

@c
void x_token(void)
{                               /* |get_x_token| without the initial |get_next| */
    while (cur_cmd > max_command_cmd) {
        expand();
        get_next();
   }
    if (cur_cs == 0)
        cur_tok = token_val(cur_cmd, cur_chr);
    else
        cur_tok = cs_token_flag + cur_cs;
}


@ A control sequence that has been \.{\\def}'ed by the user is expanded by
\TeX's |macro_call| procedure.

Before we get into the details of |macro_call|, however, let's consider the
treatment of primitives like \.{\\topmark}, since they are essentially
macros without parameters. The token lists for such marks are kept in five
global arrays of pointers; we refer to the individual entries of these
arrays by symbolic macros |top_mark|, etc. The value of |top_mark(x)|, etc.
is either |null| or a pointer to the reference count of a token list.

The variable |biggest_used_mark| is an aid to try and keep the code
somehwat efficient without too much extra work: it registers the
highest mark class ever instantiated by the user, so the loops
in |fire_up| and |vsplit| do not have to traverse the full range
|0..biggest_mark|.

@c
halfword top_marks_array[(biggest_mark + 1)];
halfword first_marks_array[(biggest_mark + 1)];
halfword bot_marks_array[(biggest_mark + 1)];
halfword split_first_marks_array[(biggest_mark + 1)];
halfword split_bot_marks_array[(biggest_mark + 1)];
halfword biggest_used_mark;

@ @c
void initialize_marks(void)
{
    int i;
    biggest_used_mark = 0;
    for (i = 0; i <= biggest_mark; i++) {
        top_mark(i) = null;
        first_mark(i) = null;
        bot_mark(i) = null;
        split_first_mark(i) = null;
        split_bot_mark(i) = null;
    }
}


@ Now let's consider |macro_call| itself, which is invoked when \TeX\ is
scanning a control sequence whose |cur_cmd| is either |call|, |long_call|,
|outer_call|, or |long_outer_call|.  The control sequence definition
appears in the token list whose reference count is in location |cur_chr|
of |mem|.

The global variable |long_state| will be set to |call| or to |long_call|,
depending on whether or not the control sequence disallows \.{\\par}
in its parameters. The |get_next| routine will set |long_state| to
|outer_call| and emit \.{\\par}, if a file ends or if an \.{\\outer}
control sequence occurs in the midst of an argument.

@c
int long_state;                 /* governs the acceptance of \.{\\par} */

@ The parameters, if any, must be scanned before the macro is expanded.
Parameters are token lists without reference counts. They are placed on
an auxiliary stack called |pstack| while they are being scanned, since
the |param_stack| may be losing entries during the matching process.
(Note that |param_stack| can't be gaining entries, since |macro_call| is
the only routine that puts anything onto |param_stack|, and it
is not recursive.)

@c
halfword pstack[9];             /* arguments supplied to a macro */


@ After parameter scanning is complete, the parameters are moved to the
|param_stack|. Then the macro body is fed to the scanner; in other words,
|macro_call| places the defined text of the control sequence at the
top of\/ \TeX's input stack, so that |get_next| will proceed to read it
next.

The global variable |cur_cs| contains the |eqtb| address of the control sequence
being expanded, when |macro_call| begins. If this control sequence has not been
declared \.{\\long}, i.e., if its command code in the |eq_type| field is
not |long_call| or |long_outer_call|, its parameters are not allowed to contain
the control sequence \.{\\par}. If an illegal \.{\\par} appears, the macro
call is aborted, and the \.{\\par} will be rescanned.

@c
void macro_call(void)
{                               /* invokes a user-defined control sequence */
    halfword r;                 /* current node in the macro's token list */
    halfword p = null;          /* current node in parameter token list being built */
    halfword q;                 /* new node being put into the token list */
    halfword s;                 /* backup pointer for parameter matching */
    halfword t;                 /* cycle pointer for backup recovery */
    halfword u, v;              /* auxiliary pointers for backup recovery */
    halfword rbrace_ptr = null; /* one step before the last |right_brace| token */
    int n = 0;                  /* the number of parameters scanned */
    halfword unbalance;         /* unmatched left braces in current parameter */
    halfword m = 0;             /* the number of tokens or groups (usually) */
    halfword ref_count;         /* start of the token list */
    int save_scanner_status = scanner_status;   /* |scanner_status| upon entry */
    halfword save_warning_index = warning_index;        /* |warning_index| upon entry */
    int match_chr = 0;          /* character used in parameter */
    warning_index = cur_cs;
    ref_count = cur_chr;
    r = token_link(ref_count);
    if (int_par(tracing_macros_code) > 0) {
        /* Show the text of the macro being expanded */
        begin_diagnostic();
        print_ln();
        print_cs(warning_index);
        token_show(ref_count);
        end_diagnostic(false);
    }
    if (token_info(r) == protected_token)
        r = token_link(r);
    if (token_info(r) != end_match_token) {
        /* Scan the parameters and make |link(r)| point to the macro body; but
           |return| if an illegal \.{\\par} is detected */
        /* At this point, the reader will find it advisable to review the explanation
           of token list format that was presented earlier, since many aspects of that
           format are of importance chiefly in the |macro_call| routine.

           The token list might begin with a string of compulsory tokens before the
           first |match| or |end_match|. In that case the macro name is supposed to be
           followed by those tokens; the following program will set |s=null| to
           represent this restriction. Otherwise |s| will be set to the first token of
           a string that will delimit the next parameter.
         */

        scanner_status = matching;
        unbalance = 0;
        long_state = eq_type(cur_cs);
        if (long_state >= outer_call_cmd)
            long_state = long_state - 2;
        do {
            set_token_link(temp_token_head, null);
            if ((token_info(r) >= end_match_token)
                || (token_info(r) < match_token)) {
                s = null;
            } else {
                match_chr = token_info(r) - match_token;
                s = token_link(r);
                r = s;
                p = temp_token_head;
                m = 0;
            }
            /* Scan a parameter until its delimiter string has been found; or, if |s=null|,
               simply scan the delimiter string; */

            /* If |info(r)| is a |match| or |end_match| command, it cannot be equal to
               any token found by |get_token|. Therefore an undelimited parameter---i.e.,
               a |match| that is immediately followed by |match| or |end_match|---will
               always fail the test `|cur_tok=info(r)|' in the following algorithm. */
          CONTINUE:
            get_token();        /* set |cur_tok| to the next token of input */
            if (cur_tok == token_info(r)) {
                /* Advance |r|; |goto found| if the parameter delimiter has been
                   fully matched, otherwise |goto continue| */
                /* A slightly subtle point arises here: When the parameter delimiter ends
                   with `\.{\#\{}', the token list will have a left brace both before and
                   after the |end_match|\kern-.4pt. Only one of these should affect the
                   |align_state|, but both will be scanned, so we must make a correction.
                 */
                r = token_link(r);
                if ((token_info(r) >= match_token)
                    && (token_info(r) <= end_match_token)) {
                    if (cur_tok < left_brace_limit)
                        decr(align_state);
                    goto FOUND;
                } else {
                    goto CONTINUE;
                }

            }
            /* Contribute the recently matched tokens to the current parameter, and
               |goto continue| if a partial match is still in effect; but abort if |s=null| */

            /* When the following code becomes active, we have matched tokens from |s| to
               the predecessor of |r|, and we have found that |cur_tok<>info(r)|. An
               interesting situation now presents itself: If the parameter is to be
               delimited by a string such as `\.{ab}', and if we have scanned `\.{aa}',
               we want to contribute one `\.a' to the current parameter and resume
               looking for a `\.b'. The program must account for such partial matches and
               for others that can be quite complex.  But most of the time we have |s=r|
               and nothing needs to be done.

               Incidentally, it is possible for \.{\\par} tokens to sneak in to certain
               parameters of non-\.{\\long} macros. For example, consider a case like
               `\.{\\def\\a\#1\\par!\{...\}}' where the first \.{\\par} is not followed
               by an exclamation point. In such situations it does not seem appropriate
               to prohibit the \.{\\par}, so \TeX\ keeps quiet about this bending of
               the rules. */

            if (s != r) {
                if (s == null) {
                    /* Report an improper use of the macro and abort */
                    print_err("Use of ");
                    sprint_cs(warning_index);
                    tprint(" doesn't match its definition");
                    help4
                        ("If you say, e.g., `\\def\\a1{...}', then you must always",
                         "put `1' after `\\a', since control sequence names are",
                         "made up of letters only. The macro here has not been",
                         "followed by the required stuff, so I'm ignoring it.");
                    error();
                    goto EXIT;

                } else {
                    t = s;
                    do {
                        store_new_token(token_info(t));
                        incr(m);
                        u = token_link(t);
                        v = s;
                        while (1) {
                            if (u == r) {
                                if (cur_tok != token_info(v)) {
                                    goto DONE;
                                } else {
                                    r = token_link(v);
                                    goto CONTINUE;
                                }
                            }
                            if (token_info(u) != token_info(v))
                                goto DONE;
                            u = token_link(u);
                            v = token_link(v);
                        }
                      DONE:
                        t = token_link(t);
                    } while (t != r);
                    r = s;      /* at this point, no tokens are recently matched */
                }
            }

            if (cur_tok == par_token)
                if (long_state != long_call_cmd)
                    if (!int_par(suppress_long_error_code)) {
                        goto RUNAWAY;
                    }
            if (cur_tok < right_brace_limit) {
                if (cur_tok < left_brace_limit) {
                    /* Contribute an entire group to the current parameter */
                    unbalance = 1;
                    while (1) {
                        fast_store_new_token(cur_tok);
                        get_token();
                        if (cur_tok == par_token) {
                            if (long_state != long_call_cmd) {
                                if (!int_par(suppress_long_error_code)) {
                                    goto RUNAWAY;

                                }
                            }
                        }
                        if (cur_tok < right_brace_limit) {
                            if (cur_tok < left_brace_limit) {
                                incr(unbalance);
                            } else {
                                decr(unbalance);
                                if (unbalance == 0)
                                    break;
                            }
                        }
                    }
                    rbrace_ptr = p;
                    store_new_token(cur_tok);

                } else {
                    /* Report an extra right brace and |goto continue| */
                    back_input();
                    print_err("Argument of ");
                    sprint_cs(warning_index);
                    tprint(" has an extra }");
                    help6
                        ("I've run across a `}' that doesn't seem to match anything.",
                         "For example, `\\def\\a#1{...}' and `\\a}' would produce",
                         "this error. If you simply proceed now, the `\\par' that",
                         "I've just inserted will cause me to report a runaway",
                         "argument that might be the root of the problem. But if",
                         "your `}' was spurious, just type `2' and it will go away.");
                    incr(align_state);
                    long_state = call_cmd;
                    cur_tok = par_token;
                    ins_error();
                    goto CONTINUE;
                    /* a white lie; the \.{\\par} won't always trigger a runaway */
                }
            } else {
                /* Store the current token, but |goto continue| if it is
                   a blank space that would become an undelimited parameter */
                if (cur_tok == space_token)
                    if (token_info(r) <= end_match_token)
                        if (token_info(r) >= match_token)
                            goto CONTINUE;
                store_new_token(cur_tok);

            }
            incr(m);
            if (token_info(r) > end_match_token)
                goto CONTINUE;
            if (token_info(r) < match_token)
                goto CONTINUE;
          FOUND:
            if (s != null) {
                /* Tidy up the parameter just scanned, and tuck it away */
                /* If the parameter consists of a single group enclosed in braces, we must
                   strip off the enclosing braces. That's why |rbrace_ptr| was introduced. */
                if ((m == 1) && (token_info(p) < right_brace_limit)
                    && (p != temp_token_head)) {
                    set_token_link(rbrace_ptr, null);
                    free_avail(p);
                    p = token_link(temp_token_head);
                    pstack[n] = token_link(p);
                    free_avail(p);
                } else {
                    pstack[n] = token_link(temp_token_head);
                }
                incr(n);
                if (int_par(tracing_macros_code) > 0) {
                    begin_diagnostic();
                    print_nl(match_chr);
                    print_int(n);
                    tprint("<-");
                    show_token_list(pstack[n - 1], null, 1000);
                    end_diagnostic(false);
                }

            }

            /* now |info(r)| is a token whose command code is either |match| or |end_match| */
        } while (token_info(r) != end_match_token);

    }
    /* Feed the macro body and its parameters to the scanner */
    /* Before we put a new token list on the input stack, it is wise to clean off
       all token lists that have recently been depleted. Then a user macro that ends
       with a call to itself will not require unbounded stack space. */
    while ((istate == token_list) && (iloc == null) && (token_type != v_template)) {
        /* conserve stack space */
        end_token_list();
    }
    begin_token_list(ref_count, macro);
    iname = warning_index;
    iloc = token_link(r);
    if (n > 0) {
        if (param_ptr + n > max_param_stack) {
            max_param_stack = param_ptr + n;
            if (max_param_stack > param_size)
                overflow("parameter stack size", (unsigned) param_size);
        }
        for (m = 0; m <= n - 1; m++)
            param_stack[param_ptr + m] = pstack[m];
        param_ptr = param_ptr + n;
    }
    goto EXIT;
  RUNAWAY:
    /* Report a runaway argument and abort */
    /* If |long_state=outer_call|, a runaway argument has already been reported. */
    if (long_state == call_cmd) {
        runaway();
        print_err("Paragraph ended before ");
        sprint_cs(warning_index);
        tprint(" was complete");
        help3("I suspect you've forgotten a `}', causing me to apply this",
              "control sequence to too much text. How can we recover?",
              "My plan is to forget the whole thing and hope for the best.");
        back_error();
    }
    pstack[n] = token_link(temp_token_head);
    align_state = align_state - unbalance;
    for (m = 0; m <= n; m++)
        flush_list(pstack[m]);

  EXIT:
    scanner_status = save_scanner_status;
    warning_index = save_warning_index;
}
