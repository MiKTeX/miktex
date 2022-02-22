% This file is part of CWEB.
% This program by Silvio Levy and Donald E. Knuth
% is based on a program by Knuth.
% It is distributed WITHOUT ANY WARRANTY, express or implied.
% Version 4.7 --- February 2022 (works also with later versions)

% Copyright (C) 1987,1990,1993 Silvio Levy and Donald E. Knuth

% Permission is granted to make and distribute verbatim copies of this
% document provided that the copyright notice and this permission notice
% are preserved on all copies.

% Permission is granted to copy and distribute modified versions of this
% document under the conditions for verbatim copying, provided that the
% entire resulting derived work is given a different name and distributed
% under the terms of a permission notice identical to this one.

% Amendments to 'common.h' resulting in this updated version were created
% by numerous collaborators over the course of many years.

% Please send comments, suggestions, etc. to tex-k@@tug.org.

% The next few sections contain stuff from the file |"common.w"| that has
% to be included in both |"ctangle.w"| and |"cweave.w"|. It appears in this
% file |"common.h"|, which is also included in |"common.w"| to propagate
% possible changes from this single source consistently.

First comes general stuff:

@i iso_types.w


@s boolean bool
@<Common code...@>=
typedef uint8_t eight_bits;
typedef uint16_t sixteen_bits;
typedef enum {
  @!ctangle, @!cweave, @!ctwill
} cweb;
extern cweb program; /* \.{CTANGLE} or \.{CWEAVE} or \.{CTWILL}? */
extern int phase; /* which phase are we in? */

@ You may have noticed that almost all \.{"strings"} in the \.{CWEB} sources
are placed in the context of the `|_|'~macro.  This is just a shortcut for the
`|@!gettext|' function from the ``GNU~gettext utilities.'' For systems that do
not have this library installed, we wrap things for neutral behavior without
internationalization.
For backward compatibility with pre-{\mc ANSI} compilers, we replace the
``standard'' header file `\.{stdbool.h}' with the
{\mc KPATHSEA\spacefactor1000} interface `\.{simpletypes.h}'.

@d _(s) gettext(s)

@<Include files@>=
#if defined(MIKTEX)
#if defined(MIKTEX_WINDIWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include <miktex/ExitThrows>
#endif
#include <ctype.h> /* definition of |@!isalpha|, |@!isdigit| and so on */
#include <kpathsea/simpletypes.h> /* |@!boolean|, |@!true| and |@!false| */
#include <stddef.h> /* definition of |@!ptrdiff_t| */
#include <stdint.h> /* definition of |@!uint8_t| and |@!uint16_t| */
#include <stdio.h> /* definition of |@!printf| and friends */
#include <stdlib.h> /* definition of |@!getenv| and |@!exit| */
#include <string.h> /* definition of |@!strlen|, |@!strcmp| and so on */
@#
#ifndef HAVE_GETTEXT
#define HAVE_GETTEXT 0
#endif
@#
#if HAVE_GETTEXT
#include <libintl.h>
#else
#define gettext(a) a
#endif

@ Code related to the character set:
@^ASCII code dependencies@>

@d and_and 04 /* `\.{\&\&}'\,; corresponds to MIT's {\tentex\char'4} */
@d lt_lt 020 /* `\.{<<}'\,; corresponds to MIT's {\tentex\char'20} */
@d gt_gt 021 /* `\.{>>}'\,; corresponds to MIT's {\tentex\char'21} */
@d plus_plus 013 /* `\.{++}'\,; corresponds to MIT's {\tentex\char'13} */
@d minus_minus 01 /* `\.{--}'\,; corresponds to MIT's {\tentex\char'1} */
@d minus_gt 031 /* `\.{->}'\,; corresponds to MIT's {\tentex\char'31} */
@d non_eq 032 /* `\.{!=}'\,; corresponds to MIT's {\tentex\char'32} */
@d lt_eq 034 /* `\.{<=}'\,; corresponds to MIT's {\tentex\char'34} */
@d gt_eq 035 /* `\.{>=}'\,; corresponds to MIT's {\tentex\char'35} */
@d eq_eq 036 /* `\.{==}'\,; corresponds to MIT's {\tentex\char'36} */
@d or_or 037 /* `\.{\v\v}'\,; corresponds to MIT's {\tentex\char'37} */
@d dot_dot_dot 016 /* `\.{...}'\,; corresponds to MIT's {\tentex\char'16} */
@d colon_colon 06 /* `\.{::}'\,; corresponds to MIT's {\tentex\char'6} */
@d period_ast 026 /* `\.{.*}'\,; corresponds to MIT's {\tentex\char'26} */
@d minus_gt_ast 027 /* `\.{->*}'\,; corresponds to MIT's {\tentex\char'27} */
@#
@d compress(c) if (loc++<=limit) return c

@<Common code...@>=
extern char section_text[]; /* text being sought for */
extern char *section_text_end; /* end of |section_text| */
extern char *id_first; /* where the current identifier begins in the buffer */
extern char *id_loc; /* just after the current identifier in the buffer */

@ Code related to input routines:
@d xisalpha(c) (isalpha((int)(c))&&((eight_bits)(c)<0200))
@d xisdigit(c) (isdigit((int)(c))&&((eight_bits)(c)<0200))
@d xisspace(c) (isspace((int)(c))&&((eight_bits)(c)<0200))
@d xislower(c) (islower((int)(c))&&((eight_bits)(c)<0200))
@d xisupper(c) (isupper((int)(c))&&((eight_bits)(c)<0200))
@d xisxdigit(c) (isxdigit((int)(c))&&((eight_bits)(c)<0200))
@d isxalpha(c) ((c)=='_' || (c)=='$')
  /* non-alpha characters allowed in identifier */
@d ishigh(c) ((eight_bits)(c)>0177)
@^high-bit character handling@>

@<Common code...@>=
extern char buffer[]; /* where each line of input goes */
extern char *buffer_end; /* end of |buffer| */
extern char *loc; /* points to the next character to be read from the buffer */
extern char *limit; /* points to the last character in the buffer */

@ Code related to file handling:
@f line x /* make |line| an unreserved word */
@d max_include_depth 10 /* maximum number of source files open
  simultaneously, not counting the change file */
@d max_file_name_length 1024
@d cur_file file[include_depth] /* current file */
@d cur_file_name file_name[include_depth] /* current file name */
@d cur_line line[include_depth] /* number of current line in current file */
@d web_file file[0] /* main source file */
@d web_file_name file_name[0] /* main source file name */

@<Common code...@>=
extern int include_depth; /* current level of nesting */
extern FILE *file[]; /* stack of non-change files */
extern FILE *change_file; /* change file */
extern char file_name[][max_file_name_length];
  /* stack of non-change file names */
extern char change_file_name[]; /* name of change file */
extern char check_file_name[]; /* name of |check_file| */
extern int line[]; /* number of current line in the stacked files */
extern int change_line; /* number of current line in change file */
extern int change_depth; /* where \.{@@y} originated during a change */
extern boolean input_has_ended; /* if there is no more input */
extern boolean changing; /* if the current line is from |change_file| */
extern boolean web_file_open; /* if the web file is being read */

@ @<Predecl...@>=
extern boolean get_line(void); /* inputs the next line */
extern void check_complete(void); /* checks that all changes were picked up */
extern void reset_input(void); /* initialize to read the web file and change file */

@ Code related to section numbers:
@<Common code...@>=
extern sixteen_bits section_count; /* the current section number */
extern boolean changed_section[]; /* is the section changed? */
extern boolean change_pending; /* is a decision about change still unclear? */
extern boolean print_where; /* tells \.{CTANGLE} to print line and file info */

@ Code related to identifier and section name storage:
@d length(c) (size_t)((c+1)->byte_start-(c)->byte_start) /* the length of a name */
@d print_id(c) term_write((c)->byte_start,length(c)) /* print identifier */
@d llink link /* left link in binary search tree for section names */
@d rlink dummy.Rlink /* right link in binary search tree for section names */
@d root name_dir->rlink /* the root of the binary search tree
  for section names */

@<Common code...@>=
typedef struct name_info {
  char *byte_start; /* beginning of the name in |byte_mem| */
  struct name_info *link;
  union {
    struct name_info *Rlink; /* right link in binary search tree for section
      names */
    char Ilk; /* used by identifiers in \.{CWEAVE} only */
  } dummy;
  void *equiv_or_xref; /* info corresponding to names */
} name_info; /* contains information about an identifier or section name */
typedef name_info *name_pointer; /* pointer into array of \&{name\_info}s */
typedef name_pointer *hash_pointer;
extern char byte_mem[]; /* characters of names */
extern char *byte_mem_end; /* end of |byte_mem| */
extern char *byte_ptr; /* first unused position in |byte_mem| */
extern name_info name_dir[]; /* information about names */
extern name_pointer name_dir_end; /* end of |name_dir| */
extern name_pointer name_ptr; /* first unused position in |name_dir| */
extern name_pointer hash[]; /* heads of hash lists */
extern hash_pointer hash_end; /* end of |hash| */
extern hash_pointer h; /* index into hash-head array */

@ @<Predecl...@>=
extern boolean names_match(name_pointer,const char *,size_t,eight_bits);@/
extern name_pointer id_lookup(const char *,const char *,eight_bits);
   /* looks up a string in the identifier table */
extern name_pointer section_lookup(char *,char *,boolean); /* finds section name */
extern void init_node(name_pointer);@/
extern void init_p(name_pointer,eight_bits);@/
extern void print_prefix_name(name_pointer);@/
extern void print_section_name(name_pointer);@/
extern void sprint_section_name(char *,name_pointer);

@ Code related to error handling:
@d spotless 0 /* |history| value for normal jobs */
@d harmless_message 1 /* |history| value when non-serious info was printed */
@d error_message 2 /* |history| value when an error was noted */
@d fatal_message 3 /* |history| value when we had to stop prematurely */
@d mark_harmless if (history==spotless) history=harmless_message
@d mark_error history=error_message
@d confusion(s) fatal(_("! This can't happen: "),s)
@.This can't happen@>

@<Common code...@>=
extern int history; /* indicates how bad this run was */

@ @<Predecl...@>=
extern int wrap_up(void); /* indicate |history| and exit */
extern void err_print(const char *); /* print error message and context */
extern void fatal(const char *,const char *); /* issue error message and die */
extern void overflow(const char *); /* succumb because a table has overflowed */

@ Code related to command line arguments:
@d show_banner flags['b'] /* should the banner line be printed? */
@d show_progress flags['p'] /* should progress reports be printed? */
@d show_happiness flags['h'] /* should lack of errors be announced? */
@d show_stats flags['s'] /* should statistics be printed at end of run? */
@d make_xrefs flags['x'] /* should cross references be output? */
@d check_for_change flags['c'] /* check temporary output for changes */

@<Common code...@>=
extern int argc; /* copy of |ac| parameter to |main| */
extern char **argv; /* copy of |av| parameter to |main| */
extern char C_file_name[]; /* name of |C_file| */
extern char tex_file_name[]; /* name of |tex_file| */
extern char idx_file_name[]; /* name of |idx_file| */
extern char scn_file_name[]; /* name of |scn_file| */
extern boolean flags[]; /* an option for each 7-bit code */
extern const char *use_language; /* prefix to \.{cwebmac.tex} in \TEX/ output */

@ Code related to output:
@d update_terminal fflush(stdout) /* empty the terminal output buffer */
@d new_line putchar('\n')
@d term_write(a,b) fflush(stdout),fwrite(a,sizeof(char),b,stdout)

@<Common code...@>=
extern FILE *C_file; /* where output of \.{CTANGLE} goes */
extern FILE *tex_file; /* where output of \.{CWEAVE} goes */
extern FILE *idx_file; /* where index from \.{CWEAVE} goes */
extern FILE *scn_file; /* where list of sections from \.{CWEAVE} goes */
extern FILE *active_file; /* currently active file for \.{CWEAVE} output */
extern FILE *check_file; /* temporary output file */

@ The procedure that gets everything rolling:
@<Predecl...@>=
extern void common_init(void);@/
extern void print_stats(void);@/
extern void cb_show_banner(void);

@ The following parameters are sufficient to handle \TEX/ (converted to
\.{CWEB}), so they should be sufficient for most applications of \.{CWEB}.

@d buf_size 1000 /* maximum length of input line, plus one */
@d longest_name 10000 /* file names, section names, and section texts
   shouldn't be longer than this */
@d long_buf_size (buf_size+longest_name) /* for \.{CWEAVE} */
@d max_bytes 1000000 /* the number of bytes in identifiers,
  index entries, and section names; must be less than $2^{24}$ */
@d max_names 10239 /* number of identifiers, strings, section names;
  must be less than 10240 */
@d max_sections 4000 /* greater than the total number of sections */

@ End of \.{COMMON} interface.
