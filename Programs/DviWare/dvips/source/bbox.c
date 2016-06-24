/*
 *   Calculates the bounding box for a one-page .dvi file.
 *   Based on a lot of other dvips stuff.  Requires a total re-scan
 *   of the page.
 *
 *   If you request an EPSF file and specify -a, you may end up
 *   processing a single page four times!
 */
#include "dvips.h" /* The copyright notice in that file is included too! */
#include <math.h>
/*
 *   The external declarations:
 */
#include "protos_add.h"

typedef struct {
  integer llx, lly, urx, ury;
} bbchardesctype;
typedef struct {
  bbchardesctype *bbchardesc;
} bbfontdesctype;
typedef struct bbtft {
   integer fontnum;
   bbfontdesctype *desc;
   struct bbtft *next;
} bbfontmaptype;

static bbfontmaptype *bbffont;

static void
bbtfmload(register fontdesctype *curfnt)
{
   register integer i, j;
   register integer li, cd = 0;
   integer scaledsize;
   integer id, nt = 0;
   integer nw, nh, nd, ns, hd;
   integer bc, ec;
   integer nco = 0, ncw = 0, npc = 0, no_repeats = 0;
   halfword *index = NULL;
   halfword *chartype = NULL;
   integer *scaled;
   integer *chardat;
   int font_level;
   integer pretend_no_chars;
   integer slant = 0;
   bbchardesctype *cc;
   register bbfontdesctype *bbcurfnt =
      (bbfontdesctype *)mymalloc(sizeof(bbfontdesctype));

   bbffont->desc = bbcurfnt;
   tfmopen(curfnt);
/*
 *   Next, we read the font data from the tfm file, and store it in
 *   our own arrays.
 */
   curfnt->dir = 0;
   id = tfm16();
   if (id != 0) {
      font_level = -1;
      if (id == 9 || id == 11) {
         if (noptex) badtfm("length");
         if (id == 9) curfnt->dir = id;
         nt = tfm16(); li = tfm16();
      }
      hd = tfm16();
      bc = tfm16(); ec = tfm16();
      nw = tfm16(); nh = tfm16(); nd = tfm16();
      ns = tfm16();
      ns += tfm16();
      ns += tfm16();
      ns += tfm16();
      li = tfm16();
      if ((hd<2 || bc>ec+1 || ec>255 || nw>256 || nh>16 || nd>16) &&
           id != 9 && id != 11)
         badtfm("header");
   } else {  /* In an .ofm file */
      if (noomega) badtfm("length");
      font_level = tfm16();
      li = tfm32();
      hd = tfm32();
      bc = tfm32(); ec = tfm32();
      nw = tfm32(); nh = tfm32(); nd = tfm32();
      ns = tfm32();
      ns += tfm32();
      ns += tfm32();
      ns += tfm32();
      li = tfm32();
      li = tfm32();
      if (font_level>1 || hd<2 || bc<0 || ec<0 || nw<2 || nh<1 || nd<1 || ns<1
                       || bc>ec+1 || ec>65535 || nw>65536 || nh>256 || nd>256)
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

   li = tfm32();
   pretend_no_chars=ec+1;
   if (id == 9 || id == 11) {
      chardat = (integer *) xmalloc(256*sizeof(integer));
      bbcurfnt->bbchardesc = (bbchardesctype *)
                             xmalloc((MAX_2BYTES_CODE+1)*sizeof(bbchardesctype));
      for (i=2; i<hd; i++)
         li = tfm32();
      index = (halfword *)xmalloc(nt * sizeof(halfword));
      chartype = (halfword *)xmalloc(nt * sizeof(halfword));
      li = tfm16();
      li = tfm16();
      for (i=1; i<nt; i++) {
         li = tfm16();
         index[i] = li;
         chartype[i] = tfm16();
      }
      for (i=0; i<256; i++)
         chardat[i] = -1;
   } else {
      if (pretend_no_chars<256) pretend_no_chars=256;
      bbcurfnt->bbchardesc = (bbchardesctype *)
                             xmalloc(pretend_no_chars*sizeof(bbchardesctype));
      for (i=2; i<((font_level==1)?nco-29:hd); i++)
         li = tfm32();
      chardat = (integer *) xmalloc(pretend_no_chars*sizeof(integer));
      for (i=0; i<pretend_no_chars; i++)
         chardat[i] = -1;
   }
   for (i=bc; i<=ec; i++) {
      if (id == 9 || id == 11) {
         chardat[i] = tfm16();
         li = tfm16();
      } else {
         if (no_repeats>0) {
            no_repeats--;
         } else if (font_level>=0) {
            cd = tfm32();
            li = tfm32();
            if (font_level==1) {
               no_repeats = tfm16();
               for (j=0; j<(npc|1); j++) tfm16();
               ncw -= 3 + npc/2;
            }
         } else {
            cd = tfm16();
            li = tfm16();
         }
         if (cd) chardat[i] = cd;
      }
   }
   if (font_level==1&&ncw!=0) {
      char *msg = concat ("Table size mismatch in ", curfnt->name);
      error(msg);
      free(msg);
   }
   scaledsize = curfnt->scaledsize;
   if (id == 9 || id == 11) {
      scaled = (integer *) xmalloc(300*sizeof(integer));
      for (i=0; i<nw + nh + nd; i++)
         scaled[i] = scalewidth(tfm32(), scaledsize);
   } else {
      scaled = (integer *) xmalloc((nh + nd)*sizeof(integer));
      for (i=0; i<nw; i++)
         tfm32();
      for (i=0; i<nh + nd; i++)
         scaled[i] = scalewidth(tfm32(), scaledsize);
   }
   for (i=0; i<ns; i++)
      tfm32();
   slant = tfm32();
   fclose(tfmfile);

   if (id == 9 || id == 11) {
      for (i=0; i<MAX_2BYTES_CODE+1; i++) {
         cc = &(bbcurfnt->bbchardesc[i]);
         if (chardat[0] != -1) {
            cc->ury = scaled[((chardat[0] >> 4) & 15) + nw];
            cc->lly = - scaled[(chardat[0] & 15) + nw + nh];
            cc->llx = 0;
            cc->urx = curfnt->chardesc[0].TFMwidth;
         } else {
            cc->llx = cc->lly = cc->urx = cc->ury = 0;
         }
      }
      for (i=1; i<nt; i++) {
         cc = &(bbcurfnt->bbchardesc[index[i]]);
         if (chardat[chartype[i]] != -1) {
            cc->ury = scaled[((chardat[chartype[i]] >> 4) & 15) + nw];
            cc->lly = - scaled[(chardat[chartype[i]] & 15) + nw + nh];
            cc->llx = 0;
            cc->urx = curfnt->chardesc[index[i]].TFMwidth;
         } else {
            cc->llx = cc->lly = cc->urx = cc->ury = 0;
         }
      }
      free(index);
      free(chartype);
   } else {
      for (i=0; i<pretend_no_chars; i++) {
         cc = &(bbcurfnt->bbchardesc[i]);
         if (chardat[i] != -1) {
            halfword iw;
            int ih, id;
            if (font_level>=0) {
               iw = chardat[i] >> 16;
               ih = (chardat[i] >> 8) & 255;
               id = chardat[i] & 255;
            } else {
               iw = chardat[i] >> 8;
               ih = (chardat[i] >> 4) & 15;
               id = chardat[i] & 15;
            }
            if (iw>=nw || ih>=nh || id>=nd) badtfm("char info");
            cc->ury = scaled[ih];
            cc->lly = - scaled[nh + id];
            cc->llx = 0;
            cc->urx = curfnt->chardesc[i].TFMwidth;
         } else {
            cc->llx = cc->lly = cc->urx = cc->ury = 0;
         }
      }
   }
   if (slant) {
      if (id == 9 || id == 11) {
         for (i=0; i<MAX_2BYTES_CODE+1; i++) {
            cc = &(bbcurfnt->bbchardesc[i]);
            li = scalewidth(cc->lly, slant);
            if (slant > 0) cc->llx += li;
            else cc->urx += li;
            li = scalewidth(cc->ury, slant);
            if (slant > 0) cc->urx += li;
            else cc->llx += li;
         }
      } else {
         for (i=0; i<pretend_no_chars; i++) {
            cc = &(bbcurfnt->bbchardesc[i]);
            li = scalewidth(cc->lly, slant);
            if (slant > 0) cc->llx += li;
            else cc->urx += li;
            li = scalewidth(cc->ury, slant);
            if (slant > 0) cc->urx += li;
            else cc->llx += li;
         }
      }
   }
   free(chardat);
   free(scaled);
}
static integer llx, lly, urx, ury;
static void
bbspecial(integer h, integer v, integer dir, int nbytes)
{
   float *r = bbdospecial(nbytes);
   if (r) {
      /* convert from magnified PostScript units back to scaled points */
      real conv = 72.0 * (real)num / (real)den * (real)mag / 254000000.0;
      if (!dir) {
         if (llx > h + r[0] / conv)
            llx = (integer) (h + r[0] / conv);
         if (lly > v - r[3] / conv)
            lly = (integer) (v - r[3] / conv);
         if (urx < h + r[2] / conv)
            urx = (integer) (h + r[2] / conv);
         if (ury < v - r[1] / conv)
            ury = (integer) (v - r[1] / conv);
      } else {
         if (llx > h + r[1] / conv)
            llx = (integer)(h + r[1] / conv);
         if (lly > v + r[2] / conv)
            lly = (integer)(v + r[2] / conv);
         if (urx < h + r[3] / conv)
            urx = (integer)(h + r[3] / conv);
         if (ury < v + r[0] / conv)
            ury = (integer)(v + r[0] / conv);
      }
   }
}
static void
bbdopage(void)
{
   register shalfword cmd;
   register bbchardesctype *bcd;
   register chardesctype *cd;
   register integer h;
   register fontmaptype *cfnt;
   bbfontmaptype *bbcfnt;
   integer fnt;
   integer mychar;
   int charmove;
   struct dvistack *sp = stack;
   integer v, w, x, y, z, dir;
   register fontdesctype *curfnt = 0;
   register bbfontdesctype *bbcurfnt = 0;

   w = x = y = z = dir = 0;
   h = 0; v = 0;
   llx = lly = 1000000000;
   urx = ury = -1000000000;
   charmove = 0;
   while (1) {
      switch (cmd=dvibyte()) {
case 138: break;
case 135: /* put3 */
         if (noptex) error("! synch");
         mychar = dvibyte();
         mychar = (mychar << 8) + dvibyte();
         mychar = (mychar << 8) + dvibyte();
         charmove = 0;
         goto dochar;
case 130: /* set3 */
         if (noptex) error("! synch");
         mychar = dvibyte();
         mychar = (mychar << 8) + dvibyte();
         mychar = (mychar << 8) + dvibyte();
         charmove = 1;
         goto dochar;
case 134: /* put2 */
         if (noomega && noptex) error("! synch");
         mychar = dvibyte();
         mychar = (mychar << 8) + dvibyte();
         charmove = 0;
         goto dochar;
case 129: /* set2 */
         if (noomega && noptex) error("! synch");
         mychar = dvibyte();
         mychar = (mychar << 8) + dvibyte();
         charmove = 1;
         goto dochar;
case 133: /* put1 */
         mychar = dvibyte();
         charmove = 0;
         goto dochar;
case 128: cmd = dvibyte(); /* set1 command drops through to setchar */
default: /* these are commands 0 (setchar0) thru 127 (setchar127) */
         mychar = cmd;
         charmove = 1;
dochar:
         cd = &(curfnt->chardesc[mychar]);
         bcd = &(bbcurfnt->bbchardesc[CD_IDX(mychar)]);
         if (!dir) {
            if (h + bcd->llx < llx) llx = h + bcd->llx;
            if (h + bcd->urx > urx) urx = h + bcd->urx;
            if (v - bcd->ury < lly) lly = v - bcd->ury;
            if (v - bcd->lly > ury) ury = v - bcd->lly;
         } else {
            if (h + bcd->ury > urx) urx = h + bcd->ury;
            if (h + bcd->lly < llx) llx = h + bcd->lly;
            if (v + bcd->llx < lly) lly = v + bcd->llx;
            if (v + bcd->urx > ury) ury = v + bcd->urx;
         }
         if (charmove) {
            if (!dir)
                h += cd->TFMwidth;
            else
                v += cd->TFMwidth;
         }
         break;
case 255: /* pTeX's dir or undefined */
         if (!noptex) {
            dir = dvibyte();
            break;
         }
case 131: case 136: case 139: /* set4, put4, bop */
case 247: case 248: case 249: /* pre, post, post_post */
case 250: case 251: case 252: case 253: case 254: /* undefined */
         error("! synch");
case 132: case 137: /* rules */
         {  integer ry, rx;
            ry = signedquad(); rx = signedquad();
            if (rx>0 && ry>0) {
               if (!dir) {
                  if (h < llx) llx = h;
                  if (v - ry < lly) lly = v - ry;
                  if (h + rx > urx) urx = h + rx;
                  if (v > ury) ury = v;
               } else {
                  if (h < llx) llx = h;
                  if (v + rx < lly) lly = v + rx;
                  if (h + ry > urx) urx = h + ry;
                  if (v > ury) ury = v;
               }
            } else
               rx = 0;
            if (cmd != 137) {
               if (!dir)
                  h += rx;
               else
                  v += rx;
            }
            break;
         }
case 141: /* push */
         sp->h = h; sp->v = v;
         sp->w = w; sp->x = x; sp->y = y; sp->z = z; sp->dir = dir;
         if (++sp >= &stack[STACKSIZE]) error("! Out of stack space");
         break;
case 140: /* eop or end of virtual character */
         return;
case 142: /* pop */
         if (--sp < stack) error("! More pops than pushes");
         h = sp->h; v = sp->v;
         w = sp->w; x = sp->x; y = sp->y; z = sp->z; dir = sp->dir;
         break;
case 143: if (!dir) h += signedbyte(); else v += signedbyte(); break;
case 144: if (!dir) h += signedpair(); else v += signedpair(); break;
case 145: if (!dir) h += signedtrio(); else v += signedtrio(); break;
case 146: if (!dir) h += signedquad(); else v += signedquad(); break;
case 147: if (!dir) h += w; else v += w; break;
case 148: if (!dir) h += (w = signedbyte()); else v += (w = signedbyte()); break;
case 149: if (!dir) h += (w = signedpair()); else v += (w = signedpair()); break;
case 150: if (!dir) h += (w = signedtrio()); else v += (w = signedtrio()); break;
case 151: if (!dir) h += (w = signedquad()); else v += (w = signedquad()); break;
case 152: if (!dir) h += x; else v += x; break;
case 153: if (!dir) h += (x = signedbyte()); else v += (x = signedbyte()); break;
case 154: if (!dir) h += (x = signedpair()); else v += (x = signedpair()); break;
case 155: if (!dir) h += (x = signedtrio()); else v += (x = signedtrio()); break;
case 156: if (!dir) h += (x = signedquad()); else v += (x = signedquad()); break;
case 157: if (!dir) v += signedbyte(); else h -= signedbyte(); break;
case 158: if (!dir) v += signedpair(); else h -= signedpair(); break;
case 159: if (!dir) v += signedtrio(); else h -= signedtrio(); break;
case 160: if (!dir) v += signedquad(); else h -= signedquad(); break;
case 161: if (!dir) v += y; else h -= y; break;
case 162: if (!dir) v += (y = signedbyte()); else h -= (y = signedbyte()); break;
case 163: if (!dir) v += (y = signedpair()); else h -= (y = signedpair()); break;
case 164: if (!dir) v += (y = signedtrio()); else h -= (y = signedtrio()); break;
case 165: if (!dir) v += (y = signedquad()); else h -= (y = signedquad()); break;
case 166: if (!dir) v += z; else h += z; break;
case 167: if (!dir) v += (z = signedbyte()); else h -= (z = signedbyte()); break;
case 168: if (!dir) v += (z = signedpair()); else h -= (z = signedpair()); break;
case 169: if (!dir) v += (z = signedtrio()); else h -= (z = signedtrio()); break;
case 170: if (!dir) v += (z = signedquad()); else h -= (z = signedquad()); break;
case 171: case 172: case 173: case 174: case 175: case 176: case 177:
case 178: case 179: case 180: case 181: case 182: case 183: case 184:
case 185: case 186: case 187: case 188: case 189: case 190: case 191:
case 192: case 193: case 194: case 195: case 196: case 197: case 198:
case 199: case 200: case 201: case 202: case 203: case 204: case 205:
case 206: case 207: case 208: case 209: case 210: case 211: case 212:
case 213: case 214: case 215: case 216: case 217: case 218: case 219:
case 220: case 221: case 222: case 223: case 224: case 225: case 226:
case 227: case 228: case 229: case 230: case 231: case 232: case 233:
case 234: case 235: case 236: case 237: case 238: /* font selection commands */
         if (cmd < 235) fnt = cmd - 171; /* fntnum0 thru fntnum63 */
         else {
            fnt = dvibyte();
            while (cmd-- > 235)
               fnt = (fnt << 8) + dvibyte();
         }
         for (cfnt=ffont; cfnt; cfnt = cfnt->next)
            if (cfnt->fontnum == fnt) break;
         curfnt = cfnt->desc;
         for (bbcfnt=bbffont; bbcfnt; bbcfnt = bbcfnt->next)
            if (bbcfnt->fontnum == fnt) goto fontfound;
         bbcfnt = (bbfontmaptype *)mymalloc(sizeof(bbfontmaptype));
         bbcfnt->fontnum = fnt;
         bbcfnt->next = bbffont;
         bbffont = bbcfnt;
         bbtfmload(curfnt);
fontfound: bbcurfnt = bbcfnt->desc;
         break;
case 243: case 244: case 245: case 246: /*fntdef1 */
         skipover(cmd - 230);
         skipover(dvibyte() + dvibyte());
         break;
case 239: bbspecial(h, v, dir, (int)dvibyte()); break;
case 240: bbspecial(h, v, dir, (int)twobytes()); break;
case 241: bbspecial(h, v, dir, (int)threebytes()); break;
case 242: bbspecial(h, v, dir, (int)signedquad()); break;
      }
   }
}
void
findbb(integer bop)
{
   integer curpos = ftell(dvifile);
   real conv = 72.0 * (real)num / (real)den * (real)mag / 254000000.0;
   real off = 72.0 / conv;
   real margin = 1.0 / conv;
   real vsize = 792.0 / conv;
   real hadj = -72.0 * hoff / 4736286.72;
   real vadj = 72.0 * voff / 4736286.72;

   fseek(dvifile, bop, 0);
   bbdopage();
   fseek(dvifile, curpos, 0);
   lly = (int) (vsize - 2 * off - lly);
   ury = (int) (vsize - 2 * off - ury);
   llx = (int)floor((llx + off - margin) * conv - hadj + 0.5);
   lly = (int)floor((lly + off + margin) * conv - vadj + 0.5);
   urx = (int)floor((urx + off + margin) * conv - hadj + 0.5);
   ury = (int)floor((ury + off - margin) * conv - vadj + 0.5);
/* no marks on the page? */
   if (llx >= urx || lly <= ury)
      llx = lly = urx = ury = 72;
#ifdef SHORTINT
   sprintf(nextstring, "%ld %ld %ld %ld", llx, ury, urx, lly);
#else
   sprintf(nextstring, "%d %d %d %d", llx, ury, urx, lly);
#endif
}
