/*
 *   Loads a tfm file.  It marks the characters as undefined.
 */
#include "dvips.h" /* The copyright notice in that file is included too! */
#ifdef KPATHSEA
#include <kpathsea/c-pathmx.h>
#include <kpathsea/concatn.h>
#endif
/*
 *   The external declarations:
 */
#include "protos.h"

FILE *tfmfile;

/*
 *   Our static variables:
 */
static char name[500];

void
badtfm(const char *s)
{
   char *msg = concatn ("! Bad TFM file ", name, ": ", s, NULL);
   error(msg);
}

/*
 *   Tries to open a tfm file in FD.  Uses cmr10.tfm if unsuccessful,
 *   and complains loudly about it.  No return, sets tfmfile global.
 */
void
tfmopen(register fontdesctype *fd)
{
   char *full_name;
   kpse_file_format_type d;
   char *stem_name = concat(fd->area, fd->name);
   
   if (!noomega) { /* search for .ofm first */
      full_name = concat(stem_name, ".ofm");
      d = ofmpath;
      if ((tfmfile = search(d, full_name, READBIN)) != NULL) {
         free(stem_name);
         free(full_name);
         return;
      }
      free(full_name);
   }

   /* try tfm */
   d = tfmpath;
   full_name = concat(stem_name, ".tfm");
   if ((tfmfile = search(d, full_name, READBIN)) != NULL) {
      free(stem_name);
      free(full_name);
      return;
   }
   sprintf(errbuf, "Can't open font metric file %.999s", full_name);
   error(errbuf);
   
   error("I will use cmr10.tfm instead, so expect bad output.");
   if ((tfmfile=search(d, "cmr10.tfm", READBIN)) != NULL) {
      free(stem_name);
      free(full_name);
      return;
   }

   error("! I can't find cmr10.tfm; please reinstall me with proper paths");
   free(stem_name);
}

static shalfword
tfmbyte(void)
{
   return(getc(tfmfile));
}

halfword
tfm16(void)
{
   register halfword a;
   a = tfmbyte ();
   return ( a * 256 + tfmbyte () );
}

uinteger
tfm24_kanji(void)
{
   register uinteger a;
   a = tfmbyte ();
   a = a * 256 + tfmbyte ();
   return ( a + tfmbyte () * 65536 );
}

integer
tfm32(void)
{
   register integer a;
   a = tfm16 ();
   if (a > 32767) a -= 65536;
   return ( a * 65536 + tfm16 () );
}

int
tfmload(register fontdesctype *curfnt)
{
   register integer i, j;
   register integer li, cd=0;
   integer scaledsize;
   integer id, nt = 0;
   integer nw, hd;
   integer bc, ec;
   integer nco=0, ncw=0, npc=0, no_repeats = 0;
   uinteger *index = NULL;
   halfword *chartype = NULL;
   integer *scaled;
   integer *chardat;
   int font_level;
   integer pretend_no_chars;
   int charcount = 0;

   tfmopen(curfnt);
/*
 *   Next, we read the font data from the tfm file, and store it in
 *   our own arrays.
 */
   curfnt->dir = 0;
   curfnt->iswide = 0;
   curfnt->codewidth = 1;
   id = tfm16();
   if (id!=0) {
      font_level = -1;
      if (id == 9 || id == 11) {
         if (noptex) badtfm("length");
         if (id == 9) curfnt->dir = id;
         nt = tfm16(); li = tfm16();
         curfnt->iswide = 1;
         curfnt->maxchars = MAX_CODE;
         curfnt->chardesc = (chardesctype *)realloc(curfnt->chardesc,
                            sizeof(chardesctype)*(MAX_CODE));
         for (i=0; i<MAX_CODE; i++) {
            curfnt->chardesc[i].TFMwidth = 0;
            curfnt->chardesc[i].packptr = NULL;
            curfnt->chardesc[i].pixelwidth = 0;
            curfnt->chardesc[i].flags = 0;
         }
      }
      hd = tfm16();
      bc = tfm16(); ec = tfm16();
      nw = tfm16();
      li = tfm32(); li = tfm32(); li = tfm32(); li = tfm16();
      if ((hd<2 || bc>ec+1 || ec>255 || nw>256) && id != 9 && id != 11)
         badtfm("header");
   } else {  /* In an .ofm file */
      if (noomega) badtfm("length");
      font_level = tfm16();
      li = tfm32(); hd = tfm32();
      bc = tfm32(); ec = tfm32();
      nw = tfm32();
      for (i=0; i<8; i++) li=tfm32();
      if (!noptex && font_level==1 && ec>=0x2E00) {
         curfnt->iswide = 1;
         if (li==5) {  /* interpret FONTDIR RT as pTeX vertical writing */
            curfnt->dir = 9;
#ifdef DEBUG
            if (dd(D_FONTS))
               fprintf_str(stderr,
                  "We will interpret font (%s.ofm) direction as pTeX vertical writing.\n",
                  curfnt->name);
#endif /* DEBUG */
         }
      }
      if (font_level>1 || hd<2 || bc<0 || ec<0 || nw<0
                       || bc>ec+1 || ec>65535 || nw>65536)
         badtfm("header");
      if (font_level==1) {
         nco = tfm32();
         ncw = tfm32();
         npc = tfm32();
         for (i=0; i<12; i++) li=tfm32();
      }
   }
   li = tfm32();
   check_checksum (li, curfnt->checksum, curfnt->name);
   li = (integer)(alpha * (real)tfm32());
   if (li > curfnt->designsize + fsizetol ||
       li < curfnt->designsize - fsizetol) {
      char *msg = concat ("Design size mismatch in font ", curfnt->name);
      error(msg);
   }
   pretend_no_chars=ec+1;
   if (id == 9 || id == 11) {
      chardat = (integer *)xmalloc(256*sizeof(integer));
      for (i=2; i<hd; i++)
         li = tfm32();
      index = (uinteger *)malloc(nt * sizeof(uinteger));
      chartype = (halfword *)malloc(nt * sizeof(halfword));
      li = tfm16();
      li = tfm16();
      for (i=1; i<nt; i++) {
         /* support new JFM spec by texjporg */
         index[i] = tfm24_kanji();
         chartype[i] = tfmbyte();
      }
      for (i=0; i<256; i++)
         chardat[i] = 256;

      for (i=bc; i<=ec; i++) {
         chardat[i] = tfmbyte();
         li = tfm16();
         li |= tfmbyte();
         if (li || chardat[i])
            charcount++;
      }
   } else {
      if (pretend_no_chars<=256) pretend_no_chars=256;
      else {
         curfnt->chardesc = (chardesctype *)
            xrealloc(curfnt->chardesc, sizeof(chardesctype)*pretend_no_chars);
         curfnt->maxchars = pretend_no_chars;
      }
      for (i=2; i<((font_level==1)?nco-29:hd); i++)
         li = tfm32();
      chardat = (integer *) xmalloc(pretend_no_chars*sizeof(integer));
      for (i=0; i<pretend_no_chars; i++)
         chardat[i] = -1;
      for (i=bc; i<=ec; i++) {
         if (no_repeats>0) {
            no_repeats--;
         } else if (font_level>=0) {
            cd = tfm16();
            li = tfm32();
            li = tfm16();
            if (font_level==1) {
               no_repeats = tfm16();
               for (j=0; j<(npc|1); j++) tfm16();
               ncw -= 3 + npc/2;
            }
         } else {
            cd = tfmbyte();
            li = tfm16();
            li = tfmbyte();
         }
         if (cd>=nw) badtfm("char info");
         if (cd) {
            chardat[i] = cd;
            charcount++;
         }
      }
      if (font_level==1&&ncw!=0) {
         char *msg = concat ("Table size mismatch in ", curfnt->name);
         error(msg);
      }
   }
   scaledsize = curfnt->scaledsize;
   scaled = (integer *) xmalloc(nw*sizeof(integer));
   for (i=0; i<nw; i++)
      scaled[i] = scalewidth(tfm32(), scaledsize);
   fclose(tfmfile);

   if (id == 9 || id == 11) {
      for (i=0; i<MAX_CODE; i++) {
         li = scaled[chardat[0]];
         curfnt->chardesc[i].TFMwidth = li;
         if (li >= 0)
            curfnt->chardesc[i].pixelwidth = ((integer)(conv*li+0.5));
         else
            curfnt->chardesc[i].pixelwidth = -((integer)(conv*-li+0.5));
         curfnt->chardesc[i].flags = (curfnt->resfont ? EXISTS : 0);
         curfnt->chardesc[i].flags2 = EXISTS;
      }
      for (i=1; i<nt; i++) {
         li = scaled[chardat[chartype[i]]];
         curfnt->chardesc[index[i]].TFMwidth = li;
         if (li >= 0)
            curfnt->chardesc[index[i]].pixelwidth = ((integer)(conv*li+0.5));
         else
            curfnt->chardesc[index[i]].pixelwidth = -((integer)(conv*-li+0.5));
      }
      free(index);
      free(chartype);
   } else {
      for (i=0; i<pretend_no_chars; i++)
         if (chardat[i]!= -1) {
            li = scaled[chardat[i]];
            curfnt->chardesc[i].TFMwidth = li;
            if (li >= 0)
               curfnt->chardesc[i].pixelwidth = ((integer)(conv*li+0.5));
            else
               curfnt->chardesc[i].pixelwidth = -((integer)(conv*-li+0.5));
            curfnt->chardesc[i].flags = (curfnt->resfont ? EXISTS : 0);
            curfnt->chardesc[i].flags2 = EXISTS;
         }
      if (ec>=256) curfnt->codewidth = 2; /* XXX: 2byte-code can have ec<256 */
   }
   free(chardat);
   free(scaled);
   curfnt->loaded = 1;
   return charcount;
}
