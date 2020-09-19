/*
 *   Here's the code to load a PK file into memory.
 *   Individual bitmaps won't be unpacked until they prove to be needed.
 */
#include "dvips.h" /* The copyright notice in that file is included too! */
#ifdef OS2
#include <stdlib.h>
#endif
#ifdef KPATHSEA
#include <kpathsea/c-pathmx.h>
#include <kpathsea/concatn.h>
#include <kpathsea/tex-glyph.h>
#include <kpathsea/tex-make.h>
#include <kpathsea/lib.h>
#ifndef MAXPATHLEN
#define MAXPATHLEN (256)
#endif
#else
#if defined(SYSV) || defined(VMS) || (defined(MSDOS) && !defined(__DJGPP__)) || defined(OS2) || defined(ATARIST)
#define MAXPATHLEN (256)
#else
#include <sys/param.h>          /* for MAXPATHLEN */
#endif
#endif
/*
 *   The external declarations:
 */
#include "protos_add.h"

char errbuf[1500];
int lastresortsizes[40];
int bitmapfontseen = 0 ;
/*
 *   Now we have some routines to get stuff from the PK file.
 *   Subroutine pkbyte returns the next byte.
 */
FILE *pkfile;
char name[MAXPATHLEN];
void
badpk(const char *s)
{
   char *msg = concatn ("! Bad PK file ", name, ": ", s, NULL);
   error(msg);
}

shalfword
pkbyte(void)
{
   register shalfword i;

   if ((i=getc(pkfile))==EOF)
      badpk("unexpected eof");
   return(i);
}

integer
pkquad(void)
{
   register integer i;

   i = pkbyte();
   if (i > 127)
      i -= 256;
   i = i * 256 + pkbyte();
   i = i * 256 + pkbyte();
   i = i * 256 + pkbyte();
   return(i);
}

integer
pktrio(void)
{
   register integer i;

   i = pkbyte();
   i = i * 256 + pkbyte();
   i = i * 256 + pkbyte();
   return(i);
}


/*
 *   pkopen opens the pk file.  This is system dependent.  We work really
 *   hard to open some sort of PK file.
 */
#ifdef VMCMS /* IBM: VM/CMS - we let DVIPS EXEC handle this after
                             the DVIPS MODULE has finished to avoid
                             complications with system calls. */
int dontmakefont = 0;
#else
#ifdef MVSXA /* IBM: MVS/XA - we let system administrator handle this on
                            MVS/XA since some printers can't get to user
                             fonts anyway */
int dontmakefont = 1;
#else
int dontmakefont = 0; /* if makefont fails once we won't try again */
#endif  /* IBM: VM/CMS */
#endif

static void
lectureuser(void) {
   static int userwarned = 0;

   if (! userwarned) {
      error("Such scaling will generate extremely poor output.");
      userwarned = 1;
   }
}
static Boolean
pkopen(register fontdesctype *fd)
{
   register char *d, *n;
   const char *name_ret;
#ifdef KPATHSEA
   int dpi_ret;
#else
   int vdpi;
#endif
   d = fd->area;
   n = fd->name;
#ifndef KPATHSEA
   if (*d==0)
      d = pkpath;
#endif
#ifdef FONTLIB
   if (*(fd->area) == 0) {
      int del;
      for (del=0; del<=RES_TOLERANCE(fd->dpi); del=del>0?-del:-del+1) {
        if ((pkfile=flisearch(n, fd->dpi + del)) != (FILE *)NULL )
          return(1);
      }
   }
#endif
   {
#ifdef KPATHSEA
     char *this_name = concat (d, n);

     pkfile = pksearch(this_name, READBIN, fd->dpi, &name_ret, &dpi_ret);

     if (!pkfile || !FILESTRCASEEQ (this_name, name_ret)) {
       char *msg = concatn ("Font ", fd->area, n, " not found; ", NULL);
       /* wasting some memory */
       if (!pkfile)
         msg = concat (msg, "characters will be left blank.");
       else
         msg = concat3 (msg, "using ", name_ret);
       dontmakefont = 1;
       error (msg);

     } else if (!kpse_bitmap_tolerance ((double) dpi_ret, (double) fd->dpi))
       {
           fd->loadeddpi = dpi_ret;
           fd->alreadyscaled = 0;
           sprintf(errbuf,
                   "Font %s at %d not found; scaling %d instead.",
                                     n, fd->dpi, dpi_ret);
           error(errbuf);
           lectureuser();
       }

     if (this_name != name_ret)
       free (this_name);

     return pkfile != NULL;
   }
#else
      int del;
      for (del=0; del<=RES_TOLERANCE(fd->dpi); del=del>0?-del:-del+1) {
         if (actualdpi == vactualdpi) {
            vdpi = 0;
         } else {
            vdpi = (2 * ((long)vactualdpi) * (fd->dpi + del) + actualdpi)
                                                       / (2 * actualdpi);
         }
#ifdef MVSXA
         sprintf(name, "pk%d(%s)", fd->dpi + del, n);
#else
         sprintf(name, "%s.%dpk", n, fd->dpi + del);
#endif
         if (0 != (pkfile=pksearch(d, name, READBIN, n, fd->dpi + del, vdpi)))
            return(1);
      }
   }
   if (d == pkpath) {
      if (actualdpi == vactualdpi) {
         vdpi = 0;
      } else {
         vdpi = (2 * ((long)vactualdpi) * fd->dpi + actualdpi)
                                                    / (2 * actualdpi);
      }
#ifdef MVSXA
      sprintf(name, "pk%d(%s)", fd->dpi, n);
#else
      sprintf(name, "%s.%dpk", n, fd->dpi);
#endif
      makefont(n, (int)fd->dpi, DPI);
      if (dontmakefont == 0 &&
          (pkfile = pksearch(d, name, READBIN, n, fd->dpi, vdpi)))
               return(1);
#ifndef MSDOS
#ifdef OS2
   if (_osmode == OS2_MODE)
      dontmakefont = 1;       /* failed so don't try again under OS/2,
                                 but do try for MSDOS */
#else
#ifndef ATARIST
      dontmakefont = 1;
#endif
#endif
#endif
   }
/*
 *   If nothing above worked, then we get desparate.  We attempt to
 *   open the stupid font at one of a small set of predefined sizes,
 *   and then use PostScript scaling to generate the correct size.
 *
 *   We much prefer scaling up to scaling down, since scaling down
 *   can omit character features, so we try the larger sizes first,
 *   and then work down.
 */
   {
      int i, j;

      if (lastresortsizes[0] && fd->dpi < 30000) {
         for (i=0; lastresortsizes[i] < fd->dpi; i++);
         for (j = i-1; j >= 0; j--) {
            if (actualdpi == vactualdpi) {
               vdpi = 0;
            } else {
               vdpi = (2 * ((long)vactualdpi) * lastresortsizes[j] + actualdpi)
                                                       / (2 * actualdpi);
            }
#ifdef MVSXA
            sprintf(name, "pk%d(%s)", lastresortsizes[j], n);
#else
            sprintf(name, "%s.%dpk", n, lastresortsizes[j]);
#endif
#ifdef FONTLIB
            if (0 != (pkfile=flisearch(n,(halfword)lastresortsizes[j]))
             || 0 != (pkfile=pksearch(d, name, READBIN, n,
                         (halfword)lastresortsizes[j], vdpi))) {
#else
            if (0 != (pkfile=pksearch(d, name, READBIN, n,
                         (halfword)lastresortsizes[j], vdpi))) {
#endif
               fd->loadeddpi = lastresortsizes[j];
               fd->alreadyscaled = 0;
               sprintf(errbuf,
                       "Font %s at %d dpi not found; scaling %d instead.",
                                         n, fd->dpi, lastresortsizes[j]);
               error(errbuf);
               lectureuser();
               return 1;
            }
         }
         for (j = i; lastresortsizes[j] < 30000; j++) {
            if (actualdpi == vactualdpi) {
               vdpi = 0;
            } else {
               vdpi = (2 * ((long)vactualdpi) * lastresortsizes[j] + actualdpi)
                                                       / (2 * actualdpi);
            }
            sprintf(name, "%s.%dpk", n, lastresortsizes[j]);
#ifdef FONTLIB
            if (0 != (pkfile=flisearch(n, (halfword)lastresortsizes[j]))
                || 0 != (pkfile=pksearch(d, name, READBIN, n,
                             (halfword)lastresortsizes[j], vdpi))) {
#else
            if (0 != (pkfile=pksearch(d, name, READBIN, n,
                            (halfword)lastresortsizes[j], vdpi))) {
#endif
               fd->loadeddpi = lastresortsizes[j];
               fd->alreadyscaled = 0;
               sprintf(errbuf,
                       "Font %s at %d dpi not found; scaling %d instead.",
                                         name, fd->dpi, lastresortsizes[j]);
               error(errbuf);
               lectureuser();
               return 1;
            }
         }
      }
   }
#ifdef MVSXA
   sprintf(name, "%s.pk%d", n, fd->dpi);
#else
   sprintf(name, "%s.%dpk", n, fd->dpi);
#endif
   sprintf(errbuf,
      "Font %s%s not found, characters will be left blank.",
      fd->area, name);
   error(errbuf);
   return(0);
#endif /* KPATHSEA */
}

/*
 *   Now our loadfont routine.  We return an integer indicating the
 *   highest character code in the font, so we know how much space
 *   to reserve for the character.  (It's returned in the font
 *   structure, along with everything else.)
 */
void
loadfont(register fontdesctype *curfnt)
{
   register integer i;
   register shalfword cmd;
   register integer k;
   register integer length = 0;
   register shalfword cc = 0;
   register integer scaledsize = curfnt->scaledsize;
   register quarterword *tempr;
   register chardesctype *cd = 0;
   int maxcc = 0;
   int munged = 0;
/*
 *   We clear out some pointers:
 */
   if (curfnt->loaded == 3) {
      for (i=0; i<256; i++) {
         curfnt->chardesc[i].TFMwidth = 0;
         curfnt->chardesc[i].packptr = NULL;
         curfnt->chardesc[i].pixelwidth = 0;
         curfnt->chardesc[i].flags &= EXISTS;
         curfnt->chardesc[i].flags2 = 0;
      }
   } else {
      for (i=0; i<256; i++) {
         curfnt->chardesc[i].TFMwidth = 0;
         curfnt->chardesc[i].packptr = NULL;
         curfnt->chardesc[i].pixelwidth = 0;
         curfnt->chardesc[i].flags = 0;
         curfnt->chardesc[i].flags2 = 0;
      }
   }
   curfnt->maxchars = 256; /* just in case we return before the end */
   curfnt->llx = 0 ;
   curfnt->lly = 0 ;
   curfnt->urx = 0 ;
   curfnt->ury = 0 ;
   if (!pkopen(curfnt)) {
      tfmload(curfnt);
      return;
   }
   bitmapfontseen = 1 ;
   curfnt->dir = 0;
   if (!quiet) {
      if (strlen(realnameoffile) + prettycolumn > STDOUTSIZE) {
         fprintf(stderr, "\n");
         prettycolumn = 0;
      }
      fprintf(stderr, "<%s>", realnameoffile);
      prettycolumn += strlen(realnameoffile) + 2;
   }
#ifdef DEBUG
   if (dd(D_FONTS))
      fprintf(stderr,"Loading pk font %s at %.1fpt\n",
         curfnt->name, (real)scaledsize/(alpha*0x100000));
#endif /* DEBUG */
   if (pkbyte()!=247)
      badpk("expected pre");
   if (pkbyte()!=89)
      badpk("wrong id byte");
   for(i=pkbyte(); i>0; i--)
      pkbyte();
   k = (integer)(alpha * (real)pkquad());
   if (k > curfnt->designsize + fsizetol
       || k < curfnt->designsize - fsizetol) {
      char *msg = concat ("Design size mismatch in font ", curfnt->name);
      error (msg);
      free (msg);
   }
   k = pkquad();
   if (k && curfnt->checksum)
      if (k!=curfnt->checksum) {
         char *msg = concat ("Checksum mismatch in font ", curfnt->name);
         error (msg);
         free (msg);
       }
   k = pkquad(); /* assume that hppp is correct in the PK file */
   k = pkquad(); /* assume that vppp is correct in the PK file */
/*
 *   Now we get down to the serious business of reading character definitions.
 */
   while ((cmd=pkbyte())!=245) {
      if (cmd < 240) {
         switch (cmd & 7) {
case 0: case 1: case 2: case 3:
            length = (cmd & 7) * 256 + pkbyte() - 3;
            cc = pkbyte();
            cd = curfnt->chardesc+cc;
            if (nosmallchars || curfnt->dpi != curfnt->loadeddpi)
               cd->flags |= BIGCHAR;
            cd->TFMwidth = scalewidth(pktrio(), scaledsize);
            cd->pixelwidth = pkbyte();
            break;
case 4: case 5: case 6:
            length = (cmd & 3) * 65536L + pkbyte() * 256L;
            length = length + pkbyte() - 4L;
            cc = pkbyte();
            cd = curfnt->chardesc+cc;
            cd->TFMwidth = scalewidth(pktrio(), scaledsize);
            cd->flags |= BIGCHAR;
            i = pkbyte();
            cd->pixelwidth = i * 256 + pkbyte();
            break;
case 7:
            length = pkquad() - 11;
            cc = pkquad();
            if (cc<0 || cc>255) badpk("character code out of range");
            cd = curfnt->chardesc + cc;
            cd->flags |= BIGCHAR;
            cd->TFMwidth = scalewidth(pkquad(), scaledsize);
            cd->pixelwidth = (pkquad() + 32768) >> 16;
            k = pkquad();
         }
         if (cd->pixelwidth == 0 && cd->TFMwidth != 0) {
            if (cd->TFMwidth > 0)
               k = (integer)(cd->TFMwidth * conv + 0.5);
            else
               k = -(integer)(-cd->TFMwidth * conv + 0.5);
            if (k != 0) {
               cd->pixelwidth = k;
               munged++;
            }
         }
         if (length <= 0)
            badpk("packet length too small");
         if (dopprescan && ((cd->flags & EXISTS) == 0)) {
            for (length--; length>0; length--)
               pkbyte();
         } else {
            if (cc > maxcc)
               maxcc = cc;
            if (bytesleft < length || (length > MINCHUNK && compressed)) {
#ifdef DEBUG
                if (dd(D_MEM))
                   fprintf(stderr,
#ifdef SHORTINT
                      "Allocating new raster memory (%ld req, %ld left)\n",
                                   length, bytesleft);
#else
                      "Allocating new raster memory (%d req, %d left)\n",
                                   (int)length, (int)bytesleft);
#endif
#endif /* DEBUG */
                if (length > MINCHUNK) {
                   tempr = (quarterword *)mymalloc((integer)length);
                } else {
                   raster = (quarterword *)mymalloc((integer)RASTERCHUNK);
                   tempr = raster;
                   bytesleft = RASTERCHUNK - length;
                   raster += length;
               }
            } else {
               tempr = raster;
               bytesleft -= length;
               raster += length;
            }
            cd->packptr = tempr;
            *tempr++ = cmd;
            for (length--; length>0; length--)
               *tempr++ = pkbyte();
            {
               // update the global font bounding box
               // this is only used to set font sizes for type 3 bitmap
               // encoding.
               integer cwidth, cheight, xoff, yoff ;
               unpack_bb(cd, &cwidth, &cheight, &xoff, &yoff) ;
               if (-xoff < curfnt->llx)
                  curfnt->llx = -xoff ;
               if (cwidth - xoff > curfnt->urx)
                  curfnt->urx = cwidth - xoff ;
               if (yoff - cheight < curfnt->lly)
                  curfnt->lly = yoff - cheight ;
               if (yoff > curfnt->ury)
                  curfnt->ury = yoff ;
            }
         }
         cd->flags2 |= EXISTS;
      } else {
         k = 0;
         switch (cmd) {
case 243:
            k = pkbyte();
            if (k > 127)
               k -= 256;
case 242:
            k = k * 256 + pkbyte();
case 241:
            k = k * 256 + pkbyte();
case 240:
            k = k * 256 + pkbyte();
            while (k-- > 0)
               i = pkbyte();
            break;
case 244:
            k = pkquad();
            break;
case 246:
            break;
default:
            badpk("! unexpected command");
         }
      }
   }
#ifdef FONTLIB
   if (flib)
      flib = 0;
   else
#endif
   fclose(pkfile);
   curfnt->loaded = 1;
   curfnt->maxchars = maxcc + 1;
   if (munged > 0) {
      static int seen = 0;
      sprintf(errbuf,
          "Font %s at %d dpi has most likely been made improperly;",
           curfnt->name, curfnt->dpi);
      error(errbuf);
      if (seen)
         return;
      seen = 1;
      sprintf(errbuf,
     "%d characters have 0 escapements but non-trivial TFM widths.", munged);
      error(errbuf);
      error(
          "I'm stumbling along as best I can, but I recommend regenerating");
      error(
          "these fonts; the problem is probably that they are non-CM fonts");
      error(
          "(such as circle10 or line10) created with a MF with the CM base");
      error("preloaded .");
   }
}
