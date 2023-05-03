/*
 *   This is the main routine for the first (prescanning) pass.
 */
#include "dvips.h" /* The copyright notice in that file is included too! */

/*
 *   The external declarations:
 */
#include "protos.h"

/*
 *  We declare this to tell everyone we are prescanning early.
 */
Boolean pprescan;
/*
 * When a font is selected during the pre-prescan, this routine makes sure
 * that the tfm or vf (but not pk) file is loaded.
 */
static void
ppreselectfont(fontdesctype *f)
{
   int i;

   curfnt = f;
   if (curfnt->loaded == 0) {
      if (!residentfont(curfnt))
         if (!virtualfont(curfnt)) {
            for (i=0; i<256; i++)
               curfnt->chardesc[i].flags = 0;
            curfnt->loaded = 3; /* we're scanning for sizes */
         }
   }
}
/*
 *   Now our scanpage routine.
 */
static void
pscanpage(void)
{
   register shalfword cmd;
   register chardesctype *cd;
   register fontmaptype *cfnt = 0;
   integer fnt;
   integer mychar;
   register frametype *frp = frames;

#ifdef DEBUG
   if (dd(D_PAGE))
#ifdef SHORTINT
   fprintf(stderr,"PPrescanning page %ld\n", pagenum);
#else   /* ~SHORTINT */
   fprintf(stderr,"PPrescanning page %d\n", pagenum);
#endif  /* ~SHORTINT */
#endif  /* DEBUG */
   curfnt = NULL;
   curpos = NULL;
   bopcolor(0);
   while (1) {
      switch (cmd=dvibyte()) {
case 255: /* pTeX's dir or undefined */
         if (!noptex) {
            dvibyte();
            break;
         }
/* illegal commands */
case 131: case 136: case 139: /* set4, put4, bop */
case 247: case 248: case 249: /* pre, post, post_post */
case 250: case 251: case 252: case 253: case 254: /* undefined */
         sprintf(errbuf,
            "! DVI file contains unexpected command (%d)",cmd);
         error(errbuf);
case 132: case 137: /* eight-byte commands setrule, putrule */
         dvibyte();
         dvibyte();
         dvibyte();
         dvibyte();
case 146: case 151: case 156: case 160: case 165: case 170:
   /* four-byte commands right4, w4, x4, down4, y4, z4 */
         dvibyte();
case 145: case 150: case 155: case 159: case 164: case 169:
   /* three-byte commands right3, w3, x3, down3, y3, z3 */
         dvibyte();
case 144: case 149: case 154: case 158: case 163: case 168:
   /* two-byte commands right2, w2, x2, down2, y2, z2 */
         dvibyte();
case 143: case 148: case 153: case 157: case 162: case 167:
   /* one-byte commands right1, w1, x1, down1, y1, z1 */
         dvibyte();
case 147: case 152: case 161: case 166: /* w0, x0, y0, z0 */
case 138: case 141: case 142: /* nop, push, pop */
         break;
case 130: case 135: /* set3, put3 */
         if (noptex) {
            sprintf(errbuf,
               "! DVI file contains unexpected pTeX command (%d)",cmd);
            error(errbuf);
         }
         mychar = dvibyte(); mychar = (mychar << 8) + dvibyte();
         mychar = (mychar << 8) + dvibyte();
         goto dochar;
case 129: case 134: /* set2, put2 */
         if (noomega && noptex) {
            sprintf(errbuf,
               "! DVI file contains unexpected command (%d)",cmd);
            error(errbuf);
         }
         mychar = dvibyte(); mychar = (mychar << 8) + dvibyte();
         goto dochar;
case 128: case 133: cmd = dvibyte(); /* set1, put1 commands drops through */
default:    /* these are commands 0 (setchar0) thru 127 (setchar 127) */
         mychar = cmd;
dochar:
         if (curfnt==NULL)
            error("! Bad DVI file: no font selected");
         if (mychar>=curfnt->maxchars) {
            if (!noptex && mychar<0x1000000 && curfnt->loaded == 2 && curfnt->kind == VF_PTEX) {
               /* fallback */
            } else {
               sprintf(errbuf,"! [pscanpage] invalid char %d from font %.500s",
                       mychar, curfnt->name);
               error(errbuf);
            }
         }
         if (curfnt->loaded == 2) { /* scanning a virtual font character */
            struct tft *ffont0;
            frp->curp = curpos;
            frp->curl = curlim;
            frp->ff = ffont;
            frp->curf = curfnt;
            if (++frp == &frames[MAXFRAME] )
               error("! virtual recursion stack overflow");
            cd = curfnt->chardesc + mychar;
            if (mychar>=curfnt->maxchars || cd->packptr == NULL) {
               if (!noptex && mychar<0x1000000 && curfnt->kind == VF_PTEX) { /* fallback */
#ifdef DEBUG
   if (dd(D_FONTS))
      fprintf_str(stderr,
              "We will fallback pTeX vf:%s to %s:%s\n",
               curfnt->name, curfnt->localfonts->desc->kind==OFM_OMEGA ? "ofm" : "tfm",
               curfnt->localfonts->desc->name);
#endif /* DEBUG */
               } else {
                  fprintf(stderr, "Wrong char code: %04X\n", mychar);
                  error("! a non-existent virtual character is being used; check vf/tfm files");
               }
               ffont0 = curfnt->localfonts;
               if (ffont0==NULL)
                  curfnt = NULL;
               else
                  ppreselectfont(ffont0->desc);
               --frp;
               curfnt = frp->curf;
               ffont = frp->ff;
               curlim = frp->curl;
               curpos = frp->curp;
            } else {
               curpos = cd->packptr + 2;
               curlim = curpos + (256*(long)(*cd->packptr)+(*(cd->packptr+1)));
               ffont = curfnt->localfonts;
               if (ffont==NULL)
                  curfnt = NULL;
               else
                  ppreselectfont(ffont->desc);
            }
         } else if (curfnt->loaded == 3)
            curfnt->chardesc[mychar].flags = EXISTS;
         break;
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
            fnt = dvibyte(); /* fnt1 */
            while (cmd-- > 235)
               fnt = (fnt << 8) + dvibyte();
         }
         for (cfnt=ffont; cfnt; cfnt = cfnt->next)
            if (cfnt->fontnum == fnt) goto fontfound;
         error("! no font selected");
fontfound:  curfnt = cfnt->desc;
         ppreselectfont(curfnt);
         break;
case 239: predospecial((integer)dvibyte(), 1); break; /* xxx1 */
case 240: predospecial((integer)twobytes(), 1); break; /* xxx2 */
case 241: predospecial(threebytes(), 1); break; /* xxx3 */
case 242: predospecial(signedquad(), 1); break; /* xxx4 */
case 243: case 244: case 245: case 246: fontdef(cmd-242); break; /* fntdef1 */
case 140: /* eop or end of virtual char */
         if (curpos) {
            --frp;
            curfnt = frp->curf;
            ffont = frp->ff;
            curlim = frp->curl;
            curpos = frp->curp;
            break;
         }
         return;
      }
   }
}
/*
 *   Finally, here's our main pprescan routine.
 */
static integer firstmatch = -1, lastmatch = -1;
void
pprescanpages(void)
{
   register int cmd;
   integer lmaxpages = maxpages;
   integer mpagenum;
   integer pageseq = 0;
   int ntfirst = notfirst;

   pprescan = 1;
   readpreamble();
/*
 *   Now we look for the first page to process.  If we get to the end of
 *   the file before the page, we complain (fatally).
 *   Incidentally, we don't use the DVI file's bop backpointer to skip
 *   over pages at high speed, because we want to look to for special
 *   header that might be in skipped pages.
 */
   while (1) {
      cmd = skipnop();
      if (cmd==248)
         error("! End of document before first specified page");
      if (cmd!=139)
         error("! Bad DVI file: expected bop");
      pagenum = signedquad();
      pageseq++;
      mpagenum = abspage ? pageseq : pagenum;
      if (mpagenum == firstpage && ntfirst)
         firstmatch++;
      if (mpagenum == lastpage && notlast)
         lastmatch++;
      if (ntfirst && mpagenum == firstpage && firstmatch == firstseq)
         ntfirst = 0;
      if (ntfirst ||
          ((evenpages && (pagenum & 1)) || (oddpages && (pagenum & 1)==0) ||
           (pagelist && !InPageList(pagenum)))) {
         skipover(40);
         skippage();
      } else {
         if (notlast && mpagenum == lastpage)
            lastmatch--;
         break;
      }
   }
/*
 *   Here we scan for each of the sections.  First we initialize some of
 *   the variables we need.  We don't have to be as smart as before in
 *   skipping pages.
 */
   skipover(40);
   while (lmaxpages > 0) {
      if (!(evenpages && (pagenum & 1)) &&
         !(oddpages && (pagenum & 1)==0) &&
         !(pagelist && !InPageList(pagenum))) {
         pscanpage();
         lmaxpages--;
      } else
         skippage();
      mpagenum = abspage ? pageseq : pagenum;
      if (mpagenum == lastpage && notlast)
         lastmatch++;
      if (notlast && mpagenum == lastpage && lastmatch == lastseq)
         lmaxpages = -1; /* we are done after this page. */
      cmd=skipnop();
      if (cmd==248) break;
      if (cmd!=139)
         error("! Bad DVI file: expected bop");
      pagenum = signedquad();
      skipover(40);
      pageseq++;
   }
   fseek(dvifile, 0L, 0);
   pprescan = 0;
}
