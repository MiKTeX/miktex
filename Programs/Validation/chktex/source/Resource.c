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

#include "ChkTeX.h"
#include "OpSys.h"
#include "Utility.h"
#include "Resource.h"

#define LNEMPTY(a) struct WordList a = {0, 1, {0}, {0}};
#define LIST(a)    struct WordList a = {0, 0, {0}, {0}};
#define LCASE(a)   LIST(a) LIST(a ## Case)
#define KEY(a,def) const char *a = def;

RESOURCE_INFO
#undef KEY
#undef LCASE
#undef LNEMPTY
#undef LIST
struct KeyWord
{
    const char *Name;
    const char **String;        /* Keyword = item */
    struct WordList *List,      /* Case-sensitive strings */
     *CaseList;                 /* Case-insensitive strings */
};

#define LNEMPTY LIST
#define LIST(name)       {#name, NULL, &name, NULL},
#define LCASE(name)      {#name, NULL, &name, &name ## Case},
#define KEY(name,def)    {#name, &name, NULL, NULL},

struct KeyWord Keys[] = {
    RESOURCE_INFO {NULL, NULL, NULL, NULL}
};

#undef KEY
#undef LCASE
#undef LNEMPTY
#undef LIST


/***************************** RESOURCE HANDLING **************************/

/* We don't include a trailing semicolon here, so that we can add it
 * at the calling site, thereby preserving proper indentation.  Double
 * semicolons are undesirable since they have been known to break some
 * compilers. */
#define TOKENBITS(name) enum name { \
  BIT(Eof),      /* End-of-file */ \
  BIT(Open),     /* { */ \
  BIT(Close),    /* } */ \
  BIT(BrOpen),   /* [ */ \
  BIT(BrClose),  /* ] */ \
  BIT(Equal),    /* = */ \
  BIT(Word),     /* Keyword */ \
  BIT(Item)      /* List item */ \
}

#undef BIT
#define BIT BITDEF1
TOKENBITS(Token_BIT);
#undef BIT
#define BIT BITDEF2
TOKENBITS(Token);
static enum Token Expect;
static unsigned long RsrcLine;

static enum Token ReadWord(char *, FILE *);
static char MapChars(char **String);



/*
 * Parses the contents of a resource file.
 *
 * Format:
 * Keyword { item1 item2 ... } [ item1 item2 ... ]
 * Keyword [ item1 item2 ... ] { item1 item2 ... }
 * Keyword = { item1 item2 ... }
 * Keyword = [ item1 item2 ... ]
 * Keyword = item
 *
 * Returns whether the attempt was a successful one.
 */

int ReadRC(const char *Filename)
{
    const char *String = NULL;
    int Success = FALSE;
    FILE *fh;
    enum Token Token;
    unsigned long Counter;

    struct KeyWord *CurWord = NULL;

    /* Interpret incoming words as ... */
    enum
    {
        whList,                 /* List elements */
        whCaseList,             /* Case insensitive list elements */
        whEqual,                /* Solo elements */
        whNone                  /* List items not accepted */
    } What = whNone;


    RsrcLine = 0;
    Expect = FLG_Word | FLG_Eof;

    if ((fh = fopen(Filename, "r")))
    {
        Success = TRUE;
        do
        {
            Token = ReadWord(ReadBuffer, fh);
            if (!(Expect & Token))
            {
                switch (Token)
                {
                case FLG_Item:
                    String = "item";
                    break;
                case FLG_Word:
                    String = "word";
                    break;
                case FLG_Equal:
                    String = "`='";
                    break;
                case FLG_Open:
                    String = "`{'";
                    break;
                case FLG_Close:
                    String = "`}'";
                    break;
                case FLG_BrOpen:
                    String = "`['";
                    break;
                case FLG_BrClose:
                    String = "`]'";
                    break;
                case FLG_Eof:
                    String = "EOF";
                    break;
                }
                PrintPrgErr(pmFaultFmt, Filename, RsrcLine, String);
                Success = FALSE;
                Token = FLG_Eof;
            }

            switch (Token)
            {
            case FLG_Word:
                for (Counter = 0; Keys[Counter].Name; Counter++)
                {
                    if (!strcasecmp(ReadBuffer, Keys[Counter].Name))
                    {
                        CurWord = &Keys[Counter];
                        Expect = (CurWord->List ? FLG_Open : 0) |
                            (CurWord->CaseList ? FLG_BrOpen : 0) | FLG_Equal;
                        break;
                    }
                }
                if (!Keys[Counter].Name)
                {
                    PrintPrgErr(pmKeyWord, ReadBuffer, Filename);
                    Success = FALSE;
                    Token = FLG_Eof;
                }
                break;
            case FLG_Item:
                switch (What)
                {
                case whEqual:
                    if (!(*(CurWord->String) = strdup(ReadBuffer)))
                    {
                        PrintPrgErr(pmStrDupErr);
                        Token = FLG_Eof;
                        Success = FALSE;
                    }

                    What = whNone;
                    Expect = FLG_Word | FLG_Eof;
                    break;
                case whCaseList:
                    if (!InsertWord(ReadBuffer, CurWord->CaseList))
                    {
                        Token = FLG_Eof;
                        Success = FALSE;
                    }
                    break;
                case whList:
                    if (!InsertWord(ReadBuffer, CurWord->List))
                    {
                        Token = FLG_Eof;
                        Success = FALSE;
                    }
                    break;
                case whNone:
                    PrintPrgErr(pmAssert);
                }
                break;
            case FLG_Equal:
                What = whEqual;
                Expect = (CurWord->List ? FLG_Open : 0) |
                    (CurWord->CaseList ? FLG_BrOpen : 0) |
                    (CurWord->String ? FLG_Item : 0);
                break;
            case FLG_BrOpen:
                if (What == whEqual)
                    ClearWord(CurWord->CaseList);
                What = whCaseList;
                Expect = FLG_Item | FLG_BrClose;
                break;
            case FLG_Open:
                if (What == whEqual)
                    ClearWord(CurWord->List);
                What = whList;
                Expect = FLG_Item | FLG_Close;
                break;
            case FLG_BrClose:
            case FLG_Close:
                Expect = (CurWord->List ? FLG_Open : 0) |
                    (CurWord->CaseList ? FLG_BrOpen : 0) |
                    FLG_Equal | FLG_Word | FLG_Eof;
                What = whNone;
                break;
            case FLG_Eof:
                break;
            }
        }
        while (Token != FLG_Eof);

        fclose(fh);
    }
    else
        PrintPrgErr(pmRsrcOpen, Filename);

    return (Success);
}

/*
 * Reads a token from the `.chktexrc' file; if the token is
 * FLG_Item or FLG_Word, Buffer will contain the plaintext of the
 * token. If not, the contents are undefined.
 */

static enum Token ReadWord(char *Buffer, FILE * fh)
{
    static char *String = NULL;
    static char StatBuf[BUFSIZ];
    enum Token Retval = FLG_Eof;

    unsigned short Chr;

    char *Ptr;
    int OnceMore = TRUE, Cont = TRUE;

    if (Buffer)
    {
        do
        {
            if (!(String && *String))
            {
                if (fgets(StatBuf, BUFSIZ - 1, fh))
                    String = strip(StatBuf, STRP_RGT);
                RsrcLine++;
            }

            Ptr = Buffer;
            if (String && (String = strip(String, STRP_LFT)))
            {
                switch (Chr = *String)
                {
                case 0:
                case CMNT:
                    String = NULL;
                    break;
                case QUOTE:    /* Quoted argument */
                    Cont = TRUE;
                    String++;

                    while (Cont)
                    {
                        switch (Chr = *String++)
                        {
                        case 0:
                        case QUOTE:
                            Cont = FALSE;
                            break;
                        case ESCAPE:
                            if (!(Chr = MapChars(&String)))
                                break;

                            /* FALLTHRU */
                        default:
                            *Ptr++ = Chr;
                        }
                    }
                    *Ptr = 0;
                    Retval = FLG_Item;
                    OnceMore = FALSE;
                    break;

#define DONEKEY (FLG_Open | FLG_Equal | FLG_BrOpen)
#define DONELIST (FLG_Close | FLG_BrClose)
#define TOKEN(c, ctxt, tk) case c: if(Expect & (ctxt)) Retval = tk; LAST(token)

                    LOOP(token,
                         TOKEN('{', DONEKEY, FLG_Open);
                         TOKEN('[', DONEKEY, FLG_BrOpen);
                         TOKEN('=', DONEKEY, FLG_Equal);
                         TOKEN(']', DONELIST, FLG_BrClose);
                         TOKEN('}', DONELIST, FLG_Close);
                        )
                    if (Retval != FLG_Eof)
                    {
                        OnceMore = FALSE;
                        String++;
                        break;
                    }

                    /* FALLTHRU */

                default:       /* Non-quoted argument */
                    OnceMore = FALSE;
                    if (Expect & FLG_Word)
                    {
                        while (Cont)
                        {
                            Chr = *String++;
                            if (isalpha((unsigned char)Chr))
                                *Ptr++ = Chr;
                            else
                                Cont = FALSE;
                        }
                        String--;
                        Retval = FLG_Word;
                    }
                    else        /* Expect & FLG_Item */
                    {
                        while (Cont)
                        {
                            switch (Chr = *String++)
                            {
                            case CMNT:
                            case 0:
                                String = NULL;
                                Cont = FALSE;
                                break;
                            case ESCAPE:
                                if (!(Chr = MapChars(&String)))
                                    break;

                                *Ptr++ = Chr;
                                break;
                            default:
                                if (!isspace((unsigned char)Chr))
                                    *Ptr++ = Chr;
                                else
                                    Cont = FALSE;
                            }
                        }
                        Retval = FLG_Item;
                    }

                    if (!(Buffer[0]))
                    {
                        PrintPrgErr(pmEmptyToken);
                        if (*String)
                            String++;
                    }
                    *Ptr = 0;
                    break;
                }
            }
            else
                OnceMore = FALSE;
        }
        while (OnceMore);
    }
    return (Retval);
}



/*
 * Translates escape codes. Give it a pointer to the char after the
 * escape char, and we'll return what it maps to.
 */

#define MAP(a,b)        case a: Tmp = b; break;

static char MapChars(char **String)
{
    int Chr, Tmp = 0;
    unsigned short Cnt;

    Chr = *((char *) (*String)++);

    switch (tolower((unsigned char)Chr))
    {
        MAP(QUOTE, QUOTE);
        MAP(ESCAPE, ESCAPE);
        MAP(CMNT, CMNT);
        MAP('n', '\n');
        MAP('r', '\r');
        MAP('b', '\b');
        MAP('t', '\t');
        MAP('f', '\f');
        MAP('{', '{');
        MAP('}', '}');
        MAP('[', '[');
        MAP(']', ']');
        MAP('=', '=');
        MAP(' ', ' ');
    case 'x':
        Tmp = 0;

        for (Cnt = 0; Cnt < 2; Cnt++)
        {
            Chr = *((*String)++);
            if (isxdigit((unsigned char)Chr))
            {
                Chr = toupper((unsigned char)Chr);
                Tmp = (Tmp << 4) + Chr;

                if (isdigit((unsigned char)Chr))
                    Tmp -= '0';
                else
                    Tmp -= 'A' - 10;
            }
            else
            {
                if (Chr)
                {
                    PrintPrgErr(pmNotPSDigit, Chr, "hex");
                    Tmp = 0;
                }
                break;
            }
        }
        break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':

        Tmp = Chr - '0';

        for (Cnt = 0; Cnt < 2; Cnt++)
        {
            Chr = *((*String)++);
            if (within('0', Chr, '7'))
                Tmp = (Tmp * 8) + Chr - '0';
            else
            {
                if (Chr)
                {
                    PrintPrgErr(pmNotPSDigit, Chr, "octal");
                    Tmp = 0;
                }
                break;
            }
        }
        break;
    case 'd':
        for (Cnt = 0; Cnt < 3; Cnt++)
        {
            if (isdigit((unsigned char)(Chr = *((*String)++))))
                Tmp = (Tmp * 10) + Chr - '0';
            else
            {
                if (Chr)
                {
                    PrintPrgErr(pmNotPSDigit, Chr, "");
                    Tmp = 0;
                }
                break;
            }
        }
        break;
    default:
        PrintPrgErr(pmEscCode, ESCAPE, Chr);
    }
    return (Tmp);
}
