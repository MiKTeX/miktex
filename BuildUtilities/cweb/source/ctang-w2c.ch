% Kpathsea changes for CWEB by Wlodek Bzyl and Olaf Weber
% Copyright 2002 Wlodek Bzyl and Olaf Weber
% This file is in the Public Domain.

@x l.27
\def\title{CTANGLE (Version 3.64)}
\def\topofcontents{\null\vfill
  \centerline{\titlefont The {\ttitlefont CTANGLE} processor}
  \vskip 15pt
  \centerline{(Version 3.64)}
  \vfill}
@y
\def\title{CTANGLE (Version 3.64k)}
\def\topofcontents{\null\vfill
  \centerline{\titlefont The {\ttitlefont CTANGLE} processor}
  \vskip 15pt
  \centerline{(Version 3.64k)}
  \vfill}
@z

This change can not be applied when `tie' is  used
(TOC file can not be typeset).

%@x l.48
%\let\maybe=\iftrue
%@y
%\let\maybe=\iffalse % print only sections that change
%@z


Section 1.

@x l.49
The ``banner line'' defined here should be changed whenever \.{CTANGLE}
is modified.

@d banner "This is CTANGLE (Version 3.64)\n"
@y
The ``banner line'' defined here should be changed whenever \.{CTANGLE}
is modified.

@d banner "This is CTANGLE, Version 3.64"
@z


Section 2.

@x l.72
@ We predeclare several standard system functions here instead of including
their system header files, because the names of the header files are not as
standard as the names of the functions. (For example, some \CEE/ environments
have \.{<string.h>} where others have \.{<strings.h>}.)

@<Predecl...@>=
extern int strlen(); /* length of string */
extern int strcmp(); /* compare strings lexicographically */
extern char* strcpy(); /* copy one string to another */
extern int strncmp(); /* compare up to $n$ string characters */
extern char* strncpy(); /* copy up to $n$ string characters */
@y
@z

Section 3.

@x l.92
int main (ac, av)
int ac;
char **av;
@y
#if defined(MIKTEX)
#  define main MIKTEXCEECALL Main
#endif
int main (int ac, char **av)
@z

@x l.100 - Add Web2C version to banner.
  if (show_banner) printf(banner); /* print a ``banner line'' */
@y
  if (show_banner) {
#if defined(MIKTEX)
    printf("%s\n", banner); /* print a ``banner line'' */
#else
    printf("%s%s\n", banner, versionstring); /* print a ``banner line'' */
#endif
  }
@z

@x l.112
@d max_bytes 90000 /* the number of bytes in identifiers,
@y
@d max_bytes 1000000 /* the number of bytes in identifiers,
@z

@x l.114
@d max_toks 270000 /* number of bytes in compressed \CEE/ code */
@d max_names 4000 /* number of identifiers, strings, section names;
@y
@d max_toks 1000000 /* number of bytes in compressed \CEE/ code */
@d max_names 10239 /* number of identifiers, strings, section names;
@z

@x l.117
@d max_texts 2500 /* number of replacement texts, must be less than 10240 */
@d hash_size 353 /* should be prime; used in |"common.w"| */
@y
@d max_texts 10239 /* number of replacement texts, must be less than 10240 */
@d hash_size 8501 /* should be prime; used in |"common.w"| */
@z

@x l.121
@d buf_size 100 /* for \.{CWEAVE} and \.{CTANGLE} */
@y
@d buf_size 1000 /* for \.{CWEAVE} and \.{CTANGLE} */
@z

Section 5.

@x common.h l.30 - boolean comes from kpathsea.
typedef short boolean;
@y
@z

Section 6.

@x common.h l.36
#include <stdio.h>
@y
#include <kpathsea/kpathsea.h>
#include <stdio.h>
#if defined(MIKTEX)
#  include <miktex/W2C/Emulation.h>
#endif
@z

@x common.h l.65
@d xisalpha(c) (isalpha(c)&&((eight_bits)c<0200))
@d xisdigit(c) (isdigit(c)&&((eight_bits)c<0200))
@d xisspace(c) (isspace(c)&&((eight_bits)c<0200))
@d xislower(c) (islower(c)&&((eight_bits)c<0200))
@d xisupper(c) (isupper(c)&&((eight_bits)c<0200))
@d xisxdigit(c) (isxdigit(c)&&((eight_bits)c<0200))
@y
@d xisalpha(c) (isalpha((eight_bits)c)&&((eight_bits)c<0200))
@d xisdigit(c) (isdigit((eight_bits)c)&&((eight_bits)c<0200))
@d xisspace(c) (isspace((eight_bits)c)&&((eight_bits)c<0200))
@d xislower(c) (islower((eight_bits)c)&&((eight_bits)c<0200))
@d xisupper(c) (isupper((eight_bits)c)&&((eight_bits)c<0200))
@d xisxdigit(c) (isxdigit((eight_bits)c)&&((eight_bits)c<0200))
@z

Section 9.

@x common.h l.109 - protos now all in cweb.h.
extern name_pointer id_lookup(); /* looks up a string in the identifier table */
extern name_pointer section_lookup(); /* finds section name */
extern void print_section_name(), sprint_section_name();
@y
#include "cweb.h"
@z

Section 10.

@x common.h l.123 - explicit types, protos now all in cweb.h.
extern history; /* indicates how bad this run was */
extern err_print(); /* print error message and context */
extern wrap_up(); /* indicate |history| and exit */
extern void fatal(); /* issue error message and die */
extern void overflow(); /* succumb because a table has overflowed */
@y
extern int history; /* indicates how bad this run was */
@z

Section 11.

@x common.h l.131 - max_file_name_length is way too small.
@d max_file_name_length 60
@y
@d max_file_name_length 1024
@z

@x common.h l.138 - explicit types.
extern include_depth; /* current level of nesting */
@y
extern int include_depth; /* current level of nesting */
@z

@x common.h l.148 - explicit types.
extern line[]; /* number of current line in the stacked files */
extern change_line; /* number of current line in change file */
@y
extern int line[]; /* number of current line in the stacked files */
extern int change_line; /* number of current line in change file */
@z

@x common.h l.153 - protos now all in cweb.h.
extern reset_input(); /* initialize to read the web file and change file */
extern get_line(); /* inputs the next line */
extern check_complete(); /* checks that all changes were picked up */
@y
@z

Section 15.

@x common.h l.192 - protos now all in cweb.h.
extern void common_init();
@y
@z

Section 21.

@x l.184
int names_match(p,first,l)
name_pointer p; /* points to the proposed match */
char *first; /* position of first character of string */
int l; /* length of identifier */
@y
int names_match (name_pointer p, const char *first, int l, char t)
@z

Section 22.

@x l.200
init_node(node)
name_pointer node;
@y
init_node (name_pointer node)
@z

@x l.207
init_p() {}
@y
init_p (name_pointer p, char t) {}
@z

Section 26.

@x l.261
void
store_two_bytes(x)
sixteen_bits x;
@y
static void
store_two_bytes (sixteen_bits x)
@z

Section 30.

@x l.337
void
push_level(p) /* suspends the current level */
name_pointer p;
@y
static void
push_level (name_pointer p) /* suspends the current level */
@z

Section 31.

@x l.356
void
pop_level(flag) /* do this when |cur_byte| reaches |cur_end| */
int flag; /* |flag==0| means we are in |output_defs| */
@y
static void
pop_level (int flag) /* do this when |cur_byte| reaches |cur_end| */
@z

Section 33.

@x l.392
void
get_output() /* sends next token to |out_char| */
@y
static void
get_output (void) /* sends next token to |out_char| */
@z

Section 37.

@x l.482
void
flush_buffer() /* writes one line to output file */
@y
static void
flush_buffer (void) /* writes one line to output file */
@z

Section 41.

@x l.534
void phase_two();
@y
static void phase_two (void);
@z

Section 42.

@x l.537
void
phase_two () {
@y
static void
phase_two (void) {
@z

Section 43.

@x l.577 Use binary mode for output files
    C_file=fopen(output_file_name,"w");
@y
    C_file=fopen(output_file_name,"wb");
@z

Section 46.

@x l.603
void output_defs();
@y
static void output_defs (void);
@z

Section 47.

@x l.606
void
output_defs()
@y
static void
output_defs (void)
@z

Section 48.

@x l.649
static void out_char();
@y
static void out_char (eight_bits);
@z

Section 49.

@x l.653
out_char(cur_char)
eight_bits cur_char;
@y
out_char (eight_bits cur_char)
@z

@x
    sixteen_bits a;
    a=0400* *cur_byte++;
    a+=*cur_byte++; /* gets the line number */
    C_printf("\n#line %d \"",a);
@:line}{\.{\#line}@>
    cur_val=*cur_byte++;
    cur_val=0400*(cur_val-0200)+ *cur_byte++; /* points to the file name */
    for (j=(cur_val+name_dir)->byte_start, k=(cur_val+name_dir+1)->byte_start;
         j<k; j++) {
      if (*j=='\\' || *j=='"') C_putc('\\');
      C_putc(*j);
    }
    C_printf("%s","\"\n");
@y
    sixteen_bits a;
    a=0400* *cur_byte++;
    a+=*cur_byte++; /* gets the line number */
#if !defined(MIKTEX)
    C_printf("\n#line %d \"",a);
#endif
@:line}{\.{\#line}@>
    cur_val=*cur_byte++;
    cur_val=0400*(cur_val-0200)+ *cur_byte++; /* points to the file name */
    for (j=(cur_val+name_dir)->byte_start, k=(cur_val+name_dir+1)->byte_start;
         j<k; j++) {
#if !defined(MIKTEX)
      if (*j=='\\' || *j=='"') C_putc('\\');
      C_putc(*j);
#endif
    }
#if defined(MIKTEX)
    C_putc('\n');
#else
    C_printf("%s","\"\n");
#endif
@z

Section 58.

@x l.814
eight_bits
skip_ahead() /* skip to next control code */
@y
static eight_bits
skip_ahead (void) /* skip to next control code */
@z

Section 60.

@x l.849
int skip_comment(is_long_comment) /* skips over comments */
boolean is_long_comment;
@y
static int
skip_comment (boolean is_long_comment) /* skips over comments */
@z

Section 62

@x l.889 - add declaration of versionstring.
#include <ctype.h> /* definition of |isalpha|, |isdigit| and so on */
@y
@z

Section 63.

@x l.901
eight_bits
get_next() /* produces the next input token */
@y
static eight_bits
get_next (void) /* produces the next input token */
@z

Section 63.

@x l.954
    else if (*loc=='>') if (*(loc+1)=='*') {loc++; compress(minus_gt_ast);}
                        else compress(minus_gt); break;
@y
    else if (*loc=='>') {if (*(loc+1)=='*') {loc++; compress(minus_gt_ast);}
                         else compress(minus_gt);} break;
@z

@x l.974
  while (isalpha(*++loc) || isdigit(*loc) || isxalpha(*loc) || ishigh(*loc));
@y
  while (isalpha((unsigned char)*++loc) || isdigit((unsigned char)*loc) || isxalpha(*loc) || ishigh(*loc));
@z

Section 76.

@x l.1200
void
scan_repl(t) /* creates a replacement text */
eight_bits t;
@y
static void
scan_repl (eight_bits t) /* creates a replacement text */
@z

Section 77.

@x l.1232 -- rename local var, not to shadow previous local
{int a=id_lookup(id_first,id_loc,0)-name_dir; app_repl((a / 0400)+0200);
  app_repl(a % 0400);}
@y
{int a_l=id_lookup(id_first,id_loc,0)-name_dir; app_repl((a_l / 0400)+0200);
  app_repl(a_l % 0400);}
@z

Section 82.

@x l.1320
        c=toupper(*id_first)-'A'+10;
@y
        c=toupper((unsigned char)*id_first)-'A'+10;
@z
@x l.1325
        c=16*c+toupper(*id_first)-'A'+10;
@y
        c=16*c+toupper((unsigned char)*id_first)-'A'+10;
@z

Section 83.

@x l.1358
void
scan_section()
@y
static void
scan_section (void)
@z

Section 90.

@x l.1458
void phase_one();
@y
static void phase_one (void);
@z

Section 91.

@x l.1461
void
phase_one() {
@y
static void
phase_one (void) {
@z

Section 92.

@x l.1476
void skip_limbo();
@y
static void skip_limbo (void);
@z

Section 93.

@x l.1479
void
skip_limbo()
@y
static void
skip_limbo (void)
@z

Section 95.

@x l.1535
print_stats() {
@y
print_stats (void) {
@z
