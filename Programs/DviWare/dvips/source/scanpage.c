/*
 *   This routine scans a page for font definitions and character usage.
 *   It keeps an estimate of the memory remaining in the printer, and
 *   returns false when that memory is exhausted, unless scanning the
 *   first page. Otherwise it returns 1 if no problems were found,
 *   or 2 if it thinks the first page of a section may overflow memory.
 */
#include "dvips.h" /* The copyright notice in that file is included too! */

/*
 *   The external declarations:
 */
#include "protos.h"

/*
 * Charge pagecost for the cost of showing the character that *cd refers to
 * and charge fontmem for the cost of downloading the character if necessary.
 * A zero result indicates that we have run out of memory.
 *   If the character is small (a small packet), we save 43 bytes (since we
 *   use a single string instead of an array with integers in the PostScript.)
 */
Boolean
prescanchar(chardesctype *cd)
{
   register quarterword *cras;
   register integer thischar;

   if ((cd->flags & (PREVPAGE | THISPAGE)) == 0) {
      /* the character hasn't been used before in the current section */
      cras = cd->packptr;
      if (curfnt->resfont) {
         thischar = PSCHARCOST;
         cd->flags |= THISPAGE;
      } else if (cras != NULL) {
         thischar = CHARCOST;
         if (cd->flags & BIGCHAR)
            thischar += 43;
         if (compressed) {
            if ((cd->flags & REPACKED) == 0) {
               repack(cd);
               cras = cd->packptr;
            }
            if ((*cras & 4))
               if ((*cras & 7) == 7)
                  thischar += getlong(cras + 17);
               else
                  thischar += getlong(cras + 9);
            else
               thischar += getlong(cras + 5);
         } else {
            if ((*cras & 4))
               if ((*cras & 7) == 7)
                  thischar += getlong(cras + 1)
                                          * ((getlong(cras + 5) + 7) >> 3);
               else
                  thischar += (cras[3] * 256L + cras[4]) *
                                 ((cras[1] * 256L + cras[2] + 7) >> 3);
            else
               thischar += ((long)(cras[2] * ((cras[1] + 7) >> 3)));
         }
         cd->flags |= THISPAGE;
      } else
         thischar = 0;
      fontmem -= thischar;
      if (fontmem <= pagecost) {
         if (pagecount > 0)
            return(0);
      }
   }
   return(1);
}
/*
 * When a font is selected during the prescan, this routine makes sure that
 * the tfm or pk file is loaded and charges fontmem for the VM overhead in
 * creating the font.  The result is true unless the section must be aborted
 * in order to keep this font from overflowing VM.
 */
Boolean
preselectfont(fontdesctype *f)
{
   curfnt = f;
   if (curfnt->loaded == 0 || curfnt->loaded == 3) {
      if (!residentfont(curfnt))
         if (!virtualfont(curfnt))
            loadfont(curfnt);
   }
   if (curfnt->psname == 0) {
      if (curfnt->loaded < 2) { /* virtual font costs nothing (yet) */
         if (curfnt->resfont)
            fontmem -= PSFONTCOST;
         else {
            fontmem -= FONTCOST + curfnt->maxchars;
            if (curfnt->loadeddpi != curfnt->dpi)
               fontmem -= 48; /* for the new font matrix */
         }
      }
      curfnt->psname = 1;
      if (fontmem <= pagecost) {
         if (pagecount > 0)
            return(0);
      }
   }
   return(1);
}
/*
 *   Now our scanpage routine.
 */
short
scanpage(void)
{
   register shalfword cmd;
   register chardesctype *cd;
   register fontmaptype *cfnt = 0;
   integer fnt;
   integer mychar;
   register frametype *frp = frames;

  if (firstboploc == 0)
     firstboploc = ftell(dvifile);
   pagecost = 0;
#ifdef DEBUG
   if (dd(D_PAGE))
#ifdef SHORTINT
   fprintf(stderr,"Scanning page %ld\n", pagenum);
#else   /* ~SHORTINT */
   fprintf(stderr,"Scanning page %d\n", pagenum);
#endif  /* ~SHORTINT */
#endif  /* DEBUG */
   curfnt = NULL;
   curpos = NULL;

   bopcolor(0); /* IBM: color - put current colorstack to bopstackdepth */
 
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
/*
 *   We are going to approximate that each string of consecutive characters
 *   requires (length of string) bytes of PostScript VM.
 */
         if (curfnt==NULL)
            error("! Bad DVI file: no font selected");
         if (mychar>=curfnt->maxchars) {
            if (!noptex && mychar<0x1000000 && curfnt->loaded == 2 && curfnt->kind == VF_PTEX) {
               /* fallback */
            } else {
               sprintf(errbuf,"! [scanpage] invalid char %d from font %.500s",
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
               else if (!preselectfont(ffont0->desc))
                  goto outofmem;
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
               else if (!preselectfont(ffont->desc))
                  goto outofmem;
            }
         } else {
            pagecost++;
            if (!prescanchar(curfnt->chardesc + mychar))
               goto outofmem;
         }
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
         printf("Font number %d not found\n", fnt);
         error("! no font selected");
fontfound: curfnt = cfnt->desc;
         if (!preselectfont(curfnt))
              goto outofmem;
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
         goto endofpage; /* eop */
      }
   }
endofpage:
   if (fontmem > pagecost)
      return(1);
   if (pagecount > 0) {
      return(0);
    }  /* IBM: color */
#ifdef SHORTINT
      fprintf(stderr, "Page %ld may be too complex to print\n", pagenum);
#else   /* ~SHORTINT */
      fprintf(stderr, "Page %d may be too complex to print\n", pagenum);
#endif  /* ~SHORTINT */
/*
 *   This case should be rare indeed.  Even with only 200K of virtual memory,
 *   at 11 bytes per char, you can have 18K characters on a page.
 */
   return(2);
outofmem:
   if (frp != frames) {
      curpos = frames->curp;
      curlim = frames->curl;
      ffont = frames->ff;
      curfnt = frames->curf;
   }
   return 0;
}
