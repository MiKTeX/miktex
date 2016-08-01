% textoken.w
%
% Copyright 2006-2011 Taco Hoekwater <taco@@luatex.org>
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
#define detokenized_line() (line_catcode_table==NO_CAT_TABLE)

/*
#define do_get_cat_code(a,b) do { \
    if (line_catcode_table<=-0xFF) \
      a= - line_catcode_table - 0xFF ; \
    else if (line_catcode_table!=DEFAULT_CAT_TABLE) \
      a=get_cat_code(line_catcode_table,b); \
    else \
      a=get_cat_code(cat_code_table_par,b); \
  } while (0)
*/

#define do_get_cat_code(a,b) do { \
    if (line_catcode_table==DEFAULT_CAT_TABLE) \
      a=get_cat_code(cat_code_table_par,b); \
    else if (line_catcode_table>-0xFF) \
      a=get_cat_code(line_catcode_table,b); \
    else \
      a= - line_catcode_table - 0xFF ; \
  } while (0)


@ The \TeX\ system does nearly all of its own memory allocation, so that it can
readily be transported into environments that do not have automatic facilities
for strings, garbage collection, etc., and so that it can be in control of what
error messages the user receives. The dynamic storage requirements of \TeX\ are
handled by providing two large arrays called |fixmem| and |varmem| in which
consecutive blocks of words are used as nodes by the \TeX\ routines.

Pointer variables are indices into this array, or into another array called
|eqtb| that will be explained later. A pointer variable might also be a special
flag that lies outside the bounds of |mem|, so we allow pointers to assume any
|halfword| value. The minimum halfword value represents a null pointer. \TeX\
does not assume that |mem[null]| exists.

@ Locations in |fixmem| are used for storing one-word records; a conventional
\.{AVAIL} stack is used for allocation in this array.

@c
smemory_word *fixmem;           /* the big dynamic storage area */
unsigned fix_mem_min;           /* the smallest location of one-word memory in use */
unsigned fix_mem_max;           /* the largest location of one-word memory in use */

@ In order to study the memory requirements of particular applications, it is
possible to prepare a version of \TeX\ that keeps track of current and maximum
memory usage. When code between the delimiters |@!stat| $\ldots$ |tats| is not
commented out, \TeX\ will run a bit slower but it will report these statistics
when |tracing_stats| is sufficiently large.

@c
int var_used, dyn_used;         /* how much memory is in use */

halfword avail;                 /* head of the list of available one-word nodes */
unsigned fix_mem_end;           /* the last one-word node used in |mem| */

halfword garbage;               /* head of a junk list, write only */
halfword temp_token_head;       /* head of a temporary list of some kind */
halfword hold_token_head;       /* head of a temporary list of another kind */
halfword omit_template;         /* a constant token list */
halfword null_list;             /* permanently empty list */
halfword backup_head;           /* head of token list built by |scan_keyword| */

@ @c
void initialize_tokens(void)
{
    halfword p;
    avail = null;
    fix_mem_end = 0;
    p = get_avail();
    temp_token_head = p;
    set_token_info(temp_token_head, 0);
    p = get_avail();
    hold_token_head = p;
    set_token_info(hold_token_head, 0);
    p = get_avail();
    omit_template = p;
    set_token_info(omit_template, 0);
    p = get_avail();
    null_list = p;
    set_token_info(null_list, 0);
    p = get_avail();
    backup_head = p;
    set_token_info(backup_head, 0);
    p = get_avail();
    garbage = p;
    set_token_info(garbage, 0);
    dyn_used = 0;               /* initialize statistics */
}

@ The function |get_avail| returns a pointer to a new one-word node whose |link|
field is null. However, \TeX\ will halt if there is no more room left.
@^inner loop@>

If the available-space list is empty, i.e., if |avail=null|, we try first to
increase |fix_mem_end|. If that cannot be done, i.e., if
|fix_mem_end=fix_mem_max|, we try to reallocate array |fixmem|. If, that doesn't
work, we have to quit.

@c
halfword get_avail(void)
{                               /* single-word node allocation */
    unsigned p;                 /* the new node being got */
    unsigned t;
    p = (unsigned) avail;       /* get top location in the |avail| stack */
    if (p != null) {
        avail = token_link(avail);      /* and pop it off */
    } else if (fix_mem_end < fix_mem_max) {     /* or go into virgin territory */
        incr(fix_mem_end);
        p = fix_mem_end;
    } else {
        smemory_word *new_fixmem;       /* the big dynamic storage area */
        t = (fix_mem_max / 5);
        new_fixmem =
            fixmemcast(realloc
                       (fixmem, sizeof(smemory_word) * (fix_mem_max + t + 1)));
        if (new_fixmem == NULL) {
            runaway();          /* if memory is exhausted, display possible runaway text */
            overflow("token memory size", fix_mem_max);
        } else {
            fixmem = new_fixmem;
        }
        memset(voidcast(fixmem + fix_mem_max + 1), 0, t * sizeof(smemory_word));
        fix_mem_max += t;
        p = ++fix_mem_end;
    }
    token_link(p) = null;       /* provide an oft-desired initialization of the new node */
    incr(dyn_used);             /* maintain statistics */
    return (halfword) p;
}

@ The procedure |flush_list(p)| frees an entire linked list of one-word nodes
that starts at position |p|.
@^inner loop@>

@c
void flush_list(halfword p)
{                               /* makes list of single-word nodes available */
    halfword q, r;              /* list traversers */
    if (p != null) {
        r = p;
        do {
            q = r;
            r = token_link(r);
            decr(dyn_used);
        } while (r != null);    /* now |q| is the last node on the list */
        token_link(q) = avail;
        avail = p;
    }
}

@ A \TeX\ token is either a character or a control sequence, and it is @^token@>
represented internally in one of two ways: (1)~A character whose ASCII code
number is |c| and whose command code is |m| is represented as the number
$2^{21}m+c$; the command code is in the range |1<=m<=14|. (2)~A control sequence
whose |eqtb| address is |p| is represented as the number |cs_token_flag+p|. Here
|cs_token_flag=@t$2^{25}-1$@>| is larger than $2^{21}m+c$, yet it is small enough
that |cs_token_flag+p< max_halfword|; thus, a token fits comfortably in a
halfword.

A token |t| represents a |left_brace| command if and only if
|t<left_brace_limit|; it represents a |right_brace| command if and only if we
have |left_brace_limit<=t<right_brace_limit|; and it represents a |match| or
|end_match| command if and only if |match_token<=t<=end_match_token|. The
following definitions take care of these token-oriented constants and a few
others.

@ A token list is a singly linked list of one-word nodes in |mem|, where each
word contains a token and a link. Macro definitions, output-routine definitions,
marks, \.{\\write} texts, and a few other things are remembered by \TeX\ in the
form of token lists, usually preceded by a node with a reference count in its
|token_ref_count| field. The token stored in location |p| is called |info(p)|.

Three special commands appear in the token lists of macro definitions. When
|m=match|, it means that \TeX\ should scan a parameter for the current macro;
when |m=end_match|, it means that parameter matching should end and \TeX\ should
start reading the macro text; and when |m=out_param|, it means that \TeX\ should
insert parameter number |c| into the text at this point.

The enclosing \.{\char'173} and \.{\char'175} characters of a macro definition
are omitted, but the final right brace of an output routine is included at the
end of its token list.

Here is an example macro definition that illustrates these conventions. After
\TeX\ processes the text

$$\.{\\def\\mac a\#1\#2 \\b \{\#1\\-a \#\#1\#2 \#2\}}$$

the definition of \.{\\mac} is represented as a token list containing

$$\def\,{\hskip2pt}
\vbox{\halign{\hfil#\hfil\cr
(reference count), |letter|\,\.a, |match|\,\#, |match|\,\#, |spacer|\,\.\ ,
\.{\\b}, |end_match|,\cr
|out_param|\,1, \.{\\-}, |letter|\,\.a, |spacer|\,\.\ , |mac_param|\,\#,
|other_char|\,\.1,\cr
|out_param|\,2, |spacer|\,\.\ , |out_param|\,2.\cr}}$$

The procedure |scan_toks| builds such token lists, and |macro_call| does the
parameter matching. @^reference counts@>

Examples such as $$\.{\\def\\m\{\\def\\m\{a\}\ b\}}$$ explain why reference
counts would be needed even if \TeX\ had no \.{\\let} operation: When the token
list for \.{\\m} is being read, the redefinition of \.{\\m} changes the |eqtb|
entry before the token list has been fully consumed, so we dare not simply
destroy a token list when its control sequence is being redefined.

If the parameter-matching part of a definition ends with `\.{\#\{}', the
corresponding token list will have `\.\{' just before the `|end_match|' and also
at the very end. The first `\.\{' is used to delimit the parameter; the second
one keeps the first from disappearing.

The |print_meaning| subroutine displays |cur_cmd| and |cur_chr| in symbolic form,
including the expansion of a macro or mark.

@c
void print_meaning(void)
{
    /* remap \mathchar onto \Umathchar */
/*
    if (cur_cmd == math_given_cmd) {
        cur_cmd = xmath_given_cmd ;
    }
*/
    print_cmd_chr((quarterword) cur_cmd, cur_chr);
    if (cur_cmd >= call_cmd) {
        print_char(':');
        print_ln();
        token_show(cur_chr);
    } else {
        /* Show the meaning of a mark node */
        if ((cur_cmd == top_bot_mark_cmd) && (cur_chr < marks_code)) {
            print_char(':');
            print_ln();
            switch (cur_chr) {
                case first_mark_code:
                    token_show(first_mark(0));
                    break;
                case bot_mark_code:
                    token_show(bot_mark(0));
                    break;
                case split_first_mark_code:
                    token_show(split_first_mark(0));
                    break;
                case split_bot_mark_code:
                    token_show(split_bot_mark(0));
                    break;
                default:
                    token_show(top_mark(0));
                    break;
            }
        }
    }
}

@ The procedure |show_token_list|, which prints a symbolic form of the token list
that starts at a given node |p|, illustrates these conventions. The token list
being displayed should not begin with a reference count. However, the procedure
is intended to be robust, so that if the memory links are awry or if |p| is not
really a pointer to a token list, nothing catastrophic will happen.

An additional parameter |q| is also given; this parameter is either null or it
points to a node in the token list where a certain magic computation takes place
that will be explained later. (Basically, |q| is non-null when we are printing
the two-line context information at the time of an error message; |q| marks the
place corresponding to where the second line should begin.)

For example, if |p| points to the node containing the first \.a in the token list
above, then |show_token_list| will print the string $$\hbox{`\.{a\#1\#2\ \\b\
->\#1\\-a\ \#\#1\#2\ \#2}';}$$ and if |q| points to the node containing the
second \.a, the magic computation will be performed just before the second \.a is
printed.

The generation will stop, and `\.{\\ETC.}' will be printed, if the length of
printing exceeds a given limit~|l|. Anomalous entries are printed in the form of
control sequences that are not followed by a blank space, e.g., `\.{\\BAD.}';
this cannot be confused with actual control sequences because a real control
sequence named \.{BAD} would come out `\.{\\BAD\ }'.

@c
#define not_so_bad(p) \
    switch (m) { \
        case assign_int_cmd: \
            if (c >= (backend_int_base) && c <= (backend_int_last)) \
                p("[internal backend integer]"); \
            break; \
        case assign_dimen_cmd: \
            if (c >= (backend_dimen_base) && c <= (backend_dimen_last)) \
                p("[internal backend dimension]"); \
            break; \
        case assign_toks_cmd: \
            if (c >= (backend_toks_base) && c <= (backend_toks_last)) \
                p("[internal backend tokenlist]"); \
            break; \
        default: \
            p("BAD"); \
            break; \
    }

void show_token_list(int p, int q, int l)
{
    int m, c;                    /* pieces of a token */
    ASCII_code match_chr = '#';  /* character used in a `|match|' */
    ASCII_code n = '0';          /* the highest parameter number, as an ASCII digit */
    tally = 0;
    if (l < 0)
        l = 0x3FFFFFFF;
    while ((p != null) && (tally < l)) {
        if (p == q) {
            /* Do magic computation */
            set_trick_count();
        }
        /* Display token |p|, and |return| if there are problems */
        if ((p < (int) fix_mem_min) || (p > (int) fix_mem_end)) {
            tprint_esc("CLOBBERED.");
            return;
        }
        if (token_info(p) >= cs_token_flag) {
            if (!((inhibit_par_tokens) && (token_info(p) == par_token)))
                print_cs(token_info(p) - cs_token_flag);
        } else {
            m = token_cmd(token_info(p));
            c = token_chr(token_info(p));
            if (token_info(p) < 0) {
                tprint_esc("BAD");
            } else {
                /*
                    Display the token $(|m|,|c|)$

                    The procedure usually ``learns'' the character code used for macro
                    parameters by seeing one in a |match| command before it runs into any
                    |out_param| commands.
                */
                switch (m) {
                    case left_brace_cmd:
                    case right_brace_cmd:
                    case math_shift_cmd:
                    case tab_mark_cmd:
                    case sup_mark_cmd:
                    case sub_mark_cmd:
                    case spacer_cmd:
                    case letter_cmd:
                    case other_char_cmd:
                        print(c);
                        break;
                    case mac_param_cmd:
                        if (!in_lua_escape && (is_in_csname==0))
                            print(c);
                        print(c);
                        break;
                    case out_param_cmd:
                        print(match_chr);
                        if (c <= 9) {
                            print_char(c + '0');
                        } else {
                            print_char('!');
                            return;
                        }
                        break;
                    case match_cmd:
                        match_chr = c;
                        print(c);
                        incr(n);
                        print_char(n);
                        if (n > '9')
                            return;
                        break;
                    case end_match_cmd:
                        if (c == 0)
                            tprint("->");
                        break;
                    default:
                        not_so_bad(tprint);
                        break;
                }
            }
        }
        p = token_link(p);
    }
    if (p != null)
        tprint_esc("ETC.");
}

@ @c
#define do_buffer_to_unichar(a,b) do { \
    a = (halfword)str2uni(buffer+b); \
    b += utf8_size(a); \
} while (0)

@ Here's the way we sometimes want to display a token list, given a pointer to
its reference count; the pointer may be null.

@c
void token_show(halfword p)
{
    if (p != null)
        show_token_list(token_link(p), null, 10000000);
}

@ |delete_token_ref|, is called when a pointer to a token list's reference count
is being removed. This means that the token list should disappear if the
reference count was |null|, otherwise the count should be decreased by one.
@^reference counts@>

@ |p| points to the reference count of a token list that is losing one
reference.

@c
void delete_token_ref(halfword p)
{
    if (token_ref_count(p) == 0)
        flush_list(p);
    else
        decr(token_ref_count(p));
}

@ @c
int get_char_cat_code(int curchr)
{
    int a;
    do_get_cat_code(a,curchr);
    return a;
}

@ @c
static void invalid_character_error(void)
{
    const char *hlp[] = {
        "A funny symbol that I can't read has just been input.",
        "Continue, and I'll forget that it ever happened.",
        NULL
    };
    deletions_allowed = false;
    tex_error("Text line contains an invalid character", hlp);
    deletions_allowed = true;
}

@ @c
static boolean process_sup_mark(void);  /* below */

static int scan_control_sequence(void); /* below */

typedef enum {
    next_line_ok,
    next_line_return,
    next_line_restart
} next_line_retval;

static next_line_retval next_line(void); /* below */

@ In case you are getting bored, here is a slightly less trivial routine: Given a
string of lowercase letters, like `\.{pt}' or `\.{plus}' or `\.{width}', the
|scan_keyword| routine checks to see whether the next tokens of input match this
string. The match must be exact, except that uppercase letters will match their
lowercase counterparts; uppercase equivalents are determined by subtracting
|"a"-"A"|, rather than using the |uc_code| table, since \TeX\ uses this routine
only for its own limited set of keywords.

If a match is found, the characters are effectively removed from the input and
|true| is returned. Otherwise |false| is returned, and the input is left
essentially unchanged (except for the fact that some macros may have been
expanded, etc.). @^inner loop@>

@c
boolean scan_keyword(const char *s)
{                               /* look for a given string */
    halfword p;                 /* tail of the backup list */
    halfword q;                 /* new node being added to the token list via |store_new_token| */
    const char *k;              /* index into |str_pool| */
    halfword save_cur_cs = cur_cs;
    if (strlen(s) == 0)        /* was assert (strlen(s) > 1); */
      return false ;           /* but not with newtokenlib  zero keyword simply doesn't match  */
    p = backup_head;
    token_link(p) = null;
    k = s;
    while (*k) {
        get_x_token();      /* recursion is possible here */
        if ((cur_cs == 0) && ((cur_chr == *k) || (cur_chr == *k - 'a' + 'A'))) {
            store_new_token(cur_tok);
            k++;
        } else if ((cur_cmd != spacer_cmd) || (p != backup_head)) {
            /*
                crashes on some alignments:

                if (p != backup_head) {
                    q = get_avail();
                    token_info(q) = cur_tok;
                    token_link(q) = null;
                    token_link(p) = q;
                    begin_token_list(token_link(backup_head), backed_up);
                } else {
                    back_input();
                }
            */
            back_input();
            if (p != backup_head) {
                begin_token_list(token_link(backup_head), backed_up);
            }
            /*  */
            cur_cs = save_cur_cs;
            return false;
        }
    }
    if (token_link(backup_head) != null)
        flush_list(token_link(backup_head));
    cur_cs = save_cur_cs;
    return true;
}

@ We can not return |undefined_control_sequence| under some conditions
 (inside |shift_case|, for example). This needs thinking.

@c

/*
    halfword active_to_cs(int curchr, int force)
    {
        halfword curcs;
        char *a, *b;
        char *utfbytes = xmalloc(8);
        int nncs = no_new_control_sequence;
        a = (char *) uni2str(0xFFFF);
        utfbytes = strcpy(utfbytes, a);
        if (force)
            no_new_control_sequence = false;
        if (curchr > 0) {
            b = (char *) uni2str((unsigned) curchr);
            utfbytes = strcat(utfbytes, b);
            free(b);
            curcs = string_lookup(utfbytes, strlen(utfbytes));
        } else {
            utfbytes[3] = '\0';
            curcs = string_lookup(utfbytes, 4);
        }
        no_new_control_sequence = nncs;
        free(a);
        free(utfbytes);
        return curcs;
    }
*/

/*static char * FFFF = "\xEF\xBF\xBF";*/ /* 0xFFFF */

halfword active_to_cs(int curchr, int force)
{
    halfword curcs;
    int nncs = no_new_control_sequence;
    if (force) {
        no_new_control_sequence = false;
    }
    if (curchr > 0) {
        char *b = (char *) uni2str((unsigned) curchr);
        char *utfbytes = xmalloc(8);
        utfbytes = strcpy(utfbytes, "\xEF\xBF\xBF");
        utfbytes = strcat(utfbytes, b);
        free(b);
        curcs = string_lookup(utfbytes, utf8_size(curchr)+3);
        free(utfbytes);
    } else {
        curcs = string_lookup("\xEF\xBF\xBF", 4); /* 0xFFFF ... why not 3 ? */
    }
    no_new_control_sequence = nncs;
    return curcs;
}

/*

    static unsigned char *uni2csstr(unsigned unic)
    {
        unsigned char *buf = xmalloc(8);
        unsigned char *pt = buf;
        *pt++ = 239; *pt++ = 191; *pt++ = 191; // 0xFFFF
        if (unic < 0x80)
            *pt++ = (unsigned char) unic;
        else if (unic < 0x800) {
            *pt++ = (unsigned char) (0xc0 | (unic >> 6));
            *pt++ = (unsigned char) (0x80 | (unic & 0x3f));
        } else if (unic >= 0x110000) {
            *pt++ = (unsigned char) (unic - 0x110000);
        } else if (unic < 0x10000) {
            *pt++ = (unsigned char) (0xe0 | (unic >> 12));
            *pt++ = (unsigned char) (0x80 | ((unic >> 6) & 0x3f));
            *pt++ = (unsigned char) (0x80 | (unic & 0x3f));
        } else {
            int u, z, y, x;
            unsigned val = unic - 0x10000;
            u = (int) (((val & 0xf0000) >> 16) + 1);
            z = (int) ((val & 0x0f000) >> 12);
            y = (int) ((val & 0x00fc0) >> 6);
            x = (int) (val & 0x0003f);
            *pt++ = (unsigned char) (0xf0 | (u >> 2));
            *pt++ = (unsigned char) (0x80 | ((u & 3) << 4) | z);
            *pt++ = (unsigned char) (0x80 | y);
            *pt++ = (unsigned char) (0x80 | x);
        }
        *pt = '\0';
        return buf;
    }

    halfword active_to_cs(int curchr, int force)
    {
        halfword curcs;
        int nncs = no_new_control_sequence;
        if (force) {
            no_new_control_sequence = false;
        }
        if (curchr > 0) {
            char * utfbytes = (char *) uni2csstr((unsigned) curchr);
            curcs = string_lookup(utfbytes, utf8_size(curchr)+3);
            free(utfbytes);
        } else {
            curcs = string_lookup(FFFF, 4); // 0xFFFF ... why not 3 ?
        }
        no_new_control_sequence = nncs;
        return curcs;
    }

*/

@ TODO this function should listen to \.{\\escapechar}

@ prints a control sequence

@c
static char *cs_to_string(halfword p)
{
    const char *s;
    char *sh;
    int k = 0;
    static char ret[256] = { 0 };
    if (p == 0 || p == null_cs) {
        ret[k++] = '\\';
        s = "csname";
        while (*s) {
            ret[k++] = *s++;
        }
        ret[k++] = '\\';
        s = "endcsname";
        while (*s) {
            ret[k++] = *s++;
        }
        ret[k] = 0;

    } else {
        str_number txt = cs_text(p);
        sh = makecstring(txt);
        s = sh;
        if (is_active_cs(txt)) {
            s = s + 3;
            while (*s) {
                ret[k++] = *s++;
            }
            ret[k] = 0;
        } else {
            ret[k++] = '\\';
            while (*s) {
                ret[k++] = *s++;
            }
            ret[k] = 0;
        }
        free(sh);
    }
    return (char *) ret;
}

@ TODO this is a quick hack, will be solved differently soon

@c
static char *cmd_chr_to_string(int cmd, int chr)
{
    char *s;
    str_number str;
    int sel = selector;
    selector = new_string;
    print_cmd_chr((quarterword) cmd, chr);
    str = make_string();
    s = makecstring(str);
    selector = sel;
    flush_str(str);
    return s;
}

@ The heart of \TeX's input mechanism is the |get_next| procedure, which we shall
develop in the next few sections of the program. Perhaps we shouldn't actually
call it the ``heart,'' however, because it really acts as \TeX's eyes and mouth,
reading the source files and gobbling them up. And it also helps \TeX\ to
regurgitate stored token lists that are to be processed again. @^eyes and mouth@>

The main duty of |get_next| is to input one token and to set |cur_cmd| and
|cur_chr| to that token's command code and modifier. Furthermore, if the input
token is a control sequence, the |eqtb| location of that control sequence is
stored in |cur_cs|; otherwise |cur_cs| is set to zero.

Underlying this simple description is a certain amount of complexity because of
all the cases that need to be handled. However, the inner loop of |get_next| is
reasonably short and fast.

When |get_next| is asked to get the next token of a \.{\\read} line,
it sets |cur_cmd=cur_chr=cur_cs=0| in the case that no more tokens
appear on that line. (There might not be any tokens at all, if the
|end_line_char| has |ignore| as its catcode.)

The value of |par_loc| is the |eqtb| address of `\.{\\par}'. This quantity is
needed because a blank line of input is supposed to be exactly equivalent to the
appearance of \.{\\par}; we must set |cur_cs:=par_loc| when detecting a blank
line.

@c
halfword par_loc;   /* location of `\.{\\par}' in |eqtb| */
halfword par_token; /* token representing `\.{\\par}' */

@ Parts |get_next| are executed more often than any other instructions of \TeX.
@^mastication@>@^inner loop@>

The global variable |force_eof| is normally |false|; it is set |true| by an
\.{\\endinput} command. |luacstrings| is the number of lua print statements
waiting to be input, it is changed by |luatokencall|.

@c
boolean force_eof; /* should the next \.{\\input} be aborted early? */
int luacstrings;   /* how many lua strings are waiting to be input? */

@ If the user has set the |pausing| parameter to some positive value, and if
nonstop mode has not been selected, each line of input is displayed on the
terminal and the transcript file, followed by `\.{=>}'. \TeX\ waits for a
response. If the response is simply |carriage_return|, the line is accepted as it
stands, otherwise the line typed is used instead of the line in the file.

@c
void firm_up_the_line(void)
{
    int k;                      /* an index into |buffer| */
    ilimit = last;
    if (pausing_par > 0) {
        if (interaction > nonstop_mode) {
            wake_up_terminal();
            print_ln();
            if (istart < ilimit) {
                for (k = istart; k <= ilimit - 1; k++)
                    print_char(buffer[k]);
            }
            first = ilimit;
            prompt_input("=>"); /* wait for user response */
            if (last > first) {
                for (k = first; k < +last - 1; k++)     /* move line down in buffer */
                    buffer[k + istart - first] = buffer[k];
                ilimit = istart + last - first;
            }
        }
    }
}

@ Before getting into |get_next|, let's consider the subroutine that is called
when an `\.{\\outer}' control sequence has been scanned or when the end of a file
has been reached. These two cases are distinguished by |cur_cs|, which is zero at
the end of a file.

@c
void check_outer_validity(void)
{
    halfword p;                 /* points to inserted token list */
    halfword q;                 /* auxiliary pointer */
    if (suppress_outer_error_par)
        return;
    if (scanner_status != normal) {
        deletions_allowed = false;
        /* Back up an outer control sequence so that it can be reread; */
        /* An outer control sequence that occurs in a \.{\\read} will not be reread,
           since the error recovery for \.{\\read} is not very powerful. */
        if (cur_cs != 0) {
            if ((istate == token_list) || (iname < 1) || (iname > 17)) {
                p = get_avail();
                token_info(p) = cs_token_flag + cur_cs;
                begin_token_list(p, backed_up); /* prepare to read the control sequence again */
            }
            cur_cmd = spacer_cmd;
            cur_chr = ' ';      /* replace it by a space */
        }
        if (scanner_status > skipping) {
            const char *errhlp[] = {
                "I suspect you have forgotten a `}', causing me",
                "to read past where you wanted me to stop.",
                "I'll try to recover; but if the error is serious,",
                "you'd better type `E' or `X' now and fix your file.",
                NULL
            };
            char errmsg[256];
            const char *startmsg;
            const char *scannermsg;
            /* Tell the user what has run away and try to recover */
            runaway();          /* print a definition, argument, or preamble */
            if (cur_cs == 0) {
                startmsg = "File ended";
            } else {
                cur_cs = 0;
                startmsg = "Forbidden control sequence found";
            }
            /* Print either `\.{definition}' or `\.{use}' or `\.{preamble}' or `\.{text}',
               and insert tokens that should lead to recovery; */
            /* The recovery procedure can't be fully understood without knowing more
               about the \TeX\ routines that should be aborted, but we can sketch the
               ideas here:  For a runaway definition we will insert a right brace; for a
               runaway preamble, we will insert a special \.{\\cr} token and a right
               brace; and for a runaway argument, we will set |long_state| to
               |outer_call| and insert \.{\\par}. */
            p = get_avail();
            switch (scanner_status) {
            case defining:
                scannermsg = "definition";
                token_info(p) = right_brace_token + '}';
                break;
            case matching:
                scannermsg = "use";
                token_info(p) = par_token;
                long_state = outer_call_cmd;
                break;
            case aligning:
                scannermsg = "preamble";
                token_info(p) = right_brace_token + '}';
                q = p;
                p = get_avail();
                token_link(p) = q;
                token_info(p) = cs_token_flag + frozen_cr;
                align_state = -1000000;
                break;
            case absorbing:
                scannermsg = "text";
                token_info(p) = right_brace_token + '}';
                break;
            default:           /* can't happen */
                scannermsg = "unknown";
                break;
            }                   /*there are no other cases */
            begin_token_list(p, inserted);
            snprintf(errmsg, 255, "%s while scanning %s of %s",
                     startmsg, scannermsg, cs_to_string(warning_index));
            tex_error(errmsg, errhlp);
        } else {
            char errmsg[256];
            const char *errhlp_no[] = {
                "The file ended while I was skipping conditional text.",
                "This kind of error happens when you say `\\if...' and forget",
                "the matching `\\fi'. I've inserted a `\\fi'; this might work.",
                NULL
            };
            const char *errhlp_cs[] = {
                "A forbidden control sequence occurred in skipped text.",
                "This kind of error happens when you say `\\if...' and forget",
                "the matching `\\fi'. I've inserted a `\\fi'; this might work.",
                NULL
            };
            const char **errhlp = (const char **) errhlp_no;
            char *ss;
            if (cur_cs != 0) {
                errhlp = errhlp_cs;
                cur_cs = 0;
            }
            ss = cmd_chr_to_string(if_test_cmd, cur_if);
            snprintf(errmsg, 255, "Incomplete %s; all text was ignored after line %d",
                 ss, (int) skip_line);
            free(ss);
            /* Incomplete \\if... */
            cur_tok = cs_token_flag + frozen_fi;
            /* back up one inserted token and call |error| */
            {
                OK_to_interrupt = false;
                back_input();
                token_type = inserted;
                OK_to_interrupt = true;
                tex_error(errmsg, errhlp);
            }
        }
        deletions_allowed = true;
    }
}

@ @c

#if 0

/*
    The other variant gives less clutter in tracing cache usage when profiling and for
    some files (like the manual) also a bit of a speedup.
*/

static boolean get_next_file(void)
{
  SWITCH:
    if (iloc <= ilimit) {
        /* current line not yet finished */
        do_buffer_to_unichar(cur_chr, iloc);

      RESWITCH:
        if (detokenized_line()) {
            cur_cmd = (cur_chr == ' ' ? 10 : 12);
        } else {
            do_get_cat_code(cur_cmd, cur_chr);
        }
        /*
            Change state if necessary, and |goto switch| if the current
            character should be ignored, or |goto reswitch| if the current
            character changes to another;

            The following 48-way switch accomplishes the scanning quickly, assuming
            that a decent C compiler has translated the code. Note that the numeric
            values for |mid_line|, |skip_blanks|, and |new_line| are spaced
            apart from each other by |max_char_code+1|, so we can add a character's
            command code to the state to get a single number that characterizes both.

            Remark [ls/hh]: checking performance indicated that this switch was the
            cause of many branch prediction errors but changing it to:

                c = istate + cur_cmd;
                if (c == (mid_line + letter_cmd) || c == (mid_line + other_char_cmd)) {
                    return true;
                } else if (c >= new_line) {
                    switch (c) {
                    }
                } else if (c >= skip_blanks) {
                    switch (c) {
                    }
                } else if (c >= mid_line) {
                    switch (c) {
                    }
                } else {
                    istate = mid_line;
                    return true;
                }

            gives as many prediction errors. So, we can indeed assume that the compiler
            does the right job, or that there is simply no other way.
        */

        switch (istate + cur_cmd) {
            case mid_line + ignore_cmd:
            case skip_blanks + ignore_cmd:
            case new_line + ignore_cmd:
            case skip_blanks + spacer_cmd:
            case new_line + spacer_cmd:
                /* Cases where character is ignored */
                goto SWITCH;
                break;
            case mid_line + escape_cmd:
            case new_line + escape_cmd:
            case skip_blanks + escape_cmd:
                /* Scan a control sequence ...; */
                istate = (unsigned char) scan_control_sequence();
                if (! suppress_outer_error_par && cur_cmd >= outer_call_cmd)
                    check_outer_validity();
                break;
            case mid_line + active_char_cmd:
            case new_line + active_char_cmd:
            case skip_blanks + active_char_cmd:
                /* Process an active-character  */
                cur_cs = active_to_cs(cur_chr, false);
                cur_cmd = eq_type(cur_cs);
                cur_chr = equiv(cur_cs);
                istate = mid_line;
                if (! suppress_outer_error_par && cur_cmd >= outer_call_cmd)
                    check_outer_validity();
                break;
            case mid_line + sup_mark_cmd:
            case new_line + sup_mark_cmd:
            case skip_blanks + sup_mark_cmd:
                /* If this |sup_mark| starts */
                if (process_sup_mark())
                    goto RESWITCH;
                else
                    istate = mid_line;
                break;
            case mid_line + invalid_char_cmd:
            case new_line + invalid_char_cmd:
            case skip_blanks + invalid_char_cmd:
                /* Decry the invalid character and |goto restart|; */
                invalid_character_error();
                return false; /* because state may be |token_list| now */
                break;
            case mid_line + spacer_cmd:
                /* Enter |skip_blanks| state, emit a space; */
                istate = skip_blanks;
                cur_chr = ' ';
                break;
            case mid_line + car_ret_cmd:
                /*
                    Finish line, emit a space. When a character of type |spacer| gets through, its
                    character code is changed to $\.{"\ "}=040$. This means that the ASCII codes
                    for tab and space, and for the space inserted at the end of a line, will be
                    treated alike when macro parameters are being matched. We do this since such
                    characters are indistinguishable on most computer terminal displays.
                 */
                iloc = ilimit + 1;
                cur_cmd = spacer_cmd;
                cur_chr = ' ';
                break;
            case skip_blanks + car_ret_cmd:
            case mid_line + comment_cmd:
            case new_line + comment_cmd:
            case skip_blanks + comment_cmd:
                /* Finish line, |goto switch|; */
                iloc = ilimit + 1;
                goto SWITCH;
                break;
            case new_line + car_ret_cmd:
                /* Finish line, emit a \.{\\par}; */
                iloc = ilimit + 1;
                cur_cs = par_loc;
                cur_cmd = eq_type(cur_cs);
                cur_chr = equiv(cur_cs);
                if (! suppress_outer_error_par && cur_cmd >= outer_call_cmd)
                    check_outer_validity();
                break;
            case skip_blanks + left_brace_cmd:
            case new_line + left_brace_cmd:
                istate = mid_line;
                /* fall through */
            case mid_line + left_brace_cmd:
                align_state++;
                break;
            case skip_blanks + right_brace_cmd:
            case new_line + right_brace_cmd:
                istate = mid_line;
                /* fall through */
            case mid_line + right_brace_cmd:
                align_state--;
                break;
            case mid_line + math_shift_cmd:
            case mid_line + tab_mark_cmd:
            case mid_line + mac_param_cmd:
            case mid_line + sub_mark_cmd:
            case mid_line + letter_cmd:
            case mid_line + other_char_cmd:
                break;
            /*
            case skip_blanks + math_shift:
            case skip_blanks + tab_mark:
            case skip_blanks + mac_param:
            case skip_blanks + sub_mark:
            case skip_blanks + letter:
            case skip_blanks + other_char:
            case new_line    + math_shift:
            case new_line    + tab_mark:
            case new_line    + mac_param:
            case new_line    + sub_mark:
            case new_line    + letter:
            case new_line    + other_char:
            */
            default:
                istate = mid_line;
                break;
        }
    } else {
        if (iname != 21)
            istate = new_line;
        /*
           Move to next line of file,
           or |goto restart| if there is no next line,
           or |return| if a \.{\\read} line has finished;
         */
        do {
            next_line_retval r = next_line();
            if (r == next_line_return) {
                return true;
            } else if (r == next_line_restart) {
                return false;
            }
        } while (0);
        check_interrupt();
        goto SWITCH;
    }
    return true;
}

#else

/* 10 times less Bim in callgrind */

/*
    escape_cmd left_brace_cmd right_brace_cmd math_shift_cmd
    tab_mark_cmd car_ret_cmd mac_param_cmd sup_mark_cmd
    sub_mark_cmd ignore_cmd spacer_cmd letter_cmd
    other_char_cmd active_char_cmd comment_cmd invalid_char_cmd
*/

static boolean get_next_file(void)
{
    int c = 0;
  SWITCH:
    if (iloc <= ilimit) {
        /* current line not yet finished */
        do_buffer_to_unichar(cur_chr, iloc);
      RESWITCH:
        if (detokenized_line()) {
            cur_cmd = (cur_chr == ' ' ? 10 : 12);
        } else {
            do_get_cat_code(cur_cmd, cur_chr);
        }
        /*
           Change state if necessary, and |goto switch| if the current
           character should be ignored, or |goto reswitch| if the current
           character changes to another;
        */
        c = istate + cur_cmd;
        if (c == (mid_line + letter_cmd) || c == (mid_line + other_char_cmd)) {
            return true;
        } else if (c >= new_line) {
            switch (c-new_line) {
                case escape_cmd:
                    istate = (unsigned char) scan_control_sequence();
                    if (! suppress_outer_error_par && cur_cmd >= outer_call_cmd)
                        check_outer_validity();
                    return true;
                case left_brace_cmd:
                    istate = mid_line;
                    align_state++;
                    return true;
                case right_brace_cmd:
                    istate = mid_line;
                    align_state--;
                    return true;
                case math_shift_cmd:
                    istate = mid_line;
                    return true;
                case tab_mark_cmd:
                    istate = mid_line;
                    return true;
                case car_ret_cmd:
                    /* Finish line, emit a \.{\\par}; */
                    iloc = ilimit + 1;
                    cur_cs = par_loc;
                    cur_cmd = eq_type(cur_cs);
                    cur_chr = equiv(cur_cs);
                    if (! suppress_outer_error_par && cur_cmd >= outer_call_cmd)
                        check_outer_validity();
                    return true;
                case mac_param_cmd:
                    istate = mid_line;
                    return true;
                case sup_mark_cmd:
                    if (process_sup_mark())
                        goto RESWITCH;
                    else
                        istate = mid_line;
                    return true;
                case sub_mark_cmd:
                    istate = mid_line;
                    return true;
                case ignore_cmd:
                    goto SWITCH;
                    return true;
                case spacer_cmd:
                    /* Cases where character is ignored */
                    goto SWITCH;
                case letter_cmd:
                    istate = mid_line;
                    return true;
                case other_char_cmd:
                    istate = mid_line;
                    return true;
                case active_char_cmd:
                    cur_cs = active_to_cs(cur_chr, false);
                    cur_cmd = eq_type(cur_cs);
                    cur_chr = equiv(cur_cs);
                    istate = mid_line;
                    if (! suppress_outer_error_par && cur_cmd >= outer_call_cmd)
                        check_outer_validity();
                    return true;
                case comment_cmd:
                    iloc = ilimit + 1;
                    goto SWITCH;
                case invalid_char_cmd:
                    invalid_character_error();
                    return false; /* because state may be |token_list| now */
                default:
                    istate = mid_line;
                    return true;
            }
        } else if (c >= skip_blanks) {
            switch (c-skip_blanks) {
                case escape_cmd:
                    /* Scan a control sequence ...; */
                    istate = (unsigned char) scan_control_sequence();
                    if (! suppress_outer_error_par && cur_cmd >= outer_call_cmd)
                        check_outer_validity();
                    return true;
                case left_brace_cmd:
                    istate = mid_line;
                    align_state++;
                    return true;
                case right_brace_cmd:
                    istate = mid_line;
                    align_state--;
                    return true;
                case math_shift_cmd:
                    istate = mid_line;
                    return true;
                case tab_mark_cmd:
                    istate = mid_line;
                    return true;
                case car_ret_cmd:
                    iloc = ilimit + 1;
                    goto SWITCH;
                case mac_param_cmd:
                    istate = mid_line;
                    return true;
                case sup_mark_cmd:
                    /* If this |sup_mark| starts */
                    if (process_sup_mark())
                        goto RESWITCH;
                    else
                        istate = mid_line;
                    return true;
                case sub_mark_cmd:
                    istate = mid_line;
                    return true;
                case ignore_cmd:
                    goto SWITCH;
                case spacer_cmd:
                    goto SWITCH;
                case letter_cmd:
                    istate = mid_line;
                    return true;
                case other_char_cmd:
                    istate = mid_line;
                    return true;
                case active_char_cmd:
                    cur_cs = active_to_cs(cur_chr, false);
                    cur_cmd = eq_type(cur_cs);
                    cur_chr = equiv(cur_cs);
                    istate = mid_line;
                    if (! suppress_outer_error_par && cur_cmd >= outer_call_cmd)
                        check_outer_validity();
                    return true;
                case comment_cmd:
                    /* Finish line, |goto switch|; */
                    iloc = ilimit + 1;
                    goto SWITCH;
                case invalid_char_cmd:
                    /* Decry the invalid character and |goto restart|; */
                    invalid_character_error();
                    return false; /* because state may be |token_list| now */
                default:
                    istate = mid_line;
                    return true;
            }
        } else if (c >= mid_line) {
            switch (c-mid_line) {
                case escape_cmd:
                    istate = (unsigned char) scan_control_sequence();
                    if (! suppress_outer_error_par && cur_cmd >= outer_call_cmd)
                        check_outer_validity();
                    return true;
                case left_brace_cmd:
                    align_state++;
                    return true;
                case right_brace_cmd:
                    align_state--;
                    return true;
                case math_shift_cmd:
                    return true;
                case tab_mark_cmd:
                    return true;
                case car_ret_cmd:
                    /*
                        Finish line, emit a space. When a character of type |spacer| gets through, its
                        character code is changed to $\.{"\ "}=040$. This means that the ASCII codes
                        for tab and space, and for the space inserted at the end of a line, will be
                        treated alike when macro parameters are being matched. We do this since such
                        characters are indistinguishable on most computer terminal displays.
                     */
                    iloc = ilimit + 1;
                    cur_cmd = spacer_cmd;
                    cur_chr = ' ';
                    return true;
                case mac_param_cmd:
                    return true;
                case sup_mark_cmd:
                    if (process_sup_mark())
                        goto RESWITCH;
                    else
                        istate = mid_line;
                    return true;
                case sub_mark_cmd:
                    return true;
                case ignore_cmd:
                    goto SWITCH;
                case spacer_cmd:
                    /* Enter |skip_blanks| state, emit a space; */
                    istate = skip_blanks;
                    cur_chr = ' ';
                    return true;
                case letter_cmd:
                    istate = mid_line;
                    return true;
                case other_char_cmd:
                    istate = mid_line;
                    return true;
                case active_char_cmd:
                    cur_cs = active_to_cs(cur_chr, false);
                    cur_cmd = eq_type(cur_cs);
                    cur_chr = equiv(cur_cs);
                    istate = mid_line;
                    if (! suppress_outer_error_par && cur_cmd >= outer_call_cmd)
                        check_outer_validity();
                    return true;
                case comment_cmd:
                    iloc = ilimit + 1;
                    goto SWITCH;
                case invalid_char_cmd:
                    invalid_character_error();
                    return false; /* because state may be |token_list| now */
                default:
                    istate = mid_line;
                    return true;
            }
        } else {
            istate = mid_line;
            return true;
        }
    } else {
        if (iname != 21) {
            istate = new_line;
        }
        /*
           Move to next line of file, or |goto restart| if there is no next line,
           or |return| if a \.{\\read} line has finished;
        */
        do {
            next_line_retval r = next_line();
            if (r == next_line_return) {
                return true;
            } else if (r == next_line_restart) {
                return false;
            }
        } while (0);
        check_interrupt();
        goto SWITCH;
    }
    return true;
}

#endif

@ Notice that a code like \.{\^\^8} becomes \.x if not followed by a hex digit.
We only support a limited set:

^^^^^^XXXXXX
^^^^XXXXXX
^^XX ^^<char>

@c

#define is_hex(a) ((a>='0'&&a<='9')||(a>='a'&&a<='f'))

#define add_nybble(c) \
    if (c<='9') { \
        cur_chr=(cur_chr<<4)+c-'0'; \
    } else { \
        cur_chr=(cur_chr<<4)+c-'a'+10; \
    }

#define set_nybble(c) \
    if (c<='9') { \
        cur_chr=c-'0'; \
    } else { \
        cur_chr=c-'a'+10; \
    }

#define one_hex_to_cur_chr(c1) \
    set_nybble(c1);

#define two_hex_to_cur_chr(c1,c2) \
    set_nybble(c1); \
    add_nybble(c2);

#define four_hex_to_cur_chr(c1,c2,c3,c4) \
    two_hex_to_cur_chr(c1,c2); \
    add_nybble(c3); \
    add_nybble(c4);

#define six_hex_to_cur_chr(c1,c2,c3,c4,c5,c6) \
    four_hex_to_cur_chr(c1,c2,c3,c4); \
    add_nybble(c5); \
    add_nybble(c6);

static boolean process_sup_mark(void)
{
    if (cur_chr == buffer[iloc]) {
        if (iloc < ilimit) {
            if ((cur_chr == buffer[iloc + 1]) && (cur_chr == buffer[iloc + 2])) {
                if ((cur_chr == buffer[iloc + 3]) && (cur_chr == buffer[iloc + 4])) {
                    /* ^^^^^^XXXXXX */
                    if ((iloc + 10) <= ilimit) {
                        int c1 = buffer[iloc +  5];
                        int c2 = buffer[iloc +  6];
                        int c3 = buffer[iloc +  7];
                        int c4 = buffer[iloc +  8];
                        int c5 = buffer[iloc +  9];
                        int c6 = buffer[iloc + 10];
                        if (is_hex(c1) && is_hex(c2) && is_hex(c3) &&
                            is_hex(c4) && is_hex(c5) && is_hex(c6)) {
                            iloc = iloc + 11;
                            six_hex_to_cur_chr(c1,c2,c3,c4,c5,c6);
                            return true;
                        } else {
                            tex_error("^^^^^^ needs six hex digits", NULL);
                        }
                    } else {
                        tex_error("^^^^^^ needs six hex digits, end of input", NULL);
                    }
                } else {
                    /* ^^^^XXXX */
                    if ((iloc + 6) <= ilimit) {
                        int c1 = buffer[iloc + 3];
                        int c2 = buffer[iloc + 4];
                        int c3 = buffer[iloc + 5];
                        int c4 = buffer[iloc + 6];
                        if (is_hex(c1) && is_hex(c2) && is_hex(c3) && is_hex(c4)) {
                            iloc = iloc + 7;
                            four_hex_to_cur_chr(c1,c2,c3,c4);
                            return true;
                        } else {
                            tex_error("^^^^ needs four hex digits", NULL);
                        }
                    } else {
                        tex_error("^^^^ needs four hex digits, end of input", NULL);
                    }
                }
            } else {
                /* ^^XX */
                if ((iloc + 2) <= ilimit) {
                    int c1 = buffer[iloc + 1];
                    int c2 = buffer[iloc + 2];
                    if (is_hex(c1) && is_hex(c2)) {
                        iloc = iloc + 3;
                        two_hex_to_cur_chr(c1,c2);
                        return true;
                    }
                }
                /* go on, no error, good old tex */
            }
        }
        /* the rest */
        {
            int c1 = buffer[iloc + 1];
            if (c1 < 0200) {
                iloc = iloc + 2;
                if (is_hex(c1) && (iloc <= ilimit)) {
                    int c2 = buffer[iloc];
                    if (is_hex(c2)) {
                        incr(iloc);
                        two_hex_to_cur_chr(c1,c2);
                        return true;
                    }
                }
                cur_chr = (c1 < 0100 ? c1 + 0100 : c1 - 0100);
                return true;
            }
        }
    }
    return false;
}

@ Control sequence names are scanned only when they appear in some line of a
file; once they have been scanned the first time, their |eqtb| location serves as
a unique identification, so \TeX\ doesn't need to refer to the original name any
more except when it prints the equivalent in symbolic form.

The program that scans a control sequence has been written carefully in order to
avoid the blowups that might otherwise occur if a malicious user tried something
like `\.{\\catcode\'15=0}'. The algorithm might look at |buffer[ilimit+1]|, but
it never looks at |buffer[ilimit+2]|.

If expanded characters like `\.{\^\^A}' or `\.{\^\^df}' appear in or just
following a control sequence name, they are converted to single characters in the
buffer and the process is repeated, slowly but surely.

@c
static boolean check_expanded_code(int *kk);    /* below */

static int scan_control_sequence(void)
{
    int retval = mid_line;
    if (iloc > ilimit) {
        cur_cs = null_cs;       /* |state| is irrelevant in this case */
    } else {
        register int cat;       /* |cat_code(cur_chr)|, usually */
        while (1) {
            int k = iloc;
            do_buffer_to_unichar(cur_chr, k);
            do_get_cat_code(cat, cur_chr);
            if (cat != letter_cmd || k > ilimit) {
                retval = (cat == spacer_cmd ? skip_blanks : mid_line);
                if (cat == sup_mark_cmd && check_expanded_code(&k))     /* If an expanded...; */
                    continue;
            } else {
                retval = skip_blanks;
                do {
                    do_buffer_to_unichar(cur_chr, k);
                    do_get_cat_code(cat, cur_chr);
                } while (cat == letter_cmd && k <= ilimit);

                if (cat == sup_mark_cmd && check_expanded_code(&k))     /* If an expanded...; */
                    continue;
                if (cat != letter_cmd) {
                    /* backtrack one character which can be utf */
                    /*
                    decr(k);
                    if (cur_chr > 0xFFFF)
                        decr(k);
                    if (cur_chr > 0x7FF)
                        decr(k);
                    if (cur_chr > 0x7F)
                        decr(k);
                    */
                    if (cur_chr <= 0x7F) {
                        k -= 1; /* in most cases */
                    } else if (cur_chr > 0xFFFF) {
                        k -= 4;
                    } else if (cur_chr > 0x7FF) {
                        k -= 3;
                    } else /* if (cur_chr > 0x7F) */ {
                        k -= 2;
                    }
                    /* now |k| points to first nonletter */
                }
            }
            cur_cs = id_lookup(iloc, k - iloc);
            iloc = k;
            break;
        }
    }
    cur_cmd = eq_type(cur_cs);
    cur_chr = equiv(cur_cs);
    return retval;
}

@ Whenever we reach the following piece of code, we will have
|cur_chr=buffer[k-1]| and |k<=ilimit+1| and
|cat=get_cat_code(cat_code_table,cur_chr)|. If an expanded code like \.{\^\^A} or
\.{\^\^df} appears in |buffer[(k-1)..(k+1)]| or |buffer[(k-1)..(k+2)]|, we will
store the corresponding code in |buffer[k-1]| and shift the rest of the buffer
left two or three places.

@c
static boolean check_expanded_code(int *kk)
{
    int l;
    int k = *kk;
    int d = 1;
    if (buffer[k] == cur_chr && k < ilimit) {
        if ((cur_chr == buffer[k + 1]) && (cur_chr == buffer[k + 2])) {
            if ((cur_chr == buffer[k + 3]) && (cur_chr == buffer[k + 4])) {
                if ((k + 10) <= ilimit) {
                    int c1 = buffer[k + 6 - 1];
                    int c2 = buffer[k + 6];
                    int c3 = buffer[k + 6 + 1];
                    int c4 = buffer[k + 6 + 2];
                    int c5 = buffer[k + 6 + 3];
                    int c6 = buffer[k + 6 + 4];
                    if (is_hex(c1) && is_hex(c2) && is_hex(c3) && is_hex(c4) && is_hex(c5) && is_hex(c6)) {
                        d = 6;
                        six_hex_to_cur_chr(c1,c2,c3,c4,c5,c6);
                    } else {
                        tex_error("^^^^^^ needs six hex digits", NULL);
                    }
                } else {
                    tex_error("^^^^^^ needs six hex digits, end of input", NULL);
                }
            } else {
                if ((k + 6) <= ilimit) {
                    int c1 = buffer[k + 4 - 1];
                    int c2 = buffer[k + 4];
                    int c3 = buffer[k + 4 + 1];
                    int c4 = buffer[k + 4 + 2];
                    if (is_hex(c1) && is_hex(c2) && is_hex(c3) && is_hex(c4)) {
                        d = 4;
                        four_hex_to_cur_chr(c1,c2,c3,c4);
                    } else {
                        tex_error("^^^^ needs four hex digits", NULL);
                    }
                } else {
                    tex_error("^^^^ needs four hex digits, end of input", NULL);
                }
            }
        } else {
            int c1 = buffer[k + 1];
            if (c1 < 0200) {
                d = 1;
                if (is_hex(c1) && (k + 2) <= ilimit) {
                    int c2 = buffer[k + 2];
                    if (is_hex(c2)) {
                        d = 2;
                        two_hex_to_cur_chr(c1,c2);
                    } else {
                        cur_chr = (c1 < 0100 ? c1 + 0100 : c1 - 0100);
                    }
                } else {
                    cur_chr = (c1 < 0100 ? c1 + 0100 : c1 - 0100);
                }
            }
        }
        if (d > 2)
            d = 2 * d - 1;
        else
            d++;
        if (cur_chr <= 0x7F) {
            buffer[k - 1] = (packed_ASCII_code) cur_chr;
        } else if (cur_chr <= 0x7FF) {
            buffer[k - 1] = (packed_ASCII_code) (0xC0 + cur_chr / 0x40);
            k++;
            d--;
            buffer[k - 1] = (packed_ASCII_code) (0x80 + cur_chr % 0x40);
        } else if (cur_chr <= 0xFFFF) {
            buffer[k - 1] = (packed_ASCII_code) (0xE0 + cur_chr / 0x1000);
            k++;
            d--;
            buffer[k - 1] = (packed_ASCII_code) (0x80 + (cur_chr % 0x1000) / 0x40);
            k++;
            d--;
            buffer[k - 1] = (packed_ASCII_code) (0x80 + (cur_chr % 0x1000) % 0x40);
        } else {
            buffer[k - 1] = (packed_ASCII_code) (0xF0 + cur_chr / 0x40000);
            k++;
            d--;
            buffer[k - 1] = (packed_ASCII_code) (0x80 + (cur_chr % 0x40000) / 0x1000);
            k++;
            d--;
            buffer[k - 1] = (packed_ASCII_code) (0x80 + ((cur_chr % 0x40000) % 0x1000) / 0x40);
            k++;
            d--;
            buffer[k - 1] = (packed_ASCII_code) (0x80 + ((cur_chr % 0x40000) % 0x1000) % 0x40);
        }
        l = k;
        ilimit = ilimit - d;
        while (l <= ilimit) {
            buffer[l] = buffer[l + d];
            l++;
        }
        *kk = k;
        return true;
    }
    return false;
}

@ All of the easy branches of |get_next| have now been taken care of. There is
one more branch.

@c static next_line_retval next_line(void)
{
    boolean inhibit_eol = false; /* a way to end a pseudo file without trailing space */
    if (iname > 17) {
        /* Read next line of file into |buffer|, or |goto restart| if the file has ended */
        incr(line);
        first = istart;
        if (!force_eof) {
            if (iname <= 20) {
                if (pseudo_input()) {   /* not end of file */
                    firm_up_the_line(); /* this sets |ilimit| */
                    line_catcode_table = DEFAULT_CAT_TABLE;
                    if ((iname == 19) && (pseudo_lines(pseudo_files) == null))
                        inhibit_eol = true;
                } else if ((every_eof_par != null) && !eof_seen[iindex]) {
                    ilimit = first - 1;
                    eof_seen[iindex] = true; /* fake one empty line */
                    if (iname != 19)
                        begin_token_list(every_eof_par, every_eof_text);
                    return next_line_restart;
                } else {
                    force_eof = true;
                }
            } else {
                if (iname == 21) {
                    if (luacstring_input()) { /* not end of strings  */
                        firm_up_the_line();
                        line_catcode_table = (short) luacstring_cattable();
                        line_partial = (signed char) luacstring_partial();
                        if (luacstring_final_line() || line_partial
                            || line_catcode_table == NO_CAT_TABLE)
                            inhibit_eol = true;
                        if (!line_partial)
                            istate = new_line;
                    } else {
                        force_eof = true;
                    }
                } else {
                    if (lua_input_ln(cur_file, 0, true)) { /* not end of file */
                        firm_up_the_line(); /* this sets |ilimit| */
                        line_catcode_table = DEFAULT_CAT_TABLE;
                    } else if ((every_eof_par != null) && (!eof_seen[iindex])) {
                        ilimit = first - 1;
                        eof_seen[iindex] = true; /* fake one empty line */
                        begin_token_list(every_eof_par, every_eof_text);
                        return next_line_restart;
                    } else {
                        force_eof = true;
                    }
                }
            }
        }
        if (force_eof) {
            if (tracing_nesting_par > 0)
                if ((grp_stack[in_open] != cur_boundary) || (if_stack[in_open] != cond_ptr))
                    if (!((iname == 19) || (iname == 21))) {
                        /* give warning for some unfinished groups and/or conditionals */
                        file_warning();
                    }
            if ((iname > 21) || (iname == 20)) {
                report_stop_file(filetype_tex);
                decr(open_parens);
            }
            force_eof = false;
            /* lua input or \.{\\scantextokens} */
            if (iname == 21 || iname == 19) {
                end_file_reading();
            } else {
                end_file_reading();
                if (! suppress_outer_error_par)
                    check_outer_validity();
            }
            return next_line_restart;
        }
        if (inhibit_eol || end_line_char_inactive)
            ilimit--;
        else
            buffer[ilimit] = (packed_ASCII_code) end_line_char_par;
        first = ilimit + 1;
        iloc = istart; /* ready to read */
    } else {
        if (!terminal_input) {
            /* \.{\\read} line has ended */
            cur_cmd = 0;
            cur_chr = 0;
            return next_line_return;    /* OUTER */
        }
        if (input_ptr > 0) {
            /* text was inserted during error recovery */
            end_file_reading();
            return next_line_restart; /* resume previous level */
        }
        if (selector < log_only)
            open_log_file();
        if (interaction > nonstop_mode) {
            if (end_line_char_inactive)
                ilimit++;
            if (ilimit == istart) {
                /* previous line was empty */
                tprint_nl("(Please type a command or say `\\end')");
            }
            print_ln();
            first = istart;
            prompt_input("*"); /* input on-line into |buffer| */
            ilimit = last;
            if (end_line_char_inactive)
                ilimit--;
            else
                buffer[ilimit] = (packed_ASCII_code) end_line_char_par;
            first = ilimit + 1;
            iloc = istart;
        } else {
            /*
                Nonstop mode, which is intended for overnight batch processing,
                never waits for on-line input.
            */
            fatal_error("*** (job aborted, no legal \\end found)");
        }
    }
    return next_line_ok;
}

@ Let's consider now what happens when |get_next| is looking at a token list.

@c
static boolean get_next_tokenlist(void)
{
    register halfword t = token_info(iloc);
    iloc = token_link(iloc); /* move to next */
    if (t >= cs_token_flag) {
        /* a control sequence token */
        cur_cs = t - cs_token_flag;
        cur_cmd = eq_type(cur_cs);
        if (cur_cmd >= outer_call_cmd) {
            if (cur_cmd == dont_expand_cmd) {
                /*
                    Get the next token, suppressing expansion. The present point in the program
                    is reached only when the |expand| routine has inserted a special marker into
                    the input. In this special case, |token_info(iloc)| is known to be a control
                    sequence token, and |token_link(iloc)=null|.
                */
                cur_cs = token_info(iloc) - cs_token_flag;
                iloc = null;
                cur_cmd = eq_type(cur_cs);
                if (cur_cmd > max_command_cmd) {
                    cur_cmd = relax_cmd;
                    cur_chr = no_expand_flag;
                    return true;
                }
            } else if (! suppress_outer_error_par) {
                check_outer_validity();
            }
        }
        cur_chr = equiv(cur_cs);
    } else {
        cur_cmd = token_cmd(t);
        cur_chr = token_chr(t);
        switch (cur_cmd) {
            case left_brace_cmd:
                align_state++;
                break;
            case right_brace_cmd:
                align_state--;
                break;
            case out_param_cmd:
                /* Insert macro parameter and |goto restart|; */
                begin_token_list(param_stack[param_start + cur_chr - 1], parameter);
                return false;
                break;
        }
    }
    return true;
}

@ Now we're ready to take the plunge into |get_next| itself. Parts of this
routine are executed more often than any other instructions of \TeX.
@^mastication@>@^inner loop@>

@ sets |cur_cmd|, |cur_chr|, |cur_cs| to next token

@c
void get_next(void)
{
  RESTART:
    cur_cs = 0;
    if (istate != token_list) {
        /* Input from external file, |goto restart| if no input found */
        if (!get_next_file())
            goto RESTART;
    } else {
        if (iloc == null) {
            end_token_list();
            goto RESTART;       /* list exhausted, resume previous level */
        } else if (!get_next_tokenlist()) {
            goto RESTART;       /* parameter needs to be expanded */
        }
    }
    /* If an alignment entry has just ended, take appropriate action */
    if ((cur_cmd == tab_mark_cmd || cur_cmd == car_ret_cmd) && align_state == 0) {
        insert_vj_template();
        goto RESTART;
    }
}

@ Since |get_next| is used so frequently in \TeX, it is convenient to define
three related procedures that do a little more:

\yskip\hang|get_token| not only sets |cur_cmd| and |cur_chr|, it also sets
|cur_tok|, a packed halfword version of the current token.

\yskip\hang|get_x_token|, meaning ``get an expanded token,'' is like |get_token|,
but if the current token turns out to be a user-defined control sequence (i.e., a
macro call), or a conditional, or something like \.{\\topmark} or
\.{\\expandafter} or \.{\\csname}, it is eliminated from the input by beginning
the expansion of the macro or the evaluation of the conditional.

\yskip\hang|x_token| is like |get_x_token| except that it assumes that |get_next|
has already been called.

\yskip\noindent In fact, these three procedures account for almost every use of
|get_next|.

No new control sequences will be defined except during a call of |get_token|, or
when \.{\\csname} compresses a token list, because |no_new_control_sequence| is
always |true| at other times.

@ sets |cur_cmd|, |cur_chr|, |cur_tok|

@c
void get_token(void)
{
    no_new_control_sequence = false;
    get_next();
    no_new_control_sequence = true;
    if (cur_cs == 0)
        cur_tok = token_val(cur_cmd, cur_chr);
    else
        cur_tok = cs_token_flag + cur_cs;
}

@ changes the string |s| to a token list

@c
halfword string_to_toks(const char *ss)
{
    halfword p; /* tail of the token list */
    halfword q; /* new node being added to the token list via |store_new_token| */
    halfword t; /* token being appended */
    const char *s = ss;
    const char *se = ss + strlen(s);
    p = temp_token_head;
    set_token_link(p, null);
    while (s < se) {
        t = (halfword) str2uni((const unsigned char *) s);
        s += utf8_size(t);
        if (t == ' ')
            t = space_token;
        else
            t = other_token + t;
        fast_store_new_token(t);
    }
    return token_link(temp_token_head);
}

@ The token lists for macros and for other things like \.{\\mark} and
\.{\\output} and \.{\\write} are produced by a procedure called |scan_toks|.

Before we get into the details of |scan_toks|, let's consider a much simpler
task, that of converting the current string into a token list. The |str_toks|
function does this; it classifies spaces as type |spacer| and everything else as
type |other_char|.

The token list created by |str_toks| begins at |link(temp_token_head)| and ends
at the value |p| that is returned. (If |p=temp_token_head|, the list is empty.)

|lua_str_toks| is almost identical, but it also escapes the three symbols that
|lua| considers special while scanning a literal string

@ changes the string |str_pool[b..pool_ptr]| to a token list

@c
halfword lua_str_toks(lstring b)
{
    halfword p;       /* tail of the token list */
    halfword q;       /* new node being added to the token list via |store_new_token| */
    halfword t;       /* token being appended */
    unsigned char *k; /* index into string */
    p = temp_token_head;
    set_token_link(p, null);
    k = (unsigned char *) b.s;
    while (k < (unsigned char *) b.s + b.l) {
        t = pool_to_unichar(k);
        k += utf8_size(t);
        if (t == ' ') {
            t = space_token;
        } else {
            if ((t == '\\') || (t == '"') || (t == '\'') || (t == 10) || (t == 13))
                fast_store_new_token(other_token + '\\');
            if (t == 10)
                t = 'n';
            if (t == 13)
                t = 'r';
            t = other_token + t;
        }
        fast_store_new_token(t);
    }
    return p;
}

@ Incidentally, the main reason for wanting |str_toks| is the function
|the_toks|, which has similar input/output characteristics.

@ changes the string |str_pool[b..pool_ptr]| to a token list

@c
halfword str_toks(lstring s)
{
    halfword p;           /* tail of the token list */
    halfword q;           /* new node being added to the token list via |store_new_token| */
    halfword t;           /* token being appended */
    unsigned char *k, *l; /* index into string */
    p = temp_token_head;
    set_token_link(p, null);
    k = s.s;
    l = k + s.l;
    while (k < l) {
        t = pool_to_unichar(k);
        k += utf8_size(t);
        if (t == ' ')
            t = space_token;
        else
            t = other_token + t;
        fast_store_new_token(t);
    }
    return p;
}

/*
    hh: most of the converter is similar to the one i made for macro so at some point i
    can make a helper; also todo: there is no need to go through the pool

*/

halfword str_scan_toks(int ct, lstring s)
{                         /* changes the string |str_pool[b..pool_ptr]| to a token list */
    halfword p;           /* tail of the token list */
    halfword q;           /* new node being added to the token list via |store_new_token| */
    halfword t;           /* token being appended */
    unsigned char *k, *l; /* index into string */
    int cc;
    p = temp_token_head;
    set_token_link(p, null);
    k = s.s;
    l = k + s.l;
    while (k < l) {
        t = pool_to_unichar(k);
        k += utf8_size(t);
        cc = get_cat_code(ct,t);
            if (cc == 0) {
                /* we have a potential control sequence so we check for it */
                int _lname = 0 ;
                int _s = 0 ;
                int _c = 0 ;
                halfword _cs = null ;
                unsigned char *_name  = k ;
                while (k < l) {
                    t = (halfword) str2uni((const unsigned char *) k);
                    _s = utf8_size(t);
                    _c = get_cat_code(ct,t);
                    if (_c == 11) {
                        k += _s ;
                        _lname = _lname + _s ;
                    } else if (_c == 10) {
                        /* we ignore a trailing space like normal scanning does */
                        k += _s ;
                        break ;
                    } else {
                        break ;
                    }
                }
                if (_s > 0) {
                    /* we have a potential \cs */
                    _cs = string_lookup((const char *) _name, _lname);
                    if (_cs == undefined_control_sequence) {
                        /* let's play safe and backtrack */
                        t = cc * (1<<21) + t ;
                        k = _name ;
                    } else {
                        t = cs_token_flag + _cs;
                    }
                } else {
                    /* just a character with some meaning, so \unknown becomes effectively */
                    /* \\unknown assuming that \\ has some useful meaning of course        */
                    t = cc * (1<<21) + t ;
                    k = _name ;
                }

            } else {
                /* whatever token, so for instance $x^2$ just works given a tex */
                /* catcode regime */
                t = cc * (1<<21) + t ;
            }
            fast_store_new_token(t);

    }
    return p;
}

@ Here's part of the |expand| subroutine that we are now ready to complete:

@c
void ins_the_toks(void)
{
    (void) the_toks();
    ins_list(token_link(temp_token_head));
}

#define set_toks_register(n,t,g) { \
    int a = (g>0) ? 4 : 0; \
    halfword ref = get_avail();  \
    set_token_ref_count(ref, 0); \
    set_token_link(ref, token_link(t)); \
    define(n + toks_base, call_cmd, ref); \
}

void combine_the_toks(int how)
{
    halfword nt;
    get_x_token();
    /* target */
    if (cur_cmd == assign_toks_cmd) {
        nt = equiv(cur_cs) - toks_base;
        /* check range */
    } else {
        back_input();
        scan_int();
        nt = cur_val;
    }
    /* source */
    do {
        get_x_token();
    } while (cur_cmd == spacer_cmd);
    if (cur_cmd == left_brace_cmd) {
        halfword x, source;
        back_input();
        x = scan_toks(false,how > 1); /* expanded or not */
        source = def_ref;
        /* action */
        if (source != null) {
            halfword target = toks(nt);
            if (target == null) {
                set_toks_register(nt,source,0);
            } else {
                halfword s = token_link(source);
                if (s != null) {
                    halfword t = token_link(target);
                    if (t == null) {
                        /* can this happen ? */
                        set_token_link(target, s);
                    } else if (odd(how)) {
                        /* prepend */
                        if (cur_level != eq_level_field(eqtb[toks_base+nt])) {
                            halfword p = temp_token_head;
                            halfword q;
                            set_token_link(p, s); /* s = head, x = tail */
                            p = x;
                            while (t != null) {
                                fast_store_new_token(token_info(t));
                                t = token_link(t);
                            }
                            set_toks_register(nt,temp_token_head,0);
                        } else {
                            set_token_link(x,t);
                            set_token_link(target,s);
                        }
                    } else {
                        /* append */
                        if (cur_level != eq_level_field(eqtb[toks_base+nt])) {
                            halfword p = temp_token_head;
                            halfword q;
                            set_token_link(p, null);
                            while (t != null) {
                                fast_store_new_token(token_info(t));
                                t = token_link(t);
                            }
                            set_token_link(p,s);
                            set_toks_register(nt,temp_token_head,0);
                        } else {
                            while (token_link(t) != null) {
                                t = token_link(t);
                            }
                            set_token_link(t,s);
                        }
                    }
                }
            }
        }
    } else {
        halfword source, ns;
        if (cur_cmd == assign_toks_cmd) {
            ns = equiv(cur_cs) - toks_base;
            /* check range */
        } else {
            back_input();
            scan_int();
            ns = cur_val;
        }
        /* action */
        source = toks(ns);
        if (source != null) {
            halfword target = toks(nt);
            if (target == null) {
                equiv(toks_base+nt) = source;
                equiv(toks_base+ns) = null;
            } else {
                halfword s = token_link(source);
                if (s != null) {
                    halfword t = token_link(target);
                    if (t == null) {
                        set_token_link(target, s);
                    } else if (odd(how)) {
                        /* prepend */
                        halfword x = s;
                        while (token_link(x) != null) {
                            x = token_link(x);
                        }
                        set_token_link(x,t);
                        set_token_link(target,s);
                    } else {
                        /* append */
                        while (token_link(t) != null) {
                            t = token_link(t);
                        }
                        set_token_link(t,s);
                    }
                     equiv(toks_base+ns) = null;
                }
            }
        }
    }
}

@ This routine, used in the next one, prints the job name, possibly modified by
the |process_jobname| callback.

@c
static void print_job_name(void)
{
   if (job_name) {
      char *s, *ss; /* C strings for jobname before and after processing */
      int callback_id, lua_retval;
      s = (char*)str_string(job_name);
      callback_id = callback_defined(process_jobname_callback);
      if (callback_id > 0) {
        lua_retval = run_callback(callback_id, "S->S", s, &ss);
        if ((lua_retval == true) && (ss != NULL))
            s = ss;
      }
      tprint(s);
   } else {
      print(job_name);
   }
}

@ Here is a routine that print the result of a convert command, using the
argument |i|. It returns |false | if it does not know to print the code |c|. The
function exists because lua code and tex code can both call it to convert
something.

@ Parse optional lua state integer, or an instance name to be stored in |sn| and
get the next non-blank non-relax non-call token.

@c

int scan_lua_state(void)
{
    int sn = 0;
    do {
        get_x_token();
    } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));
    back_input();
    if (cur_cmd != left_brace_cmd) {
        if (scan_keyword("name")) {
            (void) scan_toks(false, true);
            sn = def_ref;
        } else {
            scan_register_num();
            if (get_lua_name(cur_val))
                sn = (cur_val - 65536);
        }
    }
    return sn;
}

@ The procedure |conv_toks| uses |str_toks| to insert the token list for
|convert| functions into the scanner; `\.{\\outer}' control sequences are allowed
to follow `\.{\\string}' and `\.{\\meaning}'.

The extra temp string |u| is needed because |pdf_scan_ext_toks| incorporates any
pending string in its output. In order to save such a pending string, we have to
create a temporary string that is destroyed immediately after.

@c
#define push_selector { \
    old_setting = selector; \
    selector = new_string; \
}

#define pop_selector { \
    selector = old_setting; \
}

static int do_variable_dvi(halfword c)
{
    return 0;
}

#define do_variable_backend_int(i) \
    cur_cmd = assign_int_cmd; \
    cur_val = backend_int_base + i; \
    cur_tok = token_val(cur_cmd, cur_val); \
    back_input();

#define do_variable_backend_dimen(i) \
    cur_cmd = assign_dimen_cmd; \
    cur_val = backend_dimen_base + i; \
    cur_tok = token_val(cur_cmd, cur_val); \
    back_input();

#define do_variable_backend_toks(i) \
    cur_cmd = assign_toks_cmd; \
    cur_val = backend_toks_base + i ; \
    cur_tok = token_val(cur_cmd, cur_val); \
    back_input();

static int do_variable_pdf(halfword c)
{
         if (scan_keyword("compresslevel"))        { do_variable_backend_int(c_pdf_compress_level); }
    else if (scan_keyword("decimaldigits"))        { do_variable_backend_int(c_pdf_decimal_digits); }
    else if (scan_keyword("imageresolution"))      { do_variable_backend_int(c_pdf_image_resolution); }
    else if (scan_keyword("pkresolution"))         { do_variable_backend_int(c_pdf_pk_resolution); }
    else if (scan_keyword("uniqueresname"))        { do_variable_backend_int(c_pdf_unique_resname); }
    else if (scan_keyword("minorversion"))         { do_variable_backend_int(c_pdf_minor_version); }
    else if (scan_keyword("pagebox"))              { do_variable_backend_int(c_pdf_pagebox); }
    else if (scan_keyword("inclusionerrorlevel"))  { do_variable_backend_int(c_pdf_inclusion_errorlevel); }
    else if (scan_keyword("ignoreunknownimages"))  { do_variable_backend_int(c_pdf_ignore_unknown_images); }
    else if (scan_keyword("gamma"))                { do_variable_backend_int(c_pdf_gamma); }
    else if (scan_keyword("imageapplygamma"))      { do_variable_backend_int(c_pdf_image_apply_gamma); }
    else if (scan_keyword("imagegamma"))           { do_variable_backend_int(c_pdf_image_gamma); }
    else if (scan_keyword("imagehicolor"))         { do_variable_backend_int(c_pdf_image_hicolor); }
    else if (scan_keyword("imageaddfilename"))     { do_variable_backend_int(c_pdf_image_addfilename); }
    else if (scan_keyword("objcompresslevel"))     { do_variable_backend_int(c_pdf_obj_compress_level); }
    else if (scan_keyword("inclusioncopyfonts"))   { do_variable_backend_int(c_pdf_inclusion_copy_font); }
    else if (scan_keyword("gentounicode"))         { do_variable_backend_int(c_pdf_gen_tounicode); }
    else if (scan_keyword("pkfixeddpi"))           { do_variable_backend_int(c_pdf_pk_fixed_dpi); }
    else if (scan_keyword("suppressoptionalinfo")) { do_variable_backend_int(c_pdf_suppress_optional_info); }

    else if (scan_keyword("horigin"))              { do_variable_backend_dimen(d_pdf_h_origin); }
    else if (scan_keyword("vorigin"))              { do_variable_backend_dimen(d_pdf_v_origin); }
    else if (scan_keyword("threadmargin"))         { do_variable_backend_dimen(d_pdf_thread_margin); }
    else if (scan_keyword("destmargin"))           { do_variable_backend_dimen(d_pdf_dest_margin); }
    else if (scan_keyword("linkmargin"))           { do_variable_backend_dimen(d_pdf_link_margin); }
    else if (scan_keyword("xformmargin"))          { do_variable_backend_dimen(d_pdf_xform_margin); }

    else if (scan_keyword("pageattr"))             { do_variable_backend_toks(t_pdf_page_attr); }
    else if (scan_keyword("pageresources"))        { do_variable_backend_toks(t_pdf_page_resources); }
    else if (scan_keyword("pagesattr"))            { do_variable_backend_toks(t_pdf_pages_attr); }
    else if (scan_keyword("xformattr"))            { do_variable_backend_toks(t_pdf_xform_attr); }
    else if (scan_keyword("xformresources"))       { do_variable_backend_toks(t_pdf_xform_resources); }
    else if (scan_keyword("pkmode"))               { do_variable_backend_toks(t_pdf_pk_mode); }
    else if (scan_keyword("trailerid"))            { do_variable_backend_toks(t_pdf_trailer_id); }

    else
        return 0;
    return 1;
}

static int do_feedback_dvi(halfword c)
{
    return 0;
}

/* codes not really needed but cleaner when testing */

#define pdftex_version  140 /* these values will not change any more */
#define pdftex_revision "0" /* these values will not change any more */

static int do_feedback_pdf(halfword c)
{
    int old_setting;            /* holds |selector| setting */
    int save_scanner_status;    /* |scanner_status| upon entry */
    halfword save_def_ref;      /* |def_ref| upon entry, important if inside `\.{\\message}' */
    halfword save_warning_index;
#if defined(MIKTEX)
#  define bool temp_boolean
#endif
    boolean bool;               /* temp boolean */
    str_number s;               /* first temp string */
    int ff;                     /* for use with |set_ff| */
    str_number u = 0;           /* third temp string, will become non-nil if a string is already being built */
    char *str;                  /* color stack init str */

    if (scan_keyword("lastlink")) {
        push_selector;
        print_int(pdf_last_link);
        pop_selector;
    } else if (scan_keyword("retval")) {
        push_selector;
        print_int(pdf_retval);
        pop_selector;
    } else if (scan_keyword("lastobj")) {
        push_selector;
        print_int(pdf_last_obj);
        pop_selector;
    } else if (scan_keyword("lastannot")) {
        push_selector;
        print_int(pdf_last_annot);
        pop_selector;
    } else if (scan_keyword("xformname")) {
        scan_int();
        check_obj_type(static_pdf, obj_type_xform, cur_val);
        push_selector;
        print_int(obj_info(static_pdf, cur_val));
        pop_selector;
    } else if (scan_keyword("creationdate")) {
        ins_list(string_to_toks(getcreationdate(static_pdf)));
        /* no further action */
        return 2;
    } else if (scan_keyword("fontname")) {
        scan_font_ident();
        if (cur_val == null_font)
            normal_error("pdf backend", "invalid font identifier when asking 'fontname'");
        pdf_check_vf(cur_val);
        if (!font_used(cur_val))
            pdf_init_font(static_pdf, cur_val);
        push_selector;
        set_ff(cur_val);
        print_int(obj_info(static_pdf, pdf_font_num(ff)));
        pop_selector;
    } else if (scan_keyword("fontobjnum")) {
        scan_font_ident();
        if (cur_val == null_font)
            normal_error("pdf backend", "invalid font identifier when asking 'objnum'");
        pdf_check_vf(cur_val);
        if (!font_used(cur_val))
            pdf_init_font(static_pdf, cur_val);
        push_selector;
        set_ff(cur_val);
        print_int(pdf_font_num(ff));
        pop_selector;
    } else if (scan_keyword("fontsize")) {
        scan_font_ident();
        if (cur_val == null_font)
            normal_error("pdf backend", "invalid font identifier when asking 'fontsize'");
        push_selector;
        print_scaled(font_size(cur_val));
        tprint("pt");
        pop_selector;
    } else if (scan_keyword("pageref")) {
        scan_int();
        if (cur_val <= 0)
            normal_error("pdf backend", "invalid page number when asking 'pageref'");
        push_selector;
        print_int(pdf_get_obj(static_pdf, obj_type_page, cur_val, false));
        pop_selector;
    } else if (scan_keyword("colorstackinit")) {
        bool = scan_keyword("page");
        if (scan_keyword("direct"))
            cur_val = direct_always;
        else if (scan_keyword("page"))
            cur_val = direct_page;
        else
            cur_val = set_origin;
        save_scanner_status = scanner_status;
        save_warning_index = warning_index;
        save_def_ref = def_ref;
        u = save_cur_string();
        scan_toks(false, true);
        s = tokens_to_string(def_ref);
        delete_token_ref(def_ref);
        def_ref = save_def_ref;
        warning_index = save_warning_index;
        scanner_status = save_scanner_status;
        str = makecstring(s);
        cur_val = newcolorstack(str, cur_val, bool);
        free(str);
        flush_str(s);
        cur_val_level = int_val_level;
        if (cur_val < 0) {
            print_err("Too many color stacks");
            help2("The number of color stacks is limited to 32768.",
                  "I'll use the default color stack 0 here.");
            error();
            cur_val = 0;
            restore_cur_string(u);
        }
        push_selector;
        print_int(cur_val);
        pop_selector;
    } else if (scan_keyword("version")) {
        push_selector;
        print_int(pdftex_version);
        pop_selector;
    } else if (scan_keyword("revision")) {
        ins_list(string_to_toks(pdftex_revision));
        return 2;
    } else {
        return 0;
    }
    return 1;
}

void conv_toks(void)
{
    int old_setting;            /* holds |selector| setting */
    halfword p, q;
    int save_scanner_status;    /* |scanner_status| upon entry */
    halfword save_def_ref;      /* |def_ref| upon entry, important if inside `\.{\\message}' */
    halfword save_warning_index;
    boolean bool;               /* temp boolean */
    str_number s;               /* first temp string */
    int sn;                     /* lua chunk name */
    str_number u = 0;           /* third temp string, will become non-nil if a string is already being built */
    int c = cur_chr;            /* desired type of conversion */
    str_number str;
    int i = 0;
    /* Scan the argument for command |c| */
    switch (c) {
        case number_code:
            scan_int();
            push_selector;
            print_int(cur_val);
            pop_selector;
            break;
        case lua_function_code:
            scan_int();
            if (cur_val <= 0) {
                normal_error("luafunction", "invalid number");
            } else {
                u = save_cur_string();
                luacstrings = 0;
                luafunctioncall(cur_val);
                restore_cur_string(u);
                if (luacstrings > 0)
                    lua_string_start();
            }
            /* no further action */
            return;
            break;
        case lua_code:
            u = save_cur_string();
            save_scanner_status = scanner_status;
            save_def_ref = def_ref;
            save_warning_index = warning_index;
            sn = scan_lua_state();
            scan_toks(false, true);
            s = def_ref;
            warning_index = save_warning_index;
            def_ref = save_def_ref;
            scanner_status = save_scanner_status;
            luacstrings = 0;
            luatokencall(s, sn);
            delete_token_ref(s);
            restore_cur_string(u);  /* TODO: check this, was different */
            if (luacstrings > 0)
                lua_string_start();
            /* no further action */
            return;
            break;
        case expanded_code:
            save_scanner_status = scanner_status;
            save_warning_index = warning_index;
            save_def_ref = def_ref;
            u = save_cur_string();
            scan_toks(false, true);
            warning_index = save_warning_index;
            scanner_status = save_scanner_status;
            ins_list(token_link(def_ref));
            def_ref = save_def_ref;
            restore_cur_string(u);
            /* no further action */
            return;
            break;
        case math_style_code:
            push_selector;
            print_math_style();
            pop_selector;
            break;
        case string_code:
            save_scanner_status = scanner_status;
            scanner_status = normal;
            get_token();
            scanner_status = save_scanner_status;
            push_selector;
            if (cur_cs != 0)
                sprint_cs(cur_cs);
            else
                print(cur_chr);
            pop_selector;
            break;
        case cs_string_code:
            save_scanner_status = scanner_status;
            scanner_status = normal;
            get_token();
            scanner_status = save_scanner_status;
            push_selector;
            if (cur_cs != 0)
                sprint_cs_name(cur_cs);
            else
                print(cur_chr);
            pop_selector;
            break;
        case roman_numeral_code:
            scan_int();
            push_selector;
            print_roman_int(cur_val);
            pop_selector;
            break;
        case meaning_code:
            save_scanner_status = scanner_status;
            scanner_status = normal;
            get_token();
            scanner_status = save_scanner_status;
            push_selector;
            print_meaning();
            pop_selector;
            break;
        case uchar_code:
            scan_char_num();
            push_selector;
            print(cur_val);
            pop_selector;
            break;
        case lua_escape_string_code:
            {
                lstring escstr;
                int l = 0;
                save_scanner_status = scanner_status;
                save_def_ref = def_ref;
                save_warning_index = warning_index;
                scan_toks(false, true);
                bool = in_lua_escape;
                in_lua_escape = true;
                escstr.s = (unsigned char *) tokenlist_to_cstring(def_ref, false, &l);
                escstr.l = (unsigned) l;
                in_lua_escape = bool;
                delete_token_ref(def_ref);
                def_ref = save_def_ref;
                warning_index = save_warning_index;
                scanner_status = save_scanner_status;
                (void) lua_str_toks(escstr);
                ins_list(token_link(temp_token_head));
                free(escstr.s);
                return;
            }
            /* no further action */
            break;
        case font_id_code:
            scan_font_ident();
            push_selector;
            print_int(cur_val);
            pop_selector;
            break;
        case font_name_code:
            scan_font_ident();
            push_selector;
            append_string((unsigned char *) font_name(cur_val),(unsigned) strlen(font_name(cur_val)));
            if (font_size(cur_val) != font_dsize(cur_val)) {
                tprint(" at ");
                print_scaled(font_size(cur_val));
                tprint("pt");
            }
            pop_selector;
            break;
        case left_margin_kern_code:
            scan_int();
            if ((box(cur_val) == null) || (type(box(cur_val)) != hlist_node))
                normal_error("marginkern", "a non-empty hbox expected");
            push_selector;
            p = list_ptr(box(cur_val));
            while ((p != null) && (type(p) == glue_node)) {
                p = vlink(p);
            }
            if ((p != null) && (type(p) == margin_kern_node) && (subtype(p) == left_side))
                print_scaled(width(p));
            else
                print_char('0');
            tprint("pt");
            pop_selector;
            break;
        case right_margin_kern_code:
            scan_int();
            if ((box(cur_val) == null) || (type(box(cur_val)) != hlist_node))
                normal_error("marginkern", "a non-empty hbox expected");
            push_selector;
            p = list_ptr(box(cur_val));
            if (p != null) {
                p = tail_of_list(p);
                /*
                    there can be a leftskip, rightskip, penalty and yes, also a disc node with a nesting
                    node that points to glue spec ... and we don't want to analyze that messy lot
                */
                while ((p != null) && (type(p) == glue_node)) {
                    p = alink(p);
                }
                if ((p != null) && ! ((type(p) == margin_kern_node) && (subtype(p) == right_side))) {
                    if (type(p) == disc_node) {
                        q = alink(p);
                        if ((q != null) && ((type(q) == margin_kern_node) && (subtype(q) == right_side))) {
                            p = q;
                        } else {
                            /*
                                officially we should look in the replace but currently protrusion doesn't
                                work anyway with "foo\discretionary{}{}{bar-} " (no following char) so we
                                don't need it now
                            */
                        }
                    }
                }
            }
            if ((p != null) && (type(p) == margin_kern_node) && (subtype(p) == right_side))
                print_scaled(width(p));
            else
                print_char('0');
            tprint("pt");
            pop_selector;
            break;
        case uniform_deviate_code:
            scan_int();
            push_selector;
            print_int(unif_rand(cur_val));
            pop_selector;
            break;
        case normal_deviate_code:
            scan_int();
            push_selector;
            print_int(norm_rand());
            pop_selector;
            break;
        case math_char_class_code:
            {
                mathcodeval mval;
                scan_int();
                mval = get_math_code(cur_val);
                push_selector;
                print_int(mval.class_value);
                pop_selector;
            }
            break;
        case math_char_fam_code:
            {
                mathcodeval mval;
                scan_int();
                mval = get_math_code(cur_val);
                push_selector;
                print_int(mval.family_value);
                pop_selector;
            }
            break;
        case math_char_slot_code:
            {
                mathcodeval mval;
                scan_int();
                mval = get_math_code(cur_val);
                push_selector;
                print_int(mval.character_value);
                pop_selector;
            }
            break;
        case insert_ht_code:
            scan_register_num();
            push_selector;
            i = cur_val;
            p = page_ins_head;
            while (i >= subtype(vlink(p)))
                p = vlink(p);
            if (subtype(p) == i)
                print_scaled(height(p));
            else
                print_char('0');
            tprint("pt");
            pop_selector;
            break;
        case job_name_code:
            if (job_name == 0)
                open_log_file();
            push_selector;
            print_job_name();
            pop_selector;
            break;
        case format_name_code:
            if (job_name == 0)
                open_log_file();
            push_selector;
            print(format_name);
            pop_selector;
            break;
        case luatex_banner_code:
            push_selector;
            tprint(luatex_banner);
            pop_selector;
            break;
        case luatex_revision_code:
            push_selector;
            print(get_luatexrevision());
            pop_selector;
            break;
        case luatex_date_code:
            push_selector;
            print_int(get_luatex_date_info());
            pop_selector;
            break;
        case etex_code:
            push_selector;
            tprint(eTeX_version_string);
            pop_selector;
            break;
        case eTeX_revision_code:
            push_selector;
            tprint(eTeX_revision);
            pop_selector;
            break;
        case font_identifier_code:
            confusion("convert");
            break;
        default:
            confusion("convert");
            break;
    }
    str = make_string();
    (void) str_toks(str_lstring(str));
    flush_str(str);
    ins_list(token_link(temp_token_head));
}

void do_feedback(void)
{
    int c = cur_chr;
    str_number str;
    int done = 1;
    switch (c) {
        case dvi_feedback_code:
            if (get_o_mode() == OMODE_DVI) {
                done = do_feedback_dvi(c);
            } else {
                tex_error("unexpected use of \\dvifeedback",null);
                return ;
            }
            if (done==0) {
                /* we recover */
                normal_warning("dvi backend","unexpected use of \\dvifeedback");
                return;
            } else if (done==2) {
                return;
            }
            break;
        case pdf_feedback_code:
            if (get_o_mode() == OMODE_PDF) {
                done = do_feedback_pdf(c);
            } else {
                tex_error("unexpected use of \\pdffeedback",null);
                return ;
            }
            if (done==0) {
                /* we recover */
                normal_warning("pdf backend","unexpected use of \\pdffeedback");
                return;
            } else if (done==2) {
                return;
            }
            break;
        default:
            confusion("feedback");
            break;
    }
    str = make_string();
    (void) str_toks(str_lstring(str));
    flush_str(str);
    ins_list(token_link(temp_token_head));
#if defined(MIKTEX)
#  undef bool
#endif
}

void do_variable(void)
{
    int c = cur_chr;
    int done = 1;
    switch (c) {
        case dvi_variable_code:
            done = do_variable_dvi(c);
            if (done==0) {
                /* we recover */
                normal_warning("dvi backend","unexpected use of \\dvivariable");
            }
            return;
            break;
        case pdf_variable_code:
            done = do_variable_pdf(c);
            if (done==0) {
                /* we recover */
                normal_warning("pdf backend","unexpected use of \\pdfvariable");
            }
            return;
            break;
        default:
            confusion("variable");
            break;
    }
}

/*
    The following code is not used as we can only set math options and not query them. If
    an option is really important we will provide a proper variable. Most options are not
    meant for users anyway but for development.
*/

/*

#define do_mathoption_int(i) \
    cur_cmd = assign_int_cmd; \
    cur_val = mathoption_int_base + i; \
    cur_tok = token_val(cur_cmd, cur_val); \
    back_input();

void do_mathoption(void)
{
         if (scan_keyword("old"))                    { do_mathoption_int(c_mathoption_no_italic_compensation_code); }
         if (scan_keyword("noitaliccompensation"))   { do_mathoption_int(c_mathoption_no_char_italic_code); }
    else if (scan_keyword("nocharitalic"))           { do_mathoption_int(c_mathoption_use_old_fraction_scaling_code); }
    else if (scan_keyword("useoldfractionscaling"))  { do_mathoption_int(c_mathoption_old_code); }
    else if (scan_keyword("umathcodemeaning"))       { do_mathoption_int(c_mathoption_umathcode_meaning_code); }
}

*/

@ This boolean is keeping track of the lua string escape state
@c
boolean in_lua_escape;

static int the_convert_string_dvi(halfword c, int i)
{
    return 0 ;
}

static int the_convert_string_pdf(halfword c, int i)
{
    int ff;
    if (get_o_mode() != OMODE_PDF) {
        return 0;
    } else if (scan_keyword("lastlink")) {
        print_int(pdf_last_link);
    } else if (scan_keyword("retval")) {
        print_int(pdf_retval);
    } else if (scan_keyword("lastobj")) {
        print_int(pdf_last_obj);
    } else if (scan_keyword("lastannot")) {
        print_int(pdf_last_annot);
    } else if (scan_keyword("xformname")) {
        print_int(obj_info(static_pdf, i));
    } else if (scan_keyword("creationdate")) {
        return 0;
    } else if (scan_keyword("fontname")) {
        set_ff(i);
        print_int(obj_info(static_pdf, pdf_font_num(ff)));
    } else if (scan_keyword("fontobjnum")) {
        set_ff(i);
        print_int(pdf_font_num(ff));
    } else if (scan_keyword("fontsize")) {
        print_scaled(font_size(i));
        tprint("pt");
    } else if (scan_keyword("pageref")) {
        print_int(pdf_get_obj(static_pdf, obj_type_page, i, false));
    } else if (scan_keyword("colorstackinit")) {
        return 0;
    } else {
        return 0;
    }
    return 1;
}

str_number the_convert_string(halfword c, int i)
{
    int old_setting;            /* saved |selector| setting */
    str_number ret = 0;
    boolean done = true ;
    old_setting = selector;
    selector = new_string;
    switch (c) {
        case number_code:
            print_int(i);
            break;
     /* case lua_function_code: */
     /* case lua_code: */
     /* case expanded_code: */
        case math_style_code:
            print_math_style();
            break;
     /* case string_code: */
     /* case cs_string_code: */
        case roman_numeral_code:
            print_roman_int(i);
            break;
     /* case meaning_code: */
        case uchar_code:
            print(i);
            break;
     /* lua_escape_string_code: */
        case font_id_code:
            print_int(i);
            break;
        case font_name_code:
            append_string((unsigned char *) font_name(i),(unsigned) strlen(font_name(i)));
            if (font_size(i) != font_dsize(i)) {
                tprint(" at ");
                print_scaled(font_size(i));
                tprint("pt");
            }
            break;
     /* left_margin_kern_code: */
     /* right_margin_kern_code: */
        case uniform_deviate_code:
            print_int(unif_rand(i));
            break;
        case normal_deviate_code:
            print_int(norm_rand());
            break;
     /* math_char_class_code: */
     /* math_char_fam_code: */
     /* math_char_slot_code: */
     /* insert_ht_code: */
        case job_name_code:
            print_job_name();
            break;
        case format_name_code:
            print(format_name);
            break;
        case luatex_banner_code:
            tprint(luatex_banner);
            break;
        case luatex_revision_code:
            print(get_luatexrevision());
            break;
        case luatex_date_code:
            print_int(get_luatex_date_info());
            break;
        case etex_code:
            tprint(eTeX_version_string);
            break;
        case eTeX_revision_code:
            tprint(eTeX_revision);
            break;
        case font_identifier_code:
            print_font_identifier(i);
            break;
        /* backend: this might become obsolete */
        case dvi_feedback_code:
            done = the_convert_string_dvi(c,i);
            break;
        case pdf_feedback_code:
            done = the_convert_string_pdf(c,i);
            break;
        /* done */
        default:
            done = false;
            break;
    }
    if (done) {
        ret = make_string();
    }
    selector = old_setting;
    return ret;
}

@ Another way to create a token list is via the \.{\\read} command. The sixteen
files potentially usable for reading appear in the following global variables.
The value of |read_open[n]| will be |closed| if stream number |n| has not been
opened or if it has been fully read; |just_open| if an \.{\\openin} but not a
\.{\\read} has been done; and |normal| if it is open and ready to read the next
line.

@c
FILE *read_file[16]; /* used for \.{\\read} */
int read_open[17];   /* state of |read_file[n]| */

void initialize_read(void)
{
    int k;
    for (k = 0; k <= 16; k++)
        read_open[k] = closed;
}

@ The |read_toks| procedure constructs a token list like that for any macro
definition, and makes |cur_val| point to it. Parameter |r| points to the control
sequence that will receive this token list.

@c
void read_toks(int n, halfword r, halfword j)
{
    halfword p; /* tail of the token list */
    halfword q; /* new node being added to the token list via |store_new_token| */
    int s;      /* saved value of |align_state| */
    int m;      /* stream number */
    scanner_status = defining;
    warning_index = r;
    p = get_avail();
    def_ref = p;
    set_token_ref_count(def_ref, 0);
    p = def_ref;                /* the reference count */
    store_new_token(end_match_token);
    if ((n < 0) || (n > 15))
        m = 16;
    else
        m = n;
    s = align_state;
    align_state = 1000000;      /* disable tab marks, etc. */
    do {
        /* Input and store tokens from the next line of the file */
        begin_file_reading();
        iname = m + 1;
        if (read_open[m] == closed) {
            /*
                Input for \.{\\read} from the terminal

                Here we input on-line into the |buffer| array, prompting the user explicitly
                if |n>=0|.  The value of |n| is set negative so that additional prompts
                will not be given in the case of multi-line input.
            */
            if (interaction > nonstop_mode) {
                if (n < 0) {
                    prompt_input("");
                } else {
                    wake_up_terminal();
                    print_ln();
                    sprint_cs(r);
                    prompt_input(" =");
                    n = -1;
                }
            } else {
                fatal_error
                    ("*** (cannot \\read from terminal in nonstop modes)");
            }

        } else if (read_open[m] == just_open) {
            /*
                Input the first line of |read_file[m]|

                The first line of a file must be treated specially, since |lua_input_ln|
                must be told not to start with |get|.
            */
            if (lua_input_ln(read_file[m], (m + 1), false)) {
                read_open[m] = normal;
            } else {
                lua_a_close_in(read_file[m], (m + 1));
                read_open[m] = closed;
            }

        } else {
            /*
                Input the next line of |read_file[m]|

                An empty line is appended at the end of a |read_file|.
            */
            if (!lua_input_ln(read_file[m], (m + 1), true)) {
                lua_a_close_in(read_file[m], (m + 1));
                read_open[m] = closed;
                if (align_state != 1000000) {
                    runaway();
                    print_err("File ended within \\read");
                    help1("This \\read has unbalanced braces.");
                    align_state = 1000000;
                    error();
                }
            }

        }
        ilimit = last;
        if (end_line_char_inactive)
            decr(ilimit);
        else
            buffer[ilimit] = (packed_ASCII_code) end_line_char_par;
        first = ilimit + 1;
        iloc = istart;
        istate = new_line;
        /* Handle \.{\\readline} and |goto done|; */
        if (j == 1) {
            while (iloc <= ilimit) {
                /* current line not yet finished */
                do_buffer_to_unichar(cur_chr, iloc);
                if (cur_chr == ' ')
                    cur_tok = space_token;
                else
                    cur_tok = cur_chr + other_token;
                store_new_token(cur_tok);
            }
        } else {
            while (1) {
                get_token();
                if (cur_tok == 0) {
                    /* |cur_cmd=cur_chr=0| will occur at the end of the line */
                    break;
                }
                if (align_state < 1000000) {
                    /* unmatched `\.\}' aborts the line */
                    do {
                        get_token();
                    } while (cur_tok != 0);
                    align_state = 1000000;
                    break;
                }
                store_new_token(cur_tok);
            }
        }
        end_file_reading();

    } while (align_state != 1000000);
    cur_val = def_ref;
    scanner_status = normal;
    align_state = s;
}

@ return a string from tokens list

@c
str_number tokens_to_string(halfword p)
{
    int old_setting;
    if (selector == new_string)
        normal_error("tokens","tokens_to_string() called while selector = new_string");
    old_setting = selector;
    selector = new_string;
    show_token_list(token_link(p), null, -1);
    selector = old_setting;
    return make_string();
}

@ @c
#define make_room(a)                     \
    if ((unsigned)i+a+1>alloci) {        \
        ret = xrealloc(ret,(alloci+64)); \
        alloci = alloci + 64;            \
    }

#define append_i_byte(a) ret[i++] = (char)(a)

#define Print_char(a) make_room(1); append_i_byte(a)

#define Print_uchar(s) {                                       \
    make_room(4);                                              \
    if (s<=0x7F) {                                             \
      append_i_byte(s);                                        \
    } else if (s<=0x7FF) {                                     \
      append_i_byte(0xC0 + (s / 0x40));                        \
      append_i_byte(0x80 + (s % 0x40));                        \
    } else if (s<=0xFFFF) {                                    \
      append_i_byte(0xE0 + (s / 0x1000));                      \
      append_i_byte(0x80 + ((s % 0x1000) / 0x40));             \
      append_i_byte(0x80 + ((s % 0x1000) % 0x40));             \
    } else if (s>=0x110000) {                                  \
      append_i_byte(s-0x11000);                                \
    } else {                                                   \
      append_i_byte(0xF0 + (s / 0x40000));                     \
      append_i_byte(0x80 + ((s % 0x40000) / 0x1000));          \
      append_i_byte(0x80 + (((s % 0x40000) % 0x1000) / 0x40)); \
      append_i_byte(0x80 + (((s % 0x40000) % 0x1000) % 0x40)); \
    } }

#define Print_esc(b) {                     \
    const char *v = b;                     \
    if (e>0 && e<STRING_OFFSET) {          \
        Print_uchar (e);                   \
    }                                      \
    make_room(strlen(v));                  \
    while (*v) { append_i_byte(*v); v++; } \
  }

#define Print_str(b) {                     \
    const char *v = b;                     \
    make_room(strlen(v));                  \
    while (*v) { append_i_byte(*v); v++; } \
  }

#define is_cat_letter(a) \
    (get_char_cat_code(pool_to_unichar(str_string((a)))) == 11)

@ the actual token conversion in this function is now functionally equivalent to
|show_token_list|, except that it always prints the whole token list. TODO: check
whether this causes problems in the lua library.

@c
char *tokenlist_to_cstring(int pp, int inhibit_par, int *siz)
{
    register int p, c, m;
    int q;
    int infop;
    char *s, *sh;
    int e = 0;
    char *ret;
    int match_chr = '#';
    int n = '0';
    unsigned alloci = 1024;
    int i = 0;
    p = pp;
    if (p == null) {
        if (siz != NULL)
            *siz = 0;
        return NULL;
    }
    ret = xmalloc(alloci);
    p = token_link(p);          /* skip refcount */
    if (p != null) {
        e = escape_char_par;
    }
    while (p != null) {
        if (p < (int) fix_mem_min || p > (int) fix_mem_end) {
            Print_esc("CLOBBERED.");
            break;
        }
        infop = token_info(p);
        if (infop >= cs_token_flag) {
            if (!(inhibit_par && infop == par_token)) {
                q = infop - cs_token_flag;
                if (q < hash_base) {
                    if (q == null_cs) {
                        Print_esc("csname");
                        Print_esc("endcsname");
                    } else {
                        Print_esc("IMPOSSIBLE.");
                    }
                } else if ((q >= undefined_control_sequence) && ((q <= eqtb_size) || (q > eqtb_size + hash_extra))) {
                    Print_esc("IMPOSSIBLE.");
                } else if ((cs_text(q) < 0) || (cs_text(q) >= str_ptr)) {
                    Print_esc("NONEXISTENT.");
                } else {
                    str_number txt = cs_text(q);
                    sh = makecstring(txt);
                    s = sh;
                    if (is_active_cs(txt)) {
                        s = s + 3;
                        while (*s) {
                            Print_char(*s);
                            s++;
                        }
                    } else {
                        if (e>=0 && e<0x110000) Print_uchar(e);
                        while (*s) {
                            Print_char(*s);
                            s++;
                        }
                        if ((!single_letter(txt)) || is_cat_letter(txt)) {
                            Print_char(' ');
                        }
                    }
                    free(sh);
                }
            }
        } else {
            if (infop < 0) {
                Print_esc("BAD");
            } else {
                m = token_cmd(infop);
                c = token_chr(infop);
                switch (m) {
                    case left_brace_cmd:
                    case right_brace_cmd:
                    case math_shift_cmd:
                    case tab_mark_cmd:
                    case sup_mark_cmd:
                    case sub_mark_cmd:
                    case spacer_cmd:
                    case letter_cmd:
                    case other_char_cmd:
                        Print_uchar(c);
                        break;
                    case mac_param_cmd:
                        if (!in_lua_escape && (is_in_csname==0))
                            Print_uchar(c);
                        Print_uchar(c);
                        break;
                    case out_param_cmd:
                        Print_uchar(match_chr);
                        if (c <= 9) {
                            Print_char(c + '0');
                        } else {
                            Print_char('!');
                            goto EXIT;
                        }
                        break;
                    case match_cmd:
                        match_chr = c;
                        Print_uchar(c);
                        n++;
                        Print_char(n);
                        if (n > '9')
                            goto EXIT;
                        break;
                    case end_match_cmd:
                        if (c == 0) {
                            Print_char('-');
                            Print_char('>');
                        }
                        break;
                    default:
                        not_so_bad(Print_esc);
                        break;
                }
            }
        }
        p = token_link(p);
    }
  EXIT:
    ret[i] = '\0';
    if (siz != NULL)
        *siz = i;
    return ret;
}

@ @c
lstring *tokenlist_to_lstring(int pp, int inhibit_par)
{
    int siz;
    lstring *ret = xmalloc(sizeof(lstring));
    ret->s = (unsigned char *) tokenlist_to_cstring(pp, inhibit_par, &siz);
    ret->l = (size_t) siz;
    return ret;
}

@ @c
void free_lstring(lstring * ls)
{
    if (ls == NULL)
        return;
    if (ls->s != NULL)
        free(ls->s);
    free(ls);
}
