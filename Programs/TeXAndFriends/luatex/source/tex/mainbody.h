/* mainbody.h

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


#ifndef MAINBODY_H
#  define MAINBODY_H

extern int luatex_version;
extern int luatex_revision;
extern const char *luatex_version_string;
extern const char *engine_name;

/*
The following parameters can be changed at compile time to extend or
reduce \TeX's capacity. They may have different values in \.{INITEX} and
in production versions of \TeX.
@.INITEX@>
@^system dependencies@>
*/

#  define ssup_max_strings 2097151

#  define inf_max_strings   100000
#  define sup_max_strings   ssup_max_strings
#  define inf_strings_free   100
#  define sup_strings_free   sup_max_strings

#  define inf_buf_size   500
#  define sup_buf_size   100000000

#  define inf_nest_size   40
#  define sup_nest_size   4000

#  define inf_max_in_open   6
#  define sup_max_in_open   600 /* was 127. now okay for some 512 nested lua calls */

#  define inf_param_size   60
#  define sup_param_size   32767

#  define inf_save_size   600
#  define sup_save_size   500000

#  define inf_stack_size   200
#  define sup_stack_size   50000

#  define inf_dvi_buf_size   800
#  define sup_dvi_buf_size   65536

#  define sup_hash_extra   sup_max_strings
#  define inf_hash_extra   0

#  define inf_expand_depth   100
#  define sup_expand_depth   10000000


#  include <stdio.h>

/* Types in the outer block */
typedef int ASCII_code;         /* characters */
typedef unsigned char eight_bits;       /* unsigned one-byte quantity */
typedef FILE *alpha_file;       /* files that contain textual data */
typedef FILE *byte_file;        /* files that contain binary data */

typedef int str_number;
typedef int pool_pointer;
typedef unsigned char packed_ASCII_code;

typedef int scaled;             /* this type is used for scaled integers */
typedef char small_number;      /* this type is self-explanatory */

typedef float glue_ratio;       /* one-word representation of a glue expansion factor */

typedef unsigned short quarterword;
typedef int halfword;

typedef unsigned char glue_ord; /* infinity to the 0, 1, 2, 3, or 4 power */

typedef unsigned short group_code;      /* |save_level| for a level boundary */

typedef int font_index;         /* index into |font_info| */

typedef int save_pointer;

/*
Characters of text that have been converted to \TeX's internal form
are said to be of type |ASCII_code|, which is a subrange of the integers.

We are assuming that our runtime system is able to read and write UTF-8.

Some of the ASCII codes without visible characters have been given symbolic
names in this program because they are used with a special meaning.
*/


#  define null_code '\0'        /* ASCII code that might disappear */
#  define carriage_return '\r'  /* ASCII code used at end of line */

/* Global variables */
extern boolean luainit;         /* are we using lua for initializations  */
extern boolean tracefilenames;  /* print file open-close  info? */


extern boolean ini_version;     /* are we \.{INITEX}? */
extern boolean dump_option;
extern boolean dump_line;
extern int bound_default;
extern char *bound_name;
extern int error_line;
extern int half_error_line;
extern int max_print_line;
extern int max_strings;
extern int strings_free;
extern int font_k;
extern int buf_size;
extern int stack_size;
extern int max_in_open;
extern int param_size;
extern int nest_size;
extern int save_size;
extern int expand_depth;
extern int parsefirstlinep;
extern int filelineerrorstylep;
extern int haltonerrorp;
extern boolean quoted_filename;

extern int total_pages;
extern int dead_cycles;

/*
In order to make efficient use of storage space, \TeX\ bases its major data
structures on a |memory_word|, which contains either a (signed) integer,
possibly scaled, or a (signed) |glue_ratio|, or a small number of
fields that are one half or one quarter of the size used for storing
integers.

If |x| is a variable of type |memory_word|, it contains up to four
fields that can be referred to as follows:
$$\vbox{\halign{\hfil#&#\hfil&#\hfil\cr
|x|&.|int|&(an |integer|)\cr
|x|&.|sc|\qquad&(a |scaled| integer)\cr
|x|&.|gr|&(a |glue_ratio|)\cr
|x.hh.lh|, |x.hh|&.|rh|&(two halfword fields)\cr
|x.hh.b0|, |x.hh.b1|, |x.hh|&.|rh|&(two quarterword fields, one halfword
  field)\cr
|x.qqqq.b0|, |x.qqqq.b1|, |x.qqqq|&.|b2|, |x.qqqq.b3|\hskip-100pt
  &\qquad\qquad\qquad(four quarterword fields)\cr}}$$
This is somewhat cumbersome to write, and not very readable either, but
macros will be used to make the notation shorter and more transparent.
The \PASCAL\ code below gives a formal definition of |memory_word| and
its subsidiary types, using packed variant records. \TeX\ makes no
assumptions about the relative positions of the fields within a word.

We are assuming 32-bit integers, a halfword must contain at least
32 bits, and a quarterword must contain at least 16 bits.
@^system dependencies@>

N.B.: Valuable memory space will be dreadfully wasted unless \TeX\ is compiled
by a \PASCAL\ that packs all of the |memory_word| variants into
the space of a single integer. This means, for example, that |glue_ratio|
words should be |short_real| instead of |real| on some computers. Some
\PASCAL\ compilers will pack an integer whose subrange is `|0..255|' into
an eight-bit field, but others insist on allocating space for an additional
sign bit; on such systems you can get 256 values into a quarterword only
if the subrange is `|-128..127|'.

The present implementation tries to accommodate as many variations as possible,
so it makes few assumptions. If integers having the subrange
`|min_quarterword..max_quarterword|' can be packed into a quarterword,
and if integers having the subrange `|min_halfword..max_halfword|'
can be packed into a halfword, everything should work satisfactorily.

It is usually most efficient to have |min_quarterword=min_halfword=0|,
so one should try to achieve this unless it causes a severe problem.
The values defined here are recommended for most 32-bit computers.

We cannot use the full range of 32 bits in a halfword, because we have
to allow negative values for potential backend tricks like web2c's
dynamic allocation, and parshapes pointers have to be able to store at
least twice the value |max_halfword| (see below). Therefore,
|max_halfword| is $2^{30}-1$
*/

#  include "tex/memoryword.h"

#  define min_quarterword 0     /*smallest allowable value in a |quarterword| */
#  define max_quarterword 65535 /* largest allowable value in a |quarterword| */
#  define min_halfword  -0x3FFFFFFF     /* smallest allowable value in a |halfword| */
#  define max_halfword 0x3FFFFFFF
                                /* largest allowable value in a |halfword| */


/*
The following procedure, which is called just before \TeX\ initializes its
input and output, establishes the initial values of the date and time.
It calls a macro-defined |dateandtime| routine.  |dateandtime| in turn is
also a C macro, which calls |get_date_and_time|, passing it the addresses of
the day, month, etc., so they can be set by the routine.
|get_date_and_time| also sets up interrupt catching if that
is conditionally compiled in the C code.
@^system dependencies@>
*/

#  define fix_date_and_time() dateandtime(time_par,day_par,month_par,year_par)

extern int get_luatexversion(void);
extern str_number get_luatexrevision(void);

extern int ready_already;

extern void main_body(void);
extern void close_files_and_terminate(void);

extern void final_cleanup(void);
extern void debug_help(void);   /* routine to display various things */

extern int main_initialize(void);

/* lazy me */
#  define get_cur_font() equiv(cur_font_loc)
#  define zset_cur_font set_cur_font


#endif
