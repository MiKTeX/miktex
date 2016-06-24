/*
 *   newobj.h
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifndef NEWOBJ_H
#define NEWOBJ_H

#include <stdio.h>
#include "ttf2tfm.h"


#if defined(HAVE_LIBKPATHSEA)
#include <kpathsea/config.h>
#define mymalloc xmalloc
#define mycalloc xcalloc
#define myrealloc xrealloc
#else
#if (defined(MSDOS) && defined(__TURBOC__)) || \
    (defined(OS2) && defined(_MSC_VER))
#define SMALLMALLOC
#endif

void *mymalloc(size_t len);
void *mycalloc(size_t nmemb, size_t len);
void *myrealloc(void *oldp, size_t len);
#endif

char *get_line(FILE *f);
Boolean texlive_getline(char **bufferp, FILE *f);
char *newstring(const char *s);

ttfinfo *newchar(Font *fnt);
kern *newkern(void);
pcc *newpcc(void);
lig *newlig(void);
stringlist *newstringlist(void);

void init_font_structure(Font *fnt);

#endif /* NEWOBJ_H */


/* end */
