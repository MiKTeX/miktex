/*

printing.w

Copyright 2009-2013 Taco Hoekwater <taco@luatex.org>

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

#define wlog(A)  fputc(A,log_file)
#define wterm(A) fputc(A,term_out)

int new_string_line = 0;
int escape_controls = 1;

/*tex

Messages that are sent to a user's terminal and to the transcript-log file are
produced by several `|print|' procedures. These procedures will direct their
output to a variety of places, based on the setting of the global variable
|selector|, which has the following possible values:

\startitemize

\startitem
    |term_and_log|, the normal setting, prints on the terminal and on the
    transcript file.
\stopitem

\startitem
    |log_only|, prints only on the transcript file.
\stopitem

\startitem
    |term_only|, prints only on the terminal.
\stopitem

\startitem
    |no_print|, doesn't print at all. This is used only in rare cases before the
    transcript file is open.
\stopitem

\startitem
    |pseudo|, puts output into a cyclic buffer that is used by the |show_context|
    routine; when we get to that routine we shall discuss the reasoning behind
    this curious mode.
\stopitem

\startitem
    |new_string|, appends the output to the current string in the string pool.
\stopitem

\startitem
    0 to 15, prints on one of the sixteen files for \.{\\write} output.
\stopitem

\stopitemize

The symbolic names `|term_and_log|', etc., have been assigned numeric codes that
satisfy the convenient relations |no_print+1=term_only|, |no_print+2=log_only|,
|term_only+2=log_only+1=term_and_log|.

Three additional global variables, |tally| and |term_offset| and |file_offset|,
record the number of characters that have been printed since they were most
recently cleared to zero. We use |tally| to record the length of (possibly very
long) stretches of printing; |term_offset| and |file_offset|, on the other hand,
keep track of how many characters have appeared so far on the current line that
has been output to the terminal or to the transcript file, respectively.

*/

/*tex transcript of \TeX\ session */

alpha_file log_file;

/*tex where to print a message */

int selector = term_only;

/*tex digits in a number being output */

int dig[23];

/*tex the number of characters recently printed */

int tally = 0;

/*tex the number of characters on the current terminal line */

int term_offset = 0;

/*tex the number of characters on the current file line */

int file_offset = 0;

/*tex circular buffer for pseudoprinting */

packed_ASCII_code trick_buf[(ssup_error_line + 1)];

/*tex threshold for pseudoprinting, explained later */

int trick_count;

/*tex another variable for pseudoprinting */

int first_count;

/*tex for minor adjustments to |show_token_list|  */

boolean inhibit_par_tokens = false;

/*tex

To end a line of text output, we call |print_ln|

*/

void print_ln(void)
{
    switch (selector) {
        case no_print:
            break;
        case term_only:
            wterm_cr();
            term_offset = 0;
            break;
        case log_only:
            wlog_cr();
            file_offset = 0;
            break;
        case term_and_log:
            wterm_cr();
            wlog_cr();
            term_offset = 0;
            file_offset = 0;
            break;
        case pseudo:
            break;
        case new_string:
            if (new_string_line > 0)
                print_char(new_string_line);
            break;
        default:
            fprintf(write_file[selector], "\n");
            break;
    }
    /*tex |tally| is not affected */
}

/*tex

The |print_char| procedure sends one byte to the desired destination. All
printing comes through |print_ln| or |print_char|, except for the case of
|tprint| (see below).

The checking of the line length is an inheritance from previosu engines and we
might drop it after release 1.0. We're not too picky about the exact match of
that length because we have utf output so length is then a bit fuzzy anyway.

*/

#define needs_escaping(A) \
    ((! escape_controls) || (A>=0x20) || (A==0x0A) || (A==0x0D) || (A==0x09))

#define escaped_char(A) \
    A+64


#define wlog_char(A) \
    if (needs_escaping(A)) { \
        wlog(A); \
    } else { \
        if (file_offset+2>=max_print_line) { \
            wlog_cr(); \
            file_offset=0; \
        } \
        wlog('^'); \
        wlog('^'); \
        wlog(escaped_char(A)); \
        file_offset += 2; \
    }


#define wterm_char(A) \
    if (needs_escaping(A)) { \
        wterm(A); \
    } else { \
        if (term_offset+2>=max_print_line) { \
            wterm_cr(); \
            term_offset=0; \
        } \
        wterm('^'); \
        wterm('^'); \
        wterm(escaped_char(A)); \
        term_offset += 2; \
    }

#define needs_wrapping(A,B) \
  (   (A>=0xC0) && \
    (((A>=0xF0) && (B+4>=max_print_line)) || \
     ((A>=0xE0) && (B+3>=max_print_line)) || \
     (             (B+2>=max_print_line))) \
  )

#define fix_term_offset(A) \
    if (needs_wrapping(A,term_offset)){ \
        wterm_cr(); \
        term_offset=0; \
    }

#define fix_log_offset(A) \
    if (needs_wrapping(A,file_offset)){ \
        wlog_cr(); \
        file_offset=0; \
    }

void print_char(int s)
{
    if (s < 0 || s > 255) {
        formatted_warning("print","weird character %i",s);
        return;
    }
    if (s == new_line_char_par) {
        if (selector < pseudo) {
            print_ln();
            return;
        }
    }
    switch (selector) {
        case no_print:
            break;
        case term_only:
            fix_term_offset(s);
            wterm_char(s);
            incr(term_offset);
            if (term_offset == max_print_line) {
                wterm_cr();
                term_offset = 0;
            }
            break;
        case log_only:
            fix_log_offset(s);
            wlog_char(s);
            incr(file_offset);
            if (file_offset == max_print_line) {
                wlog_cr();
                file_offset = 0;
            }
            break;
        case term_and_log:
            fix_term_offset(s);
            fix_log_offset(s);
            wterm_char(s);
            wlog_char(s);
            incr(term_offset);
            incr(file_offset);
            if (term_offset == max_print_line) {
                wterm_cr();
                term_offset = 0;
            }
            if (file_offset == max_print_line) {
                wlog_cr();
                file_offset = 0;
            }
            break;
        case pseudo:
            if (tally < trick_count)
                trick_buf[tally % error_line] = (packed_ASCII_code) s;
            break;
        case new_string:
            append_char(s);
            break;
        default:
            fprintf(write_file[selector], "%c", s);
    }
    incr(tally);
}

/*tex

An entire string is output by calling |print|. Note that if we are outputting the
single standard ASCII character \.c, we could call |print("c")|, since |"c"=99|
is the number of a single-character string, as explained above. But
|print_char("c")| is quicker, so \TeX\ goes directly to the |print_char| routine
when it knows that this is safe. (The present implementation assumes that it is
always safe to print a visible ASCII character.)

The first 256 entries above the 17th unicode plane are used for a special trick:
when \TeX\ has to print items in that range, it will instead print the character
that results from substracting 0x110000 from that value. This allows
byte-oriented output to things like \.{\\specials} and \.{\\pdfextension
literals}. Todo: Perhaps it would be useful to do the same substraction while
typesetting.


*/

void print(int s)
{
    if (s >= str_ptr) {
        normal_warning("print","bad string pointer");
        return;
    } else if (s < STRING_OFFSET) {
        if (s < 0) {
            normal_warning("print","bad string offset");
        } else {
            /*tex We're not sure about this so it's disabled for now! */
            if ((false) && (selector > pseudo)) {
                /*tex internal strings are not expanded */
                print_char(s);
                return;
            }
            if (s == new_line_char_par) {
                if (selector < pseudo) {
                    print_ln();
                    return;
                }
            }
            if (s <= 0x7F) {
                print_char(s);
            } else if (s <= 0x7FF) {
                print_char(0xC0 + (s / 0x40));
                print_char(0x80 + (s % 0x40));
            } else if (s <= 0xFFFF) {
                print_char(0xE0 + (s / 0x1000));
                print_char(0x80 + ((s % 0x1000) / 0x40));
                print_char(0x80 + ((s % 0x1000) % 0x40));
            } else if (s >= 0x110000) {
                int c = s - 0x110000;
                if (c >= 256) {
                    formatted_warning("print", "bad raw byte to print (c=%d), skipped",c);
                } else {
                    print_char(c);
                }
            } else {
                print_char(0xF0 + (s / 0x40000));
                print_char(0x80 + ((s % 0x40000) / 0x1000));
                print_char(0x80 + (((s % 0x40000) % 0x1000) / 0x40));
                print_char(0x80 + (((s % 0x40000) % 0x1000) % 0x40));
            }
        }
        return;
    }
    if (selector == new_string) {
        append_string(str_string(s), (unsigned) str_length(s));
        return;
    }
    lprint(&str_lstring(s));
}

void lprint (lstring *ss) {
    /*tex current character code position */
    unsigned char *j, *l;
    j = ss->s;
    l = j + ss->l;
    while (j < l) {
        /*tex I don't bother checking the last two bytes explicitly */
        /* 0x110000 in utf=8: 0xF4 0x90 0x80 0x80 */
        if ((j < l - 4) && (*j == 0xF4) && (*(j + 1) == 0x90)) {
            int c = (*(j + 2) - 128) * 64 + (*(j + 3) - 128);
            assert(c >= 0 && c < 256);
            print_char(c);
            j = j + 4;
        } else {
            print_char(*j);
            incr(j);
        }
    }
}

/*tex

The procedure |print_nl| is like |print|, but it makes sure that the string
appears at the beginning of a new line.

*/

/*tex Moev to the beginning of the next line. */

void print_nlp(void)
{
    if (new_string_line > 0) {
        print_char(new_string_line);
    } else if ((selector < no_print) ||
               ((term_offset > 0) && (odd(selector))) ||
               ((file_offset > 0) && (selector >= log_only))) {
        print_ln();
    }
}

/*tex Prints string |s| at beginning of the next line. */

void print_nl(str_number s)
{
    print_nlp();
    print(s);
}

/*tex

|char *| versions of the same procedures. |tprint| is different because it uses
buffering, which works well because most of the output actually comes through
|tprint|.

*/

#define t_flush_buffer(target,offset) \
    buffer[i++] = '\n'; \
    buffer[i++] = '\0';\
    fputs(buffer, target); \
    i = 0; \
    buffer[0] = '\0'; \
    offset=0;

void tprint(const char *sss)
{
    char *buffer = NULL;
    int i = 0;
    int newlinechar = (lua_only == 1) ? 10 : new_line_char_par;
    int dolog = 0;
    int doterm = 0;
    switch (selector) {
        case no_print:
            return;
            break;
        case term_only:
            doterm = 1;
            break;
        case log_only:
            dolog = 1;
            break;
        case term_and_log:
            dolog = 1;
            doterm = 1;
            break;
        case pseudo:
            while (*sss) {
               if (tally < trick_count) {
                   trick_buf[tally % error_line] = (packed_ASCII_code) *sss++;
               tally++;
               } else {
                   return;
               }
            }
            return;
            break;
        case new_string:
            append_string((const unsigned char *)sss, (unsigned) strlen(sss));
            return;
            break;
        default:
            {
                char *newstr = xstrdup(sss);
                char *s;
                for (s=newstr;*s;s++) {
                    if (*s == newlinechar) {
                        *s = '\n';
                    }
                }
                fputs(newstr, write_file[selector]);
                free(newstr);
                return;
            }
            break;
    }
    /*tex What is left is the 3 term/log settings. */
    if (dolog || doterm) {
#if defined(MIKTEX)
        buffer = xmalloc(strlen(sss)*3 + 10);
#else
        buffer = xmalloc(strlen(sss)*3);
#endif
        buffer[0] = '\0';
        /*tex The |wrapup_run| callback acts when the log file is already closed.*/
        if (dolog && log_opened_global) {
            const unsigned char *ss = (const unsigned char *) sss;
            while (*ss) {
                int s = *ss++;
                if (needs_wrapping(s,file_offset) || s == newlinechar) {
                    t_flush_buffer(log_file,file_offset);
                }
                if (s != newlinechar) {
                    buffer[i++] = s;
                    if (file_offset++ == max_print_line) {
                        t_flush_buffer(log_file,file_offset);
                    }
                }
            }
            if (*buffer) {
                buffer[i++] = '\0';
                fputs(buffer, log_file);
                buffer[0] = '\0';
            }
            i = 0;
        }
        if (doterm) {
            const unsigned char *ss = (const unsigned char *) sss;
            while (*ss) {
                int s = *ss++;
                if (needs_wrapping(s,term_offset) || s == newlinechar) {
                    t_flush_buffer(term_out,term_offset);
                }
                if (s != newlinechar) {
                    if (needs_escaping(s)) {
                        buffer[i++] = s;
                    } else {
                        buffer[i++] = '^';
                        buffer[i++] = '^';
                        buffer[i++] = escaped_char(s);
                        term_offset += 2;
                    }
                    if (++term_offset == max_print_line) {
                        t_flush_buffer(term_out,term_offset);
                    }
                }
            }
            if (*buffer) {
                buffer[i++] = '\0';
                fputs(buffer, term_out);
            }
        }
        free(buffer);
    }
}

void tprint_nl(const char *s)
{
    print_nlp();
    tprint(s);
}

/*tex

Here is the very first thing that \TeX\ prints: a headline that identifies the
version number and format package. The |term_offset| variable is temporarily
incorrect, but the discrepancy is not serious since we assume that the banner and
format identifier together will occupy at most |max_print_line| character
positions.

*/

void print_banner(const char *v)
{
    int callback_id = callback_defined(start_run_callback);
    if (callback_id == 0) {
#if defined(MIKTEX)
        miktex_print_banner(term_out, MyName, v);
#else
        fprintf(term_out, "This is " MyName ", Version %s%s ", v, WEB2CVERSION);
#endif
        if (format_ident > 0)
            print(format_ident);
        print_ln();
        if (show_luahashchars) {
            wterm(' ');
            fprintf(term_out,"Number of bits used by the hash function (" my_name "): %d",LUAI_HASHLIMIT);
            print_ln();
        }
        if (shellenabledp) {
            wterm(' ');
            if (restrictedshell) {
                fprintf(term_out, "restricted ");
            }
            fprintf(term_out, "system commands enabled.\n");
        }
    } else if (callback_id > 0) {
        run_callback(callback_id, "->");
    }
}

void log_banner(const char *v)
{
    const char *months[] = { "   ",
        "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
        "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
    };
    unsigned month = (unsigned) month_par;
    if (month > 12)
        month = 0;
#if defined(MIKTEX)
    miktex_print_banner(log_file, MyName, v);
#else
    fprintf(log_file, "This is " MyName ", Version %s%s ", v, WEB2CVERSION);
#endif
    print(format_ident);
    print_char(' ');
    print_char(' ');
    print_int(day_par);
    print_char(' ');
    fprintf(log_file, "%s", months[month]);
    print_char(' ');
    print_int(year_par);
    print_char(' ');
    print_two(time_par / 60);
    print_char(':');
    print_two(time_par % 60);
    if (shellenabledp) {
        wlog_cr();
        wlog(' ');
        if (restrictedshell)
            fprintf(log_file, "restricted ");
        fprintf(log_file, "system commands enabled.");
    }
    if (filelineerrorstylep) {
        wlog_cr();
        fprintf(log_file, " file:line:error style messages enabled.");
    }
}

void print_version_banner(void)
{
    fprintf(term_out, "%s", luatex_banner);
}

/*tex

The procedure |print_esc| prints a string that is preceded by the user's escape
character (which is usually a backslash).

*/

void print_esc(str_number s)
{
    /*tex Set variable |c| to the current escape character: */
    int c = escape_char_par;
    if (c >= 0 && c < 0x110000)
        print(c);
    print(s);
}

/*tex

This prints escape character, then |s|.

*/

void tprint_esc(const char *s)
{
    /*tex Set variable |c| to the current escape character: */
    int c = escape_char_par;
    if (c >= 0 && c < 0x110000)
        print(c);
    tprint(s);
}

/*tex

An array of digits in the range |0..15| is printed by |print_the_digs|.

*/

void print_the_digs(eight_bits k)
{
    /*tex prints |dig[k-1]|$\,\ldots\,$|dig[0]| */
    while (k-- > 0) {
        if (dig[k] < 10)
            print_char('0' + dig[k]);
        else
            print_char('A' - 10 + dig[k]);
    }
}

/*tex

The following procedure, which prints out the decimal representation of a given
integer |n|, has been written carefully so that it works properly if |n=0| or if
|(-n)| would cause overflow. It does not apply |mod| or |div| to negative
arguments, since such operations are not implemented consistently by all PASCAL
compilers.

*/

void print_int(longinteger n)
{
    /*tex index to current digit; we assume that $|n|<10^{23}$ */
    int k = 0;
    /*tex used to negate |n| in possibly dangerous cases */
    longinteger m;
    if (n < 0) {
        print_char('-');
        if (n > -100000000) {
            n = -n;
        } else {
            m = -1 - n;
            n = m / 10;
            m = (m % 10) + 1;
            k = 1;
            if (m < 10)
                dig[0] = (int) m;
            else {
                dig[0] = 0;
                incr(n);
            }
        }
    }
    do {
        dig[k] = (int) (n % 10);
        n = n / 10;
        incr(k);
    } while (n != 0);
    print_the_digs((eight_bits) k);
}

/*tex

Here is a trivial procedure to print two digits; it is usually called with a
parameter in the range |0<=n<=99|.

*/

void print_two(int n)
{
    n = abs(n) % 100;
    print_char('0' + (n / 10));
    print_char('0' + (n % 10));
}

/*tex

Hexadecimal printing of nonnegative integers is accomplished by |print_hex|.

*/

void print_qhex(int n)
{
    /*tex index to current digit; we assume that $0\L n<16^{22}$ */
    int k = 0 ;
    print_char('"');
    do {
        dig[k] = n % 16;
        n = n / 16;
        incr(k);
    } while (n != 0);
    print_the_digs((eight_bits) k);
}

/*tex

Roman numerals are produced by the |print_roman_int| routine. Readers who like
puzzles might enjoy trying to figure out how this tricky code works; therefore no
explanation will be given. Notice that 1990 yields \.{mcmxc}, not \.{mxm}.

*/

void print_roman_int(int n)
{
    char *j, *k;
    int u, v;
    char mystery[] = "m2d5c2l5x2v5i";
    j = (char *) mystery;
    v = 1000;
    while (1) {
        while (n >= v) {
            print_char(*j);
            n = n - v;
        }
        if (n <= 0) {
            /*tex nonpositive input produces no output */
            return;
        }
        k = j + 2;
        u = v / (*(k - 1) - '0');
        if (*(k - 1) == '2') {
            k = k + 2;
            u = u / (*(k - 1) - '0');
        }
        if (n + u >= v) {
            print_char(*k);
            n = n + u;
        } else {
            j = j + 2;
            v = v / (*(j - 1) - '0');
        }
    }
}

/*tex

The |print| subroutine will not print a string that is still being created. The
following procedure will.

*/

void print_current_string(void)
{
    /*tex points to current character code */
    unsigned j = 0;
    while (j < cur_length)
        print_char(cur_string[j++]);
}

/*tex

The procedure |print_cs| prints the name of a control sequence, given a pointer
to its address in |eqtb|. A space is printed after the name unless it is a single
nonletter or an active character. This procedure might be invoked with invalid
data, so it is ``extra robust.'' The individual characters must be printed one at
a time using |print|, since they may be unprintable.

*/

void print_cs(int p)
{
    str_number t = cs_text(p);
    if (p < hash_base) {
        /* nullcs */
        if (p == null_cs) {
            tprint_esc("csname");
            tprint_esc("endcsname");
            print_char(' ');
        } else {
            tprint_esc("IMPOSSIBLE.");
        }
    } else if ((p >= undefined_control_sequence) &&
               ((p <= eqtb_size) || p > eqtb_size + hash_extra)) {
        tprint_esc("IMPOSSIBLE.");
    } else if (t >= str_ptr) {
        tprint_esc("NONEXISTENT.");
    } else {
        if (is_active_cs(t)) {
            print(active_cs_value(t));
        } else {
            print_esc(t);
            if (single_letter(t)) {
                if (get_cat_code(cat_code_table_par, pool_to_unichar(str_string(t))) == letter_cmd)
                    print_char(' ');
            } else {
                print_char(' ');
            }
        }
    }
}

/*tex

Here is a similar procedure; it avoids the error checks, and it never prints a
space after the control sequence.

*/
void sprint_cs(pointer p)
{
    str_number t;
    if (p == null_cs) {
        tprint_esc("csname");
        tprint_esc("endcsname");
    } else {
        t = cs_text(p);
        if (is_active_cs(t))
            print(active_cs_value(t));
        else
            print_esc(t);
    }
}

void sprint_cs_name(pointer p)
{
    str_number t;
    if (p != null_cs) {
        t = cs_text(p);
        if (is_active_cs(t))
            print(active_cs_value(t));
        else
            print(t);
    }
}

/*tex

This procedure is never called when |interaction<scroll_mode|.

*/

void prompt_input(const char *s)
{
    wake_up_terminal();
    tprint(s);
    term_input();
}

/*tex

Then there is a subroutine that prints glue stretch and shrink, possibly followed
by the name of finite units:

*/

void print_glue(scaled d, int order, const char *s)
{
    print_scaled(d);
    if ((order < normal) || (order > filll)) {
        tprint("foul");
    } else if (order > normal) {
        tprint("fi");
        while (order > sfi) {
            print_char('l');
            decr(order);
        }
    } else if (s != NULL) {
        tprint(s);
    }
}

/*tex

The next subroutine prints a whole glue specification

*/

void print_spec(int p, const char *s)
{
    if (p < 0) {
        print_char('*');
    } else {
        print_scaled(width(p));
        if (s != NULL)
            tprint(s);
        if (stretch(p) != 0) {
            tprint(" plus ");
            print_glue(stretch(p), stretch_order(p), s);
        }
        if (shrink(p) != 0) {
            tprint(" minus ");
            print_glue(shrink(p), shrink_order(p), s);
        }
    }
}

/*tex

We can reinforce our knowledge of the data structures just introduced by
considering two procedures that display a list in symbolic form. The first of
these, called |short_display|, is used in ``overfull box'' messages to give the
top-level description of a list. The other one, called |show_node_list|, prints a
detailed description of exactly what is in the data structure.

The philosophy of |short_display| is to ignore the fine points about exactly what
is inside boxes, except that ligatures and discretionary breaks are expanded. As
a result, |short_display| is a recursive procedure, but the recursion is never
more than one level deep. @^recursion@>

A global variable |font_in_short_display| keeps track of the font code that is
assumed to be present when |short_display| begins; deviations from this font will
be printed.

*/

/*tex An internal font number: */

int font_in_short_display;

/*tex

Boxes, rules, inserts, whatsits, marks, and things in general that are sort of
``complicated'' are indicated only by printing `\.{[]}'.

We print a bit more than original \TEX. A value of 0 or 1 or any large value will
behave the same as before. The reason for this extension is that a |name| not
always makes sense.

\starttyping
0   \foo xyz
1   \foo (bar)
2   <bar> xyz
3   <bar @ ..> xyz
4   <id>
5   <id: bar>
6   <id: bar @ ..> xyz
\stoptyping

*/

void print_font_identifier(internal_font_number f)
{
    str_number fonttext;
    fonttext = font_id_text(f);
    if (tracing_fonts_par >= 2 && tracing_fonts_par <= 6) {
        /*tex < > is less likely to clash with text parenthesis */
        tprint("<");
        if (tracing_fonts_par >= 2 && tracing_fonts_par <= 3) {
            print_font_name(f);
            if (tracing_fonts_par >= 3 || font_size(f) != font_dsize(f)) {
                tprint(" @ ");
                print_scaled(font_size(f));
                tprint("pt");
            }
        } else if (tracing_fonts_par >= 4 && tracing_fonts_par <= 6) {
            print_int(f);
            if (tracing_fonts_par >= 5) {
                tprint(": ");
                print_font_name(f);
                if (tracing_fonts_par >= 6 || font_size(f) != font_dsize(f)) {
                    tprint(" @ ");
                    print_scaled(font_size(f));
                    tprint("pt");
                }
            }
        }
        print_char('>');
    } else {
        /*tex old method, inherited from pdftex  */
        if (fonttext > 0) {
            print_esc(fonttext);
        } else {
            tprint_esc("FONT");
            print_int(f);
        }
        if (tracing_fonts_par > 0) {
            tprint(" (");
            print_font_name(f);
            if (font_size(f) != font_dsize(f)) {
                tprint("@");
                print_scaled(font_size(f));
                tprint("pt");
            }
            print_char(')');
        }
    }
}

/*tex
    We could do this much nicer but then we need to also adapt short_display a
    bit and we have to be as compatible as possible in the log for some macro
    packages.

    The callback is also responsible for either or not reporting the character
    number itself.
*/

void print_character_info(halfword p)
{
    int callback_id = callback_defined(glyph_info_callback);
    if (callback_id) {
        char* str = NULL;
        run_callback(callback_id, "N->R", p, &str);
        if (str == NULL) {
            print_qhex(character(p));
        } else {
            tprint(str);
            free(str);
        }
    } else {
        print(character(p));
    }
}

/*tex
    This prints highlights of list |p|.
*/

void short_display(int p)
{
    while (p != null) {
        if (is_char_node(p)) {
            if (lig_ptr(p) != null) {
                short_display(lig_ptr(p));
            } else {
                if (font(p) != font_in_short_display) {
                    if (!is_valid_font(font(p))) {
                        print_char('*');
                    } else {
                        print_font_identifier(font(p));
                    }
                    print_char(' ');
                    font_in_short_display = font(p);
                }
                print_character_info(p);
            }
        } else {
            /*tex Print a short indication of the contents of node |p| */
            print_short_node_contents(p);
        }
        p = vlink(p);
    }
}

/*tex

The |show_node_list| routine requires some auxiliary subroutines: one to print a
font-and-character combination, one to print a token list without its reference
count, and one to print a rule dimension.

*/

/*tex

This prints |char_node| data.

*/

void print_font_and_char(int p)
{
    if (!is_valid_font(font(p)))
        print_char('*');
    else
        print_font_identifier(font(p));
    print_char(' ');
    print_character_info(p);
}

/*tex

This prints token list data in braces

*/

void print_mark(int p)
{
    print_char('{');
    if ((p < (int) fix_mem_min) || (p > (int) fix_mem_end))
        tprint_esc("CLOBBERED.");
    else
        show_token_list(token_link(p), null, max_print_line - 10);
    print_char('}');
}

/*tex

This prints dimensions of a rule node.

*/

void print_rule_dimen(scaled d)
{
    if (is_running(d))
        print_char('*');
    else
        print_scaled(d);
}

/*tex

Since boxes can be inside of boxes, |show_node_list| is inherently recursive,
@^recursion@> up to a given maximum number of levels. The history of nesting is
indicated by the current string, which will be printed at the beginning of each
line; the length of this string, namely |cur_length|, is the depth of nesting.

A global variable called |depth_threshold| is used to record the maximum depth of
nesting for which |show_node_list| will show information. If we have
|depth_threshold=0|, for example, only the top level information will be given
and no sublists will be traversed. Another global variable, called |breadth_max|,
tells the maximum number of items to show at each level; |breadth_max| had better
be positive, or you won't see anything.

*/

/*tex The maximum nesting depth in box displays: */

int depth_threshold;

/*tex The maximum number of items shown at the same list level: */

int breadth_max;

/*tex

The recursive machinery is started by calling |show_box|. Assign the values
|depth_threshold:=show_box_depth| and |breadth_max:=show_box_breadth|

*/

void show_box(halfword p)
{
    depth_threshold = show_box_depth_par;
    breadth_max = show_box_breadth_par;
    if (breadth_max <= 0)
        breadth_max = 5;
    /*tex the show starts at |p| */
    show_node_list(p);
    print_ln();
}

/*tex

Helper for debugging purposes. It prints highlights of list |p|

*/

void short_display_n(int p, int m)
{
    int i = 0;
    font_in_short_display = null_font;
    if (p == null)
        return;
    while (p != null) {
        if (is_char_node(p)) {
            if (p <= max_halfword) {
                if (font(p) != font_in_short_display) {
                    if (!is_valid_font(font(p)))
                        print_char('*');
                    else
                        print_font_identifier(font(p));
                    print_char(' ');
                    font_in_short_display = font(p);
                }
                print_character_info(p);
            }
        } else {
            if ( (type(p) == glue_node) ||
                 (type(p) == disc_node) ||
                 (type(p) == penalty_node) ||
                ((type(p) == kern_node) && (subtype(p) == explicit_kern ||
                                            subtype(p) == italic_kern   ))) {
                incr(i);
            }
            if (i >= m)
                return;
            if (type(p) == disc_node) {
                print_char('|');
                short_display(vlink(pre_break(p)));
                print_char('|');
                short_display(vlink(post_break(p)));
                print_char('|');
            } else {
                /*tex Print a short indication of the contents of node |p| */
                print_short_node_contents(p);
            }
        }
        p = vlink(p);
        if (p == null)
            return;
    }
    update_terminal();
}

/*tex

When debugging a macro package, it can be useful to see the exact control
sequence names in the format file. For example, if ten new csnames appear, it's
nice to know what they are, to help pinpoint where they came from. (This isn't a
truly ``basic'' printing procedure, but that's a convenient module in which to
put it.)

*/

void print_csnames(int hstart, int hfinish)
{
    int h;
    unsigned char *c, *l;
    fprintf(stderr, "fmtdebug:csnames from %d to %d:", (int) hstart, (int) hfinish);
    for (h = hstart; h <= hfinish; h++) {
        if (cs_text(h) > 0) {
            /*tex We have anything at this position. */
            c = str_string(cs_text(h));
            l = c + str_length(cs_text(h));
            while (c < l) {
                /*tex Print the characters. */
                fputc(*c++, stderr);
            }
            fprintf(stderr, "|");
        }
    }
}

/*tex

A helper for printing file:line:error style messages. Look for a filename in
|full_source_filename_stack|, and if we fail to find one fall back on the
non-file:line:error style.

*/

void print_file_line(void)
{
    int level = in_open;
    while ((level > 0) && (full_source_filename_stack[level] == 0))
        decr(level);
    if (level == 0) {
        tprint_nl("! ");
    } else {
        tprint_nl("");
        tprint(full_source_filename_stack[level]);
        print_char(':');
        if (level == in_open)
            print_int(line);
        else
            print_int(line_stack[level + 1]);
        tprint(": ");
    }
}

/*tex

\TeX\ is occasionally supposed to print diagnostic information that goes only
into the transcript file, unless |tracing_online| is positive. Here are two
routines that adjust the destination of print commands:

*/

void begin_diagnostic(void)
{
    global_old_setting = selector;
    if ((tracing_online_par <= 0) && (selector == term_and_log)) {
        decr(selector);
        if (history == spotless)
            history = warning_issued;
    }
}

/*tex

Restore proper conditions after tracing.

*/

void end_diagnostic(boolean blank_line)
{
    tprint_nl("");
    if (blank_line)
        print_ln();
    selector = global_old_setting;
}

/*
    Indentation (based on a suggestion by PO on tex-implementors but adapted to
    out situation and with built-in limits and so.
*/

void print_input_level(void)
{
    int callback_id = callback_defined(input_level_string_callback);
    if (callback_id>0) {
        char *s = NULL;
        if (run_callback(callback_id, "d->S", input_ptr, &s)) {
            if (s && strlen(s) > 0) {
                tprint_nl(s);
                free(s);
            } else {
                print_ln();
            }
        } else {
            print_ln();
        }
    } else {
        int m = level_max;
        if (m) {
            int l = input_ptr;
            int c = level_chr > 0 ? level_chr : '.';
            if (l > m) {
                tprint_nl("[");
                print_int((l/m)*m);
                print(']');
                l = l % m;
            } else {
                print_ln();
            }
            while (l > 0) {
               print(c);
               l--;
            }
        } else {
            print_ln();
        }
    }
}

/*tex

Of course we had better declare another global variable, if the previous routines
are going to work.

*/

int global_old_setting;
