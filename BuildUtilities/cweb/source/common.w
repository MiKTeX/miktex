% This file is part of CWEB.
% This program by Silvio Levy and Donald E. Knuth
% is based on a program by Knuth.
% It is distributed WITHOUT ANY WARRANTY, express or implied.
% Version 4.7 --- February 2022

% Copyright (C) 1987,1990,1993,2000 Silvio Levy and Donald E. Knuth

% Permission is granted to make and distribute verbatim copies of this
% document provided that the copyright notice and this permission notice
% are preserved on all copies.

% Permission is granted to copy and distribute modified versions of this
% document under the conditions for verbatim copying, provided that the
% entire resulting derived work is given a different name and distributed
% under the terms of a permission notice identical to this one.

% Amendments to 'common.w' resulting in this updated version were created
% by numerous collaborators over the course of many years.

% Please send comments, suggestions, etc. to tex-k@@tug.org.

\def\v{\char'174} % vertical (|) in typewriter font

\def\title{Common code for CTANGLE and CWEAVE (Version 4.7)}
\def\topofcontents{\null\vfill
  \centerline{\titlefont Common code for {\ttitlefont CTANGLE} and
    {\ttitlefont CWEAVE}}
  \vskip 15pt
  \centerline{(Version 4.7)}
  \vfill}
\def\botofcontents{\vfill
\noindent
Copyright \copyright\ 1987, 1990, 1993, 2000 Silvio Levy and Donald E. Knuth
\bigskip\noindent
Permission is granted to make and distribute verbatim copies of this
document provided that the copyright notice and this permission notice
are preserved on all copies.

\smallskip\noindent
Permission is granted to copy and distribute modified versions of this
document under the conditions for verbatim copying, provided that the
entire resulting derived work is given a different name and distributed
under the terms of a permission notice identical to this one.
}

\pageno=\contentspagenumber \advance\pageno by 1
\let\maybe=\iftrue

@** Introduction.  This file contains code common
to both \.{CTANGLE} and \.{CWEAVE}, which roughly concerns the following
problems: character uniformity, input routines, error handling and
parsing of command line.  We have tried to concentrate in this file
all the system dependencies, so as to maximize portability.

In the texts below we will
sometimes use \.{CWEB} to refer to either of the two component
programs, if no confusion can arise.

The file begins with a few basic definitions.

@c
@<Include files@>@/
@h
@<Common code for \.{CWEAVE} and \.{CTANGLE}@>@/
@<Global variables@>@/
@<Predeclaration of procedures@>

@ The details will be filled in due course.  The interface of this module
is included first.  It is also used by the main programs.

@i common.h

@ In certain cases \.{CTANGLE} and \.{CWEAVE} should do almost, but not
quite, the same thing.  In these cases we've written common code for
both, differentiating between the two by means of the global variable
|program|.

@<Global var...@>=
boolean program; /* \.{CWEAVE} or \.{CTANGLE}? */

@ \.{CWEAVE} operates in three phases: First it inputs the source
file and stores cross-reference data, then it inputs the source once again and
produces the \TEX/ output file, and finally it sorts and outputs the index.
Similarly, \.{CTANGLE} operates in two phases.
The global variable |phase| tells which phase we are in.

@<Global var...@>=
int phase; /* which phase are we in? */

@ There's an initialization procedure that gets both \.{CTANGLE} and
\.{CWEAVE} off to a good start. We will fill in the details of this
procedure later.

@c
void
common_init(void)
{
  @<Initialize pointers@>@;
  @<Set the default options common to \.{CTANGLE} and \.{CWEAVE}@>@;
  @<Scan arguments and open output files@>@;
}

@* The character set.
\.{CWEB} uses the conventions of \CEE/ programs found in the standard
\.{ctype.h} header file.

A few character pairs are encoded internally as single characters,
using the definitions in the interface sections above. These definitions
are consistent with an extension of ASCII code originally developed at
MIT and explained in Appendix~C of {\sl The \TEX/book\/}; thus, users
who have such a character set can type things like \.{\char'32} and
\.{\char'4} instead of \.{!=} and \.{\&\&}. (However, their files will
not be too portable until more people adopt the extended code.)

If the character set is not ASCII, the definitions given may conflict
with existing characters; in such cases, other arbitrary codes should be
substituted. The indexes to \.{CTANGLE} and \.{CWEAVE} mention every
case where similar codes may have to be changed in order to
avoid character conflicts. Look for the entry ``ASCII code dependencies''
in those indexes.

@^ASCII code dependencies@>
@^system dependencies@>

@<Global var...@>=
char section_text[longest_name+1]; /* text being sought for */
char *section_text_end = section_text+longest_name; /* end of |section_text| */
char *id_first; /* where the current identifier begins in the buffer */
char *id_loc; /* just after the current identifier in the buffer */

@** Input routines.  The lowest level of input to the \.{CWEB} programs
is performed by |input_ln|, which must be told which file to read from.
The return value of |input_ln| is |true| if the read is successful and
|false| if not (generally this means the file has ended). The conventions
of \TEX/ are followed; i.e., the characters of the next line of the file
are copied into the |buffer| array,
and the global variable |limit| is set to the first unoccupied position.
Trailing blanks are ignored. The value of |limit| must be strictly less
than |buf_size|, so that |buffer[buf_size-1]| is never filled.

Since |buf_size| is strictly less than |long_buf_size|,
some of \.{CWEB}'s routines use the fact that it is safe to refer to
|*(limit+2)| without overstepping the bounds of the array.

@<Global var...@>=
char buffer[long_buf_size]; /* where each line of input goes */
char *buffer_end=buffer+buf_size-2; /* end of |buffer| */
char *loc=buffer; /* points to the next character to be read from the buffer */
char *limit=buffer; /* points to the last character in the buffer */

@ In the unlikely event that your standard I/O library does not
support |feof|, |getc|, and |ungetc| you may have to change things here.
@^system dependencies@>

@c
static boolean input_ln( /* copies a line into |buffer| or returns |false| */
FILE *fp) /* what file to read from */
{
  register int c=EOF; /* character read; initialized so some compilers won't complain */
  register char *k; /* where next character goes */
  if (feof(fp)) return false; /* we have hit end-of-file */
  limit = k = buffer; /* beginning of buffer */
  while (k<=buffer_end && (c=getc(fp)) != EOF && c!='\n')
    if ((*(k++) = c) != ' ') limit = k;
  if (k>buffer_end)
    if ((c=getc(fp))!=EOF && c!='\n') {
      ungetc(c,fp); loc=buffer; err_print("! Input line too long");
@.Input line too long@>
    }
  if (c==EOF && limit==buffer) return false; /* there was nothing after
    the last newline */
  return true;
}

@ @<Predecl...@>=@+static boolean input_ln(FILE *);

@ Now comes the problem of deciding which file to read from next.
Recall that the actual text that \.{CWEB} should process comes from two
streams: a |web_file|, which can contain possibly nested include
commands~\.{@@i}, and a |change_file|, which might also contain
includes.  The |web_file| together with the currently open include
files form a stack |file|, whose names are stored in a parallel stack
|file_name|.  The boolean |changing| tells whether or not we're reading
from the |change_file|.

The line number of each open file is also kept for error reporting and
for the benefit of \.{CTANGLE}.

@<Global var...@>=
int include_depth; /* current level of nesting */
FILE *file[max_include_depth]; /* stack of non-change files */
FILE *change_file; /* change file */
char file_name[max_include_depth][max_file_name_length];
  /* stack of non-change file names */
char change_file_name[max_file_name_length]; /* name of change file */
static char alt_web_file_name[max_file_name_length]; /* alternate name to try */
int line[max_include_depth]; /* number of current line in the stacked files */
int change_line; /* number of current line in change file */
int change_depth; /* where \.{@@y} originated during a change */
boolean input_has_ended; /* if there is no more input */
boolean changing; /* if the current line is from |change_file| */
boolean web_file_open=false; /* if the web file is being read */

@ When |changing==false|, the next line of |change_file| is kept in
|change_buffer|, for purposes of comparison with the next
line of |cur_file|. After the change file has been completely input, we
set |change_limit=change_buffer|,
so that no further matches will be made.

Here's a shorthand expression for inequality between the two lines:

@d lines_dont_match (change_limit-change_buffer != limit-buffer @| ||
  strncmp(buffer, change_buffer, (size_t)(limit-buffer)) != 0)

@<Global var...@>=
static char change_buffer[buf_size]; /* next line of |change_file| */
static char *change_limit; /* points to the last character in |change_buffer| */

@ Procedure |prime_the_change_buffer|
sets |change_buffer| in preparation for the next matching operation.
Since blank lines in the change file are not used for matching, we have
|(change_limit==change_buffer && !changing)| if and only if
the change file is exhausted. This procedure is called only when
|changing| is |true|; hence error messages will be reported correctly.

@c
static void
prime_the_change_buffer(void)
{
  change_limit=change_buffer; /* this value is used if the change file ends */
  @<Skip over comment lines in the change file; |return| if end of file@>@;
  @<Skip to the next nonblank line; |return| if end of file@>@;
  @<Move |buffer| and |limit| to |change_buffer| and |change_limit|@>@;
}

@ @<Predecl...@>=@+static void prime_the_change_buffer(void);

@ While looking for a line that begins with \.{@@x} in the change file, we
allow lines that begin with \.{@@}, as long as they don't begin with \.{@@y},
\.{@@z}, or \.{@@i} (which would probably mean that the change file is fouled up).

@<Skip over comment lines in the change file...@>=
while(true) {
  change_line++;
  if (!input_ln(change_file)) return;
  if (limit<buffer+2) continue;
  if (buffer[0]!='@@') continue;
  if (xisupper(buffer[1])) buffer[1]=tolower((int)buffer[1]);
  if (buffer[1]=='x') break;
  if (buffer[1]=='y' || buffer[1]=='z' || buffer[1]=='i') {
    loc=buffer+2;
    err_print("! Missing @@x in change file");
@.Missing @@x...@>
  }
}

@ Here we are looking at lines following the \.{@@x}.

@<Skip to the next nonblank line...@>=
do {
  change_line++;
  if (!input_ln(change_file)) {
    err_print("! Change file ended after @@x");
@.Change file ended...@>
    return;
  }
} while (limit==buffer);

@ @<Move |buffer| and |limit| to |change_buffer| and |change_limit|@>=
change_limit=change_buffer+(ptrdiff_t)(limit-buffer);
strncpy(change_buffer,buffer,(size_t)(limit-buffer+1));

@ The following procedure is used to see if the next change entry should
go into effect; it is called only when |changing| is |false|.
The idea is to test whether or not the current
contents of |buffer| matches the current contents of |change_buffer|.
If not, there's nothing more to do; but if so, a change is called for:
All of the text down to the \.{@@y} is supposed to match. An error
message is issued if any discrepancy is found. Then the procedure
prepares to read the next line from |change_file|.

When a match is found, the current section is marked as changed unless
the first line after the \.{@@x} and after the \.{@@y} both start with
either `\.{@@*}' or `\.{@@\ }' (possibly preceded by whitespace).

This procedure is called only when |buffer<limit|, i.e., when the
current line is nonempty.

@d if_section_start_make_pending(b)@/
  *limit='!';@+
  for (loc=buffer;xisspace(*loc);loc++) ;@+
  *limit=' ';@/
  if (*loc=='@@' && (xisspace(*(loc+1)) || *(loc+1)=='*')) change_pending=b

@c
static void
check_change(void) /* switches to |change_file| if the buffers match */
{
  int n=0; /* the number of discrepancies found */
  if (lines_dont_match) return;
  change_pending=false;
  if (!changed_section[section_count]) {
    if_section_start_make_pending(true);
    if (!change_pending) changed_section[section_count]=true;
  }
  while (true) {
    changing=print_where=true; change_line++;
    if (!input_ln(change_file)) {
      err_print("! Change file ended before @@y");
@.Change file ended...@>
      change_limit=change_buffer; changing=false;
      return;
    }
    if (limit>buffer+1 && buffer[0]=='@@') {
      char xyz_code=xisupper(buffer[1])? tolower((int)buffer[1]): buffer[1];
      @<If the current line starts with \.{@@y},
        report any discrepancies and |return|@>@;
    }
    @<Move |buffer| and |limit|...@>@;
    changing=false; cur_line++;
    while (!input_ln(cur_file)) { /* pop the stack or quit */
      if (include_depth==0) {
        err_print("! CWEB file ended during a change");
@.CWEB file ended...@>
        input_has_ended=true; return;
      }
      include_depth--; cur_line++;
    }
    if (lines_dont_match) n++;
  }
}

@ @<Predecl...@>=@+static void check_change(void);

@ @<If the current line starts with \.{@@y}...@>=
if (xyz_code=='x' || xyz_code=='z') {
  loc=buffer+2; err_print("! Where is the matching @@y?");
@.Where is the match...@>
  }
else if (xyz_code=='y') {
  if (n>0) {
    loc=buffer+2;
    printf("\n! Hmm... %d ",n);
    err_print("of the preceding lines failed to match");
@.Hmm... n of the preceding...@>
  }
  change_depth=include_depth;
  return;
}

@ The |reset_input| procedure, which gets \.{CWEB} ready to read the
user's \.{CWEB} input, is used at the beginning of phase one of \.{CTANGLE},
phases one and two of \.{CWEAVE}.

@c
void
reset_input(void)
{
  limit=buffer; loc=buffer+1; buffer[0]=' ';
  @<Open input files@>@;
  include_depth=cur_line=change_line=0;
  change_depth=include_depth;
  changing=true; prime_the_change_buffer(); changing=!changing;
  limit=buffer; loc=buffer+1; buffer[0]=' '; input_has_ended=false;
}

@ The following code opens the input files.
@^system dependencies@>

@<Open input files@>=
if ((web_file=fopen(web_file_name,"r"))==NULL) {
  strcpy(web_file_name,alt_web_file_name);
  if ((web_file=fopen(web_file_name,"r"))==NULL)
       fatal("! Cannot open input file ", web_file_name);
}
@.Cannot open input file@>
@.Cannot open change file@>
web_file_open=true;
if ((change_file=fopen(change_file_name,"r"))==NULL)
       fatal("! Cannot open change file ", change_file_name);

@ The |get_line| procedure is called when |loc>limit|; it puts the next
line of merged input into the buffer and updates the other variables
appropriately. A space is placed at the right end of the line.
This procedure returns |!input_has_ended| because we often want to
check the value of that variable after calling the procedure.

If we've just changed from the |cur_file| to the |change_file|, or if
the |cur_file| has changed, we tell \.{CTANGLE} to print this
information in the \CEE/ file by means of the |print_where| flag.

@<Global var...@>=
sixteen_bits section_count; /* the current section number */
boolean changed_section[max_sections]; /* is the section changed? */
boolean change_pending; /* if the current change is not yet recorded in
  |changed_section[section_count]| */
boolean print_where=false; /* should \.{CTANGLE} print line and file info? */

@ @c
boolean get_line(void) /* inputs the next line */
{
  restart:
  if (changing && include_depth==change_depth)
    @<Read from |change_file| and maybe turn off |changing|@>@;
  if (! changing || include_depth>change_depth) {
    @<Read from |cur_file| and maybe turn on |changing|@>@;
    if (changing && include_depth==change_depth) goto restart;
  }
  if (input_has_ended) return false;
  loc=buffer; *limit=' ';
  if (buffer[0]=='@@' && (buffer[1]=='i' || buffer[1]=='I')) {
    loc=buffer+2; *limit='"';
    while (*loc==' '||*loc=='\t') loc++;
    if (loc>=limit) {
      err_print("! Include file name not given");
@.Include file name ...@>
      goto restart;
    }
    if (include_depth>=max_include_depth-1) {
      err_print("! Too many nested includes");
@.Too many nested includes@>
      goto restart;
    }
    include_depth++; /* push input stack */
    @<Try to open include file, abort push if unsuccessful, go to |restart|@>@;
  }
  return true;
}

@ When an \.{@@i} line is found in the |cur_file|, we must temporarily
stop reading it and start reading from the named include file.  The
\.{@@i} line should give a complete file name with or without
double quotes.
If the environment variable \.{CWEBINPUTS} is set, or if the compiler flag
of the same name was defined at compile time,
\.{CWEB} will look for include files in the directory thus named, if
it cannot find them in the current directory.
(Colon-separated paths are not supported.)
The remainder of the \.{@@i} line after the file name is ignored.

@d too_long() {include_depth--;
        err_print("! Include file name too long"); goto restart;}

@<Try to open...@>= {
  char temp_file_name[max_file_name_length];
  char *cur_file_name_end=cur_file_name+max_file_name_length-1;
  char *kk, *k=cur_file_name;
  size_t l; /* length of file name */

  if (*loc=='"') {
    loc++;
    while (*loc!='"' && k<=cur_file_name_end) *k++=*loc++;
    if (loc==limit) k=cur_file_name_end+1; /* unmatched quote is `too long' */
  } else
    while (*loc!=' '&&*loc!='\t'&&*loc!='"'&&k<=cur_file_name_end) *k++=*loc++;
  if (k>cur_file_name_end) too_long();
@.Include file name ...@>
  *k='\0';
  if ((cur_file=fopen(cur_file_name,"r"))!=NULL) {
    cur_line=0; print_where=true;
    goto restart; /* success */
  }
  if ((kk=getenv("CWEBINPUTS"))!=NULL) {
@.CWEBINPUTS@>
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
    if (k+l+2>=cur_file_name_end) too_long();
@.Include file name ...@>
    for (; k>=cur_file_name; k--) *(k+l+1)=*k;
    strcpy(cur_file_name,temp_file_name);
    cur_file_name[l]='/'; /* \UNIX/ pathname separator */
    if ((cur_file=fopen(cur_file_name,"r"))!=NULL) {
      cur_line=0; print_where=true;
      goto restart; /* success */
    }
  }
  include_depth--; err_print("! Cannot open include file"); goto restart;
}

@ @<Read from |cur_file|...@>= {
  cur_line++;
  while (!input_ln(cur_file)) { /* pop the stack or quit */
    print_where=true;
    if (include_depth==0) {input_has_ended=true; break;}
    else {
      fclose(cur_file); include_depth--;
      if (changing && include_depth==change_depth) break;
      cur_line++;
    }
  }
  if (!changing && !input_has_ended)
   if (limit-buffer==change_limit-change_buffer)
    if (buffer[0]==change_buffer[0])
      if (change_limit>change_buffer) check_change();
}

@ @<Read from |change_file|...@>= {
  change_line++;
  if (!input_ln(change_file)) {
    err_print("! Change file ended without @@z");
@.Change file ended...@>
    buffer[0]='@@'; buffer[1]='z'; limit=buffer+2;
  }
  if (limit>buffer) { /* check if the change has ended */
    if (change_pending) {
      if_section_start_make_pending(false);
      if (change_pending) {
        changed_section[section_count]=true; change_pending=false;
      }
    }
    *limit=' ';
    if (buffer[0]=='@@') {
      if (xisupper(buffer[1])) buffer[1]=tolower((int)buffer[1]);
      if (buffer[1]=='x' || buffer[1]=='y') {
        loc=buffer+2;
        err_print("! Where is the matching @@z?");
@.Where is the match...@>
      }
      else if (buffer[1]=='z') {
        prime_the_change_buffer(); changing=!changing; print_where=true;
      }
    }
  }
}

@ At the end of the program, we will tell the user if the change file
had a line that didn't match any relevant line in |web_file|.

@c
void
check_complete(void) {
  if (change_limit!=change_buffer) { /* |changing| is |false| */
    strncpy(buffer,change_buffer,(size_t)(change_limit-change_buffer+1));
    limit=buffer+(ptrdiff_t)(change_limit-change_buffer);
    changing=true; change_depth=include_depth; loc=buffer;
    err_print("! Change file entry did not match");
@.Change file entry did not match@>
  }
}

@** Storage of names and strings.
Both \.{CWEAVE} and \.{CTANGLE} store identifiers, section names and
other strings in a large array of |char|s, called |byte_mem|.
Information about the names is kept in the array |name_dir|, whose
elements are structures of type |name_info|, containing a pointer into
the |byte_mem| array (the address where the name begins) and other data.
A |name_pointer| variable is a pointer into |name_dir|.  You find the
complete layout of |name_info| in the interface sections above.

The actual sequence of characters in the name pointed to by a |name_pointer
p| appears in positions |p->byte_start| to |(p+1)->byte_start-1|, inclusive.

The names of identifiers are found by computing a hash address |h| and
then looking at strings of bytes signified by the |name_pointer|s
|hash[h]|, |hash[h]->link|, |hash[h]->link->link|, \dots,
until either finding the desired name or encountering the null pointer.

The names of sections are stored in |byte_mem| together
with the identifier names, but a hash table is not used for them because
\.{CTANGLE} needs to be able to recognize a section name when given a prefix of
that name. A conventional binary search tree is used to retrieve section names,
with fields called |llink| and |rlink| (where |llink| takes the place
of |link|).  The root of this tree is stored in |name_dir->rlink|;
this will be the only information in |name_dir[0]|.

Since the space used by |rlink| has a different function for
identifiers than for section names, we declare it as a |union|.

The last component of |name_info| is different for \.{CTANGLE} and
\.{CWEAVE}.  In \.{CTANGLE}, if |p| is a pointer to a section name,
|p->equiv| is a pointer to its replacement text, an element of the
array |text_info|.  In \.{CWEAVE}, on the other hand, if
|p| points to an identifier, |p->xref| is a pointer to its
list of cross-references, an element of the array |xmem|.  The make-up
of |text_info| and |xmem| is discussed in the \.{CTANGLE} and \.{CWEAVE}
source files, respectively; here we just declare a common field
|equiv_or_xref| as a pointer to |void|.

@<Global var...@>=
char byte_mem[max_bytes]; /* characters of names */
char *byte_mem_end = byte_mem+max_bytes-1; /* end of |byte_mem| */
name_info name_dir[max_names]; /* information about names */
name_pointer name_dir_end = name_dir+max_names-1; /* end of |name_dir| */

@ The first unused position in |byte_mem| and |name_dir| is
kept in |byte_ptr| and |name_ptr|, respectively.  Thus we
usually have |name_ptr->byte_start==byte_ptr|, and certainly
we want to keep |name_ptr<=name_dir_end| and |byte_ptr<=byte_mem_end|.

@<Global var...@>=
char *byte_ptr; /* first unused position in |byte_mem| */
name_pointer name_ptr; /* first unused position in |name_dir| */

@ @<Init...@>=
name_dir->byte_start=byte_ptr=byte_mem; /* position zero in both arrays */
name_ptr=name_dir+1; /* |name_dir[0]| will be used only for error recovery */
name_ptr->byte_start=byte_mem; /* this makes name 0 of length zero */
root=NULL; /* the binary search tree starts out with nothing in it */

@ The hash table itself
consists of |hash_size| entries of type |name_pointer|, and is
updated by the |id_lookup| procedure, which finds a given identifier
and returns the appropriate |name_pointer|. The matching is done by the
function |names_match|, which is slightly different in
\.{CWEAVE} and \.{CTANGLE}.  If there is no match for the identifier,
it is inserted into the table.

@d hash_size 353 /* should be prime */

@<Global var...@>=
name_pointer hash[hash_size]; /* heads of hash lists */
hash_pointer hash_end = hash+hash_size-1; /* end of |hash| */
hash_pointer h; /* index into hash-head array */

@ Initially all the hash lists are empty.

@<Init...@>=
for (h=hash; h<=hash_end; *h++=NULL) ;

@ Here is the main procedure for finding identifiers:

@c
name_pointer
id_lookup( /* looks up a string in the identifier table */
const char *first, /* first character of string */
const char *last, /* last character of string plus one */
eight_bits t) /* the |ilk|; used by \.{CWEAVE} only */
{
  const char *i=first; /* position in |buffer| */
  int h; /* hash code; shadows |hash_pointer h| */
  size_t l; /* length of the given identifier */
  name_pointer p; /* where the identifier is being sought */
  if (last==NULL) for (last=first; *last!='\0'; last++);
  l=(size_t)(last-first); /* compute the length */
  @<Compute the hash code |h|@>@;
  @<Compute the name location |p|@>@;
  if (p==name_ptr) @<Enter a new name into the table at position |p|@>@;
  return p;
}

@ A simple hash code is used: If the sequence of
character codes is $c_1c_2\ldots c_n$, its hash value will be
$$(2^{n-1}c_1+2^{n-2}c_2+\cdots+c_n)\,\bmod\,|hash_size|.$$

@<Compute the hash...@>=
h=(int)((eight_bits)*i);
while (++i<last) h=(h+h+(int)((eight_bits)*i)) % hash_size;
@^high-bit character handling@>

@ If the identifier is new, it will be placed in position |p=name_ptr|,
otherwise |p| will point to its existing location.

@<Compute the name location...@>=
p=hash[h];
while (p && !names_match(p,first,l,t)) p=p->link;
if (p==NULL) {
  p=name_ptr; /* the current identifier is new */
  p->link=hash[h]; hash[h]=p; /* insert |p| at beginning of hash list */
}

@ The information associated with a new identifier must be initialized
in a slightly different way in \.{CWEAVE} than in \.{CTANGLE}; hence the
|init_p| procedure.

@<Enter a new name...@>= {
  if (byte_ptr+l>byte_mem_end) overflow("byte memory");
  if (name_ptr>=name_dir_end) overflow("name");
  strncpy(byte_ptr,first,l);
  (++name_ptr)->byte_start=byte_ptr+=l;
  init_p(p,t);
}

@ If |p| is a |name_pointer| variable, as we have seen,
|p->byte_start| is the beginning of the area where the name
corresponding to |p| is stored.  However, if |p| refers to a section
name, the name may need to be stored in chunks, because it may
``grow'': a prefix of the section name may be encountered before
the full name.  Furthermore we need to know the length of the shortest
prefix of the name that was ever encountered.

We solve this problem by inserting two extra bytes at |p->byte_start|,
representing the length of the shortest prefix, when |p| is a
section name. Furthermore, the last byte of the name will be a blank
space if |p| is a prefix. In the latter case, the name pointer
|p+1| will allow us to access additional chunks of the name:
The second chunk will begin at the name pointer |(p+1)->link|,
and if it too is a prefix (ending with blank) its |link| will point
to additional chunks in the same way. Null links are represented by
|name_dir|.

@d first_chunk(p) ((p)->byte_start+2)
@d prefix_length(p) (size_t)((eight_bits)*((p)->byte_start)*256 +
                (eight_bits)*((p)->byte_start+1))
@d set_prefix_length(p,m) (*((p)->byte_start)=(char)((m)/256),
                 *((p)->byte_start+1)=(char)((m)%256))

@c
void
print_section_name(
name_pointer p)
{
  char *ss, *s = first_chunk(p);
  name_pointer q = p+1;
  while (p!=name_dir) {
    ss = (p+1)->byte_start-1;
    if (*ss==' ' && ss>=s) p=q->link,q=p;
    else ss++,p=name_dir,q=NULL;
    term_write(s,(size_t)(ss-s));
    s = p->byte_start;
  }
  if (q) term_write("...",3); /* complete name not yet known */
}

@ @c
void
sprint_section_name(
  char *dest,
  name_pointer p)
{
  char *ss, *s = first_chunk(p);
  name_pointer q = p+1;
  while (p!=name_dir) {
    ss = (p+1)->byte_start-1;
    if (*ss==' ' && ss>=s) p=q->link,q=p;
    else ss++,p=name_dir;
    strncpy(dest,s,(size_t)(ss-s)), dest+=ss-s;
    s = p->byte_start;
  }
  *dest='\0';
}

@ @c
void
print_prefix_name(
name_pointer p)
{
  char *s = first_chunk(p);
  size_t l = prefix_length(p);
  term_write(s,l);
  if (s+l<(p+1)->byte_start) term_write("...",3);
}

@ When we compare two section names, we'll need a function analogous to
|strcmp|. But we do not assume the strings
are null-terminated, and we keep an eye open for prefixes and extensions.

@d less 0 /* the first name is lexicographically less than the second */
@d equal 1 /* the first name is equal to the second */
@d greater 2 /* the first name is lexicographically greater than the second */
@d prefix 3 /* the first name is a proper prefix of the second */
@d extension 4 /* the first name is a proper extension of the second */

@<Predecl...@>=
static int web_strcmp(char *,size_t,char *,size_t);@/
static name_pointer add_section_name(name_pointer,int,char *,char *,boolean);@/
static void extend_section_name(name_pointer,char *,char *,boolean);

@ @c
static int web_strcmp( /* fuller comparison than |strcmp| */
  char *j, /* beginning of first string */
  size_t j_len, /* length of first string */
  char *k, /* beginning of second string */
  size_t k_len) /* length of second string */
{
  char *j1=j+j_len, *k1=k+k_len;
  while (k<k1 && j<j1 && *j==*k) k++, j++;
  if (k==k1) if (j==j1) return equal;
    else return extension;
  else if (j==j1) return prefix;
  else if (*j<*k) return less;
  else return greater;
}

@ Adding a section name to the tree is straightforward if we know its
parent and whether it's the |rlink| or |llink| of the parent.  As a
special case, when the name is the first section being added, we set the
``parent'' to |NULL|.  When a section name is created, it has only one
chunk, which however may be just a prefix; the full name will
hopefully be unveiled later.  Obviously, |prefix_length| starts
out as the length of the first chunk, though it may decrease later.

The information associated with a new node must be initialized
differently in \.{CWEAVE} and \.{CTANGLE}; hence the
|init_node| procedure, which is defined differently in \.{cweave.w}
and \.{ctangle.w}.

@c
static name_pointer
add_section_name( /* install a new node in the tree */
name_pointer par, /* parent of new node */
int c, /* right or left? */
char *first, /* first character of section name */
char *last, /* last character of section name, plus one */
boolean ispref) /* are we adding a prefix or a full name? */
{
  name_pointer p=name_ptr; /* new node */
  char *s=first_chunk(p);
  size_t name_len=(size_t)(last-first+(int)ispref); /* length of section name */
  if (s+name_len>byte_mem_end) overflow("byte memory");
  if (name_ptr+1>=name_dir_end) overflow("name");
  (++name_ptr)->byte_start=byte_ptr=s+name_len;
  if (ispref) {
    *(byte_ptr-1)=' ';
    name_len--;
    name_ptr->link=name_dir;
    (++name_ptr)->byte_start=byte_ptr;
  }
  set_prefix_length(p,name_len);
  strncpy(s,first,name_len);
  p->llink=p->rlink=NULL;
  init_node(p);
  return par==NULL ? (root=p) : c==less ? (par->llink=p) : (par->rlink=p);
}

@ @c
static void
extend_section_name(
name_pointer p, /* name to be extended */
char *first, /* beginning of extension text */
char *last, /* one beyond end of extension text */
boolean ispref) /* are we adding a prefix or a full name? */
{
  char *s;
  name_pointer q=p+1;
  size_t name_len=(size_t)(last-first+(int)ispref);
  if (name_ptr>=name_dir_end) overflow("name");
  while (q->link!=name_dir) q=q->link;
  q->link=name_ptr;
  s=name_ptr->byte_start;
  name_ptr->link=name_dir;
  if (s+name_len>byte_mem_end) overflow("byte memory");
  (++name_ptr)->byte_start=byte_ptr=s+name_len;
  strncpy(s,first,name_len);
  if (ispref) *(byte_ptr-1)=' ';
}

@ The |section_lookup| procedure is supposed to find a
section name that matches a new name, installing the new name if
it doesn't match an existing one. The new name is the string
between |first| and |last|; a ``match'' means that the new name
exactly equals or is a prefix or extension of a name in the tree.

@c
name_pointer
section_lookup( /* find or install section name in tree */
char *first,char *last, /* first and last characters of new name */
boolean ispref) /* is the new name a prefix or a full name? */
{
  int c=less; /* comparison between two names; initialized so some compilers won't complain */
  name_pointer p=root; /* current node of the search tree */
  name_pointer q=NULL; /* another place to look in the tree */
  name_pointer r=NULL; /* where a match has been found */
  name_pointer par=NULL; /* parent of |p|, if |r| is |NULL|;
            otherwise parent of |r| */
  size_t name_len=(size_t)(last-first+1);
  @<Look for matches for new name among shortest prefixes, complaining
        if more than one is found@>@;
  @<If no match found, add new name to tree@>@;
  @<If one match found, check for compatibility and return match@>@;
}

@ A legal new name matches an existing section name if and only if it
matches the shortest prefix of that section name.  Therefore we can
limit our search for matches to shortest prefixes, which eliminates
the need for chunk-chasing at this stage.

@<Look for matches for new name among...@>=
while (p) { /* compare shortest prefix of |p| with new name */
  c=web_strcmp(first,name_len,first_chunk(p),prefix_length(p));
  if (c==less || c==greater) { /* new name does not match |p| */
    if (r==NULL) /* no previous matches have been found */
      par=p;
    p=(c==less?p->llink:p->rlink);
  } else { /* new name matches |p| */
    if (r!=NULL) { /* and also |r|: illegal */
      fputs("\n! Ambiguous prefix: matches <",stdout);
@.Ambiguous prefix ... @>
      print_prefix_name(p);
      fputs(">\n and <",stdout);
      print_prefix_name(r);
      err_print(">");
      return name_dir; /* the unsection */
    }
    r=p; /* remember match */
    p=p->llink; /* try another */
    q=r->rlink; /* we'll get back here if the new |p| doesn't match */
  }
  if (p==NULL)
    p=q, q=NULL; /* |q| held the other branch of |r| */
}

@ @<If no match ...@>=
  if (r==NULL) /* no matches were found */
    return add_section_name(par,c,first,last+1,ispref);

@ Although error messages are given in anomalous cases, we do return the
unique best match when a discrepancy is found, because users often
change a title in one place while forgetting to change it elsewhere.

@<If one match found, check for compatibility and return match@>=
switch(section_name_cmp(&first,name_len,r)) {
              /* compare all of |r| with new name */
  case prefix:
    if (!ispref) {
      fputs("\n! New name is a prefix of <",stdout);
@.New name is a prefix...@>
      print_section_name(r);
      err_print(">");
    }
    else if (name_len<prefix_length(r)) set_prefix_length(r,name_len);
    /* fall through */
  case equal: break;
  case extension: if (!ispref || first<=last)
        extend_section_name(r,first,last+1,ispref);
      break;
  case bad_extension:
      fputs("\n! New name extends <",stdout);
@.New name extends...@>
      print_section_name(r);
      err_print(">");
    break;
  default: /* no match: illegal */
    fputs("\n! Section name incompatible with <",stdout);
@.Section name incompatible...@>
    print_prefix_name(r);
    fputs(">,\n which abbreviates <",stdout);
    print_section_name(r);
    err_print(">");
}
return r;

@ The return codes of |section_name_cmp|, which compares a string with
the full name of a section, are those of |web_strcmp| plus
|bad_extension|, used when the string is an extension of a
supposedly already complete section name.  This function has a side
effect when the comparison string is an extension: It advances the
address of the first character of the string by an amount equal to
the length of the known part of the section name.

The name \.{@@<foo...@@>} should be an acceptable ``abbreviation''
for \.{@@<foo@@>}. If such an abbreviation comes after the complete
name, there's no trouble recognizing it. If it comes before the
complete name, we simply append a null chunk. This logic requires
us to regard \.{@@<foo...@@>} as an ``extension'' of itself.

@d bad_extension 5

@c
static int section_name_cmp(
char **pfirst, /* pointer to beginning of comparison string */
size_t len, /* length of string */
name_pointer r) /* section name being compared */
{
  char *first=*pfirst; /* beginning of comparison string */
  name_pointer q=r+1; /* access to subsequent chunks */
  char *ss, *s=first_chunk(r);
  int c=less; /* comparison */
  boolean ispref; /* is chunk |r| a prefix? */
  while (true) {
    ss=(r+1)->byte_start-1;
    if (*ss==' ' && ss>=r->byte_start) ispref=true,q=q->link;
    else ispref=false,ss++,q=name_dir;
    switch(c=web_strcmp(first,len,s,(size_t)(ss-s))) {
    case equal: if (q==name_dir)
        if (ispref) {
          *pfirst=first+(ptrdiff_t)(ss-s);
          return extension; /* null extension */
        } else return equal;
      else return (q->byte_start==(q+1)->byte_start)? equal: prefix;
    case extension:
      if (!ispref) return bad_extension;
      first += ss-s;
      if (q!=name_dir) {len -= (int)(ss-s); s=q->byte_start; r=q; continue;}
      *pfirst=first; return extension;
    default: return c;
    }
  }
}

@ @<Predec...@>=@+static int section_name_cmp(char **,size_t,name_pointer);

@** Reporting errors to the user.
A global variable called |history| will contain one of four values
at the end of every run: |spotless| means that no unusual messages were
printed; |harmless_message| means that a message of possible interest
was printed but no serious errors were detected; |error_message| means that
at least one error was found; |fatal_message| means that the program
terminated abnormally. The value of |history| does not influence the
behavior of the program; it is simply computed for the convenience
of systems that might want to use such information.

@<Global var...@>=
int history=spotless; /* indicates how bad this run was */

@ The command `|err_print("! Error message")|' will report a syntax error to
the user, by printing the error message at the beginning of a new line and
then giving an indication of where the error was spotted in the source file.
Note that no period follows the error message, since the error routine
will automatically supply a period. A newline is automatically supplied
if the string begins with |"!"|.

@c
void
err_print( /* prints `\..' and location of error message */
const char *s)
{
  *s=='!'? printf("\n%s",s) : printf("%s",s);
  if (web_file_open) @<Print error location based on input buffer@>@;
  update_terminal; mark_error;
}

@ The error locations can be indicated by using the global variables
|loc|, |cur_line|, |cur_file_name| and |changing|,
which tell respectively the first
unlooked-at position in |buffer|, the current line number, the current
file, and whether the current line is from |change_file| or |cur_file|.
This routine should be modified on systems whose standard text editor
has special line-numbering conventions.
@^system dependencies@>

@<Print error location based on input buffer@>=
{char *k,*l; /* pointers into |buffer| */
if (changing && include_depth==change_depth)
  printf(". (l. %d of change file)\n", change_line);
else if (include_depth==0) printf(". (l. %d)\n", cur_line);
  else printf(". (l. %d of include file %s)\n", cur_line, cur_file_name);
l= (loc>=limit? limit: loc);
if (l>buffer) {
  for (k=buffer; k<l; k++)
    if (*k=='\t') putchar(' ');
    else putchar(*k); /* print the characters already read */
  new_line;
  for (k=buffer; k<l; k++) putchar(' '); /* space out the next line */
}
for (k=l; k<limit; k++) putchar(*k); /* print the part not yet read */
if (*limit=='|') putchar('|'); /* end of \CEE/ text in section names */
putchar(' '); /* to separate the message from future asterisks */
}

@ When no recovery from some error has been provided, we have to wrap
up and quit as graciously as possible.  This is done by calling the
function |wrap_up| at the end of the code.

\.{CTANGLE} and \.{CWEAVE} have their own notions about how to
print the job statistics.  See the function(s) |print_stats| in the
interface above and in the index.

Some implementations may wish to pass the |history| value to the
operating system so that it can be used to govern whether or not other
programs are started. Here, for instance, we pass the operating system
a status of |EXIT_SUCCESS| if and only if only harmless messages were printed.
@^system dependencies@>

@c
int wrap_up(void) {
  if (show_progress) new_line;
  if (show_stats)
    print_stats(); /* print statistics about memory usage */
  @<Print the job |history|@>@;
  if (history > harmless_message) return EXIT_FAILURE;
  else return EXIT_SUCCESS;
}

@ @<Print the job |history|@>=
switch (history) {
case spotless:
  if (show_happiness) puts("(No errors were found.)"); break;
case harmless_message:
  puts("(Did you see the warning message above?)"); break;
case error_message:
  puts("(Pardon me, but I think I spotted something wrong.)"); break;
case fatal_message: default:
  puts("(That was a fatal error, my friend.)");
}

@ When there is no way to recover from an error, the |fatal| subroutine is
invoked. This happens most often when |overflow| occurs.

The two parameters to |fatal| are strings that are essentially
concatenated to print the final error message.

@c void
fatal(
  const char *s,const char *t)
{
  if (*s) err_print(s);
  err_print(t);
  history=fatal_message; exit(wrap_up());
}

@ An overflow stop occurs if \.{CWEB}'s tables aren't large enough.

@c void
overflow(
  const char *t)
{
  printf("\n! Sorry, %s capacity exceeded",t); fatal("","");
}
@.Sorry, capacity exceeded@>

@ Sometimes the program's behavior is far different from what it should be,
and \.{CWEB} prints an error message that is really for the \.{CWEB}
maintenance person, not the user. In such cases the program says
|confusion("indication of where we are")|.

@** Command line arguments.
The user calls \.{CWEAVE} and \.{CTANGLE} with arguments on the command line.
These are either file names or flags to be turned off (beginning with |"-"|)
or flags to be turned on (beginning with |"+"|).
The following globals are for communicating the user's desires to the rest
of the program. The various file name variables contain strings with
the names of those files. Most of the 128 flags are undefined but available
for future extensions.

@<Global var...@>=
int argc; /* copy of |ac| parameter to |main| */
char **argv; /* copy of |av| parameter to |main| */
char C_file_name[max_file_name_length]; /* name of |C_file| */
char tex_file_name[max_file_name_length]; /* name of |tex_file| */
char idx_file_name[max_file_name_length]; /* name of |idx_file| */
char scn_file_name[max_file_name_length]; /* name of |scn_file| */
boolean flags[128]; /* an option for each 7-bit code */

@ The |flags| will be initially |false|. Some of them are set to~|true| before
scanning the arguments; if additional flags are |true| by default they
should be set before calling |common_init|.

@<Set the default options common to \.{CTANGLE} and \.{CWEAVE}@>=
show_banner=show_happiness=show_progress=make_xrefs=true;

@ We now must look at the command line arguments and set the file names
accordingly.  At least one file name must be present: the \.{CWEB}
file.  It may have an extension, or it may omit the extension to get |".w"| or
|".web"| added.  The \TEX/ output file name is formed by replacing the \.{CWEB}
file name extension by |".tex"|, and the \CEE/ file name by replacing
the extension by |".c"|, after removing the directory name (if any).

If there is a second file name present among the arguments, it is the
change file, again either with an extension or without one to get |".ch"|.
An omitted change file argument means that |"/dev/null"| should be used,
when no changes are desired.
@^system dependencies@>

If there's a third file name, it will be the output file.

@c
static void
scan_args(void)
{
  char *dot_pos; /* position of |'.'| in the argument */
  char *name_pos; /* file name beginning, sans directory */
  register char *s; /* register for scanning strings */
  boolean found_web=false,found_change=false,found_out=false;
             /* have these names been seen? */

  strcpy(change_file_name,"/dev/null");
  while (--argc > 0) {
    if ((**(++argv)=='-'||**argv=='+')&&*(*argv+1)) @<Handle flag argument@>@;
    else {
      s=name_pos=*argv;@+dot_pos=NULL;
      while (*s)
        if (*s=='.') dot_pos=s++;
        else if (*s=='/') dot_pos=NULL,name_pos=++s;
        else s++;
      if (!found_web) @<Make
        |web_file_name|, |tex_file_name|, and |C_file_name|@>@;
      else if (!found_change) @<Make |change_file_name|@>@;
      else if (!found_out) @<Override |tex_file_name| and |C_file_name|@>@;
        else @<Print usage error message and quit@>@;
    }
  }
  if (!found_web) @<Print usage error message and quit@>@;
}

@ @<Pred...@>=@+static void scan_args(void);

@ We use all of |*argv| for the |web_file_name| if there is a |'.'| in it,
otherwise we add |".w"|. If this file can't be opened, we prepare an
|alt_web_file_name| by adding |"web"| after the dot.
The other file names come from adding other things
after the dot.  We must check that there is enough room in
|web_file_name| and the other arrays for the argument.

@<Make |web_file_name|...@>=
{
  if (s-*argv > max_file_name_length-5)
    @<Complain about argument length@>@;
  if (dot_pos==NULL)
    sprintf(web_file_name,"%s.w",*argv);
  else {
    strcpy(web_file_name,*argv);
    *dot_pos='\0'; /* string now ends where the dot was */
  }
  sprintf(alt_web_file_name,"%s.web",*argv);
  sprintf(tex_file_name,"%s.tex",name_pos); /* strip off directory name */
  sprintf(idx_file_name,"%s.idx",name_pos);
  sprintf(scn_file_name,"%s.scn",name_pos);
  sprintf(C_file_name,"%s.c",name_pos);
  found_web=true;
}

@ @<Make |change_file_name|...@>=
{
  if (strcmp(*argv,"-")!=0) {
    if (s-*argv > max_file_name_length-4)
      @<Complain about argument length@>@;
    if (dot_pos==NULL)
      sprintf(change_file_name,"%s.ch",*argv);
    else strcpy(change_file_name,*argv);
  }
  found_change=true;
}

@ @<Override...@>=
{
  if (s-*argv > max_file_name_length-5)
    @<Complain about argument length@>@;
  if (dot_pos==NULL) {
    sprintf(tex_file_name,"%s.tex",*argv);
    sprintf(idx_file_name,"%s.idx",*argv);
    sprintf(scn_file_name,"%s.scn",*argv);
    sprintf(C_file_name,"%s.c",*argv);
  } else {
    strcpy(tex_file_name,*argv);
    strcpy(C_file_name,*argv);
    if (make_xrefs) { /* indexes will be generated */
      *dot_pos='\0';
      sprintf(idx_file_name,"%s.idx",*argv);
      sprintf(scn_file_name,"%s.scn",*argv);
    }
  }
  found_out=true;
}

@ @d flag_change (**argv!='-')
@<Handle flag...@>=
for(dot_pos=*argv+1;*dot_pos>'\0';dot_pos++)
  flags[(eight_bits)*dot_pos]=flag_change;

@ @<Print usage error message and quit@>=
{
if (program==ctangle)
  fatal(
"! Usage: ctangle [options] webfile[.w] [{changefile[.ch]|-} [outfile[.c]]]\n"
   ,"");
@.Usage:@>
else fatal(
"! Usage: cweave [options] webfile[.w] [{changefile[.ch]|-} [outfile[.tex]]]\n"
   ,"");
}

@ @<Complain about arg...@>= fatal("! Filename too long\n", *argv);
@.Filename too long@>

@** Output. Here is the code that opens the output file:
@^system dependencies@>

@<Global var...@>=
FILE *C_file; /* where output of \.{CTANGLE} goes */
FILE *tex_file; /* where output of \.{CWEAVE} goes */
FILE *idx_file; /* where index from \.{CWEAVE} goes */
FILE *scn_file; /* where list of sections from \.{CWEAVE} goes */
FILE *active_file; /* currently active file for \.{CWEAVE} output */

@ @<Scan arguments and open output files@>=
scan_args();
if (program==ctangle) {
  if ((C_file=fopen(C_file_name,"wb"))==NULL)
    fatal("! Cannot open output file ", C_file_name);
@.Cannot open output file@>
}
else {
  if ((tex_file=fopen(tex_file_name,"wb"))==NULL)
    fatal("! Cannot open output file ", tex_file_name);
}

@** Index.
