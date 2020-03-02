/*
 *   These routines do most of the communicating with the printer.
 *
 *   LINELENGTH tells the maximum line length to send out.  It's been
 *   reduced to 72 because sometimes PostScript files are included in
 *   mail messages and many mailers mutilate longer lines.
 */
#define LINELENGTH (72)
#include "dvips.h" /* The copyright notice in that file is included too! */
#include <ctype.h>
#include <stdlib.h>

#ifdef OS2
#ifdef _MSC_VER
#define popen(pcmd, pmode)  _popen(pcmd, pmode)
#define pclose(pstream) _pclose(pstream)
#endif
#endif
#if !defined(MIKTEX)
#if defined(WIN32) && defined(KPATHSEA)
#undef fopen
#undef popen
#undef pclose
extern FILE *generic_fsyscp_fopen(const char *name, const char *mode);
#define fopen(file, fmode)  generic_fsyscp_fopen(file, fmode)
#define popen(pcmd, pmode)  fsyscp_popen(pcmd, pmode)
#define pclose(pstream) _pclose(pstream)
#endif
#endif

#ifdef __DJGPP__
#include <unistd.h>	/* for `isatty' */
#endif

/* UCS -> UTF-8 */
#define UCStoUTF8B1(x)  (0xc0 + (((x) >>  6) & 0x1f))
#define UCStoUTF8B2(x)  (0x80 + (((x)      ) & 0x3f))

#define UCStoUTF8C1(x)  (0xe0 + (((x) >> 12) & 0x0f))
#define UCStoUTF8C2(x)  (0x80 + (((x) >>  6) & 0x3f))
#define UCStoUTF8C3(x)  (0x80 + (((x)      ) & 0x3f))

#define UCStoUTF8D1(x)  (0xf0 + (((x) >> 18) & 0x07))
#define UCStoUTF8D2(x)  (0x80 + (((x) >> 12) & 0x3f))
#define UCStoUTF8D3(x)  (0x80 + (((x) >>  6) & 0x3f))
#define UCStoUTF8D4(x)  (0x80 + (((x)      ) & 0x3f))

/* UTF-32 over U+FFFF -> UTF-16 surrogate pair */
#define UTF32toUTF16HS(x)  (0xd800 + (((x-0x10000) >> 10) & 0x3ff))
#define UTF32toUTF16LS(x)  (0xdc00 + (  x                 & 0x3ff))

/*
 *   The external declarations:
 */
#include "protos.h"

char preamblecomment[256]; /* usually "TeX output ..." */
integer rdir = 0, fdir = 0;
/*
 *   We need a few statics to take care of things.
 */
static void chrcmd(char c);
static void tell_needed_fonts(void);
static void print_composefont(void);
static void setdir(int d);
static int JIStoSJIS(int c);

static Boolean any_dir = 0; /* did we output any direction commands? */
static Boolean jflag = 0;
static integer rhh, rvv;
static int instring;
static Boolean lastspecial = 1;
static shalfword d;
static Boolean popened = 0;
static int lastfont;
static char strbuffer[LINELENGTH + 20], *strbp = strbuffer;
static struct papsiz *finpapsiz;
static struct papsiz defpapsiz = {
   0, 40258437L, 52099154L, "letter", ""
};
#ifdef CREATIONDATE
#if (!defined(VMS) && !defined(MSDOS) && !(defined(OS2) && defined(_MSC_VER)) && !defined(ATARIST))
 /* VAXC/MSDOS don't like/need this !! */
#include <sys/types.h>
#ifndef WIN32
#include <sys/time.h> /* time(), at least on BSD Unix */
#endif
#endif
#include <time.h> /* asctime() and localtime(), at least on BSD Unix */
static time_t jobtime;

#if defined(_MSC_VER)
#define strtoll _strtoi64
#endif

#define INVALID_EPOCH_VALUE ((time_t) -1)

static time_t
get_unique_time_if_given(void)
{
   const char *source_date_epoch;
   int64_t epoch;
   char *endptr;
   time_t ret = INVALID_EPOCH_VALUE;

   source_date_epoch = getenv("SOURCE_DATE_EPOCH");
   if (source_date_epoch) {
      errno = 0;
      epoch = strtoll(source_date_epoch, &endptr, 10);
      if (!(epoch < 0 || *endptr != '\0' || errno != 0)) {
         ret = (time_t) epoch;
#if defined(_MSC_VER)
         if (ret > 32535291599ULL)
            ret = 32535291599ULL;
#endif
      }
   }
   return ret;
}
#endif /* CREATIONDATE */

/*
 *   This routine copies a file down the pipe.  Search path uses the
 *   header path.
 *
 *   We add code to handle the case of MS-DOS font files.
 *
 *   Format:  80 {01,02} four byte length in littleendian order data
 *   repeated possibly multiple times.
 */
static const char *hxdata = "0123456789ABCDEF";
static int infigure;
static char possibleDSCLine[81],
       *dscLinePointer = possibleDSCLine, *dscLineEnd = possibleDSCLine + 80;
void
copyfile_general(const char *s, struct header_list *cur_header)
{
   FILE *f = NULL;
   int c, prevc = '\n';
   long len;
   /* begin DOS EPS code */
   int doseps = 0;
   unsigned long dosepsbegin, dosepsend = 0;
   int removingBytes = 0;
   const char *scanForEnd = 0;
   int scanningFont = 0;

   /* end DOS EPS code */
#if defined(VMCMS) || defined (MVSXA)
   register char *lastdirsep;
   register char *trunc_s;
   trunc_s = s;
#endif
   dscLinePointer = possibleDSCLine;

   switch (infigure) {
   case 1:
/*
 *   Look in headerpath too, just in case.  This allows common header
 *   or figure files to be installed in the .../ps directory.
 */
      f = search(figpath, s, READBIN);
#if !defined(MIKTEX)
#if defined(WIN32)
      if (f == 0 && file_system_codepage != win32_codepage) {
         int tmpcp = file_system_codepage;
         file_system_codepage = win32_codepage;
         f = search(figpath, s, READBIN);
         file_system_codepage = tmpcp;
      }
#endif
#endif
      if (f == 0)
         f = search(headerpath, s, READBIN);
#if defined(VMCMS) || defined (MVSXA)
      lastdirsep = strrchr(s, '/');
      if ( NULL != lastdirsep ) trunc_s = lastdirsep + 1;
#ifdef VMCMS
      sprintf(errbuf,
   "Couldn't find figure file %s with CMS name %s; continuing.", s, trunc_s);
      if (secure == 2) {
         strcat(errbuf, "\nNote that an absolute path or a relative path with .. are denied in -R2 mode.");
      }
#else /* VMCMS */
      sprintf(errbuf,
    "Couldn't find figure file %s with MVS name %s; continuing.", s, trunc_s);
      if (secure == 2) {
         strcat(errbuf, "\nNote that an absolute path or a relative path with .. are denied in -R2 mode.");
      }
#endif /* VMCMS */
#else /* VMCMS || MVSXA */
      sprintf(errbuf, "Could not find figure file %.500s; continuing.", s);
      if (secure == 2) {
         strcat(errbuf, "\nNote that an absolute path or a relative path with .. are denied in -R2 mode.");
      }
#endif /* VMCMS || MVSXA */
      break;
#ifndef VMCMS
#ifndef MVSXA
#ifndef VMS
#if !defined(MSDOS) || defined(__DJGPP__)
#ifndef ATARIST
#ifndef __THINK__
   case 2:
#ifdef SECURE
      sprintf(errbuf, "<%.500s>: Tick filename execution disabled", s);
#else
#ifdef OS2
      if (_osmode == OS2_MODE) {
#endif
      if (secure == 0) {
         sprintf(errbuf, "Execution of <%.500s> failed ", s);
         f = popen(s, "r");
         if (f != 0)
            SET_BINARY(fileno(f));
	}
	else {
      sprintf(errbuf,"Secure mode is %d so execute <%.500s> will not run",
              secure, s);
	}
#ifdef OS2
      }
#endif
#endif
      break;
#endif
#endif
#endif
#endif
#endif
#endif
   default:
      f = search(headerpath, s, READBIN);
      if(cur_header && (cur_header->precode || cur_header->postcode)) {
	 if(f==NULL)
	    f = search(figpath, s, READBIN);
      }
      sprintf(errbuf, "! Could not find header file %.500s.", s);
      if (secure == 2) {
         strcat(errbuf, "\nNote that an absolute path or a relative path with .. are denied in -R2 mode.");
      }
      break;
   }
   if (f==NULL)
      error(errbuf);
   else {
      if (! quiet) {
#if defined(VMCMS) || defined (MVSXA)
         if (strlen(s) + prettycolumn > STDOUTSIZE) {
#else
         if (strlen(realnameoffile) + prettycolumn > STDOUTSIZE) {
#endif
            fprintf(stderr, "\n");
            prettycolumn = 0;
         }
#if defined(VMCMS) || defined (MVSXA)
         fprintf(stderr, "<%s>", trunc_s);
#else
         fprintf(stderr, "<%s>", realnameoffile);
#endif
         fflush(stderr);
#if defined(VMCMS) || defined (MVSXA)
         prettycolumn += 2 + strlen(s);
#else
         prettycolumn += 2 + strlen(realnameoffile);
#endif
      }
      if (linepos != 0)
         putc('\n', bitfile);
/*
 *   Suggested by Andrew Trevorrow; don't ever BeginFont a file ending in .enc
 */
      if (infont && strstr(s,".enc"))
         infont = 0;
      if (! disablecomments) {
         if (infigure)
            fprintf(bitfile, "%%%%BeginDocument: %s\n", s);
         else if (infont)
            fprintf(bitfile, "%%%%BeginFont: %s\n", infont);
#ifdef HPS
         else if (noprocset) {}
#endif
         else
            fprintf(bitfile, "%%%%BeginProcSet: %s 0 0\n", s);
      }
      if (cur_header && cur_header->precode) {
         fprintf(bitfile, "%s\n", cur_header->precode);
         free(cur_header->precode);
      }
      c = getc(f);
      if (c == 0x80) {
         while (1) {
            c = getc(f);
            switch(c) {
case 1:
case 2:
               len = getc(f);
               len += getc(f) * 256L;
               len += getc(f) * 65536L;
               len += getc(f) * 256L * 65536;
               if (c == 1) {
                  while (len > 0) {
                     c = getc(f);
                     if (c == EOF) {
                        error("premature EOF in MS-DOS font file");
                        len = 0;
                     } else {
		        if (c == '\r') { /* Mac- or DOS-style text file */
                           putc('\n', bitfile);
			   if ((c = getc(f)) == '\n') /* DOS-style text */
			      len--; /* consume, but don't generate NL */
			   else
			      ungetc(c, f);
			}
                        else
                           putc(c, bitfile);
                        len--;
                     }
                  }
               } else {
                  putc('\n', bitfile);
                  prevc = 0;
                  while (len > 0) {
                     c = getc(f);
                     if (c == EOF) {
                        error("premature EOF in MS-DOS font file");
                        len = 0;
                     } else {
                        putc(hxdata[c >> 4], bitfile);
                        putc(hxdata[c & 15], bitfile);
                        len--;
                        prevc += 2;
                        if (prevc >= 76) {
                           putc('\n', bitfile);
                           prevc = 0;
                        }
                     }
                  }
               }
               break;
case 3:
               goto msdosdone;
default:
               error("saw type other than 1, 2, or 3 in MS-DOS font file");
               break;
            }
            c = getc(f);
            if (c == EOF)
               break;
            if (c != 0x80) {
               error("saw non-MSDOS header in MSDOS font file");
               break;
            }
         }
msdosdone:
         prevc = 0;
      } else {
/* begin DOS EPS code */
         if (c == 'E'+0x80) {
            if ((getc(f)=='P'+0x80) && (getc(f)=='S'+0x80)
	                            && (getc(f)=='F'+0x80)) {
               doseps = 1;
               dosepsbegin = getc(f);
               dosepsbegin += getc(f) * 256L;
               dosepsbegin += getc(f) * 65536L;
               dosepsbegin += getc(f) * 256L * 65536;
               dosepsend = getc(f);
               dosepsend += getc(f) * 256L;
               dosepsend += getc(f) * 65536L;
               dosepsend += getc(f) * 256L * 65536;
               fseek(f, dosepsbegin, 0);
               c = getc(f);
               dosepsend--;
            }
            else {
               rewind(f);
               c = getc(f);
            }
         }
/* end DOS EPS code */
         if (c != EOF) {
            while (1) {
               if (c == '\n') { /* end or beginning of line; check DSC */
                  *dscLinePointer = 0; /* make sure we terminate!
                                         * might be a new empty line! */
                  if (strncmp(possibleDSCLine, "%%BeginBinary:", 14) == 0 ||
                      strncmp(possibleDSCLine, "%%BeginData:", 12) == 0 ||
                      strncmp(possibleDSCLine, "%%BeginFont:", 12) == 0) {
                     integer size = 0;
                     char *p = possibleDSCLine;
                     *dscLinePointer = 0;
                     *dscLineEnd = 0;
                     if (scanForEnd == 0 && removecomments) {
                        fputs(possibleDSCLine, bitfile);
                        putc('\n', bitfile);
                     }

                     if (strncmp(possibleDSCLine, "%%BeginFont:", 12) == 0) {

                       /* Theoretically, we could wait until we see
                          the "currentfile eexec" and possibly even
                          check that the following data really looks
                          like binary before we begin verbatim
                          copying, but that would complicate the code
                          beyond our present needs.  If we were going
                          to do that much work, this entire chunk of
                          code should probably be
                          rewritten. [dmj@ams.org, 2007/08/20] */

                       scanForEnd = "%%EndFont";
                       scanningFont = 1;
                     } else {
                       scanningFont = 0;

                     scanForEnd = 0;
                     while (*p != ':')
                        p++;
                     p++;
                     while (*p && *p <= ' ')
                        p++;
                     if ('0' > *p || *p > '9') {
                        /*
                         *   No byte count!  We need to scan for end binary
                         *   or end data, and hope we get it right.  Really
                         *   the file is malformed.
                         */
                        scanForEnd = "Yes";
                     }
                     while ('0' <= *p && *p <= '9') {
                        size = size * 10 + *p - '0';
                        p++;
                     }
                     while (*p && *p <= ' ')
                        p++;
                     if (*p == 'h' || *p == 'H')
                        /*
                         *   Illustrator 8 and 9 have bogus byte counts
                         *   for hex data.  But if it is hex, we assume
                         *   that it is safe to use ASCII scanning, so
                         *   we do so.
                         */
                        scanForEnd = "Yes";
                     while (*p > ' ') /* ignore Hex/Binary/ASCII */
                        p++;
                     while (*p && *p <= ' ')
                        p++;
                     putc(c, bitfile);
                     if (c == '\r') { /* maybe we have a DOS-style text file */
                        c = getc(f);
                        if (c == '\n') {
                           putc(c, bitfile);
                           dosepsend--;
                        } else
                           ungetc(c, f);
                     }
                     if (scanForEnd != 0) {
                        if (strncmp(possibleDSCLine, "%%BeginBinary", 13) == 0)
                           scanForEnd = "%%EndBinary";
                        else
                           scanForEnd = "%%EndData";
                     }
                     }
                     if (scanForEnd == 0) {
                        if (strncmp(p, "lines", 5) != 0 &&
                            strncmp(p, "Lines", 5) != 0) {
                           for (; size>0; size--) {
                              c = getc(f);
                              dosepsend--;
                              if (c == EOF)
                                 error(
                                 "! premature end of file in binary section");
                              putc(c, bitfile);
                           }
                        } else {
                           /*
                            *  Count both newlines and returns, and when either
                            *  goes over the count, we are done.
                            */
                           int newlines=0, returns=0;
                           while (newlines < size && returns < size) {
                              c = getc(f);
                              dosepsend--;
                              if (c == EOF)
                                 error(
                                    "! premature end of file in binary section");
                              putc(c, bitfile);
                              if (c == '\n')
                                 newlines++;
                              else if (c == '\r')
                                 returns++;
                           }
                           /*
                            *   If we've seen precisely one too few newlines,
                            *   and the next char is a newline, include it too.
                            */
                           if (returns == newlines + 1) {
                              if ((c = getc(f)) == '\n') {
                                 putc(c, bitfile);
                                 dosepsend--;
                              } else {
                                 ungetc(c, f);
                              }
                           }
                        }
                        c = getc(f);
                        dosepsend--;
                        if (c == '\n' || c == '\r') {
                           putc(c, bitfile);
			   if (c == '\r') { /* DOS-style text file? */
			      c = getc(f);
                              dosepsend--;
			      if (c == '\n') {
			         putc(c, bitfile);
			         c = getc(f);
                                 dosepsend--;
			      }
			   } else {
			      c = getc(f);
                              dosepsend--;
                           }
                        }
                        if (c != '%') {
                           /*   try to find %%EndBinary or %%EndData anywhere
                                in the rest of the file, and pretend it
                                worked; this works around various Illustrator
                                bugs.   -tgr, 14 June 2003                  */
                           const char *m1 = "%%EndData";
                           const char *m2 = "%%EndBinary";
                           const char *p1 = m1;
                           const char *p2 = m2;
                           error(
               " expected to see %%EndBinary at end of data; struggling on");
                           while (1) {
                              putc(c, bitfile);
                              if (c == '\r' || c == '\n') {
			         if (c == '\r') { /* DOS-style text file? */
			            c = getc(f);
			            if (c != '\n')
				       ungetc(c, f);
                                    else
                                       dosepsend--;
			         }
                                 break;
			      }
                              c = getc(f);
                              dosepsend--;
                              if (c == EOF)
                                 error(
                                 "! premature end of file in binary section");
 /*
  *   By the way, this code can be fooled by things like %%%EndBinary
  *   or even %%EndBi%%EndBinary, but this isn't valid DSC anyway.
  *   This comment is mainly here to prevent anyone else from emulating
  *   this code when doing stream-based substring matching.
  */
                              if (c == *p1) {
                                 p1++;
                                 if (*p1 == 0)
                                    break;
                              } else {
                                 p1 = m1;
                              }
                              if (c == *p2) {
                                 p2++;
                                 if (*p2 == 0)
                                    break;
                              } else {
                                 p2 = m2;
                              }
                           }
                        }
                        while (1) {
                           putc(c, bitfile);
                           if (c == '\r' || c == '\n') {
			      if (c == '\r') { /* DOS-style text file? */
			         c = getc(f);
			         if (c != '\n')
				    ungetc(c, f);
                                 else {
                                    putc(c, bitfile);
                                    dosepsend--;
                                 }
			      }
                              break;
			   }
                           c = getc(f);
                           removingBytes = 0;
                           dosepsend--;
                           if (c == EOF)
                              error(
                                 "! premature end of file in binary section");
                        }
                        c = getc(f);
                        dosepsend--;
                     }
                  } else if (scanForEnd && strncmp(possibleDSCLine, scanForEnd,
                                                   strlen(scanForEnd))==0) {
                     scanForEnd = 0;
                     scanningFont = 0;
                  }
                  dscLinePointer = possibleDSCLine;
               } else if (dscLinePointer < dscLineEnd) {
                  *dscLinePointer++ = c;
                  if (removecomments && scanForEnd == 0 &&
                      c == '%' && dscLinePointer == possibleDSCLine + 1) {
                     /* % is first char */
                     c = getc(f);
                     if (c == '%' || c == '!')
                        removingBytes = 1;
                     if (c != EOF)
                        ungetc(c, f);
                     c = '%';
                  }
               }
#if defined(VMCMS) || defined (MVSXA)
               if (c != 0x37  || scanningFont) {
#else
               if (c != 4 || scanningFont) {
#endif
                  if (!removingBytes)
                     putc(c, bitfile);
               }
               prevc = c;
/* begin DOS EPS code */
               if (doseps && (dosepsend <= 0))
                  break;      /* stop at end of DOS EPS PostScript section */
/* end DOS EPS code */
               c = getc(f);
               dosepsend--;
               if (c == EOF)
                  break;
               else if (c == '\r' && ! scanningFont) {
		  c = getc(f);
		  if (c == '\n') { /* DOS-style text file? */
		     if (!removingBytes) putc('\r', bitfile);
                     dosepsend--;
		  } else
		     ungetc(c, f);
                  c = '\n';
	       }
               if (prevc == '\n')
                  removingBytes = 0;
            }
         }
      }
      if (prevc != '\n')
         putc('\n', bitfile);
      linepos = 0;
#ifndef VMCMS
#ifndef MVSXA
#ifndef VMS
#if !defined(MSDOS) || defined(__DJGPP__)
#ifndef ATARIST
#ifndef __THINK__
      if (infigure == 2)
#ifdef OS2
         {
            if (_osmode == OS2_MODE)
               pclose(f);
         }
#else
         pclose(f);
#endif
      else
#endif
#endif
#endif
#endif
#endif
#endif
         fclose(f);
      if (cur_header && cur_header->postcode) {
         fprintf(bitfile, "\n%s", cur_header->postcode);
         free(cur_header->postcode);
      }
      if (!disablecomments) {
         if (infigure)
            fprintf(bitfile, "\n%%%%EndDocument\n");
         else if (infont)
            fprintf(bitfile, "\n%%%%EndFont\n");
#ifdef HPS
         else if (noprocset) {}
#endif
         else
            fprintf(bitfile, "\n%%%%EndProcSet\n");
      }
   }
}

void
copyfile(const char *s)
{
   copyfile_general(s, NULL);
}

/*
 *   For included PostScript graphics, we use the above routine, but
 *   with no fatal error message.
 */
void
figcopyfile(char *s, int systemtype)
{
   infigure = systemtype ? 2 : 1;
   copyfile(s);
   infigure = 0;
}
/*
 *   This next routine writes out a `special' character.  In this case,
 *   we simply put it out, since any special character terminates the
 *   preceding token.
 */
void
specialout(char c)
{
   if (linepos >= LINELENGTH) {
      putc('\n', bitfile);
      linepos = 0;
   }
   putc(c, bitfile);
   linepos++;
   lastspecial = 1;
}

void
stringend(void)
{
   if (linepos + instring >= LINELENGTH - 2) {
      putc('\n', bitfile);
      linepos = 0;
   }
   putc('(', bitfile);
   *strbp = 0;
   fputs(strbuffer, bitfile);
   putc(')', bitfile);
   linepos += instring + 2;
   lastspecial = 1;
   instring = 0;
   strbp = strbuffer;
}

#ifdef SHIFTLOWCHARS
/*
 *   moving chars 0-32 and 127 to higher positions
 *   is desirable when using some fonts
 */
int
T1Char(int c)
{
  int tmpchr = c;
  if (shiftlowchars && curfnt->resfont) {
    if ((tmpchr <= 0x20)&&(tmpchr>=0)) {
      if (tmpchr < 0x0A) {
        tmpchr += 0xA1;
      }
      else {
        tmpchr += 0xA3;
      }
    }
    else if (tmpchr == 0x7F) {
      tmpchr = 0xC4;
    }
  }
  if (curfnt->chardesc[tmpchr].flags2 & EXISTS)
    tmpchr = c;
  return tmpchr;
}
#endif

void
scout(unsigned char c)   /* string character out */
{
/*
 *   Is there room in the buffer?  LINELENGTH-6 is used because we
 *   need room for (, ), and a possible four-byte string \000, for
 *   instance.  If it is too long, we send out the string.
 */
   jflag = 0;
   if (instring > LINELENGTH-6) {
      stringend();
      chrcmd('p');
   }
#ifdef SHIFTLOWCHARS
   c=T1Char(c);
#endif
/*  changed next line to hex representation for VMCMS port
   if (c<' ' || c > 126 || c=='%' ) {
*/
   if ( c<0x20 || c>= 0x7F || c==0x25 ) {
      *strbp++ = '\\';
      *strbp++ = '0' + ((c >> 6) & 3);
      *strbp++ = '0' + ((c >> 3) & 7);
      *strbp++ = '0' + (c & 7);
      instring += 4;
   } else {
#if defined(VMCMS) || defined (MVSXA)
     c = ascii2ebcdic[c];
#endif
     if (c == '(' || c == ')' || c == '\\') {
       *strbp++ = '\\';
       *strbp++ = c;
       instring += 2;
     } else {
       *strbp++ = c;
       instring++;
     }
   }
}

static void
scout2(int c)
{
   char s[64];

   sprintf(s, "<%04x>p", c);
   cmdout(s);
}

static void
jscout(int c, char *fs)   /* string character out */
{
   char s[64];

   if (!dir) {
      numout(hh);
      numout(vv);
   } else {
      numout(vv);
      numout(-hh);
   }
   if (strstr(fs,"-UTF32-")!=NULL) {
      snprintf(s, sizeof(s), "a<%08x>p", c);
   } else if (strstr(fs,"-UTF8-")!=NULL) {
      if (c<0x80) {
         snprintf(s, sizeof(s), "a<%02x>p", c);
      } else if (c<0x800) {
	 snprintf(s, sizeof(s), "a<%02x%02x>p", UCStoUTF8B1(c), UCStoUTF8B2(c));
      } else if (c<0x10000) {
	 snprintf(s, sizeof(s), "a<%02x%02x%02x>p", UCStoUTF8C1(c),
                 UCStoUTF8C2(c), UCStoUTF8C3(c));
      } else if (c<0x110000) {
	 snprintf(s, sizeof(s), "a<%02x%02x%02x%02x>p", UCStoUTF8D1(c),
		 UCStoUTF8D2(c), UCStoUTF8D3(c), UCStoUTF8D4(c));
      } else {
         error("warning: Illegal code value.");
      }
   } else if (c>0xffff && strstr(fs,"-UTF16-")!=NULL) {
      snprintf(s, sizeof(s), "a<%04x%04x>p",
	       UTF32toUTF16HS(c), UTF32toUTF16LS(c));
   } else {
      if ((strstr(fs,"-RKSJ-")!=NULL)) c = JIStoSJIS(c);
      snprintf(s, sizeof(s), "a<%04x>p", c);
   }
   cmdout(s);
   instring = 0;
   jflag = 1;
   strbuffer[0] = '\0';
}

void
cmdout(const char *s)
{
   int l;

   /* hack added by dorab */
   if (instring && !jflag) {
        stringend();
        chrcmd('p');
   }
   l = strlen(s);
   if ((! lastspecial && linepos >= LINELENGTH - 20) ||
           linepos + l >= LINELENGTH) {
      putc('\n', bitfile);
      linepos = 0;
   } else if (! lastspecial) {
      putc(' ', bitfile);
      linepos++;
   }
   fputs(s, bitfile);
   linepos += l;
   lastspecial = 0;
}

void psnameout(const char *s) {
   // we lead with a special, so we don't need the space.
   lastspecial = 1 ;
   cmdout(s) ;
}

void pslineout(const char *s) {
   fputs(s, bitfile) ;
   fprintf(bitfile, "\n");
   linepos = 0;
}

static void
chrcmd(char c)
{
   if ((! lastspecial && linepos >= LINELENGTH - 20) ||
       linepos + 2 > LINELENGTH) {
      putc('\n', bitfile);
      linepos = 0;
   } else if (! lastspecial) {
      putc(' ', bitfile);
      linepos++;
   }
   putc(c, bitfile);
   linepos++;
   lastspecial = 0;
}

void
floatout(float n)
{
   char buf[50];

   snprintf(buf, sizeof(buf), "%.2f", n);
   cmdout(buf);
}

void
doubleout(double n)
{
   char buf[50];

   snprintf(buf, sizeof(buf), "%g", n);
   cmdout(buf);
}

void
numout(integer n)
{
   char buf[50];

#ifdef SHORTINT
   snprintf(buf, sizeof(buf), "%ld", n);
#else
   snprintf(buf, sizeof(buf), "%d", n);
#endif
   cmdout(buf);
}

void
mhexout(register unsigned char *p,
        register long len)
{
   register const char *hexchar = hxdata;
   register int n, k;

   while (len > 0) {
      if (linepos > LINELENGTH - 2) {
         putc('\n', bitfile);
         linepos = 0;
      }
      k = (LINELENGTH - linepos) >> 1;
      if (k > len)
         k = len;
      len -= k;
      linepos += (k << 1);
      while (k--) {
         n = *p++;
         putc(hexchar[n >> 4], bitfile);
         putc(hexchar[n & 15], bitfile);
      }
   }
}

static void
fontout(halfword n)
{
   char buf[6];

   if (instring && !jflag) {
      stringend();
      chrcmd('p');
   }
   makepsname(buf, n);
   cmdout(buf);

   lastfont = curfnt->psname;
}

void
hvpos(void)
{
   if (!dir) {
      if (rvv != vv || jflag) {
         if (instring) {
            stringend();
            numout(hh);
            numout(vv);
            chrcmd('y');
         } else if (rhh != hh || jflag) {
            numout(hh);
            numout(vv);
            chrcmd('a') ;
         } else { /* hard to get this case, but it's there when you need it! */
            numout(vv - rvv);
            chrcmd('x');
         }
         rvv = vv;
      } else if (rhh != hh || jflag) {
         if (instring) {
            stringend();
            if (hh - rhh < 5 && rhh - hh < 5) {
#if defined(VMCMS) || defined (MVSXA) /*  should replace 'p' in non-VMCMS, non-MVSXA line as well */
               chrcmd(ascii2ebcdic[(char)(112 + hh - rhh)]);
#else
               chrcmd((char)('p' + hh - rhh));
#endif
            } else if (hh - rhh < d + 5 && rhh - hh < 5 - d) {
#if defined(VMCMS) || defined (MVSXA) /* should replace 'g' in non-VMCMS, non-MVSXA line as well  */
               chrcmd(ascii2ebcdic[(char)(103 + hh - rhh - d)]);
#else
               chrcmd((char)('g' + hh - rhh - d));
#endif
               d = hh - rhh;
            } else {
               numout(hh - rhh);
               chrcmd('b');
               d = hh - rhh;
            }
         } else {
            numout(hh - rhh);
            chrcmd('w');
         }
      }
      rhh = hh;
   } else {
      if (rhh != hh || jflag) {
         if (instring) {
            stringend();
            numout(vv);
            numout(-hh);
            chrcmd('y');
         } else if (rvv != vv || jflag) {
            numout(vv);
            numout(-hh);
            chrcmd('a');
         } else { /* hard to get this case, but it's there when you need it! */
            numout(rhh - hh);
            chrcmd('x');
         }
         rhh = hh;
      } else if (rvv != vv || jflag) {
         if (instring) {
            stringend();
            if (vv - rvv < 5 && rvv - vv < 5) {
#if defined(VMCMS) || defined (MVSXA) /*  should replace 'p' in non-VMCMS, non-MVSXA line as well */
               chrcmd(ascii2ebcdic[(char)(112 + vv - rvv)]);
#else
               chrcmd((char)('p' + vv - rvv));
#endif
            } else if (vv - rvv < d + 5 && rvv - vv < 5 - d) {
#if defined(VMCMS) || defined (MVSXA) /* should replace 'g' in non-VMCMS, non-MVSXA line as well  */
               chrcmd(ascii2ebcdic[(char)(103 + vv - rvv - d)]);
#else
               chrcmd((char)('g' + vv - rvv - d));
#endif
               d = vv - rvv;
            } else {
               numout(vv - rvv);
               chrcmd('b');
               d = vv - rvv;
            }
         } else {
            numout(vv - rvv);
            chrcmd('w');
         }
      }
      rvv = vv;
   }
}

/*
 *   initprinter opens the bitfile and writes the initialization sequence
 *   to it.
 */
void
newline(void)
{
   if (linepos != 0) {
      fprintf(bitfile, "\n");
      linepos = 0;
   }
   lastspecial = 1;
}

void
nlcmdout(const char *s)
{
   newline();
   cmdout(s);
   newline();
}
/*
 *   Is the dimension close enough for a match?  We use 5bp
 *   as a match; this is 65536*72.27*5/72 or 328909 scaled points.
 */
static int
indelta(integer i)
{
   if (i < 0)
      i = -i;
   return (i <= 328909);
}
/*
 *   A case-irrelevant string compare.
 */
static int
mlower(int c)
{
   if ('A' <= c && c <= 'Z')
      return c - 'A' + 'a';
   else
      return c;
}
static int
ncstrcmp(const char *a, const char *b)
{
   while (*a && (*a == *b ||
                       mlower(*a) == mlower(*b)))
      a++, b++;
   if (*a == 0 && *b == 0)
      return 0;
   else
      return 1;
}
/*
 *   Find the paper size.
 */
static void
findpapersize(void) {
   if (finpapsiz == 0) {
      struct papsiz *ps;
      struct papsiz *fps = 0;
      int    ih, iv, it;
      int    mindiff = 0x7fffffff;

      if (tryepsf && !landscape) {
         finpapsiz = &defpapsiz;
         hpapersize = defpapsiz.xsize;
         vpapersize = defpapsiz.ysize;
         return;
      }
      if (cropmarks) {
/*
 *   If user wanted crop marks, we increase the size of the page by
 *   a half inch all around.
 */
         if (hpapersize == 0 || vpapersize == 0) {
            error(
 "warning: -k crop marks wanted, but no paper size specified; using default");
            if (landscape) {
               hpapersize = defpapsiz.ysize;
               vpapersize = defpapsiz.xsize;
            } else {
               hpapersize = defpapsiz.xsize;
               vpapersize = defpapsiz.ysize;
            }
         }
         hpapersize += 2368143L;
         vpapersize += 2368143L;
         add_header(CROPHEADER);
      }
      if (paperfmt && *paperfmt) {
         for (ps = papsizes; ps; ps = ps->next)
            if (ncstrcmp(paperfmt, ps->name)==0)
               finpapsiz = ps;
         if (finpapsiz == 0)
            error("no match for papersize");
      }
      if (finpapsiz == 0 && hpapersize > 0 && vpapersize > 0) {
         for (ps=papsizes; ps; ps = ps->next) {
            ih = ps->xsize-hpapersize;
            iv = ps->ysize-vpapersize;
            if (ih < 0) ih = -ih;
            if (iv < 0) iv = -iv;
            it = ih;
            if (it < iv) it = iv;
            if (it < mindiff) {
               mindiff = it;
               fps = ps;
            }
         }
         if (indelta(mindiff))
            landscape = 0;
         else
            fps = 0;
         mindiff = 0x7fffffff;
         if (fps == 0) {
            for (ps=papsizes; ps; ps = ps->next) {
               iv = ps->ysize-hpapersize;
               ih = ps->xsize-vpapersize;
               if (ih < 0) ih = -ih;
               if (iv < 0) iv = -iv;
               it = ih;
               if (it < iv) it = iv;
               if (it < mindiff) {
                  mindiff = it;
                  fps = ps;
               }
            }
            if (indelta(mindiff))
               landscape = 1;
            else
               fps = 0;
            if (fps == 0) {
               for (ps=papsizes; ps; ps = ps->next) {
                  if (ps->ysize == 0 && ps->xsize == 0) {
                     fps = ps;
                     break;
                  }
               }
               if (fps == 0) {
                  landscape = (hpapersize > vpapersize);
                  error(
                    "no match for special paper size found; using default");
               }
            }
         }
#if defined(MIKTEX)
         if (miktex_no_landscape && landscape)
         {
           landscape = 0;
         }
#endif
         finpapsiz = fps;
      }
      if (finpapsiz == 0) {
         if (papsizes)
            finpapsiz = papsizes;
         else
            finpapsiz = &defpapsiz;
/*
 *   But change xsize/ysize to match so bounding box works.
 */
         if (hpapersize && vpapersize) {
            if (landscape) {
               finpapsiz->ysize = hpapersize;
               finpapsiz->xsize = vpapersize;
            } else {
               finpapsiz->xsize = hpapersize;
               finpapsiz->ysize = vpapersize;
            }
         }
      }
/*
 *   Here, there was no papersize special.  We set the paper size from
 *   the selected paper format.  If the selected paper format has no
 *   sizes, we use the defaults.
 */
      if (hpapersize == 0 || vpapersize == 0) {
         if (finpapsiz->xsize == 0 || finpapsiz->ysize == 0) {
            finpapsiz->xsize = defpapsiz.xsize;
            finpapsiz->ysize = defpapsiz.ysize;
         }
         if (landscape) {
            vpapersize = finpapsiz->xsize;
            hpapersize = finpapsiz->ysize;
         } else {
            hpapersize = finpapsiz->xsize;
            vpapersize = finpapsiz->ysize;
         }
/*
 *   Here, there was a papersize special, but the selected paper
 *   format has 0 0 for sizes.  We set the sizes here so that the
 *   bounding box works.
 */
      } else if (finpapsiz->xsize == 0 || finpapsiz->ysize == 0) {
         finpapsiz->xsize = hpapersize;
         finpapsiz->ysize = vpapersize;
/*
 *   Here, the user specified a size with -t, and there was a
 *   papersize special, and its sizes were greater than zero.
 *   We make sure the sizes are okay.  Note that the user must have
 *   specified landscape if this is desired.
 */
      } else if (paperfmt && *paperfmt) {
         if (landscape) {
            if (!indelta(vpapersize - finpapsiz->xsize) ||
                !indelta(hpapersize - finpapsiz->ysize)) {
               if (vpapersize > finpapsiz->xsize ||
                   hpapersize > finpapsiz->ysize)
                  error("warning: -t selected paper may be too small");
               else
                  error("note: -t selected paper may be too large");
            }
         } else {
            if (!indelta(hpapersize - finpapsiz->xsize) ||
                !indelta(vpapersize - finpapsiz->ysize)) {
               if (hpapersize > finpapsiz->xsize ||
                   vpapersize > finpapsiz->ysize)
                  error("warning: -t selected paper may be too small");
               else
                  error("note: -t selected paper may be too large");
            }
         }
      }
   }
}
/*
 *   Convert scaled points to PostScript points.  This is the same
 *   as return (i * 72 / (65536 * 72.27)), which is the same as
 *   dividing by 65781.76, but we want to round up.
 */
static int
topoints(integer i)
{
   i += 65780L;
   return (i / 6578176L)*100 + (i % 6578176) * 100 / 6578176;
}
/*
 *   Send out the special paper stuff.  If `hed' is non-zero, only
 *   send out lines starting with `!' else send all other lines out.
 */
static void
paperspec(const char *s, int hed)
{
   int sendit;

   while (*s) {
      s++;
      if (*s == '\0')
         return;
      if (*s == '!') {
         s++;
         while (*s == ' ') s++;
         sendit = hed;
      } else
         sendit = ! hed;
      if (sendit) {
         while (*s && *s != '\n')
            putc(*s++, bitfile);
         putc('\n', bitfile);
      } else {
         while (*s && *s != '\n')
            s++;
      }
   }
}
static char *
epsftest(integer bop)
{
   if (tryepsf && paperfmt == 0 && *iname) {
      findbb(bop+44);
      return nextstring;
   }
   return 0;
}
static char *isepsf = 0;
static int endprologsent;
void
open_output(void) {
   FILE * pf = NULL;
   if (*oname != 0) {
/*
 *   We check to see if the first character is a exclamation
 *   point, and popen if so.
 */
      if (*oname == '!' || *oname == '|') {
#if defined(MSDOS) && !defined(__DJGPP__) || defined(VMS) || defined(VMCMS) || defined(MVSXA) || defined(__THINK__) || defined(ATARIST)
            error("! can't open output pipe");
#else
#ifdef OS2
         if (_osmode != OS2_MODE) {
            error("! can't open output pipe");
         } else {
#endif
#ifdef __DJGPP__
        /* Feature: if they pipe to "lpr" and there's no executable by
           that name anywhere in sight, write to local printer instead.

           We do this up front, before even trying to popen, because on
           MS-DOS popen always succeeds for writing (it only opens a
           temporary file), and by the time we get to actually run the
           (possibly nonexistent) program in pclose, it's too late to
           fall back.

           We don't use kpathsea functions here because they don't
           know about DOS-specific executable extensions, while we
           want to be able to find "lpr.exe", "lpr.com", "lpr.bat" etc.  */
        extern char *__dosexec_find_on_path(const char *,
                                            char **, char *);
        extern char **environ;
        char *p = oname + 1;
        char found[FILENAME_MAX];

        while (ISSPACE(*p))
          p++;
        if (strncmp(p, "lpr", 3) == 0 && (ISSPACE(p[3]) || p[3] == '\0')
            && !__dosexec_find_on_path(oname+1, (char **)0, found)
            && !__dosexec_find_on_path(oname+1, environ, found))
           pf = fopen("PRN", "w");
#endif
	 if (pf == NULL && (pf = popen(oname+1, "w")) != NULL) {
	    popened = 1;
	    SET_BINARY(fileno(pf));
	 }
         if (pf == NULL)
            error("! couldn't open output pipe");
	 bitfile = pf;
#ifdef OS2
         }
#endif
#endif /* MSDOS && !__DJGPP__ || VMS || ... */
      } else {
#if defined(MIKTEX)
        if ((bitfile = fopen(oname, "wb")) == nullptr)
        {
          error("! couldn't open PostScript file");
        }
#else
         if ((bitfile=fopen(oname,"w"))==NULL)
            error("! couldn't open PostScript file");
         SET_BINARY(fileno(bitfile));
#endif
      }
   } else {
      bitfile = stdout;
   }

   /* Even PostScript output may include binary characters, so switch
      bitfile to binary mode.  */
   if (O_BINARY && !isatty(fileno(bitfile)))
      SET_BINARY(fileno(bitfile));
}
void
initprinter(sectiontype *sect)
{
   int n = sect->numpages * pagecopies * collatedcopies;
#ifdef HPS
   if (!HPS_FLAG)
#endif
      open_output();

   findpapersize();
   if (disablecomments) {
      fprintf(bitfile,
             "%%!PS (but not EPSF; comments have been disabled)\n");
      fprintf(stderr, "Warning:  no %%%%Page comments generated.\n");
   } else {
      if (multiplesects) {
         fprintf(bitfile,
             "%%!PS (but not EPSF because of memory limits)\n");
         fprintf(stderr, "Warning:  no %%%%Page comments generated.\n");
      } else {
         isepsf = epsftest(sect->bos);
         if (isepsf)
            fprintf(bitfile, "%%!PS-Adobe-2.0 EPSF-2.0\n");
         else
            fprintf(bitfile, "%%!PS-Adobe-2.0\n");
      }
      if (tryepsf && isepsf == 0)
         error("We tried, but couldn't make it EPSF.");
      fprintf(bitfile, "%%%%Creator: %s\n", banner + 8);
      if (*iname)
         fprintf(bitfile, "%%%%Title: %s\n", iname);
#ifdef CREATIONDATE
      jobtime = get_unique_time_if_given();
      if (jobtime == INVALID_EPOCH_VALUE) {
         jobtime = time(0);
      }
      fprintf(bitfile, "%%%%CreationDate: %s", asctime(gmtime(&jobtime)));
#endif
      if (! isepsf) {
/*
 *   Normally, we wouldn't want to add that second field
 *   indicating that the page order is reversed, as per page
 *   644 of the Red book.  But we have to, for many existing
 *   spoolers.
 */
        fprintf(bitfile, "%%%%Pages: %d%s\n", (sepfiles ? n : totalpages),
                                                    (reverse?" -1":""));
        fprintf(bitfile, "%%%%PageOrder: %sscend\n", reverse?"De":"A");
      }
      if (landscape) {
         fprintf(bitfile, "%%%%Orientation: Landscape\n");
         fprintf(bitfile, "%%%%BoundingBox: 0 0 %d %d\n",
              topoints(finpapsiz->xsize), topoints(finpapsiz->ysize));
      } else if (isepsf)
         fprintf(bitfile, "%%%%BoundingBox: %s\n", isepsf);
      else
         fprintf(bitfile, "%%%%BoundingBox: 0 0 %d %d\n",
              topoints(finpapsiz->xsize), topoints(finpapsiz->ysize));
      tell_needed_fonts();
      paperspec(finpapsiz->specdat, 1);
      fprintf(bitfile, "%%%%EndComments\n");
/*
 *   If we encode Type 3 fonts with an encoding vector, this can cause
 *   Distiller's autoorientation to get confused.  We remedy this by
 *   emitting underdocumented ViewingOrientation comments right after
 *   EndComments.  Known defect: if a user "flips" the landscape to be
 *   180 degrees using one of the \special{} commands available, the
 *   document will be rendered in the viewer upside down.  (But only
 *   with bitmap font encoding enabled and bitmapped fonts actually used.)
 *   --tgr, 29 February 2020.
 */
      if (encodetype3 && bitmapfontseen) {
         fprintf(bitfile, "%%%%BeginDefaults\n") ;
         if (landscape) {
            fprintf(bitfile, "%%%%ViewingOrientation: 0 -1 1 0\n") ;
         } else {
            fprintf(bitfile, "%%%%ViewingOrientation: 1 0 0 1\n") ;
         }
         fprintf(bitfile, "%%%%EndDefaults\n") ;
      }
   }
   {
      int i, len;
      char *p;

/*
 *   Here, too, we have to be careful not to exceed the line length
 *   limitation, if possible.
 */
      fprintf(bitfile, "%%DVIPSWebPage: %s\n", banner2);
      fprintf(bitfile, "%%DVIPSCommandLine:");
      len = 18;
      for (i=0; i<gargc; i++) {
         p = gargv[i];
         while (*p > ' ')
            p++;
         if (*p)
            len += 2;
         len += strlen(gargv[i]) + 1;
         if (len > LINELENGTH) {
            fprintf(bitfile, "\n%%+");
            len = strlen(gargv[i]) + 3;
            if (*p)
               len += 2;
         }
         fprintf(bitfile, (*p ? " \"%s\"" : " %s"), gargv[i]);
      }
      fprintf(bitfile, "\n%%DVIPSParameters: dpi=%d", actualdpi);
      if (actualdpi != vactualdpi)
         fprintf(bitfile, "x%d", vactualdpi);
      if (compressed)
         fprintf(bitfile, ", compressed");
      if (removecomments)
         fprintf(bitfile, ", comments removed");
      fputc('\n', bitfile);
   }
#if defined(VMCMS) || defined (MVSXA)  /* convert preamblecomment to ebcdic so we can read it */
   {
      int i;
      for ( i=0; preamblecomment[i]; i++ )
          preamblecomment[i] = ascii2ebcdic[preamblecomment[i]];
   }
#endif
   fprintf(bitfile, "%%DVIPSSource: %s\n", preamblecomment);
   linepos = 0;
   endprologsent = 0;
   if (safetyenclose)
      fprintf(bitfile, "/SafetyEnclosure save def\n");
   print_composefont();
   if (! headers_off)
      send_headers();
}
void
setup(void) {
   newline();
   if (endprologsent == 0 && !disablecomments) {
      fprintf(bitfile, "%%%%EndProlog\n");
      fprintf(bitfile, "%%%%BeginSetup\n");
      if (vactualdpi == actualdpi)
         fprintf(bitfile, "%%%%Feature: *Resolution %ddpi\n",
                                           actualdpi);
      else
         fprintf(bitfile, "%%%%Feature: *Resolution %dx%ddpi\n",
                                           actualdpi, vactualdpi);
      if (multiplesects && *(finpapsiz->specdat)) {
         fprintf(bitfile, "TeXDict begin\n");
         paperspec(finpapsiz->specdat, 0);
         fprintf(bitfile, "end\n");
      }
      if (manualfeed)
         fprintf(bitfile, "%%%%Feature: *ManualFeed True\n");
#ifdef HPS
      if (!HPS_FLAG)
#endif
      if (multiplesects)
         fprintf(bitfile, "%%%%EndSetup\n");
   }
   if (multiplesects && ! disablecomments)
      fprintf(bitfile, "%%DVIPSBeginSection\n");
   cmdout("TeXDict");
   cmdout("begin");
   if (endprologsent || disablecomments || multiplesects == 0) {
      fprintf(bitfile, "\n");
      paperspec(finpapsiz->specdat, 0);
   }
   if (manualfeed) cmdout("@manualfeed");
   if (landscape) cmdout("@landscape");
   if (numcopies != 1) {
      numout((integer)numcopies);
      cmdout("@copies");
   }
   cmdout("end");
   if (endprologsent == 0 && !disablecomments) {
      newline();
      endprologsent = 1;
#ifdef HPS
      if (!HPS_FLAG)
#endif
         if (! multiplesects)
            fprintf(bitfile, "%%%%EndSetup\n");
   }
#ifdef HPS
  if (HPS_FLAG) {
    fclose(bitfile);
    set_bitfile("body.tmp",0);
  }
#endif
}
/*
 *   cleanprinter is the antithesis of the above routine.
 */
void
cleanprinter(void)
{
   fprintf(bitfile, "\n");
   fprintf(bitfile, "userdict /end-hook known{end-hook}if\n");
   if (safetyenclose)
      fprintf(bitfile, "SafetyEnclosure restore\n");
   if (!disablecomments)
      fprintf(bitfile, "%%%%EOF\n");
   if (sendcontrolD)
      putc(4, bitfile);
   if (ferror(bitfile))
      error("Problems with file writing; probably disk full.");
#if !defined(MSDOS) || defined(__DJGPP__)
#ifndef VMS
#ifndef MVSXA
#ifndef VMCMS
#ifndef __THINK__
#ifndef ATARIST
#ifdef OS2
   if (_osmode == OS2_MODE)
#endif
      if (popened)
         pclose(bitfile);
#endif
#endif
#endif
#endif
#endif
#endif
   if (popened == 0)
      fclose(bitfile);
   bitfile = NULL;
}

/* this tells dvips that it has no clue where it is. */
static int thispage = 0;
static integer rulex, ruley;
void
psflush(void) {
   rulex = ruley = rhh = rvv = -314159265;
   lastfont = -1;
}
/*
 *   pageinit initializes the output variables.
 */
void
pageinit(void)
{
   psflush();
   newline();
   thispage++;
   if (!disablecomments) {
      if (multiplesects)
#ifdef SHORTINT
         fprintf(bitfile, "%%DVIPSSectionPage: %ld\n", pagenum);
      else if (! isepsf)
         fprintf(bitfile, "%%%%Page: %ld %d\n", pagenum, thispage);
#else
         fprintf(bitfile, "%%DVIPSSectionPage: %d\n", pagenum);
      else if (! isepsf)
         fprintf(bitfile, "%%%%Page: %d %d\n", pagenum, thispage);
#endif
   }
   linepos = 0;
   cmdout("TeXDict");
   cmdout("begin");
#ifdef HPS
   if (HPS_FLAG) {
      cmdout("HPSdict");
      cmdout("begin");
   }
#endif
   if (landscape) cmdout("@landscape");
   numout((integer)pagenum);
   numout((integer)thispage-1);
   cmdout("bop");
   d = 0;
}



/*
 *   This routine ends a page.
 */
void
pageend(void)
{
   if (instring) {
      stringend();
      chrcmd('p');
   }
   if (dir)
      cmdout("-90 rotate");
   if (any_dir)
      cmdout("dyy");
   cmdout("eop");
   cmdout("end");
#ifdef HPS
   if (HPS_FLAG)
      cmdout("end");
#endif
}

/*
 *   drawrule draws a rule at the specified position.
 *   It does nothing to save/restore the current position,
 *   or even draw the current string.  (Rules are normally
 *   set below the baseline anyway, so this saves us on
 *   output size almost always.)
 */
void
drawrule(integer rw, integer rh)
{
#if defined(MIKTEX)
  if (miktex_no_rules)
  {
    rulex = rw;
    ruley = rh;
    return;
  }
#endif
   if (!dir) {
     numout((integer)hh);
     numout((integer)vv);
   } else {
     numout((integer)vv);
     numout((integer)-hh);
   }
   if (rw == rulex && rh == ruley)
      chrcmd('V');
   else {
      numout((integer)rw);
      numout((integer)rh);
      chrcmd('v');
      rulex = rw;
      ruley = rh;
   }
}

/*
 *   drawchar draws a character at the specified position.
 */
void
drawchar(chardesctype *c, int cc)
{
   if (rdir != dir || fdir != curfnt->dir) {
      if (curfnt->dir == 9)
         setdir(dir+2);
      else
         setdir(dir);
      rdir = dir;
      fdir = curfnt->dir;
   }

   if (curfnt->iswide == 0 && curfnt->codewidth == 2) {
      Boolean savejflag = jflag;
      jflag = 1;
      hvpos();
      jflag = savejflag;
      if (lastfont != curfnt->psname)
         fontout(curfnt->psname);
#if defined(MIKTEX)
      if (!miktex_no_glyphs)
#endif
      scout2(cc);
   }
   else if (curfnt->iswide) {
      if (lastfont != curfnt->psname)
         fontout(curfnt->psname);
#if defined(MIKTEX)
      if (!miktex_no_glyphs)
#endif
      jscout(cc, curfnt->resfont->PSname);
   }
   else {
      if (jflag) {
         if (!dir){
            numout(hh);
            numout(vv);
         }
         else {
            numout(vv);
            numout(-hh);
         }
         chrcmd('a');
         rhh = hh;
         rvv = vv;
      }
      else hvpos();
      if (lastfont != curfnt->psname)
         fontout(curfnt->psname);
      scout((unsigned char)cc);
   }
   if (!dir)
      rhh = hh + c->pixelwidth; /* rvv = rv */
   else
      rvv = vv + c->pixelwidth; /* rhh = rh */
}
/*
 *   This routine sends out the document fonts comment.
 */
static void
tell_needed_fonts(void) {
   struct header_list *hl = ps_fonts_used;
   char *q;
   int roomleft = -1;

   if (hl == 0)
      return;
   while (0 != (q=get_name(&hl))) {
      if ((int)strlen(q) >= roomleft) {
         if (roomleft != -1) {
            fprintf(bitfile, "\n%%%%+");
            roomleft = LINELENGTH - 3;
         } else {
            fprintf(bitfile, "%%%%DocumentFonts:");
            roomleft = LINELENGTH - 16;
         }
      }
      fprintf(bitfile, " %s", q);
      roomleft -= strlen(q) + 1;
   }
   fprintf(bitfile, "\n");
}

static void print_composefont(void)
{
   struct header_list *hl = ps_fonts_used;
   int  len;
   char *q, *p;

   if (hl == 0)
      return;
   while (0 != (q=get_name(&hl))) {
     len = strlen(q);
     if(len > 11 && (!strncmp(q+len-10, "Identity-H", 10) ||
             !strncmp(q+len-10, "Identity-V", 10))) {
       fprintf(bitfile, "%%%%BeginFont: %s\n", q);
       fprintf(bitfile, "/%s ", q);
       fprintf(bitfile, "/%s ", q+len-10);
       fprintf(bitfile, "[/");
       for(p=q; p <= q+len-12; p++)
          fprintf(bitfile, "%c", *p);
       fprintf(bitfile, "] composefont pop\n");
       fprintf(bitfile, "%%%%EndFont\n");
     }
   }
}

static void setdir(int d)
{
   if (instring) {
      stringend();
      chrcmd('p');
   }
   switch(d) {
   case 1 :
      cmdout("dyt");
      break;
   case 2 :
      cmdout("dty");
      break;
   case 3 :
      cmdout("dtt");
      break;
   default :
      cmdout("dyy");
      break;
   }
   linepos += 4;
   any_dir = 1;
}

void cmddir(void)
{
   if (dir != rdir) {
      if (dir)
         cmdout("90 rotate");
      else
         cmdout("-90 rotate");
      rdir = dir;
   }
}

static int JIStoSJIS(int c)
{
    int high, low;
    int nh,nl;

    high = (c>>8) & 0xff;
    low = c & 0xff;
    nh = ((high-0x21)>>1) + 0x81;
    if (nh>0x9f)
    nh += 0x40;
    if (high & 1) {
        nl = low + 0x1f;
        if (low>0x5f)
        nl++;
    }
    else
        nl = low + 0x7e;
    if (((nh >= 0x81 && nh <= 0x9f) || (nh >= 0xe0 && nh <= 0xfc)) &&
        (nl >= 0x40 && nl <= 0xfc && nl != 0x7f))
        return((nh<<8) | nl);
    else
        return(0x813f);
}
