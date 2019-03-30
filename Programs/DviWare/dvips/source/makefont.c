/* makefont.c */
#include "dvips.h" /* The copyright notice in that file is included too! */
#include <math.h>
/*
 *   The external declarations:
 */
#include "protos.h"
#ifdef OS2
#include <stdlib.h>
#endif
#if !defined(WIN32)
extern char *getenv();
extern int system();
#endif
#if defined MSDOS || defined OS2
/*
 *  Write mfjob file
 */
void
mfjobout(char *font, double mag)
{
   if (mfjobfile == (FILE *)NULL) {
      char pkout[128];
      char *p;
      int i;
      for (p=pkpath, i=0; *p && *p!=PATHSEP && i<127; p++) {
         if (*p=='%') {
            p++;
            switch(*p) { /* convert %x codes to mfjob @y codes */
               case 'b':
                  sprintf(pkout+i,"%d",actualdpi);
                  break;
               case 'd':
                  strcpy(pkout+i,"@Rr");
                  break;
               case 'f':
                  strcpy(pkout+i,"@f");
                  break;
               case 'p':
                  strcpy(pkout+i,"pk");
                  break;
               case 'm':
                  strcpy(pkout+i, mfmode ? mfmode : "default");
                  break;
               case '%':
                  strcpy(pkout+i,"%");
                  break;
               default:
                  sprintf(pkout+i, "%%%c", *p);
                  fprintf(stderr,"Unknown option %%%c in pk path\n",*p);
            }
            i += strlen(pkout+i);
         }
         else
           pkout[i++] = *p;
      }
      /* *p='\0'; Could some DOS person explain to me what this does? */
      pkout[i] = 0;
      mfjobfile =  fopen(mfjobname,"w");
      if (mfjobfile == (FILE *)NULL)
         return;
      fprintf(mfjobfile,"input[dvidrv];\n{\ndriver=dvips;\n");
      if (actualdpi == vactualdpi)
         fprintf(mfjobfile,"mode=%s[%d];\n",mfmode,actualdpi);
      else
         fprintf(mfjobfile,"mode=%s[%d %d];\n",mfmode,actualdpi,vactualdpi);
      fprintf(mfjobfile,"output=pk[%s];\n",pkout);
   }
   fprintf(mfjobfile,"{font=%s; mag=%f;}\n",font,mag);
   fprintf(stderr,
        "Appending {font=%s; mag=%f;} to %s\n",font,mag,mfjobname);
}
#endif
/*
 *   Calculate magstep values.
 */
static int
magstep(register int n, register int bdpi)
{
   register float t;
   int neg = 0;

   if (n < 0) {
      neg = 1;
      n = -n;
   }
   if (n & 1) {
      n &= ~1;
      t = 1.095445115;
   } else
      t = 1.0;
   while (n > 8) {
      n -= 8;
      t = t * 2.0736;
   }
   while (n > 0) {
      n -= 2;
      t = t * 1.2;
   }
   if (neg)
      return((int)floor(0.5 + bdpi / t));
   else
      return((int)floor(0.5 + bdpi * t));
}
#ifdef MAKEPKCMD
static char *defcommand = MAKEPKCMD " %n %d %b %m";
#else
#ifdef OS2
static char *doscommand = "command /c MakeTeXP %n %d %b %m";
static char *os2command = "MakeTeXP %n %d %b %m";
#define defcommand ( _osmode==OS2_MODE ? os2command : doscommand )
#else
#ifdef MSDOS
static char *defcommand = "command /c MakeTeXP %n %d %b %m";
#else
#ifdef VMCMS
static char *defcommand = "EXEC MakeTeXPK %n %d %b %m";
#else
#ifdef ATARIST
static char *defcommand = "maketexp %n %d %b %m";
#else
static char *defcommand = "MakeTeXPK %n %d %b %m";
#endif
#endif
#endif
#endif
#endif
char *command = 0;
/*
 *   This routine tries to create a font by executing a command, and
 *   then opening the font again if possible.
 */
static char buf[125];
void
makefont(char *name, int dpi, int bdpi)
{
#if defined(MIKTEX)
  std::shared_ptr<Session> session = Session::Get();
  if (!quiet)
  {
    fprintf(stderr, "- makepkfont (%s, %d, %d, %s)\n", name, dpi, bdpi, mfmode == nullptr ? "<null>" : mfmode);
  }
  if (dontmakefont == 0)
  {
    PathName pathMakePk;
    std::vector<std::string> arguments = session->MakeMakePkCommandLine(name, dpi, bdpi, mfmode, pathMakePk, TriState::Undetermined);
    Process::Run(pathMakePk, arguments);
  }
#else
   register char *p, *q;
   register int m, n;
   int modegiven = 0;
#if defined MSDOS || defined OS2 || defined(ATARIST)
   double t;
#endif

   if (command == 0)
      if (secure == 0 && (command=getenv("MAKETEXPK")))
         command = newstring(command);
      else 
         command = defcommand;
   for (p=command, q=buf; *p; p++)
      if (*p != '%')
         *q++ = *p;
      else {
         switch (*++p) {
case 'n' : case 'N' :
            strcpy(q, name);
            break;
case 'd' : case 'D' :
            sprintf(q, "%d", dpi);
            break;
case 'b' : case 'B' :
            sprintf(q, "%d", bdpi);
            break;
case 'o' : case 'O' :
            sprintf(q, "%.99s", mfmode ? mfmode : "default");
            modegiven = 1;
            break;
case 'm' : case 'M' :
/*
 *   Here we want to return a string.  If we can find some integer
 *   m such that floor(0.5 + bdpi * 1.2 ^ (m/2)) = dpi, we write out
 *      magstep(m/2)
 *   where m/2 is a decimal number; else we write out
 *      dpi/bdpi
 *   We do this for the very slight improvement in accuracy that
 *   magstep() gives us over the rounded dpi/bdpi.
 */
            m = 0;
            if (dpi < bdpi) {
               while (1) {
                  m--;
                  n = magstep(m, bdpi);
                  if (n == dpi)
                     break;
                  if (n < dpi || m < -40) {
                     m = 9999;
                     break;
                  }
               }
            } else if (dpi > bdpi) {
               while (1) {
                  m++;
                  n = magstep(m, bdpi);
                  if (n == dpi)
                     break;
                  if (n > dpi || m > 40) {
                     m = 9999;
                     break;
                  }
               }
            }
#if defined MSDOS || defined OS2
/* write out magnification as decimal number */
            if (m == 9999) {
               t = (double)dpi/bdpi;
            } else {
               if (m < 0)
                    n = -m;
               else
                    n = m;
               if (n & 1) {
                    n &= ~1;
                    t = 1.095445115;
               } else
                    t = 1.0;
               while (n > 0) {
                    n -= 2;
                    t = t * 1.2;
               }
               if (m < 0)
                    t = 1 / t;
            }
            sprintf(q, "%12.9f", t);
#else
#ifndef ATARIST
            if (m == 9999) {
#else
            {
#endif
               sprintf(q, "%d+%d/%d", dpi/bdpi, dpi%bdpi, bdpi);
            } else if (m >= 0) {
               sprintf(q, "magstep\\(%d.%d\\)", m/2, (m&1)*5);
            } else {
               sprintf(q, "magstep\\(-%d.%d\\)", (-m)/2, (m&1)*5);
            }
#endif
            break;
case 0 :    *q = 0;
            break;
default:    *q++ = *p;
            *q = 0;
            break;
         }
         q += strlen(q);
      }
   *q = 0;
   if (mfmode && !modegiven) {
      strcpy(q, " ");
      strcat(q, mfmode);
   }
#ifdef OS2
   if ((_osmode == OS2_MODE) && filter)
      strcat(buf, quiet ? " >nul" : " 1>&2");
#else
#ifndef VMCMS   /* no filters and no need to print to stderr */
#ifndef MVSXA
#ifndef MSDOS
#ifndef ATARIST
   if (filter)
      strcat(buf, quiet ? " >/dev/null" : " 1>&2");
#endif
#endif
#endif
#endif
#endif

#if defined MSDOS || defined OS2
   if (! quiet && mfjobname == (char *)NULL)
      fprintf(stderr, "- %s\n", buf);
   if (dontmakefont == 0) {
      if (mfjobname != (char *)NULL)
         mfjobout(name,t);
      else
         system(buf);
   }
#else
   if (! quiet)
      fprintf(stderr, "- %s\n", buf);
   if (dontmakefont == 0)
      system(buf);
#endif
   else {
      static FILE *fontlog = 0;

      if (fontlog == 0) {
         fontlog = fopen("missfont.log", "a");
         if (fontlog != 0) {
            fprintf(stderr,
#ifndef VMCMS
                  "Appending font creation commands to missfont.log\n");
#else
  "\nMissing font data will be passed to DVIPS EXEC via MISSFONT LOG\n");
#endif
         }
      }
      if (fontlog != 0) {
         fprintf(fontlog, "%s\n", buf);
         fflush(fontlog);
      }
   }
#endif
}
