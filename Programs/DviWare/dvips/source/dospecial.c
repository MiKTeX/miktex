/* $Id: dospecial.c 50650 2019-03-30 01:50:10Z karl $
 *   This routine handles special commands;
 *   predospecial() is for the prescan, dospecial() for the real thing.
 */
#include "dvips.h" /* The copyright notice in that file is included too! */

#ifdef KPATHSEA
#include <kpathsea/c-ctype.h>
#include <kpathsea/tex-hush.h>
#else /* ! KPATHSEA */
#include <ctype.h>
#include <stdlib.h>
#ifndef WIN32
extern int atoi();
extern int system();
#endif /* WIN32*/
#endif
/*
 *   The external declarations:
 */
#include "protos.h"

static int specialerrors = 20;

struct bangspecial {
   struct bangspecial *next;
   char *actualstuff;
} *bangspecials = NULL;

void
specerror(const char *s)
{
   if (specialerrors > 0
#ifdef KPATHSEA
       && !kpse_tex_hush ("special")
#endif
       ) {
      error(s);
      specialerrors--;
   } else if (specialerrors == 0
#ifdef KPATHSEA
	      && !kpse_tex_hush ("special")
#endif
	      ) {
      error("more errors in special, being ignored . . .");
      error("(perhaps dvips doesn't support your macro package?)");
      specialerrors--;
   }
}

static void
outputstring(register char *p)
{
   putc('\n', bitfile);
   while(*p) {
      putc(*p, bitfile);
      p++;
   }
   putc('\n', bitfile);
}

static void
trytobreakout(register char *p)
{
   register int i;
   register int instring = 0;
   int lastc = 0;

   i = 0;
   putc('\n', bitfile);

   if(*p == '%') {
      while(*p) {
         putc(*p, bitfile);
         p++;
      }
      putc('\n', bitfile);
      return;
   }

   while (*p) {
      if (i > 65 && *p == ' ' && instring == 0) {
         putc('\n', bitfile);
         i = 0;
      } else {
         putc(*p, bitfile);
         i++;
      }
      if (*p == '(' && lastc != '\\')
         instring = 1;
      else if (*p == ')' && lastc != '\\')
         instring = 0;
      lastc = *p;
      p++;
   }
   putc('\n', bitfile);
}

static void
dobs(register struct bangspecial *q)
{
   if (q) {
      dobs(q->next);
      trytobreakout(q->actualstuff);
   }
}

/* added for dvi2ps & jdvi2kps format */
static char *mfgets(char *buf, unsigned int bytes, FILE *fp);

static void
fgetboundingbox(char *f, float *llx_p, float *lly_p, float *urx_p, float *ury_p)
{
   FILE *fp;
   char buf[BUFSIZ+1];

   fp = search(figpath, f, READ);
   if (fp == 0)
      fp = search(headerpath, f, READ);
   if (fp) {
      while (mfgets(buf, BUFSIZ, fp) != 0) {
         if (buf[0] == '%' && buf[1] == '%'
             && strncmp(buf+2, "BoundingBox:", 12) == 0) {
             if (sscanf(buf+14, "%f %f %f %f", llx_p, lly_p, urx_p, ury_p) == 4) {
                fclose(fp);
                return;
             }
          }
      }
      fclose(fp);
   }
   sprintf(errbuf, "Couldn't get BoundingBox of %.500s: assuming full A4 size", f);
   specerror(errbuf);
   *llx_p = 0.0;
   *lly_p = 0.0;
   *urx_p = 595.0;
   *ury_p = 842.0;
}

static char *
mfgets(char *buf, unsigned int bytes, FILE *fp)
{
   int i, cc;

   for (i = 0; i < bytes; i++) {
      cc = fgetc(fp);
      if (cc == 0x0a || cc == 0x0d) {
         if (cc == 0x0d) {
            cc = fgetc(fp);
            if (cc != 0x0a) {
               ungetc(cc, fp);
            }
         }
         cc = 0x0a;
         buf[i] = cc;
         buf[i+1] = '\0';
         return buf;
      } else if (cc == EOF) {
         buf[i] = '\0';
         if (i == 0) return NULL;
         else return buf;
      } else {
         buf[i] = cc;
      }
   }
   buf[i] = '\0';
   return buf;
}

static void
floatroundout(float f)
{
   integer i;
   i = (integer)(f<0 ? f-0.5 : f+0.5);
   if (i-f < 0.001 && i-f > -0.001) {
      numout((integer)i);
   } else {
      floatout(f);
   }
}
/* end of addition */

void
outbangspecials(void) {
   if (bangspecials) {
      cmdout("TeXDict");
      cmdout("begin");
      cmdout("@defspecial\n");
      dobs(bangspecials);
      cmdout("\n@fedspecial");
      cmdout("end");
   }
}

/* We recommend that new specials be handled by the following general
 * (and extensible) scheme, in which the user specifies one or more
 * `key=value' pairs separated by spaces.
 * The known keys are given in KeyTab; they take values
 * of one of the following types:
 *
 * None: no value, just a keyword (in which case the = sign is omitted)
 * String: the value should be "<string without double-quotes"
 *                          or '<string without single-quotes'
 * Integer: the value should be a decimal integer (%d format)
 * Number: the value should be a decimal integer or real (%f format)
 * Dimension: like Number, but will be multiplied by the scaledsize
 *       of the current font and converted to default PostScript units
 * (Actually, strings are allowed in all cases; the delimiting quotes
 *  are simply stripped off if present.)
 *
 */

typedef enum {None, String, Integer, Number, Dimension} ValTyp;
typedef struct {
   const char    *Entry;
   ValTyp  Type;
} KeyDesc;

#define NKEYS    (sizeof(KeyTab)/sizeof(KeyTab[0]))

KeyDesc KeyTab[] = {{"psfile",  String}, /* j==0 in the routine below */
                    {"ifffile", String}, /* j==1 */
                    {"tekfile", String}, /* j==2 */
                    {"hsize",   Number},
                    {"vsize",   Number},
                    {"hoffset", Number},
                    {"voffset", Number},
                    {"hscale",  Number},
                    {"vscale",  Number},
                    {"angle",   Number},
                    {"llx", Number},
                    {"lly", Number},
                    {"urx", Number},
                    {"ury", Number},
                    {"rwi", Number},
                    {"rhi", Number},
                    {"clip", None}};

#ifndef KPATHSEA
#define TOLOWER Tolower
#ifdef VMS
#ifndef __GNUC__	/* GNUC tolower is too simple */
#define Tolower tolower
#endif
#else
#ifdef VMCMS    /* IBM: VM/CMS */
#define Tolower __tolower
#else
#ifdef MVSXA    /* IBM: MVS/XA */
#define Tolower __tolower
#else
/*
 * compare strings, ignore case
 */
char
Tolower(register char c)
{
   if ('A' <= c && c <= 'Z')
      return(c+32);
   else
      return(c);
}
#endif
#endif  /* IBM: VM/CMS */
#endif
#endif /* !KPATHSEA */
static int
IsSame(const char *a, const char *b)
{
   for(; *a != '\0'; ) {
      if( TOLOWER(*a) != TOLOWER(*b) )
         return( 0 );
      a++;
      b++;
   }
   return( *b == '\0' );
}

char *KeyStr;       /* Key and ... */
const char *ValStr; /* ... String values found */
long ValInt; /* Integer value found */
float ValNum; /* Number or Dimension value found */

static char *
GetKeyVal(char *str, int *tno) /* returns NULL if none found, else next scan point */
     /* str : starting point for scan */
     /* tno : table entry number of keyword, or -1 if keyword not found */
{
   register char *s;
   register int i;
   register char t;

   for (s=str; *s <= ' ' && *s; s++); /* skip over blanks */
   if (*s == '\0')
      return (NULL);
   KeyStr = s;
   while (*s>' ' && *s!='=') s++;
   if (0 != (t = *s))
      *s++ = 0;

   for(i=0; i<NKEYS; i++)
      if( IsSame(KeyStr, KeyTab[i].Entry) )
         goto found;
   *tno = -1;
   return (s);

found: *tno = i;
   if (KeyTab[i].Type == None)
      return (s);

   if (t && t <= ' ') {
      for (; *s <= ' ' && *s; s++); /* now look for the value part */
      if ((t = *s)=='=')
         s++;
   }
   ValStr = "";
   if ( t == '=' ) {
      while (*s <= ' ' && *s)
         s++;
      if (*s=='\'' || *s=='\"')
         t = *s++;               /* get string delimiter */
      else t = ' ';
      ValStr = s;
      while (*s!=t && *s)
         s++;
      if (*s)
         *s++ = 0;
   }
   switch (KeyTab[i].Type) {
 case Integer:
      if(sscanf(ValStr,"%ld",&ValInt)!=1) {
          sprintf(errbuf,"Non-integer value (%.500s) given for keyword %.500s",
              ValStr, KeyStr);
          specerror(errbuf);
          ValInt = 0;
      }
      break;
 case Number:
 case Dimension:
      if(sscanf(ValStr,"%f",&ValNum)!=1) {
          sprintf(errbuf,"Non-numeric value (%.500s) given for keyword %.500s",
              ValStr, KeyStr);
          specerror(errbuf);
          ValNum = 0.0;
      }
      if (KeyTab[i].Type==Dimension) {
         if (curfnt==NULL)
            error("! No font selected");
         ValNum = ValNum * ((double)curfnt->scaledsize) * conv * 72 / DPI;
      }
      break;
 default: break;
   }
   return (s);
}

/*
 *   Now our routines.  We get the number of bytes specified and place them
 *   into the string buffer, and then parse it. Numerous conventions are
 *   supported here for historical reasons.
 *
 *   To support GNUplot's horribly long specials, we go ahead and malloc a
 *   new string buffer if necessary.
 */

void
predospecial(integer numbytes, Boolean scanning)
{
   register char *p = nextstring;
   register int i = 0;
   int j;
   static int omega_specials = 0;

   if (numbytes < 0 || numbytes > maxstring - nextstring) {
      if (numbytes < 0 || numbytes > (INT_MAX - 1000) / 2 ) {
         error("! Integer overflow in predospecial");
         exit(1);
      }
      p = nextstring = mymalloc(1000 + 2 * numbytes);
      maxstring = nextstring + 2 * numbytes + 700;
   }
   for (i=numbytes; i>0; i--)
#ifdef VMCMS /* IBM: VM/CMS */
      *p++ = ascii2ebcdic[(char)dvibyte()];
#else
#ifdef MVSXA /* IBM: MVS/XA */
      *p++ = ascii2ebcdic[(char)dvibyte()];
#else
      *p++ = (char)dvibyte();
#endif /* IBM: VM/CMS */
#endif
   if (pprescan)
      return;
   while (p[-1] <= ' ' && p > nextstring)
      p--; /* trim trailing blanks */
   if (p==nextstring) return; /* all blank is no-op */
   *p = 0;
   p = nextstring;
   while (*p <= ' ')
      p++;
#ifdef DEBUG
   if (dd(D_SPECIAL))
      fprintf(stderr, "Preprocessing special: %s\n", p);
#endif

/*
 *   We use strncmp() here to also pass things like landscape()
 *   or landscape: or such.
 */

   switch (*p) {
case 'o':
   if (strncmp(p, "om:", 3)==0) {       /* Omega specials ignored */
        if (omega_specials==0) {
                fprintf(stderr, "Omega specials are currently ignored\n");
                omega_specials++;
        }
        return;
   }
   break;
case 'l':
   if (strncmp(p, "landscape", 9)==0) {
      if (hpapersize || vpapersize)
         error(
             "both landscape and papersize specified:  ignoring landscape");
      else
         landscape = 1;
      return;
   }
#if defined(MIKTEX)
   if (miktex_no_landscape && landscape)
     {
       landscape = 0;
     }
#endif
   break;
case 'p':
   if (strncmp(p, "pos:", 4)==0) return; /* positional specials */
   if (strncmp(p, "papersize", 9)==0) {
      p += 9;
      while (*p == '=' || *p == ' ')
         p++;
      if (lastpsizwins || hpapersize == 0 || vpapersize == 0) {
         if (landscape) {
            error(
             "both landscape and papersize specified:  ignoring landscape");
            landscape = 0;
         }
         handlepapersize(p, &hpapersize, &vpapersize);
      }
      return;
   }
   break;
case 'x':
   if (strncmp(p, "xtex:", 5)==0) return;
   break;
case 's':
   if (strncmp(p, "src:", 4)==0) return; /* source specials */
   break;

case 'h':
   if (strncmp(p, "header", 6)==0) {
      char *q, *r, *pre = NULL, *post = NULL;
      p += 6;
      while ((*p <= ' ' || *p == '=' || *p == '(') && *p != 0)
         p++;
      if(*p == '{') {
	 p++;
	 while ((*p <= ' ' || *p == '=' || *p == '(') && *p != 0)
	    p++;
	 q = p;
	 while (*p != '}' && *p != 0)
	    p++;
	 r = p-1;
	 while ((*r <= ' ' || *r == ')') && r >= q)
	    r--;
	 if (*p != 0) p++;
	 r[1] = 0; /* q is the file name */
	 while ((*p <= ' ' || *p == '=' || *p == '(') && *p != 0)
	    p++;
	 if(strncmp(p, "pre", 3) == 0) {
	    int bracecount = 1, num_bytes = 0;
	    while(*p != '{' && *p != 0)
	       p++;
	    if (*p != 0) p++;
	    for(r = p; *r != 0; r++) {
	       if (*r == '{') bracecount++;
	       else if (*r == '}') bracecount--;
	       if (bracecount == 0) break;
	       num_bytes++;
	    }
	    pre = (char *)malloc(num_bytes+1);
	    r = pre;
	    for (j=0; j < num_bytes; j++)
	       *r++ = *p++;
	    *r = 0;
	    if (*p != 0) p++;
	 }
	 while ((*p <= ' ' || *p == '=' || *p == '(') && *p != 0)
	    p++;
	 if(strncmp(p, "post", 4) == 0) {
	    int bracecount = 1, num_bytes = 0;
	    while(*p != '{' && *p != 0)
	       p++;
	    if (*p != 0) p++;
	    for(r = p; *r != 0; r++) {
	       if (*r == '{') bracecount++;
	       else if (*r == '}') bracecount--;
	       if (bracecount == 0) break;
	       num_bytes++;
	    }
	    post = (char *)malloc(num_bytes+1);
	    r = post;
	    for (j=0; j < num_bytes; j++)
	       *r++ = *p++;
	    *r = 0;
	 }
	 if (strlen(q) > 0)
	    add_header_general(q, pre, post);
      } else {
	 q = p;  /* we will remove enclosing parentheses */
	 p = p + strlen(p) - 1;
	 while ((*p <= ' ' || *p == ')') && p >= q)
	    p--;
	 p[1] = 0;
	 if (p >= q)
	    add_header(q);
      }
   }
   break;
/* IBM: color - added section here for color header and color history */
/* using strncmp in this fashion isn't perfect; if later someone wants
   to introduce a verb like colorspace, well, just checking
   strcmp(p, "color", 5) will not do.  But we leave it alone for the moment.
   --tgr */
case 'b':
   if (strncmp(p, "background", 10) == 0) {
      usescolor = 1;
      p += 10;
      while ( *p && *p <= ' ' ) p++;
      background(p);
      return;
   }
   break;
case 'c':
   if (strncmp(p, "color", 5) == 0) {
      usescolor = 1;
      p += 5;
      while ( *p && *p <= ' ' ) p++;
      if (strncmp(p, "push", 4) == 0 ) {
         p += 4;
         while ( *p && *p <= ' ' ) p++;
         pushcolor(p, 0);
      } else if (strncmp(p, "pop", 3) == 0 ) {
         popcolor(0);
      } else {
         resetcolorstack(p,0);
      }
   }   /* IBM: color - end changes */
   break;
case '!':
   {
      register struct bangspecial *q;
      p++;
      q = (struct bangspecial *)mymalloc((integer)(sizeof(struct bangspecial)));
      q->actualstuff = mymalloc(strlen(p) + 1);
      strcpy(q->actualstuff, p);
      q->next = bangspecials;
      bangspecials = q;
      usesspecial = 1;
      return;
   }
   break;
default:
   break;
   }
   usesspecial = 1;  /* now the special prolog will be sent */
   if (scanning && *p != '"' && (p=GetKeyVal(p, &j)) != NULL && j==0
       && *ValStr != '`') /* Don't bother to scan compressed files.  */
      scanfontcomments(ValStr);
}

/* Return 1 if S is readable along figpath, 0 if not. */
static int
maccess(char *s)
{
   FILE *f = search(figpath, s, "r");
   int found = (f != 0);
   if (found)
      (*close_file) (f);
   return found;
}

const char *tasks[] = { 0, "iff2ps", "tek2ps" };

#define PSFILESIZ 511
static char psfile[PSFILESIZ];

void
dospecial(integer numbytes)
{
   register char *p = nextstring;
   register int i = 0;
   int j, systemtype = 0;
   register const char *q;
   Boolean psfilewanted = 1;
   const char *task = 0;
   char cmdbuf[111];
#ifdef HPS
if (HPS_FLAG && PAGEUS_INTERUPPTUS) {
     HREF_COUNT--;
     start_new_box();
     PAGEUS_INTERUPPTUS = 0;
     }
if (HPS_FLAG && NEED_NEW_BOX) {
       vertical_in_hps();
       NEED_NEW_BOX = 0;
       }
#endif
   if (nextstring + numbytes > maxstring)
      error("! out of string space in dospecial");
   for (i=numbytes; i>0; i--)
#ifdef VMCMS /* IBM: VM/CMS */
      *p++ = ascii2ebcdic[(char)dvibyte()];
#else
#ifdef MVSXA /* IBM: MVS/XA */
      *p++ = ascii2ebcdic[(char)dvibyte()];
#else
      *p++ = (char)dvibyte();
#endif  /* IBM: VM/CMS */
#endif
   while (p[-1] <= ' ' && p > nextstring)
      p--; /* trim trailing blanks */
   if (p==nextstring) return; /* all blank is no-op */
   *p = 0;
   p = nextstring;
   while (*p <= ' ')
      p++;
#ifdef DEBUG
   if (dd(D_SPECIAL))
      fprintf(stderr, "Processing special: %s\n", p);
#endif

   switch (*p) {
case 'o':
   if (strncmp(p, "om:", 3)==0) {       /* Omega specials ignored */
        return;
   }
   break;
case 'e':
   if (strncmp(p, "em:", 3)==0) {	/* emTeX specials in emspecial.c */
	emspecial(p);
	return;
   }

/* added for dvi2ps special */
   if (strncmp(p, "epsfile=", 8)==0) {  /* epsf.sty for dvi2ps-j */
      float llx, lly, urx, ury;
      unsigned psfilelen = 0;

      p += 8;
      while (*p && !isspace((unsigned char)*p)) {
        if (psfilelen < PSFILESIZ) {
          psfile[psfilelen] = *p;
          psfilelen++;
          p++;
        } else {
          psfile[psfilelen] = 0; /* should not strictly be necessary */
          sprintf(errbuf,
                  "! epsfile=%.20s... argument longer than %d characters",
                  psfile, PSFILESIZ);
          error(errbuf);
        }
      }
      if (psfilelen == 0) {
        error ("! epsfile= argument empty");
      }
      psfile[psfilelen] = 0;
#if defined(MIKTEX_WINDOWS)
      // fixes #2336 dvips chokes on graphic files with accents
      // https://sourceforge.net/p/miktex/bugs/2336/
      std::string utf8FileName;
      if (!Utils::IsUTF8(psfile))
      {
        utf8FileName = MiKTeX::Util::StringUtil::AnsiToUTF8(psfile);
        strcpy_s(psfile, utf8FileName.c_str());
      }
#endif
      p += strlen(psfile);
      fgetboundingbox(psfile, &llx, &lly, &urx, &ury);
      hvpos();
      cmdout("@beginspecial");
      floatroundout(llx);
      cmdout("@llx");
      floatroundout(lly);
      cmdout("@lly");
      floatroundout(urx);
      cmdout("@urx");
      floatroundout(ury);
      cmdout("@ury");

      while ((p = GetKeyVal(p, &j))) {
         switch (j) {
            case 3: /* hsize */
               floatroundout(ValNum*10);
               cmdout("@rwi");
               break;
            case 4: /* vsize */
               floatroundout(ValNum*10);
               cmdout("@rhi");
               break;
            case 7: /* hscale */
               floatroundout((urx-llx)*ValNum*10);
               cmdout("@rwi");
               break;
            case 8: /* vscale */
               floatroundout((ury-lly)*ValNum*10);
               cmdout("@rhi");
               break;
            default:
               sprintf(errbuf, "Unknown keyword `%.500s' in \\special{epsfile=%.500s...} will be ignored\n",
               KeyStr, psfile);
               specerror(errbuf);
               break;
         }
      }
      cmdout("@setspecial");
      numout((integer)0);
      cmdout("lly");
      cmdout("ury");
      cmdout("sub");
      cmdout("TR");
      figcopyfile(psfile, 0);
      cmdout("\n@endspecial");
      return;
   }
/* end addition */
   break;
case 'p':
   if (strncmp(p, "pos:", 4)==0) return; /* positional specials */
   if (strncmp(p, "ps:", 3)==0) {
        psflush(); /* now anything can happen. */
        if (p[3]==':') {
           if (strncmp(p+4, "[nobreak]", 9) == 0) {
              hvpos();
              outputstring(&p[13]);
           } else if (strncmp(p+4, "[begin]", 7) == 0) {
              hvpos();
              trytobreakout(&p[11]);
           } else if (strncmp(p+4, "[end]", 5) == 0)
              trytobreakout(&p[9]);
           else trytobreakout(&p[4]);
        } else if (strncmp(p+3, " plotfile ", 10) == 0) {
             char *sfp;
             hvpos();
             p += 13;
           /*
            *  Fixed to allow popen input for plotfile
            *  TJD 10/20/91
            */
           while (*p == ' ') p++;
           if (*p == '"') {
             p++;
             for (sfp = p; *sfp && *sfp != '"'; sfp++);
           } else {
             for (sfp = p; *sfp && *sfp != ' '; sfp++);
           }
           *sfp = '\0';
           if (*p == '`')
             figcopyfile(p+1, 1);
           else
             figcopyfile (p, 0);
           /* End TJD changes */
        } else {
           hvpos();
           trytobreakout(&p[3]);
           psflush();
           hvpos();
        }
        return;
   }
   if (strncmp(p, "papersize", 9) == 0)
      return;
#ifdef TPIC
   if (strncmp(p, "pn ", 3) == 0) {setPenSize(p+2); return;}
   if (strncmp(p, "pa ", 3) == 0) {addPath(p+2); return;}
#endif

/* added for jdvi2kps special */
   if (strncmp(p, "postscriptbox", 13)==0) { /* epsbox.sty for jdvi2kps */
      float w, h;
      float llx, lly, urx, ury;
      if (strlen(p)-13-6 >= PSFILESIZ) { /* -6 for the braces */
         /* We're not allowing as long a name as we could, since however
            many characters the two {%fpt} arguments consume is not
            taken into account. But parsing it all so we know the
            character length is too much work for this obscure special. */
         sprintf(errbuf,
                 "! postscriptbox{} arguments longer than %d characters",
                 PSFILESIZ);
         error(errbuf);
      }
      if (sscanf(p+13, "{%fpt}{%fpt}{%[^}]}", &w, &h, psfile) != 3)
         break;
#if defined(MIKTEX_WINDOWS)
      // fixes #2336 dvips chokes on graphic files with accents
      // https://sourceforge.net/p/miktex/bugs/2336/
      std::string utf8FileName;
      if (! Utils::IsUTF8(psfile))
      {
        utf8FileName = MiKTeX::Util::StringUtil::AnsiToUTF8(psfile);
        strcpy_s(psfile, utf8FileName.c_str());
      }
#endif
      fgetboundingbox(psfile, &llx, &lly, &urx, &ury);
      hvpos();
      cmdout("@beginspecial");
      floatroundout(llx);
      cmdout("@llx");
      floatroundout(lly);
      cmdout("@lly");
      floatroundout(urx);
      cmdout("@urx");
      floatroundout(ury);
      cmdout("@ury");
      floatroundout(w*10*72/72.27);
      cmdout("@rwi");
      floatroundout(h*10*72/72.27);
      cmdout("@rhi");
      cmdout("@setspecial");
      figcopyfile(psfile, 0);
      cmdout("\n@endspecial");
      return;
   }
/* end addition */
   break;
case 'l':
   if (strncmp(p, "landscape", 9)==0) return;
   break;
case '!':
   return;
case 'h':
   if (strncmp(p, "header", 6)==0) return;
#ifdef HPS
   if (strncmp(p, "html:", 5)==0) {
     if (! HPS_FLAG) return;
	 		p += 5;
			while (isspace((unsigned char)*p))
   		p++;
			if (*p == '<') {
    			char               *sp = p;
    			char               *str;
    			int                 ii=0;int len;int lower_len;

    			while ((*p) && (*p != '>')) {
						ii++;
						p++;
   			 }
    		str = (char *)mymalloc(ii+2);
   			strncpy(str,sp+1,ii-1);
    		str[ii-1] = 0;len=strlen(str);
				if(len>6) lower_len=6; else lower_len=len;
				for(ii=0;ii<lower_len;ii++) str[ii]=tolower((unsigned char)str[ii]);
				do_html(str);
   			free(str);
				} else
#ifdef KPATHSEA
				  if (!kpse_tex_hush ("special"))
#endif
				    {

    			printf("Error in html special\n");
    			return;
				}
	return;
   }
#else
   if (strncmp(p, "html:", 5)==0) return;
#endif
case 'w':
case 'W':
   if (strncmp(p+1, "arning", 6) == 0) {
      static int maxwarns = 50;
      if (maxwarns > 0) {
         error(p);
         maxwarns--;
      } else if (maxwarns == 0) {
         error(". . . rest of warnings suppressed");
         maxwarns--;
      }
      return;
   }
#ifdef TPIC
   if (strcmp(p, "wh") == 0) {whitenLast(); return;}
#endif
   break;
case 'b':
   if ( strncmp(p, "background", 10) == 0 )
      return; /* already handled in prescan */
#ifdef TPIC
   if (strcmp(p, "bk") == 0) {blackenLast(); return;}
#endif
   break;
case 'c':
   if (strncmp(p, "color", 5) == 0) {
      p += 5;
      while ( *p && *p <= ' ' ) p++;
      if (strncmp(p, "push", 4) == 0 ) {
         p += 4;
         while ( *p && *p <= ' ' ) p++;
         pushcolor(p,1);
      } else if (strncmp(p, "pop", 3) == 0 ) {
         popcolor(1);
      } else {
         resetcolorstack(p,1);
      }
      return;
   } /* IBM: color - end changes*/
   break;
case 'f':
#ifdef TPIC
   if (strcmp(p, "fp") == 0) {flushPath(0); return;}
#endif
   break;
case 'i':
#ifdef TPIC
   if (strcmp(p, "ip") == 0) {flushPath(1); return;} /* tpic 2.0 */
   if (strncmp(p, "ia ", 3) == 0) {arc(p+2, 1); return;} /* tpic 2.0 */
#endif
   break;
case 'd':
#ifdef TPIC
   if (strncmp(p, "da ", 3) == 0) {flushDashed(p+2, 0); return;}
   if (strncmp(p, "dt ", 3) == 0) {flushDashed(p+2, 1); return;}
#endif
   break;
case 's':
   if (strncmp(p, "src:", 4) == 0) return; /* source specials */
#ifdef TPIC
   if (strcmp(p, "sp") == 0) {flushSpline(p+2); return;} /* tpic 2.0 */
   if (strncmp(p, "sp ", 3) == 0) {flushSpline(p+3); return;} /* tpic 2.0 */
   if (strcmp(p, "sh") == 0) {shadeLast(p+2); return;} /* tpic 2.0 */
   if (strncmp(p, "sh ", 3) == 0) {shadeLast(p+3); return;} /* tpic 2.0 */
#endif
   break;
case 'a':
#ifdef TPIC
   if (strncmp(p, "ar ", 3) == 0) {arc(p+2, 0); return;} /* tpic 2.0 */
#endif
   break;
case 't':
#ifdef TPIC
   if (strncmp(p, "tx ", 3) == 0) {SetShade(p+3); return;}
#endif
   break;
case '"':
   hvpos();
   cmdout("@beginspecial");
   cmdout("@setspecial");
   trytobreakout(p+1);
   cmdout("\n@endspecial");
   return;
   break;
default:
   break;
   }
/* At last we get to the key/value conventions */
   psfile[0] = '\0';
   hvpos();
   cmdout("@beginspecial");

   while( (p=GetKeyVal(p,&j)) != NULL )
      switch (j) {
 case -1: /* for compatibility with old conventions, we allow a file name
           * to be given without the 'psfile=' keyword */
         if (!psfile[0] && maccess(KeyStr)==1) { /* yes we can read it */
             if (strlen(KeyStr) >= PSFILESIZ) {
               sprintf(errbuf, 
           "! Bare filename (%.20s...) in \\special longer than %d characters",
                       KeyStr, PSFILESIZ);
             }
#if defined(MIKTEX_WINDOWS)
           // fixes #2336 dvips chokes on graphic files with accents
           // https://sourceforge.net/p/miktex/bugs/2336/
         {
           std::string utf8FileName;
           if (! Utils::IsUTF8(KeyStr))
           {
             utf8FileName = MiKTeX::Util::StringUtil::AnsiToUTF8(KeyStr);
             strcpy_s(psfile, utf8FileName.c_str());
           }
         }
#else
             strcpy(psfile,KeyStr);
#endif
         } else {
           if (strlen(KeyStr) < 40) {
              sprintf(errbuf,
                      "Unknown keyword (%s) in \\special will be ignored",
                              KeyStr);
           } else {
              sprintf(errbuf,
                     "Unknown keyword (%.40s...) in \\special will be ignored",
                              KeyStr);
           }
           specerror(errbuf);
         }
         break;
 case 0: case 1: case 2: /* psfile */
         if (psfile[0]) {
           sprintf(errbuf, "More than one \\special %s given; %.40s ignored",
                    "psfile", ValStr);
           specerror(errbuf);
         } else {
           if (strlen(ValStr) >= PSFILESIZ) {
               sprintf(errbuf, 
           "! PS filename (%.20s...) in \\special longer than %d characters",
                       ValStr, PSFILESIZ);
	       error(errbuf);
           }
#if defined(MIKTEX_WINDOWS)
           // fixes #2336 dvips chokes on graphic files with accents
           // https://sourceforge.net/p/miktex/bugs/2336/
           {
             std::string utf8FileName;
             if (! Utils::IsUTF8(ValStr))
             {
               utf8FileName = MiKTeX::Util::StringUtil::AnsiToUTF8(ValStr);
               strcpy_s(psfile, utf8FileName.c_str());
             }
             else
             {
               strcpy_s(psfile, ValStr);
             }
           }
#else
           strcpy(psfile, ValStr);
#endif
         }
         task = tasks[j];
         break;
 default: /* most keywords are output as PostScript procedure calls */
         if (KeyTab[j].Type == Integer)
            numout((integer)ValInt);
         else if (KeyTab[j].Type == String)
            for (q=ValStr; *q; q++)
               scout(*q);
         else if (KeyTab[j].Type == None);
         else { /* Number or Dimension */
            ValInt = (integer)(ValNum<0? ValNum-0.5 : ValNum+0.5);
            if (ValInt-ValNum < 0.001 && ValInt-ValNum > -0.001)
                numout((integer)ValInt);
            else {
               snprintf(cmdbuf, sizeof(cmdbuf), "%f", ValNum);
               cmdout(cmdbuf);
            }
         }
      snprintf(cmdbuf, sizeof(cmdbuf), "@%s", KeyStr);
      cmdout(cmdbuf);
      }

   cmdout("@setspecial");

   if(psfile[0]) {
      if (task == 0) {
#if defined(MIKTEX)
         systemtype = (psfile[0]=='`' ? 1 : 0) ;
#else
         systemtype = (psfile[0]=='`');
#endif
         figcopyfile(psfile+systemtype, systemtype);
      } else {
         fprintf (stderr, "dvips: iff2ps and tek2ps are not supported.\n");
      }
   } else if (psfilewanted
#ifdef KPATHSEA
	      && !kpse_tex_hush ("special")
#endif
	      )
      specerror("No \\special psfile was given; figure will be blank");

   cmdout("@endspecial");
}

/*
 *   Handles specials encountered during bounding box calculations.
 *   Currently we only deal with psfile's or PSfiles and only those
 *   that do not use rotations.
 */
static float rbbox[4];
float *
bbdospecial(int nbytes)
{
   char *p = nextstring;
   int i, j;
   char seen[NKEYS];
   float valseen[NKEYS];

   if (nbytes < 0 || nbytes > maxstring - nextstring) {
      if (nbytes < 0 || nbytes > (INT_MAX - 1000) / 2 ) {
         error("! Integer overflow in bbdospecial");
         exit(1);
      }
      p = nextstring = mymalloc(1000 + 2 * nbytes);
      maxstring = nextstring + 2 * nbytes + 700;
   }
   if (nextstring + nbytes > maxstring)
      error("! out of string space in bbdospecial");
   for (i=nbytes; i>0; i--)
#ifdef VMCMS /* IBM: VM/CMS */
      *p++ = ascii2ebcdic[(char)dvibyte()];
#else
#ifdef MVSXA /* IBM: MVS/XA */
      *p++ = ascii2ebcdic[(char)dvibyte()];
#else
      *p++ = (char)dvibyte();
#endif  /* IBM: VM/CMS */
#endif
   while (p[-1] <= ' ' && p > nextstring)
      p--; /* trim trailing blanks */
   if (p==nextstring) return NULL; /* all blank is no-op */
   *p = 0;
   p = nextstring;
   while (*p && *p <= ' ')
      p++;
   if (strncmp(p, "psfile", 6)==0 || strncmp(p, "PSfile", 6)==0) {
      float originx = 0, originy = 0, hscale = 1, vscale = 1,
            hsize = 0, vsize = 0;
      for (j=0; j<NKEYS; j++)
         seen[j] = 0;
      j = 0;
      while ((p=GetKeyVal(p, &j))) {
         if (j >= 0 && j < NKEYS && KeyTab[j].Type == Number) {
	    seen[j]++;
	    valseen[j] = ValNum;
         }
      }
      /*
       *   This code mimics what happens in @setspecial.
       */
      if (seen[3])
         hsize = valseen[3];
      if (seen[4])
         vsize = valseen[4];
      if (seen[5])
         originx = valseen[5];
      if (seen[6])
         originy = valseen[6];
      if (seen[7])
         hscale = valseen[7] / 100.0;
      if (seen[8])
         vscale = valseen[8] / 100.0;
      if (seen[10] && seen[12])
         hsize = valseen[12] - valseen[10];
      if (seen[11] && seen[13])
         vsize = valseen[13] - valseen[11];
      if (seen[14] || seen[15]) {
         if (seen[14] && seen[15] == 0) {
	    hscale = vscale = valseen[14] / (10.0 * hsize);
         } else if (seen[15] && seen[14] == 0) {
	    hscale = vscale = valseen[15] / (10.0 * vsize);
         } else {
            hscale = valseen[14] / (10.0 * hsize);
            vscale = valseen[15] / (10.0 * vsize);
         }
      }
      /* at this point, the bounding box in PostScript units relative to
         the current dvi point is
           originx originy originx+hsize*hscale originy+vsize*vscale
         We'll let the bbox routine handle the remaining math.
       */
      rbbox[0] = originx;
      rbbox[1] = originy;
      rbbox[2] = originx+hsize*hscale;
      rbbox[3] = originy+vsize*vscale;
      return rbbox;
   }
   return 0;
}
