#ifndef _UTYPE_H
#define _UTYPE_H

/* This file was generated using the program 'makeutype' */

#include <ctype.h>		/* Include here so we can control it. If a system header includes it later bad things happen */
#include <unibasics.h>		/* Include here so we can use pre-defined int types to correctly size constant data arrays. */
#ifdef tolower
# undef tolower
#endif
#ifdef isupper
# undef isupper
#endif
#ifdef isalpha
# undef isalpha
#endif
#ifdef isdigit
# undef isdigit
#endif
#ifdef isalnum
# undef isalnum
#endif
#ifdef isspace
# undef isspace
#endif
#ifdef ishexdigit
# undef ishexdigit
#endif

#define ____L	0x1
#define ____U	0x2
#define ____TITLE	0x4
#define ____D	0x8
#define ____S	0x10
#define ____X	0x20
#define ____COMBINE	0x40
#define ____AL	0x80

extern const unsigned short ____tolower[];
extern const unsigned short ____utype[];

#define tolower(ch) (____tolower[(ch)+1])
#define isupper(ch) (____utype[(ch)+1]&____U)
#define isalpha(ch) (____utype[(ch)+1]&(____L|____U|____TITLE|____AL))
#define isdigit(ch) (____utype[(ch)+1]&____D)
#define isalnum(ch) (____utype[(ch)+1]&(____L|____U|____TITLE|____AL|____D))
#define isspace(ch) (____utype[(ch)+1]&____S)
#define ishexdigit(ch) (____utype[(ch)+1]&____X)
#define iscombining(ch) (____utype[(ch)+1]&____COMBINE)

#endif
