/*
 *   vplaux.c
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "ttf2tfm.h"
#include "newobj.h"
#include "ttfenc.h"
#include "texenc.h"
#include "tfmaux.h"
#include "vplaux.h"
#include "errormsg.h"
#include "case.h"


#undef PI
#define PI 3.14159265358979323846264338327

     
#define vout(s) fprintf(out, s)

#define voutln(str)         {fprintf(out, "%s\n", str); vlevout(level);}
#define voutln2(f, s)       {fprintf(out, f, s); vlevnlout(level);}
#define voutln3(f, a, b)    {fprintf(out, f, a, b); vlevnlout(level);}
#define voutln4(f, a, b, c) {fprintf(out, f, a, b, c); vlevnlout(level);}


static char vcharbuf[6];
static char vnamebuf[100];

/* the depth of parenthesis nesting in VPL file being written */
static int level;


static FILE *out;


static void
vlevout(register int l)
{
  while (l--)
    vout("   ");
}


static void
vlevnlout(int level)
{
  vout("\n");
  vlevout(level);
}


static void
vleft(int *levelp)
{
  (*levelp)++;
  vout("(");
}


static void
vright(int *levelp)
{
  (*levelp)--;
  voutln(")");
}


static char *
vchar(int c,
      char *buf,
      Boolean forceoctal)
{
  if (forceoctal == 0 && isalnum(c))
    (void)sprintf(buf, "C %c", c);
  else
    (void)sprintf(buf, "O %o", (unsigned)c);
  return buf;
}


static char *
vname(int c,
      char *buf,
      ttfinfo **array,
      Boolean forceoctal)
{
  if (!forceoctal && isalnum(c))
    buf[0] = '\0';
  else
    sprintf(buf, " (comment %s)", array[c]->adobename);
  return buf;
}


static int
texheight(register ttfinfo *ti,
          ttfinfo *ac,
          int xh)
{
  register const char **p;
  register ttfinfo *aci, *acci;
  char buffer[200];


  if (xh <= 50 || *(ti->adobename + 1))
    return ti->ury;                     /* that was the simple case */

  for (p = accents; *p; p++)    /* otherwise we look for accented letters.  */
                                /* We even check glyphs not in any encoding */
    if (NULL != (aci = findadobe(*p, ac)))
    {
      strcpy(buffer, ti->adobename);
      strcat(buffer, *p);
      if (NULL != (acci = findadobe(buffer, ac)))
        return acci->ury - aci->ury + xh;
    }
  return ti->ury;
}


/*
 *   Compute uppercase mapping, when making a small caps font.
 */

void
upmap(Font *fnt)
{
  register ttfinfo *ti, *Ti;
  register const char *p;
  register char *q;
  register pcc *np, *nq;
  int i, j;
  char lwr[50];


  for (Ti = fnt->charlist; Ti; Ti = Ti->next)
  {
    p = Ti->adobename;
    if (isupper((unsigned char)*p))
    {
      q = lwr;
      for (; *p; p++)
        *q++ = tolower((unsigned char)*p);
      *q = '\0';

      if (NULL != (ti = findmappedadobe(lwr, fnt->inencptrs)))
      {
        for (i = ti->outcode; i >= 0; i = fnt->nextout[i])
          fnt->uppercase[i] = Ti;
        for (i = Ti->outcode; i >= 0; i = fnt->nextout[i])
          fnt->lowercase[i] = ti;
      }
    }
  }

  /*
   *   Note that, contrary to the normal true/false conventions,
   *   uppercase[i] is NULL and lowercase[i] is non-NULL when `i' is the
   *   ASCII code of an uppercase letter; and vice versa for lowercase 
   *   letters.
   */

  if (NULL != (ti = findmappedadobe("germandbls", fnt->inencptrs)))
    if (NULL != (Ti = findmappedadobe("S", fnt->inencptrs)))
                                                /* we also construct SS */
    {
      for (i = ti->outcode; i >= 0; i = fnt->nextout[i])
        fnt->uppercase[i] = ti;
      ti->incode = -1;
      ti->width = Ti->width << 1;
      ti->llx = Ti->llx;
      ti->lly = Ti->lly;
      ti->urx = Ti->width + Ti->urx;
      ti->ury = Ti->ury;
      ti->kerns = Ti->kerns;

      np = newpcc();
      np->partname = "S";
      nq = newpcc();
      nq->partname = "S";
      nq->xoffset = Ti->width;
      np->next = nq;
      ti->pccs = np;
      ti->constructed = True;
    }

  for (i = 0; casetable[i].upper; i++)
  {
    if ((ti = findmappedadobe(casetable[i].lower, fnt->inencptrs)))
      for (j = ti->outcode; j >= 0; j = fnt->nextout[j])
        fnt->uppercase[j] = findmappedadobe(casetable[i].upper,
                                            fnt->inencptrs);
  }
}

/*
 *   The logic above seems to work well enough, but it leaves useless
 *   characters like `fi' and `fl' in the font if they were present
 *   initially, and it omits characters like `dotlessj' if they are
 *   absent initially.
 */


void
writevpl(Font *fnt, char makevpl, Boolean forceoctal)
{
  register int i, j, k;
  register ttfinfo *ti;
  register lig *nlig;
  register kern *nkern;
  register pcc *npcc;
  ttfinfo *asucc, *asub, *api;
  ttfptr *kern_eq;
  int xoff, yoff, ht;
  int bc, ec;
  char buf[200];
  char header[256];
  Boolean unlabeled;
  float Slant;


  out = fnt->vplout;

  header[0] = '\0';
  strncat(header, "Created by `", 12);
  strncat(header, fnt->titlebuf, 255 - 12 - 1);
  strncat(header, "'", 1);

  voutln2("(VTITLE %s)", header);
  voutln("(COMMENT Please change VTITLE if you edit this file)");
  (void)sprintf(buf, "TeX-%s%s%s%s",
                fnt->fullname,
                (fnt->efactor == 1.0 ? "" : "-E"),
                (fnt->slant == 0.0 ? "" : "-S"),
                (makevpl == 1 ? "" : "-CSC"));

  if (strlen(buf) > 19) /* too long, will retain first 9 and last 10 chars */
  {
    register char *p, *q;


    for (p = &buf[9], q = &buf[strlen(buf)-10]; p < &buf[19];
         p++, q++)
      *p = *q;
    buf[19] = '\0';
  }
  voutln2("(FAMILY %s)", buf);

  {
    char tbuf[300];
    const char *base_encoding = fnt->codingscheme;

      
    if (strcmp(fnt->outencoding->name, base_encoding) == 0)
      sprintf(tbuf, "%s", fnt->outencoding->name);
    else
      sprintf(tbuf, "%s + %s", base_encoding, fnt->outencoding->name);
      
    if (strlen(tbuf) > 39)
    {
      warning("Coding scheme too long; shortening to 39 characters");
      tbuf[39] = '\0';
    }
    voutln2("(CODINGSCHEME %s)", tbuf);
  }

  {
    long t, sc;
    char *s;
    int n, pos;


    s = header;
    n = strlen(s);
    t = ((long)n) << 24;
    sc = 16;
    pos = 18;

    voutln(
      "(COMMENT The following `HEADER' lines are equivalent to the string)");
    voutln2("(COMMENT   \"%s\")", header);

    while (n > 0)
    {
      t |= ((long)(*(unsigned char *)s++)) << sc;
      sc -= 8;
      if (sc < 0)
      {
        voutln3("(HEADER D %d O %lo)", pos, t);
        t = 0;
        sc = 24;
        pos++;
      }
      n--;
    }
    if (t)
      voutln3("(HEADER D %d O %lo)", pos, t);
  }

  voutln("(DESIGNSIZE R 10.0)");
  voutln("(DESIGNUNITS R 1000)");
  voutln("(COMMENT DESIGNSIZE (1 em) IS IN POINTS)");
  voutln("(COMMENT OTHER DIMENSIONS ARE MULTIPLES OF DESIGNSIZE/1000)");

#if 0
  /* Let vptovf compute the checksum. */
  voutln2("(CHECKSUM O %lo)", cksum ^ 0xFFFFFFFF);
#endif

  if (fnt->boundarychar >= 0)
    voutln2("(BOUNDARYCHAR O %lo)", (unsigned long)fnt->boundarychar);

  vleft(&level);
  voutln("FONTDIMEN");

  Slant = fnt->slant - fnt->efactor * tan(fnt->italicangle * (PI / 180.0));

  if (Slant)
    voutln2("(SLANT R %f)", Slant);
  voutln2("(SPACE D %d)", fnt->fontspace);
  if (!fnt->fixedpitch)
  {
    voutln2("(STRETCH D %d)", transform(200, 0, fnt->efactor, fnt->slant));
    voutln2("(SHRINK D %d)", transform(100, 0, fnt->efactor, fnt->slant));
  }
  voutln2("(XHEIGHT D %d)", fnt->xheight);
  voutln2("(QUAD D %d)", transform(1000, 0, fnt->efactor, fnt->slant));
  voutln2("(EXTRASPACE D %d)",
          fnt->fixedpitch ? fnt->fontspace :
                            transform(111, 0, fnt->efactor, fnt->slant));
  vright(&level);

  vleft(&level);
  voutln("MAPFONT D 0");
  voutln2("(FONTNAME %s)", fnt->fullname);
#if 0
  voutln2("(FONTCHECKSUM O %lo)", (unsigned long)cksum);
#endif
  vright(&level);

  if (makevpl > 1)
  {
    vleft(&level);
    voutln("MAPFONT D 1");
    voutln2("(FONTNAME %s)", fnt->fullname);
    voutln2("(FONTAT D %d)", (int)(1000.0 * fnt->capheight + 0.5));
#if 0
    voutln2("(FONTCHECKSUM O %lo)", (unsigned long)cksum);
#endif
    vright(&level);
  }

  for (i = 0; i <= 0xFF && fnt->outencptrs[i] == NULL; i++)
    ;
  bc = i;
  for (i = 0xFF; i >= 0 && fnt->outencptrs[i] == NULL; i--)
    ;
  ec = i;

  vleft(&level);
  voutln("LIGTABLE");
  ti = findadobe("||", fnt->charlist);
  unlabeled = True;
  for (nlig = ti->ligs; nlig; nlig = nlig->next)
    if (NULL != (asucc = findmappedadobe(nlig->succ, fnt->inencptrs)))
    {
      if (NULL != (asub = findmappedadobe(nlig->sub, fnt->inencptrs)))
        if (asucc->outcode >= 0)
          if (asub->outcode >= 0)
          {
            if (unlabeled)
            {
               voutln("(LABEL BOUNDARYCHAR)");
               unlabeled = False;
            }
            for (j = asucc->outcode; j >= 0; j = fnt->nextout[j])
              voutln4("(%s %s O %o)", vplligops[nlig->op],
                      vchar(j, vcharbuf, forceoctal),
                      (unsigned)asub->outcode);
          }
    }
  if (!unlabeled)
    voutln("(STOP)");

  for (i = bc; i <= ec; i++)
    if ((ti = fnt->outencptrs[i]) && ti->outcode == i)
    {
      unlabeled = True;
      if (fnt->uppercase[i] == NULL)
                                /* omit ligatures from smallcap lowercase */
        for (nlig = ti->ligs; nlig; nlig = nlig->next)
          if ((((NULL != (asucc = findmappedadobe(nlig->succ, fnt->inencptrs)))
                && (asucc->outcode >= 0))
               || ((strcmp(nlig->succ, "||") == 0)
                   && (fnt->boundarychar >= 0)))
              && (NULL != (asub = findmappedadobe(nlig->sub, fnt->inencptrs)))
              && (asub->outcode >= 0))
          {
            if (unlabeled)
            {
              for (j = ti->outcode; j >= 0; j = fnt->nextout[j])
                voutln3("(LABEL %s)%s",
                        vchar(j, vcharbuf, forceoctal),
                        vname(j, vnamebuf,
                              fnt->outencptrs, forceoctal));
              unlabeled = False;
            }
            if (asucc != NULL)
              for (j = asucc->outcode; j >= 0; j = fnt->nextout[j])
              {
                voutln4("(%s %s O %o)", vplligops[nlig->op],
                        vchar(j, vcharbuf, forceoctal),
                        (unsigned)asub->outcode);
                if (nlig->boundleft)
                  break;
              }
            else /* boundary char */
            {
              voutln4("(%s %s O %o)", vplligops[nlig->op],
                      vchar(fnt->boundarychar, vcharbuf, forceoctal),
                      (unsigned)asub->outcode);
            }
          }
 
      for (nkern = (fnt->uppercase[i] ? fnt->uppercase[i]->kerns : ti->kerns);
           nkern; nkern=nkern->next)
        if (NULL != (asucc = findmappedadobe(nkern->succ, fnt->inencptrs)))
          for (j = asucc->outcode; j >= 0; j = fnt->nextout[j])
          {
            if (fnt->uppercase[j] == NULL)
            {
              if (unlabeled)
              {
                for (k = ti->outcode; k >= 0; k = fnt->nextout[k])
                  voutln3("(LABEL %s)%s",
                          vchar(k, vcharbuf, forceoctal),
                          vname(k, vnamebuf, fnt->outencptrs, forceoctal));
                unlabeled = False;
              }

              /*
               *   If other characters have the same kerns as this
               *   one, output the label here.  This makes the TFM
               *   file much smaller than if we output all the
               *   kerns again under a different label.
               */

              for (kern_eq = ti->kern_equivs; kern_eq;
                   kern_eq = kern_eq->next)
              {
                k = kern_eq->ch->outcode;
                if (k >= 0 && k <= 0xFF)
                  voutln3("(LABEL %s)%s",
                          vchar(k, vcharbuf, forceoctal),
                          vname(k, vnamebuf, fnt->outencptrs, forceoctal));
              }
              ti->kern_equivs = NULL;   /* Only output those labels once. */

              if (fnt->uppercase[i])
              {
                if (fnt->lowercase[j])
                {
                  for (k = fnt->lowercase[j]->outcode; k >= 0;
                       k = fnt->nextout[k])
                    voutln4("(KRN %s R %.1f)%s",
                            vchar(k, vcharbuf, forceoctal),
                            fnt->capheight * nkern->delta,
                            vname(k, vnamebuf, fnt->outencptrs, forceoctal));
                }
                else
                  voutln4("(KRN %s R %.1f)%s",
                          vchar(j, vcharbuf, forceoctal),
                          fnt->capheight * nkern->delta,
                          vname(j, vnamebuf, fnt->outencptrs, forceoctal));
              }
              else
              {
                voutln4("(KRN %s R %d)%s",
                        vchar(j, vcharbuf, forceoctal),
                        nkern->delta,
                        vname(j, vnamebuf, fnt->outencptrs, forceoctal));
                if (fnt->lowercase[j])
                  for (k = fnt->lowercase[j]->outcode; k >= 0;
                       k = fnt->nextout[k])
                    voutln4("(KRN %s R %.1f)%s",
                            vchar(k, vcharbuf, forceoctal),
                            fnt->capheight * nkern->delta,
                            vname(k, vnamebuf, fnt->outencptrs, forceoctal));
              }
            }
          }
          if (!unlabeled)
            voutln("(STOP)");
    }
  vright(&level);

  for (i = bc; i <= ec; i++)
    if (NULL != (ti = fnt->outencptrs[i]))
    {
      vleft(&level);
      fprintf(out, "CHARACTER %s%s\n   ",
                   vchar(i, vcharbuf, forceoctal),
                   vname(i, vnamebuf, fnt->outencptrs, forceoctal));

      if (fnt->uppercase[i])
      {
        ti = fnt->uppercase[i];
        voutln2("(CHARWD R %.1f)", fnt->capheight * (ti->width));
        if (0 != (ht = texheight(ti, fnt->charlist, fnt->xheight)))
          voutln2("(CHARHT R %.1f)", fnt->capheight * ht);
        if (ti->lly)
          voutln2("(CHARDP R %.1f)", -fnt->capheight * ti->lly);
        if (ti->urx > ti->width)
          voutln2("(CHARIC R %.1f)", fnt->capheight * (ti->urx - ti->width));
      }
      else
      {
        voutln2("(CHARWD R %d)", ti->width);
        if (0 != (ht = texheight(ti, fnt->charlist, fnt->xheight)))
          voutln2("(CHARHT R %d)", ht);
        if (ti->lly)
          voutln2("(CHARDP R %d)", -ti->lly);
        if (ti->urx > ti->width)
          voutln2("(CHARIC R %d)", ti->urx - ti->width);
      }
 
      if (ti->incode != i || fnt->uppercase[i] || ti->constructed)
      {
        vleft(&level);
        voutln("MAP");
        if (fnt->uppercase[i])
          voutln("(SELECTFONT D 1)");

        if (ti->pccs && (ti->incode < 0 || ti->constructed))
        {
          xoff = 0;
          yoff = 0;

          for (npcc = ti->pccs; npcc; npcc = npcc->next)
            if (NULL != (api = findmappedadobe(npcc->partname,
                                               fnt->inencptrs)))
              if (api->outcode >= 0)
              {
                if (npcc->xoffset != xoff)
                {
                  if (fnt->uppercase[i])
                  {
                    voutln2("(MOVERIGHT R %.1f)",
                            fnt->capheight * (npcc->xoffset - xoff));
                  }
                  else
                    voutln2("(MOVERIGHT R %d)", npcc->xoffset - xoff);

                  xoff = npcc->xoffset;
                }

                if (npcc->yoffset != yoff)
                {
                  if (fnt->uppercase[i])
                  {
                    voutln2("(MOVEUP R %.1f)",
                            fnt->capheight * (npcc->yoffset - yoff));
                  }
                  else
                    voutln2("(MOVEUP R %d)", npcc->yoffset - yoff);

                  yoff = npcc->yoffset;
                }

                voutln2("(SETCHAR O %o)", (unsigned)api->incode);
                xoff += fnt->outencptrs[api->outcode]->width;
              }
        }
        else
          voutln2("(SETCHAR O %o)", (unsigned)ti->incode);
          vright(&level);
      }
      vright(&level);
    }

    if (level)
      oops("I forgot to match the parentheses.");
}


void
writeovp(Font *fnt)
{
  register int i;
  register ttfinfo *ti;
  long bc, ec;
  char header[256];
  float Slant;
  ttfinfo *ofm_array[65536];


  out = fnt->vplout;

  header[0] = '\0';
  strncat(header, "Created by `", 12);
  strncat(header, fnt->titlebuf, 255 - 12 - 1);
  strncat(header, "'", 1);

  voutln2("(VTITLE %s)", header);
  voutln("(COMMENT Please change VTITLE if you edit this file)");

  voutln("(OFMLEVEL H 1)");
  voutln("(FONTDIR TL)");
  voutln2("(FAMILY %s)", fnt->outname);
  voutln("(FACE F MRR)");
  voutln("(CODINGSCHEME UNSPECIFIED)");
  voutln("(DESIGNSIZE R 10.0)");
  voutln("(DESIGNUNITS R 1000)");
  voutln("(COMMENT DESIGNSIZE (1 em) IS IN POINTS)");
  voutln("(COMMENT OTHER DIMENSIONS ARE MULTIPLES OF DESIGNSIZE/1000)");

#if 0
  /* Let vptovf compute the checksum. */
  voutln2("(CHECKSUM O %lo)", cksum ^ 0xFFFFFFFF);
#endif

  vleft(&level);
  voutln("FONTDIMEN");

  Slant = fnt->slant - fnt->efactor * tan(fnt->italicangle * (PI / 180.0));

  if (Slant)
    voutln2("(SLANT R %f)", Slant);
  voutln2("(SPACE D %d)", fnt->fontspace);
  if (!fnt->fixedpitch)
  {
    voutln2("(STRETCH D %d)", transform(200, 0, fnt->efactor, fnt->slant));
    voutln2("(SHRINK D %d)", transform(100, 0, fnt->efactor, fnt->slant));
  }
  voutln2("(XHEIGHT D %d)", fnt->xheight);
  voutln2("(QUAD D %d)", transform(1000, 0, fnt->efactor, fnt->slant));
  voutln2("(EXTRASPACE D %d)",
          fnt->fixedpitch ? fnt->fontspace :
                            transform(111, 0, fnt->efactor, fnt->slant));
  vright(&level);

  for (i = 0; i < fnt->subfont_num; i++)
  {
    vleft(&level);
    voutln2("MAPFONT D %d", i);
    voutln2("(FONTNAME %s)", (fnt->subfont_list[i]).name);
    voutln2("(FONTCHECKSUM O %lo)", (fnt->subfont_list[i]).cksum);
    vright(&level);
  }

  for (i = 0; i <= 0xFFFF; i++)
    ofm_array[i] = NULL;

  for (ti = fnt->charlist; ti != NULL; ti = ti->next)
    if (ti->charcode >= 0 && ti->charcode <= 0xFFFF)
      ofm_array[ti->charcode] = ti;

  for (i = 0; i <= 0xFFFF && ofm_array[i] == NULL; i++)
    ;
  bc = i;
  for (i = 0xFFFF; i >= 0 && ofm_array[i] == NULL; i--)
    ;
  ec = i;

  if (ec < bc)
    oops("No TTF characters.");

  for (i = bc; i <= ec; i++)
  {
    if ((ti = ofm_array[i]) == NULL) continue;

    vleft(&level);
    fprintf(out, "CHARACTER H %lX\n   ", ti->charcode);

    voutln2("(CHARWD R %d)", ti->width);
    if (ti->ury)
      voutln2("(CHARHT R %d)", ti->ury);
    if (ti->lly)
      voutln2("(CHARDP R %d)", -ti->lly);
    if (ti->urx > ti->width)
      voutln2("(CHARIC R %d)", ti->urx - ti->width);

    vleft(&level);
    voutln("MAP");
    if (ti->fntnum)
      voutln2("(SELECTFONT D %d)", ti->fntnum);

    voutln2("(SETCHAR H %X)", (unsigned)ti->incode);
    vright(&level);

    vright(&level);
  }

  if (level)
    oops("I forgot to match the parentheses.");
}


/* end */
