/*
 *   This is the main routine.
 */
#include "dvips.h" /* The copyright notice there is included too! */
#ifdef KPATHSEA
#include <kpathsea/c-pathch.h>
#include <kpathsea/proginit.h>
#include <kpathsea/progname.h>
#include <kpathsea/tex-hush.h>
#include <kpathsea/tex-make.h>
#include <kpathsea/variable.h>
#include <kpathsea/version.h>
#include <kpathsea/lib.h>
#ifdef strdup
#undef strdup
#endif
#define strdup xstrdup
#include "paths.h"
#else /* ! KPATHSEA */
#include <stdlib.h> /* for malloc, etc. */
#if !defined(SYSV) && !defined(WIN32)
extern char *strtok(); /* some systems don't have this in strings.h */
#endif
#if defined(WIN32)
#include <io.h>
#include <fcntl.h>
#include <string.h>
#define O_BINARY _O_BINARY
#endif
#define DIR_SEP DIRSEP
#endif /* KPATHSEA */
#ifdef VMS
#define GLOBAL globaldef
#ifdef __GNUC__
#include "climsgdef.h"	/* created by hand, extracted from STARLET.MLB */
			/* and put in GNU_CC:[INCLUDE.LOCAL]           */
#include "ctype.h"
#include "descrip.h"
#else
#include climsgdef
#include ctype
#include descrip
#endif
#endif
#ifdef __APPLE__ /* for Mac OS X, T. Uchiyama */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#if !defined(MIKTEX)
#if defined(WIN32) && defined(KPATHSEA)
FILE *generic_fsyscp_fopen(const char *filename, const char *mode)
{
  FILE *f;

  f = fsyscp_fopen (filename, mode);

  if (f == NULL && file_system_codepage != win32_codepage) {
    int tmpcp = file_system_codepage;
    file_system_codepage = win32_codepage;
    f = fsyscp_fopen (filename, mode);
    file_system_codepage = tmpcp;
  }

  return f;
}
#undef fopen
#define fopen(file, fmode)  generic_fsyscp_fopen(file, fmode)
#endif
#endif

#ifndef DEFRES
#define DEFRES (600)
#endif

/*
 *   The external declarations:
 */
#include "protos.h"

/*
 *   First we define some globals.
 */
#ifdef VMS
    static char ofnme[252],infnme[252],pap[40],thh[20];
#endif

/* PS fonts fully downloaded as headers */
char *downloadedpsnames[DOWNLOADEDPSSIZE];

int unused_top_of_psnames;   /* unused top number of downloadedpsnames[#] */
fontdesctype *fonthead;      /* list of all fonts mentioned so far */
fontdesctype *curfnt;        /* the currently selected font */
sectiontype *sections;       /* sections to process document in */
Boolean partialdownload = 1; /* turn on partial downloading */
Boolean manualfeed;          /* manual feed? */
Boolean compressed;          /* compressed? */
Boolean downloadpspk;        /* use PK for downloaded PS fonts? */
Boolean safetyenclose;       /* enclose in save/restore for stupid spoolers? */
Boolean removecomments = 0;  /* remove comments from included PS? */
Boolean nosmallchars;        /* disable small char optimization for X4045? */
Boolean cropmarks;           /* add cropmarks? */
Boolean abspage = 0;         /* are page numbers absolute? */
Boolean tryepsf = 0;         /* should we try to make it espf? */
int secure = 1;              /* make safe for suid */
int secure_option = 0;        /* set by -R */
int collatedcopies = 1;      /* how many collated copies? */
int sectioncopies = 1;       /* how many times to repeat each section? */
integer pagecopies = 1;      /* how many times to repeat each page? */
shalfword linepos = 0;       /* where are we on the line being output? */
integer maxpages;            /* the maximum number of pages */
Boolean notfirst, notlast;   /* true if a first page was specified */
Boolean evenpages, oddpages; /* true if doing only even/only odd pages */
Boolean pagelist;            /* true if using page ranges */
Boolean sendcontrolD;        /* should we send a control D at end? */
Boolean shiftlowchars;       /* shift [0-32, 127] characters higher? */
integer firstpage;           /* the number of the first page if specified */
integer lastpage;
integer firstseq;
integer lastseq;
integer hpapersize, vpapersize; /* horizontal and vertical paper size */
integer hoff, voff;          /* horizontal and vertical offsets */
integer maxsecsize = 0;       /* the maximum size of a section */
integer firstboploc;         /* where the first bop is */
Boolean sepfiles;            /* each section in its own file? */
int numcopies;               /* number of copies of each page to print */
const char *oname;           /* output file name */
char *iname;                 /* dvi file name */
char *fulliname;             /* same, with current working directory */
char *strings;               /* strings for program */
char *nextstring, *maxstring; /* string pointers */
FILE *dvifile, *bitfile;     /* dvi and output files */
quarterword *curpos;        /* current position in virtual character packet */
quarterword *curlim;         /* final byte in virtual character packet */
fontmaptype *ffont;          /* first font in current frame */
real conv;                   /* conversion ratio, pixels per DVI unit */
real vconv;                  /* conversion ratio, pixels per DVI unit */
real alpha;                  /* conversion ratio, DVI unit per TFM unit */
double mag;                  /* the magnification of this document */
integer num, den;            /* the numerator and denominator */
int overridemag;             /* substitute for mag value in DVI file? */
int actualdpi = DEFRES;      /* the actual resolution of the printer */
int vactualdpi = DEFRES;     /* the actual resolution of the printer */
int maxdrift;                /* max pixels away from true rounded position */
int vmaxdrift;               /* max pixels away from true rounded position */
char *paperfmt;              /* command-line paper format */
int landscape = 0;           /* landscape mode */
integer fontmem;             /* memory remaining in printer */
integer pagecount;           /* page counter for the sections */
integer pagenum;             /* the page number we currently look at */
long bytesleft;              /* number of bytes left in raster */
quarterword *raster;         /* area for raster manipulations */
integer hh, vv;              /* horizontal and vertical pixel positions */
Boolean noomega = 0;         /* Omega extensions are enabled */
Boolean noptex = 0;          /* pTeX extensions are enabled */
Boolean lastpsizwins = 1;    /* if 1, last \special{papersize=w,h} wins */

/*-----------------------------------------------------------------------*
 * The PATH definitions cannot be defined on the command line because so many
 * DEFINE's overflow the DCL buffer when using the GNU CC compiler.
 *-----------------------------------------------------------------------*/
#if defined(VMS) && defined(__GNUC__)
#include "vms_gcc_paths.h"
#endif

const char *infont;                /* is the file we are downloading a font? */
#ifndef KPATHSEA
#ifdef ATARIST
#   define TFMPATH "."
#   define PKPATH "."
#   define VFPATH "."
#   define FIGPATH "."
#   define HEADERPATH "."
#   define CONFIGPATH "."
#endif
char *tfmpath = TFMPATH;     /* pointer to directories for tfm files */
char *pkpath = PKPATH;       /* pointer to directories for pk files */
char *vfpath = VFPATH;       /* pointer to directories for vf files */
char *figpath = FIGPATH;     /* pointer to directories for figure files */
char *headerpath = HEADERPATH; /* pointer to directories for header files */
char *configpath = CONFIGPATH; /* where to find config files */
#ifndef PICTPATH
#ifndef __THINK__
#define PICTPATH "."
#else
#define PICTPATH ":"
#endif
#endif
char *pictpath = PICTPATH;   /* where IFF/etc. pictures are found */
#ifdef SEARCH_SUBDIRECTORIES
char *fontsubdirpath = FONTSUBDIRPATH;
#endif
#endif /* ! KPATHSEA */
#ifdef FONTLIB
char *flipath = FLIPATH;     /* pointer to directories for fli files */
char *fliname = FLINAME;     /* pointer to names of fli files */
#endif
integer swmem;               /* font memory in the PostScript printer */
int quiet;                   /* should we only print errors to stderr? */
int filter;                  /* act as filter default output to stdout,
                                               default input to stdin? */
int dvips_debug_flag;        /* output config and map files to stderr if 1 */
int prettycolumn;            /* the column we are at when running pretty */
int gargc;                   /* global argument count */
char **gargv;                /* global argument vector */
int totalpages = 0;          /* total number of pages */
Boolean reverse;             /* are we going reverse? */
Boolean usesPSfonts;         /* do we use local PostScript fonts? */
Boolean usesspecial;         /* do we use \special? */
Boolean headers_off;         /* do we send headers or not? */
Boolean usescolor;           /* IBM: color - do we use colors? */
const char *headerfile;      /* default header file */
char *warningmsg;            /* a message to write, if set in config file */
Boolean multiplesects;       /* more than one section? */
Boolean disablecomments;     /* should we suppress any EPSF comments? */
char *printer;               /* what printer to send this to? */
char *mfmode;                /* default MF mode */
char *mflandmode;            /* allow an optional landscape mode def */
int mfmode_option;            /* set by -mode command-line option */
int oname_option;             /* set by -o option */
frametype frames[MAXFRAME];  /* stack for virtual fonts */
integer pagecost;               /* memory used on the page being prescanned */
int delchar;                    /* characters to delete from prescanned page */
integer fsizetol;               /* max dvi units error for psfile font sizes */
Boolean includesfonts;          /* are fonts used in included psfiles? */
fontdesctype *fonthd[MAXFONTHD];/* list headers for included fonts of 1 name */
int nextfonthd;                 /* next unused fonthd[] index */
char xdig[256];                 /* table for reading hexadecimal digits */
char banner[] = BANNER;        /* our startup message */
char banner2[] = BANNER2;      /* our second startup message */
Boolean noenv = 0;             /* ignore PRINTER envir variable? */
Boolean dopprescan = 0;        /* do we do a scan before the prescan? */
struct papsiz *papsizes;       /* all available paper size */
int headersready;              /* ready to check headers? */
#if defined(MSDOS) || defined(OS2) || defined(ATARIST)
char *mfjobname = NULL;         /* name of mfjob file if given */
FILE *mfjobfile = NULL;         /* mfjob file for font making instructions */
#endif
#ifdef DEBUG
integer debug_flag = 0;
#endif /* DEBUG */
char queryline[256];                /* interactive query of options */
int qargc;
char *qargv[32];
char queryoptions;
#if defined(MIKTEX)
int miktex_no_glyphs = 0;
int miktex_no_rules = 0;
int miktex_no_landscape = 0;
int miktex_pedantic = 0;
int miktex_allow_all_paths = 0;
#endif
#ifdef HPS
Boolean HPS_FLAG = 0;
#endif

/* Declare the routine to get the current working directory.  */

/* The getwd/getcwd stuff here is not portable, and I don't think it's
   worth it to include xgetcwd.d just so we can print the full pathname
   of the DVI file in the output. --karl */
#define IGNORE_CWD

#ifndef IGNORE_CWD
#ifndef HAVE_GETCWD
extern char *getwd (); /* said to be faster than getcwd (SunOS man page) */
#define getcwd(b, len)  getwd(b) /* used here only when b nonnull */
#else
#ifdef ANSI
extern char *getcwd (char *, int);
#else
extern char *getcwd ();
#endif /* not ANSI */
#endif /* not HAVE_GETWD */
#if defined(SYSV) || defined(VMS) || (defined(MSDOS) && !defined(__DJGPP__)) || defined(OS2) || defined(ATARIST)
#define MAXPATHLEN (256)
#else
#include <sys/param.h>          /* for MAXPATHLEN */
#endif
#endif /* not IGNORE_CWD */

static const char *helparr[] = {
#ifndef VMCMS
"Usage: dvips [OPTION]... FILENAME[.dvi]",
#else
"    VM/CMS Usage:",
"           dvips fname [ftype [fmode]] [options]",
"or",
"           dvips fname[.ftype[.fmode]] [options]",
#endif
"Convert DVI input files to PostScript.",
"See http://tug.org/dvips/ for the full manual and other information.",
"",
"Options:",
"-a*  Conserve memory, not time       -A   Print only odd (TeX) pages",
"-b # Page copies, for posters e.g.   -B   Print only even (TeX) pages",
"-bitmapfontenc [on,off,strict] control bitmap font encoding",
"-c # Uncollated copies               -C # Collated copies",
"-d # Debugging                       -D # Resolution",
"-e # Maxdrift value                  -E*  Try to create EPSF",
"-f*  Run as filter                   -F*  Send control-D at end",
#ifdef SHIFTLOWCHARS
"                                     -G*  Shift low chars to higher pos.",
#endif
"-h f Add header file",
"-i*  Separate file per section",
"-j*  Download fonts partially",
"-k*  Print crop marks                -K*  Pull comments from inclusions",
"-l # Last page                       -L*  Last special papersize wins",
"-m*  Manual feed                     -M*  Don't make fonts",
"-mode s Metafont device name",
"-n # Maximum number of pages         -N*  No structured comments",
"-noomega  Disable Omega extensions",
"-noptex   Disable pTeX extensions",
"-o f Output file                     -O c Set/change paper offset",
#if defined(MSDOS) || defined(OS2)
"-p # First page                      -P s Load $s.cfg",
#else
"-p # First page                      -P s Load config.$s",
#endif
"-pp l Print only pages listed",
"-q*  Run quietly",
"-r*  Reverse order of pages          -R*  Run securely",
"-s*  Enclose output in save/restore  -S # Max section size in pages",
"-t s Paper format                    -T c Specify desired page size",
"-u s PS mapfile                      -U*  Disable string param trick",
"-v   Print version number and quit   -V*  Send downloadable PS fonts as PK",
"-x # Override dvi magnification      -X # Horizontal resolution",
"-y # Multiply by dvi magnification   -Y # Vertical resolution",
#ifdef HPS
"-z*  Hyper PS                        -Z*  Compress bitmap fonts",
#else
"                                     -Z*  Compress bitmap fonts",
#endif
/*"-   Interactive query of options", */
"    # = number   f = file   s = string  * = suffix, `0' to turn off",
"    c = comma-separated dimension pair (e.g., 3.2in,-32.1cm)",
"    l = comma-separated list of page ranges (e.g., 1-4,7-9)", 0};

void
help(int status)
{
   const char **p;
   FILE *f = status == 0 ? stdout : stderr;
   for (p=helparr; *p; p++)
      fprintf (f, "%s\n", *p);

   putc ('\n', f);
#ifdef KPATHSEA
   fputs (kpse_bug_address, f);
#endif
}

static void
freememforpsnames(void)
{
   int i;

   for (i = 0; i < unused_top_of_psnames && downloadedpsnames[i]; i++)
      free (downloadedpsnames[i]);
}

/*
 *   This error routine prints an error message; if the first
 *   character is !, it aborts the job.
 */

static char *progname;

void
error_with_perror(const char *s, const char *fname)
{
   if (prettycolumn > 0)
        fprintf(stderr,"\n");
   prettycolumn = 0;
   fprintf(stderr, "%s: %s", progname, s);
   if (fname) {
     putc (' ', stderr);
     perror (fname);
   } else {
     putc ('\n', stderr);
   }

#if defined(MIKTEX)
   if (miktex_pedantic || *s=='!') {
#else
   if (*s=='!') {
#endif
      freememforpsnames();
      if (bitfile != NULL) {
         cleanprinter();
      }
      exit(1); /* fatal */
   }
}

/*
 *   This error routine prints an error message; if the first
 *   character is !, it aborts the job.
 */
void
error(const char *s)
{
   error_with_perror (s, NULL);
}

#ifndef KPATHSEA
char *
concat(char *s1, char *s2)
{
  char *s = malloc(strlen(s1)+strlen(s2)+1);
  if (s == NULL) {
    fprintf(stderr, "Malloc failed to give %d bytes.\nAborting\n",
	    strlen(s1)+strlen(s2)+1);
    exit(1);
  }
  strcpy(s, s1);
  strcat(s, s2);
}
#endif

/* Report a warning if both checksums are nonzero, they don't match, and
   the user hasn't turned it off.  */

void
check_checksum(unsigned c1, unsigned c2, const char *name)
{
  if (c1 && c2 && c1 != c2
#ifdef KPATHSEA
      && !kpse_tex_hush ("checksum")
#endif
      ) {
     char *msg = concat ("Checksum mismatch in %s", name);
     error (msg);
     free (msg);
   }
}

/*
 *   This is our malloc that checks the results.  We debug the
 *   allocations but not the frees, since memory fragmentation
 *   might be such that we can never use the free'd memory and
 *   it's wise to be conservative.  The only real place we free
 *   is when repacking *huge* characters anyway.
 */
#ifdef DEBUG
static integer totalalloc = 0;
#endif
char *
mymalloc(integer n)
{
   char *p;

#ifdef SMALLMALLOC
   if (n > 65500L)
      error("! can't allocate more than 64K!");
#endif
   if (n <= 0) /* catch strange 0 mallocs in flib.c without breaking code */
      n = 1;
#ifdef DEBUG
   totalalloc += n;
   if (dd(D_MEM)) {
#ifdef SHORTINT
      fprintf(stderr, "Alloc %ld\n", n);
#else
      fprintf(stderr, "Alloc %d\n", n);
#endif
   }
#endif
   p = (char *) malloc(n);
   if (p == NULL)
      error("! no memory");
   return p;
}
static void
morestrings(void) {
   strings = mymalloc((integer)STRINGSIZE);
   nextstring = strings;
   maxstring = strings + STRINGSIZE - 200;
   *nextstring++ = 0;
}
void
checkstrings(void) {
   if (nextstring - strings > STRINGSIZE / 2)
      morestrings();
}
/*
 *   Initialize sets up all the globals and data structures.
 */
static void
initialize(void)
{
   int i;
   const char *s;

   nextfonthd = 0;
   for (i=0; i<256; i++)
      xdig[i] = 0;
   i = 0;
   for (s="0123456789ABCDEF"; *s!=0; s++)
      xdig[(int)*s] = i++;
   i = 10;
   for (s="abcdef"; *s!=0; s++)
      xdig[(int)*s] = i++;
   for(i=0; i < DOWNLOADEDPSSIZE; i++)
      downloadedpsnames[i] = NULL;
   unused_top_of_psnames = 0;
   morestrings();
   maxpages = 100000;
   numcopies = 1;
   iname = fulliname = strings;
   bitfile = NULL;
   bytesleft = 0;
   swmem = SWMEM;
   oname = OUTPATH;
   sendcontrolD = 0;
   shiftlowchars = 0;
   multiplesects = 0;
   disablecomments = 0;
   maxdrift = -1;
   vmaxdrift = -1;
   dontmakefont = !MAKE_TEX_PK_BY_DEFAULT;
}
/*
 *   This routine copies a string into the string `pool', safely.
 */
char *
newstring(const char *s)
{
   int l;
   char *q;

   if (s == NULL)
      return(NULL);
   l = strlen(s);
   if (nextstring + l >= maxstring)
      morestrings();
   if (nextstring + l >= maxstring)
      error("! out of string space");
   strcpy(nextstring, s);
   q = nextstring;
   nextstring += l + 1;
   return(q);
}
static void
newoutname(void) {
   static int seq = 0;
   static char *seqptr = 0;
   char *p, *q;

   if (oname == 0 || *oname == 0)
      error("! need an output file name to specify separate files");
   if (*oname != '!' && *oname != '|') {
      if (seqptr == 0) {
         q = newstring(oname);
         for (p = q; *p; p++)    /* find last dot after last slash */
            if (*p == '.')
               seqptr = p + 1;
            else if (IS_DIR_SEP (*p))
               seqptr = 0;
#if !defined(MIKTEX)
#ifdef WIN32
            else if (IS_KANJI(p))
               p++;
#endif
#endif
         if (seqptr == 0)
            seqptr = p;
         nextstring += 5; /* make room for the number, up to five digits */
         oname = q;
      }
      sprintf(seqptr, "%03d", ++seq);
   }
}
/*
 *   This routine reverses a list, where a list is defined to be any
 *   structure whose first element is a pointer to another such structure.
 */
VOID *
revlist(VOID *p)
{
   struct list {
      struct list *next;
   } *pp = (struct list *)p, *qq = 0, *tt;

   while (pp) {
      tt = pp->next;
      pp->next = qq;
      qq = pp;
      pp = tt;
   }
   return (VOID *)qq;
}
/* this asks for a new set of arguments from the command line */
static void
queryargs(void)
{
   fputs("Options: ",stdout);
   fgets(queryline,256,stdin);
   qargc=1;
   if ( (qargv[1] = strtok(queryline," \n")) != (char *)NULL ) {
      qargc=2;
      while ( ((qargv[qargc] = strtok((char *)NULL," \n")) != (char *)NULL)
            && (qargc < 31) )
         qargc++;
   }
   qargv[qargc] = (char *)NULL;
}

/*
 *   Finally, our main routine.
 */
#if defined(MIKTEX)
#  define main MIKTEXCEECALL Main
#endif
#ifdef VMS
main(void)
#else
int
main(int argc, char **argv)
#endif
{
   int i, lastext = -1;
#ifdef MVSXA
   int firstext = -1;
#endif
   sectiontype *sects;

#ifdef WIN32
#if defined(KPATHSEA)
   int ac;
   char **av, *enc;
#endif
   SET_BINARY(fileno(stdin));
   SET_BINARY(fileno(stdout));
#endif

#ifdef __APPLE__ /* for Mac OS X, T. Uchiyama */
   struct rlimit rl;
   getrlimit(RLIMIT_STACK, &rl);
   rl.rlim_cur = 2048 * 1024;
   setrlimit(RLIMIT_STACK, &rl);
#endif
#ifdef KPATHSEA
   kpse_set_program_name (argv[0], "dvips");
   kpse_set_program_enabled (kpse_pk_format, MAKE_TEX_PK_BY_DEFAULT, kpse_src_compile);
#if !defined(MIKTEX)
#ifdef WIN32
   enc = kpse_var_value("command_line_encoding");
   if (get_command_line_args_utf8(enc, &ac, &av)) {
      argc = ac;
      argv = av;
   }
#endif
#endif
#endif

#ifdef __THINK__
   argc = dcommand(&argv); /* do I/O stream redirection */
#endif
#ifdef VMS		/* Grab the command-line buffer */
   short len_arg;
   $DESCRIPTOR( verb_dsc, "DVIPS ");	/* assume the verb is always DVIPS */
   struct dsc$descriptor_d temp_dsc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_D, 0};

   progname = &thh[0];
   strcpy(progname,"DVIPS%ERROR");

   lib$get_foreign( &temp_dsc, 0, &len_arg, 0);	/* Get the command line */
   str$prefix(&temp_dsc, &verb_dsc);		/* prepend the VERB     */
   len_arg += verb_dsc.dsc$w_length;		/* update the length    */
   temp_dsc.dsc$a_pointer[len_arg] = '\0';	/* terminate the string */
   gargv = &temp_dsc.dsc$a_pointer;		/* point to the buffer  */
   gargc = 1;					/* only one big argv    */
#else
   progname = argv[0];
   gargv = argv;
   gargc = argc;
/* we sneak a look at the first arg in case it's debugging */
#ifdef DEBUG
   if (argc > 1 && strncmp(argv[1], "-d", 2)==0) {
      if (argv[1][2]==0 && argc > 2) {
         if (sscanf(argv[2], "%d", &debug_flag)==0)
            debug_flag = 0;
      } else {
         if (sscanf(argv[1]+2, "%d", &debug_flag)==0)
            debug_flag = 0;
      }
   }
#if defined(KPATHSEA) && defined(KPSE_DEBUG)
   if (dd(D_FILES)) KPSE_DEBUG_SET (KPSE_DEBUG_FOPEN);
   if (dd(D_PATHS)) KPSE_DEBUG_SET (KPSE_DEBUG_PATHS);
   if (dd(D_STAT)) KPSE_DEBUG_SET (KPSE_DEBUG_STAT);
   if (dd(D_HASH)) KPSE_DEBUG_SET (KPSE_DEBUG_HASH);
   if (dd(D_EXPAND)) KPSE_DEBUG_SET (KPSE_DEBUG_EXPAND);
   if (dd(D_SEARCH)) KPSE_DEBUG_SET (KPSE_DEBUG_SEARCH);
#endif /* KPATHSEA && KPSE_DEBUG */
#endif /* DEBUG */
#ifdef KPATHSEA
   if (argc > 1) {
      if (argc == 2 && strncmp(argv[1], "-?", 2) == 0) {
         printf("%s %s\n", banner, banner2);
         help(0);
         exit(0);
      }
      if (argc == 2 && strncmp(argv[1], "-v", 2) == 0) {
         printf("%s %s\n", banner, banner2);
         exit(0);
      }
   }
#endif /* KPATHSEA */
#endif /* VMS */
   dvips_debug_flag = 0;
   { /* for compilers incompatible with c99 */
      char *s = getenv ("DVIPSDEBUG");
      if (s) {
         dvips_debug_flag = 1;
      } else {
         s = getenv ("KPATHSEA_DEBUG");
         if (s) {
           dvips_debug_flag = 1;
         }
      }
   }
   initialize();
   checkenv(0);
#if defined(MIKTEX)
   {
     std::shared_ptr<Session> session = Session::Get();
     int old_miktex_allow_all_paths = miktex_allow_all_paths;
     miktex_allow_all_paths = 1;
     std::vector<PathName> configFiles;
     PathName relPath ("dvips/config");
     relPath /= CONFIGFILE;
     if (session->FindFile(relPath.ToString(), MIKTEX_PATH_TEXMF_PLACEHOLDER, { Session::FindFileOption::All }, configFiles))
     {
       for (std::vector<PathName>::const_reverse_iterator it = configFiles.rbegin(); it != configFiles.rend(); ++it)
       {
         getdefaults(const_cast<char*>(it->GetData()));
       }
     }
     miktex_allow_all_paths = old_miktex_allow_all_paths;
   }
#else
   getdefaults(CONFIGFILE);
#endif
   getdefaults((char *)0);
/*
 *   This next whole big section of code is straightforward; we just scan
 *   the options.  An argument can either immediately follow its option letter
 *   or be separated by spaces.  Any argument not preceded by '-' and an
 *   option letter is considered a file name; the program complains if more
 *   than one file name is given, and uses stdin if none is given.
 */
#ifdef VMS
   vmscli(void);
   papsizes = (struct papsiz *)revlist((void *)papsizes); /* Added by PWD 21-Mar-1997 */
#else
   queryoptions = 0;
   do
   {
      for (i=1; i<argc; i++) {
         if (*argv[i]=='-') {
            char *p=argv[i]+2;
            char c=argv[i][1];
#ifdef KPATHSEA
            /* print information and exit if dvips finds options
               --help or --version */
            if (strlen (argv[i] + 1) == 5 && strcmp (argv[i] + 1, "-help") == 0) {
               help (0);
               exit (0);
            }
            if (strlen (argv[i] + 1) == 8 &&
                strcmp (argv[i] + 1, "-version") == 0) {
               puts (BANNER);
#if defined(MIKTEX)
               {
                  puts(MiKTeX::Core::Utils::GetMiKTeXBannerString().c_str());
               }
#else
               puts (kpathsea_version_string);
#endif
               puts ("Copyright 2020 Radical Eye Software.\n\
There is NO warranty.  You may redistribute this software\n\
under the terms of the GNU General Public License\n\
and the Dvips copyright.\n\
For more information about these matters, see the files\n\
named COPYING and dvips.h.\n\
Primary author of Dvips: T. Rokicki.");
               exit (0);
            }
#endif /* KPATHSEA */
            switch (c) {
case '-':
               queryoptions = 1;
               break;
case 'a':
               dopprescan = (*p != '0');
               break;
case 'b':
               if (strcmp(p, "itmapfontenc") == 0) {
                  p = argv[++i] ;
                  if (strcmp(p, "off") == 0) {
                     bitmapencopt(0) ; // disable bitmap font enc feature
                  } else if (strcmp(p, "on") == 0) {
                     bitmapencopt(1) ; // try to include bitmap font encs
                  } else if (strcmp(p, "strict") == 0) {
                     bitmapencopt(2) ; // issue warnings for missing encs
                  } else {
                     error(
               "! -bitmapfontenc option only supports off, on, and strict") ;
                  }
                  break ;
               }
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               if (sscanf(p, "%d", &pagecopies)==0)
                  error("! Bad number of page copies option (-b).");
               if (pagecopies < 1 || pagecopies > 1000)
                  error("! can only print one to a thousand page copies");
               break;
case 'c' :
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               if (sscanf(p, "%d", &numcopies)==0)
                  error("! Bad number of copies option (-c).");
               break;
case 'd' :
#ifdef DEBUG
               {
                  int old_debug = debug_flag;
                  static int warned_already = 0;

                  if (*p == 0 && argv[i+1])
                     p = argv[++i];
                  if (sscanf(p, "%d", &debug_flag)==0)
                     error("! Bad debug option (-d).");
                  if (debug_flag != old_debug && warned_already++ == 0) {
                     fprintf(stderr,
  "I found a debug option that was not the first argument to the dvips\n");
                     fprintf(stderr,
  "command.  Some debugging output may have been lost because of this.\n");
                  }
                  break;
               }
#else
               error("not compiled in debug mode");
               break;
#endif /* DEBUG */
case 'e' :
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               if (sscanf(p, "%d", &maxdrift)==0 || maxdrift<0)
                  error("! Bad maxdrift option (-e).");
               vmaxdrift = maxdrift;
               break;
case 'f' :
               filter = (*p != '0');
               if (filter)
                  oname = "";
               noenv = 1;
               sendcontrolD = 0;
               break;
case 'u' :
               {
                  char PSname[300];
                  if (*p == 0 && argv[i+1])
                     p = argv[++i];
                  strcpy(PSname, p);
                  if (!strchr(PSname, '.'))
                     strcat(PSname, ".map");     /* default extension */
                  if (PSname[0] == '+')
                     getpsinfo(PSname+1);
                  else
                     psmapfile = strdup(PSname); /* a cute small memory leak (just as in 'p' option handling in resident.c) */
               }
               break;
case 'h' : case 'H' :
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               if (strcmp(p, "-") == 0)
                  headers_off = 1;
               else
                  add_header(p);
               break;
case 'i':
               sepfiles = (*p != '0');
               if (sepfiles && maxsecsize == 0) {
                 maxsecsize = 1; /* default section size to one page/file */
               }
               break;
case 'j':
               partialdownload = (*p != '0');
               break;
case 'k':
               cropmarks = (*p != '0');
               break;
case 'R':
               if (*p == '0') {
                  secure = 0;
               } else if (*p == '2') {
                  secure = 2;
               } else {
                  secure = 1;
               }
               if (secure)
                  secure_option = 1; /* Never used */
               break;
case 'S':
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               if (sscanf(p, "%d", &maxsecsize)==0)
                  error("! Bad section size arg (-S).");
               break;
case 'm' :
               if (STREQ (p, "ode") && argv[i+1]) {
                 mfmode = argv[++i];
                 mfmode_option = 1;
               } else
                 manualfeed = (*p != '0');
               break;
case 'n' :
               if (STREQ (p, "oomega")) {
                 noomega = 1;
               } else if (STREQ (p, "optex")) {
                 noptex = 1;
               } else {
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
#ifdef SHORTINT
               if (sscanf(p, "%ld", &maxpages)==0)
#else        /* ~SHORTINT */
               if (sscanf(p, "%d", &maxpages)==0)
#endif        /* ~SHORTINT */
                  error("! Bad number of pages option (-n).");
               }
               break;
case 'o' :
               if (*p == 0 && argv[i+1] &&
                   (STREQ (argv[i+1], "-") || argv[i+1][0] != '-'))
                  p = argv[++i];
               oname_option = 1;
               oname = p;
               noenv = 1;
               sendcontrolD = 0;
#if defined(MSDOS) || defined(OS2)
                if (*oname && oname[strlen(oname)-1] == ':')
                    oname[strlen(oname)-1] = 0;     /* strip ':' off */
#endif
               break;
case 'O' :
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               handlepapersize(p, &hoff, &voff);
               break;
case 'T' :
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               handlepapersize(p, &hpapersize, &vpapersize);
               if (landscape) {
                  error(
              "both landscape and papersize specified; ignoring landscape");
                  landscape = 0;
               }
               lastpsizwins = 0;
               break;
case 'p' :
#if defined(MSDOS) || defined(OS2) || defined(ATARIST)
               /* check for emTeX job file (-pj=filename) */
               if (*p == 'j') {
                 p++;
                 if (*p == '=' || *p == ':')
                   p++;
                 mfjobname = newstring(p);
                 break;
               }
               /* must be page number instead */
#endif
               if (*p == 'p') {  /* a -pp specifier for a page list? */
                  p++;
                  if (*p == 0 && argv[i+1])
                     p = argv[++i];
                  if (ParsePages(p))
                     error("! Bad page list specifier (-pp).");
                  pagelist = 1;
                  break;
               }
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               if (*p == '=') {
                  abspage = 1;
                  p++;
               }
#ifdef SHORTINT
               switch(sscanf(p, "%ld.%ld", &firstpage, &firstseq)) {
#else        /* ~SHORTINT */
               switch(sscanf(p, "%d.%d", &firstpage, &firstseq)) {
#endif        /* ~SHORTINT */
case 1:           firstseq = 0;
case 2:           break;
default:
#ifdef KPATHSEA
                  error(concat3 ("! Bad first page option (-p ", p, ")."));
#else
                  error("! Bad first page option (-p).");
#endif
               }
               notfirst = 1;
               break;
case 'l':
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               if (*p == '=') {
                  abspage = 1;
                  p++;
               }
#ifdef SHORTINT
               switch(sscanf(p, "%ld.%ld", &lastpage, &lastseq)) {
#else        /* ~SHORTINT */
               switch(sscanf(p, "%d.%d", &lastpage, &lastseq)) {
#endif        /* ~SHORTINT */
case 1:           lastseq = 0;
case 2:           break;
default:
#ifdef KPATHSEA
                  error(concat3 ("! Bad last page option (-l ", p, ")."));
#else
                  error("! Bad last page option (-l).");
#endif
               }
               notlast = 1;
               break;
case 'A':
               oddpages = 1;
               break;
case 'B':
               evenpages = 1;
               break;
case 'q' : case 'Q' :
               quiet = (*p != '0');
               break;
case 'r' :
               reverse = (*p != '0');
               break;
case 't' :
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               if (strcmp(p, "landscape") == 0) {
                  if (hpapersize || vpapersize)
                     error(
             "both landscape and papersize specified; ignoring landscape");
                  else
                     landscape = 1;
               } else
                  paperfmt = p;
               break;
case 'v':
                printf ("%s %s\n", banner, banner2);
                exit (0);
                break;
case 'x' : case 'y' :
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               if (sscanf(p, "%lg", &mag)==0 || mag < 1 ||
                          mag > 1000000)
                  error("! Bad magnification parameter (-x or -y).");
               overridemag = (c == 'x' ? 1 : -1);
               break;
case 'C' :
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               if (sscanf(p, "%d", &collatedcopies)==0)
                  error("! Bad number of collated copies option (-C).");
               break;
case 'D' :
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               if (sscanf(p, "%d", &actualdpi)==0 || actualdpi < 10 ||
                          actualdpi > 10000)
                  error("! Bad dpi parameter (-D).");
               vactualdpi = actualdpi;
               /* If we had the mode from config.ps, don't use it.
                  If they specified one with -mode, keep it.  */
               if (!mfmode_option)
                 mfmode = NULL;
               break;
case 'E' :
               tryepsf = (*p != '0');
               removecomments = disablecomments = 0;
               if (tryepsf && maxsecsize == 0)
                  maxsecsize = 1;
               break;
case 'K' :
               removecomments = (*p != '0');
               break;
case 'L' :
               lastpsizwins = (*p != '0');
               break;
case 'U' :
               nosmallchars = (*p != '0');
               break;
case 'X' :
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               if (sscanf(p, "%d", &actualdpi)==0 || actualdpi < 10 ||
                          actualdpi > 10000)
                  error("! Bad dpi parameter (-D).");
               break;
case 'Y' :
               if (*p == 0 && argv[i+1])
                  p = argv[++i];
               if (sscanf(p, "%d", &vactualdpi)==0 || vactualdpi < 10 ||
                          vactualdpi > 10000)
                  error("! Bad dpi parameter (-D).");
               vactualdpi = vactualdpi;
               break;
case 'F' :
               sendcontrolD = (*p != '0');
               break;
case 'G' :
               shiftlowchars = (*p != '0');
	       break;
case 'M':
#if defined(MIKTEX)
{
  if (strcmp(p, "iKTeX:noglyphs") == 0)
  {
    miktex_no_glyphs = 1;
    break;
  }
  if (strcmp(p, "iKTeX:norules") == 0)
  {
    miktex_no_rules = 1;
    break;
  }
  if (strcmp(p, "iKTeX:nolandscape") == 0)
  {
    miktex_no_landscape = 1;
    break;
  }
  if (strcmp(p, "iKTeX:pedantic") == 0)
  {
    miktex_pedantic = 1;
    break;
  }
  if (strcmp(p, "iKTeX:allowallpaths") == 0)
  {
    miktex_allow_all_paths = 1;
    break;
  }
}
#endif
               dontmakefont = (*p != '0');
#ifdef KPATHSEA
               kpse_set_program_enabled (kpse_pk_format, !dontmakefont,
                                         kpse_src_cmdline);
#endif
               break;
case 'N' :
               disablecomments = (*p != '0');
               break;
case 'P' :
               {
                  struct papsiz *opapsiz = papsizes;
                  struct papsiz *npapsiz;
                  papsizes = 0;
                  if (*p == 0 && argv[i+1])
                     p = argv[++i];
                  printer = p;
                  noenv = 1;
                  if (!getdefaults("")) {
                    /* If no config file, default the output name.  */
                    oname = concat ("| lpr -P", printer);
                  }
                  npapsiz = opapsiz;
                  while (npapsiz && npapsiz->next)
                     npapsiz = npapsiz->next;
                  if (npapsiz) {
                     npapsiz->next = papsizes;
                     papsizes = opapsiz;
                  }
	       }
               break;
case 's':
               safetyenclose = (*p != '0');
               break;
case 'V':
               downloadpspk = (*p != '0');
               break;
case 'Z':
               compressed = (*p != '0');
               break;
#ifdef HPS
case 'z':
               HPS_FLAG = (*p != '0');
               break;
#endif
case '?':
               break; /* We print the banner and help msg below.  */
default:
#ifdef KPATHSEA
               error(concat3 ("! Invalid option `", argv[i],
                              "'. Try --help for more information."));
#else
               error(
     "! Bad option, not one of acdefhijklmnopqrstxyzABCDEFKMNOPSTUXYZ?");
#endif
            }
         } else {
            if (*iname == 0) {
               register char *p;

               lastext = 0;
               iname = nextstring;
               p = argv[i];
	       if (NAME_BEGINS_WITH_DEVICE(p)) { /* get past DOSISH drive */
		  *nextstring++ = *p++;
		  *nextstring++ = *p++;
	       }
               while (*p) {
                  *nextstring = *p++;
                  if (*nextstring == '.')
                     lastext = nextstring - iname;
                  else if (IS_DIR_SEP(*nextstring))
                     lastext = 0;
#if !defined(MIKTEX)
#ifdef WIN32
                  else if (IS_KANJI(p-1)) {
                     nextstring++;
                     *nextstring = *p++;
                  }
#endif
#endif
                  nextstring++;
               }
               *nextstring++ = '.';
               *nextstring++ = 'd';
               *nextstring++ = 'v';
               *nextstring++ = 'i';
               *nextstring++ = 0;
            } else
#ifdef KPATHSEA
               error(concat3("! Second input filename (", argv[i],
                             ") specified."));
#else
               error("! Two input file names specified.");
#endif
         }
      }
      if (noenv == 0) {
         register char *p;
         struct papsiz *opapsiz = papsizes;
         papsizes = 0;
	 if (0 != (p = getenv("PRINTER"))) {
#if defined(MSDOS) || defined(OS2)
            strcpy(nextstring, p);
            strcat(nextstring, ".cfg");
#else
            strcpy(nextstring, "config.");
            strcat(nextstring, p);
#endif
            getdefaults(nextstring);
         }
         {
            struct papsiz *npapsiz = opapsiz;
            while (npapsiz && npapsiz->next)
               npapsiz = npapsiz->next;
            if (npapsiz) {
               npapsiz->next = papsizes;
               papsizes = opapsiz;
            }
         }
      }
      papsizes = (struct papsiz *)revlist((void *)papsizes);
      if (queryoptions != 0) {            /* get new options */
         fprintf(stderr, "%s %s\n", banner, banner2);
         help(1);
         queryargs();
         if (qargc == 1)
           queryoptions = 0;
         else {
           qargv[0] = argv[0];
           argc=qargc;
           argv=qargv;
         }
      }
   } while (queryoptions != 0);
#endif
#if defined(KPATHSEA) && defined(KPSE_DEBUG) && defined(DEBUG) /* this should really be part of a subroutine */
   if (dd(D_FILES)) KPSE_DEBUG_SET (KPSE_DEBUG_FOPEN);
   if (dd(D_PATHS)) KPSE_DEBUG_SET (KPSE_DEBUG_PATHS);
   if (dd(D_STAT)) KPSE_DEBUG_SET (KPSE_DEBUG_STAT);
   if (dd(D_HASH)) KPSE_DEBUG_SET (KPSE_DEBUG_HASH);
   if (dd(D_EXPAND)) KPSE_DEBUG_SET (KPSE_DEBUG_EXPAND);
   if (dd(D_SEARCH)) KPSE_DEBUG_SET (KPSE_DEBUG_SEARCH);
#endif /* KPATHSEA && KPSE_DEBUG && DEBUG */
   checkenv(1);
#ifdef KPATHSEA
   kpse_init_prog ("DVIPS", actualdpi, mfmode, "cmr10");
   kpse_make_tex_discard_errors = quiet;
#endif
/*
 *   The logic here is a bit convoluted.  Since all `additional'
 *   PostScript font information files are loaded *before* the master
 *   one, and yet they should be able to override the master one, we
 *   have to add the information in the master list to the *ends* of
 *   the hash chain.  We do this by reversing the lists, adding them
 *   to the front, and then reversing them again.
 */
   revpslists();
   getpsinfo((char *)NULL);
   revpslists();
   if (dvips_debug_flag) {
      if (!quiet)
         fprintf(stderr, "\n%s %s\n", banner, banner2);
      prettycolumn = 0;
   } else {
      if (!quiet)
         fprintf(stderr, "%s %s\n", banner, banner2);
   }
   if (*iname) {
      dvifile = fopen(iname, READBIN);
/*
 *   Allow names like a.b.
 */
      if (dvifile == 0) {
         iname[strlen(iname)-4] = 0; /* remove the .dvi suffix */
         dvifile = fopen(iname, READBIN);
      }
   }
   if (oname[0] == '-' && oname[1] == 0)
      oname = "";
   else if (*oname == 0 && ! filter && *iname) {
      /* determine output name from input name */
      oname = nextstring;
#ifndef VMCMS  /* get stuff before LAST "." */
      lastext = strlen(iname) - 1;
      while (iname[lastext] != '.' && lastext > 0)
         lastext--;
      if (iname[lastext] != '.')
         lastext = strlen(iname) - 1;
#else   /* for VM/CMS we take the stuff before FIRST "." */
      lastext = strchr(iname,'.') - iname;
      if ( lastext <= 0 )     /* if no '.' in "iname" */
         lastext = strlen(iname) -1;
#endif
#ifdef MVSXA /* IBM: MVS/XA */
      if (strchr(iname, '(') != NULL  &&
          strchr(iname, ')') != NULL) {
      firstext = strchr(iname, '(') - iname + 1;
      lastext = strrchr(iname, ')') - iname - 1;
         }
      else {
      if (strrchr(iname, '.') != NULL) {
      lastext = strrchr(iname, '.') - iname - 1;
           }
         else lastext = strlen(iname) - 1;
      if (strchr(iname, '\'') != NULL)
         firstext = strchr(iname, '.') - iname + 1;
         else firstext = 0;
      }
#endif  /* IBM: MVS/XA */
#ifdef MVSXA /* IBM: MVS/XA */
      for (i=firstext; i<=lastext; i++)
#else
      for (i=0; i<=lastext; i++)
#endif
         *nextstring++ = iname[i];
      if (iname[lastext] != '.')
         *nextstring++ = '.';
#ifndef VMCMS
      *nextstring++ = 'p';
      *nextstring++ = 's';
#else  /* might as well keep things uppercase */
      *nextstring++ = 'P';
      *nextstring++ = 'S';
#endif
      *nextstring++ = 0;
/*
 *   Now we check the name, and `throw away' any prefix information.
 *   This means throwing away anything before (and including) a colon
 *   or slash.
 */
      {
         const char *p = NAME_BEGINS_WITH_DEVICE(oname) ? oname + 2 : oname;

         for (oname=p; *p && p[1]; p++)
            if (IS_DIR_SEP(*p))
               oname = p + 1;
#if !defined(MIKTEX)
#ifdef WIN32
            else if (IS_KANJI(p))
               p++;
#endif
#endif
      }
   }
#ifdef DEBUG
   if (dd(D_PATHS)) {
#ifdef SHORTINT
        fprintf(stderr,"input file %s output file %s swmem %ld\n",
#else /* ~SHORTINT */
           fprintf(stderr,"input file %s output file %s swmem %d\n",
#endif /* ~SHORTINT */
           iname, oname, swmem);
#ifndef KPATHSEA
   fprintf(stderr,"tfm path %s\npk path %s\n", tfmpath, pkpath);
   fprintf(stderr,"fig path %s\nvf path %s\n", figpath, vfpath);
   fprintf(stderr,"config path %s\nheader path %s\n",
                  configpath, headerpath);
#endif
#ifdef FONTLIB
   fprintf(stderr,"fli path %s\nfli names %s\n", flipath, fliname);
#endif
   } /* dd(D_PATHS) */
#endif /* DEBUG */

/*
 *   Check if oname != iname
 */
   if (iname && *iname && oname && *oname) {
#ifdef DOSISH
      if (strcasecmp (iname, oname) == 0) {
#else
      if (strcmp (iname, oname) == 0) {
#endif
         fprintf (stderr, "! Output name should be different from input name.\n");
         exit (20);
      }
   }
/*
 *   Now we try to open the dvi file.
 */
   if (!quiet && warningmsg)
      error(warningmsg);
   headersready = 1;
   headerfile = (compressed? CHEADERFILE : HEADERFILE);
   add_header(headerfile);
   if (*iname != 0) {
      fulliname = nextstring;
#ifndef IGNORE_CWD
      if (!IS_DIR_SEP(*iname) && !NAME_BEGINS_WITH_DEVICE(iname)) {
        getcwd(nextstring, MAXPATHLEN + 2);
        while (*nextstring++);
#ifdef VMS		/* VMS doesn't need the '/' character appended */
        nextstring--;	/* so just back up one byte. */
#else
        *(nextstring-1) = '/';
#endif
      }
#endif
      strcpy(nextstring,iname);
      while (*nextstring++); /* advance nextstring past iname */
   } else if (filter) {
      dvifile = stdin;
      if (O_BINARY && !isatty(fileno(stdin)))
	 SET_BINARY(fileno(stdin));
   } else {
#ifdef KPATHSEA
      fprintf (stderr, "dvips: Missing DVI file argument (or -f).\n");
      fprintf (stderr, "dvips: Try --help for more information.\n");
#else
      help(1);
#endif
      exit(1);
   }
   if (dvifile==NULL) {
      error_with_perror("DVI file can't be opened:", iname);
      exit(1);
   }
   if (fseek(dvifile, 0L, 0) < 0)
      error("! DVI file must not be a pipe.");

   initcolor();
#ifdef FONTLIB
   fliload();    /* read the font libaries */
#endif
/*
 *   Now we do our main work.
 */
   swmem += fontmem;
   if (maxdrift < 0) {
      if (actualdpi <= 599)
         maxdrift = actualdpi / 100;
      else if (actualdpi < 1199)
         maxdrift = actualdpi / 200 + 3;
      else
         maxdrift = actualdpi / 400 + 6;
   }
   if (vmaxdrift < 0) {
      if (vactualdpi <= 599)
         vmaxdrift = vactualdpi / 100;
      else if (vactualdpi < 1199)
         vmaxdrift = vactualdpi / 200 + 3;
      else
         vmaxdrift = vactualdpi / 400 + 6;
   }
   if (dopprescan)
      pprescanpages();
   prescanpages();
#if defined MSDOS || defined OS2 || defined(ATARIST)
   if (mfjobfile != (FILE*)NULL) {
     char answer[5];
     fputs("}\n",mfjobfile);
     fclose(mfjobfile);
     fputs("Exit to make missing fonts now (y/n)? ",stdout);
     fgets(answer,4,stdin);
     if (*answer=='y' || *answer=='Y')
       exit(8); /* exit with errorlevel 8 for emTeX dvidrv */
   }
#endif
   if (includesfonts)
      add_header(IFONTHEADER);
   if (usesPSfonts)
      add_header(PSFONTHEADER);
   if (usesspecial)
      add_header(SPECIALHEADER);
   if (usescolor)  /* IBM: color */
      add_header(COLORHEADER);
#ifdef HPS
   if (HPS_FLAG)
      add_header(HPSHEADER);
#endif
   sects = sections;
   totalpages *= collatedcopies;
   if (sects == NULL || sects->next == NULL) {
      sectioncopies = collatedcopies;
      collatedcopies = 1;
   } else {
      if (! sepfiles)
         multiplesects = 1;
   }
   totalpages *= pagecopies;
   if (tryepsf) {
      if (paperfmt || landscape || manualfeed ||
          collatedcopies > 1 || numcopies > 1 || cropmarks ||
          *iname == 0 ||
           (totalpages > 1 && !(sepfiles && maxsecsize == 1))) {
         error("Can't make it EPSF, sorry");
         tryepsf = 0;
      }
   }
#ifdef HPS
   if (HPS_FLAG)
      set_bitfile("head.tmp", 0);
#endif
   if (! sepfiles) {
      initprinter(sections);
      outbangspecials();
   }

   for (i=0; i<collatedcopies; i++) {
      sects = sections;
      while (sects != NULL) {
         if (sepfiles) {
            newoutname();
            if (! quiet) {
               if (prettycolumn + strlen(oname) + 6 > STDOUTSIZE) {
                  fprintf(stderr, "\n");
                  prettycolumn = 0;
               }
               fprintf(stderr, "(-> %s) ", oname);
               prettycolumn += strlen(oname) + 6;
            }
#ifdef HPS
            if (HPS_FLAG)
               set_bitfile("head.tmp", 0);
#endif
            initprinter(sects);
            outbangspecials();
         } else if (! quiet) {
            if (prettycolumn > STDOUTSIZE) {
               fprintf(stderr, "\n");
               prettycolumn = 0;
            }
            fprintf(stderr, ". ");
            prettycolumn += 2;
         }
         fflush(stderr);
         dosection(sects, sectioncopies);
         sects = sects->next;
         if (sepfiles) {
#ifdef HPS
            if (HPS_FLAG)
               finish_hps();
#endif
            cleanprinter();
	 }
      }
   }
   freememforpsnames();
   if (! sepfiles) {
#ifdef HPS
      if (HPS_FLAG)
         finish_hps();
#endif
      cleanprinter();
   }
   if (! quiet)
      fprintf(stderr, "\n");
#ifdef DEBUG
   if (dd(D_MEM)) {
#ifdef SHORTINT
      fprintf(stderr, "Total memory allocated:  %ld\n", totalalloc);
#else
      fprintf(stderr, "Total memory allocated:  %d\n", totalalloc);
#endif
   }
#endif
   return 0;
   /*NOTREACHED*/
}
#ifdef VMS
#include "[.vms]vmscli.c"
#endif

#ifdef VMCMS  /* IBM: VM/CMS */
#include "dvipscms.h"
#endif

#ifdef MVSXA  /* IBM: MVS/XA */
#include "dvipsmvs.h"
#endif
