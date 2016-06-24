/*
 *  ChkTeX, header file for main program.
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

#ifndef FINDERRS_H
#define FINDERRS_H
#include "ChkTeX.h"


#undef ERRMSGS
#undef MSG

#define ERRMSGS \
 MSG(emMinFault, etErr, iuOK, ctNone,\
     INTERNFAULT)\
 MSG(emSpaceTerm, etWarn, iuOK, ctOutMath,\
     "Command terminated with space.")\
 MSG(emNBSpace, etWarn, iuOK, ctNone,\
     "Non-breaking space (`~') should have been used.")\
 MSG(emEnclosePar, etWarn, iuOK, ctInMath,\
     "You should enclose the previous parenthesis with `{}\'.")\
 MSG(emItInNoIt, etWarn, iuOK, ctNone,\
     "Italic correction (`\\/') found in non-italic buffer.")\
 MSG(emItDup, etWarn, iuOK, ctNone,\
     "Italic correction (`\\/') found more than once.")\
 MSG(emNoItFound, etWarn, iuOK, ctNone,\
     "No italic correction (`\\/') found.")\
 MSG(emAccent, etWarn, iuOK, ctNone,\
     "Accent command `%s' needs use of `\\%c%s'.")\
 MSG(emWrongDash, etWarn, iuOK, ctOutMath,\
     "Wrong length of dash may have been used.")\
 MSG(emExpectC, etWarn, iuOK, ctNone,\
     "`%s' expected, found `%s'.")\
 MSG(emSoloC, etWarn, iuOK, ctNone,\
     "Solo `%s' found.")\
 MSG(emEllipsis, etWarn, iuOK, ctNone,\
     "You should use %s to achieve an ellipsis.")\
 MSG(emInterWord, etWarn, iuOK, ctOutMath,\
     "Interword spacing (`\\ ') should perhaps be used.")\
 MSG(emInterSent, etWarn, iuOK, ctOutMath,\
     "Intersentence spacing (`\\@') should perhaps be used.")\
 MSG(emNoArgFound, etErr, iuOK, ctNone,\
     "Could not find argument for command.")\
 MSG(emNoMatchC, etWarn, iuOK, ctOutMath,\
     "No match found for `%s'.")\
 MSG(emMathStillOn, etWarn, iuOK, ctNone,\
     "Mathmode still on at end of LaTeX file.")\
 MSG(emNoMatchCC, etWarn, iuOK, ctNone,\
     "Number of `%c' doesn't match the number of `%c'!")\
 MSG(emUseQuoteLiga, etWarn, iuOK, ctNone,\
     "Use either `` or '' as an alternative to `\"'.")\
 MSG(emUseOtherQuote, etWarn, iuNotSys, ctNone,\
     "Use \"'\" (ASCII 39) instead  of \"\xB4\" (ASCII 180).")\
 MSG(emUserWarn, etWarn, iuOK, ctNone,\
     "User-specified pattern found: %s.")\
 MSG(emNotIntended, etWarn, iuNotSys, ctNone,\
     "This command might not be intended.")\
 MSG(emComment, etMsg, iuNotSys, ctNone,\
     "Comment displayed.")\
 MSG(emThreeQuotes, etWarn, iuOK, ctNone,\
     "Either %c\\,%c%c or %c%c\\,%c will look better.")\
 MSG(emFalsePage, etWarn, iuOK, ctOutMath,\
     "Delete this space to maintain correct pagereferences.")\
 MSG(emEmbrace, etWarn, iuOK, ctInMath,\
     "You might wish to put this between a pair of `{}'")\
 MSG(emSpacePunct, etWarn, iuOK, ctOutMath,\
     "You ought to remove spaces in front of punctuation.")\
 MSG(emNoCmdExec, etWarn, iuOK, ctNone,\
     "Could not execute LaTeX command.")\
 MSG(emItPunct, etWarn, iuOK, ctNone,\
     "Don't use \\/ in front of small punctuation.")\
 MSG(emUseTimes, etWarn, iuOK, ctNone,\
     "$\\times$ may look prettier here.")\
 MSG(emMultiSpace, etWarn, iuNotSys, ctOutMath,\
     "Multiple spaces detected in input.")\
 MSG(emIgnoreText, etWarn, iuOK, ctNone,\
    "This text may be ignored.")\
 MSG(emBeginQ, etWarn, iuOK, ctOutMath,\
   "Use ` to begin quotation, not '.")\
 MSG(emEndQ, etWarn, iuOK, ctOutMath,\
   "Use ' to end quotation, not `.")\
 MSG(emQuoteMix, etWarn, iuOK, ctNone,\
     "Don't mix quotes.")\
 MSG(emWordCommand, etWarn, iuOK, ctInMath,\
     "You should perhaps use `\\%s' instead.")\
 MSG(emSpaceParen, etWarn, iuOK, ctOutMath,\
     "You should put a space %s parenthesis.")\
 MSG(emNoSpaceParen, etWarn, iuOK, ctOutMath,\
     "You should avoid spaces %s parenthesis.")\
 MSG(emQuoteStyle, etWarn, iuOK, ctOutMath,\
     "You should not use punctuation %s quotes.")\
 MSG(emDblSpace, etWarn, iuOK, ctOutMath,\
     "Double space found.")\
 MSG(emPunctMath, etWarn, iuOK, ctNone,\
     "You should put punctuation %s math mode.")\
 MSG(emTeXPrim, etWarn, iuNotSys, ctNone,\
     "You ought to not use primitive TeX in LaTeX code.")\
 MSG(emRemPSSpace, etWarn, iuOK, ctOutMath,\
     "You should remove spaces in front of `%s'") \
 MSG(emNoCharMean, etWarn, iuOK, ctNone,\
     "`%s' is normally not followed by `%c'.")\
 MSG(emUserWarnRegex, etWarn, iuOK, ctNone,\
     "User Regex: %.*s.")

#undef MSG
#define MSG(num, type, inuse, ctxt, text) num,

enum ErrNum
{
    ERRMSGS emMaxFault
};
#undef MSG

enum Context
{
    ctNone = 0x00,
    ctInMath = 0x01,
    ctOutMath = 0x02,
    ctInHead = 0x04,
    ctOutHead = 0x08
};

struct ErrMsg
{
    enum ErrNum Number;
    enum
    {
        etMsg,
        etWarn,
        etErr
    } Type;

    enum
    {
        iuNotSys,
        iuNotUser,
        iuOK
    } InUse;

    /* Requirements posed upon environment */
    enum Context Context;

    const char *Message;
};

enum ItState
{
    itOff,
    itOn,
    itCorrected
};


enum DotLevel
{
    dtUnknown = 0x0,
    dtCDots = 0x1,
    dtLDots = 0x2,
    dtDots = 0x4
};

extern struct ErrMsg LaTeXMsgs[emMaxFault + 1];

extern char *OutputFormat;

int CheckSilentRegex(void);
int FindErr(const char *, const unsigned long);
void PrintError(const char *, const char *, const long, const long,
                const long, const enum ErrNum, ...);
void PrintStatus(unsigned long Lines);

#endif /* FINDERRS */
