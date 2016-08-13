% mainbody.w
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

\def\eTeX{e-\TeX}
\def\Aleph{Aleph}
\def\pdfTeX{pdf\TeX}

@ @c


#include "ptexlib.h"

@
pdfTeX is copyright (C) 1996-2006 Han The Thanh, <thanh@@pdftex.org>.

e-TeX is copyright (C) 1994,98 by Peter Breitenlohner.

This is LuaTeX, a continuation of $\pdfTeX$ and $\Aleph$.  LuaTeX is a
document compiler intended to simplify high-quality typesetting for
many of the world's languages.  It is an extension of D. E. Knuth's
\TeX, which was designed essentially for the typesetting of languages
using the Latin alphabet.

The $\Aleph$ subsystem loosens many of the restrictions imposed by~\TeX:
register numbers are no longer limited to 8~bits;  fonts may have more
than 256~characters;  more than 256~fonts may be used;  etc.

% This program is directly derived from Donald E. Knuth's TeX;
% the change history which follows and the reward offered for finders of
% bugs refer specifically to TeX; they should not be taken as referring
% to LuaTeX, pdfTeX, nor e-TeX, although the change history is relevant in that it
% demonstrates the evolutionary path followed.  This program is not TeX;
% that name is reserved strictly for the program which is the creation
% and sole responsibility of Professor Knuth.

% Version 0 was released in September 1982 after it passed a variety of tests.
% Version 1 was released in November 1983 after thorough testing.
% Version 1.1 fixed ``disappearing font identifiers'' et alia (July 1984).
% Version 1.2 allowed `0' in response to an error, et alia (October 1984).
% Version 1.3 made memory allocation more flexible and local (November 1984).
% Version 1.4 fixed accents right after line breaks, et alia (April 1985).
% Version 1.5 fixed \the\toks after other expansion in \edefs (August 1985).
% Version 2.0 (almost identical to 1.5) corresponds to "Volume B" (April 1986).
% Version 2.1 corrected anomalies in discretionary breaks (January 1987).
% Version 2.2 corrected "(Please type...)" with null \endlinechar (April 1987).
% Version 2.3 avoided incomplete page in premature termination (August 1987).
% Version 2.4 fixed \noaligned rules in indented displays (August 1987).
% Version 2.5 saved cur_order when expanding tokens (September 1987).
% Version 2.6 added 10sp slop when shipping leaders (November 1987).
% Version 2.7 improved rounding of negative-width characters (November 1987).
% Version 2.8 fixed weird bug if no \patterns are used (December 1987).
% Version 2.9 made \csname\endcsname's "relax" local (December 1987).
% Version 2.91 fixed \outer\def\a0{}\a\a bug (April 1988).
% Version 2.92 fixed \patterns, also file names with complex macros (May 1988).
% Version 2.93 fixed negative halving in allocator when mem_min<0 (June 1988).
% Version 2.94 kept open_log_file from calling fatal_error (November 1988).
% Version 2.95 solved that problem a better way (December 1988).
% Version 2.96 corrected bug in "Infinite shrinkage" recovery (January 1989).
% Version 2.97 corrected blunder in creating 2.95 (February 1989).
% Version 2.98 omitted save_for_after at outer level (March 1989).
% Version 2.99 caught $$\begingroup\halign..$$ (June 1989).
% Version 2.991 caught .5\ifdim.6... (June 1989).
% Version 2.992 introduced major changes for 8-bit extensions (September 1989).
% Version 2.993 fixed a save_stack synchronization bug et alia (December 1989).
% Version 3.0 fixed unusual displays; was more \output robust (March 1990).
% Version 3.1 fixed nullfont, disabled \write{\the\prevgraf} (September 1990).
% Version 3.14 fixed unprintable font names and corrected typos (March 1991).
% Version 3.141 more of same; reconstituted ligatures better (March 1992).
% Version 3.1415 preserved nonexplicit kerns, tidied up (February 1993).
% Version 3.14159 allowed fontmemsize to change; bulletproofing (March 1995).
% Version 3.141592 fixed \xleaders, glueset, weird alignments (December 2002).
% Version 3.1415926 was a general cleanup with minor fixes (February 2008).


% Although considerable effort has been expended to make the LuaTeX program
% correct and reliable, no warranty is implied; the authors disclaim any
% obligation or liability for damages, including but not limited to
% special, indirect, or consequential damages arising out of or in
% connection with the use or performance of this software. This work has
% been a ``labor of love'' and the authors hope that users enjoy it.

A large piece of software like \TeX\ has inherent complexity that cannot
be reduced below a certain level of difficulty, although each individual
part is fairly simple by itself. The \.{WEB} language is intended to make
the algorithms as readable as possible, by reflecting the way the
individual program pieces fit together and by providing the
cross-references that connect different parts. Detailed comments about
what is going on, and about why things were done in certain ways, have
been liberally sprinkled throughout the program.  These comments explain
features of the implementation, but they rarely attempt to explain the
\TeX\ language itself, since the reader is supposed to be familiar with
{\sl The \TeX book}.
@.WEB@>
@:TeXbook}{\sl The \TeX book@>

The present implementation has a long ancestry, beginning in the summer
of~1977, when Michael~F. Plass and Frank~M. Liang designed and coded
a prototype
@^Plass, Michael Frederick@>
@^Liang, Franklin Mark@>
@^Knuth, Donald Ervin@>
based on some specifications that the author had made in May of that year.
This original proto\TeX\ included macro definitions and elementary
manipulations on boxes and glue, but it did not have line-breaking,
page-breaking, mathematical formulas, alignment routines, error recovery,
or the present semantic nest; furthermore,
it used character lists instead of token lists, so that a control sequence
like \.{\\halign} was represented by a list of seven characters. A
complete version of \TeX\ was designed and coded by the author in late
1977 and early 1978; that program, like its prototype, was written in the
{\mc SAIL} language, for which an excellent debugging system was
available. Preliminary plans to convert the {\mc SAIL} code into a form
somewhat like the present ``web'' were developed by Luis Trabb~Pardo and
@^Trabb Pardo, Luis Isidoro@>
the author at the beginning of 1979, and a complete implementation was
created by Ignacio~A. Zabala in 1979 and 1980. The \TeX82 program, which
@^Zabala Salelles, Ignacio Andr\'es@>
was written by the author during the latter part of 1981 and the early
part of 1982, also incorporates ideas from the 1979 implementation of
@^Guibas, Leonidas Ioannis@>
@^Sedgewick, Robert@>
@^Wyatt, Douglas Kirk@>
\TeX\ in {\mc MESA} that was written by Leonidas Guibas, Robert Sedgewick,
and Douglas Wyatt at the Xerox Palo Alto Research Center.  Several hundred
refinements were introduced into \TeX82 based on the experiences gained with
the original implementations, so that essentially every part of the system
has been substantially improved. After the appearance of ``Version 0'' in
September 1982, this program benefited greatly from the comments of
many other people, notably David~R. Fuchs and Howard~W. Trickey.
A final revision in September 1989 extended the input character set to
eight-bit codes and introduced the ability to hyphenate words from
different languages, based on some ideas of Michael~J. Ferguson.
@^Fuchs, David Raymond@>
@^Trickey, Howard Wellington@>
@^Ferguson, Michael John@>

No doubt there still is plenty of room for improvement, but the author
is firmly committed to keeping \TeX82 ``frozen'' from now on; stability
and reliability are to be its main virtues.

On the other hand, the \.{WEB} description can be extended without changing
the core of \TeX82 itself, and the program has been designed so that such
extensions are not extremely difficult to make.
The |banner| string defined here should be changed whenever \TeX\
undergoes any modifications, so that it will be clear which version of
\TeX\ might be the guilty party when a problem arises.
@^extensions to \TeX@>
@^system dependencies@>

This program contains code for various features extending \TeX,
therefore this program is called `\eTeX' and not
`\TeX'; the official name `\TeX' by itself is reserved
for software systems that are fully compatible with each other.
A special test suite called the ``\.{TRIP} test'' is available for
helping to determine whether a particular implementation deserves to be
known as `\TeX' [cf.~Stanford Computer Science report CS1027,
November 1984].

A similar test suite called the ``\.{e-TRIP} test'' is available for
helping to determine whether a particular implementation deserves to be
known as `\eTeX'.

@ This is the first of many sections of \TeX\ where global variables are
defined.

@c
boolean luainit;                /* are we using lua for initializations  */
boolean tracefilenames;         /* print file open-close  info? */


@ This program has two important variations: (1) There is a long and slow
version called \.{INITEX}, which does the extra calculations needed to
@.INITEX@>
initialize \TeX's internal tables; and (2)~there is a shorter and faster
production version, which cuts the initialization to a bare minimum.

@c
boolean ini_version;            /* are we \.{INITEX}? */
boolean dump_option;            /* was the dump name option used? */
boolean dump_line;              /* was a \.{\%\AM format} line seen? */
int bound_default;              /* temporary for setup */
char *bound_name;               /* temporary for setup */
int error_line;                 /* width of context lines on terminal error messages */
int half_error_line;            /* width of first lines of contexts in terminal
                                   error messages; should be between 30 and |error_line-15| */
int max_print_line;             /* width of longest text lines output; should be at least 60 */
int max_strings;                /* maximum number of strings; must not exceed |max_halfword| */
int strings_free;               /* strings available after format loaded */
int font_k;                     /* loop variable for initialization */
int buf_size;                   /* maximum number of characters simultaneously present in
                                   current lines of open files and in control sequences between
                                   \.{\\csname} and \.{\\endcsname}; must not exceed |max_halfword| */
int stack_size;                 /* maximum number of simultaneous input sources */
int max_in_open;                /* maximum number of input files and error insertions that
                                   can be going on simultaneously */
int param_size;                 /* maximum number of simultaneous macro parameters */
int nest_size;                  /* maximum number of semantic levels simultaneously active */
int save_size;                  /* space for saving values outside of current group; must be
                                   at most |max_halfword| */
int expand_depth;               /* limits recursive calls of the |expand| procedure */
int parsefirstlinep;            /* parse the first line for options */
int filelineerrorstylep;        /* format messages as file:line:error */
int haltonerrorp;               /* stop at first error */
boolean quoted_filename;        /* current filename is quoted */

@ @c
int get_luatexversion(void)
{
    return luatex_version;
}

str_number get_luatexrevision(void)
{
    return luatex_revision;
}

int get_luatex_date_info(void)
{
    return luatex_date_info;    /* todo, silly value */
}


@ This is it: the part of \TeX\ that executes all those procedures we have
written.

We have noted that there are two versions of \TeX82. One, called \.{INITEX},
@.INITEX@>
has to be run first; it initializes everything from scratch, without
reading a format file, and it has the capability of dumping a format file.
The other one is called `\.{VIRTEX}'; it is a ``virgin'' program that needs
@.VIRTEX@>
to input a format file in order to get started.

@c
#define const_chk(A) do {			\
	if (A < inf_##A) A = inf_##A;		\
	if (A > sup_##A) A = sup_##A;		\
    } while (0)

#define setup_bound_var(A,B,C) do {				\
	if (luainit>0) {					\
	    get_lua_number("texconfig",B,&C);			\
	    if (C==0) C=A;					\
	} else {						\
	    integer x;						\
	    setupboundvariable(&x, B, A);			\
	    C = (int)x;						\
	}							\
    } while (0)


int ready_already = 0;

int main_initialize(void)
{
    /* In case somebody has inadvertently made bad settings of the ``constants,''
       \LuaTeX\ checks them using a variable called |bad|. */
    int bad = 0;
    /* Bounds that may be set from the configuration file. We want the user to
       be able to specify the names with underscores, but \.{TANGLE} removes
       underscores, so we're stuck giving the names twice, once as a string,
       once as the identifier. How ugly. */

    setup_bound_var(15000, "max_strings", max_strings);
    setup_bound_var(100, "strings_free", strings_free);
    setup_bound_var(3000, "buf_size", buf_size);
    setup_bound_var(50, "nest_size", nest_size);
    setup_bound_var(15, "max_in_open", max_in_open);
    setup_bound_var(60, "param_size", param_size);
    setup_bound_var(4000, "save_size", save_size);
    setup_bound_var(300, "stack_size", stack_size);
    setup_bound_var(16384, "dvi_buf_size", dvi_buf_size);
    setup_bound_var(79, "error_line", error_line);
    setup_bound_var(50, "half_error_line", half_error_line);
    setup_bound_var(79, "max_print_line", max_print_line);
    setup_bound_var(0, "hash_extra", hash_extra);
    setup_bound_var(72, "pk_dpi", pk_dpi);
    setup_bound_var(10000, "expand_depth", expand_depth);

    /* Check other constants against their sup and inf. */
    const_chk(buf_size);
    const_chk(nest_size);
    const_chk(max_in_open);
    const_chk(param_size);
    const_chk(save_size);
    const_chk(stack_size);
    const_chk(dvi_buf_size);
    const_chk(max_strings);
    const_chk(strings_free);
    const_chk(hash_extra);
    const_chk(pk_dpi);
    if (error_line > ssup_error_line)
        error_line = ssup_error_line;

    /* array memory allocation */
    buffer = xmallocarray(packed_ASCII_code, (unsigned) buf_size);
    nest = xmallocarray(list_state_record, (unsigned) nest_size);
    save_stack = xmallocarray(save_record, (unsigned) save_size);
    input_stack = xmallocarray(in_state_record, (unsigned) stack_size);
    input_file = xmallocarray(alpha_file, (unsigned) max_in_open);
    input_file_callback_id = xmallocarray(int, (unsigned) max_in_open);
    line_stack = xmallocarray(int, (unsigned) max_in_open);
    eof_seen = xmallocarray(boolean, (unsigned) max_in_open);
    grp_stack = xmallocarray(save_pointer, (unsigned) max_in_open);
    if_stack = xmallocarray(pointer, (unsigned) max_in_open);
    source_filename_stack = xmallocarray(str_number, (unsigned) max_in_open);
    full_source_filename_stack = xmallocarray(char *, (unsigned) max_in_open);
    param_stack = xmallocarray(halfword, (unsigned) param_size);
    dvi_buf = xmallocarray(eight_bits, (unsigned) dvi_buf_size);

    if (ini_version) {
        fixmem = xmallocarray(smemory_word, fix_mem_init + 1);
        memset(voidcast(fixmem), 0, (fix_mem_init + 1) * sizeof(smemory_word));
        fix_mem_min = 0;
        fix_mem_max = fix_mem_init;
        eqtb_top = eqtb_size + hash_extra;
        if (hash_extra == 0)
            hash_top = undefined_control_sequence;
        else
            hash_top = eqtb_top;
        hash = xmallocarray(two_halves, (unsigned) (hash_top + 1));
        memset(hash, 0, sizeof(two_halves) * (unsigned) (hash_top + 1));
        eqtb = xmallocarray(memory_word, (unsigned) (eqtb_top + 1));
        memset(eqtb, 0, sizeof(memory_word) * (unsigned) (eqtb_top + 1));
        init_string_pool_array((unsigned) max_strings);
        reset_cur_string();
    }
    /* Check the ``constant'' values... */
    if ((half_error_line < 30) || (half_error_line > error_line - 15))
        bad = 1;
    if (max_print_line < 60)
        bad = 2;
    if (dvi_buf_size % 8 != 0)
        bad = 3;
    if (hash_prime > hash_size)
        bad = 5;
    if (max_in_open >= (sup_max_in_open+1)) /* 128 */
        bad = 6;
    /* Here are the inequalities that the quarterword and halfword values
       must satisfy (or rather, the inequalities that they mustn't satisfy): */
    if ((min_quarterword > 0) || (max_quarterword < 0x7FFF))
        bad = 11;
    if ((min_halfword > 0) || (max_halfword < 0x3FFFFFFF))
        bad = 12;
    if ((min_quarterword < min_halfword) || (max_quarterword > max_halfword))
        bad = 13;
    if (font_base < min_quarterword)
        bad = 15;
    if ((save_size > max_halfword) || (max_strings > max_halfword))
        bad = 17;
    if (buf_size > max_halfword)
        bad = 18;
    if (max_quarterword - min_quarterword < 0xFFFF)
        bad = 19;
    if (cs_token_flag + eqtb_size + hash_extra > max_halfword)
        bad = 21;
    if (bad > 0) {
        wterm_cr();
        fprintf(term_out,
                "Ouch---my internal constants have been clobbered! ---case %d",
                (int) bad);
    } else {
        initialize();           /* set global variables to their starting values */
        if (ini_version) {
            /* initialize all the primitives */
            no_new_control_sequence = false;
            first = 0;
            initialize_commands();
            initialize_etex_commands();
            init_str_ptr = str_ptr;
            no_new_control_sequence = true;
            fix_date_and_time();
        }
        ready_already = 314159;
    }
    return bad;
}

@ @c
void main_body(void)
{
    static char pdftex_map[] = "pdftex.map";
    int bad = main_initialize();
    history = fatal_error_stop; /* in case we quit during initialization */
    t_open_out();               /* open the terminal for output */
    if (!luainit)
        tracefilenames = true;
    if (bad > 0) {
        goto FINAL_END;
    }
    print_banner(luatex_version_string);

    /* Get the first line of input and prepare to start */
    /* When we begin the following code, \TeX's tables may still contain garbage;
       the strings might not even be present. Thus we must proceed cautiously to get
       bootstrapped in.

       But when we finish this part of the program, \TeX\ is ready to call on the
       |main_control| routine to do its work.
     */
    initialize_inputstack();    /* this copies the command-line */
    if (buffer[iloc] == '*')
        incr(iloc);
    if ((format_ident == 0) || (buffer[iloc] == '&') || dump_line) {
        char *fname = NULL;
        if (format_ident != 0 && !ini_version)
            initialize();       /* erase preloaded format */
        if ((fname = open_fmt_file()) == NULL)
            goto FINAL_END;
#if defined(MIKTEX)
        if (! load_fmt_file(fname, true))
		{
            zwclose (fmt_file);
			miktex_luatex_renew_format_file (fname);
			if ((fname = open_fmt_file()) == 0)
			{
				goto FINAL_END;
			}
			if (! load_fmt_file(fname, false))
			{
				zwclose (fmt_file);
				goto FINAL_END;
			}
        }
#else
        if (!load_fmt_file(fname)) {
            zwclose(fmt_file);
            goto FINAL_END;
        }
#endif
        zwclose(fmt_file);
        while ((iloc < ilimit) && (buffer[iloc] == ' '))
            incr(iloc);
    }
    if (output_mode_option != 0)
        output_mode_par = output_mode_value;
    if (draft_mode_option != 0) {
        draft_mode_par = draft_mode_value;
    }
    /* can this be moved? */
    pdf_init_map_file((char *) pdftex_map);
    /* */
    if (end_line_char_inactive)
        decr(ilimit);
    else
        buffer[ilimit] = (packed_ASCII_code) end_line_char_par;
    fix_date_and_time();
    random_seed = (microseconds * 1000) + (epochseconds % 1000000);
    init_randoms(random_seed);
    initialize_math();
    fixup_selector(log_opened_global);
    check_texconfig_init();
    if ((iloc < ilimit) && (get_cat_code(cat_code_table_code, buffer[iloc]) != escape_cmd))
        start_input();          /* \.{\\input} assumed */
    /* DIR: Initialize |text_dir_ptr| */
    text_dir_ptr = new_dir(0);

    history = spotless;         /* ready to go! */
    /* Initialize synctex primitive */
    synctexinitcommand();
    main_control();             /* come to life */
    flush_node(text_dir_ptr);
    final_cleanup();            /* prepare for death */
    close_files_and_terminate();
  FINAL_END:
    do_final_end();
}


@ Here we do whatever is needed to complete \TeX's job gracefully on the
local operating system. The code here might come into play after a fatal
error; it must therefore consist entirely of ``safe'' operations that
cannot produce error messages. For example, it would be a mistake to call
|str_room| or |make_string| at this time, because a call on |overflow|
might lead to an infinite loop.
@^system dependencies@>

Actually there's one way to get error messages, via |prepare_mag|;
but that can't cause infinite recursion.
@^recursion@>

This program doesn't bother to close the input files that may still be open.

@c
void close_files_and_terminate(void)
{
    int callback_id;
    callback_id = callback_defined(stop_run_callback);
    finalize_write_files();
    if (tracing_stats_par > 0) {
        if (callback_id == 0) {
            /* Output statistics about this job */
            /* The present section goes directly to the log file instead of using
               |print| commands, because there's no need for these strings to take
               up |string_pool| memory when a non-{\bf stat} version of \TeX\ is being used.
             */

            if (log_opened_global) {
                fprintf(log_file,
                        "\n\nHere is how much of LuaTeX's memory you used:\n");
                fprintf(log_file, " %d string%s out of %d\n",
                        (int) (str_ptr - init_str_ptr),
                        (str_ptr == (init_str_ptr + 1) ? "" : "s"),
                        (int) (max_strings - init_str_ptr + STRING_OFFSET));
                fprintf(log_file, " %d,%d words of node,token memory allocated",
                        (int) var_mem_max, (int) fix_mem_max);
                print_node_mem_stats();
                fprintf(log_file,
                        " %d multiletter control sequences out of %ld+%d\n",
                        (int) cs_count, (long) hash_size, (int) hash_extra);
                fprintf(log_file, " %d font%s using %d bytes\n",
                        (int) max_font_id(), (max_font_id() == 1 ? "" : "s"),
                        (int) font_bytes);
                fprintf(log_file,
                        " %di,%dn,%dp,%db,%ds stack positions out of %di,%dn,%dp,%db,%ds\n",
                        (int) max_in_stack, (int) max_nest_stack,
                        (int) max_param_stack, (int) max_buf_stack,
                        (int) max_save_stack + 6, (int) stack_size,
                        (int) nest_size, (int) param_size, (int) buf_size,
                        (int) save_size);
            }
        }
    }
    wake_up_terminal();
    /* rubish, these pdf arguments, passed, needs to be fixed, e.g. with a dummy in dvi */
    wrapup_backend();
    /* Close {\sl Sync\TeX} file and write status */
    synctexterminate(log_opened_global);       /* Let the {\sl Sync\TeX} controller close its files. */
    free_text_codes();
    free_math_codes();
    if (log_opened_global) {
        wlog_cr();
        selector = selector - 2;
        if ((selector == term_only) && (callback_id == 0)) {
            tprint_nl("Transcript written on ");
            tprint_file_name(NULL, texmf_log_name, NULL);
            print_char('.');
            print_ln();
        }
        lua_a_close_out(log_file);
    }
}


@ We get to the |final_cleanup| routine when \.{\\end} or \.{\\dump} has
been scanned and |its_all_over|\kern-2pt.

@c
void final_cleanup(void)
{
    int c;                      /* 0 for \.{\\end}, 1 for \.{\\dump} */
    halfword i;                 /* for looping marks  */
    halfword t;                 /* was a global temp_ptr */
    c = cur_chr;
    if (job_name == 0)
        open_log_file();
    while (input_ptr > 0)
        if (istate == token_list)
            end_token_list();
        else
            end_file_reading();
    while (open_parens > 0) {
        report_stop_file(filetype_tex);
        decr(open_parens);
    }
    if (cur_level > level_one) {
        tprint_nl("(\\end occurred inside a group at level ");
        print_int(cur_level - level_one);
        print_char(')');
        show_save_groups();
    }
    while (cond_ptr != null) {
        tprint_nl("(\\end occurred when ");
        print_cmd_chr(if_test_cmd, cur_if);
        if (if_line != 0) {
            tprint(" on line ");
            print_int(if_line);
        }
        tprint(" was incomplete)");
        if_line = if_line_field(cond_ptr);
        cur_if = subtype(cond_ptr);
        t = cond_ptr;
        cond_ptr = vlink(cond_ptr);
        flush_node(t);
    }
    if (callback_defined(stop_run_callback) == 0)
        if (history != spotless)
            if ((history == warning_issued) || (interaction < error_stop_mode))
                if (selector == term_and_log) {
                    selector = term_only;
                    tprint_nl("(see the transcript file for additional information)");
                    selector = term_and_log;
                }
    if (c == 1) {
        if (ini_version) {
            for (i = 0; i <= biggest_used_mark; i++) {
                delete_top_mark(i);
                delete_first_mark(i);
                delete_bot_mark(i);
                delete_split_first_mark(i);
                delete_split_bot_mark(i);
            }
            for (c = last_box_code; c <= vsplit_code; c++)
                flush_node_list(disc_ptr[c]);
            if (last_glue != max_halfword) {
                flush_node(last_glue);
            }
            while (pseudo_files != null)
                pseudo_close(); /* flush pseudo files */
            store_fmt_file();
            return;
        }
        tprint_nl("(\\dump is performed only by INITEX)");
        return;
    }
}

@ Once \TeX\ is working, you should be able to diagnose most errors with
the \.{\\show} commands and other diagnostic features.
An additional routine called |debug_help|
will come into play when you type `\.D' after an error message;
|debug_help| also occurs just before a fatal error causes \TeX\ to succumb.
@^debugging@>
@^system dependencies@>

The interface to |debug_help| is primitive, but it is good enough when used
with a debugger that allows you to set breakpoints and to read
variables and change their values. After getting the prompt `\.{debug \#}', you
type either a negative number (this exits |debug_help|), or zero (this
goes to a location where you can set a breakpoint, thereby entering into
dialog with the debugger), or a positive number |m| followed by
an argument |n|. The meaning of |m| and |n| will be clear from the
program below. (If |m=13|, there is an additional argument, |l|.)
@.debug \#@>

@c
#ifdef DEBUG
void debug_help(void)
{                               /* routine to display various things */
    int k;
    int m = 0, n = 0, l = 0;
    while (1) {
        wake_up_terminal();
        tprint_nl("debug # (-1 to exit):");
        update_terminal();
        (void) fscanf(term_in, "%d", &m);
        if (m < 0)
            return;
        else if (m == 0)
            abort();            /* go to every label at least once */
        else {
            (void) fscanf(term_in, "%d", &n);
            switch (m) {
            case 1:
                print_word(varmem[n]);  /* display |varmem[n]| in all forms */
                break;
            case 2:
                print_int(info(n));
                break;
            case 3:
                print_int(link(n));
                break;
            case 4:
                print_word(eqtb[n]);
                break;
            case 6:
                print_int(save_type(n));
                print_int(save_level(n));
                print_word(save_word(n));
                break;
            case 7:
                show_box(n);    /* show a box, abbreviated by |show_box_depth| and |show_box_breadth| */
                break;
            case 8:
                breadth_max = 10000;
                depth_threshold = 0x7FFFFFFF;
                show_node_list(n);      /* show a box in its entirety */
                break;
            case 9:
                show_token_list(n, null, 1000);
                break;
            case 10:
                print(n);
                break;
            case 13:
                (void) fscanf(term_in, "%d", &l);
                print_cmd_chr(n, l);
                break;
            case 14:
                for (k = 0; k <= n; k++)
                    print(buffer[k]);
                break;
            case 15:
                font_in_short_display = null_font;
                short_display(n);
                break;
            default:
                tprint("?");
                break;
            }
        }
    }
}
#endif
