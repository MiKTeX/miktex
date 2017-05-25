%option never-interactive

%{
  /* $Id: tex2xindy.l,v 1.24 2010/05/10 23:02:17 jschrod Exp $
     ============================================================
     (history at end)

=head1 NAME

tex2xindy - a preprocessor of the xindy index processor

=head1 SYNOPSIS

 tex2xindy [-o] [attr_file]

=head1 DESCRIPTION

B<tex2xindy> transforms a LaTeX index file C<.idx> (or an C<.aux>
file) into a B<xindy> raw index file.

It is a filter that reads from F<stdin> a file in the input format of
LaTeX's raw index file, i.e., with C<\indexentry> tags. It outputs on
F<stdout> a B<xindy> raw index file, i.e., with C<indexentry> clauses.

If the option B<-o> is not specified, B<tex2xindy> handles ^^-notation
of TeX and outputs the octet that is represented: C<^^ab> in the input
gets output as the octet 0xab. If C<^^^^abcd> or C<^^^^^^^^abcdefab>
are detected, they are output as is.

If the option B<-o> is specified, B<tex2xindy> operates in I<Omega
mode> and handles its ^^-notation: Then C<^^ab>, C<^^^^abcd>, and
C<^^^^^^^^abcdefab> represent Unicode characters with code points
0xab, 0xabcd, and 0xabcdefab respectively. They are output in UTF-8
encoding.

If the optional argument F<attr_file> is specified, B<tex2xindy>
writes all index key attributes into this file.


=head1 DEFICITS

This program was written since it was not easily possible to extract
the parser from the old makeindex system. Therefore it does not find
all errors in the input as the makeindex(1) version.

Additionally it uses only the default input specifiers of
makeindex(1). If other input specifiers (cf. manual page of
makeindex(1)) are needed, the input specifiers (starting from the
pattern C<KEYWORD>, see below) must be changed and the program must be
recompiled.

The particular missing feature is configuration of the quote and the
actual characters, maybe also the escape, subitem (level), and encap
characters. Argument and range delimiters seem to be less of a problem.

In fact, input markup handling (and thus B<tex2xindy>) should be
incorporated into the B<xindy> kernel, to be able to specify
configuration in xindy style files.

=head1 SEE ALSO

texindy(1),
xindy(1),
makeindex(1)

=head1 AUTHOR

Roger Kehr, Institut fuer Theoretische Informatik, TU Darmstadt

=head1 COPYRIGHT AND LICENSE

Copyright (c) 1996,1997 Roger Kehr.
Copyright (c) 2006 Joachim Schrod.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

=cut

*/
%}

%top{
#if defined(MIKTEX)
#define YY_NO_UNISTD_H
#include <miktex/First.h>
#include <miktex/Definitions.h>
#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#  include <io.h>
#endif
#include <getopt.h>
#include <fcntl.h>
#define register
#endif
#include <stdio.h>
#include <stdlib.h>
#if !defined(MIKTEX)
#ifdef WIN32
# include <fcntl.h>
# ifndef __MINGW32__  /* this should be W32TeX */
#  include <kpathsea/getopt.h>
#  define YY_NO_UNISTD_H 1
#  define __STDC_VERSION__ 199901L
# endif
#else
# include <unistd.h>
#endif
#endif
}

%{
#define QUOTE_ECHO qs(yytext)

FILE* attrfd = NULL;

static int lineno    = 1;
static int braces    = 0;
static int xref_mode = 0;
static int glos_mode = 0;
static int errors    = 0;
static int omega_mode = 0;

#define multiple_err \
    { \
        fprintf(stderr, "line %d: multiple %s's\n", lineno, yytext); \
	errors++; \
    }

#define NO_RANGE     0
#define OPEN_RANGE   1
#define CLOSE_RANGE  2

int range = NO_RANGE;

/* This function quotes a character, if necessary. */

static void qc( char ch ) {
  switch (ch) {
    case '\\' :
    case '"'  :
	putchar( '\\' );
	break;
  }
  putchar(ch);
}

/* Quote an entire string. */

static void qs( char* str ) {
  while ( *str ) {
    qc( *str++ );
  }
}

/*
   Output an Omega character, ^^-notation in yytext, in UTF-8 encoding.
   The parameter gets passed the amount of hex digits in the ^^-notation.
   In front of the hex digits are the same amount of ^ chars.
*/
static void omega_output_utf8 ( int width )
{
    unsigned long int c = strtoul (yytext+width, NULL, 16);
    if (c < 0x80) {
	putchar (c);
    }
    else if (c < 0x800) {
	putchar (0xC0 | (c>>6));
	putchar (0x80 | (c & 0x3F));
    }
    else if (c < 0x10000) {
	putchar (0xE0 | (c>>12));
	putchar (0x80 | (c>>6 & 0x3F));
	putchar (0x80 | (c & 0x3F));
    }
    else if (c < 0x200000) {
	putchar (0xF0 | (c>>18));
	putchar (0x80 | (c>>12 & 0x3F));
	putchar (0x80 | (c>>6 & 0x3F));
	putchar (0x80 | (c & 0x3F));
    }
    else if (c < 0x4000000) {
	putchar (0xF8 | (c>>24));
	putchar (0x80 | (c>>18 & 0x3F));
	putchar (0x80 | (c>>12 & 0x3F));
	putchar (0x80 | (c>>6 & 0x3F));
	putchar (0x80 | (c & 0x3F));
    }
    else if (c < 0x80000000) {
	putchar (0xFC | (c>>30));
	putchar (0x80 | (c>>24 & 0x3F));
	putchar (0x80 | (c>>18 & 0x3F));
	putchar (0x80 | (c>>12 & 0x3F));
	putchar (0x80 | (c>>6 & 0x3F));
	putchar (0x80 | (c & 0x3F));
    }
    else { /* out of range, output U+FFFD REPLACEMENT CHARACTER */
	fputs("\xef\xbf\xbd",stdout);
	fprintf (stderr, "line %d: character %#lx not in UTF-8 range\n",
		 lineno, c);
    }
}

%}

KEYWORD  \\indexentry
KEYWORDG \\glossaryentry
ENCAP    \|
ACTUAL   @
ESCAPE   \\
LEVEL    !
IS       :
QUOTE    \"
ROPEN    \(
RCLOSE   \)
ARGOPEN  \{
ARGCLOSE \}

HATHAT8BIT	\^{2}[0-9a-fA-F]{2}
HATHAT16BIT	\^{4}[0-9a-fA-F]{4}
HATHAT32BIT	\^{8}[0-9a-fA-F]{8}

%s key print skip2location location encap xref is skiplocation

%%

<INITIAL>{KEYWORD}[ \t]*{ARGOPEN} {
			  printf("(indexentry :tkey ((\"");
			  xref_mode = 0;
			  glos_mode = 0;
			  BEGIN(key);
			}

<INITIAL>{KEYWORDG}[ \t]*{ARGOPEN} {
			  printf("(indexentry :tkey ((\"");
			  xref_mode = 0;
			  glos_mode = 1;
			  BEGIN(key);
			}

{ESCAPE}{QUOTE}        { QUOTE_ECHO; }
{QUOTE}\n              { qc(yytext[1]); lineno++; }
{QUOTE}.               { qc(yytext[1]); }
<key,print>{ENCAP}     {
			 printf("\")) :attr \"");
                         range = NO_RANGE;
			 BEGIN(encap);
		       }
<key,print>{IS}        {
			 if ( glos_mode ) {
                             printf("\")) :attr \"is\" :xref (\"");
                             range = NO_RANGE;
			     if (attrfd) fprintf(attrfd, "\txref\n");
			     BEGIN(is);
			 } else {
			     ECHO;
			 }
		       }
<key>{LEVEL}           { printf("\") (\""); }
<print>{LEVEL}         { printf("\") (\""); BEGIN(key); }
<key,print>{ARGOPEN}   { ECHO; braces++; }
<key,print>{ARGCLOSE}  {
			 if ( braces > 0 ) {
			     ECHO;
			     braces--;
			 } else {
			     printf("\"))");
			     BEGIN(skip2location);
			 }
                       }

<key>{ACTUAL}          { printf("\" \""); BEGIN(print); }
<print>{ACTUAL}        { multiple_err; }

<encap>{ENCAP}         { multiple_err; }
<encap>{ROPEN}         { range = OPEN_RANGE; }
<encap>{RCLOSE}        { range = CLOSE_RANGE; }
<encap>{ARGOPEN}       { /* close :attr "... */
                         printf("\" :xref (\"");
			 if (attrfd) fprintf(attrfd, "\txref\n");
		         braces++;
		         BEGIN(xref);
                       }
<encap>{ARGCLOSE}      {
			 if ( braces > 0 ) {
			     ECHO;
			     braces--;
			 } else {
			     putchar('"');
			     if (attrfd) fprintf(attrfd, "\tlocref\n");
			     switch (range) {
			       case OPEN_RANGE:
				   printf(" :open-range");
				   break;
			       case CLOSE_RANGE:
				   printf(" :close-range");
				   break;
			     }
			     BEGIN(skip2location);
			 }
                       }
<encap>.|\n            {
			 QUOTE_ECHO;
                         if (attrfd)  fwrite(yytext, sizeof(char), 1, attrfd);
			 if (*yytext == '\n')  lineno++;
	               }



<xref>{LEVEL}          { printf("\" \""); }
<xref,is>{ARGOPEN}     { ECHO; braces++; }
<xref>{ARGCLOSE}       {
			 if ( braces > 1 ) {
			     ECHO;
			     braces--;
			 } else if ( braces == 1 ) {
			     braces--;
			 } else {
			     printf("\")");
			     xref_mode = 1;
			     BEGIN(skip2location);
		         }
                       }
<is>{ARGCLOSE}         {
			 if ( braces > 0 ) {
			     ECHO;
			     braces--;
			 } else {
			     printf("\")");
			     xref_mode = 1;
			     BEGIN(skip2location);
		         }
                       }

<skip2location>[ \t]*{ARGOPEN} {
                         if (xref_mode) {
			     BEGIN(skiplocation);
			 } else {
			     printf(" :locref \"");
			     BEGIN(location);
			 }
                       }
<location>{ARGCLOSE}  { printf("\")\n"); BEGIN(0); }

<skiplocation>{ARGCLOSE} { printf(")\n"); BEGIN(0); }
<skiplocation>.		 { ; }

<key,print,xref,is,location>{HATHAT8BIT} {
			if ( omega_mode ) {
			    omega_output_utf8(2);
			} else {
			    unput( strtol(yytext+2, NULL, 16) );
			}
		      }
<key,print,xref,is,location>{HATHAT16BIT} {
			if ( omega_mode ) {
			    omega_output_utf8(4);
			} else {
			    ECHO;
			}
		      }
<key,print,xref,is,location>{HATHAT32BIT} {
			if ( omega_mode ) {
			    omega_output_utf8(8);
			} else {
			    ECHO;
			}
		      }

<key,print,xref,is,location>\n	      { ECHO; lineno++; }
<key,print,xref,is,location>.	      { QUOTE_ECHO; }

\n                    { lineno++; }
[ \t]		      { ; }
.                     {
			fprintf(stderr,
				"Encountered unexpected char '%c' on line %d\n",
				yytext[0], lineno);
		      }

%%

static void usage (void)
{
    fputs("This is tex2xindy, $Revision: 1.24 $.\n", stderr);
    fputs("usage: tex2xindy [-o] [attr_file]\n", stderr);
    exit (1);
}

#if defined(MIKTEX)
int MIKTEXCEECALL Main(int argc, char ** argv)
#else
int
main(int argc, char* argv[])
#endif
{
    int option;

#ifdef WIN32
    setmode (fileno(stdout), _O_BINARY);
#else
    extern char *optarg;
    extern int optind, optopt;
#endif

    while ( (option=getopt(argc, argv, ":o")) != -1 ) {
	switch (option) {
	  case 'o':
	      omega_mode = 1;
	      break;
	  case '?':
	  default:
	      fprintf(stderr, "Unrecognized option: -%c\n", optopt);
	      usage();
	      /* NOTREACHED */
	}
    }

    /* More than one argument left? */
    if ( optind < argc - 1 )  usage();

    /* attr_file specified? */
    if ( optind == argc - 1 ) {
	fprintf(stderr, "Writing attribute names to file \"%s\".\n",
		argv[optind]);
	if ( (attrfd = fopen(argv[optind], "wb")) == NULL ) {
	    perror (argv[optind]);
	    exit (1);
	}
    }

    yylex();
    if (attrfd) fclose(attrfd);
    return (errors > 0);		/* 1 or 0 */
}


/*
 *  Define dummy yywrap so we no longer depend on `libfl.a'.
 */
int yywrap (void)
{
    return 1;
}


/* ============================================================

  $Log: tex2xindy.l,v $
  Revision 1.24  2010/05/10 23:02:17  jschrod
      Use ANSI C function definitions. (Patch from peb.)

  Revision 1.23  2009/12/03 00:42:14  jschrod
      Define internal functions as static to silence gcc warnings.

  Revision 1.22  2009/12/03 00:36:09  jschrod
      Define yywrap(), to be independent from libfl.

  Revision 1.21  2006/07/30 09:44:04  jschrod
      While quote is not output, escape+quote *is* output.

  Revision 1.20  2006/07/18 21:40:45  jschrod
      Add parenthesis as recommended by gcc -Wall.
      Use ECHO instead of QUOTE_ECHO where quoting will not be needed.
      Code cleanup: sanitize braces and indentation.

  Revision 1.19  2006/07/18 21:26:34  jschrod
      Add man page as POD to tex2xindy source.
      Add support for ^^-notation, both in TeX and in new Omega mode.
      Add usage message and proper option parsing.

  Revision 1.18  2006/02/21 12:12:05  jschrod
      Don't output escape or quote characters.

  Revision 1.17  2006/02/21 02:05:00  jschrod
      Quote really quotes the next char, now.

  Revision 1.16  2005/06/27 20:49:06  jschrod
      Keywords may be inmidst a line, as Jörg Sommer showed.
      "." rule merging did not work for attributes, and for skiplocation.
      Output error message if we come across chars outside our FSA.

  Revision 1.15  2005/06/14 16:22:08  jschrod
      Fold in changes from Thomas Henlich, as of 2000-02-14:
  Paranoia fix: recognize KEYWORD only in INITIAL start condition.
  Recognize \n and count lines (multiline entries do exist!).
  Merged "." rules.
  Fixed error messages (multiple ...).

  Revision 1.14  2004/10/05 21:10:48  jschrod
      Handles quote character correctly (bug ticket 1006128).
  Test case: german-sty.

  Revision 1.13  2002/01/29 13:32:58  thenlich
  Fixed bug in crossref-to-subitem bugfix. The original one proposed by Roger Kehr
  was right, only I copied it wrongly (shame on me). Thanks to Torsten Bronger
  for pointing it out to me.

  Revision 1.12  1999/10/13 08:06:44  kehr
  Changed revision number message back to RCS-tag. No functional changes.

  Revision 1.11  1999/10/13 08:05:10  kehr
  Merged bugfix from Thomas Henlich with my own modifications.

   1.8 Fixed minor quoting bugs (for increased makeindex compatibility).
        1999-07-28 Thomas Henlich <henlich@mmers1.mw.tu-dresden.de>
       Fixed crossref to subitem.
        1999-09-23 Thomas Henlich <henlich@mmers1.mw.tu-dresden.de>

   1.7 Fixed a bug concerning writing to and closing non-existing file (stat)
       Fixed a bug concerning quoting the " character.
       Enhancement for processing glossary files.
        1999-06-14 Thomas Henlich <henlich@mmers1.mw.tu-dresden.de>

  Revision 1.10  1999/09/23 06:53:32  kehr
  Fixed wrong raw index generation for cross-ref keywords.

  Revision 1.9  1999/04/27 09:01:17  kehr
  Defined a clean exit code as suggested by Klaus Guntermann
  <guntermann@iti.informatik.tu-darmstadt.de>.

  Revision 1.8  1999/04/27 08:57:48  kehr
  Added quoting patch sent by Thomas Henlich <thenlich@Rcs1.urz.tu-dresden.de>

  Revision 1.7  1999/04/27 08:56:59  kehr
  Another checkin.

  Revision 1.6  1997/02/13 14:42:50  kehr
  Minor changes.

  Revision 1.5  1997/01/21 16:44:36  kehr
  Bugfix: State <print> was not changed back to <key> if {LEVEL} was found.

  Revision 1.4  1997/01/17 16:30:37  kehr
  Rewrote makeindex4 and heavily modified tex2xindy.

  Revision 1.3  1996/07/19  17:15:18  kehr
  Made this program comaptible with lex by correctly enclosing all
  actions--stupid bug, 'cause lex didn't complain and flex did some
  wonderful matching of all the actions such that no error was
  discovered.

  Revision 1.2  1996/07/11  14:16:55  kehr
  Complete Major checkin before changing the letter-groups.

  Revision 1.1  1996/06/24  08:53:57  kehr
  Initial checkin of the Makefile, README and tex2xindy.
*/
