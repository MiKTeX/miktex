% inputstack.w
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

@ @c
#define end_line_char int_par(end_line_char_code)
#define error_context_lines int_par(error_context_lines_code)

in_state_record *input_stack = NULL;
int input_ptr = 0;              /* first unused location of |input_stack| */
int max_in_stack = 0;           /* largest value of |input_ptr| when pushing */
in_state_record cur_input;      /* the ``top'' input state */


int in_open = 0;                /* the number of lines in the buffer, less one */
int open_parens = 0;            /* the number of open text files */
alpha_file *input_file = NULL;
int line = 0;                   /* current line number in the current source file */
int *line_stack = NULL;
str_number *source_filename_stack = NULL;
char **full_source_filename_stack = NULL;


int scanner_status = 0;         /* can a subfile end now? */
pointer warning_index = null;   /* identifier relevant to non-|normal| scanner status */
pointer def_ref = null;         /* reference count of token list being defined */

@ Here is a procedure that uses |scanner_status| to print a warning message
when a subfile has ended, and at certain other crucial times:

@c
void runaway(void)
{
    pointer p = null; /* head of runaway list */
    if (scanner_status > skipping) {
        switch (scanner_status) {
        case defining:
            tprint_nl("Runaway definition");
            p = def_ref;
            break;
        case matching:
            tprint_nl("Runaway argument");
            p = temp_token_head;
            break;
        case aligning:
            tprint_nl("Runaway preamble");
            p = hold_token_head;
            break;
        case absorbing:
            tprint_nl("Runaway text");
            p = def_ref;
            break;
        default:
            /* there are no other cases */
            break;
        }
        print_char('?');
        print_ln();
        show_token_list(token_link(p), null, error_line - 10);
    }
}

@ The |param_stack| is an auxiliary array used to hold pointers to the token
lists for parameters at the current level and subsidiary levels of input.
This stack is maintained with convention (2), and it grows at a different
rate from the others.

@c
pointer *param_stack = NULL;    /* token list pointers for parameters */
int param_ptr = 0;              /* first unused entry in |param_stack| */
int max_param_stack = 0;        /* largest value of |param_ptr|, will be |<=param_size+9| */

@ The input routines must also interact with the processing of
\.{\\halign} and \.{\\valign}, since the appearance of tab marks and
\.{\\cr} in certain places is supposed to trigger the beginning of special
$v_j$ template text in the scanner. This magic is accomplished by an
|align_state| variable that is increased by~1 when a `\.{\char'173}' is
scanned and decreased by~1 when a `\.{\char'175}' is scanned. The |align_state|
is nonzero during the $u_j$ template, after which it is set to zero; the
$v_j$ template begins when a tab mark or \.{\\cr} occurs at a time that
|align_state=0|.

@c
int align_state = 0;            /* group level with respect to current alignment */

@ Thus, the ``current input state'' can be very complicated indeed; there
can be many levels and each level can arise in a variety of ways. The
|show_context| procedure, which is used by \TeX's error-reporting routine to
print out the current input state on all levels down to the most recent
line of characters from an input file, illustrates most of these conventions.
The global variable |base_ptr| contains the lowest level that was
displayed by this procedure.

@c
int base_ptr = 0;               /* shallowest level shown by |show_context| */

@ The status at each level is indicated by printing two lines, where the first
line indicates what was read so far and the second line shows what remains
to be read. The context is cropped, if necessary, so that the first line
contains at most |half_error_line| characters, and the second contains
at most |error_line|. Non-current input levels whose |token_type| is
`|backed_up|' are shown only if they have not been fully read.

@c
static void print_token_list_type(int t)
{
    switch (t) {
        case parameter:
            tprint_nl("<argument> ");
            break;
        case u_template:
        case v_template:
            tprint_nl("<template> ");
            break;
        case backed_up:
            if (iloc == null)
                tprint_nl("<recently read> ");
            else
                tprint_nl("<to be read again> ");
            break;
        case inserted:
            tprint_nl("<inserted text> ");
            break;
        case macro:
            print_ln();
            print_cs(iname);
            break;
        case output_text:
            tprint_nl("<output> ");
            break;
        case every_par_text:
            tprint_nl("<everypar> ");
            break;
        case every_math_text:
            tprint_nl("<everymath> ");
            break;
        case every_display_text:
            tprint_nl("<everydisplay> ");
            break;
        case every_hbox_text:
            tprint_nl("<everyhbox> ");
            break;
        case every_vbox_text:
            tprint_nl("<everyvbox> ");
            break;
        case every_job_text:
            tprint_nl("<everyjob> ");
            break;
        case every_cr_text:
            tprint_nl("<everycr> ");
            break;
        case mark_text:
            tprint_nl("<mark> ");
            break;
        case every_eof_text:
            tprint_nl("<everyeof> ");
            break;
        case write_text:
            tprint_nl("<write> ");
            break;
        default:
            tprint_nl("?");
            /* this should never happen */
            break;
    }
}

@ Here it is necessary to explain a little trick. We don't want to store a long
string that corresponds to a token list, because that string might take up
lots of memory; and we are printing during a time when an error message is
being given, so we dare not do anything that might overflow one of \TeX's
tables. So `pseudoprinting' is the answer: We enter a mode of printing
that stores characters into a buffer of length |error_line|, where character
$k+1$ is placed into \hbox{|trick_buf[k mod error_line]|} if
|k<trick_count|, otherwise character |k| is dropped. Initially we set
|tally:=0| and |trick_count:=1000000|; then when we reach the
point where transition from line 1 to line 2 should occur, we
set |first_count:=tally| and |trick_count:=@tmax@>(error_line,
tally+1+error_line-half_error_line)|. At the end of the
pseudoprinting, the values of |first_count|, |tally|, and
|trick_count| give us all the information we need to print the two lines,
and all of the necessary text is in |trick_buf|.

Namely, let |l| be the length of the descriptive information that appears
on the first line. The length of the context information gathered for that
line is |k=first_count|, and the length of the context information
gathered for line~2 is $m=\min(|tally|, |trick_count|)-k$. If |l+k<=h|,
where |h=half_error_line|, we print |trick_buf[0..k-1]| after the
descriptive information on line~1, and set |n:=l+k|; here |n| is the
length of line~1. If $l+k>h$, some cropping is necessary, so we set |n:=h|
and print `\.{...}' followed by
$$\hbox{|trick_buf[(l+k-h+3)..k-1]|,}$$
where subscripts of |trick_buf| are circular modulo |error_line|. The
second line consists of |n|~spaces followed by |trick_buf[k..(k+m-1)]|,
unless |n+m>error_line|; in the latter case, further cropping is done.
This is easier to program than to explain.

@ The following code sets up the print routines so that they will gather
the desired information.

@c
void set_trick_count(void)
{
    first_count = tally;
    trick_count = tally + 1 + error_line - half_error_line;
    if (trick_count < error_line)
        trick_count = error_line;
}

#define begin_pseudoprint() do { \
    l=tally; \
    tally=0; \
    selector=pseudo; \
    trick_count=1000000; \
  } while (0)

#define PSEUDO_PRINT_THE_LINE()	do { \
    begin_pseudoprint(); \
    if (buffer[ilimit]==end_line_char) \
        j=ilimit; \
    else \
        j=ilimit+1; /* determine the effective end of the line */ \
    if (j>0) { \
        for (i=istart;i<=j-1;i++) { \
            if (i==iloc) \
                set_trick_count(); \
            print_char(buffer[i]); \
        } \
    } \
} while (0)

/*
    We don't care too much if we stay a bit too much below the max error_line
    even if we have more room on the line. If length is really an issue then
    any length is. After all one can set the length larger.
*/

#define print_valid_utf8(q) do { \
    c = (int)trick_buf[q % error_line]; \
    if (c < 128) { \
        print_char(c); \
    } else if (c < 194) { \
        /* invalid */ \
    } else if (c < 224) { \
        print_char(c); \
        print_char(trick_buf[(q+1) % error_line]); \
    } else if (c < 240) { \
        print_char(c); \
        print_char(trick_buf[(q+1) % error_line]); \
        print_char(trick_buf[(q+2) % error_line]); \
    } else if (c < 245) { \
        print_char(c); \
        print_char(trick_buf[(q+1) % error_line]); \
        print_char(trick_buf[(q+2) % error_line]); \
        print_char(trick_buf[(q+3) % error_line]); \
    } else { \
        /* invalid */ \
    } \
} while (0)

@ @c
void show_context(void)
{                                /* prints where the scanner is */
    int old_setting;             /* saved |selector| setting */
    int nn = -1;                 /* number of contexts shown so far, less one */
    boolean bottom_line = false; /* have we reached the final context to be shown? */
    int i;                       /* index into |buffer| */
    int j;                       /* end of current line in |buffer| */
    int l;                       /* length of descriptive information on line 1 */
    int m;                       /* context information gathered for line 2 */
    int n;                       /* length of line 1 */
    int p;                       /* starting or ending place in |trick_buf| */
    int q;                       /* temporary index */
    int c;                       /* used in sanitizer */
    base_ptr = input_ptr;
    input_stack[base_ptr] = cur_input;
    /* store current state */
    while (true) {
        cur_input = input_stack[base_ptr];      /* enter into the context */
        if (istate != token_list) {
            if ((iname > 21) || (base_ptr == 0))
                bottom_line = true;
        }
        if ((base_ptr == input_ptr) || bottom_line || (nn < error_context_lines)) {
            /* Display the current context */
            if ((base_ptr == input_ptr) || (istate != token_list) || (token_type != backed_up) || (iloc != null)) {
                /* we omit backed-up token lists that have already been read */
                tally = 0;      /* get ready to count characters */
                old_setting = selector;
                if (istate != token_list) {
                    /*
                        Print location of current line

                        This routine should be changed, if necessary, to give the best possible
                        indication of where the current line resides in the input file. For example,
                        on some systems it is best to print both a page and line number.
                     */
                    if (iname <= 17) {
                        if (terminal_input) {
                            if (base_ptr == 0)
                                tprint_nl("<*>");
                            else
                                tprint_nl("<insert> ");
                        } else {
                            tprint_nl("<read ");
                            if (iname == 17)
                                print_char('*');
                            else
                                print_int(iname - 1);
                            print_char('>');
                        };
                    } else {
                        tprint_nl("l.");
                        if (iindex == in_open) {
                            print_int(line);
                        } else {     /* input from a pseudo file */
                            print_int(line_stack[iindex + 1]);
                        }
                    }
                    print_char(' ');
                    PSEUDO_PRINT_THE_LINE();
                } else {
                    print_token_list_type(token_type);

                    begin_pseudoprint();
                    if (token_type < macro)
                        show_token_list(istart, iloc, 100000);
                    else
                        show_token_list(token_link(istart), iloc, 100000);      /* avoid reference count */
                }
                /* stop pseudoprinting */
                selector = old_setting;
                /* Print two lines using the tricky pseudoprinted information */
                if (trick_count == 1000000)
                    set_trick_count();
                /* |set_trick_count| must be performed */
                if (tally < trick_count)
                    m = tally - first_count;
                else
                    m = trick_count - first_count;      /* context on line 2 */
                if (l + first_count <= half_error_line) {
                    p = 0;
                    n = l + first_count;
                } else {
                    tprint("...");
                    p = l + first_count - half_error_line + 3;
                    n = half_error_line;
                }
                for (q = p; q <= first_count - 1; q++)
                    print_valid_utf8(q);
                    print_ln();
                /* print |n| spaces to begin line~2 */
                for (q = 1; q <= n; q++)
                    print_char(' ');
                if (m + n <= error_line)
                    p = first_count + m;
                else
                    p = first_count + (error_line - n - 3);
                for (q = first_count; q <= p - 1; q++)
                    print_valid_utf8(q);
                if (m + n > error_line)
                    tprint("...");
                incr(nn);
            }
        } else if (nn == error_context_lines) {
            tprint_nl("...");
            incr(nn);
            /* omitted if |error_context_lines<0| */
        }
        if (bottom_line)
            break;
        decr(base_ptr);
    }
    /* restore original state */
    cur_input = input_stack[input_ptr];
}

@ The following subroutines change the input status in commonly needed ways.

First comes |push_input|, which stores the current state and creates a
new level (having, initially, the same properties as the old).

@c

/* enter a new input level, save the old */

# define pop_input() \
    cur_input=input_stack[--input_ptr]

# define push_input() \
    if (input_ptr > max_in_stack) { \
        max_in_stack = input_ptr; \
        if (input_ptr == stack_size) \
            overflow("input stack size", (unsigned) stack_size); \
    } \
    input_stack[input_ptr] = cur_input; \
    nofilter = false; \
    incr(input_ptr);

@
Here is a procedure that starts a new level of token-list input, given
a token list |p| and its type |t|. If |t=macro|, the calling routine should
set |name| and |loc|.

@c
void begin_token_list(halfword p, quarterword t)
{
    push_input();
    istate = token_list;
    istart = p;
    token_type = (unsigned char) t;
    if (t >= macro) {
        /* the token list starts with a reference count */
        add_token_ref(p);
        if (t == macro) {
            param_start = param_ptr;
        } else {
            iloc = token_link(p);
            if (int_par(tracing_macros_code) > 1) {
                begin_diagnostic();
                tprint_nl("");
                if (t == mark_text)
                    tprint_esc("mark");
                else if (t == write_text)
                    tprint_esc("write");
                else
                    print_cmd_chr(assign_toks_cmd,
                                  t - output_text + output_routine_loc);
                tprint("->");
                token_show(p);
                end_diagnostic(false);
            }
        }
    } else {
        iloc = p;
    }
}

@ When a token list has been fully scanned, the following computations
should be done as we leave that level of input. The |token_type| tends
to be equal to either |backed_up| or |inserted| about 2/3 of the time.
@^inner loop@>

@c
void end_token_list(void)
{
    /* leave a token-list input level */
    if (token_type >= backed_up) {
        /* token list to be deleted */
        if (token_type <= inserted) {
            flush_list(istart);
        } else {
            /* update reference count */
            delete_token_ref(istart);
            if (token_type == macro) {
                /* parameters must be flushed */
                while (param_ptr > param_start) {
                    decr(param_ptr);
                    flush_list(param_stack[param_ptr]);
                }
            }
        }
    } else if (token_type == u_template) {
        if (align_state > 500000)
            align_state = 0;
        else
            fatal_error("(interwoven alignment preambles are not allowed)");
    }
    pop_input();
    check_interrupt();
}

@ Sometimes \TeX\ has read too far and wants to ``unscan'' what it has
seen. The |back_input| procedure takes care of this by putting the token
just scanned back into the input stream, ready to be read again. This
procedure can be used only if |cur_tok| represents the token to be
replaced. Some applications of \TeX\ use this procedure a lot,
so it has been slightly optimized for speed.
@^inner loop@>

@c

/* undoes one token of input */

void back_input(void)
{
    halfword p; /* a token list of length one */
    while ((istate == token_list) && (iloc == null) && (token_type != v_template)) {
        /* conserve stack space */
        end_token_list();
    }
    p = get_avail();
    set_token_info(p, cur_tok);
    if (cur_tok < right_brace_limit) {
        if (cur_tok < left_brace_limit)
            decr(align_state);
        else
            incr(align_state);
    }
    push_input();
    istate = token_list;
    istart = p;
    token_type = backed_up;
    iloc = p;                   /* that was |back_list(p)|, without procedure overhead */
}

@ Insert token |p| into \TeX's input
@c

void reinsert_token(boolean a, halfword pp)
{
    halfword t;
    t = cur_tok;
    cur_tok = pp;
    if (a) {
        halfword p;
        p = get_avail();
        set_token_info(p, cur_tok);
        set_token_link(p, iloc);
        iloc = p;
        istart = p;
        if (cur_tok < right_brace_limit) {
            if (cur_tok < left_brace_limit)
                decr(align_state);
            else
                incr(align_state);
        }
    } else {
        back_input();
    }
    cur_tok = t;
}

@ The |begin_file_reading| procedure starts a new level of input for lines
of characters to be read from a file, or as an insertion from the
terminal. It does not take care of opening the file, nor does it set |loc|
or |limit| or |line|.
@^system dependencies@>

@c
void begin_file_reading(void)
{
    if (in_open == max_in_open)
        overflow("text input levels", (unsigned) max_in_open);
    if (first == buf_size)
        check_buffer_overflow(first);
    incr(in_open);
    push_input();
    iindex = (unsigned char) in_open;
    source_filename_stack[iindex] = 0;
    full_source_filename_stack[iindex] = NULL;
    eof_seen[iindex] = false;
    grp_stack[iindex] = cur_boundary;
    if_stack[iindex] = cond_ptr;
    line_stack[iindex] = line;
    istart = first;
    istate = mid_line;
    iname = 0;                  /* |terminal_input| is now |true| */
    line_catcode_table = DEFAULT_CAT_TABLE;
    line_partial = false;
    /* Prepare terminal input {\sl Sync\TeX} information */
    synctex_tag = 0;
}

@ Conversely, the variables must be downdated when such a level of input
is finished:

@c
void end_file_reading(void)
{
    first = istart;
    line = line_stack[iindex];
    if ((iname >= 18) && (iname <= 20))
        pseudo_close();
    else if (iname == 21)
        luacstring_close(iindex);
    else if (iname > 17)
        lua_a_close_in(cur_file, 0);    /* forget it */
    pop_input();
    decr(in_open);
}

@ In order to keep the stack from overflowing during a long sequence of
inserted `\.{\\show}' commands, the following routine removes completed
error-inserted lines from memory.

@c
void clear_for_error_prompt(void)
{
    while ((istate != token_list) && terminal_input
           && (input_ptr > 0) && (iloc > ilimit))
        end_file_reading();
    print_ln();
    clear_terminal();
}

@ To get \TeX's whole input mechanism going, we perform the following actions.

@c
void initialize_inputstack(void)
{
    input_ptr = 0;
    max_in_stack = 0;
    source_filename_stack[0] = 0;

    full_source_filename_stack[0] = NULL;
    in_open = 0;
    open_parens = 0;
    max_buf_stack = 0;

    grp_stack[0] = 0;
    if_stack[0] = null;
    param_ptr = 0;
    max_param_stack = 0;
    first = buf_size;
    do {
        buffer[first] = 0;
        decr(first);
    } while (first != 0);
    scanner_status = normal;
    warning_index = null;
    first = 1;
    istate = new_line;
    istart = 1;
    iindex = 0;
    line = 0;
    iname = 0;
    nofilter = false;
    force_eof = false;
    luacstrings = 0;
    line_catcode_table = DEFAULT_CAT_TABLE;
    line_partial = false;
    align_state = 1000000;
    if (!init_terminal())
        exit(EXIT_FAILURE);     /* |goto final_end|; */
    ilimit = last;
    first = last + 1;           /* |init_terminal| has set |loc| and |last| */
}

@ The global variable |pseudo_files| is used to maintain a stack of
pseudo files.  The |pseudo_lines| field of each pseudo file points to
a linked list of variable size nodes representing lines not yet
processed: the |subtype| field contains the size of this node,
all the following words contain ASCII codes.

/*

    hh: todo: if this is really critical code (which it isn't) then we can
    consider a c stack and store a pointer to a line in the line node instead
    which saves splitting here and reconstructing later.

*/


@c
halfword pseudo_files; /* stack of pseudo files */

static halfword string_to_pseudo(str_number str, int nl)
{
    halfword i, r, q = null;
    unsigned l, len;
    four_quarters w;
    int sz;
    halfword h = new_node(pseudo_file_node, 0);
    unsigned char *s = str_string(str);
    len = (unsigned) str_length(str);
    l = 0;
    while (l < len) {
        unsigned m = l; /* start of current line */
        while ((l < len) && (s[l] != nl))
            l++;
        sz = (int) (l - m + 7) / 4;
        if (sz == 1)
            sz = 2;
        r = new_node(pseudo_line_node, sz);
        i = r;
        while (--sz > 1) {
            w.b0 = s[m++];
            w.b1 = s[m++];
            w.b2 = s[m++];
            w.b3 = s[m++];
            varmem[++i].qqqq = w;
        }
        w.b0 = (quarterword) (l > m ? s[m++] : ' ');
        w.b1 = (quarterword) (l > m ? s[m++] : ' ');
        w.b2 = (quarterword) (l > m ? s[m++] : ' ');
        w.b3 = (quarterword) (l > m ? s[m] : ' ');
        varmem[++i].qqqq = w;
        if (q == null) {
            pseudo_lines(h) = r;
        } else {
            vlink(q) = r ; /* no prev node here so no couple_nodes !*/
        }
        q = r ;
        if (s[l] == nl)
            l++;
    }
    return h;
}

@ The |pseudo_start| procedure initiates reading from a pseudo file.

@c
void pseudo_from_string(void)
{
    str_number s;  /* string to be converted into a pseudo file */
    halfword p;    /* for list construction */
    s = make_string();
    /* Convert string |s| into a new pseudo file */
    p = string_to_pseudo(s, int_par(new_line_char_code));
    vlink(p) = pseudo_files;
    pseudo_files = p;
    flush_str(s);
    /* Initiate input from new pseudo file */
    begin_file_reading(); /* set up |cur_file| and new level of input */
    line = 0;
    ilimit = istart;
    iloc = ilimit + 1; /* force line read */
    if (int_par(tracing_scan_tokens_code) > 0) {
        if (term_offset > max_print_line - 3)
            print_ln();
        else if ((term_offset > 0) || (file_offset > 0))
            print_char(' ');
        iname = 20;
        tprint("( ");
        incr(open_parens);
        update_terminal();
    } else {
        iname = 18;
    }
    /* Prepare pseudo file {\sl Sync\TeX} information */
    synctex_tag = 0;
}

void pseudo_start(void)
{
    int old_setting;
    scan_general_text();
    old_setting = selector;
    selector = new_string;
    token_show(temp_token_head);
    selector = old_setting;
    flush_list(token_link(temp_token_head));
    str_room(1);
    pseudo_from_string();
}

@ @c
void lua_string_start(void)
{
    begin_file_reading(); /* set up |cur_file| and new level of input */
    line = 0;
    ilimit = istart;
    iloc = ilimit + 1; /* force line read */
    iname = 21;
    luacstring_start(iindex);
}

@ Here we read a line from the current pseudo file into |buffer|.

@c
/* inputs the next line or returns |false| */

boolean pseudo_input(void)
{
    halfword p;      /* current line from pseudo file */
    int sz;          /* size of node |p| */
    four_quarters w; /* four ASCII codes */
    halfword r;      /* loop index */
    last = first;    /* cf.\ Matthew 19\thinspace:\thinspace30 */
    p = pseudo_lines(pseudo_files);
    if (p == null) {
        return false;
    } else {
        pseudo_lines(pseudo_files) = vlink(p);
        sz = subtype(p);
        if (4 * sz - 3 >= buf_size - last)
            check_buffer_overflow(last + 4 * sz);
        last = first;
        for (r = p + 1; r <= p + sz - 1; r++) {
            w = varmem[r].qqqq;
            buffer[last]     = (packed_ASCII_code) w.b0;
            buffer[last + 1] = (packed_ASCII_code) w.b1;
            buffer[last + 2] = (packed_ASCII_code) w.b2;
            buffer[last + 3] = (packed_ASCII_code) w.b3;
            last += 4;
        }
        if (last >= max_buf_stack)
            max_buf_stack = last + 1;
        while ((last > first) && (buffer[last - 1] == ' '))
            decr(last);
        flush_node(p);
    }
    return true;
}

@ When we are done with a pseudo file we `close' it.

@c
/* close the top level pseudo file */

void pseudo_close(void)
{
    halfword p;
    p = vlink(pseudo_files);
    flush_node(pseudo_files);
    pseudo_files = p;
}
