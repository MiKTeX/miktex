/*  $Id: devnag.c,v 1.15 2008-03-09 15:57:59 icebearsoft Exp $
    Version 2.15

 *
 Preprocessor for Devanagari for TeX package
 Copyright (C) 1991-1998  University of Groningen, The Netherlands
 *
 Author   : Frans J. Velthuis <velthuis@rc.rug.nl>
 Date     : 09 May 1991
 *
 The maintainer of this program is now Zdenek Wagner <zdenek.wagner@gmail.com>.
 *
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 1, or (at your option)
 any later version.
 *
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 *
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 Modified by Marc Csernel, March 1998, to support LaTeX commands
 with arguments and environments between the dn delimiters.
 */

/*
 Version 2.0 of devnag is the first major revision of the program
 since its original release. It is based on version 1.6 (there are
 no releases 1.7, 1.8 or 1.9), and, in addition to Marc Csernel's
 LaTeX extensions, it incorporates the following modifications made
 by John Smith in 1998-9:

 1.  A major change has been made to the way in which the
     preprocessor handles "@" commands. Previously it was only
     possible to use these at the start of the .dn file; now they may
     be placed anywhere in the text, provided that they do not occur
     within passages of Devanagari (where "@" has its own meaning).
     New "negative" commands have been added to reverse the effect of
     most existing commands: thus it is now possible to enable or
     disable specific features for specific passages of text. @hindi
     may be disabled with @sanskrit; @dollars with @nodollars or
     @dolmode0; @hyphen with @nohyphen; @tabs (a new LaTeX feature
     added by Marc Csernel) with @notabs. Since "@" is a perfectly
     legal character in TeX, lines beginning with "@" that do not
     match any valid "@" command are flagged with a warning, but
     processing of the file continues. (In the somewhat unlikely
     event that you actually need to have a line of TeX text
     consisting exactly of, say, "@hindi", the preprocessor may be
     fooled by typing "{}@hindi", "{@hindi}", or anything similar.)

 2.  The old line-splitting code and the @obeylines preprocessor
     command to disable it have been withdrawn, so that the program
     now outputs one line to the .tex file for every line of the
     .dn file. In devnag 2.0, attempted use of @obeylines produces
     a specific warning; this is likely to be withdrawn in a future
     release, after which attempts to use it will cause "@obeylines"
     to be typeset as a part of the text of the document (see 1.
     above).

 3.  To complete the line-by-line correspondence between the input
     and output files, comments are no longer stripped out, and
     preprocessor commands (such as "@hyphen") are passed through
     into the output file in the form of comments ("%@hyphen").

 4.  There is now an option to show the version of the program. If
     devnag is invoked as "devnag -v" it will print a banner giving
     the version number and other information, and then exit.

 5.  If the input file has a name such as "x.y.dn", the output file
     will now be "x.y.tex", not, as previously, "x.tex". In addition,
     it is now mandatory for the files to have the suffixes ".dn" and
     ".tex", though as before these are supplied by the program if
     omitted by the user. This change has been made for safety
     reasons: formerly, typing "devnag myfile.tex" would delete
     the contents of myfile.tex.

 6.  The sequence ~nj~na now produces the correct form.

 7.  The sequences r.l and r.L now produce the correct forms.

 8.  The sequences ttrya, ttrva, khrya and khrva now produce the
     correct forms.

 9.  The correct "caret" form of -r is now produced under kh, ch,
     etc., even when a subscript vowel or viraama follows. This has
     necessitated a new macro (\qc), which must be defined in
     dnmacs.tex (for Plain TeX) or dev.sty (for LaTeX). The
     definition is:

     \def\qc#1#2{\setbox0=\hbox{#1}#1\subscr{\char126\kern1.5ex\lower1.25ex
     \hbox{\char#2}\kern-1.5ex}}

     Suitable versions of the files are distributed with the current
     release of the devnag package.

 10. A fragment of code which permitted hyphenation in the middle of
     a consonant sequence containing a viraama has been removed.
     All hyphenation before consonants stopped with viraama has
     also been eliminated.

 11. A change has been introduced in the output that devnag produces
     from input such as {\dn .sa.t"siraa.h}, i.e. in cases where an
     i-matra vowel is associated with a consonant sequence containing
     a viraama. The previous behaviour was to treat the consonant
     sequence as if it were a normal conjunct by placing the vowel
     symbol before the sequence as a whole. The majority opinion
     seems to be that this is undesirable, and that the vowel symbol
     should follow the consonant to which the viraama is attached.
     A change has therefore been made to implement this. However, the
     previous behaviour can be reinstated by means of a new
     preprocessor command, @vconjuncts.

 12. Tab characters in the input file, which previously were treated
     as fatal errors, are now silently converted to spaces.

 13. (Internal coding changes) The order of the C code has been
     normalised from the original Pascal-like bottom-up sequence
     to a more conventional top-down version. Numerous comments have
     been added to assist potential future developers in understanding
     the way the program works. A number of bug-fixes and other
     improvements have been applied. The insecure gets() calls used
     in prompting for user input have been replaced by fgets().
     Variables used only within a single function have been declared
     local to that function.  Some variables have been renamed in
     the interests of clarity and consistency, and some redundant
     ones have been eliminated. The functions ch_find() and st_find(),
     which simply duplicated the standard library functions strchr()
     and strstr(), have also been eliminated.
 */

/*
 Version 2.01 is a bugfix release, containing two changes in
 coding. (1) Previous versions of devnag have output "[" and "]"
 for Devanagari half-"sa and half-gha. Under some circumstances
 this can interact badly with LaTeX, and the form of output has
 been changed to avoid the problem. (2) \cline has been added to
 the list of LaTeX commands which devnag knows how to handle.
 -- John Smith
 */

/*
 Version 2.02 redefines all ^^* characters to be output as
 \3XXw because it is more robust. It puts a definition
 of \DevnagVersion at the beginning of the output file so
 that the new preprocessed files can be automatically recognized
 (Z. Wagner, zdenek.wagner@gmail.com, http://icebearsoft.euweb.cz).
 This version also defines a new preprodessor directive
 @modernhindi, which functions in the same way as @hindi but
 uses far fewer Sanskrit-style ligatures, preferring conjuncts
 built from half-consonant forms wherever possible. (Thanks to
 Raymond Doctor for helpful advice on which ligatures to permit
 and which to suppress.) The definition of @sanskrit has been
 changed so that it now restores all the ligatures disabled by
 @modernhindi, not just those disabled by @hindi. The sequences
 kk.sa, .tk.sa, dmya and hmya have been modified to produce
 k+k.sa, .t+k.sa, d+mya, h+mya (rather than, as previously,
 kk+.sa, .tk+.sa dm+ya, hm+ya). The code which handles all such
 substitutions has been radically improved. The special warning
 message  prompted by the long-defunct preprocessor directive
 @obeylines has been removed.
 -- John Smith
 */

/*
 Version 2.1 makes the following changes:
 *
 * Material within {\dn ... } (or $ ... $) may be enclosed within
   angle brackets < ... >; it is then not processed, but is passed
   through verbatim to the output file, e.g.
     {\dn eka do <\hrule width 10 em> tiina caara}
 * Changes to the fonts have made an "open ya" glyph available. This
   is now used wherever the preprocessor would previously have
   produced consonant + viraama + ya.
 * The fonts now contain defined glyphs for .n.na; the preprocessor
   uses these.
 * In previous releases the "half j~na" glyph was not used in @hindi
   and @modernhindi modes (the program output "half ja + half ~na").
   This has been corrected.
 * The output now avoids using the following control symbols: \", \#,
   \$, \%, \&, \'.
 -- John Smith
 */

/*
 Version 2.11 is a bugfix release to deal with two problems:
 *
 * Newline characters were not being properly handled in material
   enclosed within angle brackets < ... >;
 * The input sequence ~a was producing the wrong output for the
   initial form of the vowel.
 -- John Smith
 */

/*
 Version 2.12 contains two modification by Kevin Carmody, one to add
 the ligature disabling character "+", and one bug fix to avoid the use
 of non-standard syntax in invocations of fixconj().
 */

/*
 Bug fix suggested by John Smith: put_syll() just after case '<' in order
 to emit pending syllable. The verbatim material should appear between
 words but you can even write {\dn pa<\kern -2pt >de} if you know what
 you are doing. However, such use is strongly discouraged.
 */

/*
 Version 2.13: ~m is now a synonym for /, i.e. candrabindu
*/

/*
 Version 2.14 contains the following modifications:

 Accented characters with codes >= 128 are allowed in \dn texts
 within {\rm ...} and <...>
 UTF-8 is not yet supported.

 The filename extensions are not fixed but have reasoanble defaults
 defined by constants DEFAULT_SRC_EXT (= ".dn") and
 DEFAULT_DEST_EXT (= ".tex"). It is explicitly forbidden for the source
 file to have the extension defined by DEFAULT_DEST_EXT. The file names
 are examined as follows:

 1. If the source filename does not have the default extension, it is
    appended. If such a file does not exist, the preprocessor tries
    again with the original file name.

 2. If the destination filename was given, its extension is checked.
    It is explicitly forbidden for the destination file to have the
    extension defined by DEFAULT_SRC_EXT. If the filename is equal
    to the name of the source file, DEFAULT_DEST_EXT is appended,
    otherwise the file name is used as is.

 3. If the destination filename was not given, it is based upon the
    source filename. If the source filename contains the
    DEFAULT_SRC_EXT extension, it is stripped of. Afterwards the
    DEFAULT_DEST_EXT extension is appended.

 The algorithm was designed to prevent typing errors but it is far
 from foolproof. The filesystem properties are not examined and all
 filename tests are case insensitive. The file names are not expanded.
 For instance, if the working directory is "/some/path", then "x.y",
 "./x.y", "../path/x.y" as well as "/some/path/x.y" refer to the same
 file but they will be treated as different by the preprocessor.
 Moreover, the algorithm does not try to follow symlinks neither does it
 examine inodes in order to discover hard links.

 Notes for compilation:
 Some C libraries do not contain strcasecmp but contain either stricmp
 or strcmpi (or both) with the same syntax. If it is the case of your
 compiler, use -DHAS_STRCMPI or -DHAS_STRICMP. GCC/EMX for OS/2
 requires -DHAS_STRICMP, gcc 3.3.3 in Linux as well as gcc 2.95.2
 from mingw (Windows) know strcasecmp.

 The preprocessor can read files with any line endings on all platforms
 (i.e. DOS CRLF, UNIX LF as well as Macintosh CR). More precisely: each
 CR followed by LF is ignored and each CR followed by anything else is
 treated as end of line. The line endings of the output file always
 conform to conventions used on the operating system where the
 preprocessor runs.
*/

/*
 Modifications in version 2.15:

 \def\DevnagVersion is inserted at the beginning of the second line
 if the first line starts with %&

 Error and warning messages written to stderr instead of stdout
 (requested by Karl Berry)
*/

const char *version = "2.15";

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#  include <miktex/unxemu.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>                       /* Marc Csernel */

/* General constants */
#define TRUE 1
#define FALSE 0
#define VIRAAM 94
#define SMALLBUF 30
#define MEDBUF 512
#define MAXBUF 2048
#define N_NOLIGS (sizeof nolig / sizeof nolig[0])
#define N_MNOLIGS (sizeof modnolig / sizeof modnolig[0])
#define ill_char  29
#define dummy 30
#define end_of_file 30
#define end_of_line 31

/*
 * Constants used in char_def structs
 */
/* ch_typ constants */
#define illegal 0
#define cmr 1
#define control 2
#define dn 3
#define numeral 4
/* ch_subtyp constants */
#define lo_vowel 0
#define hi_vowel 1
#define consonant 2
#define special 3

/* Constants representing symbols: defined as indexes into out_string[] */
#define LBRACE 273
#define RBRACE 264
#define RE 263             /* \thinspace on switching from cmr to dn */
#define RDT 266            /* raised r-dot (repha + anusvara) */
#define RN 265             /* numeral, using cmr or dn as appropriate */
#define RS 256             /* \thinspace on switching from dn to cmr */

/* Default extensions */
#define DEFAULT_SRC_EXT ".dn"
#define DEFAULT_DEST_EXT ".tex"

/*
Some C libraries do not have strcasecmp but have either strcmpi or stricmp
with the same syntax. Select the one you have.
*/
#ifdef HAS_STRCMPI
#define strcasecmp(s1,s2) strcmpi(s1,s2)
#elif HAS_STRICMP
#define strcasecmp(s1,s2) stricmp(s1,s2)
#endif

/* Global variable declarations */
FILE *f_in, *f_out;
char *p_in;
int charbuf, wasCR = FALSE, charpresent = FALSE;
int number;
char symbol;
char infil[MEDBUF], outfil[MEDBUF];
char inbuf[MAXBUF], outbuf[MAXBUF];
char word[MEDBUF];
short syll[SMALLBUF];
int n_halves, buf_idx;
short half_codes[SMALLBUF];
short joincode;
unsigned char hindi_mode, mhindi_mode, dollar_mode, d_found;
unsigned char tabs_mode;                   /* Marc Csernel */
unsigned char no_dn, wait_syll, do_hyphen, do_vconjuncts;
unsigned char cons_seen, vow_seen, front_r, cmr_mode, num_mode;
unsigned char lig_block;                   /* Kevin Carmody */
short chr_idx, cons_code;
const char *banner =
"Preprocessor for Devanagari for TeX package\n\
Copyright (C) 1991-1998  University of Groningen, The Netherlands\n\
Author     : Frans J. Velthuis <velthuis@rc.rug.nl>\n\
Maintainer : Zdenek Wagner <zdenek.wagner@gmail.com>";

/* Function declarations */
int main(int argc, char **argv);
void dnproc(void);
void put_ch(short code);
void sendchar(char c);
void put_sym(short code);
void put_word(void);
void put_syll(void);
void tst_half(void);
void put_macro(short macro);
void err_ill(const char *str);
#if defined(MIKTEX)
inline void err_ill(char ch)
{
  char str[2];
  str[0] = ch;
  str[1] = 0;
  err_ill(str);
}
#endif
char inp_ch(void);
void expand(void);
char find_dn(void);
int sindex(int i, short t);
void fixconj(short *wrong, short *right);
char *fgetline(char *buf, int n, FILE *f);

/* --------- Addition by Marc Csernel 1998 --------- */

int citation;
int optchapter;
int linenumber;
int test_command(void);
char test_sub_com(void);
char comm_double_args(void);
char comm_special(void);
char comm_opt(void);
char comm_chapter(void);
char comm_cite(void);
char *getsubarg(void);
char comm_begin(void);
char ignore(void);

/* ------------------ End Addition ----------------- */

/*
 * *** Struct declarations: major data types ***
 */

/*
 A struct of type char_def contains the following information about
 any given character:

 ch_typ:     dn      -- a character of the Devanagari syllabary
	     cmr     -- a Roman character available within {\dn }
	     numeral -- self-evident: either Devanagari or ASCII as
			specified
	     control -- special TeX character ("\", "{", "}", etc.)
	     illegal -- none of the above
 ch_subtyp:  consonant
	     lo_vowel (subscript)
	     hi_vowel ("regular" and superscript)
	     special  -- none of the above
 ch_code:    For ch_typ dn only, gives the octal value of the
	     character's position in the dvng fonts. For vowels,
	     refers to the initial form (ch_subcode refers to
	     non-initial form); composite forms such as initial "o"
	     (= initial "a" + non-initial "o") have special values
	     greater than 257 decimal.
 ch_subcode: Used for ch_typ dn only. For vowels, gives the octal
	     value of the non-initial form ("a" has the special value
	     257 decimal). For consonants, gives the index into
	     cons_table[] where the "half-form" of the consonant is
	     defined.
*/
struct  char_def {
   short ch_typ, ch_subtyp, ch_code, ch_subcode;
};

/*
 A struct of type cons_joins contains the following information about
 any given consonant or consonant-group Ca:

 n_ligs:   The number of "simple" (two-member) ligatures listed in
	   lig_table[] for Ca.
 lig_code: The index into lig_table[] of the first such ligature.
 r_type:   Has the value 0 if Ca takes the normal form of following "r"
	   (decimal 125) or if "Cra" is a predefined ligature; 1 if Ca
	   requires the "caret" form of following "r" (decimal 126);
	   2 if "Cra" is handled as "\qb{Ca}".
 j_code:   Gives the octal value of the "half-form" of Ca if one exists,
	   otherwise 0.
 */
struct cons_joins {
   short n_ligs, lig_code;
   short r_type, j_code;
};

/*
 A struct of type ligs contains the following information about any
 given ligature:

 sym_code: The index into char_table[] of the final member of the
           ligature. May be expressed as as a character constant
           (e.g. 't') or as a number (e.g. 6, referring to ".s").
 sym_new:  Gives either (a) the octal value of the ligature, or (b) a
	   negative number to be handled by expand(), which will reverse
	   the sign and subtract 1 (-5 becomes 4), and use the result as
	   an index into r_ligs[]. (This is done for "ktra", "gra",
	   ".dra", "dra", "dhra" and ".s.tra".)
 join_idx: Gives the index into cons_table[] of the ligature, or 0.
 in_use:   Boolean TRUE or FALSE according to whether the ligature is
	   currently enabled (they are all initially enabled in
	   lig_table[], but may be disabled by e.g. @hindi).
 */
struct ligs {
   char sym_code;
   short sym_new, join_idx;
   unsigned char in_use;
};

/*
 * *** Data structures used by devnag ***
 */

/* Ligatures to be disabled by @hindi */
short nolig[] = {
   2,3,6,7,16,25,27,34,35,39,40,46,47,49,56,58,59,62,65,
   78,81,82,84,85,86,87,88,89,90,91,92,93,94,95,98,99,
   100,101,105
};

/* Ligatures to be disabled by @modernhindi */
short modnolig[] = {
   0,1,2,3,4,6,7,9,10,11,12,14,16,17,18,19,20,21,22,23,
   24,25,26,27,28,29,30,31,32,33,34,35,36,39,40,41,42,43,
   44,45,46,47,48,49,50,52,53,54,56,58,59,62,64,65,69,70,
   71,72,73,74,75,76,77,78,80,81,82,84,85,86,87,88,89,90,91,
   92,93,94,95,98,99,100,101,102,103,104,105,118,119
};

/*
 * ligidxs[] maps the ligature-numbers that the user may specify
 * with @lig and @nolig to the appropriate entries in lig_table[]
 */
short ligidxs[] = {
   0,1,2,3,4,6,7,9,11,12,13,14,118,16,17,18,19,20,21,22,
   23,24,25,26,27,28,29,30,31,32,33,34,35,36,38,39,40,41,
   42,43,44,45,46,47,48,49,50,52,53,119,54,55,56,58,59,60,
   61,62,63,64,65,66,68,69,70,71,72,73,74,75,76,77,78,80,
   81,82,84,85,86,87,88,89,90,91,92,93,94,95,96,98,99,100,
   101,102,104,117,105,106,107,108,109,110,111,112,113
};

/*
 * In the following character sets, final space is a dummy value
 */
/* Characters that may be preceded by "." */
char chset1[] = {
   'k','t','d','o','n','s','r','g',
   'h','a','.','K','T','D','l','L','R','m',' '
};
/* Characters with a special meaning if preceded by "a" */
char chset2[] = {'a','i','u',' '};
/* Characters that may be preceded by '"' */
char chset3[] = {'n','s',' '};
/* Characters that may be followed by "h" */
char chset4[] = {'k','g','c','j','t','d','p','b','R',' '};
/* Replacements for chset2 characters when preceded by "a" */
char chset5[] = {'A','E','O'};
/* Characters that may be preceded by "~" */
char chset6[] = {'n','o','a','r','m',' '};

/*
 * Private character set used internally by devnag -- basically
 * ASCII, but with special characters allotted slots below 32:
 * conversion from input representation done by dnproc().
 */
struct char_def char_table[] = {
     {illegal,special,0,0},                /* 1 not used     */
     {dn,consonant,'\126',34},             /* 2 .t           */
     {dn,consonant,'\130',36},             /* 3 .d           */
     {dn,special,'\72',0},                 /* 4 .o           */
     {dn,consonant,'\132',1},              /* 5 .n           */
     {dn,consonant,'\161',2},              /* 6 .s           */
     {dn,lo_vowel,'\33',2},                /* 7 .r           */
     {dn,consonant,'\13',3},               /* 8 .g           */
     {dn,special,'\54',0},                 /* 9 .h           */
     {dn,special,'\137',0},                /* 10 .a          */
     {dn,special,'\24',0},                 /* 11 ..          */
     {dn,consonant,'\14',4},               /* 12 .K          */
     {dn,consonant,'\127',35},             /* 13 .T          */
     {dn,consonant,'\131',57},             /* 14 .D          */
     {dn,lo_vowel,'\30','\37'},            /* 15 .l          */
     {dn,lo_vowel,'\31','\174'},           /* 16 .L          */
     {dn,lo_vowel,'\21','\16'},            /* 17 .R          */
     {dn,consonant,'\122',32},             /* 18 "n          */
     {dn,consonant,'\146',5},              /* 19 "s          */
     {dn,consonant,'\170',0},              /* 20 Rh          */
     {dn,consonant,'\32',11},              /* 21 ~n          */
     {dn,hi_vowel,267,262},                /* 22 ~o          */
     {dn,hi_vowel,275,4},                  /* 23 ~a          */
     {dn,consonant,'\35',31},              /* 24 ~r          */
     {illegal,special,0,0},                /* 25 not used    */
     {illegal,special,0,0},                /* 26 not used    */
     {illegal,special,0,0},                /* 27 not used    */
     {illegal,special,0,0},                /* 28 not used    */
     {illegal,special,0,0},                /* 29 not used    */
     {control,special,0,0},                /* 30 dummy       */
     {control,special,0,0},                /* 31 end_of_line */
     {control,special,0,0},                /* 32 space       */
     {cmr,special,0,0},                    /* !              */
     {illegal,special,0,0},                /* "              */
     {dn,special,'\25',0},                 /* #              */
     {illegal,special,0,0},                /* $              */
     {control,special,0,0},                /* %              */
     {dn,special,0,0},                     /* &              */
     {cmr,special,0,0},                    /* ' to *         */
     {cmr,special,0,0},                    /* ' to *         */
     {cmr,special,0,0},                    /* ' to *         */
     {cmr,special,0,0},                    /* ' to *         */
     {dn,special,0,0},                     /* +              */
                                           /*  Kevin Carmody:
                                            *  "+" changed
                                            *  from cmr
                                            *  to dn         */
     {cmr,special,0,0},                    /* , to -         */
     {cmr,special,0,0},                    /* , to -         */
     {illegal,special,0,0},                /* .              */
     {dn,special,'\40',0},                 /* /              */
     {numeral,special,0,0},                /* 0 to 9         */
     {numeral,special,0,0},                /* 0 to 9         */
     {numeral,special,0,0},                /* 0 to 9         */
     {numeral,special,0,0},                /* 0 to 9         */
     {numeral,special,0,0},                /* 0 to 9         */
     {numeral,special,0,0},                /* 0 to 9         */
     {numeral,special,0,0},                /* 0 to 9         */
     {numeral,special,0,0},                /* 0 to 9         */
     {numeral,special,0,0},                /* 0 to 9         */
     {numeral,special,0,0},                /* 0 to 9         */
     {cmr,special,0,0},                    /* : and ;        */
     {cmr,special,0,0},                    /* : and ;        */
     {control,special,0,0},                /* <              */
     {cmr,special,0,0},                    /* =              */
     {control,special,0,0},                /* >              */
     {cmr,special,0,0},                    /* ?              */
     {dn,special,'\177',0},                /* @              */
     {dn,hi_vowel,258,'\101'},             /* A              */
     {dn,consonant,'\102',6},              /* B              */
     {dn,consonant,'\103',33},             /* C              */
     {dn,consonant,'\104',7},              /* D              */
     {dn,hi_vowel,259,'\173'},             /* E              */
     {illegal,special,0,0},                /* F              */
     {dn,consonant,'\107',8},              /* G              */
     {dn,special,'\54',0},                 /* H              */
     {dn,hi_vowel,'\111','\106'},          /* I              */
     {dn,consonant,'\112',9},              /* J              */
     {dn,consonant,'\113',10},             /* K              */
     {dn,consonant,'\17',30},              /* L              */
     {dn,special,'\134',0},                /* M              */
     {illegal,special,0,0},                /* N              */
     {dn,hi_vowel,260,'\117'},             /* O              */
     {dn,consonant,'\120',12},             /* P              */
     {illegal,special,0,0},                /* Q              */
     {dn,consonant,'\167',0},              /* R              */
     {illegal,special,0,0},                /* S              */
     {dn,consonant,'\124',13},             /* T              */
     {dn,lo_vowel,'\125',1},               /* U              */
     {illegal,special,0,0},                /* V to Z         */
     {illegal,special,0,0},                /* V to Z         */
     {illegal,special,0,0},                /* V to Z         */
     {illegal,special,0,0},                /* V to Z         */
     {illegal,special,0,0},                /* V to Z         */
     {cmr,special,0,0},                    /* [              */
     {control,special,0,0},                /* \              */
     {cmr,special,0,0},                    /* ]              */
     {illegal,special,0,0},                /* ^              */
     {dn,special,0,0},                     /* _              */
					   /*  Marc Csernel:
					    *  "_" changed
					    *  from illegal
					    *  to dn         */
     {cmr,special,0,0},                    /* `              */
     {dn,hi_vowel,'\141',257},             /* a              */
     {dn,consonant,'\142',14},             /* b              */
     {dn,consonant,'\143',15},             /* c              */
     {dn,consonant,'\144',37},             /* d              */
     {dn,hi_vowel,'\145',3},               /* e              */
     {dn,consonant,'\47',16},              /* f              */
     {dn,consonant,'\147',17},             /* g              */
     {dn,consonant,'\150',38},             /* h              */
     {dn,hi_vowel,'\151','\105'},          /* i              */
     {dn,consonant,'\152',18},             /* j              */
     {dn,consonant,'\153',19},             /* k              */
     {dn,consonant,'\154',20},             /* l              */
     {dn,consonant,'\155',21},             /* m              */
     {dn,consonant,'\156',22},             /* n              */
     {dn,hi_vowel,261,'\157'},             /* o              */
     {dn,consonant,'\160',23},             /* p              */
     {dn,consonant,'\52',24},              /* q              */
     {dn,consonant,'\162',0},              /* r              */
     {dn,consonant,'\163',25},             /* s              */
     {dn,consonant,'\164',26},             /* t              */
     {dn,lo_vowel,'\165',0},               /* u              */
     {dn,consonant,'\166',27},             /* v              */
     {illegal,special,0,0},                /* not used       */
     {illegal,special,0,0},                /* not used       */
     {dn,consonant,'\171',28},             /* y              */
     {dn,consonant,'\51',29},              /* z              */
     {control,special,0,0},                /* left brace     */
     {dn,special,'\56',0},                 /* |              */
     {control,special,0,0},                /* right brace    */
     {illegal,special,0,0},                /* ~              */
     {illegal,special,0,0}                 /* del            */
};

/*
 * Unordered table of consonants and consonant-groups giving
 * information on the available ligatures, the form taken by a
 * following "r", and the location in the dvng fonts of the
 * "half-form" (if any)
 */
struct cons_joins cons_table[] = {
     {0,0,1,0},                            /* 0          */
     {1,120,0,'\27'},                      /* 1 .n       */
     {2,100,0,'\11'},                      /* 2 .s       */
     {0,0,0,'\34'},                        /* 3 .g       */
     {0,0,1,'\7'},                         /* 4 .k       */
     {6,94,0,'\133'},                      /* 5 "s       */
     {1,88,0,'\74'},                       /* 6 B        */
     {2,78,0,'\100'},                      /* 7 D        */
     {1,16,0,'\135'},                      /* 8 G        */
     {0,0,1,'\44'},                        /* 9 J        */
     {0,0,1,'\110'},                       /* 10 K       */
     {2,39,1,'\26'},                       /* 11 ~n      */
     {1,115,0,'\45'},                      /* 12 P       */
     {0,0,0,'\114'},                       /* 13 T       */
     {3,85,0,'\116'},                      /* 14 b       */
     {2,34,0,'\121'},                      /* 15 c       */
     {1,116,0,'\10'},                      /* 16 f       */
     {1,15,0,'\140'},                      /* 17 g       */
     {2,37,0,'\76'},                       /* 18 j       */
     {9,0,0,'\77'},                        /* 19 k       */
     {1,91,0,'\123'},                      /* 20 l       */
     {2,89,0,'\115'},                      /* 21 m       */
     {1,80,0,'\6'},                        /* 22 n       */
     {4,81,0,'\75'},                       /* 23 p       */
     {0,0,1,'\12'},                        /* 24 q       */
     {2,105,0,'\55'},                      /* 25 s       */
     {3,55,0,'\50'},                       /* 26 t       */
     {2,92,0,'\46'},                       /* 27 v       */
     {0,0,1,'\5'},                         /* 28 y       */
     {1,114,0,'\36'},                      /* 29 z       */
     {0,0,1,'\20'},                        /* 30 L       */
     {0,0,1,'\35'},                        /* 31 ~r      */
     {8,17,1,0},                           /* 32 "n      */
     {1,36,1,0},                           /* 33 C       */
     {4,41,1,0},                           /* 34 .t      */
     {1,45,1,0},                           /* 35 .th     */
     {6,46,1,0},                           /* 36 .d      */
     {11,58,2,0},                          /* 37 d       */
     {7,107,1,0},                          /* 38 h       */
     {3,9,1,0},                            /* 39 k t     */
     {1,14,1,0},                           /* 40 k v     */
     {3,25,1,0},                           /* 41 "n k    */
     {1,28,1,0},                           /* 42 "n kh   */
     {1,29,1,0},                           /* 43 "n g    */
     {2,30,1,0},                           /* 44 "n gh   */
     {1,32,1,0},                           /* 45 "n k t  */
     {1,33,1,0},                           /* 46 "n k .s */
     {1,52,1,0},                           /* 47 .d g    */
     {1,53,1,0},                           /* 48 .d gh   */
     {2,71,2,0},                           /* 49 d d     */
     {2,73,2,0},                           /* 50 d dh    */
     {1,75,1,0},                           /* 51 d bh    */
     {1,77,2,0},                           /* 52 d v     */
     {3,102,1,0},                          /* 53 .s .t   */
     {0,0,1,'\43'},                        /* 54 k .s    */
     {0,0,1,0202},                         /* 55 t t     */
     {1,117,1,0},                          /* 56 .s .t r */
     {1,54,1,0},                           /* 57 .dh     */
     {1,12,1,0},                           /* 58 k n     */
     {1,13,1,0},                           /* 59 k r     */
     {0,0,1,0351},                         /* 60 g r     */
     {0,0,1,0350},                         /* 61 gh n    */
     {0,0,1,0352},                         /* 62 j ~n    */
     {0,0,1,0353},                         /* 63 ~n c    */
     {0,0,1,0354},                         /* 64 t r     */
     {1,69,1,0},                           /* 65 d g     */
     {1,70,1,0},                           /* 66 d gh    */
     {0,0,1,0361},                         /* 67 dh n    */
     {0,0,1,0362},                         /* 68 dh r    */
     {0,0,1,0363},                         /* 69 p t     */
     {0,0,1,0364},                         /* 70 "s c    */
     {0,0,1,0365},                         /* 71 "s r    */
     {0,0,1,0366},                         /* 72 "s v    */
     {1,118,1,0},                          /* 73 k t r   */
     {1,119,1,0},                          /* 74 .d r    */
     {1,76,1,0},                           /* 75 d r     */
     {0,0,2,0}                             /* 76 d b     */
};

/*
 * Ordered table of available ligatures giving each ligature's
 * position in the dvng fonts (or a negative number for use by
 * expand()) and index into cons_table[]
 *
 * Note that lig_table[96] is an entry for a ligature that was
 * removed from the fonts as of release 2.1. It is therefore
 * marked as (permanently) FALSE; it has not been removed from
 * the table since to do so would change the index values of all
 * succeeding entries, and these occur frequently in the code.
 */
struct ligs lig_table[] = {
     {'k',0303,0,TRUE},                    /* 0   k k        */
     {'t',0304,39,TRUE},                   /* 1   k t        */
     {'n',0307,58,TRUE},                   /* 2   k n        */
     {'m',0311,0,TRUE},                    /* 3   k m        */
     {'y',0310,0,TRUE},                    /* 4   k y        */
     {'r',0207,59,TRUE},                   /* 5   k r        */
     {'l',0312,0,TRUE},                    /* 6   k l        */
     {'v',0313,40,TRUE},                   /* 7   k v        */
     {6,042,54,TRUE},                      /* 8   k .s       */
     {'y',0305,0,TRUE},                    /* 9   k t y      */
     {'r',-5,73,TRUE},                     /* 10  k t r      */
     {'v',0306,0,TRUE},                    /* 11  k t v      */
     {'y',0346,0,TRUE},                    /* 12  k n y      */
     {'y',0347,0,TRUE},                    /* 13  k r y      */
     {'y',0314,0,TRUE},                    /* 14  k v y      */
     {'r',-2,60,TRUE},                     /* 15  g r        */
     {'n',0315,61,TRUE},                   /* 16  gh n       */
     {'k',0254,41,TRUE},                   /* 17  "n k       */
     {'K',0262,42,TRUE},                   /* 18  "n kh      */
     {'g',0275,43,TRUE},                   /* 19  "n g       */
     {'G',0277,44,TRUE},                   /* 20  "n gh      */
     {18,0274,0,TRUE},                     /* 21  "n "n      */
     {'n',0265,0,TRUE},                    /* 22  "n n       */
     {'m',0301,0,TRUE},                    /* 23  "n m       */
     {'y',0302,0,TRUE},                    /* 24  "n y       */
     {'t',0255,45,TRUE},                   /* 25  "n k t     */
     {'y',0257,0,TRUE},                    /* 26  "n k y     */
     {6,0260,46,TRUE},                     /* 27  "n k .s    */
     {'y',0272,0,TRUE},                    /* 28  "n kh y    */
     {'y',0276,0,TRUE},                    /* 29  "n g y     */
     {'y',0271,0,TRUE},                    /* 30  "n gh y    */
     {'r',0300,0,TRUE},                    /* 31  "n gh r    */
     {'y',0256,0,TRUE},                    /* 32  "n k t y   */
     {'v',0261,0,TRUE},                    /* 33  "n k .s v  */
     {'c',0316,0,TRUE},                    /* 34  c c        */
     {21,0317,0,TRUE},                     /* 35  c ~n       */
     {'y',0320,0,TRUE},                    /* 36  ch y       */
     {21,0342,62,TRUE},                    /* 37  j ~n       */
     {'r',0205,0,TRUE},                    /* 38  j r        */
     {'c',0321,63,TRUE},                   /* 39  ~n c       */
     {'j',0322,0,TRUE},                    /* 40  ~n j       */
     {'k',0326,0,TRUE},                    /* 41  .t k       */
     {2,0323,0,TRUE},                      /* 42  .t .t      */
     {13,0341,0,TRUE},                     /* 43  .t .th     */
     {'y',0324,0,TRUE},                    /* 44  .t y       */
     {'y',0325,0,TRUE},                    /* 45  .th y      */
     {'g',0263,47,TRUE},                   /* 46  .d g       */
     {'G',0264,48,TRUE},                   /* 47  .d gh      */
     {3,0345,0,TRUE},                      /* 48  .d .d      */
     {'m',0273,0,TRUE},                    /* 49  .d m       */
     {'y',0267,0,TRUE},                    /* 50  .d y       */
     {'r',-6,74,TRUE},                     /* 51  .d r       */
     {'y',0270,0,TRUE},                    /* 52  .d g y     */
     {'r',0266,0,TRUE},                    /* 53  .d gh r    */
     {'y',0344,0,TRUE},                    /* 54  .dh y      */
     {'t',0201,55,TRUE},                   /* 55  t t        */
     {'n',0327,0,TRUE},                    /* 56  t n        */
     {'r',057,64,TRUE},                    /* 57  t r        */
     {'g',0213,65,TRUE},                   /* 58  d g        */
     {'G',0212,66,TRUE},                   /* 59  d gh       */
     {'d',0214,49,TRUE},                   /* 60  d d        */
     {'D',0210,50,TRUE},                   /* 61  d dh       */
     {'n',0221,0,TRUE},                    /* 62  d n        */
     {'b',0223,76,TRUE},                   /* 63  d b        */
     {'B',0211,51,TRUE},                   /* 64  d bh       */
     {'m',0224,0,TRUE},                    /* 65  d m        */
     {'y',0215,0,TRUE},                    /* 66  d y        */
     {'r',-3,75,TRUE},                     /* 67  d r        */
     {'v',0222,52,TRUE},                   /* 68  d v        */
     {'r',0355,0,TRUE},                    /* 69  d g r      */
     {'r',0356,0,TRUE},                    /* 70  d gh r     */
     {'y',0220,0,TRUE},                    /* 71  d d y      */
     {'v',0370,76,TRUE},                   /* 72  d d v      */
     {'y',0217,0,TRUE},                    /* 73  d dh y     */
     {'v',0371,0,TRUE},                    /* 74  d dh v     */
     {'y',0216,0,TRUE},                    /* 75  d bh y     */
     {'y',0357,0,TRUE},                    /* 76  d r y      */
     {'y',0225,0,TRUE},                    /* 77  d v y      */
     {'n',0360,67,TRUE},                   /* 78  dh n       */
     {'r',-4,68,TRUE},                     /* 79  dh r       */
     {'n',0340,0,TRUE},                    /* 80  n n        */
     {'t',0330,69,TRUE},                   /* 81  p t        */
     {'n',0331,0,TRUE},                    /* 82  p n        */
     {'r',0376,0,TRUE},                    /* 83  p r        */
     {'l',0332,0,TRUE},                    /* 84  p l        */
     {'n',0247,0,TRUE},                    /* 85  b n        */
     {'b',0251,0,TRUE},                    /* 86  b b        */
     {'v',0333,0,TRUE},                    /* 87  b v        */
     {'n',0336,0,TRUE},                    /* 88  bh n       */
     {'n',0337,0,TRUE},                    /* 89  m n        */
     {'l',0335,0,TRUE},                    /* 90  m l        */
     {'l',0245,0,TRUE},                    /* 91  l l        */
     {'n',0246,0,TRUE},                    /* 92  v n        */
     {'v',0250,0,TRUE},                    /* 93  v v        */
     {'c',0226,70,TRUE},                   /* 94  "s c       */
     {'n',0227,0,TRUE},                    /* 95  "s n       */
     {'b',0233,0,FALSE},                   /* 96  "s b       */
     {'r',0231,71,TRUE},                   /* 97  "s r       */
     {'l',0232,0,TRUE},                    /* 98  "s l       */
     {'v',0230,72,TRUE},                   /* 99  "s v       */
     {2,0243,53,TRUE},                     /* 100  .s .t     */
     {13,0244,0,TRUE},                     /* 101 .s .th     */
     {'y',0367,0,TRUE},                    /* 102 .s .t y    */
     {'r',-1,56,TRUE},                     /* 103 .s .t r    */
     {'v',0253,0,TRUE},                    /* 104 .s .t v    */
     {'n',0334,0,TRUE},                    /* 105  s n       */
     {'r',0372,0,TRUE},                    /* 106  s r       */
     {5,0242,0,TRUE},                      /* 107  h .n      */
     {'n',0241,0,TRUE},                    /* 108  h n       */
     {'m',0234,0,TRUE},                    /* 109  h m       */
     {'y',0235,0,TRUE},                    /* 110  h y       */
     {'r',0240,0,TRUE},                    /* 111  h r       */
     {'l',0236,0,TRUE},                    /* 112  h l       */
     {'v',0237,0,TRUE},                    /* 113  h v       */
     {'r',0206,0,TRUE},                    /* 114  z r       */
     {'r',0203,0,TRUE},                    /* 115  ph r      */
     {'r',0204,0,TRUE},                    /* 116  f r       */
     {'y',0252,0,TRUE},                    /* 117  .s .t r y */
     {'y',0374,0,TRUE},                    /* 118  k t r y   */
     {'y',0373,0,TRUE},                    /* 119  .d r y    */
     {5,0233,0,TRUE}                       /* 120  .n .n     */
};

/*
 * Table of the actual codes output by devnag
 */
const char *out_string[] = {
   "\\7{","\\8{","\\9{","\\?","\\<","\\305w","\\306w",        /*   0-6   */
   "\\307w","\\308w","\\309w","\\30Aw","\\30Bw","\\30Cw",     /*   7-12  */
   "\\0","\\qx{","\\30Fw","\\310w","\\311w","\\312w",         /*  13-18  */
   "\\313w","\\314w","\\315w","\\316w","\\317w","\\318w",     /*  19-24  */
   "\\319w","\\31Aw","\\31Bw","\\31Cw","\\31Dw",              /*  25-29  */
   "\\31Ew","\\qy{","\\1","!","\\322w","\\323w","\\324w",     /*  30-36  */
   "\\325w","\\326w","\\327w","(",")","\\32Aw","+",",",       /*  37-44  */
   "-",".","/","0","1","2","3","4","5","6","7","8","9",       /*  45-57  */
   ":",";","<","=",">","?","@",                               /*  58-64  */
   "A","B","C","D","E","F","G","H","I","J","K","L","M",       /*  65-77  */
   "N","O","P","Q","R","S","T","U","V","W","X","Y","Z",       /*  78-90  */
   "\\35Bw","\\2","\\35Dw","\\qq{","\\35Fw","`",              /*  91-96  */
   "a","b","c","d","e","f","g","h","i","j","k","l","m",       /*  97-109 */
   "n","o","p","q","r","s","t","u","v","w","x","y","z",       /* 110-122 */
   "\\4","\\qz{","\\5","\\6{","\\37Fw",                       /* 123-127 */
   "\\380w","\\381w","\\382w","\\383w","\\384w","\\385w",     /* 128-133 */
   "\\386w","\\387w","\\388w","\\389w","\\38Aw","\\38Bw",     /* 134-139 */
   "\\38Cw","\\38Dw","\\38Ew","\\38Fw","\\390w","\\391w",     /* 140-145 */
   "\\392w","\\393w","\\394w","\\395w","\\396w","\\397w",     /* 146-151 */
   "\\398w","\\399w","\\39Aw","\\39Bw","\\39Cw","\\39Dw",     /* 152-157 */
   "\\39Ew","\\39Fw","\\3A0w","\\3A1w","\\3A2w","\\3A3w",     /* 158-163 */
   "\\3A4w","\\3A5w","\\3A6w","\\3A7w","\\3A8w","\\3A9w",     /* 164-169 */
   "\\3AAw","\\3ABw","\\3ACw","\\3ADw","\\3AEw","\\3AFw",     /* 170-175 */
   "\\3B0w","\\3B1w","\\3B2w","\\3B3w","\\3B4w","\\3B5w",     /* 176-181 */
   "\\3B6w","\\3B7w","\\3B8w","\\3B9w","\\3BAw","\\3BBw",     /* 182-187 */
   "\\3BCw","\\3BDw","\\3BEw","\\3BFw","\\3C0w","\\3C1w",     /* 188-193 */
   "\\3C2w","\\3C3w","\\3C4w","\\3C5w","\\3C6w","\\3C7w",     /* 194-199 */
   "\\3C8w","\\3C9w","\\3CAw","\\3CBw","\\3CCw","\\3CDw",     /* 200-205 */
   "\\3CEw","\\3CFw","\\3D0w","\\3D1w","\\3D2w","\\3D3w",     /* 206-211 */
   "\\3D4w","\\3D5w","\\3D6w","\\3D7w","\\3D8w","\\3D9w",     /* 212-217 */
   "\\3DAw","\\3DBw","\\3DCw","\\3DDw","\\3DEw","\\3DFw",     /* 218-223 */
   "\\3E0w","\\3E1w","\\3E2w","\\3E3w","\\3E4w","\\3E5w",     /* 224-229 */
   "\\3E6w","\\3E7w","\\3E8w","\\3E9w","\\3EAw","\\3EBw",     /* 230-235 */
   "\\3ECw","\\3EDw","\\3EEw","\\3EFw","\\3F0w","\\3F1w",     /* 236-241 */
   "\\3F2w","\\3F3w","\\3F4w","\\3F5w","\\3F6w","\\3F7w",     /* 242-247 */
   "\\3F8w","\\3F9w","\\3FAw","\\3FBw","\\3FCw","\\3FDw",     /* 248-253 */
   "\\3FEw","\\3FFw",                                         /* 254-255 */
   "{\\rs ","","aA","e\\?","aO","ao","A\\<","\\re}","}",      /* 256-264 */
   "\\rn{","{\\rdt}","aA\\<","{\\qva}","{\\qvb}","{\\qvc}",   /* 265-270 */
   "\\qa{","\\qb{","{","\\qc{","a\\<"                         /* 271-275 */
};

/*
 * Table specifying the form of following "r" for various consonants
 * and consonant-groups (used by expand())
 */
short r_ligs[6][2] = {
     {0243,1},                             /* .s .t */
     {0147,0},                             /* g     */
     {0144,2},                             /* d     */
     {0104,0},                             /* dh    */
     {0304,1},                             /* k t   */
     {0130,1}                              /* .d    */
};

/* --------- Addition by Marc Csernel 1998 --------- */

/* The array contains a list of LaTeX commands preceded by a code:

     1   a mandatory argument follows. e.g. \end{enumerate}
     2   the text inclosed by a preceding brace must not be translated
	 till next closing brace.
     3   two mandatory arguments follow. e.g. \multicolumn{nb}{desc}{text}
     4   For begin: a mandatory argument followed by zero, or one optional,
	 depending on the first one e.g. \begin{itemize} or
	 \begin{tabular}{....}
     5   a mandatory argument and optional ones which  must not be translated.
	 \raisebox{raise}[ht][prof]{text to be translated}
     6   one or more optional argument(s) which must not be translated and
	 a mandatory one which must be translated e.g.
	 \framebox[6cm][s]{text to be translated}
     7   The command cite: an optional argument which must be translated
	 and a mandatory one which mustn't.
     8   Commands like chapter, section... with optional argument which
	 must be translated and a mandatory one which must also be
	 translated.

*/

typedef struct {
   int Typ_Com;
   const char *Name_com;
} typcom;

typcom TabCom[] = {
     {4,"begin"},
     {1,"end"},
     {1,"label"},
     {1,"pageref"},
     {1,"ref"},
     {1,"index"},
     {1,"hspace"},
     {1,"hspace*"},
     {1,"vspace"},
     {1,"vspace*"},
     {1,"addvspace"},
     {1,"enlargethispage"},
     {1,"enlargethispage*"},
     {1,"include"},
     {1,"input"},
     {1,"parbox"},
     {1,"stepcounter"},
     {1,"refstepcounter"},
     {1,"bibitem"},
     {1,"cline"},
     {2,"rm"},
     {2,"kern"},
     {2,"hskip"},
     {2,"vskip"},
     {2,"vadjust"},
     {3,"addtolength"},
     {3,"setlength"},
     {3,"setcounter"},
     {3,"addtocounter"},
     {3,"multicolumn"},
     {3,"rule"},
     {3,"addcontentsline"},
     {5,"raisebox"},
     {6,"framebox"},
     {6,"makebox"},
     {7,"cite"},
     {8,"chapter"},
     {8,"section"},
     {8,"subsection"},
     {8,"subsubsection"},
     {8,"paragraph"},
     {8,"subparagraph"},
     {8,"part"},
     {0,""}
};

/*
 For the command begin, depending on the first argument we will
 have zero (default), 1, 2.. other arguments following. Within
 the following array the first element of an item describes the
 number of arguments, the second one the name of the first
 argument related with i.e

	\begin {tabular}

 will have 1 argument.
 */
typcom TabSubCom[]= {
     {1,"tabular"},
     {1,"supertabular"},
     {1,"longtable"},
     {1,"thebibliography"},
     {0,""}
};

char command[100];       /* the text of a current command to test */
int nbchcomm;            /* the number of character of the curent command */
char subcom [1000];      /* for debugging etc. */

/*
 This fuction tests if the LaTeX command is one of the TabCom[]
 array. In this case the code (Typ_Com) is returned.
 */
int test_command(void) {
   int i = 0;
   while (TabCom[i].Name_com[0] != 0) {
      if (!strcmp(command,TabCom[i++].Name_com)) return TabCom[i-1].Typ_Com;
   }
   return 0;
}

/*
 This routine writes down all characters it finds; it looks
 for a '{' followed by some text and a '}'; no nesting of braces
 is considered.
 */
char test_sub_com(void) {
   int nbsub = 0;
   while (symbol != '}') {
      sendchar(symbol);
      subcom[nbsub++] = symbol;
      subcom[nbsub] = 0;
      symbol = inp_ch();
   }
   sendchar(symbol);
   subcom[nbsub++] = symbol;
   subcom[nbsub] = 0;
   symbol = inp_ch();
   return symbol;
}

/*
 This routine writes down all characters it finds; it looks
 twice for a '{' followed by some text and a '}'; for the
 second time nesting of braces is considered.
 */
char comm_double_args(void) {
   int nbsub = 0;
   while (symbol != '}') {
      sendchar(symbol);
      subcom[nbsub++] = symbol;
      subcom[nbsub] = 0;
      symbol = inp_ch();
   }
   /* looking for the end of the second argument */
   sendchar(symbol);
   symbol = inp_ch();
   while (symbol != '{') {
      sendchar(symbol);
      symbol = inp_ch();
   }
   sendchar(symbol);
   symbol = inp_ch();
   symbol = ignore();
   sendchar(symbol);
   symbol = inp_ch();
   return symbol;
}

/*
 For LaTeX commands such as \raisebox{5pt}[10pt][25pt]{.sa.t&vidha.m}
 where the second argument must be translated
 */
char comm_special(void) {
   int nbsub = 0;
   while (symbol != '}') {
      sendchar(symbol);
      subcom[nbsub++] = symbol;
      subcom[nbsub] = 0;
      symbol = inp_ch();
   }
   while (symbol != '{') {
      sendchar(symbol);
      subcom[nbsub++] = symbol;
      subcom[nbsub] = 0;
      symbol = inp_ch();
   }
   return symbol;
}

/*
 For LaTeX commands such as \framebox[10pt][25pt]{.sa.t&vidha.m}
 where optional arguments are not translated and the mandatory
 one must be translated
 */
char comm_opt(void) {
   int nbsub = 0;
   while (symbol != '{') {
      sendchar(symbol);
      subcom[nbsub++] = symbol;
      subcom[nbsub] = 0;
      symbol = inp_ch();
   }
   return symbol;
}

char comm_chapter(void) {
   if (symbol == '[') {
      optchapter = 1;
      sendchar(symbol);
      symbol=inp_ch();
   }
   if (symbol == '*') {
      sendchar(symbol);
      symbol=inp_ch();
   }
   return symbol;
}

char comm_cite(void) {
   if (symbol == '[') {
      citation = 1;
      sendchar(symbol);
      symbol = inp_ch();
   }
   else test_sub_com();
   return symbol;
}

char *getsubarg(void) {
   char com[20], *result;
   int i = 0, j = 0;
   while (subcom[i] != '{') i++;
   i++;
   while (subcom[i] == ' ') i++;
   while (isalpha((unsigned char)subcom[i])) com[j++]= subcom[i++];
   com[j] = 0;
   i = 0;
   result = (char*) malloc (strlen(com)+1);
   strcpy (result,com);
   return result;
}

/*
 The LaTeX command BEGIN treatment
 */
char comm_begin(void) {
   int i = 0, nbargs = 0;
   char *com;
   int nbsub = 0;
   while (symbol != '}') {
      sendchar(symbol);
      subcom[nbsub++] = symbol;
      subcom[nbsub] = 0;
      symbol = inp_ch();
   }
   sendchar(symbol);
   symbol = inp_ch();
   com = getsubarg();
   while (TabSubCom[i].Name_com[0] != 0) {
      if (!strcmp(com,TabSubCom[i++].Name_com))
	nbargs = TabSubCom[i-1].Typ_Com;
   }
   if (!nbargs) {                   /* argument not found within the table */
      free(com);
      return symbol;
   }
   while (nbargs--) {
      while (symbol != '{') {
	 sendchar(symbol);
	 subcom[nbsub++] = symbol;
	 subcom[nbsub] = 0;
	 symbol = inp_ch();
      }
      sendchar(symbol);
      symbol = inp_ch();
      symbol =  ignore ();
   }
   sendchar(symbol);
   symbol = inp_ch();
   free(com);
   return symbol;
}

/*
 This routine assumes that a '{' has already been read; it
 looks for the corresponding closing brace '}'; nesting of
 braces is considered.
 */
char ignore(void) {
   int nbsub = 0;
   int nbbraces = 1;
   while (nbbraces ) {
      subcom[nbsub++] = symbol;
      subcom[nbsub] = 0;
      switch (symbol) {
       case '{':
	 nbbraces++;
	 sendchar(symbol);
	 break;
       case '}':
	 nbbraces--;
	 if (!nbbraces) return symbol;
	 sendchar(symbol);
	 break;
       case ' ':
       case '\n':
	 sendchar(symbol);
	 put_word();
	 break;
       default :
	 sendchar(symbol);
	 break;
      }
      symbol = inp_ch();
   }
   return symbol ;
}

/* ------------------ End Addition ----------------- */

/*
 * *** Devnag function definitions ***
 */

int main(int argc, char **argv) {
   char *s_ptr;
   int i;
   unsigned char dn_yes, start_file;

   /* if invoked with "-v", print banner and exit */

   if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
      printf("devnag v%s\n", version);
      puts(banner);
      exit(0);
   }

   /* get file specifications */

   if (argc == 3) {
      strcpy(infil, argv[1]);
      strcpy(outfil, argv[2]);
   }
   else {
      if (argc == 2) {
	 strcpy(infil, argv[1]);
	 *outfil = '\0';
      }
      else {
	 do {
	    printf("input file: ");
	    fgets(infil, MEDBUF, stdin);
	    infil[strlen(infil)-1] = '\0';
	 } while (strlen(infil) == 0);
	 printf("output file: ");
	 fgets(outfil, MEDBUF, stdin);
	 outfil[strlen(outfil)-1] = '\0';
      }
   }
   /* Check the source file extension */
   if (strcasecmp(infil+strlen(infil)-strlen(DEFAULT_DEST_EXT), DEFAULT_DEST_EXT) == 0) {
      fprintf(stderr, "source file extension %s is forbidden\n", DEFAULT_DEST_EXT);
      exit(1);
   }
   s_ptr = infil+strlen(infil)-strlen(DEFAULT_SRC_EXT);
   if (strcasecmp(s_ptr, DEFAULT_SRC_EXT) != 0) s_ptr = NULL;
   if (!s_ptr) strcat(infil, DEFAULT_SRC_EXT);

   /* Try to open the source file */
   f_in = fopen(infil, "rb");
   if (!f_in && !s_ptr) {
      /* strip extension which has not been supplied and try again */
      infil[strlen(infil)-strlen(DEFAULT_SRC_EXT)] = '\0';
      f_in = fopen(infil, "rb");
   }
   if (!f_in) {
      if (s_ptr) fprintf(stderr, "cannot open file %s\n", infil);
      else fprintf(stderr, "cannot open either %s or %s%s\n", infil, infil, DEFAULT_SRC_EXT);
      exit(1);
   }

   /* Destination file name */
   if (strlen(outfil) == 0) {
      strcpy(outfil, infil);
      if (strcasecmp(outfil+strlen(outfil)-strlen(DEFAULT_SRC_EXT), DEFAULT_SRC_EXT) == 0) {
         outfil[strlen(outfil)-strlen(DEFAULT_SRC_EXT)] = '\0';
      }
      strcat(outfil, DEFAULT_DEST_EXT);
   } else {
      if (strcasecmp(outfil+strlen(outfil)-strlen(DEFAULT_SRC_EXT), DEFAULT_SRC_EXT) == 0) {
         fclose(f_in);
         fprintf(stderr, "destination file extension %s is forbidden\n", DEFAULT_SRC_EXT);
         exit(1);
      }
      if (strcasecmp(infil, outfil) == 0) strcat(outfil, DEFAULT_DEST_EXT);
   }
#ifdef TEXLIVE
/* In TeX Live we want to share files between Unix and Windows systems.  */
#define FOPEN_W_MODE "wb"
#else
#if defined(MIKTEX_WINDOWS)
#define FOPEN_W_MODE "wb"
#else
#define FOPEN_W_MODE "w"
#endif
#endif
   if ((f_out = fopen(outfil, FOPEN_W_MODE)) == NULL) {
      fclose(f_in);
      fprintf(stderr, "cannot open %s for output\n", outfil);
      exit(1);
   }

   /* initialization */

   cons_seen = FALSE;
   vow_seen = FALSE;
   front_r = FALSE;
   wait_syll = FALSE;
   cmr_mode = FALSE;
   num_mode = FALSE;
   hindi_mode = FALSE;
   mhindi_mode = FALSE;
   dollar_mode = 0;
   do_hyphen = FALSE;
   do_vconjuncts = FALSE;
   tabs_mode = FALSE;                      /* Marc Csernel */
   lig_block = FALSE;                      /* Kevin Carmody */
   start_file = TRUE;

   chr_idx = 0;
   n_halves = 0;                           /* Kevin Carmody */
   *word = '\0';
   *inbuf = '\0';
   *outbuf = '\0';
   linenumber = 1;


   /* main loop */

   do {
      p_in = fgetline(inbuf, MAXBUF, f_in);
      if (start_file) {
         if (inbuf[0] != '%' || inbuf[1] != '&') {
            fprintf(f_out, "\\def\\DevnagVersion{%s}", version);
            start_file = FALSE;
          }
      }

      /* read preprocessor commands */

      if (*inbuf == '@') {
	 while (TRUE) {
	    if (strstr(inbuf, "dollars\n") == inbuf+1) {
	       dollar_mode = 1;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "nodollars\n") == inbuf+1) {
	       dollar_mode = 0;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "dolmode0\n") == inbuf+1) {
	       dollar_mode = 0;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "dolmode1\n") == inbuf+1) {
	       dollar_mode = 1;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "dolmode2\n") == inbuf+1) {
	       dollar_mode = 2;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "dolmode3\n") == inbuf+1) {
	       dollar_mode = 3;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "hindi\n") == inbuf+1) {
	       hindi_mode = TRUE;
	       if (mhindi_mode) {
		  for (i = 0; i < N_MNOLIGS; i++)
		    lig_table[modnolig[i]].in_use = TRUE;
		  mhindi_mode = FALSE;
	       }
	       for (i = 0; i < N_NOLIGS; i++)
		 lig_table[nolig[i]].in_use = FALSE;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "modernhindi\n") == inbuf+1) {
	       hindi_mode = TRUE;
	       mhindi_mode = TRUE;
	       for (i = 0; i < N_MNOLIGS; i++)
		 lig_table[modnolig[i]].in_use = FALSE;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "sanskrit\n") == inbuf+1) {
	       hindi_mode = FALSE;
	       mhindi_mode = FALSE;
	       for (i = 0; i < N_MNOLIGS; i++)
		 lig_table[modnolig[i]].in_use = TRUE;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "lig") == inbuf+1) {
	       s_ptr = inbuf;
	       while (TRUE) {
		  if ((s_ptr = strchr(s_ptr, ' ')) == NULL) break;
		  s_ptr++;
		  if (sscanf(s_ptr, "%d", &number) != 0)
		    lig_table[ligidxs[number-1]].in_use = TRUE;
	       }
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "nolig") == inbuf+1) {
	       s_ptr = inbuf;
	       while (TRUE) {
		  if ((s_ptr = strchr(s_ptr, ' ')) == NULL) break;
		  s_ptr++;
		  if (sscanf(s_ptr, "%d", &number) != 0)
		    lig_table[ligidxs[number-1]].in_use = FALSE;
	       }
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "hyphen\n") == inbuf+1) {
	       do_hyphen = TRUE;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "nohyphen\n") == inbuf+1) {
	       do_hyphen = FALSE;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "vconjuncts\n") == inbuf+1) {
	       do_vconjuncts = TRUE;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }
	    if (strstr(inbuf, "novconjuncts\n") == inbuf+1) {
	       do_vconjuncts = FALSE;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }

/* --------- Addition by Marc Csernel 1998 --------- */

	    if (strstr(inbuf, "tabs\n") == inbuf+1) {
	       tabs_mode = TRUE;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }

/* ------------------ End Addition ----------------- */

	    if (strstr(inbuf, "notabs\n") == inbuf+1) {
	       tabs_mode = FALSE;
	       fprintf(f_out, "%%%s", inbuf);
	       linenumber++;
	       break;
	    }

	    fprintf(stderr, "Warning: possible illegal preprocessor command at line ");
	    fprintf(stderr, "%d:\n%s", linenumber, inbuf);
	    fprintf(f_out, "%s", inbuf);
	    linenumber++;
	    break;
	 }
	 *inbuf = '\0';
      }
      else {
	 if (!find_dn()) fputs(inbuf, f_out);

	 /* process DN text */

	 else do {
	    buf_idx = 0;
	    symbol = inp_ch();
	    dnproc();
	    dn_yes = find_dn();
	    if (!dn_yes) {
	       strcat(outbuf, inbuf);
	       fputs(outbuf, f_out);
	    }
	 }
	 while (dn_yes);
	 *inbuf = '\0';
	 linenumber++;
	 *outbuf = '\0';;
      }
   }
   while (p_in != NULL);
   fclose(f_in);
   fclose(f_out);
   exit(0);
}


/*
 * Read and parse DN input, and convert it into the character set
 * defined in char_table[]
 */
void dnproc(void) {
   short i;
   unsigned char saved, dnready;
   short brace_lev;
   int test_val;                           /* Marc Csernel */
   char savchr = 0;
   char wrong[10];
   brace_lev = 1;
   saved = FALSE;
   dnready = FALSE;
   do {
      switch(symbol) {
       case '.':
	 savchr = inp_ch();
	 i = 0;
	 do { i++; } while ((i != 19) && (chset1[i-1] != savchr));
	 if (i == 19) {
	    wrong[0] = '.';
	    wrong[1] = savchr;
	    wrong[2] = '\0';
	    err_ill(wrong);
	 }
	 else {
	    if (i < 4) {
	       savchr = inp_ch();
	       if (savchr == 'h')  i += 11;
	       else saved = TRUE;
	       if (i == 1)  err_ill(".k");
	    }
	 }
	 if (i == 18) put_ch('M');
	 else put_ch(i);
	 break;

/* --------- Addition by Marc Csernel 1998 --------- */

	 /* cite and chapter command */
	 case ']':
	 if (citation) {
	    citation = 0;
	    if (num_mode) put_ch(dummy);
	    put_syll();
	    test_sub_com();
	 }
	 if (optchapter) {
	    optchapter = 0;
	    if (num_mode) put_ch(dummy);
	    put_syll();
	    sendchar(symbol);
	    break;
	 }

/* ------------------ End Addition ----------------- */

       case 'i':
	 savchr = inp_ch();
	 if (savchr == 'i')  put_ch('I');
	 else {
	    put_ch(symbol);
	    saved = TRUE;
	 }
	 break;
       case 'u':
	 savchr = inp_ch();
	 if (savchr == 'u') put_ch('U');
	 else {
	    put_ch(symbol);
	    saved = TRUE;
	 }
	 break;
       case 'a':
	 savchr = inp_ch();
	 i = 0;
	 do { i++; } while ((i != 4) && (chset2[i-1] != savchr));
	 if (i == 4) {
	    put_ch(symbol);
	    saved = TRUE;
	 }
	 else put_ch(chset5[i-1]);
	 break;
       case '\"':
	 savchr = inp_ch();
	 i = 0;
	 do { i++; } while ((i != 3) && (chset3[i-1] != savchr));
	 if (i == 3) {
	    wrong[0] = '\"';
	    wrong[1] = savchr;
	    wrong[2] = '\0';
	    err_ill(wrong);
	 }
	 else put_ch((short)(i+17));
	 break;
       case '~':
	 savchr = inp_ch();
	 i = 0;
	 do { i++; } while ((i != 5) && (chset6[i-1] != savchr));
	 if (i == 6) {
	    wrong[0] = '~';
	    wrong[1] = savchr;
	    wrong[2] = '\0';
	    err_ill(wrong);
	 }
	 else if (i == 5) put_ch(47);
	 else put_ch((short)(i+20));
	 break;
       case end_of_line:
	 put_ch(end_of_line);
	 break;
       case '$':
	 if (!dollar_mode) put_ch(symbol);
	 else {
	    if (no_dn) put_ch(dummy);
	    else put_ch('}');
	    dnready = TRUE;
	    put_word();
	    memmove(inbuf, inbuf+buf_idx, strlen(inbuf+buf_idx)+1);
	 }
	 break;
       case '}':
	 brace_lev -= 1;
	 if ((brace_lev == 0) && (!d_found)) {
	    if (no_dn) put_ch(dummy);
	    else put_ch(symbol);
	    dnready = TRUE;
	    put_word();
	    memmove(inbuf, inbuf+buf_idx, strlen(inbuf+buf_idx)+1);
	 }
	 else put_ch(symbol);
	 break;
       case '{':
	 put_ch(symbol);
	 brace_lev += 1;
	 break;
       case '%':
	 put_ch(symbol);
	 do {
	    symbol = inp_ch();
	    if (symbol != end_of_line) sendchar(symbol);
	 }
	 while (symbol != end_of_line);
	 sendchar('\n');
	 break;
       case '<':
	 put_syll();
	 do {
	    symbol = inp_ch();
	    if (symbol == end_of_line) symbol = '\n';
	    if (symbol != '>') sendchar(symbol);
	 }
	 while (symbol != '>');
	 break;
       case '\\':
	 put_ch(symbol);
	 symbol = inp_ch();
	 if (symbol == end_of_line) {
	    put_word();
	    strcat(outbuf, "\n");
	    fputs(outbuf, f_out);
	    *outbuf = '\0';
	 }
	 else {

/* --------- Addition by Marc Csernel 1998 --------- */

	    /* Modified MC May 98 to take \\[5pt] into account */
	    if (symbol == '\\') {
	       sendchar(symbol);
	       symbol = inp_ch();
	       if (symbol == '[') {
		  while (symbol != ']') {
		     sendchar(symbol);
		     symbol = inp_ch();
		  }
		  sendchar(symbol);
		  symbol = inp_ch();
	       }
	       savchr = symbol;
	       saved = TRUE;
	       break;
	    }

/* ------------------ End Addition ----------------- */

	    if (!isalpha((unsigned char)symbol)) sendchar(symbol);
	    else {
	      nbchcomm = 0;                             /* Marc Csernel */
	       do {
		  command[nbchcomm++] = symbol;         /* Marc Csernel */
		  sendchar(symbol);
		  symbol = inp_ch();
	       }
	       while (isalpha((unsigned char)symbol));

/* --------- Addition by Marc Csernel 1998 --------- */

	       command[nbchcomm++] = 0;
	       test_val = test_command();
	       switch (test_val) {
		case 1:
		  savchr = test_sub_com();
		  break;
		case 2:
		  savchr = ignore();
		  break;
		case 3:
		  savchr = comm_double_args();
		  break;
		case 4:
		  savchr = comm_begin();
		  break;
		case 5:
		  savchr = comm_special();
		  break;
		case 6:
		  savchr = comm_opt();
		  break;
		case 7:
		  savchr = comm_cite();
		  break;
		case 8:
		  savchr = comm_chapter();
		  break;
		default:
		  savchr = symbol;
	       }

/* ------------------ End Addition ----------------- */

	       saved = TRUE;
	    }
	 }
	 break;
       case ill_char:
	 err_ill('\0');
	 break;
       case end_of_file:
	 fputs("Error: missing }", stderr);
	 exit(1);
       default:
	 if (symbol < 0) err_ill('\0');  /* accented character inside dn mode */
	 i = 0;
	 do { i++; } while ((i != 10) && (chset4[i-1] != symbol));
	 if (i == 10) put_ch(symbol);
	 else {
	    savchr = inp_ch();
	    if (savchr == 'h') {
	       if (i == 9)  put_ch(20);
	       else put_ch((short)(symbol-32));
	    }
	    else {
	       put_ch(symbol);
	       saved = TRUE;
	    }
	 }
      }
      if (saved) {
	 symbol = savchr;
	 saved = FALSE;
      }
      else {
	 if (!dnready) symbol = inp_ch(); }
   }
   while (!dnready);
}

/*
 * Read and parse input in the character set defined by char_table[],
 * and convert it into TeX text and macros for the dvng fonts.
 */
void put_ch(short code) {
   short cons_idx = 0;
   char cstr[2];
   struct char_def *c_ptr;
   struct cons_joins *q_ptr;
   short i, j;
   /* --------- Addition by Kevin Carmody 2005 --------- */
   /*
    * Conjunct fixes used by fixconj()
    */
   short wrong_1[] = {63, 63, 113};
   short right_1[] = {63, 34, 0};                              /* kk.sa   */
   short wrong_2[] = {94, 214, 264, 113};
   short right_2[] = {94, 86, 264, 34, 0};                     /* .tk.sa  */
   short wrong_3[] = {22, 62, 26};
   short right_3[] = {22, 226, 0};                             /* ~nj~na  */
   short wrong_4[] = {274, 88, 264, 273, 57, 52, 264, 121};
   short right_4[] = {126, 88, 264, 43, 0};                    /* .drya   */
   short wrong_5[] = {94, 140, 264, 118};
   short right_5[] = {94, 100, 264, 146, 0};                   /* ddva    */
   short wrong_6[] = {94, 136, 264, 118};
   short right_6[] = {94, 100, 264, 64, 118, 0};               /* ddhva   */
   short wrong_7[] = {271, 100, 264, 273, 57, 52, 264, 121};
   short right_7[] = {272, 100, 264, 43, 0};                   /* drya    */
   short wrong_8[] = {9, 274, 86, 264, 273, 57, 52, 264, 121};
   short right_8[] = {9, 126, 86, 264, 43, 0};                 /* .s.trya */
   /* ------------------ End Addition ----------------- */

   c_ptr = &char_table[code-1];
   switch(c_ptr->ch_typ) {
    case dn:
      if (cmr_mode) {
	 cmr_mode = FALSE;
	 put_sym(RE);
      }
      else {
	 if (num_mode) {
	    num_mode = FALSE;
	    put_sym(RBRACE);
	 }
      }
      switch(c_ptr->ch_subtyp) {
       case hi_vowel:
	 if (wait_syll) {
	    put_syll();
	    if (do_hyphen) strcat(word, "\\-");
	 }
         if (lig_block) err_ill("+");      /* Kevin Carmody */
	 if (cons_seen) {
	    if (syll[chr_idx-1] < 0) expand();
	    if (code == 'i') {
	       for (i = chr_idx; i>= (cons_idx+1); i--)
		 syll[i] = syll[i-1];
	       syll[cons_idx] = c_ptr->ch_subcode;
	    }
	    else {
	       syll[chr_idx] = c_ptr->ch_subcode;
	       if ((code != 'A') && (code != 'a')) vow_seen = TRUE;
	    }
	 }
	 else syll[chr_idx] = c_ptr->ch_code;
	 chr_idx += 1;
	 wait_syll = TRUE;
	 cons_seen = FALSE;
	 break;
       case lo_vowel:
	 if (wait_syll) {
	    put_syll();
	    if (do_hyphen) strcat(word, "\\-");
	 }
         if (lig_block) err_ill("+");      /* Kevin Carmody */
	 if (cons_seen) {
	    if (syll[chr_idx-1] < 0) expand();
	    if ((syll[chr_idx-1] == 'r') && (code == 'u'))
	      syll[--chr_idx] = 'z';
	    else {
	       if ((syll[chr_idx-1] == 'r') && (code == 'U'))
		 syll[--chr_idx] = '!';
	       else {
		  if ((syll[chr_idx-1] == 'h') && (code == 7))
		    syll[--chr_idx] = 0343;
		  else {
		     if ((syll[chr_idx-1] == 'r') && ((code == 7)
			   || (code == 15) || (code == 16) || (code == 17))) {
			syll[--chr_idx] = c_ptr->ch_code;
			front_r = TRUE;
		     }
		     else {
			put_macro(c_ptr->ch_subcode);
			chr_idx -= 1;
		     }
		  }
	       }
	    }
	 }
	 else syll[chr_idx] = c_ptr->ch_code;
	 chr_idx += 1;
	 wait_syll = TRUE;
	 cons_seen = FALSE;
	 break;
       case consonant:
	 if (wait_syll) {
	    put_syll();
	    if (do_hyphen) strcat(word, "\\-");
         }
	 if (!cons_seen) {

	    /* Is this the first consonant? */

	    cons_seen = TRUE;
	    cons_idx = chr_idx;
	    cons_code = c_ptr->ch_subcode;
	    syll[chr_idx] = c_ptr->ch_code;
	    chr_idx += 1;
	    tst_half();
	 }
	 else {
	    q_ptr = &cons_table[cons_code];
	    if (syll[cons_idx] == 'r') {

	       /* Was there an initial "r"? */

	       front_r = TRUE;
	       syll[chr_idx-1] = c_ptr->ch_code;
	       cons_code = c_ptr->ch_subcode;
	       tst_half();
	    }
	    else {
	       i = q_ptr->lig_code;
	       for (j = i+(q_ptr->n_ligs); i != j; i++)
		 if (lig_table[i].sym_code == code) break;
               if ((i != j) && (lig_table[i].in_use)
		   && (!lig_block)) {       /* lig_block added Kevin Carmody */

		  /* Is there a ligature? */

		  syll[chr_idx-1] = lig_table[i].sym_new;
		  cons_code = lig_table[i].join_idx;
		  joincode = cons_table[cons_code].j_code;
		  if (joincode != 0) {
		     half_codes[0] = joincode;
		     n_halves = 1;
		  }
		  else {

		     /* ... or a half-form? */

		     joincode = cons_table[c_ptr->ch_subcode].j_code;
		     if ((joincode != 0) && (n_halves != 0)) {
			half_codes[n_halves] = joincode;
			n_halves += 1;
		     }
		     else n_halves = 0;
		  }
	       }
	       else {
                  if ((code == 'r') && (!lig_block)) {
		                            /* lig_block added Kevin Carmody */
                     if (q_ptr->r_type != 0) {

			/* "Special" non-initial "r"? */

			syll[chr_idx] = syll[chr_idx-1];
			syll[chr_idx-1] = (q_ptr->r_type == 1) ? '\176' : 272;
			syll[chr_idx+1] = RBRACE;
			chr_idx += 2;
		     }
		     else {

			/* "Normal" non-initial "r"? */

			syll[chr_idx] = '\175';
			chr_idx += 1;
			n_halves = 0;
		     }
		     cons_code = 0;
		  }
		  else {
                     if ((q_ptr->j_code != 0) && (!lig_block)) {
			                 /* lig_block added by Kevin Carmody */

			/* Can we build a conjunct using a ligature? */

			syll[chr_idx-1] = q_ptr->j_code;
			syll[chr_idx] = c_ptr->ch_code;
			cons_code = c_ptr->ch_subcode;
			chr_idx += 1;
			tst_half();
		     }
		     else {
			if (n_halves != 0) {

			   /* ... or a half-form? */

			   if (syll[chr_idx-1] != RBRACE) {
			      /*
			       * There might already have been a
			       * subscript "-r" in the conjunct!
			       * (E.g. khrya, ttrya)
			       *                   -- John Smith
			       */
			      for (i = 0; i < n_halves; i++)
				syll[chr_idx-1+i] = half_codes[i];
			      chr_idx += n_halves;
			      syll[chr_idx-1] = c_ptr->ch_code;
			      cons_code = c_ptr->ch_subcode;
			      tst_half();
			   }
			   else {
			      for (i = 0; i < n_halves; i++)
				syll[chr_idx-2+i] = half_codes[i];
			      chr_idx += n_halves;
			      syll[chr_idx-1] = c_ptr->ch_code;
			      syll[chr_idx-2] = RBRACE;
			      cons_code = c_ptr->ch_subcode;
			      tst_half();
			   }
			}
			else {

			   /* Fall back on viraama */

			   if (syll[chr_idx-1] < 0) expand();
			   put_macro(VIRAAM);
			   cons_code = c_ptr->ch_subcode;
			   syll[chr_idx] = c_ptr->ch_code;
			   chr_idx += 1;
			   tst_half();
			}
		     }
		  }
		  /*
		   * This hack is to secure the correct representation
		   * for various conjuncts that may assume undesirable
		   * forms.
		   */
/* --------- Modification by Kevin Carmody 2005 --------- */
                  fixconj(wrong_1, right_1);
                  fixconj(wrong_2, right_2);
                  fixconj(wrong_3, right_3);
                  fixconj(wrong_4, right_4);
                  fixconj(wrong_5, right_5);
                  fixconj(wrong_6, right_6);
                  fixconj(wrong_7, right_7);
                  fixconj(wrong_8, right_8);
/* ------------------ End Modification ----------------- */
		  /*
		   * Now replace the sequence consonant + viraama + ya
		   * by consonant + open ya (glyph became available only
		   * Devnag 2.1).
		   */
		  for (i = 0; i < chr_idx-3; i++) {
		     if ((syll[i] == 94) && (syll[i+2] == 264)
			 && (syll[i+3] == 121)) {
			syll[i] = syll[i+1];
			syll[i+1] = 43;
			chr_idx = i + 2;
		     }
		  }
	       }
	    }
	 }
         lig_block = FALSE;                /* Kevin Carmody */
	 break;
       case special:
         if (lig_block) err_ill("+");      /* Kevin Carmody */
	 if (cons_seen) { if (syll[chr_idx-1] < 0) expand(); }
	 if ((code == 'M') && (front_r)) {
	    front_r = FALSE;
	    if (vow_seen) syll[chr_idx] = 270;
	    else syll[chr_idx] = RDT;
	    chr_idx += 1;
	    put_syll();
	 }
	 else {
	    if ((code == 'M') && (syll[chr_idx-1] == 'I')) {
	       syll[chr_idx-1] = 18;
	       put_syll();
	    }
	    else {
	       if ((code == 'M') && (vow_seen)) {
		  syll[chr_idx] = 268;
		  chr_idx += 1;
		  put_syll();
	       }

/* --------- Addition by Marc Csernel 1998 --------- */

	       else {
		  if (code == '_') {
		     if(!cons_seen) err_ill("_");
		     else {
			put_macro(VIRAAM);
			put_syll();
		     }
		  }
		  else {
		     if (code == '&') {
			if(!tabs_mode) {
			   if  (!cons_seen) {
			      fprintf(stderr, "Error: tabs_mode not selected\n");
			      err_ill("&");
			   }
			   else {
			      put_macro(VIRAAM);
			      put_syll();
			   }
			}
			else {
			   if(!cons_seen) sendchar((char)code);
			   else {
			      put_macro(VIRAAM);
			      put_syll();
			   }
			}
		     }

/* ------------------ End Addition ----------------- */

		     else {

/* --------- Addition by Kevin Carmody 2005 --------- */

                        if (code == '+') {  /* half letter or viraama */
                           if (!cons_seen) err_ill("+");
                           lig_block = TRUE;
                        }
/* ------------------ End Addition ------------------ */

                        else {
                           if ((!hindi_mode) && (cons_seen)) put_macro(VIRAAM);
                           put_syll();
                           put_sym(c_ptr->ch_code);
                        }
		     }
		  }
	       }
	    }
	 }
      }
      break;
    case illegal:
      cstr[0] = code;
      cstr[1] = '\0';
      err_ill(cstr);
      break;
    case control:
      if (cmr_mode) {
	 cmr_mode = FALSE;
	 put_sym(RE);
      }
      else {
         if (lig_block) err_ill("+");      /* Kevin Carmody */
	 if (num_mode) {
	    num_mode = FALSE;
	    put_sym(RBRACE);
	 }
	 else {
	    if (cons_seen) {
	       if (syll[chr_idx-1] < 0) expand();
	    }
	    if ((!hindi_mode) && (cons_seen)) put_macro(VIRAAM);
	    put_syll();
	 }
      }
      if (code == end_of_line) {
	 put_word();
	 strcat(outbuf, "\n");
	 fputs(outbuf, f_out);
	 *outbuf = '\0';
      }
      else if (code != dummy) sendchar((char)code);
      break;
    case cmr:
      if (cmr_mode) sendchar((char)code);
      else {
       cmr_mode = TRUE;
       if (num_mode) {
	  num_mode = FALSE;
	  put_sym(RBRACE);
       }
       else {
          if (lig_block) err_ill("+");      /* Kevin Carmody */
	  if (cons_seen) { if (syll[chr_idx-1] < 0) expand(); }
	  if ((!hindi_mode) && (cons_seen)) put_macro(VIRAAM);
	  put_syll();
       }
       put_sym(RS);
       sendchar((char)code);
    }
      break;
    case numeral:
      if (num_mode) sendchar((char)code);
      else {
	 num_mode = TRUE;
	 if (cmr_mode) {
	    cmr_mode = FALSE;
	    put_sym(RE);
	 }
	 else {
            if (lig_block) err_ill("+");      /* Kevin Carmody */
	    if (cons_seen) { if (syll[chr_idx-1] < 0) expand(); }
	    if ((!hindi_mode) && (cons_seen)) put_macro(VIRAAM);
	    put_syll();
	 }
	 put_sym(RN);
	 sendchar((char)code);
      }
   }
}

/*
 * Append a character to word[]; if character is whitespace,
 * call put_word().
 */
void sendchar(char c) {
   int i = strlen(word);
   word[i] = c == end_of_line ? '\n' : c;
   word[i+1] = '\0';
   if (isspace((unsigned char)c)) put_word();
}

/*
 * Use the current chacter value as an index into out_string[], and
 * append the code found there to the end of word[].
 */
void put_sym(short code) {
   strcat(word, out_string[code]);
}

/*
 * Append word[] to outbuf[].
 */
void put_word(void) {
   /*
    * The hack is to avoid hyphenation before any consonant stopped
    * with viraama.                                   -- John Smith
    */
   char *w_ptr;
   if (do_hyphen) {                                          /* hack */
      do {
	 w_ptr = strstr(word, "\\-\\qq{");
	 if (w_ptr != NULL) memmove(w_ptr, w_ptr+2, strlen(w_ptr+2)+1);
      }
      while (w_ptr != NULL);
   }
   strcat(outbuf, word);
   *word = '\0';
}

/*
 * Append syll[] to word[], using the codes defined in out_string[]
 */
void put_syll(void) {
   /*
    * put_syll() now checks to see whether "old-style" treatment of
    * consonant sequences containing viraama has been requested with
    * the @vconjuncts preprocessor command. If not, it re-orders the
    * characters in any such sequence containing "i" so that the
    * i-matra vowel is placed after the viraama rather than before
    * the entire sequence.                             -- John Smith
    */
   short i;
   int ipos, vpos, bpos;
   if (do_vconjuncts == FALSE) {
      ipos = sindex(0, '\105');            /* 'i' */
      if (ipos != -1) {
	 vpos = sindex(ipos, '\136');      /* viraama */
	 if (vpos == -1) {
	    vpos = sindex(ipos, 271);
	    if (vpos == -1) vpos = sindex(ipos, 274);
	    if (vpos != -1) {
	       vpos = sindex(vpos, 273);   /* alternative viraama */
	       if ((syll[vpos+1] != '9') || (syll[vpos+2] != '4')) vpos = -1;
	    }
	 }
	 if (vpos != -1) {
	    bpos = sindex(vpos, 264);      /* right brace after viraama */
	    if (bpos != -1) {
	       for (i = ipos+1; i <= bpos; i++)
		 syll[i-1] = syll[i];
	       syll[bpos] = '\105';
	    }
	 }
      }
   }
   for (i = 0; i < chr_idx; i++)
     strcat(word, out_string[syll[i]]);
   if (front_r) {
      if (vow_seen) strcat(word, out_string[269]);
      else strcat(word, out_string[13]);
      front_r = FALSE;
   }
   chr_idx = 0;
   cons_seen = FALSE;
   vow_seen = FALSE;
   wait_syll = FALSE;
   lig_block = FALSE;                   /* Kevin Carmody */
}

/*
 * Check whether a "half-form" exists for current consonant, and
 * store the result.
 */
void tst_half(void) {
   joincode = cons_table[cons_code].j_code;
   if (joincode != 0) {
      half_codes[0] = joincode;
      n_halves = 1;
   }
   else n_halves = 0;
}

/*
 * Manipulate contents of syll[] to incorporate a macro call,
 * using values that are indices into out_string[] (so that
 * put_syll() will produce the correct output).
 */
void put_macro(short macro) {
   char tmp[5];
   int lt, i;
   if (syll[chr_idx-1] == '\175') {
      syll[chr_idx+1] = '\175';
      syll[chr_idx] = RBRACE;
      syll[chr_idx-1] = syll[chr_idx-2];
      syll[chr_idx-2] = macro;
      chr_idx += 2;
   }
   else {
      if (syll[chr_idx-1] == RBRACE) {
	 /*
	  * This is a bit of a hack on Velthuis's part! What it
	  * really means is: if syll[] currently consists of "dr"
	  * OR a consonant(-group) with the subscript "caret" form
	  * of "-r" already attached, AND if a subscript vowel
	  * (or viraama) is to be added to it. In pre-2.0 releases
	  * of devnag this always produced "\qa{consonant}{vowel}",
	  * i.e. it used the simple, non-"caret" form of "-r",
	  * whatever the consonant to which it was to be attached.
	  * I have fixed this bug (feature?), but at the cost of
	  * invoking a new macro, \qc, which will have to be
	  * defined in dnmacs.tex etc. for the new code to work.
	  *                                          -- John Smith
	  */
	 if (syll[chr_idx-3] == 126) syll[chr_idx-3] = 274;
	 else syll[chr_idx-3] = 271;
	 syll[chr_idx] = LBRACE;
	 sprintf(tmp, "%d", macro);
	 lt = strlen(tmp);
	 chr_idx += 1;
	 for (i = 0; i < lt; i++)
	   syll[chr_idx+i] = tmp[i];
	 chr_idx += lt;
	 syll[chr_idx] = RBRACE;
	 chr_idx += 1;
      }
      else {
	 syll[chr_idx] = syll[chr_idx-1];
	 syll[chr_idx-1] = macro;
	 syll[chr_idx+1] = RBRACE;
	 chr_idx += 2;
      }
   }
}

/*
 * Exit with error message in the case of illegal input.
 */
void err_ill(const char *str) {
   fprintf(stderr, "Error: illegal character(s) \"%s\" detected at line %d:\n",
	  str, linenumber);
#if defined(MIKTEX)
   fprintf(stderr, "%s", inbuf);
#else
   fprintf(stderr, inbuf);
#endif
   exit(1);
}

/*
 * Read and return the next character from inbuf[], taking appropriate
 * action in the case of EOL, EOF or an illegal character.
 */
char inp_ch(void) {
   char ch, ch_out;
   ch = inbuf[buf_idx++];
   if (ch == '\n') {
      if (p_in == 0) ch_out = end_of_file;
      else {
	 p_in = fgetline(inbuf, MAXBUF, f_in);
	 linenumber++;
	 buf_idx = 0;
	 ch_out = end_of_line;
      }
   }
   else {
      if (ch == '\t') ch_out = ' ';        /* Change TABs to spaces */
      else {
	 if ((unsigned)ch < 32) ch_out = ill_char;  /* Allow accented characters */
	 else ch_out = ch;
      }
   }
   return(ch_out);
}

/*
 * Invoked if syll[chr_idx-1] is negative (instead of containing
 * an octal character value). Fixes appropriate representation
 * for "r".
 */
void expand(void) {
   short indx;
   indx = -1-syll[chr_idx-1];
   if (r_ligs[indx][1] != 0) {
      syll[chr_idx] = r_ligs[indx][0];
      syll[chr_idx-1] = (r_ligs[indx][1] == 1) ? '\176' : 272;
      syll[chr_idx+1] = RBRACE;
      chr_idx += 2;
   }
   else {
      syll[chr_idx-1] = r_ligs[indx][0];
      syll[chr_idx] = '\175';
      chr_idx += 1;
   }
   cons_code = 0;
}

/*
 * Search inbuf[] for "{\dn" followed by non-letter or, in dollar
 * mode only, for "$". Return TRUE or FALSE as appropriate. If
 * TRUE, send inbuf[] text prior to mode-switch to outbuf[], followed
 * by mode-switch indicator, and make inbuf[] start at beginning of
 * Devanagari text.
 */
char find_dn(void) {
   char *d_ptr;
   char *dn_ptr;
   char *svbuf;
   int again;
   d_found = FALSE;
   svbuf = inbuf;
   do {
      again = FALSE;
      dn_ptr = strstr(svbuf, "{\\dn");
      if (dn_ptr != NULL) {
	 again = isalpha((unsigned char)dn_ptr[4]);
	 svbuf = dn_ptr+4;
      }
   }
   while (again);
   if (dollar_mode) {
      d_ptr = strchr(inbuf, '$');
      if ((d_ptr != NULL) && ((dn_ptr == NULL) || (d_ptr < dn_ptr))) {
	 d_found = TRUE;
	 dn_ptr = d_ptr;
      }
   }
   if (dn_ptr == NULL) return(FALSE);
   strncat(outbuf, inbuf, dn_ptr-inbuf);
   no_dn = FALSE;
   if (!d_found) {
      if (dn_ptr[4] == '#') {              /* equivalent to @dolmode3 */
	 no_dn = TRUE;
	 dn_ptr += 1;
      }
      else strcat(outbuf, "{\\dn");
      dn_ptr += 4;
   }
   else {
      switch(dollar_mode) {
       case 1:
	 strcat(outbuf, "{\\dn ");
	 break;
       case 2:
	 strcat(outbuf, "\\pdn ");
       case 3:
	 no_dn = TRUE;
      }
      dn_ptr += 1;
   }
   memmove(inbuf, dn_ptr, strlen(dn_ptr)+1);
   return(TRUE);
}

/*
 * Return index of character value in syll[] starting at syll[i],
 * -1 if none
 */
int sindex(int i, short t) {
   int j;
   for (j = i; j < chr_idx; j++) {
      if (syll[j] == t) return j;
   }
   return -1;
}

/*
 * Replace infelicitous conjuncts
 */
void fixconj(short *wrong, short *right) {
   int i;
   for (i = 0; i < chr_idx; i++) {
      if (syll[i] != wrong[i]) return;
   }
   for (i = 0; right[i] != 0; i++) {
      syll[i] = right[i];
   }
   chr_idx = i;
}

/*
 * Read a line with either line ending (UNIX = LF, DOS = CR LF, Mac = CR)
 * return the pointer to the buffer of NULL at end of file or I/O Error
 * (replacement of fgets, requires global variables)
 */
char *fgetline(char *buf, int n, FILE *f) {
   int k;
   buf[0] = '\0';
   for (k = 0; k < n-1 && (k == 0 || buf[k-1] != '\n'); ) {
      if (charpresent) charpresent = FALSE;
      else charbuf = fgetc(f);
      if (wasCR) {
         wasCR = FALSE;
         buf[k++] = '\n';
         if (charbuf != '\n') charpresent = TRUE;
      } else {
         if (charbuf == EOF) {
            if (k) {
               charpresent = TRUE;   break;
            }
            else return NULL;
         }
         if (charbuf == '\r') wasCR = TRUE;
         else buf[k++] = charbuf;
      }
   }
   buf[k] = '\0';
   return buf;
}
