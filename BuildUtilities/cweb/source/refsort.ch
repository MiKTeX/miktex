Changes for the REFSORT utility from the CTWILL tarball.

This minimal set of changes tries to satisfy the GCC compiler.

This file is not copyrighted and can be used freely.

@x
\datethis
@y
\datethis
\let\maybe=\iffalse % print only sections that change
@z

@x standard C library interface
#include "stdio.h"
#include "strings.h"
#include "ctype.h"
@y
#include <stdio.h>
#include <string.h>
#include <ctype.h>
@z

@x declare return type
main()
@y
int main()
@z

@x variable not used
{ register int k;
@y
{
@z

@x add 'Index.' section
  for (;*p;p++) *q++=*p;
}
@y
  for (;*p;p++) *q++=*p;
}

@* Index.
@z
