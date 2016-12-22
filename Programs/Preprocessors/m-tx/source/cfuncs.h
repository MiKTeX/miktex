/*  cfuncs.h  DPL 1997-03-31, 2001-12-22  */

#ifndef CFUNCS_H
# define CFUNCS_H

#define STRINGS_H
typedef char Char;
typedef void Void;
#define Static static
#define Local static
#define SHORT_MAX 65535

# define FileNotFound     10
# define FileWriteError   38
#ifndef STDIO_H
# include <stdio.h>
#endif
#ifndef CTYPE_H
# include <ctype.h>
#endif
#ifndef STRING_H
# include <string.h>
#endif
#ifndef STDLIB_H
# include <stdlib.h>
#endif
#ifndef _FNSIZE
# define _FNSIZE 120
#endif

typedef unsigned  boolean;
#define false 0
#define true 1

extern int _EscIO (int);
extern int _EscIO2 (int errorcode, char* filename);
extern int _Escape (int);
extern int P_eof (FILE *);
extern int _SETIO (int, int);

extern void PASCAL_MAIN (int, char **);
extern int P_argc;
extern int P_ioresult;
extern char **P_argv;

#ifdef CFUNCS_C
# define vextern
#else
# define vextern extern
#endif
/* String handling primitives. */

extern void scan1(char *s, short p, short *n);
/* Read an integer at position p of s */
extern boolean startsWith(char *s1, char *s2);
/* True if s1 starts with s2 */
extern short pos1(char c, char *s);
extern short posNot(char c, char *s);
extern void insertChar(char c, char *s, short p);
extern char *substr_(char *Result, char *s, short start, short count);
extern void getNum(char *line, short *k);
extern void getTwoNums(char *line, short *k1, short *k2);
extern void toUpper(char *s);
extern void delete1(char *s, short p);
extern void predelete(char *s, short l);
extern void shorten(char *s, short new_length);
extern short strpos2 (char *s1, char *s2, short p);
extern short nextWordBound(char *s, char trigger, short p);

#undef vextern

#endif /*CFUNCS_H*/

