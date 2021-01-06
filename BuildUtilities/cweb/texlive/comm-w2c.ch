% Kpathsea changes for CWEB by Wlodek Bzyl and Olaf Weber
% Copyright 2002 Wlodek Bzyl and Olaf Weber
% This file is in the Public Domain.

@x l.20
\def\title{Common code for CTANGLE and CWEAVE (Version 3.64)}
\def\topofcontents{\null\vfill
  \centerline{\titlefont Common code for {\ttitlefont CTANGLE} and
    {\ttitlefont CWEAVE}}
  \vskip 15pt
  \centerline{(Version 3.64)}
  \vfill}
@y
\def\Kpathsea/{{\mc KPATHSEA\spacefactor1000}}
\def\title{Common code for CTANGLE and CWEAVE (Version 3.64k)}
\def\topofcontents{\null\vfill
  \centerline{\titlefont Common code for {\ttitlefont CTANGLE} and
    {\ttitlefont CWEAVE}}
  \vskip 15pt
  \centerline{(Version 3.64k)}
  \vfill}
@z

This change can not be applied when `tie' is  used
(TOC file can not be typeset).

%@x l.42
%\let\maybe=\iftrue
%@y
%\let\maybe=\iffalse % print only changed modules
%@z

Section 1.

@x l.63
@<Predeclaration of procedures@>@/
@y
#include "cweb.h"
@<Predeclaration of procedures@>@/
@z

Section 2. 
We use the definition from `kpathsea/types.h':

  typedef enum { false = 0, true = 1 } boolean;

Note that this definition also occurs in common.h.
@x l.74
typedef short boolean;
@y
@z


Section 4.

@x l.91
common_init()
@y
common_init (void)
@z

@x l.93
  @<Initialize pointers@>;
@y
  @<Initialize pointers@>;
  @<Set up |PROGNAME| feature and initialize the search path mechanism@>;
@z

Section 5.

@x l.103
#include <ctype.h>
@y
#if !defined(MIKTEX)
#define CWEB
#endif
#include "cpascal.h"
#include <ctype.h>
#if defined(MIKTEX)
#define register
#endif
@z

Section 7.

@x l.153
@d buf_size 100 /* for \.{CWEAVE} and \.{CTANGLE} */
@y
@d buf_size 1000 /* for \.{CWEAVE} and \.{CTANGLE} */
@z

@x l.156
@d xisspace(c) (isspace(c)&&((unsigned char)c<0200))
@d xisupper(c) (isupper(c)&&((unsigned char)c<0200))
@y
@d xisspace(c) (isspace((unsigned char)c)&&((unsigned char)c<0200))
@d xisupper(c) (isupper((unsigned char)c)&&((unsigned char)c<0200))
@z

Section 9.

@x l.173
int input_ln(fp) /* copies a line into |buffer| or returns 0 */
FILE *fp; /* what file to read from */
@y
int input_ln (FILE *fp) /* copies a line into |buffer| or returns 0 */
@z

@x l.181
    if ((*(k++) = c) != ' ') limit = k;
@y
    if ((*(k++) = c) != ' ' && c!='\r') limit = k;
@z

Section 10.

@x l.207 - max_file_name_length is way too small.
@d max_file_name_length 60
@y
@d max_file_name_length 1024
@z

@x l.221 - no alt_web_file_name needed.
char alt_web_file_name[max_file_name_length]; /* alternate name to try */
@y
@z

Section 12.

@x l.252
void
prime_the_change_buffer()
@y
static void
prime_the_change_buffer (void)
@z

@x l.271
  if (xisupper(buffer[1])) buffer[1]=tolower(buffer[1]);
@y
  if (xisupper(buffer[1])) buffer[1]=tolower((unsigned char)buffer[1]);
@z

Section 16.

@x l.321
void
check_change() /* switches to |change_file| if the buffers match */
@y
static void
check_change (void) /* switches to |change_file| if the buffers match */
@z

@x l.340
      char xyz_code=xisupper(buffer[1])? tolower(buffer[1]): buffer[1];
@y
      char xyz_code=xisupper(buffer[1])? tolower((unsigned char)buffer[1]): buffer[1];
@z

Section 18.

@x l.380
reset_input()
@y
reset_input (void)
@z

Section 19.

@x l.394
if ((web_file=fopen(web_file_name,"r"))==NULL) {
  strcpy(web_file_name,alt_web_file_name);
  if ((web_file=fopen(web_file_name,"r"))==NULL)
       fatal("! Cannot open input file ", web_file_name);
}
@y
if ((found_filename=kpse_find_cweb(web_file_name))==NULL ||
    (web_file=fopen(found_filename,"r"))==NULL) {
  fatal("! Cannot open input file ", web_file_name);
} else if (strlen(found_filename) < max_file_name_length) {
  strcpy(web_file_name, found_filename);
  free(found_filename);
#if defined(MIKTEX)
  MiKTeX::Util::PathName sourceDir (web_file_name);
  sourceDir.RemoveFileSpec ();
  MiKTeX::Core::Session::Get()->AddInputDirectory(sourceDir, true);
#endif
}
@z

@x l.402
if ((change_file=fopen(change_file_name,"r"))==NULL)
       fatal("! Cannot open change file ", change_file_name);
@y
if ((found_filename=kpse_find_cweb(change_file_name))==NULL ||
    (change_file=fopen(found_filename,"r"))==NULL) {
  fatal("! Cannot open change file ", change_file_name);
} else if (strlen(found_filename) < max_file_name_length) {
  strcpy(change_file_name, found_filename);
  free(found_filename);
}
@z

@x l.415
@d max_sections 2000 /* number of identifiers, strings, section names;
@y
@d max_sections 10239 /* number of identifiers, strings, section names;
@z

Section 21.

@x l.427
int get_line() /* inputs the next line */
@y
int get_line (void) /* inputs the next line */
@z

Section 22.

@x l.472
#include <stdlib.h> /* declaration of |getenv| and |exit| */
@y
#include <kpathsea/kpathsea.h> /* include every \Kpathsea/ header */
#include <stdlib.h> /* declaration of |getenv| and |exit| */
#include "help.h"

@ The \.{ctangle} and \.{cweave} programs from the original \.{CWEB}
package use the compile-time default directory or the value of the
environment variable \.{CWEBINPUTS} as an alternative place to be
searched for files, if they could not be found in the current
directory.

This version uses the \Kpathsea/ mechanism for searching files. 
The directories to be searched for come from three sources:

 (a)~a user-set environment variable \.{CWEBINPUTS}
    (overriden by \.{CWEBINPUTS\_cweb});\par
 (b)~a line in \Kpathsea/ configuration file \.{texmf.cnf},\hfil\break
    e.g. \.{CWEBINPUTS=.:$TEXMF/texmf/cweb//}
    or \.{CWEBINPUTS.cweb=.:$TEXMF/texmf/cweb//};\hangindent=2\parindent\par
 (c)~compile-time default directories \.{.:$TEXMF/texmf/cweb//}
    (specified in \.{texmf.in}).


@d kpse_find_cweb(name) kpse_find_file(name,kpse_cweb_format,true)

@ The simple file searching is replaced by `path searching' mechanism
that \Kpathsea/ library provides.

We set |kpse_program_name| to a |"cweb"|.  This means if the
variable |CWEBINPUTS.cweb| is present in \.{texmf.cnf} (or |CWEBINPUTS_cweb|
in the environment) its value will be used as the search path for
filenames.  This allows different flavors of \.{CWEB} to have
different search paths.

FIXME: Not sure this is the best way to go about this.

@<Set up |PROGNAME| feature and initialize the search path mechanism@>=
kpse_set_program_name(argv[0], "cweb"); 
@z


Section 23.

@x l.475
  char temp_file_name[max_file_name_length];
  char *cur_file_name_end=cur_file_name+max_file_name_length-1;
  char *k=cur_file_name, *kk;
  int l; /* length of file name */
@y
  char *cur_file_name_end=cur_file_name+max_file_name_length-1;
  char *k=cur_file_name;
@z

@x l.489
  if ((cur_file=fopen(cur_file_name,"r"))!=NULL) {
@y
  if ((found_filename=kpse_find_cweb(cur_file_name))!=NULL &&
      (cur_file=fopen(found_filename,"r"))!=NULL) {
    /* Copy name for #line directives. */
    if (strlen(found_filename) < max_file_name_length) {
      strcpy(cur_file_name, found_filename);
      free(found_filename);
    }
@z

Replaced by Kpathsea `kpse_find_file'

@x l.493
  kk=getenv("CWEBINPUTS");
  if (kk!=NULL) {
    if ((l=strlen(kk))>max_file_name_length-2) too_long();
    strcpy(temp_file_name,kk);
  }
  else {
#ifdef CWEBINPUTS
    if ((l=strlen(CWEBINPUTS))>max_file_name_length-2) too_long();
    strcpy(temp_file_name,CWEBINPUTS);
#else
    l=0;
#endif /* |CWEBINPUTS| */
  }
  if (l>0) {
    if (k+l+2>=cur_file_name_end)  too_long();
@.Include file name ...@>
    for (; k>= cur_file_name; k--) *(k+l+1)=*k;
    strcpy(cur_file_name,temp_file_name);
    cur_file_name[l]='/'; /* \UNIX/ pathname separator */
    if ((cur_file=fopen(cur_file_name,"r"))!=NULL) {
      cur_line=0; print_where=1;
      goto restart; /* success */
    }
  }
@y
@z

Section 26.

@x l.553
      if (xisupper(buffer[1])) buffer[1]=tolower(buffer[1]);
@y
      if (xisupper(buffer[1])) buffer[1]=tolower((unsigned char)buffer[1]);
@z

@x l.571
check_complete(){
@y
check_complete (void) {
@z

@x l.589
@d max_bytes 90000 /* the number of bytes in identifiers,
@y
@d max_bytes 1000000 /* the number of bytes in identifiers,
@z

@x l.591
@d max_names 4000 /* number of identifiers, strings, section names;
@y
@d max_names 10239 /* number of identifiers, strings, section names;
@z

@x l.642
@d hash_size 353 /* should be prime */
@y
@d hash_size 8501 /* should be prime */
@z

Section 33.

@x l.650
@ @<Predec...@>=
extern int names_match();
@y
@ @<External functions@>=
extern int names_match (name_pointer, const char*, int, char);
@z

Section 35.

@x l.661
id_lookup(first,last,t) /* looks up a string in the identifier table */
char *first; /* first character of string */
char *last; /* last character of string plus one */
char t; /* the |ilk|; used by \.{CWEAVE} only */
@y
/* looks up a string in the identifier table */
id_lookup (const char *first, const char *last, char t)
@z

@x l.667
  char *i=first; /* position in |buffer| */
@y
  const char *i=first; /* position in |buffer| */
@z

@x l.668 - rename local var, not to shadow global
  int h; /* hash code */
@y
  int h; /* hash code */
@z

Section 36.

@x l.684 - use renamed local var
h=(unsigned char)*i;
while (++i<last) h=(h+h+(int)((unsigned char)*i)) % hash_size;
@y
h=(unsigned char)*i;
while (++i<last) h=(h+h+(int)((unsigned char)*i)) % hash_size;
@z

Section 37.

@x l.692 - use renamed local var
p=hash[h];
@y
p=hash[h];
@z

@x l.696 - use renamed local var
  p->link=hash[h]; hash[h]=p; /* insert |p| at beginning of hash list */
@y
  p->link=hash[h]; hash[h]=p; /* insert |p| at beginning of hash list */
@z

Section 38.

@x l.703
@<Pred...@>=
void init_p();
@y
@<External functions@>=
extern void init_p (name_pointer p, char t);
@z

Section 42.

@x l.766
print_section_name(p)
name_pointer p;
@y
print_section_name (name_pointer p)
@z

Section 43.

@x l.785
sprint_section_name(dest,p)
  char*dest;
  name_pointer p;
@y
sprint_section_name (char *dest, name_pointer p)
@z

Section 44.

@x l.805
void
print_prefix_name(p)
name_pointer p;
@y
static void
print_prefix_name (name_pointer p)
@z

Section 45.

@x l.826
int web_strcmp(j,j_len,k,k_len) /* fuller comparison than |strcmp| */
  char *j, *k; /* beginning of first and second strings */
  int j_len, k_len; /* length of strings */
@y
/* fuller comparison than |strcmp| */
static int
web_strcmp (char *j, int j_len, char *k, int k_len)
@z

@x l.830 -- rename local vars, not to shadow math function
  char *j1=j+j_len, *k1=k+k_len;
  while (k<k1 && j<j1 && *j==*k) k++, j++;
  if (k==k1) if (j==j1) return equal;
    else return extension;
  else if (j==j1) return prefix;
@y
  char *j1=j+j_len, *k1=k+k_len;
  while (k<k1 && j<j1 && *j==*k) k++, j++;
  if (k==k1) if (j==j1) return equal;
    else return extension;
  else if (j==j1) return prefix;
@z

Section 46.

@x l.852
@<Prede...@>=
extern void init_node();
@y
@<External functions@>=
extern void init_node (name_pointer node);
@z

Section 47.

@x l.856
name_pointer
add_section_name(par,c,first,last,ispref) /* install a new node in the tree */
name_pointer par; /* parent of new node */
int c; /* right or left? */
char *first; /* first character of section name */
char *last; /* last character of section name, plus one */
int ispref; /* are we adding a prefix or a full name? */
@y
static name_pointer
add_section_name (name_pointer par, int c, char *first, char *last,
                  int ispref)  /* install a new node in the tree */
@z

Section 48.

@x l.885
void
extend_section_name(p,first,last,ispref)
name_pointer p; /* name to be extended */
char *first; /* beginning of extension text */
char *last; /* one beyond end of extension text */
int ispref; /* are we adding a prefix or a full name? */
@y
static void
extend_section_name (name_pointer p, char *first, char *last, int ispref)
@z

Section 49.

@x l.914
section_lookup(first,last,ispref) /* find or install section name in tree */
char *first, *last; /* first and last characters of new name */
int ispref; /* is the new name a prefix or a full name? */
@y
/* find or install section name in tree */
section_lookup (char *first, char *last, int ispref)
@z

Section 53.

@x l.1018
int section_name_cmp();
@y
static int section_name_cmp (char**, int, name_pointer);
@z

Section 54.

@x l.1021
int section_name_cmp(pfirst,len,r)
char **pfirst; /* pointer to beginning of comparison string */
int len; /* length of string */
name_pointer r; /* section name being compared */
@y
static int
section_name_cmp (char **pfirst, int len, name_pointer r)
@z

Section 57.

@x l.1092
@<Predecl...@>=
void  err_print();
@y
@<External functions@>=
extern void  err_print (const char*);
@z

Section 58.

@x l.1098
err_print(s) /* prints `\..' and location of error message */
char *s;
@y
err_print (const char *s) /* prints `\..' and location of error message */
@z

Section 60.

@x l.1140
@<Prede...@>=
int wrap_up();
extern void print_stats();
@y
@<External functions@>=
extern int wrap_up (void);
extern void print_stats (void);
@z

Section 61.

@x l.1151
int wrap_up() {
@y
int wrap_up (void) {
@z

Section 63.

@x l.1173
@<Predec...@>=
void fatal(), overflow();
@y
@<External functions@>=
extern void fatal (const char*, const char*);
extern void overflow (const char*);
@z

Section 64.

@x l.1180
fatal(s,t)
  char *s,*t;
@y
fatal (const char *s, const char *t)
@z

Section 65.

@x l.1191
overflow(t)
  char *t;
@y
overflow (const char *t)
@z

Section 67.

@x l.1212
the names of those files. Most of the 128 flags are undefined but available
for future extensions.
@y
the names of those files. Most of the 128 flags are undefined but available
for future extensions.

We use `kpathsea' library functions to find literate sources and
NLS configuration files. When the files you expect are not
being found, the thing to do is to enable `kpathsea' runtime
debugging by assigning to |kpathsea_debug| variable a small number
via `\.{-d}' option. The meaning of number is shown below. To set
more than one debugging options sum the corresponding numbers.
$$\halign{\hskip5em\tt\hfil#&&\qquad\tt#\cr
 1&report `\.{stat}' calls\cr
 2&report lookups in all hash tables\cr
 4&report file openings and closings\cr
 8&report path information\cr
16&report directory list\cr
32&report on each file search\cr
64&report values of variables being looked up\cr}$$
Debugging output is always written to |stderr|, and begins with the string
`\.{kdebug:}'.
@z

@x l.1218
@d show_happiness flags['h'] /* should lack of errors be announced? */
@y
@d show_happiness flags['h'] /* should lack of errors be announced? */
@d show_kpathsea_debug flags['d']
  /* should results of file searching be shown? */
@z

@x l.1234
show_banner=show_happiness=show_progress=1;
@y
show_banner=show_happiness=show_progress=1;
@z

Section 69.

@x l.1252
void scan_args();
@y
static void scan_args (void);
@z


Section 70.

@x l.1255
void
scan_args()
@y
static void
scan_args (void)
@z


Section 71.

@x l.1282 - use a define for /dev/null
  if (found_change<=0) strcpy(change_file_name,"/dev/null");
@y
  if (found_change<=0) strcpy(change_file_name,DEV_NULL);
@z

@x l.1302 - no alt_web_file_name
  sprintf(alt_web_file_name,"%s.web",*argv);
@y
@z


Section 74.

@x l.1344
@ @<Handle flag...@>=
{
@y
@ @<Handle flag...@>=
{
  if (strcmp("-help",*argv)==0 || strcmp("--help",*argv)==0)
    @<Display help message and exit@>;
  if (strcmp("-version",*argv)==0 || strcmp("--version",*argv)==0)
    @<Display version information and exit@>;
@z

@x l.1347
  else flag_change=1;
@y
  else flag_change=1;
  if (*(*argv+1)=='d')
    if (sscanf(*argv+2,"%u",&kpathsea_debug)!=1) @<Print usage error...@>;
@z

@x l.1349
    flags[*dot_pos]=flag_change;
@y
    flags[(unsigned char)*dot_pos]=flag_change;
@z

Section 75.

@x l.1354
if (program==ctangle)
  fatal(
"! Usage: ctangle [options] webfile[.w] [{changefile[.ch]|-} [outfile[.c]]]\n"
   ,"");
@.Usage:@>
else fatal(
"! Usage: cweave [options] webfile[.w] [{changefile[.ch]|-} [outfile[.tex]]]\n"
   ,"");
@y
if (program==ctangle) {
  fprintf(stderr, "ctangle: Need one to three file arguments.\n");
#if defined(MIKTEX)
  throw (1);
#else
  usage("ctangle");
#endif
} else {
  fprintf(stderr, "cweave: Need one to three file arguments.\n");
#if defined(MIKTEX)
  throw (1);
#else
  usage("cweave");
#endif
}
@z

Section 77.

@x l.1375
FILE *active_file; /* currently active file for \.{CWEAVE} output */
@y
FILE *active_file; /* currently active file for \.{CWEAVE} output */
char *found_filename; /* filename found by |kpse_find_file| */
@z

Section 78.

@x l.1380 Use binary mode for output files
  if ((C_file=fopen(C_file_name,"w"))==NULL)
@y
  if ((C_file=fopen(C_file_name,"wb"))==NULL)
@z

@x l.1386 Use binary mode for output files
  if ((tex_file=fopen(tex_file_name,"w"))==NULL)
@y
  if ((tex_file=fopen(tex_file_name,"wb"))==NULL)
@z


Section 81. (reused)

@x l.1403
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
@ We declare some more prototypes for exported function in cases where this
could not be done easily without changing section numbers.

@<External functions@>=
extern void common_init (void);
extern int input_ln (FILE *fp);
extern void reset_input (void);
extern int get_line (void);
extern void check_complete (void);
extern name_pointer id_lookup (const char *first, const char *last, char t);
extern void print_section_name (name_pointer p);
extern void sprint_section_name (char *dest, name_pointer p);
extern name_pointer section_lookup (char *first, char *last, int ispref);
@z

@x
@** Index.
@y
@** External functions.  In order to allow for type checking we create a
header file \.{cweb.h} containing the declaration of all functions defined
in \.{common.w} and used in \.{ctangle.w} and \.{cweave.w} or vice versa.

@(cweb.h@>=
@=/* Prototypes for functions, either@>
@= * declared in common.w and used in ctangle.w and cweave.w, or@>
@= * used in common.w and declared in ctangle.w and cweave.w.  */@>
@<External functions@>@;
#if !defined(MIKTEX)
extern const char *versionstring;
#endif

@** System dependent changes.

@ Modules for dealing with help messages and version info.

@<Display help message and exit@>=
#if defined(MIKTEX)
throw (0);
#else
usagehelp(program==ctangle ? CTANGLEHELP : CWEAVEHELP, NULL);
@.--help@>
#endif

@ Will have to change these if the version numbers change (ouch).

@d ctangle_banner "This is CTANGLE, Version 3.64"
@d cweave_banner "This is CWEAVE, Version 3.64"

@<Display version information and exit@>=
#if defined(MIKTEX)
  throw (0); // todo
#else
printversionandexit((program==ctangle ? ctangle_banner : cweave_banner),
  "Silvio Levy and Donald E. Knuth", NULL, NULL);
@.--version@>
#endif

@** Index.
@z
