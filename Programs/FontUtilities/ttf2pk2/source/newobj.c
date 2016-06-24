/*
 *   newobj.c
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000, 2002 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stddef.h>         /* for size_t */
#include <string.h>

#include "newobj.h"
#include "ttf2tfm.h"
#include "errormsg.h"
#include "texenc.h"


#if !defined(HAVE_LIBKPATHSEA)
void *
mymalloc(size_t len)
{
  void *p;


#ifdef SMALLMALLOC
  if (len > 65500L)
    oops("Cannot allocate more than 64kByte.");
#endif

  if (len)
    p = malloc(len);
  else
    p = malloc(1);

  if (p == NULL)
    oops("Out of memory.");

  return p;
}


void *
mycalloc(size_t nmemb, size_t len)
{
  void *p;

  p = calloc(nmemb ? nmemb : 1, len ? len : 1);

  if (p == NULL)
    oops("Out of memory.");

  return p;
}


void *
myrealloc(void *oldp, size_t len)
{
  void *p;


#ifdef SMALLMALLOC
  if (len > 65500L)
    oops("Cannot allocate more than 64kByte.");
#endif

  if (len)
    p = realloc(oldp, len);
  else
    p = realloc(oldp, 1);

  if (p == NULL)
    oops("Out of memory.");

  return p;
}
#endif


/*
 *   This routine gets a line from a file, supporting continuation with
 *   '\' as the last character on the line.
 *
 *   In case of error, NULL is returned.  If EOF is reached a pointer to
 *   a null string is returned.  The final newline will be retained.
 */

char *
get_line(FILE *f)
{
  size_t linelen, len;
  char *buffer;
  int c;


  linelen = 80;
  len = 0;

  buffer = (char *)mymalloc(linelen);

  while (1)
  {
    c = fgetc(f);
    buffer[len++] = c;

    if (len == linelen - 1)
    {
      linelen += 80;
      buffer = (char *)myrealloc(buffer, linelen);
    }

  again:
    switch (c)
    {
    case '\\':
      c = fgetc(f);
      if (c == '\n')
      {
        len--;
        break;
      }
      else
      {
        buffer[len++] = c;
        goto again;
      }
    case '\n':
      buffer[len] = '\0';
      return buffer;
    case EOF:
      buffer[len - 1] = '\0';
      if (feof(f))
        return buffer;
      else
        return NULL;
    default:
      break;
    }
  }
}


/*
 *   texlive_getline() is a wrapper function for get_line().  It returns `False' in
 *   case of error and expects a pointer to a buffer to store the current
 *   line.   Additionally, the final newline character is stripped.
 */

Boolean
texlive_getline(char **bufferp, FILE *f)
{
  size_t l;


  *bufferp = get_line(f);

  if (!(*bufferp && **bufferp))
    return 0;

  l = strlen(*bufferp);
  if (l > 0)
    (*bufferp)[l - 1] = '\0';

  return 1;
}


char *
newstring(const char *s)
{
  char *q;

  if (s)
  {
    q = mymalloc(strlen(s) + 1);
    (void)strcpy(q, s);
    return q;
  }
  else
    return NULL;
}


ttfinfo *
newchar(Font *fnt)
{
  register ttfinfo *ti;


  ti = (ttfinfo *)mymalloc(sizeof (ttfinfo));

  ti->next = fnt->charlist;

  ti->charcode = -1;
  ti->glyphindex = -1;
  ti->incode = -1;
  ti->outcode = -1;
  ti->adobename = NULL;

  ti->width = -1;
  ti->llx = -1;
  ti->lly = -1;
  ti->urx = -1;
  ti->ury = -1;

  ti->ligs = NULL;
  ti->kerns = NULL;
  ti->kern_equivs = NULL;
  ti->pccs = NULL;

  ti->constructed = False;

  ti->wptr = 0;
  ti->hptr = 0;
  ti->dptr = 0;
  ti->iptr = 0;

  ti->fntnum = -1;

  fnt->charlist = ti;

  return ti;
}


kern *
newkern(void)
{
  register kern *nk;


  nk = (kern *)mymalloc(sizeof (kern));
  nk->next = NULL;
  nk->succ = NULL;
  nk->delta = 0;

  return nk;
}


pcc *
newpcc(void)
{
  register pcc *np;


  np = (pcc *)mymalloc(sizeof (pcc));
  np->next = NULL;
  np->partname = NULL;
  np->xoffset = 0;
  np->yoffset = 0;

  return np;
}


lig *
newlig(void)
{
  register lig *nl;


  nl = (lig *)mymalloc(sizeof (lig));
  nl->next = NULL;
  nl->succ = NULL;
  nl->sub = NULL;
  nl->op = 0;               /* the default =: op */
  nl->boundleft = 0;

  return nl;
}


stringlist *
newstringlist(void)
{
  register stringlist *sl;


  sl = (stringlist *)mymalloc(sizeof (stringlist));
  sl->next = NULL;
  sl->old_name = NULL;
  sl->new_name = NULL;
  sl->single_replacement = False;

  return sl;
}


void
init_font_structure(Font *fnt)
{
  int i;


  fnt->ttfname = NULL;

  fnt->tfm_path = NULL;
  fnt->tfm_ext = NULL;

  fnt->outname = NULL;
  fnt->subfont_name = NULL;
  fnt->outname_postfix = NULL;
  fnt->fullname = NULL;

  fnt->vplout = NULL;
  fnt->tfmout = NULL;

  fnt->inencname = NULL;
  fnt->inencoding = NULL;

  fnt->replacements = NULL;
  fnt->replacementname = NULL;

  fnt->outencname = NULL;
  fnt->outencoding = NULL;

  fnt->sfdname = NULL;

  fnt->sawligkern = False;
  fnt->subfont_ligs = False;
  fnt->ligname = NULL;
  fnt->write_enc = False;

  fnt->charlist = NULL;

  fnt->boundarychar = -1;
  fnt->codingscheme = default_codingscheme;
  fnt->titlebuf = NULL;

  fnt->cksum = 0;
  fnt->subfont_num = 0;
  fnt->subfont_list = NULL;

  fnt->units_per_em = 0;
  fnt->italicangle = 0.0;
  fnt->fixedpitch = 0;

  fnt->fontindex = 0;
  fnt->pid = 3;
  fnt->eid = 1;

  fnt->xheight = 0;
  fnt->fontspace = 0;

  fnt->y_offset = 0.25;

  fnt->efactor = 1.0;
  fnt->slant = 0;
  fnt->capheight = 0.8;
  fnt->PSnames = No;
  fnt->rotate = False;

  fnt->efactorparam = NULL;
  fnt->slantparam = NULL;
  fnt->fontindexparam = NULL;
  fnt->pidparam = NULL;
  fnt->eidparam = NULL;
  fnt->y_offsetparam = NULL;

  for (i = 0; i < 256; i++)
  {
    fnt->inencptrs[i] = NULL;
    fnt->outencptrs[i] = NULL;
    fnt->uppercase[i] = NULL;
    fnt->lowercase[i] = NULL;
    fnt->sf_code[i] = -1;
    fnt->nextout[i] = -1;           /* encoding chains have length 0 */
  }
}


/* end */
