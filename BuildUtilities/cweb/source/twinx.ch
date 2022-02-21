Changes for the TWINX utility from the CTWILL tarball.

This minimal set of changes tries to satisfy the GCC compiler
and it fixes a few minor issues. See the comments after '@x'.

This file is not copyrighted and can be used freely.

Limbo.

@x l.1
\datethis
@y
\datethis
\let\maybe=\iffalse % print only sections that change
@z

Section 1.

@x l.10 Standard C interface.
#include <stdio.h>
@y
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
@z

@x l.15
main(argc,argv)
  int argc;
  char *argv[];
@y
int main(
  int argc,
  char *argv[])
@z

@x l.21
    f=fopen(*++argv,"r");
    if (!f)
@y
    if((f=fopen(*++argv,"r"))==NULL)
@z

@x l.28
      f=fopen(*argv,"r");
      if (!f)
@y
      if((f=fopen(*argv,"r"))==NULL)
@z

Section 3.

@x l.49 Document minor change in behavior.
@ @<Scan file |f|...@>=
@y
@ For your convenience, \.{TWINX} grabs the first ``word'' in \.{\\title} and
turns it into uppercase form.

@<Scan file |f|...@>=
@z

@x l.57 FIX: Fetch only the first word from the '\title'.
    for (p=buf+11,q=title;*p && *p!='}';p++) *q++=*p;
@y
    for (p=buf+11,q=title;*p&&*p!=' '&&*p!='}';p++) *q++=toupper(*p);
@z

Section 4.

@x l.96
@<Type...@>=
@y
@s node_struct int

@<Type...@>=
@z

@x l.102 Compiler warning.
  char *id;
@y
  const char *id;
@z

Section 5.

@x l.115
char *save_string(s)
  char *s;
@y
char *save_string(
  char *s)
@z

Section 6.

@x l.145
node *new_node()
@y
node *new_node(void)
@z

Section 11.

@x l.216 FIX: Don't count masked braces.
    if (*p=='{') bal++;
    else if (*p=='}') bal--;
@y
    switch (*p) {
    case '\\': *q++=*p++; break;
    case '{': bal++; break;
    case '}': bal--; break;
    }
@z

Section 17.

@x l.347
int compare(p,q)
  node *p,*q;
@y
int compare(
  node *p, node *q)
@z

Section 19.

@x l.379 Compiler warning.
  for (j=1;collate[j];j++) ord[collate[j]]=j;
@y
  for (j=1;collate[j];j++) ord[(int)collate[j]]=j;
@z

Section 20.

@x l.390
collapse(p,q)
  node *p,*q;
@y
void collapse(
  node *p, node *q)
@z

Section 22.

@x l.414 Compiler warning.
{@+register char *p=x->id;
@y
{@+register const char *p=x->id;
@z

@x l.434 FIX: Don't mask already masked underscore.
    if (*p=='_') putchar('\\');
@y
    if (*p=='_'&&*(p-1)!='\\') putchar('\\');
@z
