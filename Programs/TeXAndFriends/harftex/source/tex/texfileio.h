/* texfileio.h
   
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


#ifndef TEXFILEIO_H
#  define TEXFILEIO_H

extern int *input_file_callback_id;
extern int read_file_callback_id[17];

extern char *luatex_find_read_file(const char *s, int n, int callback_index);
extern boolean luatex_open_input(FILE ** f_ptr, const char *fn, int filefmt,
                                 const_string fopen_mode, boolean must_exist);
extern boolean luatex_open_output(FILE ** f_ptr, const char *fn,
                                  const_string fopen_mode);

extern boolean lua_a_open_in(alpha_file * f, char *fn, int n);
extern boolean lua_a_open_out(alpha_file * f, char *fn, int n);
extern boolean lua_b_open_out(alpha_file * f, char *fn);
extern void lua_a_close_in(alpha_file f, int n);
extern void lua_a_close_out(alpha_file f);

extern packed_ASCII_code *buffer;
extern int first;
extern int last;
extern int max_buf_stack;

extern boolean lua_input_ln(alpha_file f, int n, boolean bypass_eoln);

/*
The user's terminal acts essentially like other files of text, except
that it is used both for input and for output. When the terminal is
considered an input file, the file variable is called |term_in|, and when it
is considered an output file the file variable is |term_out|.
@^system dependencies@>
*/

#  define term_in stdin         /* the terminal as an input file */
#  define term_out stdout       /* the terminal as an output file */


/*
Here is how to open the terminal files.  |t_open_out| does nothing.
|t_open_in|, on the other hand, does the work of ``rescanning,'' or getting
any command line arguments the user has provided.  It's defined in C.
*/

#  define t_open_out()          /*  output already open for text output */

/*
Sometimes it is necessary to synchronize the input/output mixture that
happens on the user's terminal, and three system-dependent
procedures are used for this
purpose. The first of these, |update_terminal|, is called when we want
to make sure that everything we have output to the terminal so far has
actually left the computer's internal buffers and been sent.
The second, |clear_terminal|, is called when we wish to cancel any
input that the user may have typed ahead (since we are about to
issue an unexpected error message). The third, |wake_up_terminal|,
is supposed to revive the terminal if the user has disabled it by
some instruction to the operating system.  The following macros show how
these operations can be specified with {\mc UNIX}.  |update_terminal|
does an |fflush|. |clear_terminal| is redefined
to do nothing, since the user should control the terminal.
@^system dependencies@>
*/

#  define update_terminal() fflush (term_out)
#  define clear_terminal() do { ; } while (0)
#  define wake_up_terminal() do { ; } while (0) /* cancel the user's cancellation of output */

extern boolean init_terminal(void);
extern void term_input(void);

extern str_number cur_name;
extern str_number cur_area;
extern str_number cur_ext;
extern pool_pointer area_delimiter;
extern pool_pointer ext_delimiter;

extern char *pack_file_name(str_number n, str_number a, str_number e);

#  define file_name_size 512

#  define format_area_length 0  /* length of its area part */
#  define format_ext_length 4   /* length of its `\.{.fmt}' part */
#  define format_extension ".fmt"
                                /* the extension, as a constant */

extern char *TEX_format_default;

extern char *open_fmt_file(void);

extern boolean name_in_progress;        /* is a file name being scanned? */
extern str_number job_name;     /* principal file name */
extern boolean log_opened_global;      /* has the transcript file been opened? */

extern unsigned char *texmf_log_name;   /* full name of the log file */

extern void open_log_file(void);
extern char *get_full_log_name (void);
extern void start_input(void);

extern int open_outfile(FILE ** f, const char *name, const char *mode);

extern boolean zopen_w_input(FILE **, const char *, int,
                             const_string fopen_mode);
extern boolean zopen_w_output(FILE **, const char *, const_string fopen_mode);
extern void zwclose(FILE *);

#  define read_tfm_file  readbinfile
#  define read_vf_file   readbinfile
#  define read_data_file readbinfile

extern boolean openinnameok(const_string);
extern boolean openoutnameok(const_string);

#  ifdef WIN32
extern FILE *Poptr;
#  endif

extern boolean open_in_or_pipe(FILE **, char *, int, const_string fopen_mode,
                               boolean must_exist);
extern boolean open_out_or_pipe(FILE **, char *, const_string fopen_mode);
extern void close_file_or_pipe(FILE *);

extern char *luatex_synctex_get_current_name(void);

#endif
