/*   $Id: dvips.h 50001 2019-02-11 18:22:31Z karl $
 *   Copyright 1986-2019 Tomas Rokicki.
 *   This is dvips, a freely redistributable PostScript driver
 *   for dvi files. You may freely use, modify and/or distribute this
 *   program or any portion thereof.
 */

/*   This file is the header for dvips's global data structures. */

#define CREATIONDATE

#define MAX_CODE 0x110000
#define MAX_2BYTES_CODE 0x10000
#define VF_MEM_UNIT 0x10000
#define CD_IDX(i)  ((i>=MAX_2BYTES_CODE ? MAX_2BYTES_CODE : i))

#define BANNER \
"This is dvips(k) 5.999 Copyright 2019 Radical Eye Software"
#define BANNER2 "(www.radicaleye.com)"
#if defined(MIKTEX)
#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#include <miktex/unxemu.h>
#include <direct.h>
#endif
#include <miktex/Core/Paths>
#include <miktex/Core/Session>
#include <sys/stat.h>
#include <limits.h>
#include <miktex/KPSE/Emulation>
#undef kpse_tex_hush
#define kpse_tex_hush(what) 1
#include <cstdlib>
using namespace MiKTeX::Core;
#define exit(status) throw(status)
#define ERRBUFSIZE 512
#define MAXPATHLEN 260
#endif
#ifdef KPATHSEA
#include "config.h"
#include "debug.h"
#endif
/*   Please don't turn debugging off! */
#ifndef DEBUG
#define DEBUG
#endif

#ifndef KPATHSEA
#include <stdio.h>
#if defined(SYSV) || defined(VMS) || defined(__THINK__) || defined(MSDOS) || defined(OS2) || defined(ATARIST) || defined(WIN32)
#include <string.h>
#else
#include <strings.h>
#endif
#endif
#if defined(lint) && defined(sun)
extern char *sprintf();
#endif
#include "paths.h"
#include "debug.h"
#ifdef VMS
#include "[.vms]vms.h"
#endif /* VMS */
#include <stdlib.h>
/*
 *   Is your malloc big?
 */
#if defined(MSDOS) && !defined(__EMX__) && !defined(DJGPP)
#define SMALLMALLOC
#endif
#if defined(OS2) && defined(_MSC_VER)
#define SMALLMALLOC
#endif
/*
 *   Constants, used to increase or decrease the capacity of this program.
 *
 *   Strings are now more dynamically allocated, so STRINGSIZE is not the
 *   strict limit it once was.  However, it still sets the maximum size
 *   of a string that can be handled in specials, so it should not be
 *   set too small.
 */
#define STRINGSIZE (200000) /* maximum total chars in strings in program */
#define RASTERCHUNK (8192)  /* size of chunk of raster */
#define MINCHUNK (240)      /* minimum size char to get own raster */
#define STACKSIZE (500)     /* maximum stack size for dvi files */
#define MAXFRAME (50)       /* maximum depth of virtual font recursion */
#define MAXFONTHD (1024)    /* number of unique names of included fonts */
#define STDOUTSIZE (75)     /* width of a standard output line */
#define DOWNLOADEDPSSIZE (1000)  /* max number of downloaded fonts to check */
/*
 *   Other constants, which define printer-dependent stuff.
 */
#define SWMEM (180000)      /* available virtual memory in PostScript printer */
#define DPI (actualdpi)     /* dots per inch */
#define VDPI (vactualdpi)   /* dots per inch */
#define RES_TOLERANCE(dpi) ((int)(1+dpi/500))
                            /* error in file name resolution versus desired */
#define FONTCOST (298)      /* overhead cost of each sw font */
#define PSFONTCOST (1100)   /* overhead cost for PostScript fonts */
#define PSCHARCOST (20)     /* overhead cost for PostScript font chars */
#define DNFONTCOST (35000)  /* overhead cost for downloaded PS font */
#define CHARCOST (15)       /* overhead cost for each character */
#define OVERCOST (30000)    /* cost of overhead */
#define DICTITEMCOST (20)   /* cost per key, value in dictionary */
#define NAMECOST (40)       /* overhead cost of each new name */
/*
 *   Type declarations.  integer must be a 32-bit signed; shalfword must
 *   be a sixteen-bit signed; halfword must be a sixteen-bit unsigned;
 *   quarterword must be an eight-bit unsigned.
 */
#if (defined(MSDOS) && !defined(DJGPP)) || (defined(OS2) && defined(_MSC_VER)) || defined(ATARIST)
typedef long integer;
typedef unsigned long uinteger;
#else
typedef int integer;
typedef unsigned int uinteger;
#endif
#ifndef KPATHSEA
typedef char boolean;
#endif
typedef double real;
typedef short shalfword;
typedef unsigned short halfword;
typedef unsigned char quarterword;
#ifdef WIN32
#define Boolean boolean
#else
#ifndef __THINK__
typedef short Boolean;
#endif
#endif
/*
 *   If the machine has a default integer size of 16 bits, and 32-bit
 *   integers must be manipulated with %ld, set the macro SHORTINT.
 */
#ifdef XENIX
#define SHORTINT
#else
#undef SHORTINT
#endif
#if defined(MSDOS) && !defined(__EMX__) && !defined(DJGPP) || defined(ATARIST)
#define SHORTINT
#endif
#if defined(OS2) && defined(_MSC_VER)
#define SHORTINT
#endif

/*
 *   This is the structure definition for resident fonts.  We use
 *   a small and simple hash table to handle these.  We don't need
 *   a big hash table.
 */
#define RESHASHPRIME (73)
struct resfont {
   struct resfont *next;
   char *Keyname, *PSname, *TeXname, *Fontfile, *Vectfile;
   char *specialinstructions;
   char *downloadheader; /* possibly multiple files */
   quarterword sent;
};

/*
 *   A chardesc describes an individual character.  Before the fonts are
 *   downloaded, the flags indicate that the character has already been used
 *   with the following meanings:
 */
typedef struct tcd {
   integer TFMwidth;
   quarterword *packptr;
   shalfword pixelwidth;
   quarterword flags, flags2;
} chardesctype;
#define EXISTS (1)
#define PREVPAGE (2)
#define THISPAGE (4)
#define TOOBIG (8) /* not used at the moment */
#define REPACKED (16)
#define BIGCHAR (32)
#define STATUSFLAGS (EXISTS|REPACKED|BIGCHAR)
/*
 *   The new field flags2 above is now an immutable field (once a font is
 *   loaded); for now it only indicates whether a character EXISTS or not.
 *   This fixes a problem with -G.
 */
/*
 *   A fontdesc describes a font.  The name, area, and scalename are located in
 *   the string pool. The nextsize pointer is used to link fonts that are used
 *   in included psfiles and differ only in scaledsize.  Such fonts also have
 *   a non-NULL scalename that gives the scaledsize as found in the included
 *   file.  The psflag indicates that the font has been used in an included
 *   psfile.  It can be 0, PREVPAGE, THISPAGE, or EXISTS.
 */
typedef struct tfd {
   integer checksum, scaledsize, designsize, thinspace, dir;
   halfword dpi, loadeddpi;
   halfword alreadyscaled;
   halfword psname;
   halfword loaded;
   quarterword psflag;
   quarterword codewidth;
   integer maxchars;
   char *name, *area;
   struct resfont *resfont;
   struct tft *localfonts;
   struct tfd *next;
   struct tfd *nextsize;
   char *scalename;
   chardesctype *chardesc;
   int iswide;
} fontdesctype;

/*  A fontmap associates a fontdesc with a font number.
 */
typedef struct tft {
   integer fontnum;
   fontdesctype *desc;
   struct tft *next;
} fontmaptype;

/*   Virtual fonts require a `macro' capability that is implemented by
 *   using a stack of `frames'.
 */
typedef struct {
   quarterword *curp, *curl;
   fontdesctype *curf;
   fontmaptype *ff;
} frametype;

/*
 *   The next type holds the font usage information in a 256-bit table;
 *   there's a 1 for each character that was used in a section.
 */
typedef struct {
   fontdesctype *fd;
   halfword psfused;
   halfword bitmap[16];
} charusetype;

/*   Next we want to record the relevant data for a section.  A section is
 *   a largest portion of the document whose font usage does not overflow
 *   the capacity of the printer.  (If a single page does overflow the
 *   capacity all by itself, it is made into its own section and a warning
 *   message is printed; the page is still printed.)
 *
 *   The sections are in a linked list, built during the prescan phase and
 *   processed in proper order (so that pages stack correctly on output) during
 *   the second phase.
 */
typedef struct t {
   integer bos;
   struct t *next;
   halfword numpages;
} sectiontype;

/*
 *   Sections are actually represented not by sectiontype but by a more
 *   complex data structure of variable size, having the following layout:
 *      sectiontype sect;
 *      charusetype charuse[numfonts];
 *      fontdesctype *sentinel = NULL;
 *   (Here numfonts is the number of bitmap fonts currently defined.)
 *    Since we can't declare this or take a sizeof it, we build it and
 *   manipulate it ourselves (see the end of the prescan routine).
 */
/*
 *   This is how we build up headers and other lists.
 */
struct header_list {
   struct header_list *next;
   const char *Hname;
   char *precode;
   char *postcode;
   char *name;
};
/*
 *   Some machines define putlong in their library.
 *   We get around this here.
 */
#define putlong was_putlong
/*
 *   Information on available paper sizes is stored here.
 */
struct papsiz {
   struct papsiz *next;
   integer xsize, ysize;
   const char *name;
   const char *specdat;
};
#ifdef MVSXA /* IBM: MVS/XA */
/* this is where we fix problems with conflicts for truncation
   of long names (we might only do this if LONGNAME not set but ...) */
#   define pprescanpages pprscnpgs  /* confict with pprescan */
#   define flushDashedPath flshDshdPth  /* conflict with flushDash */
#   define PageList PgList  /* re-definition conflict with pagelist  */
/* adding ascii2ebcdic conversion table to extern */
    extern char ascii2ebcdic[];
#endif  /* IBM: MVS/XA */
#ifdef VMCMS /* IBM: VM/CMS */
/* this is where we fix problems with conflicts for truncation
   of long names (we might only do this if LONGNAME not set but ...) */
#   define pprescanpages pprscnpgs  /* confict with pprescan */
#   define flushDashedPath flshDshdPth  /* conflict with flushDash */
/* adding ascii2ebcdic conversion table to extern */
    extern char ascii2ebcdic[];
/* redefining fopen for VMCMS, see DVIPSCMS.C */
    extern FILE *cmsfopen();
#   ifdef fopen
#      undef fopen
#   endif
#   define fopen cmsfopen
#define downloadpspk dnldpspk
#endif  /* IBM: VM/CMS */
/*
 *   Remove namespace conflict with standard library on Macintosh
 */
#ifdef __THINK__
#define newstring newpoolstring
#endif

#ifndef VOID
#define VOID void
#endif

#define USE_PCLOSE (801)
#define USE_FCLOSE (802)
#if defined(MIKTEX)
#define USE_MIKTEX_CLOSE_FILE (803)
#endif

/* Things that KPATHSEA knows, and are useful even without it. */
#if !defined(KPATHSEA)

#if defined(MSDOS) || defined(OS2) || defined(WIN32)
#define FOPEN_ABIN_MODE "ab"
#define FOPEN_RBIN_MODE "rb"
#else
#define FOPEN_ABIN_MODE "a"
#define FOPEN_RBIN_MODE "r"
#endif

#if (defined MSDOS || defined OS2 || defined WIN32)
#define WRITEBIN "wb"
#else
#ifdef VMCMS
#define WRITEBIN "wb, lrecl=1024, recfm=f"
#else
#define WRITEBIN "w"
#endif
#endif

#if defined(WIN32)
#define STDC_HEADERS
#include <io.h>
#include <fcntl.h>
#define O_BINARY _O_BINARY
#define register
#define SET_BINARY(fd) (void)_setmode((fd), _O_BINARY)
#else /* !WIN32 */
#define SET_BINARY(fd) (void)0
#endif

#if defined(DEVICESEP)
#define IS_DEVICE_SEP(c) ((c) == DEVICESEP)
#else
#define IS_DEVICE_SEP(c) 0
#endif
#define STREQ(s1, s2) (((s1) != NULL) && ((s2) != NULL) && !strcmp((s1), (s2)))

#if defined(MSDOS) || defined(OS2) || defined(WIN32)
#define NAME_BEGINS_WITH_DEVICE(name) (*(name) && IS_DEVICE_SEP((name)[1]))
#define IS_DIR_SEP(c) ((c) == '/' || (c) == '\\')
#define FILESTRCASEEQ(s1, s2) (strcasecmp (s1, s2) == 0)
#else
#define NAME_BEGINS_WITH_DEVICE(name) 0
#define IS_DIR_SEP(c) ((c) == DIRSEP)
#define FILESTRCASEEQ STREQ
#endif

#define TOLOWER(c) tolower(c)
#define ISALNUM(c) isalnum(c)
#define ISXDIGIT(c) (isascii (c) && isxdigit(c))

#endif
