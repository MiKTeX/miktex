/*
 *  ChkTeX, utility functions.
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
#include "Utility.h"
#include "Resource.h"
#include "OpSys.h"

typedef unsigned long HASH_TYPE;

/***************************** SUPPORT FUNCTIONS ************************/


/*
 * Copies a string with a maximum length of `len' starting at `pos' in
 * `source' into `dest'.
 * Returns -1 if the pos value is beyond the length of the source value,
 * else NULL.
 */


short substring(const char *source, char *dest, unsigned long pos, long len)
{
    const char *Start;
    short Retval = -1;

    if (len >= 0)
    {
        if (strlen(source) > pos)
        {
            Start = &source[pos];

            while ((len-- > 0) && (*dest++ = *Start++))
                ;

            if (len == -1)
                Retval = 0;
        }
    }
    else
        Retval = 0L;

    *dest = 0;

    return (Retval);
}


/*
 * Determine whether a file exists.
 *
 */


int fexists(const char *Filename)
{
    int Retval;

#if defined(F_OK) && defined(R_OK) && defined(HAVE_ACCESS)

    Retval = access(Filename, F_OK | R_OK) == 0;
#else

    FILE *fh;

    if (fh = fopen(Filename, "r"))
    {
        Retval = TRUE;
        fclose(fh);
    }
    else
        Retval = FALSE;
#endif

    return (Retval);
}



/*
 * 'Safe' memset() replacement.
 * Just tries to check the parameters, so that the risk of killing
 * innocent memory is lowered.
 * Please note that the `n' parameter now is an signed longword, not
 * an size_t. Won't permit that `n' exceeds BUFLEN, nor negative sizes.
 * Returns `to' if some memset()'ing was done, NULL if not.
 */

void *sfmemset(void *to, int c, long n)
{
    if (to && (n > 0))
    {
        n = min(n, BUFSIZ);

        return (memset(to, c, (size_t) n));
    }
    return (NULL);
}


/*
 * Quick replace function
 * Replaces every occurrence of a character in a string with another one.
 */

void strrep(char *String,       /* String to replace within.    */
            const char From,    /* Character to search for.     */
            const char To)      /* Character to put instead.    */
{
    register int c;
    while ((c = *String++))
    {
        if (c == From)
            String[-1] = To;
    }
}

/*
 * Replaces every char not in Prot with To in Buf
 */

void strxrep(char *Buf, const char *Prot, const char To)
{
    int c;

    while ((c = *Buf))
    {
        if (!strchr(Prot, c))
            *Buf = To;
        Buf++;
    }
}

/*
 * Strips tail and/or front of a string
 * Kills trailing/leading spaces. The macro/function LATEX_SPACE(char c)
 * is used to decide whether a space should be skipped. This function
 * should return TRUE if the character should be skipped, FALSE if not.
 * Returns the string which was passed onto it.
 */


char *strip(char *str,          /* String to strip */
            const enum Strip flags)
/* One of the following: */
/* STRP_LFT - Strips leading  blanks */
/* STRP_RGT - Strips trailing blanks */
/* STRP_BTH - Strips on both sides   */
{
    char *bufptr = str;
    char *nlptr;
    char c;

    if (bufptr && (c = *bufptr))
    {
        if (flags & STRP_LFT)
        {
            if (LATEX_SPACE(c) && c)
            {
                do
                {
                    c = *++bufptr;
                }
                while (LATEX_SPACE(c) && c);
            }
        }

        if (flags & STRP_RGT)
        {
            if (c && *bufptr)
            {
                nlptr = bufptr;

                while (*++nlptr)
                    ;

                do
                {
                    *nlptr = 0;
                    c = *--nlptr;
                }
                while (LATEX_SPACE(c) && c && (nlptr > bufptr));

            }
            else
                *bufptr = 0;
        }
    }
    return (bufptr);
}


/*
 * Converts all the chars in the string passed into lowercase.
 */

#ifndef HAVE_STRLWR
char *strlwr(char *String)
{
    char *Bufptr;
    char TmpC;

    for (Bufptr = String; (TmpC = *Bufptr); Bufptr++)
        *Bufptr = tolower((unsigned char)TmpC);

    return (String);
}
#endif

/*
 * Returns a duplicate of the string passed.
 */

#ifndef HAVE_STRDUP
char *strdup(const char *String)
{
    char *Retval = NULL;
    size_t Len;

    if (String)
    {
        Len = strlen(String) + 1;
        if ((Retval = malloc(Len)))
            memcpy(Retval, String, Len);
    }

    return (Retval);
}
#endif

/*
 * Does the same as strdup, but adds a zero-filled padding, length extra bytes.
 */

char *strdupx(const char *String, int Extra)
{
    char *Retval = NULL;
    size_t Len;

    if (String)
    {
        Len = strlen(String) + 1 + Extra;
        if ((Retval = malloc(Len)))
            strncpy(Retval, String, Len);
    }

    return (Retval);
}

/*
 * Case-insensitive comparison of two strings.
 */

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *a, const char *b)
{
    int aa, bb;

    do
    {
        aa = *a++;
        bb = *b++;
    }
    while (aa && (tolower((unsigned char)aa) == tolower((unsigned char)bb)));
    /* bb != 0 is implicit */

    return (tolower((unsigned char)aa) - tolower((unsigned char)bb));
}
#endif

/*
 * Not all reallocs are intelligent enough to handle NULL's as
 * parameters. This fixes this.
 */

void *saferealloc(void *b, size_t n)
{
    void *Retval = NULL;

    if (b)
    {
        if (n)
            Retval = realloc(b, n);
        else
            free(b);
    }
    else
        Retval = malloc(n);

    return (Retval);
}

/*
 * Repeatedly writes the From string over To so that we overwrite Len bytes.
 * Does nothing if passed empty/NULL string.
 */

void strwrite(char *To, const char *From, unsigned long Len)
{
    unsigned long i, j;
    unsigned long FromLen = strlen(From);

    Len = min(Len, BUFSIZ);

    if (To && From)
    {
        switch (FromLen)
        {
        case 0:
            break;
        case 1:
            memset(To, *From, Len);
            break;
        default:
            for (i = j = 0; i < Len; i++, j++)
            {
                if (j >= FromLen)
                    j = 0;
                To[i] = From[j];
            }
        }
    }
}

/*
 * Checks whether Cmp comes after Str.
 *
 */

int strafter(const char *Str, const char *Cmp)
{
    return (strncmp(Str, Cmp, strlen(Cmp)));
}

/*
 * Checks whether Cmp comes before Str. Returns 0 if so, non-zero if not.
 *
 */

int strinfront(const char *Str, const char *Cmp)
{
    int CmpLen;

    if ((CmpLen = strlen(Cmp)))
    {
        Cmp += CmpLen;
        Str++;

        while ((*--Cmp == *--Str) && (--CmpLen > 0))
            ;

        return (CmpLen);
    }
    else
        return (1);
}


/*************************** HASH INDEX **************************/

/*
 * Hashes a string. The string ought be rather short.
 *
 * The algorithm was designed by Peter Weinberger. This version was
 * adapted from Dr Dobb's Journal April 1996 page 26.
 */

static unsigned long HashWord(const char *str)
{
    register unsigned long h = 0, hbit, c;

    while ((c = *str++))
    {
        h = (h << 4) ^ c;
        if ((hbit = h & 0xf0000000))
            h ^= hbit >> 24;
        h &= ~hbit;
    }

    return (h);
}

/*
 * Inserts a string into a hash index. Note: You'll have to
 * duplicate the string yourself.
 */

void InsertHash(char *a, struct Hash *h)
{
    struct HashEntry **he, *newhe;

    if (!h->Index)
    {
        if (!((h->Index = calloc(HASH_SIZE, sizeof(struct HashEntry *)))))
            PrintPrgErr(pmWordListErr);
    }

    he = &h->Index[HashWord(a) % HASH_SIZE];

    if ((newhe = malloc(sizeof(struct HashEntry))))
    {
        newhe->Next = *he;
        newhe->Str = a;
        *he = newhe;
    }
    else
        PrintPrgErr(pmWordListErr);
}

/*
 * Checks whether a string previously has been registered in a
 * hash index.
 */

char *HasHash(const char *a, const struct Hash *h)
{
    struct HashEntry *he;
    HASH_TYPE i;                /* Special magic to optimize SAS/C */

    /* Do we have a hash? */
    if (!h->Index)
        return NULL;

    /* Find entry in hash */
    i = HashWord(a);
    i %= HASH_SIZE;
    he = h->Index[i];

    /* Search in the entry for the item */
    while (he)
    {
        if (!strcmp(he->Str, a))
            return (he->Str);
        else
            he = he->Next;
    }
    return (NULL);
}

/*
 * Clears a hash table.
 */

void ClearHash(struct Hash *h)
{
    int i;
    struct HashEntry *he, *next;
    if (h && h->Index)
    {
        /* Free all the memory */
        for ( i = 0; i < HASH_SIZE; ++i )
        {
            he = h->Index[i];
            while ( he )
            {
                next = he->Next;
                free( he );
                he = next;
            }
        }
        /* Reset the hash table */
        memset(h->Index, '\0', HASH_SIZE * sizeof(struct HashEntry *));
    }
}

/*
 * Rehashes a wordlist. If you change any of the elem's, you must
 * call this.
 *
 */

static void ReHash(struct WordList *WL)
{
    unsigned long i = 0;

    ClearHash(&WL->Hash);
    FORWL(i, *WL) InsertHash(WL->Stack.Data[i], &WL->Hash);
}

/*************************** WORDLIST HANDLING **************************/

/*
 * Inserts a duplicate of `Word' into the `Wordlist' structure. You do thus
 * not need to make a duplicate of `Word' yourself.
 */

int InsertWord(const char *Word, struct WordList *WL)
{
    char *WrdCpy;
    unsigned long Len;

    if ((WrdCpy = strdupx(Word, WALLBYTES)))
    {
        if (StkPush(WrdCpy, &WL->Stack))
        {
            Len = strlen(Word);
            if (WL->MaxLen < Len)
                WL->MaxLen = Len;

            InsertHash(WrdCpy, &WL->Hash);
            return (TRUE);
        }

        free(WrdCpy);
    }

    return (FALSE);
}

/*
 * Clears a WordList; removing all items.
 */

void ClearWord(struct WordList *WL)
{
    char *Word;
    if (WL)
    {
        while ( (Word = StkPop( &WL->Stack )) )
        {
            free(Word);
        }
        WL->Stack.Used = 0;
        WL->MaxLen = 0;
        ClearHash(&WL->Hash);
        if (WL->NonEmpty)
            InsertWord("", WL);
    }
}

/*
 * Query whether a `Word' is previously InsertWord()'ed into the WL
 * structure. Does case-sensitive comparison.
 *
 * Returns the data in the list.
 */


char *HasWord(const char *Word, struct WordList *WL)
{
    return HasHash(Word, &WL->Hash);
}

/*
 * Make all the words in a list lower case for later case-insensitive
 * comparison.
 */

void MakeLower(struct WordList *wl)
{
    unsigned long i;
    FORWL(i, *wl) strlwr(wl->Stack.Data[i]);
    ReHash(wl);
}

/*
 * Calls strrep on each argument in a list.
 */

void ListRep(struct WordList *wl, const char From, const char To)
{
    unsigned long i;
    FORWL(i, *wl) strrep(wl->Stack.Data[i], From, To);
    ReHash(wl);
}



/************************** GENERIC STACK  ******************************/

/*
 * Push something onto a stack. Returns TRUE if successful, else FALSE.
 * Note: You can not push a NULL Data element.
 */

int StkPush(void *Data, struct Stack *Stack)
{
    unsigned long NewSize;
    void **NewBuf;

    if (Data && Stack)
    {
        if (Stack->Used >= Stack->Size)
        {
            NewSize = Stack->Size + MINPUDDLE;

            if ((NewBuf = saferealloc(Stack->Data,
                                      (size_t) NewSize * sizeof(void *))))
            {
                Stack->Size = NewSize;
                Stack->Data = NewBuf;
            }
            else
                return (FALSE);
        }

        Stack->Data[Stack->Used++] = Data;
        return (TRUE);
    }

    return (FALSE);
}

/*
 * Pops an element from the stack.
 *
 */

void *StkPop(struct Stack *Stack)
{
    void *Retval = NULL;

    if (Stack && (Stack->Used > 0))
    {
        Retval = Stack->Data[--Stack->Used];

#ifdef NO_DIRTY_TRICKS

        {
            void **NewBuf;

            if (Stack->Used < (Stack->Size / 2))
            {
                unsigned long NewSize;
                NewSize = Stack->Size - MINPUDDLE;
                NewSize = max(NewSize, MINPUDDLE);

                if (NewBuf = saferealloc(Stack->Data,
                                         (size_t) NewSize * sizeof(void *)))
                {
                    Stack->Size = NewSize;
                    Stack->Data = NewBuf;
                }
            }
        }
#endif

    }
    return (Retval);
}

/*
 * Returns the topmost element of the stack.
 */

void *StkTop(struct Stack *Stack)
{
    if (Stack && (Stack->Used > 0))
        return (Stack->Data[Stack->Used - 1]);
    else
        return (NULL);
}

/****************************** INPUT STACK *****************************/

int PushFileName(const char *Name, struct Stack *stack)
{
    FILE *fh = NULL;
    static char NameBuf[BUFSIZ];

    if (Name && stack)
    {
        if (LocateFile(Name, NameBuf, ".tex", &TeXInputs))
        {
            if ((fh = fopen(NameBuf, "r")))
            {
                return (PushFile(NameBuf, fh, stack));
            }
        }
        PrintPrgErr(pmNoTeXOpen, Name);
    }
    return (FALSE);
}


int PushFile(const char *Name, FILE * fh, struct Stack *stack)
{
    struct FileNode *fn;

    if (Name && fh && stack)
    {
        if ((fn = malloc(sizeof(struct FileNode))))
        {
            if ((fn->Name = strdup(Name)))
            {
                fn->fh = fh;
                fn->Line = 0L;
                if (StkPush(fn, stack))
                    return (TRUE);
                free(fn->Name);
            }
            free(fn);
        }
        PrintPrgErr(pmNoStackMem);
    }

    return (FALSE);
}

char *FGetsStk(char *Dest, unsigned long len, struct Stack *stack)
{
    static short HasSeenLong = 0;
    struct FileNode *fn;
    char *Retval = NULL;
    size_t Retlen = 0;

    if ((fn = StkTop(stack)))
    {
        do
        {
            Retval = fgets(Dest, (int)len, fn->fh);
            if (Retval) {
                Retlen = strlen(Retval);

                if (Retval[Retlen-1] == '\n' || Retlen < len-1)
                    fn->Line++;
                /* We only want the long lines warning once per file */
                else if (!HasSeenLong)
                {
                    PrintPrgErr(pmLongLines, len-2);
                    HasSeenLong = 1;
                }
                break;
            }

            fn = StkPop(stack);
            fclose(fn->fh);
            if ( fn->Name != NULL )
                free(fn->Name);
            free(fn);
            HasSeenLong = 0;
        }
        while (!Retval && (fn = StkTop(stack)));
    }

    return (Retval);
}

const char *CurStkName(struct Stack *stack)
{
    struct FileNode *fn;
    static const char *LastName = "";

    if (PseudoInName && (stack->Used <= 1))
        return (PseudoInName);
    else
    {
        if ((fn = StkTop(stack)))
        {
            if ( stack->Used == 1 && strlen(LastName) == 0 && fn->Name )
            {
                LastName = strdup(fn->Name);
            }
            return (fn->Name);
        }
        else
            return (LastName);
    }
}


FILE *CurStkFile(struct Stack * stack)
{
    struct FileNode *fn;

    if ((fn = StkTop(stack)))
        return (fn->fh);
    else
        return (NULL);
}

unsigned long CurStkLine(struct Stack *stack)
{
    struct FileNode *fn;
    static unsigned long LastLine = 0L;

    if ((fn = StkTop(stack)))
        return (LastLine = fn->Line);
    else
        return (LastLine);
}



/************************** CHARACTER STACK ******************************/

/*
 * Pushes the character on the stack.
 */

struct ErrInfo *PushChar(const char c, const unsigned long Line,
                         const unsigned long Column, struct Stack *Stk,
                         const char *LineCpy)
{
    char Buf[2];

    Buf[0] = c;
    Buf[1] = 0;

    return (PushErr(Buf, Line, Column, 1, LineCpy, Stk));
}

struct ErrInfo *PushErr(const char *Data, const unsigned long Line,
                        const unsigned long Column,
                        const unsigned long ErrLen, const char *LineCpy,
                        struct Stack *Stk)
{
    struct ErrInfo *ci;

    if ((ci = malloc(sizeof(struct ErrInfo))))
    {
        if ((ci->Data = strdup(Data)))
        {
            if ((ci->File = strdup(CurStkName(&InputStack))))
            {
                if ((ci->LineBuf = strdup(LineCpy)))
                {
                    ci->Line = Line;
                    ci->ErrLen = ErrLen;
                    ci->Column = Column;
                    ci->Flags = efNone;

                    if (StkPush(ci, Stk))
                        return (ci);

                    free(ci->LineBuf);
                }
                else
                    PrintPrgErr(pmStrDupErr);

                free(ci->File);
            }
            else
                PrintPrgErr(pmStrDupErr);

            free(ci->Data);
        }
        else
            PrintPrgErr(pmStrDupErr);
        free(ci);
    }

    return (NULL);
}

/*
 * Finds the uppermost entry in the stack with a data matching
 * String.
 */

struct ErrInfo *TopMatch(struct Stack *Stack, char *String)
{
    int i;
    struct ErrInfo *retval = NULL;

    if (Stack && String)
    {
        for (i = Stack->Used - 1; i >= 0; i--)
        {
            if (!strcmp(String, ((struct ErrInfo *) Stack->Data[i])->Data))
            {
                retval = (struct ErrInfo *) Stack->Data[i];
                break;
            }
        }
    }
    return (retval);
}

/*
 * Returns and removes a character from the stack, returns NULL if
 * the stack is empty.
 */


struct ErrInfo *PopErr(struct Stack *Stack)
{
    return ((struct ErrInfo *) StkPop(Stack));
}

/*
 * Same as PopChar(), but lets the error alone on the stack.
 */


struct ErrInfo *TopErr(struct Stack *Stack)
{
    return ((struct ErrInfo *) StkTop(Stack));
}

/*
 * Free all resources associated with a struct FreeInfo.
 */

void FreeErrInfo(struct ErrInfo *ei)
{
    if (ei)
    {
        if (ei->Data)
            free(ei->Data);
        if (ei->File)
            free(ei->File);
        if (ei->LineBuf)
            free(ei->LineBuf);

        free(ei);
    }
}


/************************* OPEN/CLOSE COUNTING **************************/

/*
 * Returns the index a given bracket (`()[]{}') character has in the
 * BrOrder array. Returns ~0 if the character was not a bracket.
 */

long BrackIndex(const char c)
{
    switch (c)
    {
    case '(':
        return (0);
    case ')':
        return (1);
    case '[':
        return (2);
    case ']':
        return (3);
    case '{':
        return (4);
    case '}':
        return (5);
    default:
        return (~0L);
    }
}

/*
 * Counts brackets for you. Give it a bracket, and it will update the
 * corresponding counter.
 */

void AddBracket(const char c)
{
    long Index;

    if ((Index = BrackIndex(c)) != -1)
        Brackets[Index]++;

}

/*
 * Returns the character that matches the given bracket, NULL if `c'
 * wasn't a bracket character.
 */

char MatchBracket(const char c)
{
    unsigned long Index;
    char Char = 0;


    if ((Index = BrackIndex(c)) != ~0UL)
        Char = BrOrder[Index ^ 1];

    return (Char);
}
