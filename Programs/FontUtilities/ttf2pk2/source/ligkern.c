/*
 *   ligkern.c
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stddef.h>         /* for size_t */
#include <ctype.h>

#include "ttf2tfm.h"
#include "ligkern.h"
#include "ttfenc.h"
#include "texenc.h"
#include "newobj.h"
#include "errormsg.h"


static char *
paramstring(char **curp)
{
  register char *p, *q;


  p = *curp;
  while (*p && !isspace((unsigned char)*p))
    p++;
  q = *curp;
  if (*p != '\0')
    *p++ = '\0';
  while (isspace((unsigned char)*p))
    p++;
  *curp = p;
  return q;
}


/*
 *   Some routines to remove kerns that match certain patterns.
 */

static kern *
rmkernmatch(kern *k,
            char *s)
{
  kern *nk;


  while (k && strcmp(k->succ, s) == 0)
    k = k->next;

  if (k)
  {
    for (nk = k; nk; nk = nk->next)
      while (nk->next && strcmp(nk->next->succ, s) == 0)
        nk->next = nk->next->next;
  }
  return k;
}


/*
 *   Recursive to one level.
 */

static void
rmkern(char *s1, char *s2,
       ttfinfo *ti,
       Font *fnt)
{
  if (ti == NULL)
  {
    if (strcmp(s1, "*") == 0)
    {
      for (ti = fnt->charlist; ti; ti = ti->next)
        rmkern(s1, s2, ti, fnt);
      return;
    }
    else
    {
      ti = findadobe(s1, fnt->charlist);
      if (ti == NULL)
        return;
    }
  }

  if (strcmp(s2, "*") == 0)
    ti->kerns = NULL; /* drop them on the floor */
  else
    ti->kerns = rmkernmatch(ti->kerns, s2);
}


/*
 *   Make the kerning for character S1 equivalent to that for S2.
 *   If either S1 or S2 do not exist, do nothing.
 *   If S1 already has kerning, do nothing.
 */

static void
addkern(char *s1, char *s2,
        Font *fnt)
{
  ttfinfo *ti1 = findadobe(s1, fnt->charlist);
  ttfinfo *ti2 = findadobe(s2, fnt->charlist);


  if (ti1 && ti2 && !ti1->kerns)
  {
    /* Put the new one at the head of the list, since order is immaterial. */
    ttfptr *ap = (ttfptr *)mymalloc(sizeof (ttfptr));


    ap->next = ti2->kern_equivs;
    ap->ch = ti1;
    ti2->kern_equivs = ap;
  }
}


/*
 *   Reads a ligkern line, if this is one.  Assumes the first character
 *   passed is `%'.
 */

void
checkligkern(char *s, Font *fnt)
{
  char *mlist[5];
  char *os;
  char *orig_s, *pos;
  size_t offset[5];
  int n;


  os = newstring(s);
  orig_s = s;

  s++;
  while (isspace((unsigned char)*s))
    s++;
  if (strncmp(s, "LIGKERN", 7) == 0)
  {
    fnt->sawligkern = True;
    s += 7;
    while (isspace((unsigned char)*s))
      s++;
    pos = s;
    while (*pos)
    {
      for (n = 0; n < 5;)
      {
        if (*pos == '\0')
          break;
        offset[n] = pos - orig_s;
        mlist[n] = paramstring(&pos);
        if (strcmp(mlist[n], ";") == 0)
          break;
        n++;
      }

      if (n > 4)
        boops(os, pos - orig_s, "Too many parameters in lig kern data.");
      if (n < 3)
        boops(os, pos - orig_s, "Too few parameters in lig kern data.");

      if (n == 3 && strcmp(mlist[1], "{}") == 0)        /* rmkern command */
        rmkern(mlist[0], mlist[2], (ttfinfo *)0, fnt);
      else if (n == 3 && strcmp(mlist[1], "<>") == 0)   /* addkern */
        addkern(mlist[0], mlist[2], fnt);
      else if (n == 3 && strcmp(mlist[0], "||") == 0 &&
               strcmp(mlist[1], "=") == 0)              /* bc command */
      {
        ttfinfo *ti = findadobe("||", fnt->charlist);


        if (fnt->boundarychar != -1)
          boops(os, offset[0], "Multiple boundary character commands?");
        if (sscanf(mlist[2], "%d", &n) != 1)
          boops(os, offset[2],
                "Expected number assignment for boundary char.");
        if (n < 0 || n > 0xFF)
          boops(os, offset[2], "Boundary character number must be 0..0xFF.");

        fnt->boundarychar = n;
        if (ti == NULL)
          oops("Internal error: boundary char.");
        ti->outcode = n; /* prime the pump, so to speak, for lig/kerns */
      }
      else if (n == 4)
      {
        int op = -1;
        ttfinfo *ti;


        for (n = 0; encligops[n]; n++)
          if (strcmp(mlist[2], encligops[n]) == 0)
          {
            op = n;
            break;
          }
        if (op < 0)
          boops(os, offset[2], "Bad ligature op specified.");

        if (NULL != (ti = findadobe(mlist[0], fnt->charlist)))
        {
          lig *lig;


          if (findadobe(mlist[1], fnt->charlist))
                                                /* remove coincident kerns */
            rmkern(mlist[0], mlist[1], ti, fnt);

          if (strcmp(mlist[3], "||") == 0)
            boops(os, offset[3], "You can't lig to the boundary character!");

          if (!fnt->fixedpitch)         /* fixed pitch fonts get *0* ligs */
          {
            for (lig = ti->ligs; lig; lig = lig->next)
              if (strcmp(lig->succ, mlist[1]) == 0)
                break;                  /* we'll re-use this structure */

            if (lig == NULL)
            {
              lig = newlig();
              lig->succ = newstring(mlist[1]);
              lig->next = ti->ligs;
              ti->ligs = lig;
            }
            lig->sub = newstring(mlist[3]);
            lig->op = op;
 
            if (strcmp(mlist[1], "||") == 0)
            {
              lig->boundleft = 1;
              if (strcmp(mlist[0], "||") == 0)
                boops(os, offset[0],
                      "You can't lig boundarychar boundarychar!");
            }
            else
              lig->boundleft = 0;
          }
        }
      }
      else
        boops(os, offset[0], "Bad form in LIGKERN command.");
    }
  }

  free(os);
}


void
getligkerndefaults(Font *fnt)
{
  int i;
  char *buffer;


  for (i = 0; staticligkern[i]; i++)
  {
    buffer = newstring(staticligkern[i]);
    checkligkern(buffer, fnt);
    free(buffer);
  }
}


/* end */
