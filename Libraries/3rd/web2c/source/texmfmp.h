/* texmfmp.h: Main include file for TeX and MF in C. This file is
   included by {tex,mf}d.h, which is the first include in the C files
   output by web2c.  */

#include "cpascal.h"
#include <kpathsea/c-pathch.h> /* for IS_DIR_SEP, used in the change files */
#include <kpathsea/tex-make.h> /* for kpse_make_tex_discard_errors */

#if !defined(MIKTEX)
#ifdef XeTeX
#ifdef XETEX_MAC
/* include this here to avoid conflict between clang's emmintrin.h and
 * texmfmem.h. Should be removed once a fixed clang is widely available
 * http://llvm.org/bugs/show_bug.cgi?id=14964 */
#include <ApplicationServices/ApplicationServices.h>
#endif
/* added typedefs for unicodefile and voidpointer */
#define XETEX_UNICODE_FILE_DEFINED	1
typedef struct {
  FILE *f;
  long  savedChar;
  short skipNextLF;
  short encodingMode;
  void *conversionData;
} UFILE;
typedef UFILE* unicodefile;

typedef void* voidpointer;
#endif
#endif

/* If we have these macros, use them, as they provide a better guide to
   the endianess when cross-compiling. */
#if defined (BYTE_ORDER) && defined (BIG_ENDIAN) && defined (LITTLE_ENDIAN)
#ifdef WORDS_BIGENDIAN
#undef WORDS_BIGENDIAN
#endif
#if BYTE_ORDER == BIG_ENDIAN
#define WORDS_BIGENDIAN
#endif
#endif
/* More of the same, but now NeXT-specific. */
#ifdef NeXT
#ifdef WORDS_BIGENDIAN
#undef WORDS_BIGENDIAN
#endif
#ifdef __BIG_ENDIAN__
#define WORDS_BIGENDIAN
#endif
#endif

#if !defined(MIKTEX)
/* Some things are the same except for the name.  */
#ifdef TeX
#if defined (pdfTeX)
#define TEXMFPOOLNAME "pdftex.pool"
#define TEXMFENGINENAME "pdftex"
#elif defined (eTeX)
#define TEXMFPOOLNAME "etex.pool"
#define TEXMFENGINENAME "etex"
#elif defined (XeTeX)
#define TEXMFPOOLNAME "xetex.pool"
#define TEXMFENGINENAME "xetex"
#elif defined (Aleph)
#define TEXMFPOOLNAME "aleph.pool"
#define TEXMFENGINENAME "aleph"
#elif defined (pTeX)
#define TEXMFPOOLNAME "ptex.pool"
#define TEXMFENGINENAME "ptex"
#include "ptexdir/kanji.h"
#elif defined (epTeX)
#define TEXMFPOOLNAME "eptex.pool"
#define TEXMFENGINENAME "eptex"
#include "ptexdir/kanji.h"
#elif defined (upTeX)
#define TEXMFPOOLNAME "uptex.pool"
#define TEXMFENGINENAME "uptex"
#include "uptexdir/kanji.h"
#elif defined (eupTeX)
#define TEXMFPOOLNAME "euptex.pool"
#define TEXMFENGINENAME "euptex"
#include "uptexdir/kanji.h"
#else
#define TEXMFPOOLNAME "tex.pool"
#define TEXMFENGINENAME "tex"
#endif
#define DUMP_FILE fmtfile
#define DUMP_FORMAT kpse_fmt_format
#define writedvi WRITE_OUT
#define flushdvi flush_out
#define OUT_FILE dvifile
#define OUT_BUF dvibuf
#endif /* TeX */
#ifdef MF
#if defined(MFLua)
#define TEXMFPOOLNAME "mflua.pool"
#define TEXMFENGINENAME "mflua"
#elif defined(MFLuaJIT)
#define TEXMFPOOLNAME "mfluajit.pool"
#define TEXMFENGINENAME "mfluajit"
#else
#define TEXMFPOOLNAME "mf.pool"
#define TEXMFENGINENAME "metafont"
#endif
#define DUMP_FILE basefile
#define DUMP_FORMAT kpse_base_format
#define writegf WRITE_OUT
#define OUT_FILE gffile
#define OUT_BUF gfbuf
#endif /* MF */
#endif

#if !defined(MIKTEX)
/* Restore underscores.  */
#define dumpname dump_name
#define kpsedvipsconfigformat kpse_dvips_config_format
#define kpsefontmapformat kpse_fontmap_format
#define kpsemfpoolformat kpse_mfpool_format
#define kpsempformat kpse_mp_format
#define kpsemppoolformat kpse_mppool_format
#define kpsetexpoolformat kpse_texpool_format
#define kpsetexformat kpse_tex_format
#endif

/* Hacks for TeX that are better not to #ifdef, see lib/openclose.c.  */
extern int tfmtemp, texinputtype;

/* pdfTeX routines also used for e-pTeX, e-upTeX, and XeTeX */
#if defined (pdfTeX) || defined (epTeX) || defined (eupTeX) || defined(XeTeX)
#if !defined (pdfTeX)
extern void pdftex_fail(const char *fmt, ...);
#endif
extern char start_time_str[];
extern void initstarttime(void);
#if !defined(XeTeX)
extern char *makecstring(integer s);
extern char *makecfilename(integer s);
#endif /* !XeTeX */
extern void getcreationdate(void);
extern void getfilemoddate(integer s);
extern void getfilesize(integer s);
extern void getfiledump(integer s, int offset, int length);
extern void convertStringToHexString(const char *in, char *out, int lin);
extern void getmd5sum(integer s, int file);
#endif

/* pdftex etc. except for tex use these for pipe support */
#if defined(TeX) && !defined(onlyTeX)
extern boolean open_in_or_pipe (FILE **, int, const_string fopen_mode);
extern boolean open_out_or_pipe (FILE **, const_string fopen_mode);
extern void close_file_or_pipe (FILE *);
#define ENABLE_PIPES 1
#else
#define ENABLE_PIPES 0
#endif

/* Executing shell commands.  */
extern int runsystem (const char *cmd);

/* The entry point.  */
extern void maininit (int ac, string *av);
#if defined(WIN32) && !defined(__MINGW32__) && defined(DLLPROC)
extern __declspec(dllexport) int DLLPROC (int ac, string *av);
#else
#undef DLLPROC
#endif

/* All but the Omega family use this. */
#if !defined(Aleph)
extern void readtcxfile (void);
extern string translate_filename;
#define translatefilename translate_filename
#endif

#ifdef TeX
/* The type `glueratio' should be a floating point type which won't
   unnecessarily increase the size of the memoryword structure.  This is
   the basic requirement.  On most machines, if you're building a
   normal-sized TeX, then glueratio must probably meet the following
   restriction: sizeof(glueratio) <= sizeof(integer).  Usually, then,
   glueratio must be `float'.  But if you build a big TeX, you can (on
   most machines) and should make it `double' to avoid loss of precision
   and conversions to and from double during calculations.  (All this
   also goes for Metafont.)  Furthermore, if you have enough memory, it
   won't hurt to have this defined to be `double' for running the
   trip/trap tests.
   
   This type is set automatically to `float' by configure if a small TeX
   is built.  */
#ifndef GLUERATIO_TYPE
#define GLUERATIO_TYPE double
#endif
typedef GLUERATIO_TYPE glueratio;

#if defined(__DJGPP__) && defined (IPC)
#undef IPC
#endif

#ifdef IPC
extern void ipcpage (int);
#endif /* IPC */
#endif /* TeX */

/* How to flush the DVI file.  */
#define flush_out() fflush (OUT_FILE)

/* Used to write to a TFM file.  */
#define put2bytes(f, h) do { \
    integer v = (integer) (h); putbyte (v >> 8, f);  putbyte (v & 0xff, f); \
  } while (0)
#define put4bytes(f, w) do { \
    integer v = (integer) (w); \
    putbyte (v >> 24, f); putbyte (v >> 16, f); \
    putbyte (v >> 8, f);  putbyte (v & 0xff, f); \
  } while (0)

#if !defined(MIKTEX)
/* Read a line of input as quickly as possible.  */
#define	inputln(stream, flag) input_line (stream)
#ifdef XeTeX
extern boolean input_line (UFILE *);
#else
extern boolean input_line (FILE *);
#endif
#endif

/* This routine has to return four values.  */
#define	dateandtime(i,j,k,l) get_date_and_time (&(i), &(j), &(k), &(l))
extern void get_date_and_time (integer *, integer *, integer *, integer *);

#if !defined(MIKTEX)
#if defined(pdfTeX) || defined(epTeX) || defined(eupTeX) || defined(XeTeX)
/* Get high-res time info. */
#define secondsandmicros(i,j) get_seconds_and_micros (&(i), &(j))
extern void get_seconds_and_micros (integer *, integer *);
#endif
#endif

/* Copy command-line arguments into the buffer, despite the name.  */
extern void topenin (void);

/* Can't prototype this since it uses poolpointer and ASCIIcode, which
   are defined later in mfd.h, and mfd.h uses stuff from here.  */
/* Therefore the department of ugly hacks decided to move this declaration
   to the *coerce.h files. */
/* extern void calledit (); */

/* These defines reroute the file i/o calls to the new pipe-enabled 
   functions in texmfmp.c*/

#if !defined(MIKTEX)
#if ENABLE_PIPES
#undef aopenin
#undef aopenout
#undef aclose
#define aopenin(f,p)  open_in_or_pipe(&(f),p,FOPEN_RBIN_MODE)
#define aopenout(f)   open_out_or_pipe(&(f),FOPEN_W_MODE)
#define aclose(f)     close_file_or_pipe(f)
#endif
#endif

#if !defined(MIKTEX)
/* `bopenin' (and out) is used only for reading (and writing) .tfm
   files; `wopenin' (and out) only for dump files.  The filenames are
   passed in as a global variable, `nameoffile'.  */
#define bopenin(f)	open_input (&(f), kpse_tfm_format, FOPEN_RBIN_MODE)
#define ocpopenin(f)	open_input (&(f), kpse_ocp_format, FOPEN_RBIN_MODE)
#define ofmopenin(f)	open_input (&(f), kpse_ofm_format, FOPEN_RBIN_MODE)

#define bopenout(f)	open_output (&(f), FOPEN_WBIN_MODE)
#define bclose		aclose
#ifdef XeTeX
/* f is declared as gzFile, but we temporarily use it for a FILE *
   so that we can use the standard open calls */
#define wopenin(f)	(open_input ((FILE**)&(f), DUMP_FORMAT, FOPEN_RBIN_MODE) \
						&& (f = gzdopen(fileno((FILE*)f), FOPEN_RBIN_MODE)))
#define wopenout(f)	(open_output ((FILE**)&(f), FOPEN_WBIN_MODE) \
						&& (f = gzdopen(fileno((FILE*)f), FOPEN_WBIN_MODE)) \
						&& (gzsetparams(f, 1, Z_DEFAULT_STRATEGY) == Z_OK))
#define wclose(f)	gzclose(f)
#else
#define wopenin(f)	open_input (&(f), DUMP_FORMAT, FOPEN_RBIN_MODE)
#define wopenout	bopenout
#define wclose		aclose
#endif
#endif

#if !defined(MIKTEX)
#ifdef XeTeX
#if ENABLE_PIPES
extern boolean u_open_in_or_pipe(unicodefile* f, integer filefmt, const_string fopen_mode, integer mode, integer encodingData);
extern void u_close_file_or_pipe(unicodefile* f);
#define uopenin(f,p,m,d) u_open_in_or_pipe(&(f), p, FOPEN_RBIN_MODE, m, d)
#define uclose(f) u_close_file_or_pipe(&(f))
#else
#define uopenin(f,p,m,d) u_open_in(&(f), p, FOPEN_RBIN_MODE, m, d)
#define uclose(f) u_close_inout(&(f))
#endif
#endif
#endif

/* Used in tex.ch (section 1338) to get a core dump in debugging mode.  */
#ifdef unix
#define dumpcore abort
#else
#define dumpcore uexit (1)
#endif

#ifdef MF
extern boolean initscreen (void);
extern void updatescreen (void);
/* Can't prototype these for same reason as `calledit' above.  */
#if 0  /* Therefore the real declaration is found in the coerce.h files.  */
extern void blankrectangle (/*screencol, screencol, screenrow, screenrow*/);
extern void paintrow (/*screenrow, pixelcolor, transspec, screencol*/);
#endif
#endif /* MF */


/* (Un)dumping.  These are called from the change file.  */
#define	dumpthings(base, len) \
  do_dump ((char *) &(base), sizeof (base), (int) (len), DUMP_FILE)
#define	undumpthings(base, len) \
  do_undump ((char *) &(base), sizeof (base), (int) (len), DUMP_FILE)

#ifndef PRIdPTR
#define PRIdPTR "ld"
#endif
#ifndef PRIxPTR
#define PRIxPTR "lx"
#endif

/* Like do_undump, but check each value against LOW and HIGH.  The
   slowdown isn't significant, and this improves the chances of
   detecting incompatible format files.  In fact, Knuth himself noted
   this problem with Web2c some years ago, so it seems worth fixing.  We
   can't make this a subroutine because then we lose the type of BASE.  */
#define undumpcheckedthings(low, high, base, len)			\
  do {                                                                  \
    unsigned i;                                                         \
    undumpthings (base, len);                                           \
    for (i = 0; i < (len); i++) {                                       \
      if ((&(base))[i] < (low) || (&(base))[i] > (high)) {              \
        FATAL5 ("Item %u (=%" PRIdPTR ") of .fmt array at %" PRIxPTR    \
                " <%" PRIdPTR " or >%" PRIdPTR,                         \
                i, (uintptr_t) (&(base))[i], (uintptr_t) &(base),       \
                (uintptr_t) low, (uintptr_t) high);                     \
      }                                                                 \
    }									\
  } while (0)

/* Like undump_checked_things, but only check the upper value. We use
   this when the base type is unsigned, and thus all the values will be
   greater than zero by definition.  */
#define undumpuppercheckthings(high, base, len)				\
  do {                                                                  \
    unsigned i;                                                         \
    undumpthings (base, len);                                           \
    for (i = 0; i < (len); i++) {                                       \
      if ((&(base))[i] > (high)) {              			\
        FATAL4 ("Item %u (=%" PRIdPTR ") of .fmt array at %" PRIxPTR    \
                " >%" PRIdPTR,                                          \
                i, (uintptr_t) (&(base))[i], (uintptr_t) &(base),       \
                (uintptr_t) high);                         		\
      }                                                                 \
    }									\
  } while (0)

/* We define the routines to do the actual work in texmfmp.c.  */
#ifdef XeTeX
#include <zlib.h>
extern void do_dump (char *, int, int, gzFile);
extern void do_undump (char *, int, int, gzFile);
#else
extern void do_dump (char *, int, int, FILE *);
extern void do_undump (char *, int, int, FILE *);
#endif

/* Use the above for all the other dumping and undumping.  */
#define generic_dump(x) dumpthings (x, 1)
#define generic_undump(x) undumpthings (x, 1)

#define dumpwd   generic_dump
#define dumphh   generic_dump
#define dumpqqqq generic_dump
#define undumpwd   generic_undump
#define undumphh   generic_undump
#define	undumpqqqq generic_undump

/* `dump_int' is called with constant integers, so we put them into a
   variable first.  */
#define	dumpint(x)							\
  do									\
    {									\
      integer x_val = (x);						\
      generic_dump (x_val);						\
    }									\
  while (0)

/* web2c/regfix puts variables in the format file loading into
   registers.  Some compilers aren't willing to take addresses of such
   variables.  So we must kludge.  */
#if defined(REGFIX) || defined(WIN32)
#define undumpint(x)							\
  do									\
    {									\
      integer x_val;							\
      generic_undump (x_val);						\
      x = x_val;							\
    }									\
  while (0)
#else
#define	undumpint generic_undump
#endif

/* Handle SyncTeX, if requested */
#if defined(TeX)
# if defined(__SyncTeX__)
#if defined(MIKTEX)
#include "synctex-common.h"
#else
#  include "synctexdir/synctex-common.h"
#endif
extern char *generic_synctex_get_current_name(void);
# endif
#endif

