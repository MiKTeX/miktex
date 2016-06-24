/*
 *
 *  This file is part of
 *	MakeIndex - A formatter and format independent index processor
 *
 *  Copyright (C) 1998-2012 by the TeX Live project.
 *  Copyright (C) 1989 by Chen & Harrison International Systems, Inc.
 *  Copyright (C) 1988 by Olivetti Research Center
 *  Copyright (C) 1987 by Regents of the University of California
 *
 *  Author:
 *	Pehong Chen
 *	Chen & Harrison International Systems, Inc.
 *	Palo Alto, California
 *	USA
 *
 *  Contributors:
 *	Please refer to the CONTRIB file that comes with this release
 *	for a list of people who have contributed to this and/or previous
 *	release(s) of MakeIndex.
 *
 *  All rights reserved by the copyright holders.  See the copyright
 *  notice distributed with this software for a complete description of
 *  the conditions under which it is made available.
 *
 */

#if defined(MIKTEX)
#  include <miktex/Version>
#  if defined(MIKTEX_WINDOWS)
#    define MIKTEX_UTF8_WRAP_ALL 1
#    include <miktex/utf8wrap.h>
#    include <miktex/unxemu.h>
#  endif
#else
#include <c-auto.h>
#endif

/***********************************************************************

			   INSTALLATION NOTES
			      <11-Nov-1989>

   At compile-time (or here, if compile-time definition is not
   available), set non-zero ONE OS_xxxx operating system flag, and if
   more than one compiler is available, ONE compiler flag:

    Operating Systems        Compiler(s)
    =================        ===========
    OS_ATARI
    OS_BS2000                CCD_2000
    OS_BSD
    OS_MVSXA                 IBM_C370
    OS_VMCMS                 IBM_C370
    OS_PCDOS                 IBM_PC_MICROSOFT
    OS_SYSV
    OS_TOPS20                KCC_20 or PCC_20
    OS_VAXVMS
    OS_XENIX
    =================        ===========

   If no OS_xxxx symbol is defined, OS_BSD is assumed.

   Standard C prototypes are now required.

   All function declarations in MakeIndex are contained at the end of
   this file.

   If the host system restricts external names to 6 characters, set
   SHORTNAMES non-zero in the appropriate OS_xxxx section below.

   Installing MakeIndex under BS2000 requires at least Version 8.5
   supporting Multiple Public Volume Sets (MPVS). Define WORK in this
   file to your CATID (default :w:) for temporary files (.ilg, .ind),
   in case of no MPVS support define WORK to NIL.

*/

/**********************************************************************/

/*
 * Establish needed operating symbols (defaulting to OS_BSD if none
 * specified at compile time).  If you add one, add it to the check
 * list at the end of this section, too.
 */

#define SHORTNAMES 0

/**********************************************************************/

#ifndef USE_KPATHSEA
#define USE_KPATHSEA 1
#endif

#if USE_KPATHSEA
#ifndef DEBUG       /* strange logic: makeindex uses DEBUG to force debugging */
#define NO_DEBUG 1  /* kpathsea uses NO_DEBUG to suppress it.                 */
#endif
#include <kpathsea/config.h>
#include <kpathsea/c-limits.h>
#include <kpathsea/c-memstr.h>
#include <kpathsea/magstep.h>
#include <kpathsea/proginit.h>
#include <kpathsea/progname.h>
#include <kpathsea/tex-glyph.h>
#include <kpathsea/expand.h>
#endif

#include    <stdio.h>

#if defined(MIKTEX)
#  define exit(status) throw(status)
#endif

#ifdef HAVE_CTYPE_H
#include    <ctype.h>
#endif

#ifndef HAVE_STRCHR
#define strchr  index                  /* convert STDC form to old K&R form */
#endif

#ifndef HAVE_STRRCHR
#define strrchr rindex                 /* convert STDC form to old K&R form */
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <string.h>
#endif
#endif

#ifdef HAVE_TYPES_H
#include <types.h>
#endif

#if    SHORTNAMES
/*
 * Provide alternate external names which are unique in the first SIX
 * characters as required for portability (and Standard C)
 */
#define check_all          chk_all
#define check_idx          chk_idx
#define check_mixsym       chk_mix
#define compare_one        cmp_one
#define compare_page       cmp_page
#define compare_string     cmp_string
#define delim_n            dlm_n
#define delim_p            dlm_p
#define delim_r            dlm_r
#define delim_t            dlm_t
#define encap_i            ecp_i
#define encap_p            ecp_p
#define encap_range        ecp_range
#define encap_s            ecp_s
#define group_skip         grp_skip
#define group_type         grp_type
#define idx_aclose         idxaclose
#define idx_actual         idxactual
#define idx_keyword        idx_kwd
#define indent_length      ind_length
#define indent_space       ind_space
#define headings_flag      hd_flag
#define heading_pre        hd_pre
#define heading_suf        hd_suf
#define symhead_pos        sym_pos
#define symhead_neg        sym_neg
#define numhead_pos        num_pos
#define numhead_neg        num_neg
#define process_idx        prc_idx
#define process_precedence prc_pre
#define range_ptr          rng_ptr
#define scan_alpha_lower   scnalw
#define scan_alpha_upper   scnaup
#define scan_arabic        scnarabic
#define scan_arg1          scna1
#define scan_arg2          scna2
#define scan_char          scnchr
#define scan_field         scnfld
#define scan_idx           scnidx
#define scan_key           scnkey
#define scan_no            scnno
#define scan_roman_lower   scnrlw
#define scan_roman_upper   scnrup
#define scan_spec          scnspc
#define scan_string        scnstr
#define scan_style         scnsty
#define setpagelen         spg_len
#define setpage_close      spg_close
#define setpage_open       spg_open
#define suffix_2p          suf_2p
#define suffix_3p          suf_3p
#define suffix_mp          suf_mp
#endif                                 /* SHORTNAMES */

#define EXIT exit

#undef TRUE
#define TRUE 1

#undef FALSE
#define FALSE 0

#undef NUL
#define NUL '\0'

#undef NIL
#define NIL ""

#define TAB '\t'
#define LFD '\n'
#define SPC ' '
#define LSQ '['
#define RSQ ']'
#define BSH '\\'

#ifdef DOSISH
#define ENV_SEPAR ';'
#define DIR_DELIM '/'
#define ALT_DIR_DELIM '\\'
#define IS_DIR_DELIM(c) ((c) == DIR_DELIM || (c) == ALT_DIR_DELIM)
#else  /* not DOSISH */
#define ENV_SEPAR ':'
#define DIR_DELIM '/'
#define IS_DIR_DELIM(c) ((c) == DIR_DELIM)
#endif /* not DOSISH */

#ifndef SW_PREFIX			/* can override at compile time */
#define SW_PREFIX  '-'
#endif

#define EXT_DELIM  '.'
#define ROMAN_SIGN '*'
#define EVEN       "even"
#define ODD        "odd"
#define ANY        "any"

extern int mk_getc (FILE *str);         /* line endings patch */
#define GET_CHAR mk_getc

#define TOASCII(i) (char)((i) + 48)

/* NB: The typecasts here are CRITICAL for correct sorting of entries
   that use characters in the range 128..255! */
#define TOLOWER(C) (isupper((unsigned char)(C)) ? \
	(unsigned char)tolower((unsigned char)(C)) : (unsigned char)(C))
#define TOUPPER(C) (isupper((unsigned char)(C)) ? \
	(unsigned char)(C) : (unsigned char)toupper((unsigned char)(C)))

#if USE_KPATHSEA /* kpathsea defines STREQ */
#undef STREQ
#undef STRNEQ
#endif

#define STREQ(A, B)  (strcmp(A, B) == 0)
#define STRNEQ(A, B) (strcmp(A, B) != 0)

#define MESSAGE(F) { \
    if (verbose) \
	fprintf(stderr, F); \
    fprintf(ilg_fp, F); \
}

#define MESSAGE1(F, S) { \
    if (verbose) \
	fprintf(stderr, F, S); \
    fprintf(ilg_fp, F, S); \
}

#if USE_KPATHSEA /* kpathsea defines a different FATAL */
#undef FATAL
#endif

#define FATAL(F) { \
    fprintf(stderr, F); \
    fprintf(stderr, USAGE, pgm_fn); \
    EXIT(1); \
}

#if USE_KPATHSEA /* kpathsea defines a different FATAL1 */
#undef FATAL1
#endif

#define FATAL1(F, S) { \
    fprintf(stderr, F, S); \
    fprintf(stderr, USAGE, pgm_fn); \
    EXIT(1); \
}

#if USE_KPATHSEA /* kpathsea defines a different FATAL2 */
#undef FATAL2
#endif

#define FATAL2(F, D1, D2) { \
    fprintf(stderr, F, D1, D2); \
    fprintf(stderr, USAGE, pgm_fn); \
    EXIT(1); \
}

#define OPEN_IN(FP)   fopen(FP, "rb")
#define OPEN_OUT(FP)  fopen(FP, "wb")
#define CLOSE(FP)     fclose(FP)

#define ISDIGIT(C)  ('0' <= C && C <= '9')
#define ISSYMBOL(C) (('!' <= C && C <= '@') || \
		     ('[' <= C && C <= '`') || \
		     ('{' <= C && C <= '~'))

/*====================================================================
Many arrays in MakeIndex are dimensioned [xxx_MAX], where the xxx_MAX
values are defined below.  The use of each of these is described in
comments.  However, no run-time check is made to ensure that these are
consistent, or reasonable!  Therefore, change them only with great
care.

The array sizes should be made generously large: there are a great
many uses of strings in MakeIndex with the strxxx() and sprintf()
functions where no checking is done for adequate target storage sizes.
Although some input checking is done to avoid this possibility, it is
highly likely that there are still several places where storage
violations are possible, with core dumps, or worse, incorrect output,
ensuing.
======================================================================*/

#define ARABIC_MAX    99	/* maximum digits in an Arabic page */
				/* number field */

#define ARGUMENT_MAX  1024	/* maximum length of sort or actual key */
				/* in index entry */

#define ARRAY_MAX     1024	/* maximum length of constant values in */
				/* style file */

#define FIELD_MAX	3	/* maximum levels of index entries (item, */
				/* subitem, subsubitem); cannot be */
				/* increased beyond 3 without */
				/* significant design changes (no field */
				/* names are known beyond 3 levels) */

#ifdef LINE_MAX		/* IBM RS/6000 AIX has this in <sys/limits.h> */
#undef LINE_MAX
#endif
#define LINE_MAX      72	/* maximum output line length (longer */
				/* ones wrap if possible) */

#define NUMBER_MAX    99	/* maximum digits in a Roman or Arabic */
				/* page number */
				/* (MAX(ARABIC_MAX,ROMAN_MAX)) */

#define PAGEFIELD_MAX 10	/* maximum fields in a composite page */
				/* number */

#define PAGETYPE_MAX  5		/* fixed at 5; see use in scanst.c */

#define ROMAN_MAX     99	/* maximum length of Roman page number */
				/* field */

#define STRING_MAX    999	/* maximum length of host filename */

/*====================================================================*/

#if USE_KPATHSEA
#if defined(MIKTEX)
#define VERSION       "version 2.15 [" MIKTEX_BANNER_STR "] (kpathsea + Thai support)"
#else
#define VERSION       "version 2.15 [TeX Live " TEX_LIVE_VERSION "] (kpathsea + Thai support)"
#endif
#else
#define VERSION       "version 2.15 [20-Nov-2007] (with Thai support)"
#endif

#define PUT_VERSION { \
    MESSAGE1("This is %s, ", pgm_fn); \
    MESSAGE1("%s.\n", VERSION); \
    need_version = FALSE; \
}

#ifdef HAVE_SETLOCALE
#define USAGE \
   "Usage: %s [-ilqrcgLT] [-s sty] [-o ind] [-t log] [-p num] [idx0 idx1 ...]\n"
#else
#define USAGE \
   "Usage: %s [-ilqrcg] [-s sty] [-o ind] [-t log] [-p num] [idx0 idx1 ...]\n"
#endif

#define STYLE_PATH "INDEXSTYLE"	/* environment variable defining search */
				/* path for style files */
#define INDEX_IDX  ".idx"
#define INDEX_ILG  ".ilg"
#define INDEX_IND  ".ind"
#define INDEX_STY  ".mst"

#define INDEX_LOG  ".log"

#define EMPTY     -9999
#define ROML      0
#define ROMU      1
#define ARAB      2
#define ALPL      3
#define ALPU      4
#undef DUPLICATE /* may already be defined for WIN32 */
#define DUPLICATE 9999

#define SYMBOL -1
#define ALPHA  -2

#define GERMAN 0

typedef struct KFIELD
{
    char    *sf[FIELD_MAX];		/* sort key */
    char    *af[FIELD_MAX];		/* actual key */
    int     group;			/* key group */
    char    lpg[NUMBER_MAX];		/* literal page */
    int     npg[PAGEFIELD_MAX];		/* page field array */
    short   count;			/* page field count */
    short   type;			/* page number type */
    char    *encap;			/* encapsulator */
    const char    *fn;			/* input filename */
    int     lc;				/* line number */
}	FIELD, *FIELD_PTR;

typedef struct KNODE
{
    FIELD   data;
    struct KNODE *next;
}	NODE, *NODE_PTR;

extern int letter_ordering;
extern int compress_blanks;
extern int init_page;
extern int merge_page;
extern int even_odd;
extern int verbose;
extern int german_sort;
extern int thai_sort;
extern int locale_sort;

extern char idx_keyword[ARRAY_MAX];
extern char idx_aopen;
extern char idx_aclose;
extern char idx_level;
extern char idx_ropen;
extern char idx_rclose;
extern char idx_quote;
extern char idx_actual;
extern char idx_encap;
extern char idx_escape;

extern char page_comp[ARRAY_MAX];
extern int page_offset[PAGETYPE_MAX];

extern char preamble[ARRAY_MAX];
extern char postamble[ARRAY_MAX];
extern char setpage_open[ARRAY_MAX];
extern char setpage_close[ARRAY_MAX];
extern char group_skip[ARRAY_MAX];
extern int headings_flag;
extern char heading_pre[ARRAY_MAX];
extern char heading_suf[ARRAY_MAX];
extern char symhead_pos[ARRAY_MAX];
extern char symhead_neg[ARRAY_MAX];
extern char numhead_pos[ARRAY_MAX];
extern char numhead_neg[ARRAY_MAX];
extern int prelen;
extern int postlen;
extern int skiplen;
extern int headprelen;
extern int headsuflen;
extern int setpagelen;

extern char item_r[FIELD_MAX][ARRAY_MAX];
extern char item_u[FIELD_MAX][ARRAY_MAX];
extern char item_x[FIELD_MAX][ARRAY_MAX];
extern int ilen_r[FIELD_MAX];
extern int ilen_u[FIELD_MAX];
extern int ilen_x[FIELD_MAX];

extern char delim_p[FIELD_MAX][ARRAY_MAX];
extern char delim_n[ARRAY_MAX];
extern char delim_r[ARRAY_MAX];
extern char delim_t[ARRAY_MAX];

extern char suffix_2p[ARRAY_MAX];
extern char suffix_3p[ARRAY_MAX];
extern char suffix_mp[ARRAY_MAX];

extern char encap_p[ARRAY_MAX];
extern char encap_i[ARRAY_MAX];
extern char encap_s[ARRAY_MAX];

extern int linemax;
extern char indent_space[ARRAY_MAX];
extern int indent_length;

extern FILE *idx_fp;
extern FILE *sty_fp;
extern FILE *ind_fp;
extern FILE *ilg_fp;

extern const char *idx_fn;
extern const char *pgm_fn;
extern const char *ind_fn;
extern const char *ilg_fn;

#ifndef MKIND_C
extern char sty_fn[];
extern char ind[];
extern char ilg[];
extern char pageno[];

#ifdef DEBUG
extern long totmem;
#endif /* DEBUG */

#endif

extern FIELD_PTR *idx_key;
extern NODE_PTR head;
extern NODE_PTR tail;

extern int idx_dot;
extern int idx_tt;
extern int idx_gt;
extern int idx_et;
extern int idx_dc;

#define DOT     "."
#define DOT_MAX 1000
#define CMP_MAX 1500

#define IDX_DOT(MAX) { \
    idx_dot = TRUE; \
    if (idx_dc++ == 0) { \
	if (verbose) \
	    fprintf(stderr, DOT); \
	fprintf(ilg_fp, DOT); \
    } \
    if (idx_dc == MAX) \
	idx_dc = 0; \
}

#define ALL_DONE { \
    if (fn_no > 0) { \
	if (verbose) \
fprintf(stderr, \
	"Overall %d files read (%d entries accepted, %d rejected).\n", \
	fn_no+1, idx_gt, idx_et); \
fprintf(ilg_fp, \
	"Overall %d files read (%d entries accepted, %d rejected).\n", \
	fn_no+1, idx_gt, idx_et); \
    } \
}

#define DONE(A, B, C, D) { \
    if (verbose) \
	fprintf(stderr, "done (%d %s, %d %s).\n", (A), B, C, D); \
    fprintf(ilg_fp, "done (%d %s, %d %s).\n", (A), B, C, D); \
}

extern void gen_ind (void);
extern int group_type (const char *str);
extern void scan_idx (void);
extern void scan_sty (void);
extern void sort_idx (void);
extern int strtoint (char *str);

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if __NeXT__
int	access (const char *, int);
#else
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#ifndef    R_OK
#define R_OK 4                         /* only symbol from sys/file.h */
#endif

#if defined(MIKTEX)
#  if defined (_MSC_VER)
#    define HAVE_SPRINTF_S 1
#    define SPRINTF_S sprintf_s
#    define HAVE_STRCPY_S 1
#    define STRCPY_S(dest, sizeofdest, src) \
       strcpy_s(dest, sizeofdest, src)
#    define STRCPY(dest, src) \
       STRCPY_S(dest, sizeof(dest) / sizeof(dest[0]), src)
#  endif
#endif
