/*
 *  ChkTeX, resource file reader.
 *  Copyright (C) 1995-96 Jens T. Berger Thielemann
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  Contact the author at:
 *              Jens Berger
 *              Spektrumvn. 4
 *              N-0666 Oslo
 *              Norway
 *              E-mail: <jensthi@ifi.uio.no>
 *
 *
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include "ChkTeX.h"

#define QUOTE   '\"'            /* Char used to quote text containing blanks    */
#define ESCAPE  '!'             /* Char used to access special characters       */
#define CMNT    '#'             /* Char used as comment                         */

/* We cannot put semicolons here as with OPTION_DEFAULTS, since
 * sometimes LIST etc. will be defined to be empty.  Thus we must
 * never put a semicolon after RESOURCE_INFO where it's called since
 * such isolated semicolons can break certain compilers.  This can
 * mess with indentation a bit, but we just have to deal with it. */
#define RESOURCE_INFO \
  LCASE(Silent)    LIST(Linker)     LIST(IJAccent) LIST(Italic)\
  LIST(ItalCmd)    LIST(PostLink)   LIST(WipeArg)  LIST(VerbEnvir)\
  LIST(MathEnvir)  LIST(MathCmd)    LIST(TextCmd)  LIST(MathRoman)\
  LIST(HyphDash)   LIST(NumDash)    LIST(WordDash) LIST(DashExcpt)\
  LIST(CenterDots) LIST(LowDots)    LIST(OutFormat)\
  LIST(Primitives) LIST(NotPreSpaced) LIST(NonItalic) LIST(NoCharNext)\
  LNEMPTY(CmdLine) LNEMPTY(TeXInputs)\
  LCASE(Abbrev)    LCASE(UserWarn)  LIST(UserWarnRegex)\
  KEY(VerbClear, "|") KEY(QuoteStyle, "Traditional")  KEY(TabSize, "8")

#define LIST(a) extern struct WordList a;
#define LNEMPTY LIST
#define LCASE(a) LIST(a) LIST(a ## Case)
#define KEY(a,def) extern const char *a;

RESOURCE_INFO
#undef KEY
#undef LCASE
#undef LNEMPTY
#undef LIST
int ReadRC(const char *);


#endif /* RESOURCE */
