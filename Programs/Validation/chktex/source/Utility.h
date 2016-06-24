/*
 *  ChkTeX, utility functions -- header file.
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

#ifndef UTILITY_H
#define UTILITY_H 1

#include "ChkTeX.h"
#include "OpSys.h"

/*
 * How many indexes we'll allocate first time
 */
#define MINPUDDLE       256

/*
 * How many bytes we want in front/end of each char buffer. > 2
 */
#define WALLBYTES       4

#ifndef HASH_SIZE
#  define HASH_SIZE    1009     /* A reasonably large prime */
#endif

#define FORWL(ind, list)  for(ind = 0; ind < (list).Stack.Used; ind++)


enum Strip
{
    STRP_LFT = 0x01,
    STRP_RGT = 0x02,
    STRP_BTH = 0x03
};

struct HashEntry
{
    struct HashEntry *Next;
    char *Str;
};

struct Hash
{
    struct HashEntry **Index;
};

struct Stack
{
    void **Data;
    unsigned long Size, Used;
};

struct WordList
{
    unsigned long MaxLen;
    int NonEmpty;
    struct Stack Stack;
    struct Hash Hash;
};
#define WORDLIST_DEFINED

struct FileNode
{
    char *Name;
    FILE *fh;
    unsigned long Line;
};

/* Rotates x n bits left (should be an int, long, etc.) */
#define ROTATEL(x,n) ((x<<n) | (x>>((CHAR_BIT*sizeof(x)) - n)))

/* Rotates x n bits right (should be an int, long, etc.) */
#define ROTATER(x,n) ((x>>n) | (x<<((CHAR_BIT*sizeof(x)) - n)))

/* Subtract 1 because sizeof includes the null terminator.
 * WARNING: To use this on a variable, the type should be char[]
 * rather than char*, since for some versions of gcc these give
 * different values. */
#define STRLEN(x)  (sizeof(x)/sizeof(x[0]) - 1)

int fexists(const char *Filename);

void *sfmemset(void *to, int c, long n);
void *saferealloc(void *old, size_t newsize);

int strafter(const char *Str, const char *Cmp);
void strrep(char *String, const char From, const char To);
void strxrep(char *Buf, const char *Prot, const char To);
char *strip(char *String, const enum Strip What);
void strwrite(char *To, const char *From, unsigned long Len);
int strinfront(const char *Str, const char *Cmp);
char *strdupx(const char *String, int Extra);
void strmove(char *a, const char *b);

void ClearHash(struct Hash *h);
void InsertHash(char *a, struct Hash *h);
char *HasHash(const char *a, const struct Hash *h);

int InsertWord(const char *Word, struct WordList *WL);
char *HasWord(const char *Word, struct WordList *WL);
void MakeLower(struct WordList *wl);
void ListRep(struct WordList *wl, const char From, const char To);
void ClearWord(struct WordList *WL);

int StkPush(void *Data, struct Stack *Stack);
void *StkPop(struct Stack *Stack);
void *StkTop(struct Stack *Stack);

FILE *CurStkFile(struct Stack *stack);
const char *CurStkName(struct Stack *stack);
unsigned long CurStkLine(struct Stack *stack);
char *FGetsStk(char *Dest, unsigned long len, struct Stack *stack);
int PushFileName(const char *Name, struct Stack *stack);
int PushFile(const char *, FILE *, struct Stack *);


void FreeErrInfo(struct ErrInfo *ei);
struct ErrInfo *PushChar(const char c, const unsigned long Line,
                         const unsigned long Column, struct Stack *Stk,
                         const char *LineCpy);
struct ErrInfo *PushErr(const char *Data, const unsigned long Line,
                        const unsigned long Column,
                        const unsigned long ErrLen, const char *LineCpy,
                        struct Stack *Stk);
struct ErrInfo *TopChar(struct Stack *Stack);
struct ErrInfo *TopErr(struct Stack *Stack);
struct ErrInfo *PopErr(struct Stack *Stack);
struct ErrInfo *TopMatch(struct Stack *Stack, char *String);

long BrackIndex(const char c);
void AddBracket(const char c);
char MatchBracket(const char c);



short substring(const char *source, char *dest, unsigned long pos, long len);

#ifndef  HAVE_STRLWR
#  define  strlwr  mystrlwr
char *mystrlwr(char *String);
#endif

#ifndef  HAVE_STRDUP
#  define  strdup  mystrdup
char *mystrdup(const char *String);
#endif


#ifndef HAVE_STRCASECMP
int strcasecmp(char *a, char *b);
#endif

#if !(defined HAVE_DECL_STPCPY && HAVE_DECL_STPCPY)
static inline char * stpcpy(char *dest, const char *src)
{
    return strcpy(dest, src) + strlen(src);
}
#endif


#endif /* UTILITY_H */
