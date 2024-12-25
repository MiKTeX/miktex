/*
 *  ChkTeX, error searching & report routines.
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


#include "ChkTeX.h"
#include "FindErrs.h"
#include "OpSys.h"
#include "Utility.h"
#include "Resource.h"

#if HAVE_PCRE || HAVE_POSIX_ERE

#if HAVE_PCRE
#include <pcreposix.h>
#else
#include <regex.h>
#endif

#define REGEX_FLAGS REG_EXTENDED
#define NUM_MATCHES 10
#define ERROR_STRING_SIZE 100

regex_t* RegexArray = NULL;
regex_t* SilentRegex = NULL;
int NumRegexes = 0;

#endif

int FoundErr = EXIT_SUCCESS;
int LastWasComment = FALSE;
int SeenSpace = FALSE;
int FrenchSpacing = FALSE;

/***************************** ERROR MESSAGES ***************************/

#undef MSG
#define MSG(num, type, inuse, ctxt, text) {num, type, inuse, ctxt, text},

struct ErrMsg LaTeXMsgs[emMaxFault + 1] = {
    ERRMSGS {emMaxFault, etErr, iuOK, 0, INTERNFAULT}
};

#define istex(c)        (isalpha((unsigned char)c) || (AtLetter && (c == '@')))
#define CTYPE(func) \
static int my_##func(int c) \
{ \
   return(func(c)); \
}

#define SUPPRESSED_ON_LINE(c)  (LineSuppressions & ((uint64_t)1<<c))

#define INUSE(c) \
    ((LaTeXMsgs[(enum ErrNum) c].InUse == iuOK) && !SUPPRESSED_ON_LINE(c))

#define PSERR2(pos,len,err,a,b) \
    PrintError(err, CurStkName(&InputStack), RealBuf, pos, len, Line, a, b)

#define PSERRA(pos,len,err,a) \
    PrintError(err, CurStkName(&InputStack), RealBuf, pos, len, Line, a)

#define HEREA(len, err, a)     PSERRA(BufPtr - Buf - 1, len, err, a)
#define PSERR(pos,len,err)     PSERRA(pos,len,err,"")

#define HERE(len, err)         HEREA(len, err, "")

#define SKIP_BACK(ptr, c, check) \
    while((c = *ptr--)) \
    { \
        if (!(check))  break; \
    } \
    ptr++;

#define SKIP_AHEAD(ptr, c, check) \
    while((c = *ptr++)) \
    { \
        if (!(check)) \
            break; \
    } \
    ptr--;


/*  -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=-  */

/*
 * A list of characters LaTeX considers as an end-of-sentence characters, which
 * should be detected when whether sentence spacing is correct.
 *
 */
static const char LTX_EosPunc[] = { '.', ':', '?', '!', 0 };

/*
 * General punctuation characters used on your system.
 */
static const char LTX_GenPunc[] = { ',', ';', 0 };

/*
 * A list of characters LaTeX considers as an small punctuation characters,
 * which should not be preceded by a \/.
 */
static const char LTX_SmallPunc[] = { '.', ',', 0 };

/*
 * A list of characters that could be considered to start a new
 * sentence, or not.
 *
 * This allows "Mr. ``X'' " to warn about the same as "Mr. X".
 *
 */
static const char LTX_BosPunc[] = {'`', '(', '[', 0};

/*
 * String used to delimit a line suppression.  This string must be
 * followed immediately by the number of the warning to be suppressed.
 * If more than one warning is to be suppressed, then multiple copies
 * of LineSuppDelim+number must be used.
 */
const char LineSuppDelim[] = "chktex ";

/*
 * String used to delimit a file suppression.  This string must be
 * followed immediately by the number of the warning to be suppressed.
 * If more than one warning is to be suppressed, then multiple copies
 * of FileSuppDelim+number must be used.
 */
const char FileSuppDelim[] = "chktex-file ";

/*
 * A bit field used to hold the suppressions for the current line.
 */
static uint64_t LineSuppressions;
/*
 * A bit field used to hold the suppressions of numbered user warnings
 * for the current line.
 */
static uint64_t UserLineSuppressions;

static unsigned long Line;

static const char *RealBuf;
static char *BufPtr;

static int ItFlag = efNone;
static int MathFlag = efNone;

NEWBUF(Buf, BUFFER_SIZE);
NEWBUF(CmdBuffer, BUFFER_SIZE);
NEWBUF(ArgBuffer, BUFFER_SIZE);

static enum ErrNum PerformCommand(const char *Cmd, char *Arg);

#ifdef isdigit
CTYPE(isdigit)
#else
#  define my_isdigit isdigit
#endif

#ifdef isalpha
CTYPE(isalpha)
#else
#  define my_isalpha isalpha
#endif

/*
 * Reads in a TeX token from Src and puts it in Dest.
 *
 */


static char *GetLTXToken(char *Src, char *Dest)
{
    int Char;

    if (Src && *Src)
    {
        if (*Src == '\\')
        {
            *Dest++ = *Src++;
            Char = *Dest++ = *Src++;

            if (istex(Char))
            {
                while (istex(Char))
                    Char = *Dest++ = *Src++;

                Src--;
                Dest--;
            }

        }
        else
            *Dest++ = *Src++;
        *Dest = 0;
    }
    else
        Src = NULL;

    return (Src);
}


/*
 * Scans the `SrcBuf' for a LaTeX arg, and puts that arg into `Dest'.
 * `Until' specifies what we'll copy. Assume the text is
 * "{foo}bar! qux} baz".
 *  GET_TOKEN       => "{foo}"
 *  GET_STRIP_TOKEN => "foo"
 *  '!'             => "{foo}bar!" (i.e. till the first "!")
 * Returns NULL if we can't find the argument, ptr to the first character
 * after the argument in other cases.
 *
 * If one of the tokens found is in the wl wordlist, and we're in the
 * outer most paren, and Until isn't a single character, we'll stop.
 * You may pass NULL as wl.
 *
 * We assume that you've previously skipped over leading spaces.
 *
 */

#define GET_TOKEN       256
#define GET_STRIP_TOKEN 257

static char *GetLTXArg(char *SrcBuf, char *OrigDest, const int Until,
	struct WordList *wl)
{
    char *Retval;
    char *TmpPtr;
    char *Dest = OrigDest;
    unsigned long DeliCnt = 0;

    *Dest = 0;
    TmpPtr = SrcBuf;

    switch (Until)
    {
    case GET_STRIP_TOKEN:
    case GET_TOKEN:
        while ((Retval = GetLTXToken(TmpPtr, Dest)))
        {
            switch (*Dest)
            {
            case '{':
                DeliCnt++;
                break;
            case '}':
                DeliCnt--;
            }
            Dest += Retval - TmpPtr;
            TmpPtr = Retval;

            if (!DeliCnt || ((DeliCnt == 1) && wl && HasWord(Dest, wl)))
                break;
        }

        if (Retval && (*OrigDest == '{') && (Until == GET_STRIP_TOKEN))
        {
            int len = strlen(OrigDest+1);
            memmove(OrigDest, OrigDest + 1, len  + 1);
            /* Strip the last '}' off */
            OrigDest[len-1] = 0;
        }
        break;
    default:
        DeliCnt = TRUE;
        while ((Retval = GetLTXArg(TmpPtr, Dest, GET_TOKEN, NULL)))
        {
            if (*Dest == Until)
                DeliCnt = FALSE;

            Dest += Retval - TmpPtr;
            TmpPtr = Retval;

            if (!DeliCnt)
                break;
        }
        break;
    }
    *Dest = 0;

    return (Retval);
}


static char *PreProcess(void)
{
    char *TmpPtr;

    /* Reset any line suppressions  */
    LineSuppressions = *(uint64_t *)StkTop(&FileSuppStack);
    UserLineSuppressions = *(uint64_t *)StkTop(&UserFileSuppStack);

    /* Kill comments. */
    strcpy(Buf, RealBuf);

    TmpPtr = Buf;

    LastWasComment = FALSE;
    while ((TmpPtr = strchr(TmpPtr, '%')))
    {
        char *EscapePtr = TmpPtr;
        int NumBackSlashes = 0;
        while (EscapePtr != Buf && EscapePtr[-1] == '\\')
        {
            ++NumBackSlashes;
            --EscapePtr;
        }

        /* If there is an even number of backslashes, then it's a comment. */
        if ((NumBackSlashes % 2) == 0)
        {
            LastWasComment = TRUE;
            PSERR(TmpPtr - Buf, 1, emComment);
            *TmpPtr = 0;
            /* Check for line suppressions */
            if (!NoLineSupp)
            {
                int error;
                const int MaxSuppressionBits = 63;

                /* Convert to lowercase to compare with LineSuppDelim */
                EscapePtr = ++TmpPtr; /* move past NUL terminator */
                while ( *EscapePtr )
                {
                    *EscapePtr = tolower((unsigned char)*EscapePtr);
                    ++EscapePtr;
                }

                EscapePtr = TmpPtr; /* Save it for later */
                while ((TmpPtr = strstr(TmpPtr, FileSuppDelim))) {
                    uint64_t errbit;
                    TmpPtr += STRLEN(FileSuppDelim);
                    error = atoi(TmpPtr);

                    if (abs(error) > MaxSuppressionBits)
                    {
                        PrintPrgErr(pmSuppTooHigh, error, MaxSuppressionBits);
                    }
                    errbit = ((uint64_t)1 << abs(error));
                    if (error > 0)
                    {
                        *(uint64_t *)StkTop(&FileSuppStack) |= errbit;
                        LineSuppressions |= errbit;
                    }
                    else
                    {
                        *(uint64_t *)StkTop(&UserFileSuppStack) |= errbit;
                        UserLineSuppressions |= errbit;
                    }
                }
                TmpPtr = EscapePtr;

                while ((TmpPtr = strstr(TmpPtr, LineSuppDelim))) {

                    TmpPtr += STRLEN(LineSuppDelim);
                    error = atoi(TmpPtr);

                    if (abs(error) > MaxSuppressionBits)
                    {
                        PrintPrgErr(pmSuppTooHigh, error, MaxSuppressionBits);
                    }

                    if (error > 0)
                    {
                        LineSuppressions |= ((uint64_t)1 << error);
                    }
                    else
                    {
                        UserLineSuppressions |= ((uint64_t)1 << (-error));
                    }
                }
            }
            break;
        }
        TmpPtr++;
    }
    return (Buf);
}

/*
 * Interpret environments
 */

static void PerformEnv(char *Env, int Begin)
{
    static char VBStr[BUFFER_SIZE] = "";

    if (HasWord(Env, &MathEnvir))
    {
        if (Begin)
            PushMode(TRUE, &MathModeStack);
        else
        {
            if (!CurStkMode(&MathModeStack))
                PSERRA(BufPtr - Buf - 4, 1, emMathModeConfusion, "on");
            StkPop(&MathModeStack);
        }
    }

    if (HasWord(Env, &TextEnvir))
    {
        if (Begin)
            PushMode(FALSE, &MathModeStack);
        else
        {
            if (CurStkMode(&MathModeStack))
                PSERRA(BufPtr - Buf - 4, 1, emMathModeConfusion, "off");
            StkPop(&MathModeStack);
        }
    }

    if (Begin && HasWord(Env, &VerbEnvir))
    {
        VerbMode = TRUE;
        strcpy(VBStr, "\\end{");
        strcat(VBStr, Env);
        strcat(VBStr, "}");
        VerbStr = VBStr;
    }
}

static char *SkipVerb(void)
{
    char *TmpPtr = BufPtr;
    int TmpC;

    if (VerbMode && BufPtr)
    {
        if (!(TmpPtr = strstr(BufPtr, VerbStr)))
            BufPtr = &BufPtr[strlen(BufPtr)];
        else
        {
            VerbMode = FALSE;
            BufPtr = &TmpPtr[strlen(VerbStr)];
            SKIP_AHEAD(BufPtr, TmpC, LATEX_SPACE(TmpC));
            if (*BufPtr)
                PSERR(BufPtr - Buf, strlen(BufPtr) - 2, emIgnoreText);
        }
    }
    return (TmpPtr);
}

#define CHECKDOTS(wordlist, dtval) \
for(i = 0; (i < wordlist.Stack.Used) && !(Back && Front);  i++) \
 { if(!strafter(PstPtr, wordlist.Stack.Data[i])) \
         Back = dtval; \
   if(!strinfront(PrePtr, wordlist.Stack.Data[i])) \
         Front = dtval; }



/*
 * Checks that the dots are correct
 */

static enum DotLevel CheckDots(char *PrePtr, char *PstPtr)
{
    unsigned long i;
    int TmpC;
    enum DotLevel Front = dtUnknown, Back = dtUnknown;

    if (CurStkMode(&MathModeStack))
    {
        PrePtr--;
#define SKIP_EMPTIES(macro, ptr) macro(ptr, TmpC, \
(LATEX_SPACE(TmpC) || (TmpC == '{') || (TmpC == '}')))

        SKIP_EMPTIES(SKIP_BACK, PrePtr);
        SKIP_EMPTIES(SKIP_AHEAD, PstPtr);

        CHECKDOTS(CenterDots, dtCDots);

        if (!(Front && Back))
        {
            CHECKDOTS(LowDots, dtLDots);
        }
        return (Front & Back);
    }
    else
        return (dtLDots);

}

static const char *Dot2Str(enum DotLevel dl)
{
    const char *Retval = INTERNFAULT;
    switch (dl)
    {
    case dtUnknown:
        Retval = "\\cdots or \\ldots";
        break;
    case dtDots:
        Retval = "\\dots";
        break;
    case dtCDots:
        Retval = "\\cdots";
        break;
    case dtLDots:
        Retval = "\\ldots";
        break;
    }
    return Retval;
}

/*
 * Wipes a command, according to the definition in WIPEARG
 */

static void WipeArgument(const char *Cmd, char *CmdPtr)
{
    unsigned long CmdLen = strlen(Cmd);
    const char *Format;
    char *TmpPtr;
    int c, TmpC;

    if (Cmd && *Cmd)
    {
        TmpPtr = &CmdPtr[CmdLen];
        Format = &Cmd[CmdLen + 1];

        while (TmpPtr && *TmpPtr && *Format)
        {
            switch (c = *Format++)
            {
            case '*':
                SKIP_AHEAD(TmpPtr, TmpC, LATEX_SPACE(TmpC));
                if (*TmpPtr == '*')
                    TmpPtr++;
                break;
            case '[':
                SKIP_AHEAD(TmpPtr, TmpC, LATEX_SPACE(TmpC));
                if (*TmpPtr == '[')
                    TmpPtr = GetLTXArg(TmpPtr, ArgBuffer, ']', NULL);
                break;
            case '(':
                SKIP_AHEAD(TmpPtr, TmpC, LATEX_SPACE(TmpC));
                if (*TmpPtr == '(')
                    TmpPtr = GetLTXArg(TmpPtr, ArgBuffer, ')', NULL);
                break;
            case '{':
                SKIP_AHEAD(TmpPtr, TmpC, LATEX_SPACE(TmpC));
                TmpPtr = GetLTXArg(TmpPtr, ArgBuffer, GET_TOKEN, NULL);
            case '}':
            case ']':
            case ')':
                break;
            default:
                PrintPrgErr(pmWrongWipeTemp, &Cmd[strlen(Cmd) + 1]);
                break;
            }
        }

        if (TmpPtr)
            strwrite(CmdPtr+CmdLen, VerbClear, TmpPtr - CmdPtr - CmdLen);
        else
            strxrep(CmdPtr+CmdLen, "()[]{}", *VerbClear);
    }
}

/*
 * Checks italic.
 *
 */

static void CheckItal(const char *Cmd)
{
    int TmpC;
    char *TmpPtr;
    if (HasWord(Cmd, &NonItalic))
        ItState = itOff;
    else if (HasWord(Cmd, &Italic))
        ItState = itOn;
    else if (HasWord(Cmd, &ItalCmd))
    {
        TmpPtr = BufPtr;
        SKIP_AHEAD(TmpPtr, TmpC, LATEX_SPACE(TmpC));
        if (*TmpPtr == '{')
        {
            ItFlag = ItState ? efItal : efNoItal;
            ItState = itOn;
        }
    }
}

/*
 * Interpret isolated commands.
 *
 */

static void PerformBigCmd(char *CmdPtr)
{
    char *TmpPtr;
    const char *ArgEndPtr;
    unsigned long CmdLen = strlen(CmdBuffer);
    int TmpC;
    enum ErrNum ErrNum;
    struct ErrInfo *ei;

    enum DotLevel dotlev, realdl = dtUnknown;

    TmpPtr = BufPtr;
    SKIP_AHEAD(TmpPtr, TmpC, LATEX_SPACE(TmpC));

    ArgEndPtr = GetLTXArg(TmpPtr, ArgBuffer, GET_STRIP_TOKEN, NULL);

    /* Kill `\verb' commands */

    if (WipeVerb)
    {
        if (!strcmp(CmdBuffer, "\\verb"))
        {
            if (*BufPtr && (*BufPtr != '*' || BufPtr[1]))
            {
                if (*BufPtr == '*')
                    TmpPtr = strchr(&BufPtr[2], BufPtr[1]);
                else
                    TmpPtr = strchr(&BufPtr[1], *BufPtr);
                if (TmpPtr)
                    strwrite(CmdPtr, VerbClear, (TmpPtr - CmdPtr) + 1);
                else
                    PSERR(CmdPtr - Buf, 5, emNoArgFound);
            }
        }
    }

    if (HasWord(CmdBuffer, &IJAccent))
    {
        if (ArgEndPtr)
        {
            TmpPtr = ArgBuffer;
            SKIP_AHEAD(TmpPtr, TmpC, TmpC == '{');      /* } */

            if ((*TmpPtr == 'i') || (*TmpPtr == 'j'))
                PrintError(emAccent, CurStkName(&InputStack), RealBuf,
                           CmdPtr - Buf, (long)strlen(CmdBuffer), Line,
                           CmdBuffer, *TmpPtr,
                           CurStkMode(&MathModeStack) ? "math" : "");
        }
        else
            PSERR(CmdPtr - Buf, CmdLen, emNoArgFound);
    }

    if (HasWord(CmdBuffer, &NotPreSpaced) && SeenSpace)
        PSERRA(CmdPtr - Buf - 1, 1, emRemPSSpace, CmdBuffer);

    if ((TmpPtr = HasWord(CmdBuffer, &NoCharNext)))
    {
        char *BPtr = BufPtr;

        TmpPtr += strlen(TmpPtr) + 1;
        SKIP_AHEAD(BPtr, TmpC, LATEX_SPACE(TmpC));

        if (strchr(TmpPtr, *BPtr))
        {
            PSERR2(CmdPtr - Buf, CmdLen, emNoCharMean, CmdBuffer, *BPtr);
        }
    }

    /* LaTeX environment tracking */
    if (!strcmp(CmdBuffer, "\\begin") || !strcmp(CmdBuffer, "\\end"))
    {
        if (ArgEndPtr)
        {
            if (!strcmp(ArgBuffer, "document"))
                InHeader = FALSE;

            if (CmdBuffer[1] == 'b')
            {
                if (!(PushErr(ArgBuffer, Line, CmdPtr - Buf,
                              CmdLen, RealBuf, &EnvStack)))
                    PrintPrgErr(pmNoStackMem);
            }
            else
            {
                if ((ei = PopErr(&EnvStack)))
                {
                    if (strcmp(ei->Data, ArgBuffer))
                        PrintError(emExpectC, CurStkName(&InputStack), RealBuf,
                                   CmdPtr - Buf,
                                   (long) strlen(CmdBuffer),
                                   Line, ei->Data, ArgBuffer);

                    FreeErrInfo(ei);
                }
                else
                    PrintError(emSoloC, CurStkName(&InputStack), RealBuf,
                               CmdPtr - Buf,
                               (long) strlen(CmdBuffer),
                               Line, ArgBuffer);
            }

            PerformEnv(ArgBuffer, (int) CmdBuffer[1] == 'b');
        }
        else
            PSERR(CmdPtr - Buf, CmdLen, emNoArgFound);
    }

    /* ConTeXt \start \stop tracking */
    if (!strncmp(CmdBuffer, "\\start", 6) || !strncmp(CmdBuffer, "\\stop", 5))
    {
        if (CmdBuffer[3] == 'a') /* start */
        {
            TmpPtr = CmdBuffer + 6;
            if (!(PushErr(TmpPtr, Line, CmdPtr - Buf + 6,
                          CmdLen - 6, RealBuf, &ConTeXtStack)))
                PrintPrgErr(pmNoStackMem);
        }
        else
        {
            TmpPtr = CmdBuffer + 5;
            if ((ei = PopErr(&ConTeXtStack)))
            {
                if (strcmp(ei->Data, TmpPtr))
                    PrintError(emExpectConTeXt, CurStkName(&InputStack), RealBuf,
                               CmdPtr - Buf + 5,
                               (long) strlen(TmpPtr),
                               Line, ei->Data, TmpPtr);

                FreeErrInfo(ei);
            }
            else
            {
                PrintError(emSoloC, CurStkName(&InputStack), RealBuf,
                           CmdPtr - Buf,
                           (long) strlen(CmdBuffer),
                           Line, TmpPtr);
            }
        }
        /* TODO: Do I need to call PerformEnv? */
        /* It handles math and verbatim environments */
    }

    CheckItal(CmdBuffer);

    if ((ErrNum = PerformCommand(CmdBuffer, BufPtr)))
        PSERR(CmdPtr - Buf, CmdLen, ErrNum);

    if (!strcmp(CmdBuffer, "\\cdots"))
        realdl = dtCDots;

    if (!strcmp(CmdBuffer, "\\ldots"))
        realdl = dtLDots;

    if (!strcmp(CmdBuffer, "\\dots"))
        realdl = dtLDots;

    if (realdl != dtUnknown)
    {
        dotlev = CheckDots(CmdPtr, BufPtr);
        if (dotlev && (dotlev != realdl))
        {
            const char *cTmpPtr = Dot2Str(dotlev);
            PSERRA(CmdPtr - Buf, CmdLen, emEllipsis, cTmpPtr);
        }
    }

    if ((TmpPtr = HasWord(CmdBuffer, &WipeArg)))
        WipeArgument(TmpPtr, CmdPtr);
}

/*
 * Check user abbreviations. Pass a pointer to the `.';
 * also ensure that it's followed by spaces, etc.
 *
 * Note: We assume that all abbrevs have been transferred from
 * AbbrevCase into Abbrev.
 */

static void CheckAbbrevs(const char *Buffer)
{
    long i;
    char *TmpPtr;
    const char *AbbPtr;

    if (INUSE(emInterWord))
    {
        TmpPtr = TmpBuffer + Abbrev.MaxLen + 2;
        *TmpPtr = 0;
        AbbPtr = Buffer;

        for (i = Abbrev.MaxLen; i >= 0; i--)
        {
            *--TmpPtr = *AbbPtr--;
            if (!isalpha((unsigned char)*AbbPtr) &&
                /* Ignore spacing problems after commands if desired */
                (*AbbPtr != '\\' || (CmdSpace & csInterWord)) &&
                HasWord(TmpPtr, &Abbrev))
            {
                PSERR(Buffer - Buf + 1, 1, emInterWord);
            }
            if (!*AbbPtr)
                break;
        }
    }
}


/*
 * Check misc. things which can't be included in the main loop.
 *
 */

static void CheckRest(void)
{
    unsigned long Count;
    long CmdLen;
    char *UsrPtr;

    /* Search for user-specified warnings */

#if ! (HAVE_PCRE || HAVE_POSIX_ERE)

    if (INUSE(emUserWarnRegex) && UserWarnRegex.Stack.Used > 0)
    {
        PrintPrgErr(pmNoRegExp);
        ClearWord( &UserWarnRegex );
    }
    else if (INUSE(emUserWarn))
    {
        strcpy(TmpBuffer, Buf);
    }

#else

    if (INUSE(emUserWarnRegex) && UserWarnRegex.Stack.Used > 0)
    {
        static char error[ERROR_STRING_SIZE];
        static regmatch_t MatchVector[NUM_MATCHES];
        int rc;
        int len = strlen(TmpBuffer);
        strcpy(TmpBuffer, Buf);

        /* Compile all regular expressions if not already compiled. */
        if ( !RegexArray && UserWarnRegex.Stack.Used > 0 )
        {
            RegexArray = (regex_t*)malloc( sizeof(regex_t) * UserWarnRegex.Stack.Used );
            if (!RegexArray)
            {
                /* Allocation failed. */
                PrintPrgErr(pmNoRegexMem);
                ClearWord(&UserWarnRegex);
                NumRegexes = 0;
            }
            else
            {
                NumRegexes = 0;
                FORWL(Count, UserWarnRegex)
                {
                    char *pattern = UserWarnRegex.Stack.Data[Count];
                    char *CommentEnd = NULL;

                    /* See if it's got a special name that it goes by.
                       Only use the comment if it's at the very beginning. */
                    if ( strncmp(pattern,"(?#",3) == 0 )
                    {
                        CommentEnd = strchr(pattern, ')');
                        /* TODO: check for PCRE/POSIX only regexes */
                        if ( CommentEnd != NULL )
                        {
                            *CommentEnd = '\0';
                            /* We're leaking a little here, but this was never freed until exit anyway... */
                            UserWarnRegex.Stack.Data[NumRegexes] = pattern+3;

                            /* Compile past the end of the comment so that it works with POSIX too. */
                            pattern = CommentEnd + 1;
                        }
                    }

                    /* Ignore PCRE and POSIX specific regexes.
                     * This is mostly to make testing easier. */
                    if ( strncmp(pattern,"PCRE:",5) == 0 )
                    {
                        #if HAVE_PCRE
                        pattern += 5;
                        #else
                        continue;
                        #endif
                    }
                    if ( strncmp(pattern,"POSIX:",6) == 0 )
                    {
                        #if HAVE_POSIX_ERE
                        pattern += 6;
                        #else
                        continue;
                        #endif
                    }

                    rc = regcomp((regex_t*)(&RegexArray[NumRegexes]),
                                 pattern, REGEX_FLAGS);

                    /* Compilation failed: print the error message */
                    if (rc != 0)
                    {
                        /* TODO: decide whether a non-compiling regex should completely stop, or just be ignored */
                        regerror(rc,(regex_t*)(&RegexArray[NumRegexes]),
                                 error, ERROR_STRING_SIZE);
                        PrintPrgErr(pmRegexCompileFailed, pattern, error);
                    }
                    else
                    {
                        if ( !CommentEnd )
                        {
                            ((char*)UserWarnRegex.Stack.Data[NumRegexes])[0] = '\0';
                        }
                        ++NumRegexes;
                    }
                }
            }
        }

        for (Count = 0; Count < NumRegexes; ++Count)
        {
            int offset = 0;
            char *ErrMessage = UserWarnRegex.Stack.Data[Count];
            const int NamedWarning = strlen(ErrMessage) > 0;

            while (offset < len)
            {
                /* Check if this warning should be suppressed. */
                if (UserLineSuppressions && NamedWarning)
                {
                    /* The warning can be named with positive or negative numbers. */
                    int UserWarningNumber = abs(atoi(ErrMessage));
                    if (UserLineSuppressions & ((uint64_t)1 << UserWarningNumber))
                    {
                        break;
                    }
                }

                rc = regexec( (regex_t*)(&RegexArray[Count]), TmpBuffer+offset,
                              NUM_MATCHES, MatchVector, 0);
                /* Matching failed: handle error cases */
                if (rc != 0)
                {
                    switch(rc)
                    {
                        case REG_NOMATCH:
                            /* no match, no problem */
                            break;
                        default:
                            regerror(rc, (regex_t*)(&RegexArray[Count]),
                                     error, ERROR_STRING_SIZE);
                            PrintPrgErr(pmRegexMatchingError, error);
                            break;
                    }

                    offset = len; /* break out of loop */
                }
                else
                {
#define MATCH (MatchVector[0])
                    if ( NamedWarning )
                    {
                        /* User specified error message */
                        PSERR2(offset + MATCH.rm_so, MATCH.rm_eo - MATCH.rm_so,
                               emUserWarnRegex,
                               strlen(ErrMessage), ErrMessage);
                    }
                    else
                    {
                        /* Default -- show the match */
                        PSERR2(offset + MATCH.rm_so, MATCH.rm_eo - MATCH.rm_so,
                               emUserWarnRegex,
                               /* The format specifier expects an int */
                               (int)(MATCH.rm_eo - MATCH.rm_so),
                               TmpBuffer + offset + MATCH.rm_so);
                    }
                    if ( MATCH.rm_eo == 0 )
                    {
                        /* Break out of loop if the match was empty.
                         * This avoids an infinite loop when the match
                         * is empty, e.g $ */
                        offset = len;
                    }
                    else
                    {
                        offset += MATCH.rm_eo;
                    }
#undef MATCH
                }
            }
        }
    }
    else if (INUSE(emUserWarn))
    {
        strcpy(TmpBuffer, Buf);
    }

#endif


    if (INUSE(emUserWarn))
    {
        FORWL(Count, UserWarn)
        {
            for (UsrPtr = TmpBuffer;
                 (UsrPtr = strstr(UsrPtr, UserWarn.Stack.Data[Count]));
                 UsrPtr++)
            {
                CmdLen = strlen(UserWarn.Stack.Data[Count]);
                PSERRA(UsrPtr - TmpBuffer, CmdLen, emUserWarn, UserWarn.Stack.Data[Count]);
            }
        }

        strlwr(TmpBuffer);

        FORWL(Count, UserWarnCase)
        {
            for (UsrPtr = TmpBuffer;
                 (UsrPtr = strstr(UsrPtr, UserWarnCase.Stack.Data[Count]));
                 UsrPtr++)
            {
                CmdLen = strlen(UserWarnCase.Stack.Data[Count]);
                PSERRA(UsrPtr - TmpBuffer, CmdLen, emUserWarn, UserWarnCase.Stack.Data[Count]);
            }
        }
    }
}


/*
 * Checks that the dash-len is correct.
 */

static void CheckDash(void)
{
    char *TmpPtr;
    int TmpC;
    long TmpCount, Len;
    struct WordList *wl = NULL;
    unsigned long i;
    int Errored;
    char *PrePtr = &BufPtr[-2];

    TmpPtr = BufPtr;
    SKIP_AHEAD(TmpPtr, TmpC, TmpC == '-');
    TmpCount = TmpPtr - BufPtr + 1;

    if (CurStkMode(&MathModeStack))
    {
        if (TmpCount > 1)
            HERE(TmpCount, emWrongDash);
    }
    else
    {
        if (LATEX_SPACE(*PrePtr) && LATEX_SPACE(*TmpPtr))
            wl = &WordDash;
        if (isdigit((unsigned char)*PrePtr) && isdigit((unsigned char)*TmpPtr))
            wl = &NumDash;
        if (isalpha((unsigned char)*PrePtr) && isalpha((unsigned char)*TmpPtr))
            wl = &HyphDash;

        if (wl)
        {
            Errored = TRUE;
            FORWL(i, *wl)
            {
                Len = strtol(wl->Stack.Data[i], NULL, 0);
                if (TmpCount == Len)
                {
                    Errored = FALSE;
                    break;
                }
            }

            if (Errored)
            {
                struct WordList *el = &DashExcpt;

                FORWL(i, *el)
                {
                    char *exception = el->Stack.Data[i];

                    char *e = exception;
                    while ( *e )
                    {
                        if ( *e == '-' && 0 == strncmp( BufPtr, e, strlen(e) ) )
                        {
                            char *f = e;
                            TmpPtr = BufPtr;
                            while ( f > exception && *(--f) == *(--TmpPtr) )
                            {
                                /* Nothing */
                            }

                            if ( f <= exception && *f == *TmpPtr )
                            {
                                Errored = FALSE;
                                break;
                            }
                        }

                        ++e;
                    }

                    if ( !Errored )
                        break;
                }
            }

            /* Check DashExcpt looking for phrase with hyphenation that doesn't
             * match what's in DashExcpt.  This really only makes sense for
             * HyphDash, but it should be cheap in the other cases. */
            if (!Errored)
            {
                TmpPtr = BufPtr-1;

                SKIP_BACK(TmpPtr, TmpC, (TmpC == '-'));
                SKIP_BACK(TmpPtr, TmpC, isalpha(TmpC));
                /* If we found a dash going backwards, the we already checked
                 * this on the first dash */
                if (*TmpPtr != '-')
                {
                    struct WordList *el;
                    /* PrePtr now points to the beginning of the hyphenated phrase */
                    PrePtr = ++TmpPtr;

                    el = &DashExcpt;

                    FORWL(i, *el)
                    {
                        int FoundHyphenDiff;
                        char *e = el->Stack.Data[i];
                        TmpPtr = PrePtr;

                        /* Walk through the strings until we find a
                         * mismatch.  */
                        FoundHyphenDiff = FALSE;
                        while (*e && *TmpPtr && *e == *TmpPtr)
                        {
                            /* Skip past characters that are the same */
                            while (*e && *TmpPtr && *e == *TmpPtr)
                            {
                                ++e;
                                ++TmpPtr;
                            }
                            /* Skip past differences in hyphens */
                            while (*e == '-' && *TmpPtr != '-')
                            {
                                ++e;
                                FoundHyphenDiff = TRUE;
                            }
                            while (*TmpPtr == '-' && *e != '-')
                            {
                                ++TmpPtr;
                                FoundHyphenDiff = TRUE;
                            }
                        }

                        /* If there was no mismatch all the way to the end of e,
                         * and TmpPtr is not in the middle of a word, then they
                         * matched ignoring hyphens, so we have found the one
                         * DashExcpt element we care about and don't have to
                         * check any others.  Moreover, if we found a difference
                         * in hyphenation, then we must warn because it matches
                         * something in DashExcpt but with improper hyphenation.
                         * It's possible they could put the same phrase in twice
                         * with different hyphenations, but that seems pretty
                         * pathological. */
                        if (*e == '\0' && !isalpha((unsigned char)*TmpPtr))
                        {
                            if (FoundHyphenDiff)
                                Errored = TRUE;
                            break;
                        }
                    }
                }
            }

            if (Errored)
                HERE(TmpCount, emWrongDash);
        }
    }
}

/*
 * Pushes and pops nesting characters.
 *
 */

static void HandleBracket(char Char)
{
    unsigned long BrOffset;     /* Offset into BrOrder array */
    struct ErrInfo *ei;
    char TmpC, Match;
    char ABuf[2], BBuf[2];
    char *TmpPtr;

    AddBracket(Char);

    if ((BrOffset = BrackIndex(Char)) != ~0UL)
    {
        if (BrOffset & 1)       /* Closing bracket of some sort */
        {
            if ((ei = PopErr(&CharStack)))
            {
                Match = MatchBracket(*(ei->Data));
                /* Return italics to proper state */
                if (ei->Flags & efNoItal)
                {
                    if (ItState == itOn)
                    {
                        TmpPtr = BufPtr;
                        SKIP_AHEAD(TmpPtr, TmpC, TmpC == '}');

                        /* If the next character is a period or comma,
                         * or the last character is, then it's not an
                         * error. */
                        /* Checking 2 characters back seems dangerous,
                         * but it's already done in CheckDash. */
                        if ( !strchr(LTX_SmallPunc, *TmpPtr) &&
                             !strchr(LTX_SmallPunc, *(TmpPtr-2)) )
                            HERE(1, emNoItFound);
                    }

                    ItState = FALSE;
                }
                else if (ei->Flags & efItal)
                    ItState = TRUE;

                /* Same for math mode */
                if (ei->Flags & efMath || ei->Flags & efNoMath)
                    StkPop(&MathModeStack);

                FreeErrInfo(ei);
            }
            else
                Match = 0;

            if (Match != Char)
            {
                ABuf[0] = Match;
                BBuf[0] = Char;
                ABuf[1] = BBuf[1] = 0;
                if (Match)
                    PrintError(emExpectC, CurStkName(&InputStack), RealBuf,
                               BufPtr - Buf - 1, 1, Line,
                               ABuf, BBuf);
                else
                    HEREA(1, emSoloC, BBuf);
            }

        }
        else                    /* Opening bracket of some sort  */
        {
            if ((ei = PushChar(Char, Line, BufPtr - Buf - 1,
                               &CharStack, RealBuf)))
            {
                if (Char == '{')
                {
                    switch (ItFlag)
                    {
                    default:
                        ei->Flags |= ItFlag;
                        ItFlag = efNone;
                        break;
                    case efNone:
                        ei->Flags |= ItState ? efItal : efNoItal;
                    }

                    switch (MathFlag)
                    {
                    case efNone:
                        break;
                    case efMath:
                    case efNoMath:
                        PushMode((MathFlag == efMath), &MathModeStack);
                        /* Save for when we exit this delimiter */
                        ei->Flags |= MathFlag;
                        /* Reset flag just in case... */
                        MathFlag = efNone;
                        break;
                    }
                }
            }

            else
                PrintPrgErr(pmNoStackMem);
        }
    }
}


/*
 * Checks to see if CmdBuffer matches any of the words in Silent, or
 * any of the regular expressions in SilentCase.
 *
 */

int CheckSilentRegex(void)
{

#if ! (HAVE_PCRE || HAVE_POSIX_ERE)

    return HasWord(CmdBuffer, &Silent) != NULL;

#else

    static char error[ERROR_STRING_SIZE];
    char *pattern;
    char *tmp;
    int i;
    int rc;
    int len = 4;                /* Enough for the (?:) */

    /* Initialize regular expression */
    if (INUSE(emSpaceTerm) && SilentCase.Stack.Used > 0)
    {
        /* Find the total length we need */
        /* There is 1 for | and the final for null terminator */
        FORWL(i, SilentCase)
        {
            len += strlen( SilentCase.Stack.Data[i] ) + 1;
        }

        /* (A|B|...) */
        tmp = (pattern = (char*)malloc( sizeof(char) * len ));

        #if HAVE_PCRE
        tmp = stpcpy(tmp,"(?:");
        #else
        tmp = stpcpy(tmp,"(");
        #endif

        FORWL(i, SilentCase)
        {
            tmp = stpcpy(tmp, SilentCase.Stack.Data[i]);
            *tmp++ = '|';
        }
        tmp = stpcpy(tmp - 1, ")");

        SilentRegex = malloc( sizeof(regex_t) );
        rc = regcomp(SilentRegex, pattern, REGEX_FLAGS);

        /* Compilation failed: print the error message */
        if (rc != 0)
        {
            regerror(rc, SilentRegex, error, ERROR_STRING_SIZE);
            PrintPrgErr(pmRegexCompileFailed, pattern, error);
            SilentRegex = NULL;
        }
        /* Ensure we won't initialize it again */
        SilentCase.Stack.Used = 0;
        free(pattern);
    }

    /* Check against the normal */
    if ( HasWord(CmdBuffer, &Silent) )
        return 1;
    if (!SilentRegex)
        return 0;

    /* Check against the regexes */
    rc = regexec(SilentRegex, CmdBuffer, 0, NULL, 0);
    if (rc == 0)
        return 1;

    /* Matching failed: handle error cases */
    switch(rc)
    {
        case REG_NOMATCH:
            return 0;
            break;
        default:
            regerror(rc, SilentRegex, error, ERROR_STRING_SIZE);
            PrintPrgErr(pmRegexMatchingError, error);
            break;
    }
    return 0;

#endif
}

/*
 * Searches the `Buf' for possible errors, and prints the errors. `Line'
 * is supplied for error printing.
 */

int FindErr(const char *_RealBuf, const unsigned long _Line)
{
    char *CmdPtr;               /* We'll have to copy each command out. */
    char *PrePtr;               /* Ptr to char in front of command, NULL if
                                 * the cmd appears as the first character  */
    char *TmpPtr;               /* Temporary pointer */
    char *ErrPtr;               /* Ptr to where an error started */

    char TmpC,                  /* Just a temp var used throughout the proc. */
      MatchC, Char;             /* Char. currently processed */
    unsigned long CmdLen;       /* Length of misc. things */
    int MixingQuotes;

    int (*pstcb) (int c);

    enum DotLevel dotlev;

    FoundErr = EXIT_SUCCESS;

    if (_RealBuf)
    {
        RealBuf = _RealBuf;
        Line = _Line;

        if (!LastWasComment)
        {
            SeenSpace = TRUE;
        }
        BufPtr = PreProcess();

        BufPtr = SkipVerb();

        /* Skip past leading whitespace which is insignificant in TeX to avoid
         * spurious warnings (Delete this space to maintain correct
         * pagereferences).  If we have seen a space we don't _need_ to skip
         * past, and doing so misses Message 30 (Multiple spaces detected).  We
         * can miss some of Message 30 in the "not SeenSpace" case too, but I
         * think it's less important, since Message 30 is for newbies.
         */
        if (!SeenSpace && BufPtr)
        {
            SKIP_AHEAD(BufPtr, TmpC, LATEX_SPACE(TmpC));
        }

        while (BufPtr && *BufPtr)
        {
            PrePtr = BufPtr - 1;
            Char = *BufPtr++;
            if (isspace((unsigned char)Char))
                Char = ' ';

            switch (Char)
            {
            case '~':
                TmpPtr = NULL;
                if (isspace((unsigned char)*PrePtr))
                    TmpPtr = PrePtr;
                else if (isspace((unsigned char)*BufPtr))
                    TmpPtr = BufPtr;

                if (TmpPtr)
                    PSERR(TmpPtr - Buf, 1, emDblSpace);
                break;

            case 'X':
            case 'x':
                TmpPtr = PrePtr;

                SKIP_BACK(TmpPtr, TmpC,
                          (LATEX_SPACE(TmpC) || strchr("$", TmpC)));

                if (isdigit((unsigned char)*TmpPtr))
                {
                    TmpPtr = BufPtr;

                    SKIP_AHEAD(TmpPtr, TmpC,
                               (LATEX_SPACE(TmpC) || strchr("$", TmpC)));

                    if (isdigit((unsigned char)*TmpPtr))
                        HERE(1, emUseTimes);
                }
                /* FALLTHRU */
                /* CTYPE: isalpha() */
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h':
            case 'i':
            case 'j':
            case 'k':
            case 'l':
            case 'm':
            case 'n':
            case 'o':
            case 'p':
            case 'q':
            case 'r':
            case 's':
            case 't':
            case 'u':
            case 'v':
            case 'w':          /* case 'x': */
            case 'y':
            case 'z':

            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
            case 'I':
            case 'J':
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'P':
            case 'Q':
            case 'R':
            case 'S':
            case 'T':
            case 'U':
            case 'V':
            case 'W':          /* case 'X': */
            case 'Y':
            case 'Z':
                if (!isalpha((unsigned char)*PrePtr) && (*PrePtr != '\\') &&
                    CurStkMode(&MathModeStack))
                {
                    TmpPtr = BufPtr;
                    CmdPtr = CmdBuffer;
                    do
                    {
                        *CmdPtr++ = Char;
                        Char = *TmpPtr++;
                    }
                    while (isalpha((unsigned char)Char));

                    *CmdPtr = 0;

                    if (HasWord(CmdBuffer, &MathRoman))
                        HEREA(strlen(CmdBuffer), emWordCommand, CmdBuffer);
                }

                break;
            case ' ':
                TmpPtr = BufPtr;
                SKIP_AHEAD(TmpPtr, TmpC, LATEX_SPACE(TmpC));

                if (*TmpPtr && *PrePtr)
                {
                    if ((TmpPtr - BufPtr) > 0)
                    {
                        HERE(TmpPtr - BufPtr + 1, emMultiSpace);
                        strwrite(BufPtr, VerbClear, TmpPtr - BufPtr - 1);
                    }
                }
                break;

            case '.':
                if ((Char == *BufPtr) && (Char == BufPtr[1]))
                {
                    const char *cTmpPtr;
                    dotlev = CheckDots(&PrePtr[1], &BufPtr[2]);
                    cTmpPtr = Dot2Str(dotlev);
                    HEREA(3, emEllipsis, cTmpPtr);
                }

                /* Regexp: "([^A-Z@.])\.[.!?:]*\s[ \`([]*[a-z]" */

                TmpPtr = BufPtr;
                SKIP_AHEAD(TmpPtr, TmpC, strchr(LTX_EosPunc, TmpC));
                if (LATEX_SPACE(*TmpPtr))
                {
                    if (!isupper((unsigned char)*PrePtr) && (*PrePtr != '@') &&
                        (*PrePtr != '.'))
                    {
                        SKIP_AHEAD(TmpPtr, TmpC,
                                   (LATEX_SPACE(TmpC) || TmpC == '\\' ||
                                    strchr(LTX_BosPunc, TmpC)));
                        if (islower((unsigned char)*TmpPtr))
                        {
                            /* If it's a silent macro, count it as space. */
                            int IsSilent = FALSE;
                            if (*(TmpPtr - 1) == '\\')
                            {
                                GetLTXToken(TmpPtr - 1, CmdBuffer);
                                IsSilent = CheckSilentRegex();
                            }

                            /* Ignore spacing problems after commands if desired */
                            TmpPtr = PrePtr;
                            SKIP_BACK(TmpPtr, TmpC, istex(TmpC));
                            if (!IsSilent &&
                                (*TmpPtr != '\\' || (CmdSpace & csInterWord)))
                            {
                                PSERR(BufPtr - Buf, 1, emInterWord);
                            }
                        }
                        else
                            CheckAbbrevs(&BufPtr[-1]);
                    }
                }

                /* FALLTHRU */
            case ':':
            case '?':
            case '!':
            case ';':
                /* Regexp: "[A-Z][A-Z][.!?:;]\s+" */

                if (isspace((unsigned char)*BufPtr) &&
                    isupper((unsigned char)*PrePtr) &&
                    (isupper((unsigned char)PrePtr[-1]) || (Char != '.')) &&
                    !FrenchSpacing)
                {
                    /* Ignore spacing problems after commands if desired */
                    TmpPtr = PrePtr;
                    SKIP_BACK(TmpPtr, TmpC, istex(TmpC));
                    if (*TmpPtr != '\\' || (CmdSpace & csInterSentence))
                        HERE(1, emInterSent);
                }

                /* FALLTHRU */
            case ',':
                if (isspace((unsigned char)*PrePtr) &&
                    !(isdigit((unsigned char)*BufPtr) &&
                      ((BufPtr[-1] == '.') || (BufPtr[-1] == ','))))
                    PSERR(PrePtr - Buf, 1, emSpacePunct);

                if (CurStkMode(&MathModeStack) &&
                    (((*BufPtr == '$') && (BufPtr[1] != '$')) ||
                     (!strafter(BufPtr, "\\)"))))
                    HEREA(1, emPunctMath, "outside inner");

                if (!CurStkMode(&MathModeStack) &&
                    (((*PrePtr == '$') && (PrePtr[-1] == '$')) ||
                     (!strinfront(PrePtr, "\\]"))))
                    HEREA(1, emPunctMath, "inside display");

                break;
            case '\'':
            case '`':
                if ((Char == *BufPtr) && (Char == BufPtr[1]))
                {
                    PrintError(emThreeQuotes, CurStkName(&InputStack), RealBuf,
                               BufPtr - Buf - 1, 3, Line,
                               Char, Char, Char, Char, Char, Char);
                }

                if (Char == '\'')
                    MatchC = '`';
                else
                    MatchC = '\'';

                TmpPtr = BufPtr;
                SKIP_AHEAD(TmpPtr, TmpC, TmpC == Char);

                MixingQuotes = FALSE;

                if ((*TmpPtr == MatchC) || (*TmpPtr == '\"') ||
                    (*TmpPtr == '\xB4')) /* xB4 = latin1 acute accent */
                    MixingQuotes = TRUE;

                SKIP_AHEAD(TmpPtr, TmpC, strchr("`\'\"\xB4", TmpC)); /* xB4 = latin1 acute accent */

                if (MixingQuotes)
                    HERE(TmpPtr - BufPtr + 1, emQuoteMix);

                switch (Char)
                {
                case '\'':
                    if (isalpha((unsigned char)*TmpPtr) &&
                        (strchr(LTX_GenPunc, *PrePtr) || isspace((unsigned char)*PrePtr)))
                        HERE(TmpPtr - BufPtr + 1, emBeginQ);

                    /* Now check quote style */
#define ISPUNCT(ptr) ((strchr(LTX_EosPunc, *ptr) || strchr(LTX_GenPunc, *ptr)) && (ptr[-1] != '\\'))

                    /* We ignore all single words/abbreviations in quotes */

                    {
                        char *WordPtr = PrePtr;
                        SKIP_BACK(WordPtr, TmpC, (isalnum((unsigned char)TmpC) ||
                                                  strchr(LTX_GenPunc, TmpC)));

                        if (*WordPtr != '`')
                        {
                            if (*PrePtr && (Quote != quTrad)
                                && ISPUNCT(PrePtr))
                                PSERRA(PrePtr - Buf, 1,
                                       emQuoteStyle, "in front of");

                            if (*TmpPtr && (Quote != quLogic)
                                && ISPUNCT(TmpPtr))
                                PSERRA(TmpPtr - Buf, 1,
                                       emQuoteStyle, "after");
                        }
                    }

                    break;
                case '`':
                    if (isalpha((unsigned char)*PrePtr) &&
                        (strchr(LTX_GenPunc, *TmpPtr) || isspace((unsigned char)*TmpPtr)))
                        HERE(TmpPtr - BufPtr + 1, emEndQ);
                    break;
                }
                BufPtr = TmpPtr;
                break;
            case '"':
                HERE(1, emUseQuoteLiga);
                break;

            case '\264':             /* ´ (in Latin-1) */
                HERE(1, emUseOtherQuote);
                break;

            case '_':
            case '^':
                if (*PrePtr != '\\')
                {
                    TmpPtr = PrePtr;
                    SKIP_BACK(TmpPtr, TmpC, LATEX_SPACE(TmpC));

                    CmdLen = 1;

                    switch (*TmpPtr)
                    {
                        /*{ */
                    case '}':
                        if (PrePtr[-1] != '\\')
                            break;

                        CmdLen++;
                        PrePtr--;
                        /* FALLTHRU */
                        /*[( */
                    case ')':
                    case ']':
                        PSERR(PrePtr - Buf, CmdLen, emEnclosePar);
                    }

                    TmpPtr = BufPtr;
                    SKIP_AHEAD(TmpPtr, TmpC, LATEX_SPACE(TmpC));

                    ErrPtr = TmpPtr;

                    if (isalpha((unsigned char)*TmpPtr))
                        pstcb = &my_isalpha;
                    else if (isdigit((unsigned char)*TmpPtr))
                        pstcb = &my_isdigit;
                    else
                        break;

                    while ((*pstcb) (*TmpPtr++))
                        ;
                    TmpPtr--;

                    if ((TmpPtr - ErrPtr) > 1)
                        PSERR(ErrPtr - Buf, TmpPtr - ErrPtr, emEmbrace);
                }
                break;
            case '-':
                CheckDash();
                break;
            case '\\':         /* Command encountered  */
                BufPtr = GetLTXToken(--BufPtr, CmdBuffer);

                if (SeenSpace)
                {
                    /* We must be careful to not point to the "previous space"
                     * when it was actually on the previous line.  This could
                     * cause us to write into someone else's memory (inside of
                     * PrintError). */
                    if (HasWord(CmdBuffer, &Linker))
                        PSERR( (PrePtr > Buf) ? (PrePtr - Buf) : 0,
                               1, emNBSpace);
                    if (HasWord(CmdBuffer, &PostLink))
                        PSERR( (PrePtr > Buf) ? (PrePtr - Buf) : 0,
                               1, emFalsePage);
                }

                if (LATEX_SPACE(*BufPtr) && !CurStkMode(&MathModeStack) &&
                    !CheckSilentRegex() && (strlen(CmdBuffer) != 2))
                {
                    PSERR(BufPtr - Buf, 1, emSpaceTerm);
                }
                else if ((*BufPtr == '\\') && (!isalpha((unsigned char)BufPtr[1])) &&
                         (!LATEX_SPACE(BufPtr[1])))
                    PSERR(BufPtr - Buf, 2, emNotIntended);

                PerformBigCmd(PrePtr + 1);
                BufPtr = SkipVerb();

                break;

            case '(':
                if (*PrePtr && !LATEX_SPACE(*PrePtr) && !isdigit((unsigned char)*PrePtr)
                    && !strchr("([{`~", *PrePtr))
                {
                    if (PrePtr[-1] != '\\')     /* Short cmds */
                    {
                        TmpPtr = PrePtr;
                        SKIP_BACK(TmpPtr, TmpC, istex(TmpC));
                        if (*TmpPtr != '\\')    /* Long cmds */
                            PSERRA(BufPtr - Buf - 1, 1, emSpaceParen,
                                   "in front of");
                    }
                }
                if (isspace((unsigned char)*BufPtr))
                    PSERRA(BufPtr - Buf, 1, emNoSpaceParen, "after");
                HandleBracket(Char);
                break;

            case ')':
                if (SeenSpace)
                    PSERRA(BufPtr - Buf - 1, 1, emNoSpaceParen,
                           "in front of");
                if (isalpha((unsigned char)*BufPtr))
                    PSERRA(BufPtr - Buf, 1, emSpaceParen, "after");
                HandleBracket(Char);
                break;

            case '}':
            case '{':
            case '[':
            case ']':
                HandleBracket(Char);
                break;
            case '$':
                if (*PrePtr != '\\')
                {
                    if (*BufPtr == '$')
                    {
                        BufPtr++;
                        TmpPtr = BufPtr;
                        SKIP_AHEAD(TmpPtr, TmpC, (TmpC != '$' && TmpC != '\0'));
                        PSERR(BufPtr - Buf - 2, TmpPtr-BufPtr+4, emDisplayMath);
                    }
                    else
                    {
                        TmpPtr = BufPtr;
                        SKIP_AHEAD(TmpPtr, TmpC, (TmpC != '$' && TmpC != '\0'));
                        PSERR(BufPtr - Buf - 1, TmpPtr-BufPtr+2, emInlineMath);
                    }
                    if (CurStkMode(&MathModeStack))
                    {
                        StkPop(&MathModeStack);
                    }
                    else
                    {
                        PushMode(TRUE, &MathModeStack);
                    }
                }

                break;
            }

            SeenSpace = LATEX_SPACE(Char);
        }

        if (!VerbMode)
        {
            CheckRest();
        }

    }

    return FoundErr;
}

/*
 * Tries to create plural forms for words. Put a '%s' where a
 * suffix should be put, e.g. "warning%s". Watch your %'s!
 */

static void Transit(FILE * fh, unsigned long Cnt, const char *Str)
{
    switch (Cnt)
    {
    case 0:
        fputs("No ", fh);
        fprintf(fh, Str, "s");
        break;
    case 1:
        fputs("One ", fh);
        fprintf(fh, Str, "");
        break;
    default:
        fprintf(fh, "%ld ", Cnt);
        fprintf(fh, Str, "s");
        break;
    }
}

/*
 * Prints the status/conclusion after doing all the testing, including
 * bracket stack status, math mode, etc.
 */

void PrintStatus(unsigned long Lines)
{
    unsigned long Cnt;
    struct ErrInfo *ei;


    while ((ei = PopErr(&CharStack)))
    {
        PrintError(emNoMatchC, ei->File, ei->LineBuf, ei->Column,
                   ei->ErrLen, ei->Line, (char *) ei->Data);
        FreeErrInfo(ei);
    }

    while ((ei = PopErr(&EnvStack)))
    {
        PrintError(emNoMatchC, ei->File, ei->LineBuf, ei->Column,
                   ei->ErrLen, ei->Line, (char *) ei->Data);
        FreeErrInfo(ei);
    }

    while ((ei = PopErr(&ConTeXtStack)))
    {
        PrintError(emNoMatchConTeXt, ei->File, ei->LineBuf, ei->Column,
                   ei->ErrLen, ei->Line, (char *) ei->Data);
        FreeErrInfo(ei);
    }

    if (CurStkMode(&MathModeStack))
    {
        PrintError(emMathStillOn, CurStkName(&InputStack), "", 0L, 0L, Lines);
    }

    for (Cnt = 0L; Cnt < (NUMBRACKETS >> 1); Cnt++)
    {
        if (Brackets[Cnt << 1] != Brackets[(Cnt << 1) + 1])
        {
            PrintError(emNoMatchCC, CurStkName(&InputStack), "", 0L, 0L, Lines,
                       BrOrder[Cnt << 1], BrOrder[(Cnt << 1) + 1]);
        }
    }

    if (!Quiet)
    {
        Transit(stderr, ErrPrint, "error%s printed; ");
        Transit(stderr, WarnPrint, "warning%s printed; ");
        Transit(stderr, UserSupp, "user suppressed warning%s; ");
        Transit(stderr, LineSupp, "line suppressed warning%s.\n");

        /* Print how to suppress warnings. */
        if ( ErrPrint + WarnPrint > 0 ) {
            fprintf(
                stderr,
                "See the manual for how to suppress some or all of these warnings/errors.\n"
                "The manual is available "
#ifdef TEX_LIVE
                "by running `texdoc chktex` or "
#endif
                "at https://www.nongnu.org/chktex/ChkTeX.pdf\n");
        }
    }
}



/*
 * Uses OutputFormat. Be sure that `String'
 * does not contain tabs, newlines, etc.
 * Prints a formatted string. Formatting codes understood:
 *  %b  - string to print Between fields (from -s option)
 *  %c  - Column position of error
 *  %d  - lenght of error (Digit)
 *  %f  - current Filename
 *  %i  - Turn on inverse printing mode.
 *  %I  - Turn off inverse printing mode.
 *  %k  - Kind of error (warning, error, message)
 *  %l  - Line number of error
 *  %m  - warning Message
 *  %n  - warning Number
 *  %u  - an Underlining line (like the one which appears when using -v1)
 *  %r  - part of line in front of error ('S' - 1)
 *  %s  - part of line which contains error (String)
 *  %t  - part of line after error ('S' + 1)
 */


void
PrintError(const enum ErrNum Error, const char *File, const char *String,
           const long Position, const long Len, const long LineNo,  ...)
{
    static                      /* Just to reduce stack usage... */
    char PrintBuffer[BUFFER_SIZE];
    va_list MsgArgs;

    char *LastNorm = OutputFormat;
    char *of;
    int c;

    enum Context Context;

    if (betw(emMinFault, Error, emMaxFault))
    {
        switch (LaTeXMsgs[Error].InUse)
        {
        case iuOK:
            if (SUPPRESSED_ON_LINE(Error))
            {
                LineSupp++;
            }
            else
            {
                Context = LaTeXMsgs[Error].Context;

                if (!HeadErrOut)
                    Context |= ctOutHead;

#define RGTCTXT(Ctxt, Var) if((Context & Ctxt) && !(Var)) break;

                RGTCTXT(ctInMath, CurStkMode(&MathModeStack));
                RGTCTXT(ctOutMath, !CurStkMode(&MathModeStack));
                RGTCTXT(ctInHead, InHeader);
                RGTCTXT(ctOutHead, !InHeader);

                /* Count how warnings or errors we've found, and
                 * update the return code with the worst. */
                switch (LaTeXMsgs[Error].Type)
                {
                case etWarn:
                    WarnPrint++;
                    FoundErr = max(FoundErr, EXIT_WARNINGS);
                    break;
                case etErr:
                    ErrPrint++;
                    FoundErr = max(FoundErr, EXIT_ERRORS);
                    break;
                case etMsg:
                    break;
                }

                while ((of = strchr(LastNorm, '%')))
                {
                    c = *of;
                    *of = 0;

                    fputs(LastNorm, OutputFile);

                    *of++ = c;

                    switch (c = *of++)
                    {
                    case 'b':
                        fputs(Delimit, OutputFile);
                        break;
                    case 'c':
                        /* TODO: need to add the offset of the column
                         * here when long lines are broken. */
                        fprintf(OutputFile, "%ld", Position + 1);
                        break;
                    case 'd':
                        fprintf(OutputFile, "%ld", Len);
                        break;
                    case 'f':
                        fputs(File, OutputFile);
                        break;
                    case 'i':
                        fputs(ReverseOn, OutputFile);
                        break;
                    case 'I':
                        fputs(ReverseOff, OutputFile);
                        break;
                    case 'k':
                        switch (LaTeXMsgs[Error].Type)
                        {
                        case etWarn:
                            fprintf(OutputFile, "Warning");
                            break;
                        case etErr:
                            fprintf(OutputFile, "Error");
                            break;
                        case etMsg:
                            fprintf(OutputFile, "Message");
                            break;
                        }
                        break;
                    case 'l':
                        fprintf(OutputFile, "%ld", LineNo);
                        break;
                    case 'm':
                        va_start(MsgArgs, LineNo);
                        vfprintf(OutputFile,
                                 LaTeXMsgs[Error].Message, MsgArgs);
                        va_end(MsgArgs);
                        break;
                    case 'n':
                        fprintf(OutputFile, "%d", Error);
                        break;
                    case 'u':
                        sfmemset(PrintBuffer, ' ', (long) Position);

                        sfmemset(&PrintBuffer[Position], '^', Len);
                        PrintBuffer[Position + Len] = 0;
                        fputs(PrintBuffer, OutputFile);
                        break;
                    case 'r':
                        substring(String, PrintBuffer, 0L, Position);
                        fputs(PrintBuffer, OutputFile);
                        break;
                    case 's':
                        substring(String, PrintBuffer, Position, Len);
                        fputs(PrintBuffer, OutputFile);
                        break;
                    case 't':
                        substring(String, PrintBuffer,
                                  Position + Len, LONG_MAX);
                        fputs(PrintBuffer, OutputFile);
                        break;
                    default:
                        fputc(c, OutputFile);
                        break;
                    }
                    LastNorm = of;
                }
                fputs(LastNorm, OutputFile);
            }
            break;
        case iuNotUser:
            UserSupp++;
            break;
        case iuNotSys:
            break;
        }
    }
}

/*
 * All commands isolated is routed through this command, so we can
 * update global statuses like math mode and whether @ is a letter
 * or not.
 */

static enum ErrNum PerformCommand(const char *Cmd, char *Arg)
{
    const char *Argument = "";
    enum ErrNum en = emMinFault;
    int TmpC;

    if (!strcmp(Cmd, "\\makeatletter"))
        AtLetter = TRUE;
    else if (!strcmp(Cmd, "\\makeatother"))
        AtLetter = FALSE;
    else if (!strcmp(Cmd, "\\frenchspacing"))
        FrenchSpacing = TRUE;
    else if (!strcmp(Cmd, "\\nonfrenchspacing"))
        FrenchSpacing = FALSE;
    else if (InputFiles && !(strcmp(Cmd, "\\input") && strcmp(Cmd, "\\include")))
    {
        SKIP_AHEAD(Arg, TmpC, LATEX_SPACE(TmpC));
        if (*Arg == '{')        /* } */
        {
            if (GetLTXArg(Arg, TmpBuffer, GET_STRIP_TOKEN, NULL))
                Argument = TmpBuffer;
        }
        else
            Argument = strip(Arg, STRP_BTH);

        if (!(Argument && PushFileName(Argument, &InputStack)))
            en = emNoCmdExec;
    }
    else if (HasWord(Cmd, &Primitives))
        en = emTeXPrim;
    else if (HasWord(Cmd, &MathCmd))
    {
        SKIP_AHEAD(Arg, TmpC, LATEX_SPACE(TmpC));
        if (*Arg == '{')
        {
            /* We will actually turn on math mode when we enter the {} */
            MathFlag = efMath;
        }
    }
    else if (HasWord(Cmd, &TextCmd))
    {
        SKIP_AHEAD(Arg, TmpC, LATEX_SPACE(TmpC));
        if (*Arg == '{')
        {
            /* We will actually turn on text mode when we enter the {} */
            MathFlag = efNoMath;
        }
    }
    else if (*Cmd == '\\')
    {
        /* Quicker check of single lettered commands. */
        switch (Cmd[1])
        {
        case '(':
        case '[':
            PushMode(TRUE, &MathModeStack);
            break;
        case ']':
        case ')':
            if (!CurStkMode(&MathModeStack))
                PSERRA(BufPtr - Buf - 2, 1, emMathModeConfusion, "on");

            StkPop(&MathModeStack);
            break;
        case '/':
            switch (ItState)
            {
            case itOn:
                ItState = itCorrected;
                Argument = Arg;

                SKIP_AHEAD(Argument, TmpC, TmpC == '{' || TmpC == '}');

                if (strchr(".,", *Argument))
                    en = emItPunct;

                break;
            case itCorrected:
                en = emItDup;
                break;
            case itOff:
                en = emItInNoIt;
            }
            break;
        }
    }

    return (en);
}
