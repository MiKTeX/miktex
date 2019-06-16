/* inputstack.h

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


#ifndef INPUTSTACK_H
#  define INPUTSTACK_H 1

/*
  The state of \TeX's input mechanism appears in the input stack, whose
  entries are records with six fields, called |state|, |index|, |start|, |loc|,
  |limit|, and |name|.
*/

typedef struct in_state_record {
    halfword start_field;
    halfword loc_field;
    halfword limit_field;
    halfword name_field;
    int synctex_tag_field;      /* stack the tag of the current file */
    signed int cattable_field:16;       /* category table used by the current line (see textoken.c) */
    quarterword state_field:8;
    quarterword index_field:8;
    boolean partial_field:8;    /* is the current line partial? (see textoken.c) */
    boolean nofilter_field:8;   /* used by token filtering */
} in_state_record;

extern in_state_record *input_stack;
extern int input_ptr;
extern int max_in_stack;
extern in_state_record cur_input;       /* the ``top'' input state */

#  define  iloc cur_input.loc_field     /* location of first unread character in |buffer| */
#  define  istate cur_input.state_field /* current scanner state */
#  define  iindex cur_input.index_field /* reference for buffer information */
#  define  istart cur_input.start_field /* starting position in |buffer| */
#  define  ilimit cur_input.limit_field /* end of current line in |buffer| */
#  define  iname cur_input.name_field   /* name of the current file  */
#  define  nofilter cur_input.nofilter_field    /* is token filtering explicitly disallowed? */
#  define  synctex_tag cur_input.synctex_tag_field      /* tag of the current file */
#  define  line_catcode_table cur_input.cattable_field
#  define  line_partial cur_input.partial_field

/*
Let's look more closely now at the control variables
(|state|,~|index|,~|start|,~|loc|,~|limit|,~|name|),
assuming that \TeX\ is reading a line of characters that have been input
from some file or from the user's terminal. There is an array called
|buffer| that acts as a stack of all lines of characters that are
currently being read from files, including all lines on subsidiary
levels of the input stack that are not yet completed. \TeX\ will return to
the other lines when it is finished with the present input file.

(Incidentally, on a machine with byte-oriented addressing, it might be
appropriate to combine |buffer| with the |str_pool| array,
letting the buffer entries grow downward from the top of the string pool
and checking that these two tables don't bump into each other.)

The line we are currently working on begins in position |start| of the
buffer; the next character we are about to read is |buffer[loc]|; and
|limit| is the location of the last character present.  If |loc>limit|,
the line has been completely read. Usually |buffer[limit]| is the
|end_line_char|, denoting the end of a line, but this is not
true if the current line is an insertion that was entered on the user's
terminal in response to an error message.

The |name| variable is a string number that designates the name of
the current file, if we are reading a text file. It is zero if we
are reading from the terminal; it is |n+1| if we are reading from
input stream |n|, where |0<=n<=16|. (Input stream 16 stands for
an invalid stream number; in such cases the input is actually from
the terminal, under control of the procedure |read_toks|.)
Finally |18<=name<=20| indicates that we are reading a pseudo file
created by the \.{\\scantokens} or \.{\\scantextokens} command.

The |state| variable has one of three values, when we are scanning such
files:
$$\baselineskip 15pt\vbox{\halign{#\hfil\cr
1) |state=mid_line| is the normal state.\cr
2) |state=skip_blanks| is like |mid_line|, but blanks are ignored.\cr
3) |state=new_line| is the state at the beginning of a line.\cr}}$$
These state values are assigned numeric codes so that if we add the state
code to the next character's command code, we get distinct values. For
example, `|mid_line+spacer|' stands for the case that a blank
space character occurs in the middle of a line when it is not being
ignored; after this case is processed, the next value of |state| will
be |skip_blanks|.
*/

#  define max_char_code 15      /* largest catcode for individual characters */

typedef enum {
    mid_line = 1,               /* |state| code when scanning a line of characters */
    skip_blanks = 2 + max_char_code,    /* |state| code when ignoring blanks */
    new_line = 3 + max_char_code + max_char_code,       /* |state| code at start of line */
} state_codes;

/*
Additional information about the current line is available via the
|index| variable, which counts how many lines of characters are present
in the buffer below the current level. We have |index=0| when reading
from the terminal and prompting the user for each line; then if the user types,
e.g., `\.{\\input paper}', we will have |index=1| while reading
the file \.{paper.tex}. However, it does not follow that |index| is the
same as the input stack pointer, since many of the levels on the input
stack may come from token lists. For example, the instruction `\.{\\input
paper}' might occur in a token list.

The global variable |in_open| is equal to the |index|
value of the highest non-token-list level. Thus, the number of partially read
lines in the buffer is |in_open+1|, and we have |in_open=index|
when we are not reading a token list.

If we are not currently reading from the terminal, or from an input
stream, we are reading from the file variable |input_file[index]|. We use
the notation |terminal_input| as a convenient abbreviation for |name=0|,
and |cur_file| as an abbreviation for |input_file[index]|.

The global variable |line| contains the line number in the topmost
open file, for use in error messages. If we are not reading from
the terminal, |line_stack[index]| holds the line number for the
enclosing level, so that |line| can be restored when the current
file has been read. Line numbers should never be negative, since the
negative of the current line number is used to identify the user's output
routine in the |mode_line| field of the semantic nest entries.

If more information about the input state is needed, it can be
included in small arrays like those shown here. For example,
the current page or segment number in the input file might be
put into a variable |@!page|, maintained for enclosing levels in
`\ignorespaces|@!page_stack:array[1..max_in_open] of integer|\unskip'
by analogy with |line_stack|.
@^system dependencies@>
*/

#  define terminal_input (iname==0)     /* are we reading from the terminal? */
#  define cur_file input_file[iindex]   /* the current |alpha_file| variable */

extern int in_open;
extern int open_parens;
extern alpha_file *input_file;
extern int line;
extern int *line_stack;
extern str_number *source_filename_stack;
extern char **full_source_filename_stack;

/*
Users of \TeX\ sometimes forget to balance left and right braces properly,
and one of the ways \TeX\ tries to spot such errors is by considering an
input file as broken into subfiles by control sequences that
are declared to be \.{\\outer}.

A variable called |scanner_status| tells \TeX\ whether or not to complain
when a subfile ends. This variable has six possible values:

\yskip\hang|normal|, means that a subfile can safely end here without incident.

\yskip\hang|skipping|, means that a subfile can safely end here, but not a file,
because we're reading past some conditional text that was not selected.

\yskip\hang|defining|, means that a subfile shouldn't end now because a
macro is being defined.

\yskip\hang|matching|, means that a subfile shouldn't end now because a
macro is being used and we are searching for the end of its arguments.

\yskip\hang|aligning|, means that a subfile shouldn't end now because we are
not finished with the preamble of an \.{\\halign} or \.{\\valign}.

\yskip\hang|absorbing|, means that a subfile shouldn't end now because we are
reading a balanced token list for \.{\\message}, \.{\\write}, etc.

\yskip\noindent
If the |scanner_status| is not |normal|, the variable |warning_index| points
to the |eqtb| location for the relevant control sequence name to print
in an error message.
*/

typedef enum {
    skipping = 1,               /* |scanner_status| when passing conditional text */
    defining = 2,               /* |scanner_status| when reading a macro definition */
    matching = 3,               /* |scanner_status| when reading macro arguments */
    aligning = 4,               /* |scanner_status| when reading an alignment preamble */
    absorbing = 5,              /* |scanner_status| when reading a balanced text */
} scanner_states;

extern int scanner_status;
extern pointer warning_index;
extern pointer def_ref;

extern void runaway(void);

/*
However, the discussion about input state really applies only to the
case that we are inputting from a file. There is another important case,
namely when we are currently getting input from a token list. In this case
|state=token_list|, and the conventions about the other state variables
are different:

\yskip\hang|loc| is a pointer to the current node in the token list, i.e.,
the node that will be read next. If |loc=null|, the token list has been
fully read.

\yskip\hang|start| points to the first node of the token list; this node
may or may not contain a reference count, depending on the type of token
list involved.

\yskip\hang|token_type|, which takes the place of |index| in the
discussion above, is a code number that explains what kind of token list
is being scanned.

\yskip\hang|name| points to the |eqtb| address of the control sequence
being expanded, if the current token list is a macro.

\yskip\hang|param_start|, which takes the place of |limit|, tells where
the parameters of the current macro begin in the |param_stack|, if the
current token list is a macro.

\yskip\noindent The |token_type| can take several values, depending on
where the current token list came from:

\yskip\hang|parameter|, if a parameter is being scanned;

\hang|u_template|, if the \<u_j> part of an alignment
template is being scanned;

\hang|v_template|, if the \<v_j> part of an alignment
template is being scanned;

\hang|backed_up|, if the token list being scanned has been inserted as
`to be read again'.

\hang|inserted|, if the token list being scanned has been inserted as
the text expansion of a \.{\\count} or similar variable;

\hang|macro|, if a user-defined control sequence is being scanned;

\hang|output_text|, if an \.{\\output} routine is being scanned;

\hang|every_par_text|, if the text of \.{\\everypar} is being scanned;

\hang|every_math_text|, if the text of \.{\\everymath} is being scanned;

\hang|every_display_text|, if the text of \.{\\everydisplay} is being scanned;

\hang|every_hbox_text|, if the text of \.{\\everyhbox} is being scanned;

\hang|every_vbox_text|, if the text of \.{\\everyvbox} is being scanned;

\hang|every_job_text|, if the text of \.{\\everyjob} is being scanned;

\hang|every_cr_text|, if the text of \.{\\everycr} is being scanned;

\hang|mark_text|, if the text of a \.{\\mark} is being scanned;

\hang|write_text|, if the text of a \.{\\write} is being scanned.

\yskip\noindent
The codes for |output_text|, |every_par_text|, etc., are equal to a constant
plus the corresponding codes for token list parameters |output_routine_loc|,
|every_par_loc|, etc.

The token list begins with a reference count if and
only if |token_type>=macro|.
@^reference counts@>

Since \eTeX's additional token list parameters precede |toks_base|, the
corresponding token types must precede |write_text|.
*/

#  define token_list 0          /* |state| code when scanning a token list */
#  define token_type iindex     /* type of current token list */
#  define param_start ilimit    /* base of macro parameters in |param_stack| */


typedef enum {
    parameter = 0,              /* |token_type| code for parameter */
    u_template = 1,             /* |token_type| code for \<u_j> template */
    v_template = 2,             /* |token_type| code for \<v_j> template */
    backed_up = 3,              /* |token_type| code for text to be reread */
    inserted = 4,               /* |token_type| code for inserted texts */
    macro = 5,                  /* |token_type| code for defined control sequences */
    output_text = 6,            /* |token_type| code for output routines */
    every_par_text = 7,         /* |token_type| code for \.{\\everypar} */
    every_math_text = 8,        /* |token_type| code for \.{\\everymath} */
    every_display_text = 9,     /* |token_type| code for \.{\\everydisplay} */
    every_hbox_text = 10,       /* |token_type| code for \.{\\everyhbox} */
    every_vbox_text = 11,       /* |token_type| code for \.{\\everyvbox} */
    every_job_text = 12,        /* |token_type| code for \.{\\everyjob} */
    every_cr_text = 13,         /* |token_type| code for \.{\\everycr} */
    mark_text = 14,             /* |token_type| code for \.{\\topmark}, etc. */
    every_eof_text = 15,        /* |token_type| code for \.{\\everyeof} */
    write_text = 16,            /* |token_type| code for \.{\\write} */
    local_text = 17,            /* |token_type| code for special purposed */
} token_types;

extern pointer *param_stack;
extern int param_ptr;
extern int max_param_stack;

extern int align_state;

extern int base_ptr;

extern void show_context(void);
extern void set_trick_count(void);

#  define back_list(A) begin_token_list(A,backed_up)    /* backs up a simple token list */
#  define ins_list(A) begin_token_list(A,inserted)      /* inserts a simple token list */

extern void begin_token_list(halfword p, quarterword t);
extern void end_token_list(void);
extern void back_input(void);
extern void reinsert_token(boolean a, halfword pp);
extern void begin_file_reading(void);
extern void end_file_reading(void);
extern void clear_for_error_prompt(void);

extern void initialize_inputstack(void);

extern halfword pseudo_files;
extern void pseudo_from_string(void);
extern void pseudo_start(void);
extern void lua_string_start(void);
extern boolean pseudo_input(void);
extern void pseudo_close(void);


#endif
