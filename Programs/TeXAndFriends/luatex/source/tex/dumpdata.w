% dumpdata.w
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

/* we start with 907: the sum of the values of the bytes of "don knuth" */

#define FORMAT_ID (907+28)
#if ((FORMAT_ID>=0) && (FORMAT_ID<=256))
#error Wrong value for FORMAT_ID.
#endif


@ After \.{INITEX} has seen a collection of fonts and macros, it
can write all the necessary information on an auxiliary file so
that production versions of \TeX\ are able to initialize their
memory at high speed. The present section of the program takes
care of such output and input. We shall consider simultaneously
the processes of storing and restoring,
so that the inverse relation between them is clear.
@.INITEX@>

The global variable |format_ident| is a string that is printed right
after the |banner| line when \TeX\ is ready to start. For \.{INITEX} this
string says simply `\.{(INITEX)}'; for other versions of \TeX\ it says,
for example, `\.{(preloaded format=plain 1982.11.19)}', showing the year,
month, and day that the format file was created. We have |format_ident=0|
before \TeX's tables are loaded. |FORMAT_ID| is a new field of type int
suitable for the identification of a format: values between 0 and 256
(included) can not be used because in the previous format they are used
for the length of  the name of the engine.
@c
str_number format_ident;
str_number format_name;         /* principal file name */


@ Format files consist of |memory_word| items, and we use the following
macros to dump words of different types:

@c
FILE *fmt_file;                 /* for input or output of format information */

@ @c
void store_fmt_file(void)
{
    int j, k, l;                /* all-purpose indices */
    halfword p;                 /* all-purpose pointer */
    int x;                      /* something to dump */
    char *format_engine;
    int callback_id;            /* |pre_dump| callback */
    char *fmtname = NULL;
    /* If dumping is not allowed, abort */
    /* The user is not allowed to dump a format file unless |save_ptr=0|.
       This condition implies that |cur_level=level_one|, hence
       the |xeq_level| array is constant and it need not be dumped. */
    if (save_ptr != 0) {
        print_err("You can't dump inside a group");
        help1("`{...\\dump}' is a no-no.");
        succumb();
    }

    /* Create the |format_ident|, open the format file, and inform the user
       that dumping has begun */
    callback_id = callback_defined(pre_dump_callback);
    if (callback_id > 0) {
        (void) run_callback(callback_id, "->");
    }
    selector = new_string;
    tprint(" (format=");
    print(job_name);
    print_char(' ');
    print_int(year_par);
    print_char('.');
    print_int(month_par);
    print_char('.');
    print_int(day_par);
    print_char(')');
    str_room(2);
    format_ident = make_string();
    print(job_name);
    format_name = make_string();
    if (interaction == batch_mode)
        selector = log_only;
    else
        selector = term_and_log;

    fmtname = pack_job_name(format_extension);
    while (!zopen_w_output(&fmt_file, fmtname, FOPEN_WBIN_MODE)) {
        fmtname = prompt_file_name("format file name", format_extension);
    }
    tprint_nl("Beginning to dump on file ");
    tprint(fmtname);
    free(fmtname);
    tprint_nl("");
    print(format_ident);

    /* Dump constants for consistency check */
    /* The next few sections of the program should make it clear how we use the
       dump/undump macros. */

    dump_int(0x57325458);       /* Web2C \TeX's magic constant: "W2TX" */
    dump_int(FORMAT_ID);

    /* Align engine to 4 bytes with one or more trailing NUL */
    x = (int) strlen(engine_name);
    format_engine = xmalloc((unsigned) (x + 4));
    strcpy(format_engine, engine_name);
    for (k = x; k <= x + 3; k++)
        format_engine[k] = 0;
    x = x + 4 - (x % 4);
    dump_int(x);
    dump_things(format_engine[0], x);
    xfree(format_engine);
    dump_int(0x57325458);       /* TODO HM, what checksum would make sense? */
    dump_int(max_halfword);
    dump_int(hash_high);
    dump_int(eqtb_size);
    dump_int(hash_prime);

    /* Dump the string pool */
    k = dump_string_pool();
    print_ln();
    print_int(k);
    tprint(" strings using ");
    print_int((longinteger) pool_size);
    tprint(" bytes");

    /* Dump the dynamic memory */
    /* By sorting the list of available spaces in the variable-size portion of
       |mem|, we are usually able to get by without having to dump very much
       of the dynamic memory.

       We recompute |var_used| and |dyn_used|, so that \.{INITEX} dumps valid
       information even when it has not been gathering statistics.
     */
    dump_node_mem();
    dump_int(temp_token_head);
    dump_int(hold_token_head);
    dump_int(omit_template);
    dump_int(null_list);
    dump_int(backup_head);
    dump_int(garbage);
    x = (int) fix_mem_min;
    dump_int(x);
    x = (int) fix_mem_max;
    dump_int(x);
    x = (int) fix_mem_end;
    dump_int(x);
    dump_int(avail);
    dyn_used = (int) fix_mem_end + 1;
    dump_things(fixmem[fix_mem_min], fix_mem_end - fix_mem_min + 1);
    x = x + (int) (fix_mem_end + 1 - fix_mem_min);
    p = avail;
    while (p != null) {
        decr(dyn_used);
        p = token_link(p);
    }
    dump_int(dyn_used);
    print_ln();
    print_int(x);
    tprint(" memory locations dumped; current usage is ");
    print_int(var_used);
    print_char('&');
    print_int(dyn_used);

    /* Dump the table of equivalents */
    /* Dump regions 1 to 4 of |eqtb| */
    /*The table of equivalents usually contains repeated information, so we dump it
       in compressed form: The sequence of $n+2$ values $(n,x_1,\ldots,x_n,m)$ in the
       format file represents $n+m$ consecutive entries of |eqtb|, with |m| extra
       copies of $x_n$, namely $(x_1,\ldots,x_n,x_n,\ldots,x_n)$.
     */
    k = null_cs;
    do {
        j = k;
        while (j < int_base - 1) {
            if ((equiv(j) == equiv(j + 1)) && (eq_type(j) == eq_type(j + 1)) &&
                (eq_level(j) == eq_level(j + 1)))
                goto FOUND1;
            incr(j);
        }
        l = int_base;
        goto DONE1;             /* |j=int_base-1| */
      FOUND1:
        incr(j);
        l = j;
        while (j < int_base - 1) {
            if ((equiv(j) != equiv(j + 1)) || (eq_type(j) != eq_type(j + 1)) ||
                (eq_level(j) != eq_level(j + 1)))
                goto DONE1;
            incr(j);
        }
      DONE1:
        dump_int(l - k);
        dump_things(eqtb[k], l - k);
        k = j + 1;
        dump_int(k - l);
    } while (k != int_base);

    /* Dump regions 5 and 6 of |eqtb| */
    do {
        j = k;
        while (j < eqtb_size) {
            if (eqtb[j].cint == eqtb[j + 1].cint)
                goto FOUND2;
            incr(j);
        }
        l = eqtb_size + 1;
        goto DONE2;             /* |j=eqtb_size| */
      FOUND2:
        incr(j);
        l = j;
        while (j < eqtb_size) {
            if (eqtb[j].cint != eqtb[j + 1].cint)
                goto DONE2;
            incr(j);
        }
      DONE2:
        dump_int(l - k);
        dump_things(eqtb[k], l - k);
        k = j + 1;
        dump_int(k - l);
    } while (k <= eqtb_size);
    if (hash_high > 0)
        dump_things(eqtb[eqtb_size + 1], hash_high);    /* dump |hash_extra| part */

    dump_int(par_loc);
    dump_int(write_loc);
    dump_math_codes();
    dump_text_codes();
    /* Dump the hash table */
    /* A different scheme is used to compress the hash table, since its lower
       region is usually sparse. When |text(p)<>0| for |p<=hash_used|, we output
       two words, |p| and |hash[p]|. The hash table is, of course, densely packed
       for |p>=hash_used|, so the remaining entries are output in a~block.
     */
    dump_primitives();
    dump_int(hash_used);
    cs_count = frozen_control_sequence - 1 - hash_used + hash_high;
    for (p = hash_base; p <= hash_used; p++) {
        if (cs_text(p) != 0) {
            dump_int(p);
            dump_hh(hash[p]);
            incr(cs_count);
        }
    }
    dump_things(hash[hash_used + 1],
                undefined_control_sequence - 1 - hash_used);
    if (hash_high > 0)
        dump_things(hash[eqtb_size + 1], hash_high);
    dump_int(cs_count);
    print_ln();
    print_int(cs_count);
    tprint(" multiletter control sequences");

    /* Dump the font information */
    dump_int(max_font_id());
    for (k = 0; k <= max_font_id(); k++) {
        /* Dump the array info for internal font number |k| */
        dump_font(k);
        tprint_nl("\\font");
        print_esc(font_id_text(k));
        print_char('=');
        tprint_file_name((unsigned char *) font_name(k),
                         (unsigned char *) font_area(k), NULL);
        if (font_size(k) != font_dsize(k)) {
            tprint(" at ");
            print_scaled(font_size(k));
            tprint("pt");
        }
    }
    print_ln();
    print_int(max_font_id());
    tprint(" preloaded font");
    if (max_font_id() != 1)
        print_char('s');
    dump_math_data();

    /* Dump the hyphenation tables */
    dump_language_data();

    /* Dump a couple more things and the closing check word */
    dump_int(interaction);
    dump_int(format_ident);
    dump_int(format_name);
    dump_int(69069);
    /* We have already printed a lot of statistics, so we set |tracing_stats:=0|
       to prevent them from appearing again. */
    tracing_stats_par = 0;

    /* Dump the lua bytecodes */
    dump_luac_registers();

    /* Close the format file */
    zwclose(fmt_file);
}

@ Corresponding to the procedure that dumps a format file, we have a function
that reads one in. The function returns |false| if the dumped format is
incompatible with the present \TeX\ table sizes, etc.

@c
#define too_small(A) do {					\
	wake_up_terminal();					\
	wterm_cr();						\
	fprintf(term_out,"---! Must increase the %s",(A));	\
	goto BAD_FMT;						\
    } while (0)

@ The inverse macros are slightly more complicated, since we need to check
the range of the values we are reading in. We say `|undump(a)(b)(x)|' to
read an integer value |x| that is supposed to be in the range |a<=x<=b|.

@c
#define undump(A,B,C) do {						\
	undump_int(x);							\
	if (x<(A) || x>(B)) goto BAD_FMT;				\
	else (C) = x;							\
    } while (0)


#define format_debug(A,B) do {					\
	if (debug_format_file) {				\
	    fprintf (stderr, "fmtdebug: %s=%d", (A), (int)(B));	\
	}							\
    } while (0)

#define undump_size(A,B,C,D) do {					\
	undump_int(x);							\
	if (x<(A))  goto BAD_FMT;					\
	if (x>(B))  too_small(C);					\
	else format_debug (C,x);					\
	(D) = x;							\
    } while (0)


@ @c
#if defined(MIKTEX)
boolean load_fmt_file(const char *fmtname, boolean silent)
#else
boolean load_fmt_file(const char *fmtname)
#endif
{
    int j, k;                   /* all-purpose indices */
    halfword p;                 /* all-purpose pointer */
    int x;                      /* something undumped */
    char *format_engine;
    /* Undump constants for consistency check */
    if (ini_version) {
        libcfree(hash);
        libcfree(eqtb);
        libcfree(fixmem);
        libcfree(varmem);
    }
    undump_int(x);
    format_debug("format magic number", x);
    if (x != 0x57325458)
        goto BAD_FMT;           /* not a format file */

    undump_int(x);
    format_debug("format id", x);
    if (x != FORMAT_ID)
        goto BAD_FMT;           /* FORMAT_ID mismatch */

    undump_int(x);
    format_debug("engine name size", x);
    if ((x < 0) || (x > 256))
        goto BAD_FMT;           /* corrupted format file */

    format_engine = xmalloc((unsigned) x);
    undump_things(format_engine[0], x);
    format_engine[x - 1] = 0;   /* force string termination, just in case */
    if (strcmp(engine_name, format_engine)) {
        wake_up_terminal();
        wterm_cr();
        fprintf(term_out, "---! %s was written by %s", fmtname, format_engine);
        xfree(format_engine);
        goto BAD_FMT;
    }
    xfree(format_engine);
    undump_int(x);
    format_debug("string pool checksum", x);
    if (x != 0x57325458) {      /* todo: @@\$ *//* check that strings are the same */
        wake_up_terminal();
        wterm_cr();
        fprintf(term_out, "---! %s was written by a different version",
                fmtname);
        goto BAD_FMT;
    }
    undump_int(x);
    if (x != max_halfword)
        goto BAD_FMT;           /* check |max_halfword| */
    undump_int(hash_high);
    if ((hash_high < 0) || (hash_high > sup_hash_extra))
        goto BAD_FMT;
    if (hash_extra < hash_high)
        hash_extra = hash_high;
    eqtb_top = eqtb_size + hash_extra;
    if (hash_extra == 0)
        hash_top = undefined_control_sequence;
    else
        hash_top = eqtb_top;
    hash = xmallocarray(two_halves, (unsigned) (1 + hash_top));
    memset(hash, 0, sizeof(two_halves) * (unsigned) (hash_top + 1));
    eqtb = xmallocarray(memory_word, (unsigned) (eqtb_top + 1));
    set_eq_type(undefined_control_sequence, undefined_cs_cmd);
    set_equiv(undefined_control_sequence, null);
    set_eq_level(undefined_control_sequence, level_zero);
    for (x = eqtb_size + 1; x <= eqtb_top; x++)
        eqtb[x] = eqtb[undefined_control_sequence];
    undump_int(x);
    if (x != eqtb_size)
        goto BAD_FMT;
    undump_int(x);
    if (x != hash_prime)
        goto BAD_FMT;

    /* Undump the string pool */
    str_ptr = undump_string_pool();
    /* Undump the dynamic memory */
    undump_node_mem();
    undump_int(temp_token_head);
    undump_int(hold_token_head);
    undump_int(omit_template);
    undump_int(null_list);
    undump_int(backup_head);
    undump_int(garbage);
    undump_int(fix_mem_min);
    undump_int(fix_mem_max);
    fixmem = xmallocarray(smemory_word, fix_mem_max + 1);
    memset(voidcast(fixmem), 0, (fix_mem_max + 1) * sizeof(smemory_word));
    undump_int(fix_mem_end);
    undump_int(avail);
    undump_things(fixmem[fix_mem_min], fix_mem_end - fix_mem_min + 1);
    undump_int(dyn_used);

    /* Undump the table of equivalents */
    /* Undump regions 1 to 6 of |eqtb| */
    k = null_cs;
    do {
        undump_int(x);
        if ((x < 1) || (k + x > eqtb_size + 1))
            goto BAD_FMT;
        undump_things(eqtb[k], x);
        k = k + x;
        undump_int(x);
        if ((x < 0) || (k + x > eqtb_size + 1))
            goto BAD_FMT;
        for (j = k; j <= k + x - 1; j++)
            eqtb[j] = eqtb[k - 1];
        k = k + x;
    } while (k <= eqtb_size);
    if (hash_high > 0)          /* undump |hash_extra| part */
        undump_things(eqtb[eqtb_size + 1], hash_high);

    undump(hash_base, hash_top, par_loc);
    par_token = cs_token_flag + par_loc;
    undump(hash_base, hash_top, write_loc);
    undump_math_codes();
    undump_text_codes();
    /* Undump the hash table */
    undump_primitives();
    undump(hash_base, frozen_control_sequence, hash_used);
    p = hash_base - 1;
    do {
        undump(p + 1, hash_used, p);
        undump_hh(hash[p]);
    } while (p != hash_used);
    undump_things(hash[hash_used + 1],
                  undefined_control_sequence - 1 - hash_used);
    if (debug_format_file)
        print_csnames(hash_base, undefined_control_sequence - 1);
    if (hash_high > 0) {
        undump_things(hash[eqtb_size + 1], hash_high);
        if (debug_format_file)
            print_csnames(eqtb_size + 1, hash_high - (eqtb_size + 1));
    }
    undump_int(cs_count);

    /* Undump the font information */
    undump_int(x);
    set_max_font_id(x);
    for (k = 0; k <= max_font_id(); k++) {
        /* Undump the array info for internal font number |k| */
        undump_font(k);
    }
    undump_math_data();

    /* Undump the hyphenation tables */
    undump_language_data();

    /* Undump a couple more things and the closing check word */
    undump(batch_mode, error_stop_mode, interaction);
    if (interactionoption != unspecified_mode)
        interaction = interactionoption;
    undump(0, str_ptr, format_ident);
    undump(0, str_ptr, format_name);
    undump_int(x);
    if (x != 69069)
        goto BAD_FMT;

    /* Undump the lua bytecodes */
    undump_luac_registers();

    prev_depth_par = ignore_depth;
    return true;                /* it worked! */
  BAD_FMT:
#if defined(MIKTEX)
    if (silent)
	{
		return (false);
	}
#endif
    wake_up_terminal();
    wterm_cr();
    fprintf(term_out, "(Fatal format file error; I'm stymied)");
    return false;
}
