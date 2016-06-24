/* printing.h

   Copyright 2009 Taco Hoekwater <taco@luatex.org>

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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */

#ifndef PRINTING_H
#  define PRINTING_H

#define last_file_selector 127 /* was 15 */

/*
    Nicer will be to start these with 0 and then use an offset for the write
    registers internally.
*/

typedef enum {
    no_print     = last_file_selector + 1, /* 16 */ /* |selector| setting that makes data disappear */
    term_only    = last_file_selector + 2, /* 17 */ /* printing is destined for the terminal only */
    log_only     = last_file_selector + 3, /* 18 */ /* printing is destined for the transcript file only */
    term_and_log = last_file_selector + 4, /* 19 */ /* normal |selector| setting */
    pseudo       = last_file_selector + 5, /* 20 */ /* special |selector| setting for |show_context| */
    new_string   = last_file_selector + 6, /* 21 */ /* printing is deflected to the string pool */
} selector_settings;

extern int escape_controls;
extern int new_string_line;

#  define ssup_error_line 255
#  define max_selector new_string /* highest selector setting */

extern alpha_file log_file;
extern int selector;
extern int dig[23];
extern int tally;
extern int term_offset;
extern int file_offset;
extern packed_ASCII_code trick_buf[(ssup_error_line + 1)];
extern int trick_count;
extern int first_count;
extern boolean inhibit_par_tokens;

/*
    Macro abbreviations for output to the terminal and to the log file are
    defined here for convenience. Some systems need special conventions
    for terminal output, and it is possible to adhere to those conventions
    by changing |wterm|, |wterm_ln|, and |wterm_cr| in this section.
    @^system dependencies@>
*/

#  define wterm_cr()   fprintf(term_out,"\n")
#  define wlog_cr()    fprintf(log_file,"\n")

extern void print_ln(void);
extern void print_char(int s);
extern void print(int s);
extern void lprint (lstring *ss);
extern void print_nl(str_number s);
extern void print_nlp(void);
extern void print_banner(const char *);
extern void log_banner(const char *);
extern void print_version_banner(void);
extern void print_esc(str_number s);
extern void print_the_digs(eight_bits k);
extern void print_int(longinteger n);
extern void print_two(int n);
extern void print_hex(int n);
extern void print_roman_int(int n);
extern void print_current_string(void);

#  define print_font_name(A) tprint(font_name(A))

extern void print_cs(int p);
extern void sprint_cs(pointer p);
extern void sprint_cs_name(pointer p);
extern void tprint(const char *s);
extern void tprint_nl(const char *s);
extern void tprint_esc(const char *s);

extern void prompt_input(const char *s);

#  define single_letter(A)                            \
     ((str_length(A)==1)||                            \
     ((str_length(A)==4)&&*(str_string((A)))>=0xF0)|| \
     ((str_length(A)==3)&&*(str_string((A)))>=0xE0)|| \
     ((str_length(A)==2)&&*(str_string((A)))>=0xC0))

#  define is_active_cs(a)           \
    (a && str_length(a)>3 &&		\
    ( *str_string(a)    == 0xEF) && \
    (*(str_string(a)+1) == 0xBF) && \
    (*(str_string(a)+2) == 0xBF))

#  define active_cs_value(A) pool_to_unichar((str_string((A))+3))

extern void print_glue(scaled d, int order, const char *s);  /* prints a glue component */
extern void print_spec(int p, const char *s);                /* prints a glue specification */

extern int font_in_short_display;                            /* an internal font number */
extern void print_font_identifier(internal_font_number f);
extern void short_display(int p);                            /* prints highlights of list |p| */
extern void print_font_and_char(int p);                      /* prints |char_node| data */
extern void print_mark(int p);                               /* prints token list data in braces */
extern void print_rule_dimen(scaled d);                      /* prints dimension in rule node */
extern int depth_threshold;                                  /* maximum nesting depth in box displays */
extern int breadth_max;                                      /* maximum number of items shown at the same list level */
extern void show_box(halfword p);
extern void short_display_n(int p, int m);                   /* prints highlights of list |p| */

extern void print_csnames(int hstart, int hfinish);
extern void print_file_line(void);

extern void begin_diagnostic(void);
extern void end_diagnostic(boolean blank_line);
extern int global_old_setting;

#endif
