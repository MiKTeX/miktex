% This is the cweb file sjisconv.w of the CJK Package Ver. 4.8.4  18-Apr-2015

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
%           cweave +a sjisconv.w
%
% This (fully compatible) CWEAVE can transform CWEB-files with alternative
% output rules (look at the position of braces below!) the author (it's me
% too :-) prefer. Otherwise this file will be formatted traditionally.

\def\title{sjisconv (CJK Version 4.8.4)}

\def\topofcontents{
  \null\vfill
  \centerline{\titlefont The {\ttitlefont sjisconv} program}
  \vskip 20pt
  \centerline{(CJK Version 4.8.4)}
  \vfill}

\def\botofcontents{
  \vfill
  \noindent
  Copyright \copyright\ 1996, 1998 by Werner Lemberg
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
This small program will convert SJIS encoded Japanese characters into a
`preprocessed' form. The need of this program arises from the fact that this
encoding uses the characters `\.{\\}', `\.{\{}', and `\.{\}}' which have
special meanings in \TeX.

Use this program as a filter:

\hskip 2em \.{sjisconv < input\_file > output\_file}


@* The program.
The only function of this program is to replace all occurrences of SJIS
encoded two byte characters \.{XY} with
\.{\char94 \char94 7fX\char94 \char94 7fZZZ\char94 \char94 7f} (\.{X}
and \.{Y} are the first and the second byte of the character; \.{ZZZ}
represents the second byte as a decimal number).

Additionally we define a \TeX\ macro at the very beginning to signal a
preprocessed file.

The following code is very simple. No error detection is done because \TeX\
which will see the output of \.{sjisconv} complains loudly if something is
wrong.

Note that the user-defined character area of SJIS (with the first bytes in
the range 0xF0--0xFC) is not supported because it is not portable.

@d banner
"sjisconv (CJK ver. 4.8.4)"

@c
#include <stdio.h>
#include <stdlib.h>@#


int main(int argc, char *argv[])
   {int ch;

    fprintf(stdout, "\\def\\CJKpreproc{%s}", banner);@#

    ch = fgetc(stdin);@#

    while(!feof(stdin))
       {if((ch >= 0x81 && ch <= 0x9F) || (ch >= 0xE0 && ch <= 0xEF))
           {fprintf(stdout, "\177%c\177", ch);@#

            ch = fgetc(stdin);
            if(!feof(stdin))
                fprintf(stdout, "%d\177", ch);
           }
        else
            fputc(ch, stdout);@#

        ch = fgetc(stdin);
       }
    exit(EXIT_SUCCESS);
    return 0;           /* never reached */
   }
