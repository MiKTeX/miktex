% Kpathsea changes for CWEB by Wlodek Bzyl and Olaf Weber
% Copyright 2002 Wlodek Bzyl and Olaf Weber
% This file is in the Public Domain.


@x l.32
\def\title{CWEAVE (Version 3.64)}
\def\topofcontents{\null\vfill
  \centerline{\titlefont The {\ttitlefont CWEAVE} processor}
  \vskip 15pt
  \centerline{(Version 3.64)}
  \vfill}
@y
\def\Kpathsea/{{\mc KPATHSEA\spacefactor1000}}
\def\SQUARE{\vrule width 2pt depth -1pt height 3pt}
\def\sqitem{\item{\SQUARE}}

\def\title{CWEAVE (Version 3.64k)}
\def\topofcontents{\null\vfill
  \centerline{\titlefont The {\ttitlefont CWEAVE} processor}
  \vskip 15pt
  \centerline{(Version 3.64k)}
  \vfill}
@z

This change can not be applied when `tie' is  used
(TOC file can not be typeset).

%@x l.51
%\let\maybe=\iftrue
%@y
%\let\maybe=\iffalse % print only changed modules
%@z


Section 1.

@x l.65
The ``banner line'' defined here should be changed whenever \.{CWEAVE}
is modified.

@d banner "This is CWEAVE (Version 3.64)\n"
@y 
The ``banner line'' defined here should be changed whenever \.{CWEAVE}
is modified.

@d banner "This is CWEAVE, Version 3.64"
@z


Section 2.

@x l.77
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

@x l.98
int main (ac, av)
int ac; /* argument count */
char **av; /* argument values */
@y
#if defined(MIKTEX)
#  define main MIKTEXCEECALL Main
#endif
int main (int ac, char **av)
@z

@x l.107 - Add Web2C version to banner.
  if (show_banner) printf(banner); /* print a ``banner line'' */
@y
  if (show_banner) {
     printf("%s%s\n", banner, versionstring); /* print a ``banner line'' */
  }
@z

@x l.112 - 'use' print_text(), used nowhere else
  return wrap_up(); /* and exit gracefully */
@y
  if (0) print_text(text_ptr);
  return wrap_up(); /* and exit gracefully */
@z

@x l.120
@d max_bytes 90000 /* the number of bytes in identifiers,
@y
@d max_bytes 1000000 /* the number of bytes in identifiers,
@z

@x l.112
@d max_names 4000 /* number of identifiers, strings, section names;
@y
@d max_names 10239 /* number of identifiers, strings, section names;
@z

@x l.124
@d max_sections 2000 /* greater than the total number of sections */
@d hash_size 353 /* should be prime */
@d buf_size 100 /* maximum length of input line, plus one */
@y
@d max_sections 10239 /* greater than the total number of sections */
@d hash_size 8501 /* should be prime */
@d buf_size 1000 /* maximum length of input line, plus one */
@z

@x l.131
@d max_refs 20000 /* number of cross-references; must be less than 65536 */
@d max_toks 20000 /* number of symbols in \CEE/ texts being parsed;
@y
@d max_refs 65535 /* number of cross-references; must be less than 65536 */
@d max_toks 65535 /* number of symbols in \CEE/ texts being parsed;
@z

@x l.134
@d max_texts 4000 /* number of phrases in \CEE/ texts being parsed;
@y
@d max_texts 10239 /* number of phrases in \CEE/ texts being parsed;
@z

@x l.136
@d max_scraps 2000 /* number of tokens in \CEE/ texts being parsed */
@y
@d max_scraps 10000 /* number of tokens in \CEE/ texts being parsed */
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

@x l.275
void
new_xref(p)
name_pointer p;
@y
static void
new_xref (name_pointer p)
@z

Section 22.

@x l.306
void
new_section_xref(p)
name_pointer p;
@y
static void
new_section_xref (name_pointer p)
@z

Section 23.

@x l.326
void
set_file_flag(p)
name_pointer p;
@y
static void
set_file_flag (name_pointer p)
@z

Section 27.

@x l.371
int names_match(p,first,l,t)
name_pointer p; /* points to the proposed match */
char *first; /* position of first character of string */
int l; /* length of identifier */
eight_bits t; /* desired ilk */
@y
int
names_match (name_pointer p, const char *first, int l, char t)
@z

@x l.383
init_p(p,t)
name_pointer p;
eight_bits t;
@y
init_p (name_pointer p, char t)
@z

@x l.391
init_node(p)
name_pointer p;
@y
init_node (name_pointer p)
@z

Section 34.

@x l.600
void   skip_limbo();
@y
static void skip_limbo (void);
@z

Section 35.

@x l.603
void
skip_limbo() {
@y
static void
skip_limbo (void) {
@z

Section 36.

@x l.625
unsigned
skip_TeX() /* skip past pure \TEX/ code */
@y
static unsigned
skip_TeX (void) /* skip past pure \TEX/ code */
@z

Section 38.

@x l.682 - Add declaration for versionstring.
#include <ctype.h> /* definition of |isalpha|, |isdigit| and so on */
@y
@z

Section 39.

@x l.696
eight_bits get_next();
@y
static eight_bits get_next (void);
@z

Section 40.

@x l.699
eight_bits
get_next() /* produces the next input token */
@y
static eight_bits
get_next (void) /* produces the next input token */
@z

Section 45.

@x l.780
    else if (*loc=='>') if (*(loc+1)=='*') {loc++; compress(minus_gt_ast);}
                        else compress(minus_gt); break;
@y
    else if (*loc=='>') {if (*(loc+1)=='*') {loc++; compress(minus_gt_ast);}
                         else compress(minus_gt);} break;
@z

@x l.800
  while (isalpha(*++loc) || isdigit(*loc) || isxalpha(*loc) || ishigh(*loc));
@y
  while (isalpha((unsigned char)*++loc) || isdigit((unsigned char)*loc) || isxalpha(*loc) || ishigh(*loc));
@z

@x l.835
    *id_loc++='$'; *id_loc++=toupper(*loc); loc++;
@y
    *id_loc++='$'; *id_loc++=toupper((unsigned char)*loc); loc++;
@z

Section 48.

@x l.870
    if (c=='\\') if (loc>=limit) continue;
      else if (++id_loc<=section_text_end) {
        *id_loc = '\\'; c=*loc++;
      }
@y
    if (c=='\\') {
      if (loc>=limit) continue;
      else if (++id_loc<=section_text_end) {
        *id_loc = '\\'; c=*loc++;
      }
    }
@z

Section 55.

@x l.971
void skip_restricted();
@y
static void skip_restricted (void);
@z

Section 56.

@x l.974
void
skip_restricted()
@y
static void
skip_restricted (void)
@z

Section 59.

@x l.1024
void phase_one();
@y
static void phase_one (void);
@z

Section 60.

@x l.1027
void
phase_one() {
@y
static void
phase_one (void) {
@z

Section 62.

@x l.1076
void C_xref();
@y
static void C_xref (eight_bits);
@z

Section 63.

@x l.1080
void
C_xref( spec_ctrl ) /* makes cross-references for \CEE/ identifiers */
  eight_bits spec_ctrl;
@y
static void
C_xref (eight_bits spec_ctrl) /* makes cross-references for \CEE/ identifiers */
@z

Section 64.

@x l.1104
void outer_xref();
@y
static void outer_xref (void);
@z

Section 65.

@x l.1107
void
outer_xref() /* extension of |C_xref| */
@y
static void
outer_xref (void) /* extension of |C_xref| */
@z

Section 74.

@x l.1266
void section_check();
@y
static void section_check (name_pointer);
@z

Section 75.

@x l.1269
void
section_check(p)
name_pointer p; /* print anomalies in subtree |p| */
@y
static void
section_check (name_pointer p) /* print anomalies in subtree |p| */
@z

Section 78.

@x l.1322
void
flush_buffer(b,per_cent,carryover)
char *b;  /* outputs from |out_buf+1| to |b|,where |b<=out_ptr| */
boolean per_cent,carryover;
@y
static void
flush_buffer (char *b, boolean per_cent, boolean carryover)
@z

Section 79.

@x l.1351
void
finish_line() /* do this at the end of a line */
@y
static void
finish_line (void) /* do this at the end of a line */
@z

Section 81.

@x l.1383
void
out_str(s) /* output characters from |s| to end of string */
char *s;
@y
static void
out_str (const char *s) /* output characters from |s| to end of string */
@z

Section 83.

@x l.1402
void break_out();
@y
static void break_out (void);
@z

Section 84.

@x l.1405
void
break_out() /* finds a way to break the output line */
@y
static void
break_out (void) /* finds a way to break the output line */
@z

Section 86.

@x l.1440
void
out_section(n)
sixteen_bits n;
@y
static void
out_section (sixteen_bits n)
@z

Section 87.

@x l.1454
void
out_name(p,quote_xalpha)
name_pointer p;
boolean quote_xalpha;
@y
static void
out_name (name_pointer p, boolean quote_xalpha)
@z

Section 88.

@x l.1484
void
copy_limbo()
@y
static void
copy_limbo (void)
@z

Section 90.

@x l.1519
eight_bits
copy_TeX()
@y
static eight_bits
copy_TeX (void)
@z

Section 91.

@x l.1449
int copy_comment();
@y
static int copy_comment (boolean, int);
@z

Section 92.

@x l.1551
int copy_comment(is_long_comment,bal) /* copies \TEX/ code in comments */
boolean is_long_comment; /* is this a traditional \CEE/ comment? */
int bal; /* brace balance */
@y
static int
copy_comment (boolean is_long_comment, int bal)
@z

Section 93.

@x l.1608
  if (phase==2) app_tok(*(loc++)) else loc++;
@y
  {if (phase==2) app_tok(*(loc++)) else loc++;}
@z

Section 99.

@x l.1783
void
print_cat(c) /* symbolic printout of a category */
eight_bits c;
@y
static void
print_cat (eight_bits c)
@z

Section 106.

@x l.2138
void
print_text(p) /* prints a token list for debugging; not used in |main| */
text_pointer p;
@y
static void
print_text (text_pointer p)
@z

Section 109.

@x l.2263
void
app_str(s)
char *s;
@y
static void
app_str (const char *s)
@z

@x l.2270
void
big_app(a)
token a;
@y
static void
big_app (token a)
@z

@x l.2287
void
big_app1(a)
scrap_pointer a;
@y
static void
big_app1 (scrap_pointer a)
@z

Section 111.

@x l.2414
token_pointer
find_first_ident(p)
text_pointer p;
@y
static token_pointer
find_first_ident (text_pointer p)
@z

Section 112.

@x l.2446
void
make_reserved(p) /* make the first identifier in |p->trans| like |int| */
scrap_pointer p;
@y
static void
make_reserved (scrap_pointer p)
@z

Section 113.

@x l.2477
void
make_underlined(p)
/* underline the entry for the first identifier in |p->trans| */
scrap_pointer p;
@y
static void
make_underlined (scrap_pointer p)
@z

Section 114.

@x l.2495
void  underline_xref();
@y
static void underline_xref (name_pointer);
@z

Section 115.

@x l.2498
void
underline_xref(p)
name_pointer p;
@y
static void
underline_xref (name_pointer p)
@z

Section 164.

@x l.3003
void
reduce(j,k,c,d,n)
scrap_pointer j;
eight_bits c;
short k, d, n;
@y
static void
reduce (scrap_pointer j, short k, eight_bits c, short d, short n)
@z

Section 165.

@x l.3029
void
squash(j,k,c,d,n)
scrap_pointer j;
eight_bits c;
short k, d, n;
@y
static void
squash (scrap_pointer j, short k, eight_bits c, short d, short n)
@z

Section 169.

@x l.3096 -- rename local var, not to shadow param
{ scrap_pointer k; /* pointer into |scrap_info| */
  if (tracing==2) {
    printf("\n%d:",n);
    for (k=scrap_base; k<=lo_ptr; k++) {
      if (k==pp) putxchar('*'); else putxchar(' ');
      if (k->mathness %4 ==  yes_math) putchar('+');
      else if (k->mathness %4 ==  no_math) putchar('-');
      print_cat(k->cat);
      if (k->mathness /4 ==  yes_math) putchar('+');
      else if (k->mathness /4 ==  no_math) putchar('-');
@y
{ scrap_pointer k_l; /* pointer into |scrap_info| */
  if (tracing==2) {
    printf("\n%d:",n);
    for (k_l=scrap_base; k_l<=lo_ptr; k_l++) {
      if (k_l==pp) putxchar('*'); else putxchar(' ');
      if (k_l->mathness %4 ==  yes_math) putchar('+');
      else if (k_l->mathness %4 ==  no_math) putchar('-');
      print_cat(k_l->cat);
      if (k_l->mathness /4 ==  yes_math) putchar('+');
      else if (k_l->mathness /4 ==  no_math) putchar('-');
@z

Section 170.

@x l.3125
text_pointer
translate() /* converts a sequence of scraps */
@y
static text_pointer
translate (void) /* converts a sequence of scraps */
@z

Section 174.

@x l.3190
void
C_parse(spec_ctrl) /* creates scraps from \CEE/ tokens */
  eight_bits spec_ctrl;
@y
static void
C_parse (eight_bits spec_ctrl) /* creates scraps from \CEE/ tokens */
@z

Section 181.

@x l.3421
void app_cur_id();
@y
static void app_cur_id (boolean);
@z

Section 182.

@x l.3424
void
app_cur_id(scrapping)
boolean scrapping; /* are we making this into a scrap? */
@y
static void
app_cur_id (boolean scrapping)
@z

Section 183.

@x l.3449
text_pointer
C_translate()
@y
static text_pointer
C_translate (void)
@z

Section 184.

@x l.3479
void
outer_parse() /* makes scraps from \CEE/ tokens and comments */
@y
static void
outer_parse (void) /* makes scraps from \CEE/ tokens and comments */
@z

Section 189.

@x l.3585
void
push_level(p) /* suspends the current level */
text_pointer p;
@y
static void
push_level (text_pointer p) /* suspends the current level */
@z

Section 190.

@x l.3605
void
pop_level()
@y
static void
pop_level (void)
@z

Section 192.

@x l.3627
eight_bits
get_output() /* returns the next token of output */
@y
static eight_bits
get_output (void) /* returns the next token of output */
@z

Section 193.

@x l.3664
void
output_C() /* outputs the current token list */
@y
static void
output_C (void) /* outputs the current token list */
@z

Section 194.

@x l.3687
void make_output();
@y
static void make_output (void);
@z

Section 195.

@x l.3690
void
make_output() /* outputs the equivalents of tokens */
@y
static void
make_output (void) /* outputs the equivalents of tokens */
@z

Section 202.

@x l.3927
    if (b=='\'' || b=='"')
      if (delim==0) delim=b;
      else if (delim==b) delim=0;
@y
    if (b=='\'' || b=='"') {
      if (delim==0) delim=b;
      else if (delim==b) delim=0;
    }
@z

Section 205.

@x l.3951
void phase_two();
@y
static void phase_two (void);
@z

Section 206.

@x l.3955
void
phase_two() {
@y
static void
phase_two (void) {
@z

Section 212.

@x l.4074
void finish_C();
@y
static void finish_C (boolean);
@z

Section 213.

@x l.4077
void
finish_C(visible) /* finishes a definition or a \CEE/ part */
  boolean visible; /* nonzero if we should produce \TEX/ output */
@y
static void
finish_C (boolean visible) /* finishes a definition or a \CEE/ part */
@z

Section 212.

@x l.4092
        if (*out_ptr=='6') out_ptr-=2;
        else if (*out_ptr=='7') *out_ptr='Y';
@y
      {
        if (*out_ptr=='6') out_ptr-=2;
        else if (*out_ptr=='7') *out_ptr='Y';
      }
@z

Section 221.

@x l.4245
void footnote();
@y
static void footnote (sixteen_bits);
@z

Section 222.

@x l.4248
void
footnote(flag) /* outputs section cross-references */
sixteen_bits flag;
@y
static void
footnote (sixteen_bits flag) /* outputs section cross-references */
@z

Section 225.

@x l.4294
void phase_three();
@y
static void phase_three (void);
@z

Section 226.

@x l.4297
void
phase_three() {
@y
static void
phase_three (void) {
@z

@x l.4309 Use binary mode for output files
  if ((idx_file=fopen(idx_file_name,"w"))==NULL)
@y
  if ((idx_file=fopen(idx_file_name,"wb"))==NULL)
@z

@x l.4324 Use binary mode for output files
  if ((scn_file=fopen(scn_file_name,"w"))==NULL)
@y
  if ((scn_file=fopen(scn_file_name,"wb"))==NULL)
@z

Section 235.

@x l.4440
strcpy(collate+1," \1\2\3\4\5\6\7\10\11\12\13\14\15\16\17");
/* 16 characters + 1 = 17 */
strcpy(collate+17,"\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37");
/* 16 characters + 17 = 33 */
strcpy(collate+33,"!\42#$%&'()*+,-./:;<=>?@@[\\]^`{|}~_");
/* 32 characters + 33 = 65 */
strcpy(collate+65,"abcdefghijklmnopqrstuvwxyz0123456789");
/* (26 + 10) characters + 65 = 101 */
strcpy(collate+101,"\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217");
/* 16 characters + 101 = 117 */
strcpy(collate+117,"\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237");
/* 16 characters + 117 = 133 */
strcpy(collate+133,"\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257");
/* 16 characters + 133 = 149 */
strcpy(collate+149,"\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277");
/* 16 characters + 149 = 165 */
strcpy(collate+165,"\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317");
/* 16 characters + 165 = 181 */
strcpy(collate+181,"\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337");
/* 16 characters + 181 = 197 */
strcpy(collate+197,"\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357");
/* 16 characters + 197 = 213 */
strcpy(collate+213,"\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377");
@y
strcpy((char *)collate+1," \1\2\3\4\5\6\7\10\11\12\13\14\15\16\17");
/* 16 characters + 1 = 17 */
strcpy((char *)collate+17,"\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37");
/* 16 characters + 17 = 33 */
strcpy((char *)collate+33,"!\42#$%&'()*+,-./:;<=>?@@[\\]^`{|}~_");
/* 32 characters + 33 = 65 */
strcpy((char *)collate+65,"abcdefghijklmnopqrstuvwxyz0123456789");
/* (26 + 10) characters + 65 = 101 */
strcpy((char *)collate+101,"\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217");
/* 16 characters + 101 = 117 */
strcpy((char *)collate+117,"\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237");
/* 16 characters + 117 = 133 */
strcpy((char *)collate+133,"\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257");
/* 16 characters + 133 = 149 */
strcpy((char *)collate+149,"\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277");
/* 16 characters + 149 = 165 */
strcpy((char *)collate+165,"\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317");
/* 16 characters + 165 = 181 */
strcpy((char *)collate+181,"\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337");
/* 16 characters + 181 = 197 */
strcpy((char *)collate+197,"\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357");
/* 16 characters + 197 = 213 */
strcpy((char *)collate+213,"\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377");
@z

Section 237.

@x l.4474
void  unbucket();
@y
static void unbucket (eight_bits);
@z

Section 238.

@x l.4477
void
unbucket(d) /* empties buckets having depth |d| */
eight_bits d;
@y
static void
unbucket (eight_bits d) /* empties buckets having depth |d| */
@z

Section 246.

@x l.4594
void section_print();
@y
static void section_print (name_pointer);
@z

Section 247.

@x l.4597
void
section_print(p) /* print all section names in subtree |p| */
name_pointer p;
@y
static void
section_print (name_pointer p) /* print all section names in subtree |p| */
@z

Section 249.

@x l.4620
print_stats() {
@y
print_stats (void) {
@z

Section 250. (added)

@x l.4643 - declare print_text(), so it can be 'used' in main.
@** Index.
@y
@ @<Predecl...@>=
static void print_text (text_pointer p);

@** Index.
@z
