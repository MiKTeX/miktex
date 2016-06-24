% This is the cweb file cefconv.w of the CJK Package Ver. 4.8.4  18-Apr-2015

% Copyright (C) 1994-2015  Werner Lemberg <wl@@gnu.org>
%
% This program is free software; you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation; either version 2 of the License, or
% (at your option) any later version.
%
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
%
% You should have received a copy of the GNU General Public License
% along with this program in doc/COPYING; if not, write to the Free
% Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
% MA 02110-1301 USA

% To print this CWEB file you should (but not must) use the CWEAVE of the
% c2cweb-package (found at the CTAN archives, e.g. ftp.dante.de) and then say
%
%           cweave +a cefconv.w
%
% This (fully compatible) CWEAVE can transform CWEB-files with alternative
% output rules (look at the position of braces below!) the author (it's me
% too :-) prefer. Otherwise this file will be formatted traditionally.

\def\title{cefconv (CJK Version 4.8.4)}

\def\topofcontents{
  \null\vfill
  \centerline{\titlefont The {\ttitlefont cefconv} program}
  \vskip 20pt
  \centerline{(CJK Version 4.8.4)}
  \vfill}

\def\botofcontents{
  \vfill
  \noindent
  Copyright \copyright\ 1996, 1998, 2002 by Werner Lemberg
  \bigskip\noindent
  Permission is granted to make and distribute verbatim copies of this
  document provided that the copyright notice and this permission notice
  are preserved on all copies.

  \smallskip\noindent
  Permission is granted to copy and distribute modified versions of this
  document under the conditions for verbatim copying, provided that the
  entire resulting derived work is distributed under the terms of a
  permission notice identical to this one.}

\pageno=\contentspagenumber \advance\pageno by 1
\let\maybe=\iftrue
\fullpageheight=240mm
\pageheight=223mm
\pagewidth=158mm
\setpage
\frenchspacing

\noinx
\nosecs
\nocon

@* Function and Use.
This small program will convert EUC encoding with CNS encoded Chinese
characters using the {\it Chinese Encoding Framework (CEF)} into a
`preprocessed' form, expanding the CEF macros.

Use this program as a filter:

\hskip 2em \.{cefconv < input\_file > output\_file}


@* The program.
In contrast to \.{cef5conv} or \.{cefsconv}, only one task will be executed:

Replacing CEF macros of the form \.{\&xx-yyzz;} (\.{xx} can be C1--C7 for
the CNS planes 1--7, C0 for Big~5 encoding, an encoding CX reserved for
IRIZ, a private encoding CY, and U for Unicode encoding; \.{yyzz} is a
hexadecimal representation of the code point in this plane) with
\medskip
\quad\.{\char94 \char94 7f72\char94 \char94 7fXX\char94 \char94 7f%
\char94 \char94 7f"0yy\char94 \char94 7f"0zz\char94 \char94 7f}\quad.
\medskip
\.{XX} is the corresponding CJK encoding of \.{xx}; the number `72'
specifies a macro in the file \.{MULEenc.sty} which further processes this
representation -- it is automatically loaded by the \.{CJK} package.

Additionally we define an empty \TeX\ macro at the very beginning to signal
a preprocessed file.

The following code is very simple. No error detection is done because \TeX\
which will see the output of \.{cefconv} complains loudly if something is
wrong.

@d banner
"cefconv (CJK ver. 4.8.4)"

@c
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>@#


int main(int argc, char *argv[])
   {int ch, i;
    unsigned char in[16];
    unsigned char out[32];
    unsigned char *inp, *outp;

    fprintf(stdout, "\\def\\CNSpreproc{%s}", banner);@#

    ch = fgetc(stdin);@#

    while(!feof(stdin))
       {if(ch == '&')  /* the macro test is hardcoded to make things simple */
           {inp = in;
            outp = out;
            *inp = ch;
            *(++inp) = fgetc(stdin);@#

            if(*inp == 'C' && !feof(stdin))
               {*(++inp) = fgetc(stdin);
                if(*inp == '0' && !feof(stdin))
                   {*(outp++) = 'B';
                    *(outp++) = 'g';
                    *(outp++) = '5';
                   }
                else if(*inp >= '1' && *inp <= '7' && !feof(stdin))
                   {*(outp++) = 'C';
                    *(outp++) = 'N';
                    *(outp++) = 'S';
                    *(outp++) = *inp;
                   }
                else if((*inp == 'X' || *inp == 'Y') && !feof(stdin))
                   {*(outp++) = 'C';
                    *(outp++) = 'E';
                    *(outp++) = 'F';
                    *(outp++) = *inp;
                   }
                else
                    goto no_macro;
               }
            else if(*inp == 'U' && !feof(stdin))
               {*(outp++) = 'U';
                *(outp++) = 'T';
                *(outp++) = 'F';
                *(outp++) = '8';
               }
            else
                goto no_macro;@#

            *(++inp) = fgetc(stdin);
            if(*inp != '-' || feof(stdin))
                goto no_macro;@#

            *(outp++) = '\177';
            *(outp++) = '\177';
            *(outp++) = '\"';
            *(outp++) = '0';@#

            *(++inp) = fgetc(stdin);
            if(isxdigit(*inp) && *inp < 0x80 && !feof(stdin))
                *(outp++) = toupper(*inp);
            else
                goto no_macro;@#

            *(++inp) = fgetc(stdin);
            if(isxdigit(*inp) && *inp < 0x80 && !feof(stdin))
                *(outp++) = toupper(*inp);
            else
                goto no_macro;@#

            *(outp++) = '\177';
            *(outp++) = '\"';
            *(outp++) = '0';@#

            *(++inp) = fgetc(stdin);
            if(isxdigit(*inp) && *inp < 0x80 && !feof(stdin))
                *(outp++) = toupper(*inp);
            else
                goto no_macro;@#

            *(++inp) = fgetc(stdin);
            if(isxdigit(*inp) && *inp < 0x80 && !feof(stdin))
                *(outp++) = toupper(*inp);
            else
                goto no_macro;@#

            *(outp++) = '\177';
            *outp = '\0';@#

            *(++inp) = fgetc(stdin);
            if(*inp != ';' || feof(stdin))
                goto no_macro;@#

            outp = out;
            fprintf(stdout, "\17772\177");
            while(*outp)
                fputc(*(outp++), stdout);@#

            ch = fgetc(stdin);
            continue;@#

no_macro:
            ch = *inp;
            i = inp - in;
            inp = in;
            while(i--)
                fputc(*(inp++), stdout);
            continue;
           }
        else
            fputc(ch, stdout);@#

        ch = fgetc(stdin);
       }
    exit(EXIT_SUCCESS);
    return 0;           /* never reached */
   }
