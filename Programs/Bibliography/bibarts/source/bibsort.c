/*  BibArts 2.5 assists you to write LaTeX texts in arts and humanities. */
/*  Copyright (C) 2022b  Timo Baumann   bibarts[at]gmx.de   (2022/10/01) */

/*  This program is free software; you can redistribute it and/or modify */
/*  it under the terms of the GNU General Public License as published by */
/*  the Free Software Foundation; either version 2 of the License, or    */
/*  (at your option) any later version.                                  */

/*  This program is distributed in the hope that it will be useful,      */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/*  GNU General Public License for more details.                         */

/*  You should have received a copy of the GNU General Public License    */
/*  along with this program; if not, write to the Free Software          */
/*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            */



/*
  FILES OF THE PACKAGE AND DOCUMENTATION
  CTAN mirrors > BibArts > bibarts.zip
    README.txt    Version history since 1.3
    bibarts.sty   LaTeX style file
    ba-short.pdf  Short documentation (English)
    ba-short.tex  Source of ba-short.pdf
    bibarts.pdf   Full documentation (German)
    bibarts.tex   Source of bibarts.pdf
    bibsort.exe   bibarts-bin to create the lists
    bibsort.c     This file; source of bibsort.exe
    COPYING       The license of the package
 */



/*
  COMPATIBILITY

  For my tests on Windows 10, I compiled bibsort.c 2.5 with MinGW, and with
  MS VisualStudio C/C++; I typed gcc ..., or cl ..., TO THE COMMAND PROMPT.

  Do NOT change this file when you do not have a qualified text
  editor - it contains control chars: '', '', '', '',  |  Are the
  '', '', '', '', '', '', '', '', '', '', '',   |  |'s in
  '', '', '', '', '', '', '', '', '', '', and ''|  a line?

  The C-compiled binary of this file is reading LaTeX .aux files.
  It can NOT read the .bar files, which BibArts 1.x did use.
  BibArts 2.5 ONLY has an EMULATION for 1.3 texts:  Keep copies of 1.x?
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>




#define  MAXLINES            65536
#define  MAXLEN               4096
#define  MAXFILES              128
#define  MAXFILENAMLEN        1024

#define  AUX_SUFFIX          ".aux"
#define  VLI_SUFFIX          ".vli"
#define  VQU_SUFFIX          ".vqu"
#define  VKC_SUFFIX          ".vkc"
#define  ABK_SUFFIX          ".abk"
#define  PER_SUFFIX          ".per"
#define  ARQ_SUFFIX          ".arq"
#define  GRR_SUFFIX          ".grr"
#define  PRR_SUFFIX          ".prr"
#define  SRR_SUFFIX          ".srr"

#define  IKILLNAME  "\\Ikillname{}"  /* -K: Damit wird ARG1 und ARG2 einer */
                                     /* Zeile ersetzt, falls diese in der  */
                                     /* vorausgehenden Zeile gleich sind;  */
#define  NKILLNAME   "\\killname{}"  /* ... und weitere gleiche Namen.     */
#define  PLURALCHAR             'P'  /* "\\Pkillname{}" */
#define  MALE             "\\male"
#define  FEMALE         "\\female"
#define  NOTDOT       "\\banotdot"
#define  HASDOT       "\\bahasdot"
#define  RELAX            "\\relax"  /* Auch um folgende spaces zu ignorieren */
#define  TeXITEM           "\\item"
#define  TYPEOUT     "\\typeout@ba"
#define  IeC              "\\IeC\t"
#define  PREPARESPACE  "\\bibsortpreparespace"
#define  PREPAREXSPACE "\\bibsortpreparexspace"
#define  PREPAREHEAD   "\\bibsortpreparehead"

#define  ABKDEF          "\\abkdef"

#define  BAFALSE          "{false}"


#define  NOKILLAUTHORS            0
#define  DOKILLAUTHORS            1
#define  DOKILLABBREVS            2
#define  DOKILLREGISTS            3
#define  DOKILLK                  4

#define  NOTEXCHANGENAMES         0
#define  EXCHANGENAMES            1

#define OPENB   "\'{\'"
#define NUMLEN  16

int ignorespaces = 0;
int hy_as_space = 0;
int kill = NOKILLAUTHORS;
int ex_n = NOTEXCHANGENAMES;
int registpoints = 0;
char  *lineptr[MAXLINES];

int FntIndexOverPages = 1;    /* new in 2.2 */

#define PCOLS        2
#define PCOLSSORT    1
#define ACOLS        2
#define ACOLSSORT    1
#define KCOLS        4
#define VCOLS        4
#define IS_Q         2
#define IS_V         1
#define NO_V         0
#define MAXBACOLS    4
char  *subAptr[MAXBACOLS][MAXLINES];
char  *subBptr[MAXBACOLS][MAXLINES];
char  *subCptr[MAXBACOLS][MAXLINES];

#define PROTECT      "\\protect"
#define KTIT         "\\ktit"
#define KTITCMD      "\\ktit\t"
#define KTITINARG    "{\\ktit}"
#define ANNOUNCEKTIT "\\announcektit"
#define KURZ         "\\kurz"
#define KURZCMD      "\\outkurz\t"
#define KTITMISS     "\?\?"   /* "\t|" before 2.5 */
#define KTITEMPTY    "\?\?"   /* "\t>" before 2.5 */
#define KTITBEG      "{"
#define KTITEND      "}"
#define KTITOP       '{'
#define KTITCL       '}'
int ktiterr = -1;    /* otherktiterr before 2.5 */

#define ACCEPTLINE   0
#define REJECTLINE   1
#define DOUBLELINE   2   /*  \                  */
#define DIFFERLINE   3   /*    Erst ***nach***  */
#define UNDEFILINE   4   /*    QQsort vergeben  */
#define ABBREVLINE   5   /*  /                  */
#define REDEF1LINE   6
#define REDEF2LINE   7
int leval[MAXLINES];

/* new errmsgs in 2.5: */
#define NOINERR      0
#define KTIT_SEV_ERR 1
#define KTIT_EMP_BAS 2
#define KTIT_EMP_ERR 3
#define KTIT_MIS_BAS 5
#define KTIT_MIS_ERR 6
#define KTIT_DBL_BAS 11
#define KTIT_DBL_ERR 12
#define OARG_IRR_BAS 23
#define OARG_IRR_ERR 24
#define OARG_BRA_BAS 47
#define OARG_BRA_ERR 48
#define KTIT_OPE_BAS 95
#define KTIT_OPE_ERR 96
int inerr[MAXLINES];  /* new in 2.5 */

#define BARESTCOLS   5
#define PAGEFNTCOL   1
#define PAGECOL      BARESTCOLS
#define FNTCOL      (BARESTCOLS+1)
#define BAALLRCOLS  (BARESTCOLS+2)
char  *subRptr[BAALLRCOLS][MAXLINES];
int    pagewgt[MAXLINES];
int     fntwgt[MAXLINES];

#define STUFFRCOL    0
#define KENTRY       0
#define VENTRY       1
#define ERRENTRY     2
#define VENTRYCH   'v'
#define KENTRYCH   'k'

#define ERRINPUTLINENO     "line \?\?"
#define ERRINPUTLINENOLEN            7

#define OVERFLOWTXT   "Counter too large"
           /* vgl. -> "\\errmessage\t" */
#define MISSINGSYMB   "\\erri@ba"
#define MISSINGERMS   "??"
#define OVERFLOWSYMB  "()"
#define   EMPTYSYMB   "[]"
char *TEXTSYMBOPEN[]  = { "\\{{", "{", NULL };
char *TEXTSYMBCLOSE[] = { "\\/}\\}", "\\/}", NULL };
int notext = 0;

int     dqsort[MAXLINES];
#define MINCATCODE   0
#define MAXCATCODE  15
#define ERRCATCODE  MINCATCODE-1
const char bareststr[] = "{}{}[][]()";
#define DQCOL        2
#define TeXACTIVE   13
#define TeXPASSIVE  12
#define LANCOL       3
#define GERMANTEX    "\\bagermanTeX"     /* err-def ist \bagermanTeX#1{}   */
#define ORIGINALTEX  "\\baoriginalTeX"   /* err-def ist \baoriginalTeX#1{} */
#define HYPHENATION  "\\language="
#define LINECOL      4
#define OLDBIGROMAN  "\\uppercase"
#define NEWFNSYMBOL  "\\ensuremath"
#define TOMFNSYMBOL  "\\TextOrMath"
#define TEXFILELINE  "\\line@ba"

#define BS_SPACE     "\\ "
#define BANCOMMA     ", "
#define BAINDEXFIRS  "  \\first@baidx{"
#define BAINDEXFOLL  "  \\follo@baidx{"
#define BAINDEXEND                "}\n"
#define CLOOPASTART  -1
#define CLOOPBSTART   0
#define BAINDEXLEN   45


#define BADNUM      0
#define GOODNUM     1
#define CHECKNUM    0
#define ANALYZENUM  1
#define HASOFLOW    0
#define HASNOOFLOW  1

#define MAXMESSN   10
#define MAXINFO    10

FILE  *infile,  *outfile;
char *filelist[MAXFILES];

int get_jobs = 0;
int lastupperascii = 0;
int thisupperascii = 0;
#define UTF_EIGHT_MAXLEN  4
#define UTF_EIGHT_MAXERR 20
char err_utf_eight[UTF_EIGHT_MAXERR][UTF_EIGHT_MAXLEN+1];
int utf_err_c = 0;
char emp_utf_eight[UTF_EIGHT_MAXERR][UTF_EIGHT_MAXLEN+1];
int utf_emp_c = 0;
char this_emp_utf_eight[UTF_EIGHT_MAXLEN+1];
char this_err_utf_eight[UTF_EIGHT_MAXLEN+1];

#define IS_BASE_ENC    0
#define IS_TONE_ENC    1
#define IS_UTFE_ENC    2
int encode = IS_BASE_ENC;

int german = 0;
int nogsty = 0;

int EinlROEMseiten = 0;
#define BA_FALSE           0
#define BA_TRUE            1
    int get_R_pg  = BA_FALSE;
    int get_A_pg  = BA_FALSE;
    int get_r_pg  = BA_FALSE;
    int get_a_pg  = BA_FALSE;
int ALPHmpfootnotes = 0;
    int get_R_fn  = BA_FALSE;
    int get_A_fn  = BA_FALSE;
    int get_r_fn  = BA_FALSE;
    int get_a_fn  = BA_FALSE;
int igleichj = 0;

#define METALINEHEAD "%%%%META%%%%"


int bago = 0;    /* new in 2.2 */
#define BAGOCMD  "\\bago"

char *soptarg = NULL;
char *foptarg = NULL;

char *mydefault = "";
char *nofnt = "(nofnt)";   /* nur "" wuerde Fehler machen */
char *unused = "(unused)";
void myfree(char *p) { if (p != mydefault && p != nofnt && p != unused && p != NULL) free(p); }



    /* Begin prototypes ... */

void print_two(int line, int left, int right);
int isLaTeXcmdletter(char a);
int entkerne_IeC(char buf[], const char *s);
int isinstr(const char *p, const char *s);
int lcmdcmp(const char *s, const char *p);
void swap(int i, int j);
int vergleich(const char *a, const char *b);
int Vergleich(const char *a, const char *b);
int llstrlen(const char *s);
int do_only(char ret[], const char form[], const char in[]);
int userusesdq(int deepsort, int linec);
int texcmp(const char *s, const char *p, int dqcats, int dqcatp);
int subdoubleauthors(int found, int addtxt, char a[], int remi, int i, const char *q, const char *p, int qline, int pline, int fe, char xx[]);

    /* ... end prototypes. */



void openfailed(const char *s)
{
   fprintf(stderr, "%%%% == Failed to open \"%s\". ==\n", s);
}


void intern_err(int n)
{
  printf("%%%%\n%%%%>   INTERNAL ERROR %d\n", n);
}


void announceexit(const char *s)
{
   fprintf(stderr, "%%%%>  I give up my job, because I %s.\n", s);
}


int mystrncpy(char *p, const char *s, int len)
{
  int l = 0, ll;

  while(*s != '\0' && l < len)
  {
    *p++ = *s++;
    ++l;
  }

  *p = '\0';   /* handle error: len < 0 */

  ll = l;
  while (ll <= len)
  {
    *p++ = '\0';
    ++ll;
  }

  return l;
}


char *strnmalloc(const char *q, int len)
{  char *p = mydefault;
   int l = 0;
   if (len < 1) { intern_err(0); len = 1; }  /* mallocError len */
   p = (char *) malloc(len);
   if (p == NULL)
   {
      printf("%%%%\n%%%%>  Error: Not enough memory.\n");
      announceexit("could not alloc enough memory");
      exit(1);
   }
   else l = mystrncpy(p, q, len-1);
   if (l != len-1) intern_err(1);      /* alloc len l != len-1 */
   return p;
}
#define HASFNTCH  '@'
#define HASUNUCH  '-'


/*--------------------------------------------------------------------------*/


const char *getmyname(const char *p, int max)
{
  const char *m = p;
  int pos = strlen(p);

  if (pos > max)
  while(*p != '\0')
  {
    if (*p == '/' || *p == '\\')
    {
      m = p+1;
      if (pos <= max) break;
    }
    ++p; --pos;
  }
  return m;
}


const char *authortable[][2] = {
  {  "\\vauthor\t",     "\\vauthor"     },
  {  "\\midvauthor\t",  "\\midvauthor"  },
  {NULL, NULL}};
#define  NAUTHOR 2



void licenseinfo(void)
{
  printf("%%%%   I guess that I am a bin-compilation of bibsort.c.  My job is to read\n");
  printf("%%%%   entries in LaTeX %s files which are made by bibarts.sty.  Use e.g.\n", AUX_SUFFIX);
  printf("%%%%   \\vli{FirstName}{FamilyName}{BookTitle, Year} and \\printvli in your\n");
  printf("%%%%   .tex file.  For usage, see ba-short.pdf (engl) or bibarts.pdf (ger).\n");
  printf("%%%%\n");
  printf("%%%%   This program is free software; you can redistribute it and/or modify\n");
  printf("%%%%   it under the terms of the GNU General Public License as published by\n");
  printf("%%%%   the Free Software Foundation; either version 2 of the License, or\n");
  printf("%%%%   (at your option) any later version.\n");
  printf("%%%%\n");
  printf("%%%%   This program is distributed in the hope that it will be useful,\n");
  printf("%%%%   but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
  printf("%%%%   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
  printf("%%%%   GNU General Public License for more details.\n");
  printf("%%%%\n");
  printf("%%%%   You should have received a copy of the GNU General Public License\n");
  printf("%%%%   along with this program; if not, write to the Free Software\n");
  printf("%%%%   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n");
  printf("%%%%\n");
}


void moremoreinfo(const char *myname)
{
  printf("%%%%                 CODEPAGES\n");
  printf("%%%%         LaTeX today writes ASCIIs > 127 in your %s-file(s), if you use\n", AUX_SUFFIX);
  printf("%%%%         \\usepackage[utf8]{inputenc}.  Then, you may start me using option\n");
  printf("%%%%  -utf8  I expect characters as definded in utf8enc.dfu 2022/06/07 v1.3c,\n");
  printf("%%%%         except those, which execute \\cyr...- or \\CYR...-cmds. Those and\n");
  printf("%%%%         further utf8-characters will be reported to screen as unknown.\n");
  printf("%%%%    -t1  You set \\UseRawInputEncoding, and ASCIIs > 127, which you use in\n");
  printf("%%%%         your .tex-file(s), reach the %s-file(s); I interpret them.\n", AUX_SUFFIX);
  printf("%%%%         **For apparently all other codepages, LaTeX writes \\cmds.**\n");
  printf("%%%%                 PRINTING OF CO-AUTHORS\n");
  printf("%%%%         To introduce a book with two authors, type e.g. in a footnote:\n");
  printf("%%%%           \\xvli{John}{Smith} *{\\vauthor{Peter}{Young}} {... [Title] ...}\n");
  printf("%%%%         Use %s{}{} in front of %s{}{} for further authors.\n", authortable[1][1], authortable[0][1]);
  printf("%%%%         Starting %s with default, \\printvli will print the list as:\n", getmyname(myname, 18));
  printf("%%%%           Smith, John / Peter Young: ... [Title] ...\n");
  printf("%%%%     -e  Exchanges the args of %s (and of %s).  \\printvli:\n", authortable[0][1], authortable[1][1]);
  printf("%%%%           Smith, John / Young, Peter: ... [Title] ...\n");
  printf("%%%%         In BOTH cases, this entry is sorted as:  Smith John Young Peter\n");
  printf("%%%%\n");
}


void moreinfo(const char *myname)
{
  printf("%%%%  -utf8  See  %s -\?\?\?\n", getmyname(myname, 40));
  printf("%%%%     -m  Write into files the meta arguments which I use to sort lines.\n");
  printf("%%%%     -b  If %s is in an arg, only text behind will have a sorting weight.\n", BAGOCMD);
  printf("%%%%    -n1  You reset fnt counters (no fnt-block-indexing over several pages).\n");
  printf("%%%%   INDEX TO PAGE AND FOOTNOTE NUMBERS (num-lists like \\printnumvkc):\n");
  printf("%%%%   (1) DEFAULT SORTING ORDER of page and footnote numbers:\n");
  printf("%%%%           fnsymbol    r/Roman    arabic    a/Alph (z=26, aa=27, ab=28)\n");
  printf("%%%%        You may change the EVALUATION ORDER (r/Roman versus a/Alph):\n");
  printf("%%%%                   / introduction = *lower roman page numbers*\n");
  printf("%%%%         DEFAULT <   appendix     = *capital Alph page numbers*\n");
  printf("%%%%                   \\ minipages    = *lower alph footnote numbers*\n");
  printf("%%%%         -s1  EVALUATE \'I\' as 1 in pages (intro Roman, appendix alph).\n");
  printf("%%%%         -f1  EVALUATE \'i\' as 1 in fnts (evaluate on roman before alph).\n");
  printf("%%%%   (2) NON-DEFAULT SORTING ORDER with an EVALUATION affixed to that order:\n");
  printf("%%%%        -s2 <xxxx>  Page number style:     -f2 <xxxx>  Footnote mark style:\n");
  printf("%%%%           with a permutation of 4 letters out of nRrAas in free order\n");
  printf("%%%%             (a=alph, A=Alph, n=arabic, R=Roman, r=roman, s=fnsymbol);\n");
  printf("%%%%           but in <xxxx>, there must be n, s, and R *or* A, and r *or* a.\n");
  printf("%%%%\n");
}


void maininfo(const char *myname)
{
  printf("%%%%   I read a LaTeX %s file (follow \\@input), and I write my output in files\n", AUX_SUFFIX);
  printf("%%%%     %s  Literature     %s  Published sources     %s  Geographic index\n", VLI_SUFFIX, VQU_SUFFIX, GRR_SUFFIX);
  printf("%%%%     %s  Short titles   %s  Unpublished sources   %s  Person index\n",     VKC_SUFFIX, ARQ_SUFFIX, PRR_SUFFIX);
  printf("%%%%     %s  Periodicals    %s  Abbreviations         %s  Subject index\n",    PER_SUFFIX, ABK_SUFFIX, SRR_SUFFIX);
  printf("%%%%\n");
  printf("%%%%  %s <LaTeXFile> [-o <OutFile>] [-g1 [OR] -g2] [-utf8] [-e] [...]\n", getmyname(myname, 13));
  printf("%%%%\n");
  printf("%%%%         DefaultSort:  0, .., 9, (A a), (B b), .., (s \\ss S), .., (Z z)  and\n");    /* udtable */
  printf("%%%%         (a [\\.\\'\\`\\^\\\"\\=\\~]a \\aa=\\r{a} [\\b\\c\\k\\d\\H\\t\\u\\v]{a} \\ae),  b, ..\n");
  printf("%%%%   -g<n> GermanSort:   (a A), .., (z Z), 0, .., 9;   and (\\\" or active \"):\n");
  printf("%%%%         -g1  GermanTelefonebookStyle:  \"a = \\\"a = ae, ..., \"s = \\ss = ss;\n");   /* gweight */
  printf("%%%%         -g2  ModernGermanDictionary:   \"a = \\\"a = a,  ..., \"s = \\ss = ss.\n");   /* mweight */
  printf("%%%%     -x  DoNotExpect german.sty: Active \"-characters do NOT produce umlauts.\n");
  printf("%%%%     -k  Idemize ~ multiple used authors in the %s and %s lists (kill).\n", VLI_SUFFIX, VQU_SUFFIX);
  printf("%%%%     -d  \\include{ / is \\ } (dos).   -h  Sort a hyphen as a space.\n");
  printf("%%%%   -i=j  Sort \'i\' as \'j\'.            -p  Sort \"P.S.\" before \"Peter\" (point).\n");
  printf("%%%%     -l  Ignore spaces (leer).       -c  Don\'t set bad page/fnt nums in {}.\n");    /*  TEXTSYMBOPEN[0], TEXTSYMBCLOSE[0] */
  printf("%%%%     -r  Typeout license (read).    -\?\?  -\?\?\?  More or more more options.\n");
  printf("%%%%\n");
}


/*--------------------------------------------------------------------------*/


int isexactlcmd(const char *p, const char *s)
{
  while(*p == *s && *s != '\t' && *s != '\0' ) { ++p; ++s; }
  if (*s == '\t' && isLaTeXcmdletter(*p) == 0) return 0;
  return 1;
}


int num_of_cmds_in_str(const char *p, const char *s)
{
  int r = 0;

  while (*p != '\0')
  {
    if (*p == '\\')
    {
      if (*(p+1) == '\\') ++p;
      else
      if (isexactlcmd(p, s) == 0) ++r;
    }
    ++p;
  }

  return r;
}


int nextbracket(const char *p, const char opench, const char closech)
{
    int c = 0, r = 0;

    while (*p != '\0')
    {
       if (*p == '\\' && *(p+1) != '\0') { ++p; ++r; }
       else if (*p == opench) ++c;
       else if (*p == closech) --c;
       ++p; ++r;
       if (c == 0) break;
    }

    if (c != 0) r = -r;

    return r;
}


int artithmetic_overflow(const char *p, const char *cc)
{
  printf("%%%%\n%%%%>  Warning: Arithmetic overflow %s-eval \"%s\".\n", p, cc);

  return INT_MAX;
}


int getarabicnum(const char *p)
{
   int num = 0, vor = 1;
   const char *mem;

   while (*p == ' ') ++p;  mem = p;

   if (*p == '-') { vor = -1; ++p; }

   while (*p >= '0' && *p <= '9' && num >= 0)
     num = (10 * num) + (*p++ - '0');

   if (num < 0) num = artithmetic_overflow("arabic", mem);

   return num * vor;
}


int firstoverlettergan(const char *p)
{
   while (*p == ' ' || isalpha(*p) != 0) ++p;

   return getarabicnum(p);
}


int secondoverlettergan(const char *p)
{
   while (*p == ' ' || isalpha(*p) != 0) ++p;
   while (*p >= '0' && *p <= '9')        ++p;
   while (*p == ' ' || isalpha(*p) != 0) ++p;

   return getarabicnum(p);
}


int getLetterletternum(const char *p)
{
   int num = 0, vor = 1;
   const char *mem;

   while (*p == ' ') ++p;  mem = p;

   if (*p == '-') { vor = -1; ++p; }

   while (num >= 0)
   {
          if (*p >= 'A' && *p <= 'Z') num = (26 * num) + (*p++ - 'A' + 1);
     else if (*p >= 'a' && *p <= 'z') num = (26 * num) + (*p++ - 'a' + 1);
     else break;
   }

   if (num < 0) num = artithmetic_overflow("a/Alph", mem);

   return num * vor;
}


int getRomanromannum(const char *p)
{
   int val = 0, nextval, num = 0, vor = 1;
   const char *mem;

   while (*p == ' ') ++p;  mem = p;

   if (*p == '-') { vor = -1; ++p; }

   while (*p != '\0' && num >= 0)
   {
      if      (*p == 'i' || *p == 'I') val =    1;
      else if (*p == 'v' || *p == 'V') val =    5;
      else if (*p == 'x' || *p == 'X') val =   10;
      else if (*p == 'l' || *p == 'L') val =   50;
      else if (*p == 'c' || *p == 'C') val =  100;
      else if (*p == 'd' || *p == 'D') val =  500;
      else if (*p == 'm' || *p == 'M') val = 1000;
      else break;

      if      (*(p+1) == '\0') nextval = 0;
      else if (*(p+1) == 'i' || *(p+1) == 'I') nextval =    1;
      else if (*(p+1) == 'v' || *(p+1) == 'V') nextval =    5;
      else if (*(p+1) == 'x' || *(p+1) == 'X') nextval =   10;
      else if (*(p+1) == 'l' || *(p+1) == 'L') nextval =   50;
      else if (*(p+1) == 'c' || *(p+1) == 'C') nextval =  100;
      else if (*(p+1) == 'd' || *(p+1) == 'D') nextval =  500;
      else if (*(p+1) == 'm' || *(p+1) == 'M') nextval = 1000;
      else nextval = 0;

      if (nextval > val) { num = num - val + nextval; ++p; }
      else                 num = num + val;

      ++p;
   }

   if (num < 0) num = artithmetic_overflow("r/Roman", mem);

   return num * vor;
}

/*--------------------------------------------------------------------------*/


int has_o_flow(const char *p)
{
   int r = HASNOOFLOW;

   if (isinstr(p, OVERFLOWTXT) == 1) r = HASOFLOW;

   return r;
}


int endnumberline(const char *p, int r)
{
  int len = strlen(RELAX);

  while (*p == ' ') ++p;
  if    (*p == '}') ++p;
  while (strncmp(p, RELAX, len) == 0) p+=len;
  while (*p == ' ') ++p;

  if    (*p != '\0') r = BADNUM;

  return r;
}


/*  ALT */
/* "*\relax "                    */   /* 1 */
/* "\dagger \relax "             */   /* 2 */
/* "\ddagger \relax "            */   /* 3 */
/* "\mathchar "278\relax "       */   /* 4 */
/* "\mathchar "27B\relax "       */   /* 5 */
/* "\delimiter "26B30D \relax "  */   /* 6 */
/* "**\relax "                   */   /* 7 */
/* "\dagger \dagger \relax "     */   /* 8 */
/* "\ddagger \ddagger \relax "   */   /* 9 */

/*  NEU */
/* "*"                           */   /* 1 */
/* "\dagger "                    */   /* 2 */
/* "\ddagger "                   */   /* 3 */
/* "\mathchar "278"              */   /* 4 */
/* "\mathchar "27B"              */   /* 5 */
/* "\delimiter "026B30D "        */   /* 6 */
/* "**"                          */   /* 7 */
/* "\dagger \dagger "            */   /* 8 */
/* "\ddagger \ddagger "          */   /* 9 */

/*  GANZ NEU */
/*  "\ensuremath  {*}"                     */   /* 1 */
/*  "\ensuremath  {\dagger }"              */   /* 2 */
/*  "\ensuremath  {\ddagger }"             */   /* 3 */
/*  "\ensuremath  {\mathsection }"         */   /* 4 */
/*  "\ensuremath  {\mathparagraph }"       */   /* 5 */
/*  "\ensuremath  {\delimiter "026B30D }"  */   /* 6 */
/*  "\ensuremath  {**}"                    */   /* 7 */
/*  "\ensuremath  {\dagger \dagger }"      */   /* 8 */
/*  "\ensuremath  {\ddagger \ddagger }"    */   /* 9 */

/*  NOCH NEUER */
/*  \TextOrMath  {\textasteriskcentered }{*}                          */   /* 1 */
/*  \TextOrMath  {\textdagger }{\dagger }                             */   /* 2 */
/*  \TextOrMath  {\textdaggerdbl }{\ddagger }                         */   /* 3 */
/*  \TextOrMath  {\textsection }{\mathsection }                       */   /* 4 */
/*  \TextOrMath  {\textparagraph }{\mathparagraph }                   */   /* 5 */
/*  \TextOrMath  {\textbardbl }{\delimiter "026B30D }                 */   /* 6 */
/*  \TextOrMath  {\textasteriskcentered \textasteriskcentered }{**}   */   /* 7 */
/*  \TextOrMath  {\textdagger \textdagger }{\dagger \dagger }         */   /* 8 */
/*  \TextOrMath  {\textdaggerdbl \textdaggerdbl }{\ddagger \ddagger } */   /* 9 */

/*  2021 vorgefunden (parallel zu BibArts 2.3) */
/*  \TextOrMath  {\textasteriskcentered }{*}                          */   /* 1 */
/*  \TextOrMath  {\textdagger }{\dagger }                             */   /* 2 */
/*  \TextOrMath  {\textdaggerdbl }{\ddagger }                         */   /* 3 */
/*  \TextOrMath  {\textsection }{\mathsection }                       */   /* 4 */
/*  \TextOrMath  {\textparagraph }{\mathparagraph }                   */   /* 5 */
/*  \TextOrMath  {\textbardbl }{\|}                                   */   /* 6 */
/*  \TextOrMath  {\textasteriskcentered \textasteriskcentered }{**}   */   /* 7 */
/*  \TextOrMath  {\textdagger \textdagger }{\dagger \dagger }         */   /* 8 */
/*  \TextOrMath  {\textdaggerdbl \textdaggerdbl }{\ddagger \ddagger } */   /* 9 */



int nextsymbolnum(int oldnum, int beginnum)
{
   if (oldnum == 0) return beginnum;
   else             return oldnum + 6;
}


int len_tom(const char *p)
{
   int len, r = 0;

   if (lcmdcmp(p, TOMFNSYMBOL) == 0)
   {
     p += (r=strlen(TOMFNSYMBOL));

     while (*p == ' ') { ++p; ++r; }

     if    (*p == '{')
     {
       /* erstes Arg von NOCH NEUER */
       len = nextbracket(p, '{', '}');
       if (len > 1) 
       {      r += len; 
       }
       else   r = 0;
     }
     else     r = 0;
   }

  return r;
}


int evalsymbol(int pg, const char *p, const int job)
{
   int r = BADNUM, num = 0, len, cmdtyp = 0, cmd = 0, vor = 1;
   const char *mem;

   while (*p == ' ') ++p;

   if (pg == 1)
   {
     if (lcmdcmp(p, NEWFNSYMBOL) == 0)
          p += strlen(NEWFNSYMBOL);
     else p += len_tom(p);
   }

   while (*p == ' ') ++p;
   if    (*p == '{') ++p;
                           mem = p;

   if (*p == '-') { vor = -1; ++p; }

   while (num >= 0)
   {
          if ((cmd == 0 || cmd == 6) && cmdtyp  < 2 && strncmp(p, "\\|", (len=strlen("\\|"))) == 0)  /* new in 2.3 */
        {
          r = GOODNUM;
          num = nextsymbolnum(num, (cmd = 6));
          cmdtyp = 1;
        }
     else if ((cmd == 0 || cmd == 6) && cmdtyp  < 2 && strncmp(p, "\\delimiter", (len=strlen("\\delimiter"))) == 0)
        {
          r = BADNUM;     /* folgen muessen \"26B30D oder \"026B30D */
          cmdtyp = 2;     /*                cmd == 6      cmd == 6  */
        }
     else if ((cmd == 0 || cmd == 6) && cmdtyp == 2 && strncmp(p, "\"26B30D", (len=strlen("\"26B30D"))) == 0)
        {
          r = GOODNUM;
          num = nextsymbolnum(num, (cmd = 6));
          cmdtyp = 1;
        }
     else if ((cmd == 0 || cmd == 6) && cmdtyp == 2 && strncmp(p, "\"026B30D", (len=strlen("\"026B30D"))) == 0)
        {
          r = GOODNUM;
          num = nextsymbolnum(num, (cmd = 6));     /* ebenfalls */
          cmdtyp = 1;
        }
     else if ((cmd == 0 || cmd == 4 || cmd == 5) && cmdtyp  < 2 && strncmp(p, "\\mathchar", (len=strlen("\\mathchar"))) == 0)
        {
          r = BADNUM;     /* folgen muessen \"27B oder \"278  */
          cmdtyp = 3;     /*              cmd == 5   cmd == 4 */
        }
     else if ((cmd == 0 || cmd == 5) && cmdtyp == 3 && strncmp(p, "\"27B", (len=strlen("\"27B"))) == 0)
        {
          r = GOODNUM;
          num = nextsymbolnum(num, (cmd = 5));
          cmdtyp = 1;
        }
     else if ((cmd == 0 || cmd == 5) && cmdtyp  < 2 && strncmp(p, "\\mathparagraph", (len=strlen("\\mathparagraph"))) == 0)
        {
          r = GOODNUM;
          num = nextsymbolnum(num, (cmd = 5));
          cmdtyp = 1;
        }
     else if ((cmd == 0 || cmd == 4) && cmdtyp == 3 && strncmp(p, "\"278", (len=strlen("\"278"))) == 0)
        {
          r = GOODNUM;
          num = nextsymbolnum(num, (cmd = 4));
          cmdtyp = 1;
        }
     else if ((cmd == 0 || cmd == 4) && cmdtyp  < 2 && strncmp(p, "\\mathsection", (len=strlen("\\mathsection"))) == 0)
        {
          r = GOODNUM;
          num = nextsymbolnum(num, (cmd = 4));
          cmdtyp = 1;
        }
     else if ((cmd == 0 || cmd == 3) && cmdtyp  < 2 && strncmp(p, "\\ddagger", (len=strlen("\\ddagger"))) == 0)
        {
          r = GOODNUM;
          num = nextsymbolnum(num, (cmd = 3));
          cmdtyp = 1;
        }
     else if ((cmd == 0 || cmd == 2) && cmdtyp  < 2 && strncmp(p, "\\dagger", (len=strlen("\\dagger"))) == 0)
        {
          r = GOODNUM;
          num = nextsymbolnum(num, (cmd = 2));
          cmdtyp = 1;
        }
     else if ((cmd == 0 || cmd == 1) && cmdtyp  < 2 && strncmp(p, "*", (len=strlen("*"))) == 0)
        {
          r = GOODNUM;
          num = nextsymbolnum(num, (cmd = 1));
          cmdtyp = 0;
        }
     else if (cmdtyp  > 0 && *p ==  ' ') len=1;
     else break;

     p += len;
   }

   if (num < 0) num = artithmetic_overflow("fnsymbol", mem);

   r = endnumberline(p, r);

   if (job == CHECKNUM) return r;
   else                 return num * vor;
}


int evalroman(const char *p, const int job)
{
   int r = BADNUM, num = 0;
   const char *mem;

   while (*p == ' ') ++p;  mem = p;

   if (*p == '-') ++p;

   while (*p != '\0')
   {
     if (    *p == 'i'
          || *p == 'v'
          || *p == 'x'
          || *p == 'l'
          || *p == 'c'
          || *p == 'd'
          || *p == 'm'
        ) 
        { ++p;
          r = GOODNUM;
        }
     else break;
   }

   r = endnumberline(p, r);

   if (job != CHECKNUM && r == GOODNUM) num = getRomanromannum(mem);

   if (job == CHECKNUM) return r;
   else                 return num;
}


int evalRoman(const char *p, const int job)
{
   int r = BADNUM, num = 0, len = strlen(OLDBIGROMAN), vor = 1;
   const char *mem;

   while (*p == ' ') ++p;  mem = p;

 if (     strncmp(p, OLDBIGROMAN, len) == 0
      || (*p == '-' && strncmp((p+1), OLDBIGROMAN, len) == 0)
    )
 {
   if (*p == '-') { vor = -1; ++p; }
   p += len;
   while (*p == ' ') ++p;
   if (*p == '{')
   { ++p;                /* kleine roem. Zahl, die gross gedruckt wird */
     if (*p == '}') r = GOODNUM;   /* OLDBIGROMAN + " {}" */
     else
     r = evalroman(p, CHECKNUM);  /* Der job ist in jedem Fall checken */
     if (vor < 0 && (*p == ' ' || *p == '-')) r = BADNUM;
     mem = p;
   }
   else r = BADNUM;
 }
 else
 {
   if (*p == '-') ++p;

   while (*p != '\0')
   {
     if (    *p == 'I'
          || *p == 'V'
          || *p == 'X'
          || *p == 'L'
          || *p == 'C'
          || *p == 'D'
          || *p == 'M'
        ) 
        { ++p;
          r = GOODNUM;
        }
     else break;
   }

   r = endnumberline(p, r);
 }

   if (job != CHECKNUM && r == GOODNUM) 
   { num = getRomanromannum(mem);
     if (num < 0 && vor < 0) r = BADNUM;
   }

   if (job == CHECKNUM) return r;
   else                 return num * vor;  /* vor == -1 von OLDBIGROMAN */
}


int evalarabic(const char *p, const int job)
{
   int r = BADNUM, num = 0;
   const char *mem;

   while (*p == ' ') ++p;  mem = p;

   if (*p == '-') ++p;

   while (*p != '\0')
   {
     if (    *p >= '0' 
          && *p <= '9'
        ) 
        { ++p;
          r = GOODNUM;
        }
     else break;
   }

   r = endnumberline(p, r);

   if (job != CHECKNUM && r == GOODNUM) num = getarabicnum(mem);

   if (job == CHECKNUM) return r;
   else                 return num;
}


int evalletter(const char *p, const int job)
{
   int r = BADNUM, num = 0;
   const char *mem;

   while (*p == ' ') ++p;  mem = p;

   if (*p == '-') ++p;

   while (*p != '\0')
   {
     if (    *p >= 'a' 
          && *p <= 'z'
        ) 
        { ++p;
          r = GOODNUM;
        }
     else break;
   }

   r = endnumberline(p, r);

   if (job != CHECKNUM && r == GOODNUM) num = getLetterletternum(mem);

   if (job == CHECKNUM) return r;
   else                 return num;
}


int evalLetter(const char *p, const int job)
{
   int r = BADNUM, num = 0;
   const char *mem;

   while (*p == ' ') ++p;  mem = p;

   if (*p == '-') ++p;

   while (*p != '\0')
   {
     if (    *p >= 'A' 
          && *p <= 'Z'
        ) 
        { ++p;
          r = GOODNUM;
        }
     else break;
   }

   r = endnumberline(p, r);

   if (job != CHECKNUM && r == GOODNUM) num = getLetterletternum(mem);

   if (job == CHECKNUM) return r;
   else                 return num;
}


int isinstr(const char *p, const char *s)
{
   const char *pp, *ss;

   while (*p != '\0')
   {
     pp = p; ss = s;
     while (*pp ==  *ss && *pp != '\0') { ++pp; ++ss; }
     if    (*ss == '\0') return 1;
     ++p;
   }

   return 0;
}


   /*    Typen-Gewichtung nicht mit Abpruef-Reihenfolge verwechseln!    */
   /* Zahl fuer Typen-Gewichtung ist gleichzeitig Typen-Identifizierer. */
   /* Abpruef-Reihenfolge legen pgnumberweight + fntnumberweight fest,  */
   /* die auch entscheiden, ob etwa DC=600 oder DC=4*3=12 gesehen wird. */
   /* !! \uppercase{xx} = OLDBIGROMAN gilt NIE als Buchstabenzaehler !! */
#define INTERNALERR  0
#define UNUSED       1     /* muss anfuehren (kleinster Wert)! */
#define NOFOOTNOTE   2
#define EMPTYNUM     3     /* Alph, alph, Roman, roman, symbol < 1 */
#define BEGINWEIGHT  4
int  pg_symbol      = 4,  fn_symbol      = 4;
int  pg_smallroman  = 5,  fn_smallroman  = 5;
int  pg_bigroman    = 6,  fn_bigroman    = 6;
int  pg_arabic      = 7,  fn_arabic      = 7;
int  pg_smallletter = 8,  fn_smallletter = 8;
int  pg_bigletter   = 9,  fn_bigletter   = 9;
#define ENDWEIGHT   10
#define TEXT        10
#define MISSNUM     11     /* Beschaedigte Zeile */
#define OVERFLOW    12

int pgnumberweight(const char *p)
{
   int r = INTERNALERR;  /* primaer i, ii, ..., 1, 2, ..., A, B, *C*, ... */


   if (    get_R_pg == BA_TRUE
        || get_A_pg == BA_TRUE
        || get_a_pg == BA_TRUE
        || get_r_pg == BA_TRUE
   )
   {
          if (p == unused)                        r = UNUSED;
     else if (p == mydefault)                     r = MISSNUM;
     else if (p == nofnt)                         r = INTERNALERR;
     else if (*p == '\0')                         r = EMPTYNUM;
     else if (has_o_flow(p)          == HASOFLOW) r = OVERFLOW;
     else if (evalarabic(p, CHECKNUM) == GOODNUM) r = pg_arabic;
     else if (get_R_pg == BA_TRUE && evalRoman (p, CHECKNUM) == GOODNUM) r = pg_bigroman;
     else if (get_A_pg == BA_TRUE && evalLetter(p, CHECKNUM) == GOODNUM) r = pg_bigletter;
     else if (get_a_pg == BA_TRUE && evalletter(p, CHECKNUM) == GOODNUM) r = pg_smallletter;
     else if (get_r_pg == BA_TRUE && evalroman (p, CHECKNUM) == GOODNUM) r = pg_smallroman;
     else if (evalsymbol(1, p, CHECKNUM) == GOODNUM) r = pg_symbol;
     else                                         r = TEXT;
   }
   else
   if (EinlROEMseiten == 1)
   {
          if (p == unused)                        r = UNUSED;
     else if (p == mydefault)                     r = MISSNUM;
     else if (p == nofnt)                         r = INTERNALERR;
     else if (*p == '\0')                         r = EMPTYNUM;
     else if (has_o_flow(p)          == HASOFLOW) r = OVERFLOW;
     else if (evalarabic(p, CHECKNUM) == GOODNUM) r = pg_arabic;
     else if (evalRoman (p, CHECKNUM) == GOODNUM) r = pg_bigroman;
     else if (evalLetter(p, CHECKNUM) == GOODNUM) r = pg_bigletter;
     else if (evalletter(p, CHECKNUM) == GOODNUM) r = pg_smallletter;
     else if (evalroman (p, CHECKNUM) == GOODNUM) r = pg_smallroman;
     else if (evalsymbol(1, p, CHECKNUM) == GOODNUM) r = pg_symbol;
     else                                         r = TEXT;
   }
   else
   {
          if (p == unused)                        r = UNUSED;
     else if (p == mydefault)                     r = MISSNUM;
     else if (p == nofnt)                         r = INTERNALERR;
     else if (*p == '\0')                         r = EMPTYNUM;
     else if (has_o_flow(p)          == HASOFLOW) r = OVERFLOW;
     else if (evalarabic(p, CHECKNUM) == GOODNUM) r = pg_arabic;
     else if (evalroman (p, CHECKNUM) == GOODNUM) r = pg_smallroman;
     else if (evalletter(p, CHECKNUM) == GOODNUM) r = pg_smallletter;
     else if (evalLetter(p, CHECKNUM) == GOODNUM) r = pg_bigletter;
     else if (evalRoman (p, CHECKNUM) == GOODNUM) r = pg_bigroman;
     else if (evalsymbol(1, p, CHECKNUM) == GOODNUM) r = pg_symbol;
     else                                         r = TEXT;
   }

     /*  OLDBIGROMAN + " {}"  */
     if (r == pg_bigroman && evalRoman (p, ANALYZENUM) == 0) r = EMPTYNUM;

   return r;
}


int fntnumberweight(const char *p)
{
   int r = INTERNALERR;  /* primaer I, II, ..., 1, 2, ..., a, b, *c*, ... */


   if (    get_R_fn == BA_TRUE
        || get_A_fn == BA_TRUE
        || get_a_fn == BA_TRUE
        || get_r_fn == BA_TRUE
   )
   {
          if (p == mydefault)                     r = MISSNUM;
     else if (p == nofnt)                         r = NOFOOTNOTE;
     else if (p == unused)                        r = INTERNALERR;
     else if (*p == '\0')                         r = EMPTYNUM;
     else if (has_o_flow(p)          == HASOFLOW) r = OVERFLOW;
     else if (evalarabic(p, CHECKNUM) == GOODNUM) r = fn_arabic;
     else if (get_r_fn == BA_TRUE && evalroman (p, CHECKNUM) == GOODNUM) r = fn_smallroman;
     else if (get_a_fn == BA_TRUE && evalletter(p, CHECKNUM) == GOODNUM) r = fn_smallletter;
     else if (get_A_fn == BA_TRUE && evalLetter(p, CHECKNUM) == GOODNUM) r = fn_bigletter;
     else if (get_R_fn == BA_TRUE && evalRoman (p, CHECKNUM) == GOODNUM) r = fn_bigroman;
     else if (evalsymbol(0, p, CHECKNUM) == GOODNUM) r = fn_symbol;
     else                                         r = TEXT;
   }
   else
   if (ALPHmpfootnotes == 1)
   {
          if (p == mydefault)                     r = MISSNUM;
     else if (p == nofnt)                         r = NOFOOTNOTE;
     else if (p == unused)                        r = INTERNALERR;
     else if (*p == '\0')                         r = EMPTYNUM;
     else if (has_o_flow(p)          == HASOFLOW) r = OVERFLOW;
     else if (evalarabic(p, CHECKNUM) == GOODNUM) r = fn_arabic;
     else if (evalroman (p, CHECKNUM) == GOODNUM) r = fn_smallroman;
     else if (evalletter(p, CHECKNUM) == GOODNUM) r = fn_smallletter;
     else if (evalLetter(p, CHECKNUM) == GOODNUM) r = fn_bigletter;
     else if (evalRoman (p, CHECKNUM) == GOODNUM) r = fn_bigroman;
     else if (evalsymbol(0, p, CHECKNUM) == GOODNUM) r = fn_symbol;
     else                                         r = TEXT;
   }
   else
   {
          if (p == mydefault)                     r = MISSNUM;
     else if (p == nofnt)                         r = NOFOOTNOTE;
     else if (p == unused)                        r = INTERNALERR;
     else if (*p == '\0')                         r = EMPTYNUM;
     else if (has_o_flow(p)          == HASOFLOW) r = OVERFLOW;
     else if (evalarabic(p, CHECKNUM) == GOODNUM) r = fn_arabic;
     else if (evalRoman (p, CHECKNUM) == GOODNUM) r = fn_bigroman;
     else if (evalLetter(p, CHECKNUM) == GOODNUM) r = fn_bigletter;
     else if (evalletter(p, CHECKNUM) == GOODNUM) r = fn_smallletter;
     else if (evalroman (p, CHECKNUM) == GOODNUM) r = fn_smallroman;
     else if (evalsymbol(0, p, CHECKNUM) == GOODNUM) r = fn_symbol;
     else                                         r = TEXT;
   }

     /*  OLDBIGROMAN + " {}"  */
     if (r == fn_bigroman && evalRoman (p, ANALYZENUM) == 0) r = EMPTYNUM;

   return r;
}


/*--------------------------------------------------------------------------*/


int issonder(const char p)
{
  if (p > '\0' && p < ' ') return 1;
  return 0;
}


      /* Neugewichtung der ASCII-Zeichen 33 und 48 bis 126.  */

      /* Im Deutschen werden Zeichen vor Buchstaben und      */
      /* Buchstaben vor Zahlen einsortiert.                  */
      /* Im Konfliktfall stehen Kleinbuchstaben vor grossen. */

int bewerten(char zz)
{
  switch(zz)
  {
      case '\0': return   0;

      case ' ': return   32;  /*  ASCII  32  \ /  normales Leerzeichen      */
      case '~': return   33;  /*  ASCII 126  / \  geschuetztes Leereichen   */

      case '!': return   48;  /*  ASCII  33 \                               */
      case ':': return   49;  /*  ASCII  58  |                              */
      case ';': return   50;  /*  ASCII  59  |                              */
      case '<': return   51;  /*  ASCII  60  |                              */
      case '=': return   52;  /*  ASCII  61  |                              */
      case '>': return   53;  /*  ASCII  62  |                              */
      case '\?':return   54;  /*  ASCII  63  |                              */
      case '@': return   55;  /*  ASCII  64  |                              */
      case '[': return   56;  /*  ASCII  91   \                             */
      case ']': return   57;  /*  ASCII  93     > Zeichen vor Buchstaben    */
                              /*              /                             */
      case '^': return   58;  /*  ASCII  94  |                              */
      case '_': return   59;  /*  ASCII  95  |                              */
      case '`': return   60;  /*  ASCII  96  |                              */
                              /*             |                              */
      case '{': return   61;  /*  ASCII 123  |                              */
      case '|': return   62;  /*  ASCII 124  |                              */
      case '}': return   63;  /*  ASCII 125 /                               */
                                  
      case 'a': return   64;  /*  ASCII  97 \                               */
      case 'A': return   65;  /*  ASCII  65  |                              */
      case 'b': return   66;  /*  ASCII  98  |                              */
      case 'B': return   67;  /*  ASCII  66  |                              */
      case 'c': return   68;  /*  ASCII  99  |                              */
      case 'C': return   69;  /*  ASCII  67  |                              */
      case 'd': return   70;  /*  ASCII 100  |                              */
      case 'D': return   71;  /*  ASCII  68  |                              */
      case 'e': return   72;  /*  ASCII 101  |                              */
      case 'E': return   73;  /*  ASCII  69  |                              */
      case 'f': return   74;  /*  ASCII 102  |                              */
      case 'F': return   75;  /*  ASCII  70  |                              */
      case 'g': return   76;  /*  ASCII 103  |                              */
      case 'G': return   77;  /*  ASCII  71  |                              */
      case 'h': return   78;  /*  ASCII 104  |                              */
      case 'H': return   79;  /*  ASCII  72  |                              */
      case 'i': return   80;  /*  ASCII 105  |                              */
      case 'I': return   81;  /*  ASCII  73  |                              */
      case 'j': return   82;  /*  ASCII 106  |                              */
      case 'J': return   83;  /*  ASCII  74  |                              */
      case 'k': return   84;  /*  ASCII 107  |                              */
      case 'K': return   85;  /*  ASCII  75  |                              */
      case 'l': return   86;  /*  ASCII 108  |                              */
      case 'L': return   87;  /*  ASCII  76  |                              */
      case 'm': return   88;  /*  ASCII 109  |                              */
      case 'M': return   89;  /*  ASCII  77  |                              */
      case 'n': return   90;  /*  ASCII 110  |                              */
      case 'N': return   91;  /*  ASCII  78  |                              */
      case 'o': return   92;  /*  ASCII 111  |                              */
      case 'O': return   93;  /*  ASCII  79  |                              */
      case 'p': return   94;  /*  ASCII 112   \                             */
      case 'P': return   95;  /*  ASCII  80     > Buchstaben: a, A, ...     */
      case 'q': return   96;  /*  ASCII 113   /                             */
      case 'Q': return   97;  /*  ASCII  81  |                              */
      case 'r': return   98;  /*  ASCII 114  |                              */
      case 'R': return   99;  /*  ASCII  82  |                              */
      case 's': return  100;  /*  ASCII 115  |                              */
      case 'S': return  101;  /*  ASCII  83  |                              */
      case 't': return  102;  /*  ASCII 116  |                              */
      case 'T': return  103;  /*  ASCII  84  |                              */
      case 'u': return  104;  /*  ASCII 117  |                              */
      case 'U': return  105;  /*  ASCII  85  |                              */
      case 'v': return  106;  /*  ASCII 118  |                              */
      case 'V': return  107;  /*  ASCII  86  |                              */
      case 'w': return  108;  /*  ASCII 119  |                              */
      case 'W': return  109;  /*  ASCII  87  |                              */
      case 'x': return  110;  /*  ASCII 120  |                              */
      case 'X': return  111;  /*  ASCII  88  |                              */
      case 'y': return  112;  /*  ASCII 121  |                              */
      case 'Y': return  113;  /*  ASCII  89  |                              */
      case 'z': return  114;  /*  ASCII 122  |                              */
      case 'Z': return  115;  /*  ASCII  90 /                               */

      case '0': return  116;  /*  ASCII  48 \                               */
      case '1': return  117;  /*  ASCII  49  |                              */
      case '2': return  118;  /*  ASCII  50  |                              */
      case '3': return  119;  /*  ASCII  51  |                              */
      case '4': return  120;  /*  ASCII  52   \                             */
      case '5': return  121;  /*  ASCII  53     > Zahlen                    */
      case '6': return  122;  /*  ASCII  54   /                             */
      case '7': return  123;  /*  ASCII  55  |                              */
      case '8': return  124;  /*  ASCII  56  |                              */
      case '9': return  125;  /*  ASCII  57 /                               */

      case '\\':return  126;  /*  ASCII  92  >    Unbekannter Akzent?       */

      default : if (issonder(zz) == 1) return zz + 256;
      return   zz;
  }
}



      /* Im Defaultfall werden Zeichen vor Zahlen und        */
      /* Zahlen vor Buchstaben einsortiert.                  */
      /* Im Konfliktfall stehen Grossbuchstaben vor kleinen. */

int dcompare(char zz)
{
  switch(zz)
  {
      case '\0': return   0;

      case ' ': return   32;  /*  ASCII  32  \ /  normales Leerzeichen      */
      case '~': return   33;  /*  ASCII 126  / \  geschuetztes Leereichen   */

      case '!': return   48;  /*  ASCII  33 \                               */
      case ':': return   49;  /*  ASCII  58  |                              */
      case ';': return   50;  /*  ASCII  59  |                              */
      case '<': return   51;  /*  ASCII  60  |                              */
      case '=': return   52;  /*  ASCII  61  |                              */
      case '>': return   53;  /*  ASCII  62  |                              */
      case '\?':return   54;  /*  ASCII  63  |                              */
      case '@': return   55;  /*  ASCII  64  |                              */
      case '[': return   56;  /*  ASCII  91   \                             */
      case ']': return   57;  /*  ASCII  93     > Zeichen vor Zahlen        */
                              /*              /                             */
      case '^': return   58;  /*  ASCII  94  |                              */
      case '_': return   59;  /*  ASCII  95  |                              */
      case '`': return   60;  /*  ASCII  96  |                              */
                              /*             |                              */
      case '{': return   61;  /*  ASCII 123  |                              */
      case '|': return   62;  /*  ASCII 124  |                              */
      case '}': return   63;  /*  ASCII 125 /                               */
                                  
      case '0': return   64;  /*  ASCII  48 \                               */
      case '1': return   65;  /*  ASCII  49  |                              */
      case '2': return   66;  /*  ASCII  50  |                              */
      case '3': return   67;  /*  ASCII  51  |                              */
      case '4': return   68;  /*  ASCII  52   \                             */
      case '5': return   69;  /*  ASCII  53     > Zahlen vor Buchstaben     */
      case '6': return   70;  /*  ASCII  54   /                             */
      case '7': return   71;  /*  ASCII  55  |                              */
      case '8': return   72;  /*  ASCII  56  |                              */
      case '9': return   73;  /*  ASCII  57 /                               */

      case 'A': return   74;  /*  ASCII  65 \                               */
      case 'a': return   75;  /*  ASCII  97  |                              */
      case 'B': return   76;  /*  ASCII  66  |                              */
      case 'b': return   77;  /*  ASCII  98  |                              */
      case 'C': return   78;  /*  ASCII  67  |                              */
      case 'c': return   79;  /*  ASCII  99  |                              */
      case 'D': return   80;  /*  ASCII  68  |                              */
      case 'd': return   81;  /*  ASCII 100  |                              */
      case 'E': return   82;  /*  ASCII  69  |                              */
      case 'e': return   83;  /*  ASCII 101  |                              */
      case 'F': return   84;  /*  ASCII  70  |                              */
      case 'f': return   85;  /*  ASCII 102  |                              */
      case 'G': return   86;  /*  ASCII  71  |                              */
      case 'g': return   87;  /*  ASCII 103  |                              */
      case 'H': return   88;  /*  ASCII  72  |                              */
      case 'h': return   89;  /*  ASCII 104  |                              */
      case 'I': return   90;  /*  ASCII  73  |                              */
      case 'i': return   91;  /*  ASCII 105  |                              */
      case 'J': return   92;  /*  ASCII  74  |                              */
      case 'j': return   93;  /*  ASCII 106  |                              */
      case 'K': return   94;  /*  ASCII  75  |                              */
      case 'k': return   95;  /*  ASCII 107  |                              */
      case 'L': return   96;  /*  ASCII  76  |                              */
      case 'l': return   97;  /*  ASCII 108  |                              */
      case 'M': return   98;  /*  ASCII  77  |                              */
      case 'm': return   99;  /*  ASCII 109  |                              */
      case 'N': return  100;  /*  ASCII  78  |                              */
      case 'n': return  101;  /*  ASCII 110  |                              */
      case 'O': return  102;  /*  ASCII  79  |                              */
      case 'o': return  103;  /*  ASCII 111   \                             */
      case 'P': return  104;  /*  ASCII  80     > Buchstaben: A, a, ...     */
      case 'p': return  105;  /*  ASCII 112   /                             */
      case 'Q': return  106;  /*  ASCII  81  |                              */
      case 'q': return  107;  /*  ASCII 113  |                              */
      case 'R': return  108;  /*  ASCII  82  |                              */
      case 'r': return  109;  /*  ASCII 114  |                              */
      case 'S': return  110;  /*  ASCII  83  |                              */
      case 's': return  111;  /*  ASCII 115  |                              */
      case 'T': return  112;  /*  ASCII  84  |                              */
      case 't': return  113;  /*  ASCII 116  |                              */
      case 'U': return  114;  /*  ASCII  85  |                              */
      case 'u': return  115;  /*  ASCII 117  |                              */
      case 'V': return  116;  /*  ASCII  86  |                              */
      case 'v': return  117;  /*  ASCII 118  |                              */
      case 'W': return  118;  /*  ASCII  87  |                              */
      case 'w': return  119;  /*  ASCII 119  |                              */
      case 'X': return  120;  /*  ASCII  88  |                              */
      case 'x': return  121;  /*  ASCII 120  |                              */
      case 'Y': return  122;  /*  ASCII  89  |                              */
      case 'y': return  123;  /*  ASCII 121  |                              */
      case 'Z': return  124;  /*  ASCII  90  |                              */
      case 'z': return  125;  /*  ASCII 122 /                               */

      case '\\':return  126;  /*  ASCII  92  >    Unbekannter Akzent?       */

      default : if (issonder(zz) == 1) return zz + 256;
      return   zz;
  }
}


/*--------------------------------------------------------------------------*/


int igspstrcmp(const char *p, const char *q)
{
   while (1)
   {
     while (*p == ' ') ++p;
     while (*q == ' ') ++q;
     if (*p == *q && *p != '\0') { ++p; ++q; } else break;
   }

   return bewerten(*p) - bewerten(*q);
}


int numcmp(const int pg, const int w, const char *p, const char *q)
{
   int j = 0;

        if (w == EMPTYNUM)    j = 0;
   else if (w == NOFOOTNOTE)  j = 0;
   else if (w == UNUSED)      j = 0;
   else if (pg == 1 && w == pg_smallroman)  j = evalroman (p, ANALYZENUM) - evalroman (q, ANALYZENUM);
   else if (pg == 0 && w == fn_smallroman)  j = evalroman (p, ANALYZENUM) - evalroman (q, ANALYZENUM);
   else if (pg == 1 && w == pg_bigroman)    j = evalRoman (p, ANALYZENUM) - evalRoman (q, ANALYZENUM);
   else if (pg == 0 && w == fn_bigroman)    j = evalRoman (p, ANALYZENUM) - evalRoman (q, ANALYZENUM);
   else if (pg == 1 && w == pg_arabic)      j = evalarabic(p, ANALYZENUM) - evalarabic(q, ANALYZENUM);
   else if (pg == 0 && w == fn_arabic)      j = evalarabic(p, ANALYZENUM) - evalarabic(q, ANALYZENUM);
   else if (pg == 1 && w == pg_smallletter) j = evalletter(p, ANALYZENUM) - evalletter(q, ANALYZENUM);
   else if (pg == 0 && w == fn_smallletter) j = evalletter(p, ANALYZENUM) - evalletter(q, ANALYZENUM);
   else if (pg == 1 && w == pg_bigletter)   j = evalLetter(p, ANALYZENUM) - evalLetter(q, ANALYZENUM);
   else if (pg == 0 && w == fn_bigletter)   j = evalLetter(p, ANALYZENUM) - evalLetter(q, ANALYZENUM);
   else if (pg == 1 && w == pg_symbol)      j = evalsymbol(1, p, ANALYZENUM) - evalsymbol(1, q, ANALYZENUM);
   else if (pg == 0 && w == fn_symbol)      j = evalsymbol(0, p, ANALYZENUM) - evalsymbol(0, q, ANALYZENUM);
   else if (w == TEXT)        j = igspstrcmp(p,                          q);
   else if (w == MISSNUM)     j = 0;
   else if (w == OVERFLOW)    j = 0;
   else intern_err(2);   /* UNKNOWN PAGE-WEIGHT NUMBER */


   return j;
}


void QQsort(int left, int right, const int deep, const int deepsort)
{
  int i, last, j, l;

  if (left >= right) return;
  swap(left, (left + right)/2);
  last = left;

  for (i = left + 1; i <= right; i++)
  {  
     j = 0; l = 0;
     while (l < deepsort)
     { 
       if ((j=vergleich(subCptr[l][i], subCptr[l][left])) != 0) break;
       if (*subAptr[l][i] == '\0' && *subAptr[l][left] != '\0') { j = -1; break; }  /* new in 2.3 */
       if (*subAptr[l][i] != '\0' && *subAptr[l][left] == '\0') { j =  1; break; }  /* new in 2.3 */
       if ((j=Vergleich(subBptr[l][i], subBptr[l][left])) != 0) break;
       if (*subBptr[l][i] == '\0' && *subBptr[l][left] == '\0')    /* M */  /* new in 2.3 */
       {
       if ((j=vergleich(subAptr[l][i], subAptr[l][left])) != 0) break;      /* new in 2.3 */
       }
       else
       if ((j=vergleich(subBptr[l][i], subBptr[l][left])) != 0) break;
       ++l;
     }


     if (j == 0)
     {
       l = 0;
       while (l < deepsort)
       { 
         if (*subBptr[l][i] != '\0' || *subBptr[l][left] != '\0')  /* M */  /* new in 2.3 */
         {
         if ((j=vergleich(subAptr[l][i], subAptr[l][left])) != 0) break;
         }
         ++l;
       }
     }


     if (j == 0)
         j =   leval[i] -   leval[left];

     if (j == 0)
         j = pagewgt[i] - pagewgt[left];

     if (j == 0)   /* pagewgt[i] == pagewgt[left] */
         j = numcmp(1, pagewgt[i], subRptr[PAGECOL][i], subRptr[PAGECOL][left]);

     if (j == 0)
         j =  fntwgt[i] -  fntwgt[left];

     if (j == 0)   /*  fntwgt[i] ==  fntwgt[left] */
         j = numcmp(0,  fntwgt[i], subRptr[FNTCOL][i],  subRptr[FNTCOL][left]);

     if (j == 0)
         j =  firstoverlettergan(subRptr[LINECOL][i]) -  firstoverlettergan(subRptr[LINECOL][left]);

     if (j == 0)
         j = secondoverlettergan(subRptr[LINECOL][i]) - secondoverlettergan(subRptr[LINECOL][left]);

     if (j == 0)
     {
       l = deepsort;
       while (l < deep) 
       {
         if ((j=vergleich(subAptr[l][i], subAptr[l][left])) != 0) break;
         ++l;
       }
     }


     if (j < 0) swap(++last, i);
  }


  swap(left, last);
  QQsort(left, last - 1, deep, deepsort);
  QQsort(last + 1, right, deep, deepsort);
}


/*--------------------------------------------------------------------------*/


void swap(int i, int j)
{
   char *temp;
   int l, temi;

         temp = lineptr[i];
   lineptr[i] = lineptr[j];
   lineptr[j] = temp;

         temi = pagewgt[i];
   pagewgt[i] = pagewgt[j];
   pagewgt[j] = temi;

         temi =  fntwgt[i];
    fntwgt[i] =  fntwgt[j];
    fntwgt[j] = temi;

         temi =  dqsort[i];
    dqsort[i] =  dqsort[j];
    dqsort[j] = temi;

         temi =  leval[i];
     leval[i] =  leval[j];
     leval[j] = temi;

         temi =  inerr[i];  /* new in 2.5 */
     inerr[i] =  inerr[j];
     inerr[j] = temi;

   l = 0;
   while (l < MAXBACOLS)
   {
               temp = subAptr[l][i];
      subAptr[l][i] = subAptr[l][j];
      subAptr[l][j] = temp;
      ++l;
   }

   l = 0;
   while (l < MAXBACOLS)
   {
               temp = subBptr[l][i];
      subBptr[l][i] = subBptr[l][j];
      subBptr[l][j] = temp;
      ++l;
   }

   l = 0;
   while (l < MAXBACOLS)
   {
               temp = subCptr[l][i];
      subCptr[l][i] = subCptr[l][j];
      subCptr[l][j] = temp;
      ++l;
   }

   l = 0;
   while (l < BAALLRCOLS)
   {
               temp = subRptr[l][i];
      subRptr[l][i] = subRptr[l][j];
      subRptr[l][j] = temp;
      ++l;
   }
}


/*--------------------------------------------------------------------------*/


int vergleich(const char *a, const char *b)
{
   while (*a != '\0' && *a == *b) { ++a; ++b; }

   if (german > 0) return bewerten(*a) - bewerten(*b);
   else            return dcompare(*a) - dcompare(*b);
}


int Vergleich(const char *a, const char *b)
{
   while (1)
   { 
     while (issonder(*a) == 1) ++a;
     while (issonder(*b) == 1) ++b;
     if (*a == '\0' || *a != *b) break;
     ++a; ++b; 
   }

   if (german > 0) return bewerten(*a) - bewerten(*b);
   else            return dcompare(*a) - dcompare(*b);
}


/*--------------------------------------------------------------------------*/


int myfgetc(void)
{
   int c = fgetc(infile);

   if (c == '\t') c = ' ';

   return c;
}


int ignFRONTSPACESgetline(char s[], int lim) 
{
        int c,i;
        i=0;
        while(             (c=myfgetc()) == ' ');
        if   (--lim > 0 &&  c            != EOF && c != '\n')
                s[i++] = c;
        while(--lim > 0 && (c=myfgetc()) != EOF && c != '\n')
                s[i++] = c;
        if (c == '\n') s[i++] = c;
        s[i] = '\0';
        return i;
}


/*--------------------------------------------------------------------------*/


int LaTeXcmp_endlen(const char *p, const char *s)
{
  int len = 0;
  
  while (*s != '\0' && *s != '\t' && *s == *p)
  {
    ++s; ++p; ++len;
  }

  if (*s == '\t' && isLaTeXcmdletter(*p) == 0)
  { 
    while (*p == ' ') { ++p; ++len; }
  }
  else len = 0;

  return len;
}


int loesche_arg(char s[], const char *p, const char *com, const char op, const char cl)
{
   int len, r = 0, reallen;

   while (*p != '\0')
   {
     if (*p == '\\')
     {
       if (*(p+1) == '\\')
       {
          s[r++] = *p++;
          s[r++] = *p++;
       }
       else
       if ((reallen = LaTeXcmp_endlen(p, com)) > 0)
       {
         p += reallen;

         if (*p == '\0') break;

         if (*p == op && (len = nextbracket(p, op, cl)) > 1)
         { 
           p += len;
         }
         else
         {
            if (*p == '\\')
            {
              ++p;
              if (isLaTeXcmdletter(*p) != 0)
              {
                while (isLaTeXcmdletter(*p) != 0) ++p;
                while (*p == ' ') ++p;
              }
              else if (*p != '\0') ++p;
            }
            else
            {
              ++p;   /* ungeklammerten Einzelbuchstaben loeschen */
            }
         }
       }
       else
       {
            s[r++] = *p++;
       }
     }
     else
     {
          s[r++] = *p++;
     }
   }

   s[r] = '\0';

   return r;
}


int entkerne_arg(char s[], const char *p, const char *com, const char op, const char cl)
{
   int l, r = 0, reallen;

   while (*p != '\0')
   {
     if (*p == '\\')
     {
       if (*(p+1) == '\\')
       {
          s[r++] = *p++;
          s[r++] = *p++;
       }
       else
       if ((reallen = LaTeXcmp_endlen(p, com)) > 0)
       {
         p += reallen;
         
         if (*p == '\0') break;

         if (*p == op && (l = nextbracket(p, op, cl)) > 1) 
         {  
            ++p;
            if (*p == ' ')
            { 
              if (r > 1 && s[r-1] == ' ' && s[r-2] == ' ') --r;
              s[r++] = '\\'; 
            }
            while (--l > 1) { s[r++] = *p++; }
            ++p;
         }
         else
         {
            s[r++] = *p++;
         }
       }
       else
       {
            s[r++] = *p++;
       }
     }
     else
     {
          s[r++] = *p++;
     }
   }

   s[r] = '\0';

   return r;
}


void print_not_tightly_err(const char *p, const char c)
{
    int len = strlen(p), sw = 0;

    printf("%%%%\n%%%%>   Warning: \'%c\' does not fit tightly after \"", c);

    if (len > 2 && *(p+(len-1)) == c)
    {
      ++p; len -= 2;
    }

    while (len > 27) { --len; ++p; ++sw; }

    if (sw > 0) printf("...");
                printf("%.*s\".\n", len, p);
}


char *fillsubAptr(const char *p, int len, const char c)
{
   char buf[MAXLEN];
   int i;

                        ++p;   len -= 2;
    while (*p == ' ') { ++p; --len; }
              strcpy(buf, p);

          buf[len] = '\0';
   if    (len > 0 && buf[len-1] == ' ') 
   { i = 2;
     while (len-i >= 0 && buf[len-i] == ' ') ++i;
     while (len-i >= 0 && isLaTeXcmdletter(buf[len-i]) != 0) ++i;
     if (len-i < 0 || buf[len-i] != '\\') print_not_tightly_err(buf, c);
   }
   while (len > 0 && buf[len-1] == ' ') --len;

   return strnmalloc(buf, len+1);
}


int form_only(char buf[], char forml, const char *onlycmdp, const char *p)
{
    int len;

    if (forml == 'l')
    {
       len =  loesche_arg(buf, p, onlycmdp, '{', '}');
    }
    else
    if (forml == 'e')
    {
       len = entkerne_arg(buf, p, onlycmdp, '{', '}');
    }
    else
    {
                  strcpy(buf, p);
       len =      strlen(buf);
      if (forml != 'X') intern_err(3);   /* unexpected forml in khvo */
    }

   return len;
}


int innerentry(const char *p, const char *icmdlist[][2], const int num)
{
  int l = 0;

  while (l < num)
  {
    if (LaTeXcmp_endlen(p, icmdlist[l][0]) > 0) break;
    ++l;
  }

  if (l >= num) l = -1;
  return l;
}

#define DELSTARARG '='
#define DELNORMARG '_'

int endofbracks(const char *p, const char fmt[])
{
  int l = 0, len = 0, pos;

  while (1)
  {
    if (fmt[l] == '\0' || fmt[l+1] == '\0' || fmt[l+2] == '\0') break;

    while (*p == ' ') { ++p; ++len; }
    if (fmt[l] == '*'        && *p != '*') { l += 3; continue; }
    if (fmt[l] == '*'        && *p == '*') { ++p; ++len; }
    if (fmt[l] == DELSTARARG && *p != '*') { l += 3; continue; }
    if (fmt[l] == DELSTARARG && *p == '*') { ++p; ++len; }
    while (*p == ' ') { ++p; ++len; }

    if ((pos=nextbracket(p, fmt[l+1], fmt[l+2])) > 1)
    {
      p   += pos;
      len += pos;
    }

    l += 3;
  }

  return len;
}


void maxprint(int len, const char *s)
{
   char buf[MAXLEN];
   int buflen = entkerne_IeC(buf, s);

   if (len > 4 && buflen > len) printf("%.*s....", len-4, buf);
   else                         printf("%s", buf);
}


void print_msg_line(int line)
{
  if (line < 0)
  {
    intern_err(39);
  }
  else
  { 
    printf(" in *inner v-cmd*:\n%%%%    ");
    maxprint(65, lineptr[line]);
    printf("\n");
  }
}


  const char *innerVcmd[][2] = {
  {  "\\printonlyvli\t",    "+[]_{}:{}*{}!{}"  },
  {  "\\printonlyvqu\t",    "+[]_{}:{}*{}!{}"  },
  { "\\xprintonlyvli\t",    "+[]_{}:{}*{}!{}"  },
  { "\\xprintonlyvqu\t",    "+[]_{}:{}*{}!{}"  },
  {           "\\vli\t",    "+[]_{}:{}*{}!{}"  },  /* wegen der  */
  {           "\\vqu\t",    "+[]_{}:{}*{}!{}"  },  /* Fehler-    */
  {          "\\xvli\t",    "+[]_{}:{}*{}!{}"  },  /* behandlung */
  {          "\\xvqu\t",    "+[]_{}:{}*{}!{}"  },  /* von KTIT   */
  {       "\\shadowv\t",    "_[]_{}_{}={}_{}"  },
  {      "\\addtovli\t",    "_[]_{}_{}={}_{}"  },
  {      "\\addtovqu\t",    "_[]_{}_{}={}_{}"  },
  {     "\\xaddtovli\t",    "_[]_{}_{}={}_{}"  },
  {     "\\xaddtovqu\t",    "_[]_{}_{}={}_{}"  },
  { NULL, NULL }};
  const char *exkv[] = {
     "\\printonlykli",
     "\\printonlykqu",
    "\\xprintonlykli",
    "\\xprintonlykqu",
     "\\printonlykli",
     "\\printonlykqu",
    "\\xprintonlykli",
    "\\xprintonlykqu",
      "\\@shadowba{}",
      "\\@shadowba{}",
      "\\@shadowba{}",
      "\\@shadowba{}",
      "\\@shadowba{}",
    NULL };
#define XVNUM  8
#define AVNUM 13


const char *starlist[][2] =
{
  { "\\vauthor\t",    ":{}:{}" },
  { "\\midvauthor\t", ":{}:{}" },
  { NULL, NULL }};
const char *exlist[] =
{
  "\\kauthor",
  "\\midkauthor",
  NULL
};
#define VKNUM 2


#define INNERLASTARGNO 5
#define INNERSTARARGNO 4
#define INNERLNARGNO   3
#define INNERFNARGNO   2
#define INNEROPARGNO   1

int get_ktit(char oldktit[], const char *p, const char *com, const char op, const char cl, const int line, int found, const int argno)
{
   int l, reallen, r = 0, ll, llen, b = 0, oldfound = found, invauthorfound = 0;
   char buf[MAXLEN], newktit[MAXLEN];


   while (*p != '\0')
   {
     if (*p == '\\')
     {
       if (*(p+1) == '\\')
       {
         buf[b++] = *p++; buf[b++] = *p++;
       }
       else
       if ((ll=innerentry(p, innerVcmd, AVNUM)) > -1)
       {
         /* inner-inner-v-cmd ueberspringen: dort darf weiteres \\ktit stehen */
         llen = LaTeXcmp_endlen(p, innerVcmd[ll][0]);
         p += llen;
         llen =     endofbracks(p, innerVcmd[ll][1]);
         p += llen;
         buf[b++] = '<'; buf[b++] = '#'; buf[b++] = '>';
       }
       else
       if ((reallen = LaTeXcmp_endlen(p, com)) > 0)
       {
         while (reallen-- > 0) buf[b++] = *p++;
         ++found;
         
         if (*p == '\0') break;

         if ((l = nextbracket(p, op, cl)) > 0) 
         {  
            r = 0;   /*  <= Copy only arg of last \\ktit  */
            if (l == 1)
            {
              newktit[r++] = op;
              newktit[r++] = *p; buf[b++] = *p++;
              newktit[r++] = cl;
            }
            else
            if (l == 2 && *p == '\\' && *(p+1) != '\0')
            {
              if (isLaTeXcmdletter(*(p+1)) == 0);
              else
              while (isLaTeXcmdletter(*(p+l)) != 0) ++l;
              newktit[r++] = op;
              while (l-- > 0) { newktit[r++] = *p; buf[b++] = *p++; }
              newktit[r++] = cl;
            }
            else 
            {
              while (l-- > 0) 
              { 
                newktit[r++] = *p; buf[b++] = *p++;
              }
            }
         }
       }
       else buf[b++] = *p++;
     }
     else   buf[b++] = *p++;
   }

   newktit[r] = '\0';
   buf[b] = '\0';

   if (strcmp(newktit, KTITINARG) == 0)
   {
     printf("%%%%\n%%%%>  Error: %s follows %s", KTIT, KTIT);
     print_msg_line(line-1);
              inerr[line-1] += KTIT_DBL_ERR;  /* new in 2.5 */
   }

   if   (r > 2 && newktit[r-1] == cl && newktit[r-2] == ' ')
   {
     l = 3;
     while (r-l > 1 && newktit[r-l] == ' ') ++l;
     while (r-l > 1 && isLaTeXcmdletter(newktit[r-l]) != 0) ++l;
     if (newktit[r-l] != '\\') print_not_tightly_err(newktit, cl);
   }


   if (newktit[0] == op)
   {    
      l = 1; while (newktit[l] == ' ') ++l;
      if (newktit[l] == cl) newktit[0] = '\0'; /* new in 2.5 */
   }


   if (found > oldfound)
   {
     strcpy(oldktit, newktit);
   }


   if (argno == INNERLASTARGNO && oldktit[0] == '\0')
   {
     if (   strlen(KTITBEG)
          + strlen(KTITMISS)
          + strlen(KTITEMPTY)
          + strlen(KTITEND)
          > MAXLEN-2
        )
     {
       intern_err(29);
     }
     else
     {
         strcpy(oldktit, KTITBEG);
       printf("%%%%\n%%%%>  ");


       if (found == 0)
       {
         printf("Error: Missing");
         strcat(oldktit, KTITMISS);
                  inerr[line-1] += KTIT_MIS_ERR;  /* new in 2.5 */
       }
       else
       { 
         printf("Warning: Empty");
         strcat(oldktit, KTITEMPTY);
                  inerr[line-1] += KTIT_EMP_ERR;  /* new in 2.5 */
       }

         printf(" %s", KTIT);
         print_msg_line(line-1);
         strcat(oldktit, KTITEND);
     }
   }


   if (argno == INNERLASTARGNO && found > 1) 
   {
     printf("%%%%\n%%%%>  Warning: %s used %d times", KTIT, found);
     print_msg_line(line-1);
              inerr[line-1] += KTIT_SEV_ERR;  /* new in 2.5 */
   }


   return found;
}


void do_innerVstarArg(char ret[], const char *p)
{
  int r = 0, l, len, end;


          strcpy(ret, p);

  while (*p != '\0')
  {
    if (*p == '\\')
    {
      if (*(p+1) == '\\')
      { 
        p += 2; r += 2; 
      }
      else
      if ((l=innerentry(p, starlist, VKNUM)) > -1)
      { 
        len = LaTeXcmp_endlen(p, starlist[l][0]);
        p += len;
        end = nextbracket(p, *(starlist[l][1]+1), *(starlist[l][1]+2));
            if (end == 2 && *p == '\\' && *(p+1) != '\0')
            {
              if (isLaTeXcmdletter(*(p+1)) == 0);
              else
              while (isLaTeXcmdletter(*(p+end)) != 0) ++end;
            }
        p += end;
                  ret[r] = '\0';
           strcat(ret, exlist[l]);
                           end = llstrlen(exlist[l]);
                      r += end;

            while (*p == ' ') ++p;
                end = nextbracket(p, *(starlist[l][1]+4), *(starlist[l][1]+5));
            if (end == 1)
            {
              ret[r++] = *(starlist[l][1]+4);
              ret[r++] = *p++;
              ret[r++] = *(starlist[l][1]+5);
            }
            else
            if (end == 2 && *p == '\\' && *(p+1) != '\0')
            {
              if (isLaTeXcmdletter(*(p+1)) == 0);
              else
              while (isLaTeXcmdletter(*(p+end)) != 0) ++end;
              ret[r++] = *(starlist[l][1]+4);
              while (end-- > 0) { ret[r++] = *p++; }
              ret[r++] = *(starlist[l][1]+5);
              while (*p == ' ') ++p;
            }
            else
            {
              while (end-- > 0) 
              { 
                ret[r++] = *p++; 
              }
            }
                  ret[r] = '\0';                  
           strcat(ret, p);
      }
      else
      {
        ++p; ++r;
      }
    }
    else
    {
      ++p; ++r;
    }
  }
}


#define OPTARG_FMT_UNDEF  '\0'

int dobracks(char buf[], const char *p, const char fmt[], const int line)
{
  int l = 0, len = 0, pos, i, found = 0, oldfound, optfound = 0;
  char ibuf[MAXLEN], obuf[MAXLEN], kbuf[MAXLEN], optarg_fmt = OPTARG_FMT_UNDEF;

  buf[0] = '\0'; kbuf[0] = '\0';

  while (1)
  {
    if (fmt[l] == '\0' || fmt[l+1] == '\0' || fmt[l+2] == '\0') break;

    while (*p == ' ') { ++p; ++len; }
    if (fmt[l] == '*'        && *p != '*') { l += 3; continue; }
    if (fmt[l] == '*'        && *p == '*') { ++p; ++len; }
    if (fmt[l] == DELSTARARG && *p != '*') { l += 3; continue; }
    if (fmt[l] == DELSTARARG && *p == '*') { ++p; ++len; }
    while (*p == ' ') { ++p; ++len; }

    if ((pos=nextbracket(p, fmt[l+1], fmt[l+2])) > 1)
    {
      if (fmt[l] == '+')
      {
        /* [OptArg] */
        oldfound = found;
                     mystrncpy(ibuf, p, pos);
        found = get_ktit(kbuf, ibuf, KTITCMD, KTITOP, KTITCL, line, found, INNEROPARGNO);
        optfound = found;

        if (    *(p+1) == 'p' 
             || *(p+1) == 'm'
             || *(p+1) == 'f'
             || *(p+1) == ']'
           )
        {
          i = 2; while (*(p+i) == ' ') ++i;
          if (*(p+1) != ']' && oldfound == found && *(p+i) == '{')
          { 
            optarg_fmt = *(p+1);
          }
          if (*(p+i) != '{' && *(p+i) != ']' && *(p+i) != '\0')
          {
            printf("%%%%\n%%%%>  Error: ");
            maxprint(14, ibuf);
            printf("  without brackets like [%c{...}]", *(p+1));
            print_msg_line(line-1);
                     inerr[line-1] += OARG_BRA_ERR;  /* new in 2.5 */
          }
        }
        else 
        {
          printf("%%%%\n%%%%>  Error: ");
          maxprint(14, ibuf);
          printf("  begins with \'%c\' instead f/m/p", *(p+1));
          print_msg_line(line-1);
                   inerr[line-1] += OARG_IRR_ERR;  /* new in 2.5 */
        }
        found -= optfound;
      }
      else
      if (fmt[l] == '!')
      {
        /* {Rest} superior to fmt[l] == '*' */
                     mystrncpy(ibuf, p, pos);
        found = get_ktit(kbuf, ibuf, KTITCMD, KTITOP, KTITCL, line, found, INNERLASTARGNO);
             strcat(buf, kbuf);
      }
      else
      if (fmt[l] == '*')
      {
        oldfound = found;
                     mystrncpy(ibuf, p, pos);
        found = get_ktit(kbuf, ibuf, KTITCMD, KTITOP, KTITCL, line, found, INNERSTARARGNO);
        do_innerVstarArg(obuf, ibuf);
             strcat(buf,  "*");           /*  keep fmt[l]  */
             strcat(buf, obuf);
        if (optfound == 0 && oldfound != found && optarg_fmt != OPTARG_FMT_UNDEF)
        {
          printf("%%%%\n%%%%>  Error: [%c{..}] without %s overwrites %c{..%s..}", optarg_fmt, KTIT, fmt[l], KTIT);
          print_msg_line(line-1);
                   inerr[line-1] += KTIT_OPE_ERR;  /* new in 2.5 */
        }
      }
      else
      if (fmt[l] == DELNORMARG || fmt[l] == DELSTARARG)
      {
                     mystrncpy(ibuf, p, pos);
        found = get_ktit(kbuf, ibuf, KTITCMD, KTITOP, KTITCL, line, found, INNERLNARGNO);
      }
      else
      {
        /* FN */
                     mystrncpy(ibuf, p, pos);
        found = get_ktit(kbuf, ibuf, KTITCMD, KTITOP, KTITCL, line, found, INNERFNARGNO);
                   strcat(buf, ibuf); 
      }
      
      p   += pos;
      len += pos;
    }

    l += 3;
  }

  return len;
}


int do_innercmd(char ret[], const char *p, const char *icmdlist[][2], const int num, const char *form)
{
  char buf[MAXLEN], cbuf[MAXLEN];
  int r = 0, l, len, end;

          strcpy(ret, p);

  while (*p != '\0')
  {
    if (*p == '\\')
    {
      if (*(p+1) == '\\')
      { 
        p += 2; r += 2; 
      }
      else
      if ((l=innerentry(p, icmdlist, num)) > -1)
      { 
        len = LaTeXcmp_endlen(p, icmdlist[l][0]);
        p += len; r += len;
        end = endofbracks(p, icmdlist[l][1]);
           mystrncpy(buf, p, end);
        p += end;
        end = do_only(cbuf, form, buf);
                  ret[r] = '\0';
          strncat(ret, cbuf, end);
           strcat(ret, p);
        r += end;
      }
      else
      {
        ++p; ++r;
      }
    }
    else
    {
      ++p; ++r;
    }
  }

  return r;
}


int do_innerVcmd(char ret[], const char *p, const char *icmdlist[][2], const int num, const int line)
{
  char buf[MAXLEN], cbuf[MAXLEN];
  const char *err = p;
  int l, len, end, pos = 0;

          strcpy(ret, p);

  while (*p != '\0')
  {
    if (*p == '\\')
    {
      if (*(p+1) == '\\')
      { 
        p += 2; pos += 2;
      }
      else
      if ((l=innerentry(p, icmdlist, num)) > -1)
      { 
        len = LaTeXcmp_endlen(p, icmdlist[l][0]);
        p += len; 
                  ret[pos] = '\0';
        if (strlen(ret) + strlen(exkv[l]) < MAXLEN)
           strcat(ret, exkv[l]);      /* Exchange v-cmd => k-cmd; */
        else { intern_err(4); break; }
         pos += strlen(exkv[l]);
        end = dobracks(buf, p, icmdlist[l][1], line);
        p += end;
        if (strlen(ret) + strlen(buf) < MAXLEN)           
           strcat(ret, buf);
        else { intern_err(5); break; }
         pos += strlen(buf);
        if (strlen(ret) + strlen(p) < MAXLEN)           
           strcat(ret,   p);
        else { intern_err(6); break; }
      }
      else
      {
        ++p; ++pos;
      }
    }
    else
    {
      ++p; ++pos;
    }
  }

  return pos;
}


int do_only(char ret[], const char form[], const char in[])
{
  int l = 0, len = 0;
  char buf[MAXLEN];
  const char *onlycmd[] = { "\\onlykurz\t",
                            "\\onlyhere\t",
                            "\\onlyvoll\t",
                            "\\onlyout\t",
                            KTITCMD,
                            NULL
                          };
#define KHVONUM 5  /* KTITCMD added in 2.5 */


  if (form[0] == '\0' || strlen(form) != KHVONUM)
  { 
    intern_err(7);  /* len of khvo */
          strcpy(ret, in);
    len = strlen(ret);
  }
  else
  {
        len = form_only(buf, form[l], onlycmd[l],  in);
         ++l;
      if  (l == KHVONUM) strcpy(ret, buf);

    while (l  < KHVONUM)
    {
        len = form_only(ret, form[l], onlycmd[l], buf);
         ++l;
       if (l == KHVONUM) break;

        len = form_only(buf, form[l], onlycmd[l], ret);
         ++l;
       if (l == KHVONUM) strcpy(ret, buf);
    }
  }

  return len;
}


/*--------------------------------------------------------------------------*/


char *standardnum(char a[], char *p)
{
   int j, beforenum = 0;
   char *top = p;

   j = 0; while (j < NUMLEN-1) a[j++] = '0'; a[j] = '\0';

   while (beforenum < NUMLEN-1 && isdigit(*p) != 0)
   {
     j = 1; while (j < NUMLEN-1) { a[j-1] = a[j]; ++j; }
     a[NUMLEN-2] = *p;
     ++beforenum;
     ++p;

     if (german == 0)
     {
       if ((*p == ',' || *p == ' ') && isdigit(*(p+1)) != 0 && isdigit(*(p+2)) != 0 && isdigit(*(p+3)) != 0 && isdigit(*(p+4)) == 0) ++p;
     }
     else
     {
       if ((*p == '.' || *p == ' ') && isdigit(*(p+1)) != 0 && isdigit(*(p+2)) != 0 && isdigit(*(p+3)) != 0 && isdigit(*(p+4)) == 0) ++p;
     }
   }

   if (german == 0)
   {
     if (*p == '.' && isdigit(*(p+1)) != 0)
     {
       j = NUMLEN-1; while (j < 2*NUMLEN-1 && isdigit(*(p+1)) != 0) { ++p; a[j++] = *p; }
       ++p; a[j] = '\0';
     }
   }
   else
   {
     if (*p == ',' && isdigit(*(p+1)) != 0)
     {
       j = NUMLEN-1; while (j < 2*NUMLEN-1 && isdigit(*(p+1)) != 0) { ++p; a[j++] = *p; }
       ++p; a[j] = '\0';
     }
   }

   if (isdigit(*p) != 0)
   {
     printf("%%%%\n%%%%>   Warning: Can\'t sort too long number \"%.*s...\".\n", 30, top);
     while (isdigit(*p) != 0) ++p;         
   }

   return p;
}


int isLaTeXcmdletter(char a)
{
  if (isalpha(a) != 0 || a == '@')
  return 1;
  return 0;
}


char *setptrtofirstbracket(char *p)
{
    while (*p != '\0')
    {
       if (*p == '\\' && *(p+1) != '\0') ++p;
       ++p;
       if (*p == '{') break;
    }
    return p;
}


void ignoreadditionaltext(const char *p, const char *i, const char *f)
{
   int ll;

   while(*p == ' ') ++p;
   if (*p != '\0' && *p != '%')
   {  
      if (strlen(i) > 58) 
           ll = 0;
      else ll = 58 - strlen(i);
      printf("%%%%\n%%%%>  Warning: Ignore additional text \"%.*s\"\n", 40, p);
      printf("%%%%    after \"%s\" -> \"%.*s\".\n", i, ll, f);
      if (*p == '\\')
      printf("%%%%    ** I grap only one \\cmd per line!  Fix that, and start me again! **\n");
   }
}


void telldefault(int rest)
{
   printf("%%%%    ** Missing %d argument(s).  The \'%%\' remains in my output.\n", rest);
}


  const char *inner_K_cmd[][2] = {
  {  "\\printonlykli\t",    ":[]:{}*{}:{}"     },
  {  "\\printonlykqu\t",    ":[]:{}*{}:{}"     },
  { "\\xprintonlykli\t",    ":[]:{}*{}:{}"     },
  { "\\xprintonlykqu\t",    ":[]:{}*{}:{}"     },
  {  "\\printonlyper\t",    ":{}"              },
  {  "\\printonlyarq\t",    ":[]:{}:{}"        },
  { NULL, NULL }};
#define INNERTITLES 4  /* 6 changed to 4 in 2.5 */

  const char *inner_PER_ARQ_ABK_cmd[][2] = {
  {  "\\printonlyper\t",    ":{}"              },
  {  "\\printonlyarq\t",    ":[]:{}:{}"        },
  {  "\\printonlydefabk\t", ":{}:{}"           },
  {  "\\printonlyabkdef\t", ":{}:{}"           },
  {  "\\printonlyabk\t",    ":{}"              },
  { NULL, NULL }};
#define INNERPAA 5  /* new in 2.5 */


int do_inneronly(char ret[], const char in[], const char *vf, const char *kf, const char *paaf)
{
  char buf[MAXLEN];
  int len;

  len = do_innercmd(buf,  in, innerVcmd,                  XVNUM,   vf);  /* AVNUM unnecessary */
  len = do_innercmd(ret, buf, inner_K_cmd,          INNERTITLES,   kf);  /* 2.5: XKPANUM changed to INNERTITLES */
  len = do_innercmd(buf, ret, inner_PER_ARQ_ABK_cmd,   INNERPAA, paaf);  /* new in 2.5 */
             strcpy(ret, buf);                                          /* new in 2.5 */

  return len;
}


char *makeAR(char *p, int cols, int rest, char bb, char eb, const char *ident, int linec, int i, char schalt, const char *formO, int ventry)
{
    int len, alen;
    char ibuf[MAXLEN], cbuf[MAXLEN];

    while (*p == ' ') ++p;

    if  (*p != bb)
    {     printf("%%%%\n%%%%>  %s error in arg %d: Missing \'%c\'.\n", ident, i+cols, bb);
          telldefault(rest-i);
          return NULL;
    }
    else
    {
       if ((len = nextbracket(p, bb, eb)) < 0)
       {  printf("%%%%\n%%%%>  %s error in arg %d: Missing \'%c\' after \"%.*s\".\n", ident, i+cols, eb, -len, p);
          telldefault(rest-i);
          return NULL;
       }
       
       if (schalt == 'A')
       {
         /* new in 2.2: Evaluate inner v-cmds in rest of v-cmds as k-cmds! */
         if (ventry != IS_V && i == VCOLS-1)
         {
            /*  alloc {li} or {qu} as 4th arg of k-cmds:  */
             alen =                  mystrncpy(cbuf, p, len);
         }
         else
         {
                                     mystrncpy(cbuf, p, len);

                                   /*  new in 2.5 for k-cmds: "elleX": */
                        /* changed in 2.5: "eell" to "eeXXX"           */

           if (ventry == IS_V)  /* MARKX */
           {
             alen =         do_inneronly(ibuf, cbuf, "eeXXX", "elleX", "leelX");
           }
           else  /*  new in 2.5: ventry == NO_V || ventry == IS_Q  */
           {
             alen =         do_inneronly(ibuf, cbuf, "leelX", "elleX", "leelX");
           }
                                /*   moved in 2.5: do_only */

           if (ventry == IS_V && i == VCOLS-1)
           {
             alen =   do_innerVcmd(cbuf, ibuf, innerVcmd, AVNUM, linec+1);
           }
           else
           {
                            strcpy(cbuf, ibuf);
           }

                /* get_add_ktit del in 2.5 */

           if (ventry == IS_V)  /* MARKX */
           {
                /*  inner v-cmds are k-cmds here / new in 2.5: */
                /*  e == \ktit{x} => x */
             alen =          do_innercmd(ibuf, cbuf, inner_K_cmd, INNERTITLES, "XXlle");
           }
           else
           {
                                  strcpy(ibuf, cbuf);
           }

             alen = do_only(cbuf, formO, ibuf);
         }

         subAptr[i][linec] = fillsubAptr(cbuf, alen, eb);
       }
       else
       if (schalt == 'R') subRptr[i][linec] = strnmalloc(p+1, len-1);
       else               intern_err(8);  /* INTERNAL ERROR (A, R) */

       p += len;
    }           

    return p;
}


void dodqdeepsort(int linec, int alllines, int deepsort)
{
  int num;

  while(linec < alllines)
  { 
    if (    nogsty == 0
         && userusesdq(deepsort, linec) == 1 
         && (num = getarabicnum(subRptr[DQCOL][linec])) >= MINCATCODE
         &&  num                                        <= MAXCATCODE
       ) dqsort[linec] = num;
    else dqsort[linec] = ERRCATCODE;
              ++linec;
  }
}


void dopgfntnumberweight(int linec, int alllines)
{
  while(linec < alllines)
  { 
    pagewgt[linec] =  pgnumberweight(subRptr[PAGECOL][linec]);
     fntwgt[linec] = fntnumberweight(subRptr[FNTCOL][linec]);
    ++linec;
  }
}


int over_fonts(char *p, int start)
{
   int i, l, neu = 0;

   const char *fonts[] =
   {
     NEWFNSYMBOL,     /* start == 0 : footnote = $^{x}$ */
     "\\rmfamily",    /* start == 1 : else = text mode  */
     "\\sffamily",
     "\\ttfamily",
     "\\mdseries",
     "\\bfseries",
     "\\upshape",
     "\\itshape",
     "\\slshape",
     "\\scshape",
     "\\em",
     "\\normalfont",
     "\\textrm",
     "\\rm",
     "\\textsf",
     "\\sf",
     "\\texttt",
     "\\tt",
     "\\textmd",
     "\\textbf",
     "\\bf",
     "\\textup",
     "\\textit",
     "\\it",
     "\\textsl",
     "\\sl",
     "\\textsc",
     "\\sc",
     "\\emph",
     "\\textnormal",
     "\\bapageframe",
     "\\pbapageframe",
     "\\bafootnoteframe",
     "\\pbafootnoteframe",
     RELAX,
     NULL,
   };
#define FONTC 35


   while(*p == '{' && *(p+1) == '}') 
   { 
     p += 2; neu += 2; 
     while (*p == ' ') { ++p; ++neu; }
   }


   while(*p == '\\')
   {
     i = start;
     l = 0;
     while (i < FONTC && lcmdcmp(p, fonts[i]) != 0) ++i;
     if(   (i < FONTC && lcmdcmp(p, fonts[i]) == 0 && (l=strlen(fonts[i])) > 0)
        || (start == 0 && (l=len_tom(p)) > 0)
       )
     {
         p+=l; 
       neu+=l;
       while (*p == ' ') { ++p; ++neu; }
     }
     else if (l == 0) break;
   }

   return neu;
}



void set_inner_number_on_subRptr(char *p, int len, int col, int line, int start)
{
   int neu;

          /* start==0: in $^{x}$ kein \ensuremath oder \TextOrMath noetig! */
   neu  = over_fonts(p, start);
   len -= neu;
     p += neu;

   while (*p == '{' && len > 2 && nextbracket(p, '{', '}') == len-1)
   {
     ++p; len -= 2;

     while (*p == ' ') { ++p; --len; }

     neu  = over_fonts(p, start);
     len -= neu;
       p += neu;
   }

   subRptr[col][line] = strnmalloc(p, len);
}


void getbrackets(int linec, int alllines, const char *ident, int deep, int filenum, const char *formO, int ventry)
{
   char *p, buf[MAXLEN];
   int len, i;

   while(linec < alllines)
   {
      i = 0; while (i < MAXBACOLS)  subAptr[i++][linec] = mydefault;
      i = 0; while (i < BAALLRCOLS) subRptr[i++][linec] = mydefault;
                                           leval[linec] = ACCEPTLINE;
                                           inerr[linec] = NOINERR;  /* new in 2.5 */

      p = lineptr[linec];
      p = setptrtofirstbracket(p);

      i = 0;        
      while (i < deep)
      {
        if ((p = makeAR(p, 1, deep+BARESTCOLS, '{', '}', ident, linec, i, 'A', formO, ventry)) == NULL) 
        { 
          leval[linec] = REJECTLINE;
          break;
        }
        ++i;
      }

      if (i == deep)
      {
        i = 0;
        while (i < BARESTCOLS)
        {
          if ((p = makeAR(p, BARESTCOLS, BARESTCOLS, bareststr[2*i], bareststr[2*i+1], ident, linec, i, 'R', "", ventry)) == NULL) 
          { 
            leval[linec] = REJECTLINE;  
            break;
          }
          ++i;
        }
        if (p != NULL) ignoreadditionaltext(p, ident, subAptr[0][linec]);

        p = subRptr[PAGEFNTCOL][linec];
                  while (*p == ' ') ++p;
        if (*p == '{' && (len = nextbracket(p, '{', '}')) > 1)
        { 
          while (*(p+1) == ' ') { ++p; --len; }

          set_inner_number_on_subRptr(p+1, len-1, PAGECOL, linec, 1);

          p += len;
                    while (*p == ' ') ++p;
          if (*p == '{' && (len = nextbracket(p, '{', '}')) > 1)
          { 
            while (*(p+1) == ' ') { ++p; --len; }

            set_inner_number_on_subRptr(p+1, len-1, FNTCOL, linec, 0);

            p += len;
                      while (*p == ' ') ++p;
            if (*p == '{' && (len = nextbracket(p, '{', '}')) > 1)
            {
              while (*(p+1) == ' ') { ++p; --len; }
              if (*(p+1) == HASUNUCH)
              {
                myfree(subRptr[PAGECOL][linec]);
                       subRptr[PAGECOL][linec] = unused;   /* UNUSED */
              }
              if (*(p+1) != HASFNTCH)
              {
                if (*subRptr[FNTCOL][linec] != '\0')
                   printf("%%%%\n%%%%>  Warning: Ignore unexpected FNT-num \"%s\".\n", subRptr[FNTCOL][linec]);
                myfree(subRptr[FNTCOL][linec]);
                       subRptr[FNTCOL][linec] = nofnt;   /* NOFOOTNOTE */
              }
              if (*(p+1) != '}' && *(p+1) != HASFNTCH && *(p+1) != HASUNUCH)
              { printf("%%%%\n%%%%>  %s warning (part 3 of arg %d):\n", ident, deep+2);
                printf("%%%%    Get no {} or {%c} or {%c}!\n", HASFNTCH, HASUNUCH);
              }
              p += len;
                        while (*p == ' ') ++p;
              if (*p != '\0')
              { printf("%%%%\n%%%%>  %s warning (parts of arg %d):\n", ident, deep+2);
                printf("%%%%    Unexpected text after {page}{footnote}{} number!\n");
              }
            }
            else 
            {   printf("%%%%\n%%%%>  %s warning (part 3 of arg %d):\n", ident, deep+2);
                printf("%%%%    Get no {} or {%c}!\n", HASFNTCH);
            }
          }
          else 
          {   printf("%%%%\n%%%%>  %s warning (part 2 of arg %d):\n", ident, deep+2);
              printf("%%%%    Get no {footnote} number!\n");
          }
        }
        else
        {     printf("%%%%\n%%%%>  %s warning (all 3 parts of arg %d):\n", ident, deep+2);
              printf("%%%%    Get no {page}{footnote}{} number!\n");
        }
      }

      if (filenum > -1)
      {
        sprintf(buf, "file %d ", filenum);

        if (firstoverlettergan(subRptr[LINECOL][linec]) > 0)
        {
          strncat(buf, subRptr[LINECOL][linec], MAXLEN-strlen(buf)-strlen(subRptr[LINECOL][linec])-1);
        }
        else 
        {
          printf("%%%%\n%%%%>  %s error in last arg: No line-number!\n", ident);
          strncat(buf,          ERRINPUTLINENO, MAXLEN-strlen(buf)-ERRINPUTLINENOLEN-1);
        }

          myfree(subRptr[LINECOL][linec]);
                 subRptr[LINECOL][linec] = strnmalloc(buf, strlen(buf)+1);
      }

      ++linec;
   }
}


  /*  Accents  \b  \c  \k  \d  \H  \t  have no equivalent in MathMode.  */
  /*  2.3: control-characters are now doublettes (CTL+G or [Esc] added).  */
  /*  2.5: \t  added behind  \\.  \\\'  \\`  \\^  \\=  \\~  ( \.X  =  \. X ).
           I sort   \a.  \a'  \a`  \a^  \a"X  \a=  \a~   and
              \a{r}  \a{b}  \a{c}  \a{k}  \a{d}  \a{H}  \a{t}  \a{u}  \a{v}
           as                   X   ...  ;
           other  \protect\a X  will EXECUTE/PRINT  \X , BUT will be
           SORTED as  X  (e.g.  \protect\a{textdollar}  as  textdollar).
   */
const char *weighttable[][2] = {
  {  "\\.\t",                 ""        }, /* CTL+A + Ctl+G */  /* MARK_POINT */
  {  "\\@tabacckludge\t.\t",  ""        }, /* new in 2.5 */
  {  "\\a\t.\t",              ""        }, /* new in 2.5 */
  {  "\\dot\t",               ""        },
  {  "\\capitaldotaccent\t",  ""        },  /* new in 2.5 = \. */
  {  "\\\'\t",                ""        }, /* CTL+B */
  {  "\\@tabacckludge\t\'\t", ""        },  /* new in 2.3, changed in 2.5 */
  {  "\\a\t\'\t",             ""        },  /* new in 2.3, changed in 2.5 */
  {  "\\acute\t",             ""        },
  {  "\\capitalacute\t",      ""        },  /* new in 2.5 = \@tabacckludge' */
  {  "\\`\t",                 ""        }, /* CTL+C */
  {  "\\@tabacckludge\t`\t",  ""        },  /* new in 2.3, changed in 2.5 */
  {  "\\a\t`\t",              ""        },  /* new in 2.3, changed in 2.5 */
  {  "\\grave\t",             ""        },
  {  "\\capitalgrave\t",      ""        },  /* new in 2.5 = \@tabacckludge` */
  {  "\\^\t",                 ""        }, /* CTL+D */
  {  "\\@tabacckludge\t^\t",  ""        },  /* new in 2.5 */
  {  "\\a\t^\t",              ""        },  /* new in 2.5 */
  {  "\\hat\t",               ""        },
  {  "\\capitalcircumflex\t", ""        },  /* new in 2.5 = \^ */
  {  "\\ddot\t",              ""        }, /* CTL+E  math-dieresis != umlaut */
  {  "\\=\t",                 ""        }, /* CTL+F */
  {  "\\@tabacckludge\t=\t",  ""        },  /* new in 2.3, changed in 2.5 */
  {  "\\a\t=\t",              ""        },  /* new in 2.3, chamged in 2.5 */
  {  "\\bar\t",               ""        },
  {  "\\capitalmacron\t",     ""        },  /* new in 2.5 = \@tabacckludge= */
  {  "\\vec\t",               ""        }, /* CTL+F + [ESC]   changed in 2.3 */
  {  "\\~\t",                 ""        }, /* CTL+K */     /* MARK_TILDE */
  {  "\\@tabacckludge\t~\t",  ""        },  /* new in 2.5 */
  {  "\\a\t~\t",              ""        },  /* new in 2.5 */
  {  "\\tilde\t",             ""        },
  {  "\\capitaltilde\t",      ""        },  /* new in 2.5 = \~ */
  {  "\\aa\t",                "a"       }, /* CTL+L */     /* \\r a \\r A */
  {  "\\AA\t",                "A"       },
  {  "\\r\t",                 ""        },                 /* \\AA \\aa */
  {  "\\@tabacckludge\tr\t",  ""        },  /* new in 2.5 */
  {  "\\a\tr\t",              ""        },  /* new in 2.5 */
  {  "\\mathring\t",          ""        },
  {  "\\capitalring\t",       ""        },  /* new in 2.5 = \r */
  {  "\\b\t",                 ""        }, /* CTL+N */
  {  "\\@tabacckludge\tb\t",  ""        },  /* new in 2.5 */
  {  "\\a\tb\t",              ""        },  /* new in 2.5 */
  {  "\\c\t",                 ""        }, /* CTL+O + CTL+G */
  {  "\\@tabacckludge\tc\t",  ""        },  /* new in 2.5 */
  {  "\\a\tc\t",              ""        },  /* new in 2.5 */
  {  "\\capitalcedilla\t",    ""        },  /* new in 2.5 = \c */
  {  "\\textcommabelow\t",    ""        }, /* CTL+O + [ESC]    new in 2.3 */
  {  "\\textogonekcentered\t",""        },  /* new in 2.5 = \k */
  {  "\\capitalogonek\t",     ""        },  /* new in 2.5 = \k */
  {  "\\k\t",                 ""        }, /* CTL+P */      /* rechte Tilde */
  {  "\\@tabacckludge\tk\t",  ""        },  /* new in 2.5 */
  {  "\\a\tk\t",              ""        },  /* new in 2.5 */
  {  "\\d\t",                 ""        }, /* CTL+Q */
  {  "\\@tabacckludge\td\t",  ""        },  /* new in 2.5 */
  {  "\\a\td\t",              ""        },  /* new in 2.5 */
  {  "\\H\t",                 ""        }, /* CTL+R */
  {  "\\@tabacckludge\tH\t",  ""        },  /* new in 2.5 */
  {  "\\a\tH\t",              ""        },  /* new in 2.5 */
  { "\\capitalhungarumlaut\t",""        },  /* new in 2.5 = \H */
  {  "\\newtie\t",            ""        },  /* new in 2.5 = \t */
  {  "\\capitalnewtie\t",     ""        },  /* new in 2.5 = \t */
  {  "\\t\t",                 ""        }, /* CTL+S */
  {  "\\@tabacckludge\tt\t",  ""        },  /* new in 2.5 */
  {  "\\a\tt\t",              ""        },  /* new in 2.5 */
  {  "\\capitaltie\t",        ""        },  /* new in 2.5 = \t */
  {  "\\u\t",                 ""        }, /* CTL+T */
  {  "\\@tabacckludge\tu\t",  ""        },  /* new in 2.5 */
  {  "\\a\tu\t",              ""        },  /* new in 2.5 */
  {  "\\breve\t",             ""        },
  {  "\\capitalbreve\t",      ""        },  /* new in 2.5 = \u */
  {  "\\v\t",                 ""        },  /* changed in 2.3 */
  {  "\\@tabacckludge\tv\t",  ""        },  /* new in 2.5 */
  {  "\\a\tv\t",              ""        },  /* new in 2.5 */
  {  "\\check\t",             ""        },  /* changed in 2.3 */
  {  "\\capitalcaron\t",      ""        },  /* new in 2.5 = \v */
  {  "\\textblank\t",         "b"       },  /* CTL+U \   new in 2.3;    */
  {  "\\textbaht\t",          "B"       },  /*        |  new in 2.3     */
  {  "\\textcent\t",          "c"       },  /*  S     |  changed in 2.3 */
  {  "\\textcentoldstyle\t",  "c"       },  /*  T     |  changed in 2.3 */
  {  "\\textcolonmonetary\t", "C"       },  /*  R     |  new in 2.3     */
  {  "\\dj\t",                "d"       },  /*  0     |  changed in 2.3 */
  {  "\\dh\t",                "d"       },  /*  K     |  changed in 2.3 (! \textsc{\dh} is like \DH) */
  {  "\\DJ\t",                "D"       },  /*  E     |  changed in 2.3 */
  {  "\\DH\t",                "D"       },  /* =\\DJ  |  changed in 2.3 */
  {  "\\hbar\t",              "h"       },  /* h-quer |  new in 2.3     */
  {  "\\l\t",                 "l"       },  /*        |  changed in 2.3 */
  {  "\\L\t",                 "L"       },  /*  S     |  changed in 2.3 */
  {  "\\textnaira\t",         "N"       },  /*  T     |  new in 2.3     */
  {  "\\o\t",                 "o"       },  /*  R     |  changed in 2.3 */
  {  "\\O\t",                 "O"       },  /*  O     |  changed in 2.3 */
  {  "\\textrecipe\t",        "R"       },  /*  K     |  new in 2.3     */
  {  "\\textpeso\t",          "P"       },  /*  E     |  new in 2.3     */
  {  "\\textwon\t",           "W"       },  /*        |  changed in 2.3 */
  {  "\\textyen\t",           "Y"       },  /*       /   changed in 2.3 */
  {  "\\st\t",                ""        }, /* CTL+V */    /* NEW in 2.2 */
  {  "\\textst\t",            ""        },                /* NEW in 2.2 */
  {  "\\underbar\t",          ""        }, /* CTL+W + CTL+G */
  {  "\\underline\t",         ""        }, /* CTL+W + [ESC]  changed in 2.3 */
  {  "\\ul\t",                ""        },  /*               changed in 2.3 */
  {  "\\textul\t",            ""        },  /*               changed in 2.3; NEW in 2.2 */
  {  "\\,",                     " "       },
  {  "\\space\t",               " "       },  /* if you say \protect\space */
  {  "\\nobreakspace\t",        " "       },  /* new in 2.3 */  /* MARK_TILDE */
  {  "\\textvisiblespace\t",    " "       },  /* new in 2.3 */
  {  "\\teskip\t",              " "       },  /* new in 2.3 */
  {  "\\pteskip\t",             " "       },  /* 1\te 000     NEW in 2.2 */
  {  "\\te\t",                 ". "       },  /*  != 1000     NEW in 2.2 */
  {  "\\textflorin\t",          "f"       },  /* changed in 2.3 */
  {  "\\i\t",                   "i"       },
  {  "\\imath\t",               "i"       },
  {  "\\j\t",                   "j"       },
  {  "\\jmath\t",               "j"       },
  {  "\\ae\t",                "ae"      },  /* \      changed in 2.3 */
  {  "\\AE\t",                "Ae"      },  /*  |     <=> \\Oe  changed in 2.3 */
  {  "\\BibArts\t",           "BibArts" },  /*  |     changed in 2.3 */
  {  "\\pBibArts\t",          "BibArts" },  /*  |     changed in 2.3 */
  {  "\\textdong\t",          "d"       },  /* CTL_X  new in 2.3 (not really \\b{\\dj}) */
  {  "\\textestimated\t",     "e"       },  /*  |     new in 2.3 */
  {  "\\ell\t",               "l"       },  /*  |     changed in 2.3 */
  {  "\\ij\t",                "i"       },  /*  |     changed in 2.3 */
  {  "\\IJ\t",                "I"       },  /*  |     changed in 2.3 */
  {  "\\Im\t",                "I"       },  /*  |     <=> \\Re */
  {  "\\LaTeX\t",             "LaTeX" },  /*  |     changed in 2.3 */
  {  "\\textnumero\t",        "N"       },  /*  |     new in 2.3 */
  {  "\\oe\t",                "oe"      },  /*  |     changed in 2.3 */
  {  "\\OE\t",                "Oe"      },  /*  |     changed in 2.3 */
  {  "\\TeX\t",               "TeX"     },  /* /      changed in 2.3 */
  {  "\\textdollaroldstyle\t","D"       },  /* \      new in 2.5 */
  {  "\\texteuro\t",          "E"       },  /*  |      */
  {  "\\textlira\t",          "L"       },  /*  |     new in 2.3 */
  {  "\\ng\t",                "n"       },  /*  |     changed in 2.3 */
  {  "\\NG\t",                "N"       },  /* CTL_Y  changed in 2.3 */
  {  "\\textsterling\t",      "P"       },  /*  |     changed in 2.3 */
  {  "\\ppounds\t",           "P"       },  /*  |     changed in 2.3 */
  {  "\\pounds\t",            "P"       },  /*  |     changed in 2.3 */
  {  "\\mathsterling\t",      "P"       },  /*  |     new in 2.5 */
  {  "\\th\t",                "th"      },  /*  |     */
  {  "\\Re\t",                "R"       },  /*  |    <=> \\Im */
  {  "\\TH\t",                "Th"      },  /* /      */
  {  "\\copyright\t",         "C"       },  /* \              */
  {  "\\textcopyright\t",     "C"       },  /*  |     siehe   */
  {  "\\pcopyright\t",        "C"       },  /*  |     \\ss    */
  {  "\\textcircledP\t",      "P"       },  /*  |     = ss  */
  {  "\\textregistered\t",    "R"       },  /* /              */
  {  "\\textcelsius\t",       "C"       },  /* \      changed in 2.5 */
  {  "\\textdegree\t",        ""        },  /* /   DEG{X} new in 2.5 */
  {  "\\alpha\t",             "a"       }, /* 28 */
  {  "\\Alpha\t",             "A"       },  /* bibarts.sty; new in 2.1 */
  {  "\\beta\t",              "b"       },
  {  "\\Beta\t",              "B"       },  /* bibarts.sty; new in 2.1 */
  {  "\\chi\t",               "ch"      },
  {  "\\Chi\t",               "Ch"      },  /* bibarts.sty; new in 2.1 */
  {  "\\delta\t",             "d"       },
  {  "\\Delta\t",             "D"       },
  {  "\\epsilon\t",           "e"       },
  {  "\\Epsilon\t",           "E"       },  /* bibarts.sty; new in 2.1 */
  {  "\\varepsilon\t",        "e"       },  /* changed in 2.3 */
  {  "\\eta\t",               "e"       }, /* 29 */
  {  "\\Eta\t",               "E"       },  /* bibarts.sty; new in 2.1 */
  {  "\\gamma\t",             "g"       },
  {  "\\Gamma\t",             "G"       },
  {  "\\iota\t",              "i"       },
  {  "\\Iota\t",              "I"       },  /* bibarts.sty; new in 2.1 */
  {  "\\kappa\t",             "k"       },
  {  "\\Kappa\t",             "K"       },  /* bibarts.sty; new in 2.1 */
  {  "\\varkappa\t",          "k"       },  /* changed in 2.3 */
  {  "\\lambda\t",            "l"       },
  {  "\\Lambda\t",            "L"       },
  {  "\\mu\t",                "m"       },
  {  "\\textmu\t",            "m"       },  /* new in 2.3 */
  {  "\\Mu\t",                "M"       },  /* bibarts.sty; new in 2.1 */
  {  "\\nu\t",                "n"       },
  {  "\\Nu\t",                "N"       },  /* bibarts.sty; new in 2.1 */
  {  "\\omicron\t",           "o"       },  /* bibarts.sty */
  {  "\\Omicron\t",           "O"       },  /* bibarts.sty; new in 2.1 */
  {  "\\omega\t",             "o"       },  /* changed in 2.3 */
  {  "\\Omega\t",             "O"       },  /* changed in 2.3 */
  {  "\\textohm\t",           "O"       },  /* new in 2.3 */
  {  "\\textmho\t",           "O"       },  /* new in 2.3 */
  {  "\\pi\t",                "p"       },
  {  "\\Pi\t",                "P"       },
  {  "\\varpi\t",             "p"       },  /* changed in 2.3 */
  {  "\\phi\t",               "ph"      },  /* changed in 2.3 */
  {  "\\Phi\t",               "Ph"      },  /* changed in 2.3 */
  {  "\\varphi\t",            "ph"      },  /* changed in 2.3 */
  {  "\\psi\t",               "ps"      }, /* 30 */  /* changed in 2.3 */
  {  "\\Psi\t",               "Ps"      },  /* changed in 2.3 */
  {  "\\rho\t",               "r"       },
  {  "\\Rho\t",               "R"       },  /* bibarts.sty; new in 2.1 */
  {  "\\varrho\t",            "r"       },  /* changed in 2.3 */
  {  "\\sigma\t",             "s"       },
  {  "\\Sigma\t",             "S"       },
  {  "\\varsigma\t",          "s"       },  /* changed in 2.3 */
  {  "\\tau\t",               "t"       },
  {  "\\Tau\t",               "T"       },  /* bibarts.sty; new in 2.1 */
  {  "\\theta\t",             "th"      },  /* changed in 2.3 */
  {  "\\Theta\t",             "Th"      },  /* changed in 2.3 */
  {  "\\vartheta\t",          "th"      },  /* changed in 2.3 */
  {  "\\xi\t",                "x"       },
  {  "\\Xi\t",                "X"       },
  {  "\\upsilon\t",           "y"       },
  {  "\\Upsilon\t",           "Y"       },
  {  "\\zeta\t",              "z"       },
  {  "\\Zeta\t",              "Z"       },  /* bibarts.sty; new in 2.1 */
  {  "\\textendash\t",          "--"      },  /* new in 2.3 */
  {  "\\textemdash\t",          "---"     },  /* new in 2.3 */
  {  "\\textellipsis\t",        "..."     },  /* new in 2.3 "-p" */
  {  "\\dots\t",                "..."     },  /* new in 2.3 "-p" */
  {  "\\textonesuperior\t",    "{1} "     },  /* new in 2.3 */
  {  "\\texttwosuperior\t",    "{2} "     },  /* new in 2.3 */
  {  "\\textthreesuperior\t",  "{3} "     },  /* new in 2.3 */
  {  "\\textonequarter\t",     "{1} {4} " },  /* new in 2.3 */
  {  "\\textonehalf\t",        "{1} {2} " },  /* new in 2.3 */
  {  "\\textthreequarters\t",  "{3} {4} " },  /* new in 2.3 */
  {  "\\textoneoldstyle\t",     "1"       },  /* new in 2.3 */
  {  "\\texttwooldstyle\t",     "2"       },  /* new in 2.3 */
  {  "\\textthreeoldstyle\t",   "3"       },  /* new in 2.3 */
  {  "\\textfouroldstyle\t",    "4"       },  /* new in 2.3 */
  {  "\\textfiveoldstyle\t",    "5"       },  /* new in 2.3 */
  {  "\\textsixoldstyle\t",     "6"       },  /* new in 2.3 */
  {  "\\textsevenoldstyle\t",   "7"       },  /* new in 2.3 */
  {  "\\texteightoldstyle\t",   "8"       },  /* new in 2.3 */
  {  "\\textnineoldstyle\t",    "9"       },  /* new in 2.3 */
  {  "\\textzerooldstyle\t",    "0"       },  /* new in 2.3 */
  { NULL, NULL }};
#define NLATEX  224


const char *udtable[][2] = { 
  {  "\\\"\ta",            "a"      },  /* \     \"a */  /* changend in 2.5*/
  {  "\\\"\to",            "o"      },  /*  |    \"o */  /* changend in 2.5*/
  {  "\\\"\tu",            "u"      },  /* CTL+E \"u */  /* changend in 2.5*/
  {  "\\\"\tA",            "A"      },  /*  |    \"A */  /* changend in 2.5*/
  {  "\\\"\tO",            "O"      },  /*  |    \"O */  /* changend in 2.5*/
  {  "\\\"\tU",            "U"      },  /*  |    \"U */  /* changend in 2.5*/
  {  "\\ss\t",             "s"      },  /*       \ss */
  {  "\\SS\t",               "SS"     },  /*       \SS */
  {  "\\sz\t",             "s"      },  /*       \sz */   /* new in 2.1 */
  {  "\\SZ\t",             "S"      },  /*       \SZ */   /* new in 2.1 */
  {  "\\flqq\t",             ""       },  /*     \flqq */   /* new in 2.2 */
  {  "\\frqq\t",             ""       },  /*     \frqq */   /* new in 2.2 */
  {  "\\glqq\t",             ""       },  /*     \glqq */   /* new in 2.2 */
  {  "\\grqq\t",             ""       },  /*     \grqq */   /* new in 2.2 */
  {  "\\/",                  ""       },  /*       \/  */   /* new in 2.2 */
  {  "\\-",                  ""       },  /*       \-  */   /* new in 2.2 */
  {  "\\oldhyc\t",           "c"      },  /*     [c-]  */   /* \          */
  {  "\\OLDHYC\t",           "C"      },  /*     [C-]  */   /*  |         */
  {  "\\oldhyl\t",           "l"      },  /*     [l-]  */   /*  |         */
  {  "\\OLDHYL\t",           "L"      },  /*     [L-]  */   /*  |         */
  {  "\\oldhym\t",           "m"      },  /*     [m-]  */   /*  |         */
  {  "\\OLDHYM\t",           "M"      },  /*     [M-]  */   /*  |         */
  {  "\\oldhyn\t",           "n"      },  /*     [n-]  */   /*  | new     */
  {  "\\OLDHYN\t",           "N"      },  /*     [N-]  */   /*  |  in     */
  {  "\\oldhyp\t",           "p"      },  /*     [p-]  */   /*  | 2.2     */
  {  "\\OLDHYP\t",           "P"      },  /*     [P-]  */   /*  |         */
  {  "\\oldhyr\t",           "r"      },  /*     [r-]  */   /*  |         */
  {  "\\OLDHYR\t",           "R"      },  /*     [R-]  */   /*  |         */
  {  "\\oldhyt\t",           "t"      },  /*     [t-]  */   /*  |         */
  {  "\\OLDHYT\t",           "T"      },  /*     [T-]  */   /*  |         */
  {  "\\oldhyf\t",           "f"      },  /*     [f-]  */   /*  |         */
  {  "\\OLDHYF\t",           "F"      },  /*     [F-]  */   /* /          */
  {  "\\hy\t",               "-"      },  /* MINUS \hy */   /* new in 2.2 */
  {  "\\fhy\t",              "-"      },  /* MINUS \fhy*/   /* new in 2.2 */
  {  "\\\"\t",             ""       },  /* /     \"  */  /* changend in 2.5*/
  {  "\\3",                "s"      },  /*       \3  */
  {  "\\ck\t",               "ck"     },  /*    [c-k]  */   /* new in 2.2 */
  {  "\\CK\t",               "CK"     },  /*    [C-K]  */   /* new in 2.2 */
  {  "\\oldhyss\t",        "s"      },  /*    [s-s]  */   /* \          */
  {  "\\OLDHYSS\t",          "SS"     },  /*    [S-S]  */   /*  | new     */
  {  "\\newhyss\t",        "s"      },  /*    [-ss]  */   /*  |  in     */
  {  "\\NEWHYSS\t",          "SS"     },  /*    [-SS]  */   /*  | 2.2     */
  {  "\\hyss\t",           "s"      },  /* [s-s/-ss] */   /*  |         */
  {  "\\HYSS\t",             "SS"     },  /* [S-S/-SS] */   /* /          */
  {  "\\capitaldieresis\ta",  "a"   },  /* new in 2.5 = \"a */
  {  "\\@tabacckludge\t\"\ta","a"   },  /* new in 2.5 = \"a */
  {  "\\a\t\"\ta",            "a"   },  /* new in 2.5 = \"a */
  {  "\\capitaldieresis\to",  "o"   },  /* new in 2.5 = \"o */
  {  "\\@tabacckludge\t\"\to","o"   },  /* new in 2.5 = \"o */
  {  "\\a\t\"\to",            "o"   },  /* new in 2.5 = \"o */
  {  "\\capitaldieresis\tu",  "u"   },  /* new in 2.5 = \"u */
  {  "\\@tabacckludge\t\"\tu","u"   },  /* new in 2.5 = \"u */
  {  "\\a\t\"\tu",            "u"   },  /* new in 2.5 = \"u */
  {  "\\capitaldieresis\tA",  "A"   },  /* new in 2.5 = \"A */
  {  "\\@tabacckludge\t\"\tA","A"   },  /* new in 2.5 = \"A */
  {  "\\a\t\"\tA",            "A"   },  /* new in 2.5 = \"A */
  {  "\\capitaldieresis\tO",  "O"   },  /* new in 2.5 = \"O */
  {  "\\@tabacckludge\t\"\tO","O"   },  /* new in 2.5 = \"O */
  {  "\\a\t\"\tO",            "O"   },  /* new in 2.5 = \"O */
  {  "\\capitaldieresis\tU",  "U"   },  /* new in 2.5 = \"U */
  {  "\\@tabacckludge\t\"\tU","U"   },  /* new in 2.5 = \"U */
  {  "\\a\t\"\tU",            "U"   },  /* new in 2.5 = \"U */
  {  "\\capitaldieresis\t",   ""    },  /* new in 2.5 = \" */
  {  "\\@tabacckludge\t\"\t", ""    },  /* new in 2.5 = \" */
  {  "\\a\t\"\t",             ""    },  /* new in 2.5 = \" */
  { NULL, NULL }};
#define NUMTEX  65


   /* \" changed to \"\t in 2.5 */
const char *gstylist[][2] = { 
  {  "\"\ta",                 NULL    },  /* \      "a */
  {  "\"\to",                 NULL    },  /*  |     "o */
  {  "\"\tu",                 NULL    },  /* CTL+E  "u */
  {  "\"\tA",                 NULL    },  /*  |     "A */
  {  "\"\tO",                 NULL    },  /*  |     "O */
  {  "\"\tU",                 NULL    },  /*  |     "U */
  {  "\"\ts",                 NULL    },  /*        "s */
  {  "\"\tS",                 NULL    },  /*        "S */
  {  "\"\tz",                 NULL    },  /*        "z */   /* new in 2.1 */
  {  "\"\tZ",                 NULL    },  /*        "Z */   /* new in 2.1 */
  {  "\"\t<",                 NULL    },  /*        "` */   /* new in 2.2 */
  {  "\"\t>",                 NULL    },  /*        "' */   /* new in 2.2 */
  {  "\"\t`",                 NULL    },  /*        "` */   /* new in 2.2 */
  {  "\"\t\'",                NULL    },  /*        "' */   /* new in 2.2 */
  {  "\"\t|",                 NULL    },  /*        "| */   /* new in 2.2 */
  {  "\"\t-",                 NULL    },  /*        "- */   /* new in 2.2 */
  {  "\"\tc",                 NULL    },  /*     [c-]  */   /* \          */
  {  "\"\tC",                 NULL    },  /*     [C-]  */   /*  |         */
  {  "\"\tl",                 NULL    },  /*     [l-]  */   /*  |         */
  {  "\"\tL",                 NULL    },  /*     [L-]  */   /*  |         */
  {  "\"\tm",                 NULL    },  /*     [m-]  */   /*  |         */
  {  "\"\tM",                 NULL    },  /*     [M-]  */   /*  |         */
  {  "\"\tn",                 NULL    },  /*     [n-]  */   /*  | new     */
  {  "\"\tN",                 NULL    },  /*     [N-]  */   /*  |  in     */
  {  "\"\tp",                 NULL    },  /*     [p-]  */   /*  | 2.2     */
  {  "\"\tP",                 NULL    },  /*     [P-]  */   /*  |         */
  {  "\"\tr",                 NULL    },  /*     [r-]  */   /*  |         */
  {  "\"\tR",                 NULL    },  /*     [R-]  */   /*  |         */
  {  "\"\tt",                 NULL    },  /*     [t-]  */   /*  |         */
  {  "\"\tT",                 NULL    },  /*     [T-]  */   /*  |         */
  {  "\"\tf",                 NULL    },  /*     [f-]  */   /*  |         */
  {  "\"\tF",                 NULL    },  /*     [F-]  */   /*  |         */
  {  "\"\t=",                 NULL    },  /* MINUS  "= */   /* new in 2.2 */
  {  "\"\t~",                 NULL    },  /* MINUS  "~ */   /* new in 2.2 */
  {  "\"\t",                  NULL    },  /* /      "  */
  { NULL, NULL }};
#define NUMGER  35


const char *gweight[][2] = { 
  {  NULL,                 "ae"     },  /* \      \"a "a */
  {  NULL,                 "oe"     },  /*  |     \"o "o */
  {  NULL,                 "ue"     },  /* CTL+E  \"u "u */
  {  NULL,                 "Ae"     },  /*  |     \"A "A */
  {  NULL,                 "Oe"     },  /*  |     \"O "O */
  {  NULL,                 "Ue"     },  /*  |     \"U "U */
  {  NULL,                 "ss"     },  /*        \ss "s */
  {  NULL,                   "SS"     },  /*        \SS "S */
  {  NULL,                 "ss"     },  /*        \sz "z */   /* new in 2.1 */
  {  NULL,                 "SS"     },  /*        \SZ "Z */   /* new in 2.1 */
  {  NULL,                   ""       },  /*     \flqq  "< */   /* new in 2.2 */
  {  NULL,                   ""       },  /*     \frqq  "> */   /* new in 2.2 */
  {  NULL,                   ""       },  /*     \glqq  "` */   /* new in 2.2 */
  {  NULL,                   ""       },  /*     \grqq  "' */   /* new in 2.2 */
  {  NULL,                   ""       },  /*        \/  "| */   /* new in 2.2 */
  {  NULL,                   ""       },  /*        \-  "- */   /* new in 2.2 */
  {  NULL,                   "c"      },  /*      [c-]  "c */   /* \          */
  {  NULL,                   "C"      },  /*      [C-]  "C */   /*  |         */
  {  NULL,                   "l"      },  /*      [l-]  "l */   /*  |         */
  {  NULL,                   "L"      },  /*      [L-]  "L */   /*  |         */
  {  NULL,                   "m"      },  /*      [m-]  "m */   /*  |         */
  {  NULL,                   "M"      },  /*      [M-]  "M */   /*  |         */
  {  NULL,                   "n"      },  /*      [n-]  "n */   /*  | new     */
  {  NULL,                   "N"      },  /*      [N-]  "N */   /*  |  in     */
  {  NULL,                   "p"      },  /*      [p-]  "p */   /*  | 2.2     */
  {  NULL,                   "P"      },  /*      [P-]  "P */   /*  |         */
  {  NULL,                   "r"      },  /*      [r-]  "r */   /*  |         */
  {  NULL,                   "R"      },  /*      [R-]  "R */   /*  |         */
  {  NULL,                   "t"      },  /*      [t-]  "t */   /*  |         */
  {  NULL,                   "T"      },  /*      [T-]  "T */   /*  |         */
  {  NULL,                   "f"      },  /*      [f-]  "f */   /*  |         */
  {  NULL,                   "F"      },  /*      [F-]  "F */   /* /          */
  {  NULL,                   "-"      },  /* MINUS \hy  "= */   /* new in 2.2 */
  {  NULL,                   "-"      },  /* MINUS \fhy "~ */   /* new in 2.2 */
  {  NULL,                 ""       },  /* /      \"  "  */
  {  NULL,                 "ss"     },  /*        \3  [] */
  {  NULL,                   "ck"     },  /*       \ck     */   /* new in 2.2 */
  {  NULL,                   "CK"     },  /*       \CK     */   /* new in 2.2 */
  {  NULL,                 "ss"     },  /*    [s-s]  */   /* \          */
  {  NULL,                   "SS"     },  /*    [S-S]  */   /*  | new     */
  {  NULL,                 "ss"     },  /*    [-ss]  */   /*  |  in     */
  {  NULL,                   "SS"     },  /*    [-SS]  */   /*  | 2.2     */
  {  NULL,                 "ss"     },  /* [s-s/-ss] */   /*  |         */
  {  NULL,                   "SS"     },  /* [S-S/-SS] */   /* /          */
  {  NULL,                   "ae"   },  /* new in 2.5 = \capitaldieresis a */
  {  NULL,                   "ae"   },  /* new in 2.5 = \@tabacckludge " a */
  {  NULL,                   "ae"   },  /* new in 2.5 = \a " a             */
  {  NULL,                   "oe"   },  /* new in 2.5 = \capitaldieresis o */
  {  NULL,                   "oe"   },  /* new in 2.5 = \@tabacckludge " o */
  {  NULL,                   "oe"   },  /* new in 2.5 = \a " o             */
  {  NULL,                   "ue"   },  /* new in 2.5 = \capitaldieresis u */
  {  NULL,                   "ue"   },  /* new in 2.5 = \@tabacckludge " u */
  {  NULL,                   "ue"   },  /* new in 2.5 = \a " u             */
  {  NULL,                   "Ae"   },  /* new in 2.5 = \capitaldieresis A */
  {  NULL,                   "Ae"   },  /* new in 2.5 = \@tabacckludge " A */
  {  NULL,                   "Ae"   },  /* new in 2.5 = \a " A             */
  {  NULL,                   "Oe"   },  /* new in 2.5 = \capitaldieresis O */
  {  NULL,                   "Oe"   },  /* new in 2.5 = \@tabacckludge " O */
  {  NULL,                   "Oe"   },  /* new in 2.5 = \a " O             */
  {  NULL,                   "Ue"   },  /* new in 2.5 = \capitaldieresis U */
  {  NULL,                   "Ue"   },  /* new in 2.5 = \@tabacckludge " U */
  {  NULL,                   "Ue"   },  /* new in 2.5 = \a " U             */
  {  NULL,                   ""     },  /* new in 2.5 = \capitaldieresis   */
  {  NULL,                   ""     },  /* new in 2.5 = \@tabacckludge "   */
  {  NULL,                   ""     },  /* new in 2.5 = \a "               */
  { NULL, NULL }};


const char *mweight[][2] = { 
  {  NULL,                 "a"      },  /* \     \"a "a */
  {  NULL,                 "o"      },  /*  |    \"o "o */
  {  NULL,                 "u"      },  /* CTL+E \"u "u */
  {  NULL,                 "A"      },  /*  |    \"A "A */
  {  NULL,                 "O"      },  /*  |    \"O "O */
  {  NULL,                 "U"      },  /*  |    \"U "U */
  {  NULL,                 "ss"     },  /*       \ss "s */
  {  NULL,                   "SS"     },  /*       \SS "S */
  {  NULL,                 "ss"     },  /*       \sz "z */   /* new in 2.1 */
  {  NULL,                 "SS"     },  /*       \SZ "Z */   /* new in 2.1 */
  {  NULL,                   ""       },  /*     \flqq "< */   /* new in 2.2 */
  {  NULL,                   ""       },  /*     \frqq "> */   /* new in 2.2 */
  {  NULL,                   ""       },  /*     \glqq "` */   /* new in 2.2 */
  {  NULL,                   ""       },  /*     \grqq "' */   /* new in 2.2 */
  {  NULL,                   ""       },  /*       \/  "| */   /* new in 2.2 */
  {  NULL,                   ""       },  /*       \-  "- */   /* new in 2.2 */
  {  NULL,                   "c"      },  /*      [c-]  "c */   /* \          */
  {  NULL,                   "C"      },  /*      [C-]  "C */   /*  |         */
  {  NULL,                   "l"      },  /*      [l-]  "l */   /*  |         */
  {  NULL,                   "L"      },  /*      [L-]  "L */   /*  |         */
  {  NULL,                   "m"      },  /*      [m-]  "m */   /*  |         */
  {  NULL,                   "M"      },  /*      [M-]  "M */   /*  |         */
  {  NULL,                   "n"      },  /*      [n-]  "n */   /*  | new     */
  {  NULL,                   "N"      },  /*      [N-]  "N */   /*  |  in     */
  {  NULL,                   "p"      },  /*      [p-]  "p */   /*  | 2.2     */
  {  NULL,                   "P"      },  /*      [P-]  "P */   /*  |         */
  {  NULL,                   "r"      },  /*      [r-]  "r */   /*  |         */
  {  NULL,                   "R"      },  /*      [R-]  "R */   /*  |         */
  {  NULL,                   "t"      },  /*      [t-]  "t */   /*  |         */
  {  NULL,                   "T"      },  /*      [T-]  "T */   /*  |         */
  {  NULL,                   "f"      },  /*      [f-]  "f */   /*  |         */
  {  NULL,                   "F"      },  /*      [F-]  "F */   /* /          */
  {  NULL,                   "-"      },  /*      \hy  "= */   /* new in 2.2 */
  {  NULL,                   "-"      },  /*      \fhy "~ */   /* new in 2.2 */
  {  NULL,                 ""       },  /* /     \"  "  */
  {  NULL,                 "ss"     },  /*       \3  [] */
  {  NULL,                   "ck"     },  /*      \ck     */   /* new in 2.2 */
  {  NULL,                   "CK"     },  /*      \CK     */   /* new in 2.2 */
  {  NULL,                 "ss"     },  /*    [s-s]  */   /* \          */
  {  NULL,                   "SS"     },  /*    [S-S]  */   /*  | new     */
  {  NULL,                 "ss"     },  /*    [-ss]  */   /*  |  in     */
  {  NULL,                   "SS"     },  /*    [-SS]  */   /*  | 2.2     */
  {  NULL,                 "ss"     },  /* [s-s/-ss] */   /*  |         */
  {  NULL,                   "SS"     },  /* [S-S/-SS] */   /* /          */
  {  NULL,                   "a"    },  /* new in 2.5 = \capitaldieresis a */
  {  NULL,                   "a"    },  /* new in 2.5 = \@tabacckludge " a */
  {  NULL,                   "a"    },  /* new in 2.5 = \a " a             */
  {  NULL,                   "o"    },  /* new in 2.5 = \capitaldieresis o */
  {  NULL,                   "o"    },  /* new in 2.5 = \@tabacckludge " o */
  {  NULL,                   "o"    },  /* new in 2.5 = \a " o             */
  {  NULL,                   "u"    },  /* new in 2.5 = \capitaldieresis u */
  {  NULL,                   "u"    },  /* new in 2.5 = \@tabacckludge " u */
  {  NULL,                   "u"    },  /* new in 2.5 = \a " u             */
  {  NULL,                   "A"    },  /* new in 2.5 = \capitaldieresis A */
  {  NULL,                   "A"    },  /* new in 2.5 = \@tabacckludge " A */
  {  NULL,                   "A"    },  /* new in 2.5 = \a " A             */
  {  NULL,                   "O"    },  /* new in 2.5 = \capitaldieresis O */
  {  NULL,                   "O"    },  /* new in 2.5 = \@tabacckludge " O */
  {  NULL,                   "O"    },  /* new in 2.5 = \a " O             */
  {  NULL,                   "U"    },  /* new in 2.5 = \capitaldieresis U */
  {  NULL,                   "U"    },  /* new in 2.5 = \@tabacckludge " U */
  {  NULL,                   "U"    },  /* new in 2.5 = \a " U             */
  {  NULL,                   ""     },  /* new in 2.5 = \capitaldieresis   */
  {  NULL,                   ""     },  /* new in 2.5 = \@tabacckludge "   */
  {  NULL,                   ""     },  /* new in 2.5 = \a "               */
  { NULL, NULL }};


int lenofMATCHaftertab(const char *t, const char *line)
{
  int r = 0;

  if (*t == '\t')
  {
    if (*(t+1) != '\0')
    {
      if (*(t+2) == '\0' || *(t+2) == '\t')
      {
        if (*(t+1) == *line) r = 1;
         else
        if (*line == '{' && *(t+1) == *(line+1) && *(line+2) == '}') r = 3;
      }
      else intern_err(34);
    }
  }

  if (r > 0 && *(t+2) == '\t')
  {
    while (*(line+r) == ' ') ++r;

    if(*(t+3) != '\0')
    {
      if (*(t+4) == '\0')
      {
        if (*(t+3) == *(line+r)) r += 1;
         else
        if (*(line+r) == '{' && *(t+3) == *(line+r+1) && *(line+r+2) == '}') r += 3;
         else
        r = 0;
      }
      else intern_err(37);
    }
  }

  return r;
}


int texlistnum(const char *p, int n, const char *list[][2])
{
   const char *entry, *line;
   int i = 0, len, atlen;

   while (i < n)
   { 
       entry = list[i][0];
       line  = p; len = 0;


       while (*line == *entry && *entry != '\0' && *entry != '\t')
       {
         ++line; ++entry; ++len;
       }
      

       if (     *entry == '\0'
            || (*entry == '\t' && *(entry+1) == '\0' && ((len == 2 && *p == '\\' && isLaTeXcmdletter(*(p+1)) == 0) || isLaTeXcmdletter(*line) == 0))
          )
         break;
       

       if (*entry == '\t' && ((len == 1 && *p == '\"') || (len == 2 && *p == '\\' && isLaTeXcmdletter(*(p+1)) == 0) || isLaTeXcmdletter(*line) == 0))
       {
         while (*line == ' ') ++line;  /* now, isLaTeXcmdletter(*line) != 0 */

                    /* new in 2.3, changed in 2.5 */
         if ((atlen=lenofMATCHaftertab(entry, line)) > 0)   /* atlen=1,3 !! */
           break;
       }

       ++i;
   }

   if (i == n) return -1;
   else        return  i;
}


int insposmaxstr(char s[], const char *p, int pos, int max)
{
   while(*p != '\0' && pos < max) s[pos++] = *p++;
   s[pos] = '\0';
   return pos;
}


int llstrlen(const char *s)
{
   int len = 0;

   while  (*s != '\0' && *s != '\t') { ++s; ++len; }

   return len;
}


char *overspacesafterlettercmd(int len, const char *q, char *p)
{
  int atlen;

  /* if (isLaTeXcmdletter(*(q+1)) != 0) deleted in 2.5 !! */

  if (len > 0)
  {
    if (*(q+len) == '\t' && ((len == 1 && *q == '\"') || (len == 2 && *q == '\\' && isLaTeXcmdletter(*(q+1)) == 0) || isLaTeXcmdletter(*p) == 0))
    {
      while (*p == ' ') ++p;  /* now, isLaTeXcmdletter(*p) != 0 */

         /* new in 2.5 because of \capitaldieresis{x} */
      if((atlen=lenofMATCHaftertab(q+len, p)) > 0)
      {
        p += atlen;
      }
    }
  }

  return p;
}


int insertweight(char in[], int i, int pos, int max)
{
       if (german == 1) pos = insposmaxstr(in, gweight[i][1], pos, max);
  else if (german == 2) pos = insposmaxstr(in, mweight[i][1], pos, max);
  else                  pos = insposmaxstr(in, udtable[i][1], pos, max);

  return pos;
}


int nichtvorziehen(char a)
{
  if (issonder(a) == 1 && a != '') 
  return 1;                /*  31  */
  return 0;
}


int isinitial(char a)
{
  if (a == '' || a == '')
  return 1;
  return 0;
}


void nzvorziehen(char in[], char *p)
{
  int pos = 0, l;
  char mem;
  
  while (*p != '\0')
  {
    if (nichtvorziehen(*p) == 1)
    {
      if (isinitial(*(p+1)) == 1)
      {
              l = pos;
        while (nichtvorziehen(*p) && isinitial(*(p+1)) == 1)
        { in[l++] = *p++; in[l++] = *p++; }
            mem = in[l-1];
        in[l-1] = in[pos+1];
        in[pos+1] = mem;
            mem = in[l-2];
        in[l-2] = in[pos];
        in[pos] = mem;
            pos = l;
      }
      else { ++p; intern_err(35); }
    }
    else 
    { 
      if (isinitial(*(p+1)) == 1) { ++p; intern_err(36); }
      else
      in[pos++] = *p++;
    }
  }

  in[pos] = '\0';
}


char *austauschen(char in[], char *p, int dqcat)
{
  int pos = 0, i, len;

  while (*p != '\0' && pos < MAXLEN-1)
  {    
    if (*p == '\\')       /* "\\\\" ist schon ' ' */
    {
      if                  ((i=texlistnum(p, NUMTEX,  udtable)) > -1)
      {                          p += (len=llstrlen( udtable[i][0]));
                              pos = insertweight(in,         i,         pos, MAXLEN-1);
                   p = overspacesafterlettercmd(len, udtable[i][0], p);
      }
      else             if ((i=texlistnum(p, NLATEX,  weighttable)) > -1)
      {                          p += (len=llstrlen( weighttable[i][0]));
                              pos = insposmaxstr(in, weighttable[i][1], pos, MAXLEN-1);
                   p = overspacesafterlettercmd(len, weighttable[i][0], p);
      }
      else if (*(p+1) == '~') p += 2;   /* falls \~ nicht in weighttable */
      else if (isLaTeXcmdletter(*(p+1)) != 0)
      { ++p; while (isLaTeXcmdletter(*p) != 0) ++p;
             while (*p == ' ') ++p;
      }
      else 
      { 
        if (issonder(*p) == 0) in[pos++] = *p;
        ++p;
      }
    }
    else
    if (*p == '\"' && dqcat == TeXACTIVE)  /* nogsty == 0 */
    {
      if (*(p+1) == '\"') p += 2;       /*  "" ist nicht in gstylist  */
      else                              /*  "~ muss in gstylist sein  */
      if (       (i=texlistnum(p, NUMGER,  gstylist)) > -1)
      {                p += (len=llstrlen( gstylist[i][0]));
                    pos = insertweight(in,          i,      pos, MAXLEN-1);
         p = overspacesafterlettercmd(len, gstylist[i][0], p);  /* new in 2.5 */
      }
      else 
      { 
        if (issonder(*p) == 0) in[pos++] = *p;
        ++p;
      }
    }
    else if (*p == '~')                         /* MARK_TILDE */
    {
        in[pos++] = ' ';
        ++p;
    }
    else 
    { 
      if (issonder(*p) == 0) in[pos++] = *p;
      ++p;
    }
  }

  in[pos] = '\0';

  return in;
}


void kill_unnecessary_spaces(char *s, char *p)
{

   while (*p == ' ') ++p;

   while (*p != '\0')
   {
          if (*p == ' ' && *(p+1) == ' ') ++p;
     else if (*p == ' ' && *(p+1) == '\0') break;
     else *s++ = *p++;
   }

   *s = '\0';
}


void swapBCstrs(const int linec)
{
  char *p;

                  p = subBptr[0][linec];
  subBptr[0][linec] = subBptr[1][linec];
  subBptr[1][linec] = p;

                  p = subCptr[0][linec];
  subCptr[0][linec] = subCptr[1][linec];
  subCptr[1][linec] = p;
}


static char *teinzerw[] = {
   "\\u{A}",    /*  128  */
   "\\k{A}",    /*  129  */
   "\\\'C",     /*  130  */
   "\\v{C}",    /*  131  */
   "\\v{D}",    /*  132  */
   "\\v{E}",    /*  133  */
   "\\k{E}",    /*  134  */
   "\\u{G}",    /*  135  */
   "\\\'L",     /*  136  */
   "\\v{L}",    /*  137  */
   "\\L{}",     /*  138  */
   "\\\'N",     /*  139  */
   "\\v{N}",    /*  140  */
   "\\NG{}",    /*  141  */
   "\\H{O}",    /*  142  */
   "\\\'R",     /*  143  */
   "\\v{R}",    /*  144  */
   "\\\'S",     /*  145  */
   "\\v{S}",    /*  146  */
   "\\c{S}",    /*  147  */
   "\\v{T}",    /*  148  */
   "\\c{T}",    /*  149  */
   "\\H{U}",    /*  150  */
   "\\r{U}",    /*  151  */
   "\\\"Y",     /*  152  */
   "\\\'Z",     /*  153  */
   "\\v{Z}",    /*  154  */
   "\\.Z",      /*  155  */
   "\\IJ{}",    /*  156  */
   "\\.I",      /*  157  */
   "\\dj{}",    /*  158  */
   "\\S{}",     /*  159  */    /* eigentl. "\\textsection{}" */
   "\\u{a}",    /*  160  */
   "\\k{a}",    /*  161  */
   "\\\'c",     /*  162  */
   "\\v{c}",    /*  163  */
   "\\v{d}",    /*  164  */
   "\\v{e}",    /*  165  */
   "\\k{e}",    /*  166  */
   "\\u{g}",    /*  167  */
   "\\\'l",     /*  168  */
   "\\v{l}",    /*  169  */
   "\\l{}",     /*  170  */
   "\\\'n",     /*  171  */
   "\\v{n}",    /*  172  */
   "\\ng{}",    /*  173  */
   "\\H{o}",    /*  174  */
   "\\\'r",     /*  175  */
   "\\v{r}",    /*  176  */
   "\\\'s",     /*  177  */
   "\\v{s}",    /*  178  */
   "\\c{s}",    /*  179  */
   "\\v{t}",    /*  180  */
   "\\c{t}",    /*  181  */
   "\\H{u}",    /*  182  */
   "\\r{u}",    /*  183  */
   "\\\"y",     /*  184  */
   "\\\'z",     /*  185  */
   "\\v{z}",    /*  186  */
   "\\.z",      /*  187  */
   "\\ij{}",    /*  188  */
   "!`",        /*  189  */    /* eigentl. "\\textexclamdown{}"   */
   "\?`",       /*  190  */    /* eigentl. "\\textquestiondown{}" */
   "\\pounds{}",/*  191  */    /* eigentl. "\\textsterling{}"     */
   "\\`A",      /*  192  */
   "\\\'A",     /*  193  */
   "\\^A",      /*  194  */
   "\\~A",      /*  195  */
   "\\\"A",     /*  196  */
   "\\r{A}",    /*  197  */
   "\\AE{}",    /*  198  */
   "\\c{C}",    /*  199  */
   "\\`E",      /*  200  */
   "\\\'E",     /*  201  */
   "\\^E",      /*  202  */
   "\\\"E",     /*  203  */
   "\\`I",      /*  204  */
   "\\\'I",     /*  205  */
   "\\^I",      /*  206  */
   "\\\"I",     /*  207  */
   "\\DH{}",    /*  208        "\\DJ{}",   208  */
   "\\~N",      /*  209  */
   "\\`O",      /*  210  */
   "\\\'O",     /*  211  */
   "\\^O",      /*  212  */
   "\\~O",      /*  213  */
   "\\\"O",     /*  214  */
   "\\OE{}",    /*  215  */
   "\\O{}",     /*  216  */
   "\\`U",      /*  217  */
   "\\\'U",     /*  218  */
   "\\^U",      /*  219  */
   "\\\"U",     /*  220  */
   "\\\'Y",     /*  221  */
   "\\TH{}",    /*  222  */
   "\\SS{}",    /*  223  */
   "\\`a",      /*  224  */
   "\\\'a",     /*  225  */
   "\\^a",      /*  226  */
   "\\~a",      /*  227  */
   "\\\"a",     /*  228  */
   "\\r{a}",    /*  229  */
   "\\ae{}",    /*  230  */
   "\\c{c}",    /*  231  */
   "\\`e",      /*  232  */
   "\\\'e",     /*  233  */
   "\\^e",      /*  234  */
   "\\\"e",     /*  235  */
   "\\`\\i{}",  /*  236  */
   "\\\'\\i{}", /*  237  */
   "\\^\\i{}",  /*  238  */
   "\\\"\\i{}", /*  239  */
   "\\dh{}",    /*  240  */
   "\\~n",      /*  241  */
   "\\`o",      /*  242  */
   "\\\'o",     /*  243  */
   "\\^o",      /*  244  */
   "\\~o",      /*  245  */
   "\\\"o",     /*  246  */
   "\\oe{}",    /*  247  */
   "\\o{}",     /*  248  */
   "\\`u",      /*  249  */
   "\\\'u",     /*  250  */
   "\\^u",      /*  251  */
   "\\\"u",     /*  252  */
   "\\\'y",     /*  253  */
   "\\th{}",    /*  254  */
   "\\ss{}"};   /*  255  */

char *getTONEenc(unsigned char c)
{
  if (c >= 128 && c <= 255)
       return teinzerw[c-128];
  else return "";
}


/*--------------------------------------------------------------------------*/


#define DOUBLETTES_BEGIN               194
#define DOUBLETTES_END                 223

#define TRIPLETTES_BEGIN               224
#define TRIPLETTES_END                 239

#define QUADRUPLES_BEGIN               240
#define QUADRUPLES_END                 244

#define UTF_BLOCK_BEGIN           128
#define UTF_BLOCK_END             191

static char *ohninetyfour[] = {
   "",                     /* 194 128 <control> = U+0080 */
   "",                     /* 194 129 <control> */
   "",                     /* 194 130 <control> */
   "",                     /* 194 131 <control> */
   "",                     /* 194 132 <control> */
   "",                     /* 194 133 <control> */
   "",                     /* 194 134 <control> */
   "",                     /* 194 135 <control> */
   "",                     /* 194 136 <control> */
   "",                     /* 194 137 <control> */
   "",                     /* 194 138 <control> */
   "",                     /* 194 139 <control> */
   "",                     /* 194 140 <control> */
   "",                     /* 194 141 <control> */
   "",                     /* 194 142 <control> */
   "",                     /* 194 143 <control> */
   "",                     /* 194 144 <control> */
   "",                     /* 194 145 <control> */
   "",                     /* 194 146 <control> */
   "",                     /* 194 147 <control> */
   "",                     /* 194 148 <control> */
   "",                     /* 194 149 <control> */
   "",                     /* 194 150 <control> */
   "",                     /* 194 151 <control> */
   "",                     /* 194 152 <control> */
   "",                     /* 194 153 <control> */
   "",                     /* 194 154 <control> */
   "",                     /* 194 155 <control> */
   "",                     /* 194 156 <control> */
   "",                     /* 194 157 <control> */
   "",                     /* 194 158 <control> */
   "",                     /* 194 159 <control> */
   "{\\nobreakspace}",     /* 194 160 ~ */
   "{\\textexclamdown}",   /* 194 161 !` */
   "{\\textcent}",         /* 194 162 */
   "{\\textsterling}",     /* 194 163 */
   "{\\textcurrency}",     /* 194 164 */
   "{\\textyen}",          /* 194 165 */
   "{\\textbrokenbar}",    /* 194 166 */
   "{\\textsection}",      /* 194 167 */
   "{\\textasciidieresis}",/* 194 168 \\\"{} */
   "{\\textcopyright}",    /* 194 169 */
   "{\\textordfeminine}",  /* 194 170 */
   "{\\guillemetleft}",    /* 194 171 {\\flqq}; before 2.5 / v1.3c: {\\guillemotleft} */
   "{\\textlnot}",         /* 194 172 NOT SIGN */
   "{\\-}",                /* 194 173 */
   "{\\textregistered}",   /* 194 174 */
   "{\\textasciimacron}",  /* 194 175 \\={} */
   "{\\textdegree}",       /* 194 176 */
   "{\\textpm}",           /* 194 177 aehnlich $\pm$ */
   "{\\texttwosuperior}",  /* 194 178 ^{2} */
   "{\\textthreesuperior}",/* 194 179 ^{3} */
   "{\\textasciiacute}",   /* 194 180 \\\'{} */
   "{\\textmu}",           /* 194 181 MICRO SIGN */
   "{\\textparagraph}",    /* 194 182 PILCROW SIGN */
   "{\\textperiodcentered}",  /* 194 183 {\\cdot} */
   "{\\textasciicedilla}", /* 194 184 {\\c\\ } (\text... yet undefined) */
   "{\\textonesuperior}",  /* 194 185 ^{1} */
   "{\\textordmasculine}", /* 194 186 */
   "{\\guillemetright}",   /* 194 187 {\\frqq}; before 2.5 / v1.3c: {\\guillemotright} */
   "{\\textonequarter}",   /* 194 188 \\frac{1}{4} */
   "{\\textonehalf}",      /* 194 189 \\frac{1}{2} */
   "{\\textthreequarters}",/* 194 190 \\frac{3}{4} */
   "{\\textquestiondown}", /* 194 191 ?` */
   NULL };

#define OHNINETYFOUR          194

char *get_ohninetyfour(unsigned char c)
{
  if (c >= UTF_BLOCK_BEGIN && c <= UTF_BLOCK_END)  /* 128 ... 191 */
       return ohninetyfour[c-UTF_BLOCK_BEGIN];
  else return "";
}


static char *ohninetyfive[] = {
   "\\`A",                 /* 195 128 */
   "\\\'A",                /* 195 129 */
   "\\^A",                 /* 195 130 */
   "\\~A",                 /* 195 131 */
   "\\\"A",                /* 195 132 */
   "\\r{A}",               /* 195 133 {\\AA} */
   "{\\AE}",               /* 195 134 */
   "\\c{C}",               /* 195 135 */
   "\\`E",                 /* 195 136 */
   "\\\'E",                /* 195 137 */
   "\\^E",                 /* 195 138 */
   "\\\"E",                /* 195 139 */
   "\\`I",                 /* 195 140 */
   "\\\'I",                /* 195 141 */
   "\\^I",                 /* 195 142 */
   "\\\"I",                /* 195 143 */
   "{\\DH}",               /* 195 144 not OT1 */
   "\\~N",                 /* 195 145 */
   "\\`O",                 /* 195 146 */
   "\\\'O",                /* 195 147 */
   "\\^O",                 /* 195 148 */
   "\\~O",                 /* 195 149 */
   "\\\"O",                /* 195 150 */
   "{\\texttimes}",        /* 195 151 */
   "{\\O}",                /* 195 152 */
   "\\`U",                 /* 195 153 */
   "\\\'U",                /* 195 154 */
   "\\^U",                 /* 195 155 */
   "\\\"U",                /* 195 156 */
   "\\\'Y",                /* 195 157 */
   "{\\TH}",               /* 195 158 not OT1 */
   "{\\ss}",               /* 195 159 */
   "\\`a",                 /* 195 160 */
   "\\\'a",                /* 195 161 */
   "\\^a",                 /* 195 162 */
   "\\~a",                 /* 195 163 */
   "\\\"a",                /* 195 164 */
   "\\r{a}",               /* 195 165 {\\aa} */
   "{\\ae}",               /* 195 166 */
   "\\c{c}",               /* 195 167 */
   "\\`e",                 /* 195 168 */
   "\\\'e",                /* 195 169 */
   "\\^e",                 /* 195 170 */
   "\\\"e",                /* 195 171 */
   "\\`{\\i}",             /* 195 172 */
   "\\\'{\\i}",            /* 195 173 */
   "\\^{\\i}",             /* 195 174 */
   "\\\"{\\i}",            /* 195 175 */
   "{\\dh}",               /* 195 176 not OT1 */
   "\\~n",                 /* 195 177 */
   "\\`o",                 /* 195 178 */
   "\\\'o",                /* 195 179 */
   "\\^o",                 /* 195 180 */
   "\\~o",                 /* 195 181 */
   "\\\"o",                /* 195 182 */
   "{\\textdiv}",          /* 195 183 */
   "{\\o}",                /* 195 184 */
   "\\`u",                 /* 195 185 */
   "\\\'u",                /* 195 186 */
   "\\^u",                 /* 195 187 */
   "\\\"u",                /* 195 188 */
   "\\\'y",                /* 195 189 */
   "{\\th}",               /* 195 190 not OT1 */
   "\\\"y",                /* 195 191 */
   NULL };

#define OHNINETYFIVE          195

char *get_ohninetyfive(unsigned char c)
{
  if (c >= UTF_BLOCK_BEGIN && c <= UTF_BLOCK_END)  /* 128 ... 191 */
       return ohninetyfive[c-UTF_BLOCK_BEGIN];
  else return "";
}


static char *ohninetysix[] = {
   "\\=A",                 /* 196 128 */
   "\\=a",                 /* 196 129 */
   "\\u{A}",               /* 196 130 */
   "\\u{a}",               /* 196 131 */
   "\\k{A}",               /* 196 132 not OT1 */
   "\\k{a}",               /* 196 133 not OT1 */
   "\\\'C",                /* 196 134 */
   "\\\'c",                /* 196 135 */
   "\\^C",                 /* 196 136 */
   "\\^c",                 /* 196 137 */
   "\\.C",                 /* 196 138 */
   "\\.c",                 /* 196 139 */
   "\\v{C}",               /* 196 140 */
   "\\v{c}",               /* 196 141 */
   "\\v{D}",               /* 196 142 */
   "\\v{d}",               /* 196 143 */
   "{\\DJ}",               /* 196 144 not OT1 */
   "{\\dj}",               /* 196 145 not OT1 */
   "\\=E",                 /* 196 146 */
   "\\=e",                 /* 196 147 */
   "\\u{E}",               /* 196 148 */
   "\\u{e}",               /* 196 149 */
   "\\.E",                 /* 196 150 */
   "\\.e",                 /* 196 151 */
   "\\k{E}",               /* 196 152 not OT1 */
   "\\k{e}",               /* 196 153 not OT1 */
   "\\v{E}",               /* 196 154 */
   "\\v{e}",               /* 196 155 */
   "\\^G",                 /* 196 156 */
   "\\^g",                 /* 196 157 */
   "\\u{G}",               /* 196 158 */
   "\\u{g}",               /* 196 159 */
   "\\.G",                 /* 196 160 */
   "\\.g",                 /* 196 161 */
   "\\c{G}",               /* 196 162 */
   "\\c{g}",               /* 196 163 */
   "\\^H",                 /* 196 164 */
   "\\^h",                 /* 196 165 */
   "",                     /* 196 166 H-quer (not v1.3c) */
   "",                     /* 196 167 {\\hbar} [?] (not v1.3c) */
   "\\~I",                 /* 196 168 */
   "\\~{\\i}",             /* 196 169 */
   "\\=I",                 /* 196 170 */
   "\\={\\i}",             /* 196 171 */
   "\\u{I}",               /* 196 172 */
   "\\u{\\i}",             /* 196 173 */
   "\\k{I}",               /* 196 174 not OT1 */
   "\\k{i}",               /* 196 175 not OT1 */
   "\\.I",                 /* 196 176 */
   "{\\i}",                /* 196 177 */
   "{\\IJ}",               /* 196 178 */
   "{\\ij}",               /* 196 179 */
   "\\^J",                 /* 196 180 */
   "\\^{\\j}",             /* 196 181 */
   "\\c{K}",               /* 196 182 */
   "\\c{k}",               /* 196 183 */
   "",                     /* 196 184 kra (not v1.3c) */
   "\\\'L",                /* 196 185 */
   "\\\'l",                /* 196 186 */
   "\\c{L}",               /* 196 187 */
   "\\c{l}",               /* 196 188 */
   "\\v{L}",               /* 196 189 */
   "\\v{l}",               /* 196 190 */
   "",                     /* 196 191 L* (not v1.3c) */
   NULL };

#define OHNINETYSIX           196

char *get_ohninetysix(unsigned char c)
{
  if (c >= UTF_BLOCK_BEGIN && c <= UTF_BLOCK_END)  /* 128 ... 191 */
       return ohninetysix[c-UTF_BLOCK_BEGIN];
  else return "";
}


static char *ohninetyseven[] = {
   "",                     /* 197 128 l* (not v1.3c) */
   "{\\L}",                /* 197 129 */
   "{\\l}",                /* 197 130 */
   "\\\'N",                /* 197 131 */
   "\\\'n",                /* 197 132 */
   "\\c{N}",               /* 197 133 */
   "\\c{n}",               /* 197 134 */
   "\\v{N}",               /* 197 135 */
   "\\v{n}",               /* 197 136 */
   "",                     /* 197 137 'n (not v1.3c) */
   "{\\NG}",               /* 197 138 */
   "{\\ng}",               /* 197 139 */
   "\\=O",                 /* 197 140 */
   "\\=o",                 /* 197 141 */
   "\\u{O}",               /* 197 142 */
   "\\u{o}",               /* 197 143 */
   "\\H{O}",               /* 197 144 */
   "\\H{o}",               /* 197 145 */
   "{\\OE}",               /* 197 146 */
   "{\\oe}",               /* 197 147 */
   "\\\'R",                /* 197 148 */
   "\\\'r",                /* 197 149 */
   "\\c{R}",               /* 197 150 */
   "\\c{r}",               /* 197 151 */
   "\\v{R}",               /* 197 152 */
   "\\v{r}",               /* 197 153 */
   "\\\'S",                /* 197 154 */
   "\\\'s",                /* 197 155 */
   "\\^S",                 /* 197 156 */
   "\\^s",                 /* 197 157 */
   "\\c{S}",               /* 197 158 */
   "\\c{s}",               /* 197 159 */
   "\\v{S}",               /* 197 160 */
   "\\v{s}",               /* 197 161 */
   "\\c{T}",               /* 197 162 */
   "\\c{t}",               /* 197 163 */
   "\\v{T}",               /* 197 164 */
   "\\v{t}",               /* 197 165 */
   "",                     /* 197 166 T-quer (not v1.3c) */
   "",                     /* 197 167 t-quer (not v1.3c) */
   "\\~U",                 /* 197 168 */
   "\\~u",                 /* 197 169 */
   "\\=U",                 /* 197 170 */
   "\\=u",                 /* 197 171 */
   "\\u{U}",               /* 197 172 */
   "\\u{u}",               /* 197 173 */
   "\\r{U}",               /* 197 174 */
   "\\r{u}",               /* 197 175 */
   "\\H{U}",               /* 197 176 */
   "\\H{u}",               /* 197 177 */
   "\\k{U}",               /* 197 178 */
   "\\k{u}",               /* 197 179 */
   "\\^W",                 /* 197 180 */
   "\\^w",                 /* 197 181 */
   "\\^Y",                 /* 197 182 */
   "\\^y",                 /* 197 183 */
   "\\\"Y",                /* 197 184 */
   "\\\'Z",                /* 197 185 */
   "\\\'z",                /* 197 186 */
   "\\.Z",                 /* 197 187 */
   "\\.z",                 /* 197 188 */
   "\\v{Z}",               /* 197 189 */
   "\\v{z}",               /* 197 190 */
   "",                     /* 197 191 Lang-s (not v1.3c) */
   NULL };

#define OHNINETYSEVEN         197

char *get_ohninetyseven(unsigned char c)
{
  if (c >= UTF_BLOCK_BEGIN && c <= UTF_BLOCK_END)  /* 128 ... 191 */
       return ohninetyseven[c-UTF_BLOCK_BEGIN];
  else return "";
}


static char *ohninetynine[] = {
   "",                     /* 199 128 dental click (not v1.3c) */
   "",                     /* 199 129 lateral click (not v1.3c) */
   "",                     /* 199 130 alveolar click (not v1.3c) */
   "",                     /* 199 131 retroflex click (not v1.3c) */
   "D\\v{Z}",              /* 199 132 new in 2.5 / v1.3c */
   "D\\v{z}",              /* 199 133 new in 2.5 / v1.3c */
   "d\\v{z}",              /* 199 134 new in 2.5 / v1.3c */
   "LJ",                   /* 199 135 new in 2.5 / v1.3c */
   "Lj",                   /* 199 136 new in 2.5 / v1.3c */
   "lj",                   /* 199 137 new in 2.5 / v1.3c */
   "NJ",                   /* 199 138 new in 2.5 / v1.3c */
   "Nj",                   /* 199 139 new in 2.5 / v1.3c */
   "nj",                   /* 199 140 new in 2.5 / v1.3c */
   "\\v{A}",               /* 199 141 */
   "\\v{a}",               /* 199 142 */
   "\\v{I}",               /* 199 143 */
   "\\v{\\i}",             /* 199 144 */
   "\\v{O}",               /* 199 145 */
   "\\v{o}",               /* 199 146 */
   "\\v{U}",               /* 199 147 */
   "\\v{u}",               /* 199 148 */
   "",                     /* 199 149 \\={\\\"U} (not v1.3c) */
   "",                     /* 199 150 \\={\\\"u} (not v1.3c) */
   "",                     /* 199 151 \\\'{\\\"U} (not v1.3c) */
   "",                     /* 199 152 \\\'{\\\"u} (not v1.3c) */
   "",                     /* 199 153 \\v{\\\"U} (not v1.3c) */
   "",                     /* 199 154 \\v{\\\"u} (not v1.3c) */
   "",                     /* 199 155 \\`{\\\"U} (not v1.3c) */
   "",                     /* 199 156 \\`{\\\"u} (not v1.3c) */
   "",                     /* 199 157 turned e (not v1.3c) */
   "",                     /* 199 158 \\={\\\"A} (not v1.3c) */
   "",                     /* 199 159 \\={\\\"a} (not v1.3c) */
   "",                     /* 199 160 \\={\\.A} (not v1.3c) */
   "",                     /* 199 161 \\={\\.a} (not v1.3c) */
   "\\={\\AE}",            /* 199 162 */
   "\\={\\ae}",            /* 199 163 */
   "",                     /* 199 164 G-quer (not v1.3c) */
   "",                     /* 199 165 g-quer (not v1.3c) */
   "\\v{G}",               /* 199 166 */
   "\\v{g}",               /* 199 167 */
   "\\v{K}",               /* 199 168 */
   "\\v{k}",               /* 199 169 */
   "\\k{O}",               /* 199 170 */
   "\\k{o}",               /* 199 171 */
   "",                     /* 199 172 \\k{\\=O} (not v1.3c) */
   "",                     /* 199 173 \\k{\\=o} (not v1.3c) */
   "",                     /* 199 174 EZH with \\v (not v1.3c) */
   "",                     /* 199 175 ezh with \\v (not v1.3c) */
   "\\v{\\j}",             /* 199 176 */
   "",                     /* 199 177 DZ (not v1.3c) */
   "",                     /* 199 178 Dz (not v1.3c) */
   "",                     /* 199 179 dz (not v1.3c) */
   "\\\'G",                /* 199 180 */
   "\\\'g",                /* 199 181 */
   "",                     /* 199 182 HWAIR (not v1.3c) */
   "",                     /* 199 183 WYNN (not v1.3c) */
   "",                     /* 199 184 \\`N (not v1.3c) */
   "",                     /* 199 185 \\`n (not v1.3c) */
   "",                     /* 199 186 \\\'{\k{A}} (not v1.3c) */
   "",                     /* 199 187 \\\'{\k{a}} (not v1.3c) */
   "",                     /* 199 188 \\\'{\\AE} (not v1.3c) */
   "",                     /* 199 189 \\\'{\\ae} (not v1.3c) */
   "",                     /* 199 190 \\\'{\\O} (not v1.3c) */
   "",                     /* 199 191 \\\'{\\o} (not v1.3c) */
   NULL };

#define OHNINETYNINE          199

char *get_ohninetynine(unsigned char c)
{
  if (c >= UTF_BLOCK_BEGIN && c <= UTF_BLOCK_END)  /* 128 ... 191 */
       return ohninetynine[c-UTF_BLOCK_BEGIN];
  else return "";
}


static char *thnullnull[] = {
   "",                     /* 200 128 A with double grave (not v1.3c) */
   "",                     /* 200 129 a with double grave (not v1.3c) */
   "",                     /* 200 130 A with inverted breve (not v1.3c) */
   "",                     /* 200 131 a with inverted breve (not v1.3c) */
   "",                     /* 200 132 E with double grave (not v1.3c) */
   "",                     /* 200 133 e with double grave (not v1.3c) */
   "",                     /* 200 134 E with inverted breve (not v1.3c) */
   "",                     /* 200 135 e with inverted breve (not v1.3c) */
   "",                     /* 200 136 I with double grave (not v1.3c) */
   "",                     /* 200 137 i with double grave (not v1.3c) */
   "",                     /* 200 138 I with inverted breve (not v1.3c) */
   "",                     /* 200 139 i with inverted breve (not v1.3c) */
   "",                     /* 200 140 O with double grave (not v1.3c) */
   "",                     /* 200 141 o with double grave (not v1.3c) */
   "",                     /* 200 142 O with inverted breve (not v1.3c) */
   "",                     /* 200 143 o with inverted breve (not v1.3c) */
   "",                     /* 200 144 R with double grave (not v1.3c) */
   "",                     /* 200 145 r with double grave (not v1.3c) */
   "",                     /* 200 146 R with inverted breve (not v1.3c) */
   "",                     /* 200 147 r with inverted breve (not v1.3c) */
   "",                     /* 200 148 U with double grave (not v1.3c) */
   "",                     /* 200 149 u with double grave (not v1.3c) */
   "",                     /* 200 150 U with inverted breve (not v1.3c) */
   "",                     /* 200 151 u with inverted breve (not v1.3c) */
   "\\textcommabelow{S}",  /* 200 152 */
   "\\textcommabelow{s}",  /* 200 153 */
   "\\textcommabelow{T}",  /* 200 154 */
   "\\textcommabelow{t}",  /* 200 155 */
   "",                     /* 200 156 YOGH (not v1.3c) */
   "",                     /* 200 157 yogh (not v1.3c) */
   "",                     /* 200 158 \\v{H} (not v1.3c) */
   "",                     /* 200 159 \\v{h} (not v1.3c) */
   "",                     /* 200 160 N with long right leg (not v1.3c) */
   "",                     /* 200 161 d with curl (not v1.3c) */
   "",                     /* 200 162 OU (not v1.3c) */
   "",                     /* 200 163 ou (not v1.3c) */
   "",                     /* 200 164 Z with hook (not v1.3c) */
   "",                     /* 200 165 z with hook (not v1.3c) */
   "",                     /* 200 166 \\.A (not v1.3c) */
   "",                     /* 200 167 \\.a (not v1.3c) */
   "",                     /* 200 168 \\c{E} (not v1.3c) */
   "",                     /* 200 169 \\c{e} (not v1.3c) */
   "",                     /* 200 170 \\={\\\"O} (not v1.3c) */
   "",                     /* 200 171 \\={\\\"o} (not v1.3c) */
   "",                     /* 200 172 \\={\\~O} (not v1.3c) */
   "",                     /* 200 173 \\={\\~o} (not v1.3c) */
   "",                     /* 200 174 \\.O (not v1.3c) */
   "",                     /* 200 175 \\.o (not v1.3c) */
   "",                     /* 200 176 [\\={\\.O}] (not v1.3c) */
   "",                     /* 200 177 [\\={\\.o}] (not v1.3c) */
   "\\=Y",                 /* 200 178 */
   "\\=y",                 /* 200 179 */
   "",                     /* 200 180 l with curl (not v1.3c) */
   "",                     /* 200 181 n with curl (not v1.3c) */
   "",                     /* 200 182 t with curl (not v1.3c) */
   "{\\j}",                /* 200 183 */
   "",                     /* 200 184 db digraph (not v1.3c) */
   "",                     /* 200 185 qp digraph (not v1.3c) */
   "",                     /* 200 186 A with diagonal stroke (not v1.3c) */
   "",                     /* 200 187 C with diagonal stroke (not v1.3c) */
   "",                     /* 200 188 {\\textcentoldstyle} (not v1.3c) */
   "",                     /* 200 189 L-quer (not v1.3c) */
   "",                     /* 200 190 T with diagonal stroke (not v1.3c) */
   "",                     /* 200 191 s with swash tail (not v1.3c) */
   NULL };

#define THNULLNULL            200

char *get_thnullnull(unsigned char c)
{
  if (c >= UTF_BLOCK_BEGIN && c <= UTF_BLOCK_END)  /* 128 ... 191 */
       return thnullnull[c-UTF_BLOCK_BEGIN];
  else return "";
}


#define THNULLTHREE           203

int ins_thnullthree(char in[], unsigned char c, int pos)
{
      if (c == 134)        /* 203 134 \\^{} */
      pos = insposmaxstr(in, "{\\textasciicircum}", pos, MAXLEN-1);
      else
      if (c == 135)        /* 203 135 \\v{} */
      pos = insposmaxstr(in, "{\\textasciicaron}", pos, MAXLEN-1);
      else
      if (c == 152)        /* 203 152 \\u{} */
      pos = insposmaxstr(in, "{\\textasciibreve}", pos, MAXLEN-1);
      else
      if (c == 153)        /* 203 153 \\.{} (\text... yet undefined) */
      pos = insposmaxstr(in, "{\\textasciidotaccent}", pos, MAXLEN-1);
      else
      if (c == 155)        /* 203 155 \\k{} (\text... yet undefined) */
      pos = insposmaxstr(in, "{\\textasciiogonek}", pos, MAXLEN-1);
      else
      if (c == 156)        /* 203 156 \\~{} */
      pos = insposmaxstr(in, "{\\textasciitilde}", pos, MAXLEN-1);
      else
      if (c == 157)        /* 203 157 \\H{} */
      pos = insposmaxstr(in, "{\\textacutedbl}", pos, MAXLEN-1);

  return pos;
}


static char *thtwentyfive_oheightyfour[] = {
   "",                        /* 225 184 128 A with ring below (not v1.3c) */
   "",                        /* 225 184 129 a with ring below (not v1.3c) */
   "\\.B",                    /* 225 184 130 */
   "\\.b",                    /* 225 184 131 */
   "",                        /* 225 184 132 \\d{B} (not v1.3c) */
   "",                        /* 225 184 133 \\d{b} (not v1.3c) */
   "",                        /* 225 184 134 \\b{B} (not v1.3c) */
   "",                        /* 225 184 135 \\b{b} (not v1.3c) */
   "",                        /* 225 184 136 \\c{\\\'C} (not v1.3c) */
   "",                        /* 225 184 137 \\c{\\\'c} (not v1.3c) */
   "",                        /* 225 184 138 \\.D (not v1.3c) */
   "",                        /* 225 184 139 \\.d (not v1.3c) */
   "",                        /* 225 184 140 \\d{D} (not v1.3c) */
   "\\d{d}",                  /* 225 184 141 */
   "",                        /* 225 184 142 \\b{D} (not v1.3c) */
   "",                        /* 225 184 143 \\b{d} (not v1.3c) */
   "",                        /* 225 184 144 \\textcommabelow{D} (not v1.3c) */
   "",                        /* 225 184 145 \\textcommabelow{d} (not v1.3c) */
   "",                        /* 225 184 146 D with circumflex below (not v1.3c) */
   "",                        /* 225 184 147 d with circumflex below (not v1.3c) */
   "",                        /* 225 184 148 [\\`{\\=E}] (not v1.3c) */
   "",                        /* 225 184 149 [\\`{\\=e}] (not v1.3c) */
   "",                        /* 225 184 150 [\\\'{\\=E}] (not v1.3c) */
   "",                        /* 225 184 151 [\\\'{\\=e}] (not v1.3c) */
   "",                        /* 225 184 152 E with circumflex below (not v1.3c) */
   "",                        /* 225 184 153 e with circumflex below (not v1.3c) */
   "",                        /* 225 184 154 E with tilde below (not v1.3c) */
   "",                        /* 225 184 155 e with tilde below (not v1.3c) */
   "",                        /* 225 184 156 [\\u{\\c{E}}] (not v1.3c) */
   "",                        /* 225 184 157 [\\u{\\c{e}}] (not v1.3c) */
   "\\.F",                    /* 225 184 158 new in 2.4 / v1.2n */
   "\\.f",                    /* 225 184 159 new in 2.4 / v1.2n */
   "\\=G",                    /* 225 184 160 */
   "\\=g",                    /* 225 184 161 */
   "",                        /* 225 184 162 \\.H (not v1.3c) */
   "",                        /* 225 184 163 \\.h (not v1.3c) */
   "",                        /* 225 184 164 \\d{H} (not v1.3c) */
   "\\d{h}",                  /* 225 184 165 */
   "",                        /* 225 184 166 \\\"H (not v1.3c) */
   "",                        /* 225 184 167 \\\"h (not v1.3c) */
   "",                        /* 225 184 168 \\c{H} [?] (not v1.3c) */
   "",                        /* 225 184 169 \\c{h} [?] (not v1.3c) */
   "",                        /* 225 184 170 H with breve below (not v1.3c) */
   "",                        /* 225 184 171 h with breve below (not v1.3c) */
   "",                        /* 225 184 172 I with tilde below (not v1.3c) */
   "",                        /* 225 184 173 i with tilde below (not v1.3c) */
   "",                        /* 225 184 174 \\\'{\\\"I} (not v1.3c) */
   "",                        /* 225 184 175 \\\'{\\\"{\\i}} (not v1.3c) */
   "\\\'K",                   /* 225 184 176 new in 2.4 / v1.2n */
   "\\\'k",                   /* 225 184 177 new in 2.4 / v1.2n */
   "",                        /* 225 184 178 \\d{K} (not v1.3c) */
   "",                        /* 225 184 179 \\d{k} (not v1.3c) */
   "",                        /* 225 184 180 \\b{K} (not v1.3c) */
   "",                        /* 225 184 181 \\b{k} (not v1.3c) */
   "",                        /* 225 184 182 \\d{L} (not v1.3c) */
   "\\d{l}",                  /* 225 184 183 */
   "",                        /* 225 184 184 \\d{\\=L} (not v1.3c) */
   "",                        /* 225 184 185 \\d{\\=l} (not v1.3c) */
   "",                        /* 225 184 186 \\b{L} (not v1.3c) */
   "",                        /* 225 184 187 \\b{l} (not v1.3c) */
   "",                        /* 225 184 188 L with circumflex below (not v1.3c) */
   "",                        /* 225 184 189 l with circumflex below (not v1.3c) */
   "",                        /* 225 184 190 \\\'M (not v1.3c) */
   "",                        /* 225 184 191 \\\'m (not v1.3c) */
   NULL };

#define THTWENTYFIVE             225
#define THTWENTYFIVE_OHEIGHTYFOUR    184

char *get_thtwentyfive_oheightyfour(unsigned char c)
{
  if (c >= UTF_BLOCK_BEGIN && c <= UTF_BLOCK_END)  /* 128 ... 191 */
       return thtwentyfive_oheightyfour[c-UTF_BLOCK_BEGIN];
  else return "";
}


static char *thtwentyfive_oheightyfive[] = {
   "",                        /* 225 185 128 \\.M (not v1.3c) */
   "",                        /* 225 185 129 \\.m (not v1.3c) */
   "",                        /* 225 185 130 \\d{M} (not v1.3c) */
   "\\d{m}",                  /* 225 185 131 */
   "",                        /* 225 185 132 \\.N (not v1.3c) */
   "\\.n",                    /* 225 185 133 */
   "",                        /* 225 185 134 \\d{N} (not v1.3c) */
   "\\d{n}",                  /* 225 185 135 */
   "",                        /* 225 185 136 N with line below (not v1.3c) */
   "",                        /* 225 185 137 n with line below (not v1.3c) */
   "",                        /* 225 185 138 N with circumflex below (not v1.3c) */
   "",                        /* 225 185 139 n with circumflex below (not v1.3c) */
   "",                        /* 225 185 140 \\\'{\\~O} (not v1.3c) */
   "",                        /* 225 185 141 \\\'{\\~o} (not v1.3c) */
   "",                        /* 225 185 142 \\\"{\\~O} (not v1.3c) */
   "",                        /* 225 185 143 \\\"{\\~o} (not v1.3c) */
   "",                        /* 225 185 144 [\\`{\\=O}] (not v1.3c) */
   "",                        /* 225 185 145 [\\`{\\=o}] (not v1.3c) */
   "",                        /* 225 185 146 [\\\'{\\=O}] (not v1.3c) */
   "",                        /* 225 185 147 [\\\'{\\=o}] (not v1.3c) */
   "",                        /* 225 185 148 \\\'P (not v1.3c) */
   "",                        /* 225 185 149 \\\'p (not v1.3c) */
   "",                        /* 225 185 150 \\.P (not v1.3c) */
   "",                        /* 225 185 151 \\.p (not v1.3c) */
   "",                        /* 225 185 152 \\.R (not v1.3c) */
   "",                        /* 225 185 153 \\.r (not v1.3c) */
   "",                        /* 225 185 154 \\d{R} (not v1.3c) */
   "\\d{r}",                  /* 225 185 155 */
   "",                        /* 225 185 156 \\d{\\=R} (not v1.3c) */
   "",                        /* 225 185 157 \\d{\\=r} (not v1.3c) */
   "",                        /* 225 185 158 R with line below (not v1.3c) */
   "",                        /* 225 185 159 r with line below (not v1.3c) */
   "",                        /* 225 185 160 \\.S (not v1.3c) */
   "",                        /* 225 185 161 \\.s (not v1.3c) */
   "",                        /* 225 185 162 \\d{S} (not v1.3c) */
   "\\d{s}",                  /* 225 185 163 */
   "",                        /* 225 185 164 \\.{\\\'S} (not v1.3c) */
   "",                        /* 225 185 165 \\.{\\\'s} (not v1.3c) */
   "",                        /* 225 185 166 \\.{\\v{S}} (not v1.3c) */
   "",                        /* 225 185 167 \\.{\\v{s}} (not v1.3c) */
   "",                        /* 225 185 168 \\d{\\.S} (not v1.3c) */
   "",                        /* 225 185 169 \\d{\\.s} (not v1.3c) */
   "",                        /* 225 185 170 \\.T (not v1.3c) */
   "",                        /* 225 185 171 \\.t (not v1.3c) */
   "",                        /* 225 185 172 \\d{T} (not v1.3c) */
   "\\d{t}",                  /* 225 185 173 */
   "",                        /* 225 185 174 T with line below (not v1.3c) */
   "",                        /* 225 185 175 t with line below (not v1.3c) */
   "",                        /* 225 185 176 T with circumfelx below (not v1.3c) */
   "",                        /* 225 185 177 t with circumfelx below (not v1.3c) */
   "",                        /* 225 185 178 U with dieresis below (not v1.3c) */
   "",                        /* 225 185 179 u with dieresis below (not v1.3c) */
   "",                        /* 225 185 180 U with tilde below (not v1.3c) */
   "",                        /* 225 185 181 u with tilde below (not v1.3c) */
   "",                        /* 225 185 182 U with circumflex below (not v1.3c) */
   "",                        /* 225 185 183 u with circumflex below (not v1.3c) */
   "",                        /* 225 185 184 [\\\'{\\~U}] (not v1.3c) */
   "",                        /* 225 185 185 [\\\'{\\~u}] (not v1.3c) */
   "",                        /* 225 185 186 [\\\"{\\=U}] (not v1.3c) */
   "",                        /* 225 185 187 [\\\"{\\=u}] (not v1.3c) */
   "",                        /* 225 185 188 \\~V (not v1.3c) */
   "",                        /* 225 185 189 \\~v (not v1.3c) */
   "",                        /* 225 185 190 \\d{V} (not v1.3c) */
   "",                        /* 225 185 191 \\d{v} (not v1.3c) */
   NULL };

#define THTWENTYFIVE_OHEIGHTYFIVE    185

char *get_thtwentyfive_oheightyfive(unsigned char c)
{
  if (c >= UTF_BLOCK_BEGIN && c <= UTF_BLOCK_END)  /* 128 ... 191 */
       return thtwentyfive_oheightyfive[c-UTF_BLOCK_BEGIN];
  else return "";
}


static char *thtwentyfive_oheightysix[] = {  /* new in 2.4 */
   "",                        /* 225 186 128 \\`W (not v1.3c) */
   "",                        /* 225 186 129 \\`w (not v1.3c) */
   "",                        /* 225 186 130 \\\'W (not v1.3c) */
   "",                        /* 225 186 131 \\\'w (not v1.3c) */
   "",                        /* 225 186 132 \\\"W (not v1.3c) */
   "",                        /* 225 186 133 \\\"w (not v1.3c) */
   "",                        /* 225 186 134 \\.W (not v1.3c) */
   "",                        /* 225 186 135 \\.w (not v1.3c) */
   "",                        /* 225 186 136 \\d{W} (not v1.3c) */
   "",                        /* 225 186 137 \\d{w} (not v1.3c) */
   "",                        /* 225 186 138 \\.X (not v1.3c) */
   "",                        /* 225 186 139 \\.x (not v1.3c) */
   "",                        /* 225 186 140 \\\"X (not v1.3c) */
   "",                        /* 225 186 141 \\\"x (not v1.3c) */
   "\\.Y",                    /* 225 186 142 new in 2.4 / v1.2n */
   "\\.y",                    /* 225 186 143 new in 2.4 / v1.2n */
   "\\^Z",                    /* 225 186 144 new in 2.4 / v1.2n */
   "\\^z",                    /* 225 186 145 new in 2.4 / v1.2n */
   "",                        /* 225 186 146 \\d{Z} (not v1.3c) */
   "",                        /* 225 186 147 \\d{z} (not v1.3c) */
   "",                        /* 225 186 148 Z with line below (not v1.3c) */
   "",                        /* 225 186 149 z with line below (not v1.3c) */
   "",                        /* 225 186 150 h with line below (not v1.3c) */
   "",                        /* 225 186 151 \\\"t (not v1.3c) */
   "",                        /* 225 186 152 \\r{w} (not v1.3c) */
   "",                        /* 225 186 153 \\r{y} (not v1.3c) */
   "",                        /* 225 186 154 a with right half ring (not v1.3c) */
   "",                        /* 225 186 155 long s with dot above (not v1.3c) */
   "",                        /* 225 186 156 long s with diagonal stroke (not v1.3c) */
   "",                        /* 225 186 157 long s with high stroke (not v1.3c) */
   "{\\SS}",                  /* 225 186 158 */
   "",                        /* 225 186 159 \\delta (not v1.3c) */
   "",                        /* 225 186 160 \\d{A} (not v1.3c) */
   "",                        /* 225 186 161 \\d{a} (not v1.3c) */
   "",                        /* 225 186 162 A with hook above (not v1.3c) */
   "",                        /* 225 186 163 a with hook above (not v1.3c) */
   "",                        /* 225 186 164 A with circumflex-acute (not v1.3c) */
   "",                        /* 225 186 165 a with circumflex-acute (not v1.3c) */
   "",                        /* 225 186 166 A with circumflex-grave (not v1.3c) */
   "",                        /* 225 186 167 a with circumflex-grave (not v1.3c) */
   "",                        /* 225 186 168 A with circumflex-hook (not v1.3c) */
   "",                        /* 225 186 169 a with circumflex-hook (not v1.3c) */
   "",                        /* 225 186 170 \\~{\\^A} (not v1.3c) */
   "",                        /* 225 186 171 \\~{\\^a} (not v1.3c) */
   "",                        /* 225 186 172 \\d{\\^A} (not v1.3c) */
   "",                        /* 225 186 173 \\d{\\^a} (not v1.3c) */
   "",                        /* 225 186 174 A with breve-acute (not v1.3c) */
   "",                        /* 225 186 175 a with breve-acute (not v1.3c) */
   "",                        /* 225 186 176 A with breve-grave (not v1.3c) */
   "",                        /* 225 186 177 a with breve-grave (not v1.3c) */
   "",                        /* 225 186 178 A with breve-hook (not v1.3c) */
   "",                        /* 225 186 179 a with breve-hook (not v1.3c) */
   "",                        /* 225 186 180 A with breve-tilde (not v1.3c) */
   "",                        /* 225 186 181 a with breve-tilde (not v1.3c) */
   "",                        /* 225 186 182 \\d{\\u{A}} (not v1.3c) */
   "",                        /* 225 186 183 \\d{\\u{a}} (not v1.3c) */
   "",                        /* 225 186 184 \\d{E} (not v1.3c) */
   "",                        /* 225 186 185 \\d{e} (not v1.3c) */
   "",                        /* 225 186 186 E with hook above (not v1.3c) */
   "",                        /* 225 186 187 e with hook above (not v1.3c) */
   "",                        /* 225 186 188 \\~E (not v1.3c) */
   "",                        /* 225 186 189 \\~e (not v1.3c) */
   "",                        /* 225 186 190 E with circumflex-acute (not v1.3c) */
   "",                        /* 225 186 191 e with circumflex-acute (not v1.3c) */
   NULL };

#define THTWENTYFIVE_OHEIGHTYSIX    186

char *get_thtwentyfive_oheightysix(unsigned char c)
{
  if (c >= UTF_BLOCK_BEGIN && c <= UTF_BLOCK_END)  /* 128 ... 191 */
       return thtwentyfive_oheightysix[c-UTF_BLOCK_BEGIN];
  else return "";
}


static char *thtwentysix_ohtwentyeight[] = {
   "",                        /* 226 128 128 en quad (not v1.3c) */
   "",                        /* 226 128 129 em quad (not v1.3c) */
   "",                        /* 226 128 130 en space (not v1.3c) */
   "",                        /* 226 128 131 em space (not v1.3c) */
   "",                        /* 226 128 132 three-per-em space (not v1.3c) */
   "",                        /* 226 128 133 four-per-em space (not v1.3c) */
   "",                        /* 226 128 134 six-per-em space (not v1.3c) */
   "",                        /* 226 128 135 figure space (not v1.3c) */
   "",                        /* 226 128 136 punctuation space (not v1.3c) */
   "",                        /* 226 128 137 thin space (not v1.3c) */
   "",                        /* 226 128 138 hair space (not v1.3c) */
   "",                        /* 226 128 139 zero width space (not v1.3c) */
   "{\\textcompwordmark}",    /* 226 128 140 (LaTeX-Example: b\u{226-128-140}g) */
   "",                        /* 226 128 141 zero width joiner (not v1.3c) */
   "",                        /* 226 128 142 left-to-right mark (not v1.3c) */
   "",                        /* 226 128 143 right-to-left mark (not v1.3c) */
   "-",                       /* 226 128 144 */
   "\\mbox{-}",               /* 226 128 145 - */
   "{\\textendash}",          /* 226 128 146 -- */
   "{\\textendash}",          /* 226 128 147 -- */
   "{\\textemdash}",          /* 226 128 148 --- */
   "{\\textemdash}",          /* 226 128 149 --- */
   "{\\textbardbl}",          /* 226 128 150 */
   "",                        /* 226 128 151 double low line (not v1.3c) */
   "{\\textquoteleft}",       /* 226 128 152 ` */
   "{\\textquoteright}",      /* 226 128 153 \' */
   "{\\quotesinglbase}",      /* 226 128 154 , */
   "",                        /* 226 128 155 single high-reversed-9 quotation mark (not v1.3c) */
   "{\\textquotedblleft}",    /* 226 128 156 `` */
   "{\\textquotedblright}",   /* 226 128 157 \'\' */
   "{\\quotedblbase}",        /* 226 128 158 ,, */
   "",                        /* 226 128 159 double high-reversed-9 quotation mark (not v1.3c) */
   "{\\textdagger}",          /* 226 128 160 {\\dag} */
   "{\\textdaggerdbl}",       /* 226 128 161 {\\ddag} */
   "{\\textbullet}",          /* 226 128 162 */
   "",                        /* 226 128 163 triangular bullet (not v1.3c) */
   "",                        /* 226 128 164 one dot leader (not v1.3c) */
   "",                        /* 226 128 165 two dot leader (not v1.3c) */
   "{\\textellipsis}",        /* 226 128 166 {\\dots} */
   "",                        /* 226 128 167 hyphenation point (not v1.3c) */
   "",                        /* 226 128 168 line separator (not v1.3c) */
   "",                        /* 226 128 169 paragraph separator (not v1.3c) */
   "",                        /* 226 128 170 left-to-right embedding (not v1.3c) */
   "",                        /* 226 128 171 right-to-left embedding (not v1.3c) */
   "",                        /* 226 128 172 pop directional formatting (not v1.3c) */
   "",                        /* 226 128 173 left-to-right override (not v1.3c) */
   "",                        /* 226 128 174 right-to-left override (not v1.3c) */
   "",                        /* 226 128 175 narrow no-break space (not v1.3c) */
   "{\\textperthousand}",     /* 226 128 176 */
   "{\\textpertenthousand}",  /* 226 128 177 */
   "",                        /* 226 128 178 prime (not v1.3c) */
   "",                        /* 226 128 179 double prime (not v1.3c) */
   "",                        /* 226 128 180 triple prime (not v1.3c) */
   "",                        /* 226 128 181 reversed prime (not v1.3c) */
   "",                        /* 226 128 182 reversed double prime (not v1.3c) */
   "",                        /* 226 128 183 reversed triple prime (not v1.3c) */
   "",                        /* 226 128 184 caret (not v1.3c) */
   "{\\guilsinglleft}",       /* 226 128 185 */
   "{\\guilsinglright}",      /* 226 128 186 */
   "{\\textreferencemark}",   /* 226 128 187 */
   "",                        /* 226 128 188 "!!" (double exclamation mark) (not v1.3c) */
   "{\\textinterrobang}",     /* 226 128 189 */
   "",                        /* 226 128 190 overline (not v1.3c) */
   "",                        /* 226 128 191 undertie (not v1.3c) */
   NULL };

#define THTWENTYSIX              226
#define THTWENTYSIX_OHTWENTYEIGHT    128

char *get_thtwentysix_ohtwentyeight(unsigned char c)
{
  if (c >= UTF_BLOCK_BEGIN && c <= UTF_BLOCK_END)  /* 128 ... 191 */
       return thtwentysix_ohtwentyeight[c-UTF_BLOCK_BEGIN];
  else return "";
}


static char *thtwentysix_ohthirty[] = {
   "",                        /* 226 130 128 _0 (not v1.3c) */
   "",                        /* 226 130 129 _1 (not v1.3c) */
   "",                        /* 226 130 130 _2 (not v1.3c) */
   "",                        /* 226 130 131 _3 (not v1.3c) */
   "",                        /* 226 130 132 _4 (not v1.3c) */
   "",                        /* 226 130 133 _5 (not v1.3c) */
   "",                        /* 226 130 134 _6 (not v1.3c) */
   "",                        /* 226 130 135 _7 (not v1.3c) */
   "",                        /* 226 130 136 _8 (not v1.3c) */
   "",                        /* 226 130 137 _9 (not v1.3c) */
   "",                        /* 226 130 138 _+ (not v1.3c) */
   "",                        /* 226 130 139 _- (not v1.3c) */
   "",                        /* 226 130 140 _= (not v1.3c) */
   "",                        /* 226 130 141 _( (not v1.3c) */
   "",                        /* 226 130 142 _) (not v1.3c) */
   "",                        /* 226 130 143 [undefined] (not v1.3c) */ 
   "",                        /* 226 130 144 _a (not v1.3c) */
   "",                        /* 226 130 145 _e (not v1.3c) */
   "",                        /* 226 130 146 _o (not v1.3c) */
   "",                        /* 226 130 147 _x (not v1.3c) */
   "",                        /* 226 130 148 _{schwa} (not v1.3c) */
   "",                        /* 226 130 149 _h (not v1.3c) */
   "",                        /* 226 130 150 _k (not v1.3c) */
   "",                        /* 226 130 151 _l (not v1.3c) */
   "",                        /* 226 130 152 _m (not v1.3c) */
   "",                        /* 226 130 153 _n (not v1.3c) */
   "",                        /* 226 130 154 _p (not v1.3c) */
   "",                        /* 226 130 155 _s (not v1.3c) */
   "",                        /* 226 130 156 _t (not v1.3c) */
   "",                        /* 226 130 157 [undefined] (not v1.3c) */
   "",                        /* 226 130 158 [undefined] (not v1.3c) */
   "",                        /* 226 130 159 [undefined] (not v1.3c) */
   "",                        /* 226 130 160 euro-currency sign (not v1.3c) */
   "{\\textcolonmonetary}",   /* 226 130 161 */
   "",                        /* 226 130 162 cruzeiro sign (not v1.3c) */
   "",                        /* 226 130 163 french franc sign (not v1.3c) */
   "{\\textlira}",            /* 226 130 164 */
   "",                        /* 226 130 165 mill sign (not v1.3c) */
   "{\\textnaira}",           /* 226 130 166 */
   "",                        /* 226 130 167 peseta sign (not v1.3c) */
   "",                        /* 226 130 168 rupee sign (not v1.3c) */
   "{\\textwon}",             /* 226 130 169 */
   "",                        /* 226 130 170 new sheqel sign (not v1.3c) */
   "{\\textdong}",            /* 226 130 171 */
   "{\\texteuro}",            /* 226 130 172 */
   "",                        /* 226 130 173 kip sign (not v1.3c) */
   "",                        /* 226 130 174 tugrik sign (not v1.3c) */
   "",                        /* 226 130 175 drachma sign (not v1.3c) */
   "",                        /* 226 130 176 german penny sign (not v1.3c) */
   "{\\textpeso}",            /* 226 130 177 */
   "",                        /* 226 130 178 guarani sign (not v1.3c) */
   "",                        /* 226 130 179 austral sign (not v1.3c) */
   "",                        /* 226 130 180 hryvnia sign (not v1.3c) */
   "",                        /* 226 130 181 cedi sign (not v1.3c) */
   "",                        /* 226 130 182 livre tournois sign (not v1.3c) */
   "",                        /* 226 130 183 spesmilo sign (not v1.3c) */
   "",                        /* 226 130 184 tenge sign (not v1.3c) */
   "",                        /* 226 130 185 indian rupee sign (not v1.3c) */
   "",                        /* 226 130 186 turkish lira sign (not v1.3c) */
   "",                        /* 226 130 187 nordic mark sign (not v1.3c) */
   "",                        /* 226 130 188 manat sign (not v1.3c) */
   "",                        /* 226 130 189 ruble sign (not v1.3c) */
   "",                        /* 226 130 190 lari sign (not v1.3c) */
   "",                        /* 226 130 191 bitcoin sign (not v1.3c) */
   NULL };

#define THTWENTYSIX_OHTHIRTY         130

char *get_thtwentysix_ohthirty(unsigned char c)
{
  if (c >= UTF_BLOCK_BEGIN && c <= UTF_BLOCK_END)  /* 128 ... 191 */
       return thtwentysix_ohthirty[c-UTF_BLOCK_BEGIN];
  else return "";
}


int ins_rest_thtwentysix(char in[], unsigned char a, unsigned char b, int pos)
{
      if (a == 129 && b == 132)        /* 226 129 132 not exactly "/" */
      pos = insposmaxstr(in, "{\\textfractionsolidus}", pos, MAXLEN-1);
      else
      if (a == 129 && b == 142)        /* 226 129 142 */
      pos = insposmaxstr(in, "{\\textasteriskcentered}", pos, MAXLEN-1);
      else
      if (a == 129 && b == 146)        /* 226 129 146 \\% */
      pos = insposmaxstr(in, "{\\textdiscount}", pos, MAXLEN-1);
      else
      if (a == 132 && b == 131)        /* 226 132 131 not exactly {\\textdegree C} */
      pos = insposmaxstr(in, "{\\textcelsius}", pos, MAXLEN-1);
      else
      if (a == 132 && b == 150)        /* 226 132 150 */
      pos = insposmaxstr(in, "{\\textnumero}", pos, MAXLEN-1);
      else
      if (a == 132 && b == 151)        /* 226 132 151 */
      pos = insposmaxstr(in, "{\\textcircledP}", pos, MAXLEN-1);
      else
      if (a == 132 && b == 158)        /* 226 132 158 */
      pos = insposmaxstr(in, "{\\textrecipe}", pos, MAXLEN-1);
      else
      if (a == 132 && b == 160)        /* 226 132 160 */
      pos = insposmaxstr(in, "{\\textservicemark}", pos, MAXLEN-1);
      else
      if (a == 132 && b == 162)        /* 226 132 162 */
      pos = insposmaxstr(in, "{\\texttrademark}", pos, MAXLEN-1);
      else
      if (a == 132 && b == 166)        /* 226 132 166 {\\Omega} */
      pos = insposmaxstr(in, "{\\textohm}", pos, MAXLEN-1);
      else
      if (a == 132 && b == 167)        /* 226 132 167 */
      pos = insposmaxstr(in, "{\\textmho}", pos, MAXLEN-1);
      else
      if (a == 132 && b == 174)        /* 226 132 174 */
      pos = insposmaxstr(in, "{\\textestimated}", pos, MAXLEN-1);
      else
      if (a == 134 && b == 144)        /* 226 134 144 {\\leftarrow} */
      pos = insposmaxstr(in, "{\\textleftarrow}", pos, MAXLEN-1);
      else
      if (a == 134 && b == 145)        /* 226 134 145 {\\uparrow} */
      pos = insposmaxstr(in, "{\\textuparrow}", pos, MAXLEN-1);
      else
      if (a == 134 && b == 146)        /* 226 134 146 {\\rightarrow} */
      pos = insposmaxstr(in, "{\\textrightarrow}", pos, MAXLEN-1);
      else
      if (a == 134 && b == 147)        /* 226 134 147 {\\downarrow} */
      pos = insposmaxstr(in, "{\\textdownarrow}", pos, MAXLEN-1);
      else
      if (a == 140 && b == 169)        /* 226 140 169 */
      pos = insposmaxstr(in, "{\\textlangle}", pos, MAXLEN-1);
      else
      if (a == 140 && b == 170)        /* 226 140 170 */
      pos = insposmaxstr(in, "{\\textrangle}", pos, MAXLEN-1);
      else
      if (a == 144 && b == 162)        /* 226 144 162 */
      pos = insposmaxstr(in, "{\\textblank}", pos, MAXLEN-1);
      else
      if (a == 144 && b == 163)        /* 226 144 163 */
      pos = insposmaxstr(in, "{\\textvisiblespace}", pos, MAXLEN-1);
      else
      if (a == 151 && b == 166)        /* 226 151 166 */
      pos = insposmaxstr(in, "{\\textopenbullet}", pos, MAXLEN-1);
      else
      if (a == 151 && b == 175)        /* 226 151 175 */
      pos = insposmaxstr(in, "{\\textbigcircle}", pos, MAXLEN-1);
      else
      if (a == 153 && b == 170)        /* 226 153 170 */
      pos = insposmaxstr(in, "{\\textmusicalnote}", pos, MAXLEN-1);
      else
      if (a == 159 && b == 168)        /* 226 159 168 */
      pos = insposmaxstr(in, "{\\textlangle}", pos, MAXLEN-1);
      else
      if (a == 159 && b == 169)        /* 226 159 169 */
      pos = insposmaxstr(in, "{\\textrangle}", pos, MAXLEN-1);

  return pos;
}


#define THTWENTYSEVEN            227


#define THTHIRTYNINE   239

int ins_ththirtynine(char in[], unsigned char b, unsigned char c, int pos)
{
      if (b == 187  && c == 191)     /* 239 187 191 */
      pos = insposmaxstr(in, "{\\ifhmode\\nobreak\\fi}", pos, MAXLEN-1);
      else
      if (b == 172  && c == 128)     /* 239 172 128 */
      pos = insposmaxstr(in, "{ff}", pos, MAXLEN-1);
      else
      if (b == 172  && c == 129)     /* 239 172 129 */
      pos = insposmaxstr(in, "{fi}", pos, MAXLEN-1);
      else
      if (b == 172  && c == 130)     /* 239 172 130 */
      pos = insposmaxstr(in, "{fl}", pos, MAXLEN-1);
      else
      if (b == 172  && c == 131)     /* 239 172 131 */
      pos = insposmaxstr(in, "{ffi}", pos, MAXLEN-1);
      else
      if (b == 172  && c == 132)     /* 239 172 132 */
      pos = insposmaxstr(in, "{ffl}", pos, MAXLEN-1);
      else
      if (b == 172  && c == 133)     /* 239 172 133 */
      pos = insposmaxstr(in, "{st}", pos, MAXLEN-1);
      else
      if (b == 172  && c == 134)     /* 239 172 134 */
      pos = insposmaxstr(in, "{st}", pos, MAXLEN-1);

  return pos;
}


int lenUTFacht(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
  int r = 1;  /* einstelliges Zeichen, es sei denn: */

  if (    a >= DOUBLETTES_BEGIN && a <= DOUBLETTES_END  /* 194 ... 223 */
       && b >= UTF_BLOCK_BEGIN  && b <= UTF_BLOCK_END   /* 128 ... 191 */
     ) r = 2;
  else
  if (    a == TRIPLETTES_BEGIN  /* 224 */
       && b >= 160 && b <= 191
       && c >= 128 && c <= 191
     ) r = 3;
  else
  if (    a >= 225 && a <= TRIPLETTES_END  /* 239 */
       && b >= 128 && b <= 191
       && c >= 128 && c <= 191
     ) r = 3;
  else
  if (    a == QUADRUPLES_BEGIN  /* 240 */
       && b >= 144 && b <= 191
       && c >= 128 && c <= 191
       && d >= 128 && d <= 191
     ) r = 4;
  else
  if (    a >= 241 && a <= 243
       && b >= 128 && b <= 191
       && c >= 128 && c <= 191
       && d >= 128 && d <= 191
     ) r = 4;
  else
  if (    a == QUADRUPLES_END  /* 244 */
       && b >= 128 && b <= 143
       && c >= 128 && c <= 191
       && d >= 128 && d <= 191
     ) r = 4;

  return r;
}


int signeless(unsigned char a)
{
  return a;
}


int reg_utf_eight(int l, int val, char ch[UTF_EIGHT_MAXERR][UTF_EIGHT_MAXLEN+1], const char *p)
{
  int i, entry = 0, found = 0;
  const char *q;

  if (val < UTF_EIGHT_MAXERR)
  {
    while (entry < val)
    {
      i = 0;
      while (i < l && ch[entry][i] == *(p+i) && *(p+i) != '\0') ++i;
      if (    i > 0                       /* i > 0 added in 2.4 */
           && ch[entry][i] == '\0' 
           && (*(p+i) == '\0' || i == l)  /* i == l added in 2.4 */
         )
      {
        found = 1;
        break;
      }
      ++entry;
    }

    if (l <= UTF_EIGHT_MAXLEN)
    {
      if (found == 0)
      {              /* gcc-troubles  ch[val][i] = *(p+i++); */
        q = p; i = 0;
        while (i < l && *q != '\0')   ch[val][i++] = *q++;
        while (i <= UTF_EIGHT_MAXLEN) ch[val][i++] = '\0';
        ++val;
      }
    }
    else intern_err(33);
  }

  return val;
}
    

int utf_eight_err(const char *p)
{
  int l = 1, c = ((unsigned char)*p), plen;

  if (c >= DOUBLETTES_BEGIN && c <= DOUBLETTES_END) l = 2;  /* 194 ... 223 */
  else
  if (c >= TRIPLETTES_BEGIN && c <= TRIPLETTES_END) l = 3;  /* 224 ... 239 */
  else
  if (c >= QUADRUPLES_BEGIN && c <= QUADRUPLES_END) l = 4;  /* 240 ... 244 */

   /* l == 1 if \GenericError stays and the char is in its arguments */
  if (l >= 1)
  {
    utf_err_c = reg_utf_eight(l, utf_err_c, err_utf_eight, p);
    if (l <= UTF_EIGHT_MAXLEN) mystrncpy(this_err_utf_eight, p, l);
    if ((plen=strlen(p)) < l) l = plen;
  }

  return l;
}


void utf_eight_emp(const char *p, int len, int pos, int oldpos)
{
  if (pos == oldpos) 
  {
    utf_emp_c = reg_utf_eight(len, utf_emp_c, emp_utf_eight, p);
    if (len <= UTF_EIGHT_MAXLEN) mystrncpy(this_emp_utf_eight, p, len);
  }
}


void get_utf_eight(char in[], const char *p)
{
  int len, oldpos = 0, pos = 0, a;

    if (*p < 0) lastupperascii = *p;  /* invalid *(p+1) == '\0' */

  while (*p != '\0' && *(p+1) != '\0' && pos < MAXLEN-1)
  {
    len = 1;  /* einstelliges Zeichen, es sei denn: */

    if (*p < 0) 
    { 
      lastupperascii = *p;

      if (*(p+2) != '\0' && *(p+3) != '\0')
      {
         len = lenUTFacht(*p, *(p+1), *(p+2), *(p+3));
      }
      else
      if (*(p+2) != '\0')
      {
         len = lenUTFacht(*p, *(p+1), *(p+2), '\0');
      }
      else
      {
         len = lenUTFacht(*p, *(p+1), '\0', '\0');
      }
    }

    a = signeless(*p);

    if (len == 2)
    { 
      if (a == OHNINETYFOUR)
      pos = insposmaxstr(in, get_ohninetyfour(*(p+1)), pos, MAXLEN-1);
      else
      if (a == OHNINETYFIVE)
      pos = insposmaxstr(in, get_ohninetyfive(*(p+1)), pos, MAXLEN-1);
      else
      if (a == OHNINETYSIX)
      pos = insposmaxstr(in, get_ohninetysix(*(p+1)), pos, MAXLEN-1);
      else
      if (a == OHNINETYSEVEN)
      pos = insposmaxstr(in, get_ohninetyseven(*(p+1)), pos, MAXLEN-1);
      else
      if (a == 198 && signeless(*(p+1)) == 146)  /* 198 146 */
      pos = insposmaxstr(in, "{\\textflorin}", pos, MAXLEN-1);
      else
      if (a == OHNINETYNINE)
      pos = insposmaxstr(in, get_ohninetynine(*(p+1)), pos, MAXLEN-1);
      else
      if (a == THNULLNULL) 
      pos = insposmaxstr(in, get_thnullnull(*(p+1)), pos, MAXLEN-1);
      else
      if (a == THNULLTHREE) pos = ins_thnullthree(in, *(p+1), pos);
      
      utf_eight_emp(p, len, pos, oldpos);
    }
    else if (len == 3)
    {
      if (a == TRIPLETTES_BEGIN)   /* 224 */
      {
        if (signeless(*(p+1)) == 184  && signeless(*(p+2)) == 191)
        pos = insposmaxstr(in, "{\\textbaht}", pos, MAXLEN-1);  /* 224 184 191 */
      }
      else
      if (a == THTWENTYFIVE)
      {  
        if (signeless(*(p+1)) == THTWENTYFIVE_OHEIGHTYFOUR)
        pos = insposmaxstr(in, get_thtwentyfive_oheightyfour(*(p+2)), pos, MAXLEN-1);
        else
        if (signeless(*(p+1)) == THTWENTYFIVE_OHEIGHTYFIVE)
        pos = insposmaxstr(in, get_thtwentyfive_oheightyfive(*(p+2)), pos, MAXLEN-1);
        else
        if (signeless(*(p+1)) == THTWENTYFIVE_OHEIGHTYSIX)  /* new in 2.4 */
        pos = insposmaxstr(in, get_thtwentyfive_oheightysix(*(p+2)), pos, MAXLEN-1);
        else
        if (signeless(*(p+1)) == 187  && signeless(*(p+2)) == 178)
        pos = insposmaxstr(in, "\\`Y", pos, MAXLEN-1);  /* 225 187 178 new in 2.4 / v1.2n */
        else
        if (signeless(*(p+1)) == 187  && signeless(*(p+2)) == 179)
        pos = insposmaxstr(in, "\\`y", pos, MAXLEN-1);  /* 225 187 179 new in 2.4 / v1.2n */
      }
      else
      if (a == THTWENTYSIX)
      {
        if (signeless(*(p+1)) == THTWENTYSIX_OHTWENTYEIGHT)
        pos = insposmaxstr(in, get_thtwentysix_ohtwentyeight(*(p+2)), pos, MAXLEN-1);
        else
        if (signeless(*(p+1)) == THTWENTYSIX_OHTHIRTY)
        pos = insposmaxstr(in, get_thtwentysix_ohthirty(*(p+2)), pos, MAXLEN-1);
        else 
        if (*(p+1) != '\0')
        pos = ins_rest_thtwentysix(in, *(p+1), *(p+2), pos);
        /* 226 129 ...;  226 132 ...;  226 134 ...;  226 140 ...;  
           226 144 ...;  226 151 ...;  226 153 ...;  226 159 ... */
      }
      else
      if (a == THTWENTYSEVEN)
      {
        if (signeless(*(p+1)) == 128  && signeless(*(p+2)) == 136)
        pos = insposmaxstr(in, "{\\textlangle}", pos, MAXLEN-1);  /* 227 128 136 new in 2.5 (v1.3c) */
        else
        if (signeless(*(p+1)) == 128  && signeless(*(p+2)) == 137)
        pos = insposmaxstr(in, "{\\textrangle}", pos, MAXLEN-1);  /* 227 128 137 new in 2.5 (v1.3c) */
      }
      else
      if (a == THTHIRTYNINE)  /* TRIPLETTES_END */
      {      
        if (*(p+1) != '\0')
        pos = ins_ththirtynine(in, *(p+1), *(p+2), pos);
        /* 239 187 ...;  239 172 ... */
      }

      utf_eight_emp(p, len, pos, oldpos);
    }
    else if (len == 4)
    {
      utf_eight_emp(p, len, pos, oldpos);
    }
    else if (len == 1)
    { 
      if (*p < 0) len = utf_eight_err(p);
      else in[pos++] = *p;
    }
    else if (len > UTF_EIGHT_MAXLEN) { intern_err(32); len = 1; }

    oldpos = pos;
    
    p += len;
  }

  while (*p != '\0' && pos < MAXLEN-1)
  {
    len = 1;
    if (*p < 0) len = utf_eight_err(p);
    else in[pos++] = *p;

    p += len;
  }

  in[pos] = '\0';
}


/*--------------------------------------------------------------------------*/


void erweitert(char in[], const char *p)
{
  int pos = 0, len;

  if (encode == IS_TONE_ENC)
  {
    while (*p != '\0' && pos < MAXLEN-1)
    {
      if (*p < 0) pos = insposmaxstr(in, getTONEenc(lastupperascii = *p), pos, MAXLEN-1);
      else in[pos++] = *p;
      ++p;
    }
  }
  else
  {
    while (*p != '\0' && pos < MAXLEN-1)
    {
      if (*p > 0) in[pos++] = *p;
      else
      {   
             lastupperascii = *p;
             thisupperascii = *p;
      }
      ++p;
    }
  }
  in[pos] = '\0';
}


const char *transformtable[][2] = {
  {                  "\\\\",  "[]"                   },
  {            "\\vspace\t",  "{}"                   },
  {            "\\hspace\t",  "{}"                   },
  {            "\\nosort\t",  "{}"                   },
  {             "\\index\t",  "{}"                   },
  {          "\\glossary\t",  "{}"                   },
  {             "\\label\t",  "{}"                   },
  {          "\\pbalabel\t",  "{}"                   },
  {           "\\balabel\t",  "{}"                   },
  {           "\\pageref\t",  "{}"                   },
  {            "\\pbaref\t",  "[]{}"                },
  {             "\\baref\t",  "[]{}"                },
  {               "\\ref\t",  "{}"                   },
  {         "\\linebreak\t",  "[]"                   },
  {       "\\nolinebreak\t",  "[]"                   },
  {         "\\pagebreak\t",  "[]"                   },
  {       "\\nopagebreak\t",  "[]"                   },
  {          "\\footnote\t",  "[]{}"                },
  {      "\\footnotetext\t",  "[]{}"                },
  {      "\\footnotemark\t",  "[]"                   },
  {          "\\framebox\t",  "[][]"                },
  {           "\\makebox\t",  "[][]"                },
  {            "\\parbox\t",  "[]{}"                },
  {          "\\raisebox\t",  "{}[][]"             },
  {              "\\rule\t",  "[]{}{}"             },
  {      "\\printonlyvqu\t",  "[]{}{}*{}{}"       },   /* innerVcmd */
  {               "\\vqu\t",  "[]{}{}*{}{}"       },
  {      "\\printonlyvli\t",  "[]{}{}*{}{}"       },   /* olny      */
  {               "\\vli\t",  "[]{}{}*{}{}"       },
  {     "\\xprintonlyvqu\t",  "[]{}{}*{}{}"       },   /* changes   */
  {              "\\xvqu\t",  "[]{}{}*{}{}"       },
  {     "\\xprintonlyvli\t",  "[]{}{}*{}{}"       },   /* IS_V      */
  {              "\\xvli\t",  "[]{}{}*{}{}"       },
  {         "\\@shadowba\t",  "{}"             },   /* new. in 2.2 */
  {           "\\shadowv\t",  "[]{}{}{}{}" },
  {          "\\addtovli\t",  "[]{}{}{}{}" },
  {          "\\addtovqu\t",  "[]{}{}{}{}" },
  {         "\\xaddtovli\t",  "[]{}{}{}{}" },
  {         "\\xaddtovqu\t",  "[]{}{}{}{}" },
  {      "\\printonlykqu\t",  "[]{}*{}{}"          },
  {      "\\printonlykli\t",  "[]{}*{}{}"          },
  {     "\\xprintonlykqu\t",  "[]{}*{}{}"          },
  {     "\\xprintonlykli\t",  "[]{}*{}{}"          },
  {           "\\shadowk\t",  "[]{}{}{}"    },
  {          "\\addtokli\t",  "[]{}{}{}"    },
  {          "\\addtokqu\t",  "[]{}{}{}"    },
  {         "\\xaddtokli\t",  "[]{}{}{}"    },
  {         "\\xaddtokqu\t",  "[]{}{}{}"    },
  {              "\\frac\t",  "{}{}"                },
  {           "\\vauthor\t",  "{}{}"                },
  {        "\\midvauthor\t",  "{}{}"                },
  {           "\\kauthor\t",  "{}"                   },
  {        "\\midkauthor\t",  "{}"                   },
  {             "\\ersch\t",  "{}[]{}{}"       },
  {            "\\persch\t",  "{}[]{}{}"       },
  {      "\\printonlyper\t",  "{}"                   },
  {      "\\printonlyarq\t",  "[]{}{}"             },  /* new in 2.2 */
  {   "\\printonlydefabk\t",  "{}{}"                },  /* new in 2.2 */
  {   "\\printonlyabkdef\t",  "{}{}"                },  /* new in 2.2 */
  {           "\\shadowo\t",  "{}"             },
  {          "\\addtoper\t",  "{}"             },
  {           "\\shadowt\t",  "{}{}"          },
  {          "\\addtoarq\t",  "{}{}"          },
  {         "\\shadowone\t",  "{}"                   },
  {          "\\addtoabk\t",  "{}"                   },
  {          "\\addtogrr\t",  "{}"                   },
  {          "\\addtoprr\t",  "{}"                   },
  {          "\\addtosrr\t",  "{}"                   },
  {         "\\shadowtwo\t",  "{}{}"                },
  {           "\\fillper\t",  "{}{}"                },
  {           "\\fillarq\t",  "{}{}"                },
  {           "\\fillgrr\t",  "{}{}"                },
  {           "\\fillprr\t",  "{}{}"                },
  {           "\\fillsrr\t",  "{}{}"                },
  {       "\\addtodefabk\t",  "{}{}"                },
  {       "\\addtoabkdef\t",  "{}{}"                },
  {        "\\errmessage\t",  "{}"                   },
  {      "\\GenericError\t",  "{}{}{}{}"          },
  {           "\\message\t",  "{}"                   },
  {           "\\typeout\t",  "{}"                   },
  {        "\\mathhexbox\t",  "{}{}{}"             },
  {           "\\phantom\t",  "{}"                   },
  {          "\\vphantom\t",  "{}"                   },
  {          "\\hphantom\t",  "{}"                   },
  {       "\\showhyphens\t",  "{}"                   },
  {    "\\sethyphenation\t",  "{}"                   },
  {    "\\selectlanguage\t",  "{}"                   },
  {     "\\discretionary\t",  "{}{}"                },
  {        "\\pstressing\t",  "{}"                   },
  {         "\\stressing\t",  "{}"                   },
  {         "\\pxbibmark\t",  "{}{}"                },  /* Nur \\p...  */
  {          "\\pxbibref\t",  "{}"                   },  /* bearbeiten! */
  {          "\\pbibmark\t",  "{}"                   },
  {           "\\bibmark\t",  "{}"                   },
  {                 KURZCMD,  "{}"                   },
  {           "\\delkurz\t",  "{}"                   },
  {             "\\begin\t",  "{}"                   },
  {               "\\end\t",  "{}"                   },
  {         "\\pfordinal\t",  "{}"                   },  /* new in 2.2 */
  {"\\@inpenc@undefined@\t",  "{}"                   },  /* new in 2.3 */
  {"\\G@refundefinedtrue\t",  "{}"                   },  /* new in 2.3 */
  {    "\\GenericWarning\t",  "{}{}"                },  /* new in 2.3 */
  { NULL, NULL }};
#define NUMTRANSV 102


/* Muessen im Bereich von issonder liegen: */
#define  WINGBR    25
#define  ANGUBR    24
#define ROUNDBR    23
#define OPTSTAR    22

char *bracketsformward(char *q, char c)
{
        if (c == '}') *q =  WINGBR;
   else if (c == ']') *q =  ANGUBR;
   else if (c == ')') *q = ROUNDBR;
   else intern_err(9);   /* BRACKETS LOST */
   return q;
}


int len_if_has_bra(const char *p, const char opench, const char closech)
{
   int l, r = 0;

   if (*p == opench && (l=nextbracket(p, opench, closech)) > 0) r = l;

   return r;
}


void exchange_v(char *p, const char *bra, int num)
{
  int l = 0, la = 0, i = 0, lb = 0, j = 0, k;
  char buf[MAXLEN], fub[MAXLEN], *lp = p, *anf = p;
  
  while (*p == ' ') { ++p; ++la; }
  if ((l=len_if_has_bra(p, *(bra+(3*num+1)), *(bra+(3*num+2)))) > 0)
  {
    la += l; while (i < la) buf[i++] = *lp++;
     p += l;
    if (*(bra+(3*(num+1))) == '')
    {
      while (*p == ' ') { ++p; ++lb; }
      if ((l=len_if_has_bra(p, *(bra+(3*(num+1)+1)), *(bra+(3*(num+1)+2)))) > 0)
      {
        lb += l; while (j < lb) fub[j++] = *lp++;
        k = 0; while (k < j) *anf++ = fub[k++];
        k = 0; while (k < i) *anf++ = buf[k++];
      }
    }
    else intern_err(10);  /* No formatter Strg+Q after Strg+P */
  }
}


int len_all_arg(const char *p, const char opench, const char closech)
{
   int l = 0, r = 0;

   if (*p == opench && (l=nextbracket(p, opench, closech)) > 0) r = l;
   else
   if ('{' == opench)
   {
     if (*p == '\\' && *(p+1) != '\0' && isLaTeXcmdletter(*(p+1)) == 0) r = 2;
     else
     if (*p == '\\' && *(p+1) != '\0' && isLaTeXcmdletter(*(p+1)) == 1)
     {
       r += 2; p += 2;
       while (*p != '\0' && isLaTeXcmdletter(*p) == 1) { ++r; ++p; }
       while (*p == ' ') { ++r; ++p; }
     }
     else
     if (*p != '{' && *p != '\0') r = 1;
   }

   return r;
}


char *ins_bracket(char *s, const char opench, const char closech, int l)
{
  char buf[MAXLEN] = { opench, '\0' }, b[2] = { closech, '\0' };

  strncat(buf, s, l);
  strcat (buf, b);
  strcat(buf, s+l);
  strcpy(s, buf);
  return s;
}


char *over_handled_TeX_arg(char *s, char *q, const char left_ch, const char right_ch)
{
  int l;

  while (*q == ' ') ++q;

  if ((l=len_if_has_bra(q, left_ch, right_ch)) < 2)
  {
    if (    (l=len_all_arg(q, left_ch, right_ch)) > 0 
         && strlen(s) < MAXLEN-2
       )
    { /*  ins_bracket inserts  2 characters:  */
      q = ins_bracket(q, left_ch, right_ch, l);
      q += l+1;
      q = bracketsformward(q, right_ch);
      ++q;
    }
  }
  else
  {
      q += l-1;
      q = bracketsformward(q, right_ch);
      ++q;
  }

  return q;
}


char *handletransformarg(char *s, char *p, const char *bra)
{
  int l = 0, num = 0;
  char *q = p;

  while (*(bra+(3*num)) != '\0' && *q != '\0')
  {
    if ('' == *(bra+(3*num)) || '' == *(bra+(3*num)))
    {
      while (*q == ' ') ++q;
      if ((l=len_all_arg(q, *(bra+(3*num+1)), *(bra+(3*num+2)))) < 1) l = 0;  /* changed in 2.3 */
      q += l;
      if ('' == *(bra+(3*num)))
      {
        while (*q == ' ') ++q;
      }
      p = q;
    }
    else
    if ('' == *(bra+(3*num)))
    {
      if (*(bra+(3*num+1)) == '' && *(bra+(3*num+2)) == '')
      {
        if ((l=nextbracket(q, '(', ')')) < 2 && (l=nextbracket(q, '[', ']')) < 2) l = 0;
      }
      else
      if (*(bra+(3*num+1)) == '' && *(bra+(3*num+2)) == '')
      {
        if (*q == '_')
        {
          l = 1;
          while (*(q+l) != '_' && *(q+l) != '\0') ++l;
          if (*(q+l) == '_') ++l;
          else if (*(q+l) == '\0') l = 0;
        }
        else
        if (*q == '|')
        {
          l = 1;
          while (*(q+l) != '|' && *(q+l) != '\0') ++l;
          if (*(q+l) == '|') ++l;
          else if (*(q+l) == '\0') l = 0;
        }
        else l = 0;
      }
      else l = 0;
      q += l;
      p = q;
    }
    else
    if ('' == *(bra+(3*num)))
    {
      while (*q == ' ') ++q;
         if (*q == '*')
      {  
           ++q;
        while (*q == ' ') ++q;
        if ((l=nextbracket(q, *(bra+(3*num+1)), *(bra+(3*num+2)))) < 2) l = 0;
        q += l;
        p = q;
      }
    }
    else
    if ('' == *(bra+(3*num)))
    {
      while (*q == ' ') ++q;
         if (*q == '*') ++q;
      while (*q == ' ') ++q;
      if (*q != *(bra+(3*num+1)) || (l=nextbracket(q, *(bra+(3*num+1)), *(bra+(3*num+2)))) < 2) l = 0;
      q += l;
      p = q;
    }
    else
    if ('' == *(bra+(3*num)))
    {
      q = over_handled_TeX_arg(s, q, *(bra+(3*num+1)), *(bra+(3*num+2)));  /* changed in 2.3 */
    }
    else
    if ('' == *(bra+(3*num)) || '' == *(bra+(3*num)))
    {
      if ('' == *(bra+(3*num))) exchange_v(q, bra, num);
      q = over_handled_TeX_arg(s, q, *(bra+(3*num+1)), *(bra+(3*num+2)));  /* changed in 2.3 */
    }
    else
    if ('*' == *(bra+(3*num)))
    {
      while (*q == ' ') ++q;
      if (*q == '*')
      { *q = OPTSTAR;
        ++q;
        q = over_handled_TeX_arg(s, q, *(bra+(3*num+1)), *(bra+(3*num+2)));  /* changed in 2.3 */
      }
    }
    ++num;
  }

  return p;
}


void transformline(char buf[], char *p)
{
  int pos = 0, i, l;
  char *s = p;

  while (*p != '\0' && pos < MAXLEN-2)
  {
    if (*p == '\\')
    {
      if ((i=texlistnum(p, NUMTRANSV, transformtable)) > -1)
      {
          if (*(p+1) == '\\') buf[pos++] = ' ';
          p += llstrlen(transformtable[i][0]);
          p = handletransformarg(s, p, transformtable[i][1]);
      }
      else buf[pos++] = *p++;
    }
    else
    if (issonder(*p) == 1)
    {
      if (*p == OPTSTAR);
      else
      {
             if (*p ==  WINGBR) buf[pos++] = '}';
        else if (*p ==  ANGUBR) buf[pos++] = ']';
        else if (*p == ROUNDBR) buf[pos++] = ')';
        else intern_err(11);  /* unknown bracket symbol */
        buf[pos++] = ' ';
      }
      ++p;
      while (*p == ' ') ++p;
    }
    else   buf[pos++] = *p++;
  }

  buf[pos] = '\0';

  /*
  if (pos > MAXLEN-4 && *p != '\0') printf("%%%%\n%%%%>  Error: Can\'t sort ...\n");
   */
}


void nosondercpy(char *s, const char *p)
{
  while (*p != '\0')
  { if (issonder(*p) == 0) *s++ = *p;
    ++p;
  }
  *s = '\0';
}


const char *overbago(const char *s)
{
  const char *p = s;
  int got = 0;

  if (bago > 0)
  {
    while (*p != '\0')
    {
      if (*p == '\\')
      {
        if (*(p+1) == '\\')
        {
          ++p;
        }
        else
        if (lcmdcmp(p, BAGOCMD) == 0)
        {
             p+=strlen(BAGOCMD);
          while (*p == ' ') ++p;
          got = 1;
          break;
        }
      }
      ++p;
    }
  }

  if (got == 0) return s;
  return p;
}


void vorbereiten(int linec, int alllines, int deepsort, int ventry)
{
    char *p, in[MAXLEN], out[MAXLEN], num[2*NUMLEN], last, buf[MAXLEN];
    int pos, i, l;

    /* dqsort geht bis zur Tiefe deepsort, nicht deep */

    while (linec < alllines)
    {
      i = 0; while (i < MAXBACOLS)  subBptr[i++][linec] = mydefault;
      i = 0; while (i < MAXBACOLS)  subCptr[i++][linec] = mydefault;

      i = 0;
      while (i < deepsort)
      {
           nosondercpy ( in, overbago(subAptr[i][linec]));
          transformline(buf,   in);  /* veraendert zweites Arg! */
       if (encode == IS_UTFE_ENC)
         get_utf_eight ( in,  buf);
       else  erweitert ( in,  buf);
        p = austauschen(buf,    in, dqsort[linec]);


           pos = 0;   last = '\0';
        while (*p != '\0' && pos < MAXLEN-1)
        {
           if  (    isalpha (*p) != 0
                 || issonder(*p) == 1
                 ||  *p == ' '
                 ||  *p == '-'
                 || (*p == '.' && registpoints == 1)   /* nur zusammen mit MARK_POINT */
               )
           {  
              if (*p == ' ' && ignorespaces == 1)
              {
                if (pos > 0 && out[pos-1] == '') --pos;
                   out[pos]   = '';   /* 31 */
              }
              else
              if (*p == '-')           /* MINUS */
              {
                   out[pos]   = '';   /* 31 */
                if (hy_as_space == 1 && pos < MAXLEN-2)
                 out[++pos]   = ' ';
              }
              else out[pos]   = *p;

                     ++pos;
           }
           else
           if  (isdigit(*p) != 0 && last != '\\')
           {
              p = standardnum(num, p);
              out[pos] = '\0';
                 strncat(out, num, MAXLEN-strlen(out)-1);
              pos = strlen(out);
              --p;
           }

           last = *p;
           ++p;
        }

         out[pos] = '\0';

         kill_unnecessary_spaces(in, out);

         nzvorziehen(out, in);

         subBptr[i][linec] = strnmalloc(out, strlen(out)+1);

         pos = 0;
         l   = 0;
         while (out[pos] != '\0') 
         { 
           if (issonder(out[pos]) == 0)
           {
             in[l] = toupper(out[pos]);
             if (igleichj == 1 && in[l] == 'I') in[l] = 'J';
             ++l;
           }
           ++pos;
         }
         subCptr[i][linec] = strnmalloc(in, l+1);

        ++i;
      }

      if (ventry == IS_V && *subAptr[0][linec] == '\0') swapBCstrs(linec);

      ++linec;
    }
}


int isvintroduced(int i)
{
   const char *p = subRptr[STUFFRCOL][i];

   while (*p == ' ') ++p;

   if (*p == '{')
   {
     p += nextbracket(p, '{', '}');
     while (*p == ' ') ++p;

     if (*p == '{')
     {
       p += nextbracket(p, '{', '}');
       while (*p == ' ') ++p;

       if (*p == '{')
       { 
         ++p;
         while (*p == ' ') ++p;

         if (*p == KENTRYCH) 
         { 
           ++p;
           while (*p == ' ') ++p;

           if (*p == '}') return KENTRY;
         }

         if (*p == VENTRYCH) 
         { 
           ++p;
           while (*p == ' ') ++p;

           if (*p == '}') return VENTRY;
         }
       }
     }
   }
      
   return ERRENTRY;
}


int getthirdnum(int i)
{
   const char *p = subRptr[STUFFRCOL][i];

   while (*p == ' ') ++p;

   if (*p == '{')
   {
     p += nextbracket(p, '{', '}');
     while (*p == ' ') ++p;

     if (*p == '{')
     {
       p += nextbracket(p, '{', '}');
       while (*p == ' ') ++p;

       if (*p == '{')
       { 
         ++p;
         while (*p == ' ') ++p;

         return getarabicnum(p);
       }
     }
   }
      
   return 0;
}


int notfirstaccepted(int line)
{
   int r = 0, acceptbefore = 0;

   while(r < line)
   { if (leval[r] == ACCEPTLINE) { ++acceptbefore; break; }
     ++r;
   }

   return acceptbefore;
}


void dofourtharg(int line)
{
   const char *p = subRptr[STUFFRCOL][line];
   int l = 0;

   while (*p == ' ') ++p;

   if (*p == '{')
   {
     p += nextbracket(p, '{', '}');
     while (*p == ' ') ++p;

     if (*p == '{')
     {
       p += nextbracket(p, '{', '}');
       while (*p == ' ') ++p;

       if (*p == '{')
       { 
         p += nextbracket(p, '{', '}');
         while (*p == ' ') ++p;

         if (*p == '{')
         { 
           l = nextbracket(p, '{', '}');

           if (l > 2)
           { 
             ++p; l -= 2;
             while (*p == ' ') { ++p; --l; };

             if (l > 0)
             { 
               fprintf(outfile, "   %.*s\n   %%%%  <- Your extra-command(s)", l, p);
               if (notfirstaccepted(line) == 0)
               fprintf(outfile, " before the 1st item");
               fprintf(outfile, ".\n");
             }
           }
         }
       }
     }
   }
      
}


void getfirsttwoargs(char *s, const char *p)
{
   int len;

   while (*p == ' ') ++p;

   if (*p == '{')
   {
     if ((len=nextbracket(p, '{', '}')) < MAXLEN-1)
     {
       while(len-- > 0) *s++ = *p++;

       while (*p == ' ') ++p;

       if (*p == '{')
       {
         if (len + (len=nextbracket(p, '{', '}')) < MAXLEN-1)
         {
           while(len-- > 0) *s++ = *p++;
         }
       }
     }
   }
   *s = '\0';
}


int baidxloop(int cloop, const char *p)
{
    cloop += fprintf(outfile, "%s", p);

  if (cloop > BAINDEXLEN)
  { 
             fprintf(outfile, "%s", BAINDEXEND);
             fprintf(outfile, "%s", BAINDEXFOLL);  /* neustart cloop */
    cloop = CLOOPBSTART;
  }

  return cloop;
}


char *negnum(char *p)
{
  if (*p == '-') return "\\,";
  return "";
}


int printipagefnt(const char *arraysep, int begin, int end, int lasti, int cloop)
{
   int fnc, pnc;

   if (    lasti > -1
        && pagewgt[begin] == pagewgt[lasti]
        && numcmp(1, pagewgt[begin], subRptr[PAGECOL][lasti], subRptr[PAGECOL][begin]) == 0
      )
   {
         cloop += fprintf(outfile, "$^{,}$");
         cloop  = baidxloop(cloop, BS_SPACE);
   }
   else
   {
                                             cloop  = baidxloop(cloop, arraysep);
          if (pagewgt[begin] ==     MISSNUM) cloop += fprintf(outfile, "{%s}", MISSINGSYMB);
     else if (pagewgt[begin] ==    EMPTYNUM) cloop += fprintf(outfile, "%s",     EMPTYSYMB);
     else if (pagewgt[begin] ==    OVERFLOW) cloop += fprintf(outfile, "%s",  OVERFLOWSYMB);
     else if (pagewgt[begin] == INTERNALERR)  intern_err(12);  /* Bad pageno */
     else if (pagewgt[begin] ==      UNUSED)  intern_err(13);  /* Unused pageno */
     else if (pagewgt[begin] ==        TEXT) cloop += fprintf(outfile, "%s%s%s", TEXTSYMBOPEN[notext], subRptr[PAGECOL][begin], TEXTSYMBCLOSE[notext]);
     else                                    cloop += fprintf(outfile, "%s", subRptr[PAGECOL][begin]);
   }

   if (end > begin)
   {
     fnc = numcmp(0,  fntwgt[begin],  subRptr[FNTCOL][end],  subRptr[FNTCOL][begin]);
     pnc = numcmp(1, pagewgt[begin], subRptr[PAGECOL][end], subRptr[PAGECOL][begin]);

     if (fnc == 1)
     {
       if (pnc > 0)
       {
         cloop += fprintf(outfile, "$^{%s}$",          subRptr[FNTCOL][begin]);
         cloop  = baidxloop(cloop, BANCOMMA);
       }
       else
       { 
         cloop += fprintf(outfile, "$^{%s}$$^{,}$",    subRptr[FNTCOL][begin]);
         cloop  = baidxloop(cloop, BS_SPACE);
       }
       
       if (pnc > 0)
       {
         cloop += fprintf(outfile, "%s$^{",            subRptr[PAGECOL][end]);
       }
       else
       {
         cloop += fprintf(outfile, "$^{");
       }
     }
     else
     if (fnc  > 1)
     {
       if (pnc > 0)
       { 
         cloop += fprintf(outfile, "$^{%s}$--%s%s$^{",    subRptr[FNTCOL][begin], negnum(subRptr[PAGECOL][end]), subRptr[PAGECOL][end]);
       }
       else
       {
         cloop += fprintf(outfile, "$^{%s-",              subRptr[FNTCOL][begin]);
       }
     }
     else 
     if (      fntwgt[end] != INTERNALERR
            && fntwgt[end] !=  NOFOOTNOTE
        )
         cloop += fprintf(outfile, "$^{");
   }
   else  cloop += fprintf(outfile, "$^{");

          if ( fntwgt[end] ==     MISSNUM) cloop += fprintf(outfile, "%s}$",  MISSINGSYMB);
     else if ( fntwgt[end] ==    EMPTYNUM) cloop += fprintf(outfile, "%s}$",    EMPTYSYMB);
     else if ( fntwgt[end] ==    OVERFLOW) cloop += fprintf(outfile, "%s}$", OVERFLOWSYMB);
     else if ( fntwgt[end] == INTERNALERR)  intern_err(14);  /* Bad fntno */
     else if ( fntwgt[end] ==  NOFOOTNOTE)  intern_err(15);  /* That is not a footnote */
     else if ( fntwgt[end] ==        TEXT) cloop += fprintf(outfile, "%s%s%s}$", TEXTSYMBOPEN[notext], subRptr[FNTCOL][end], TEXTSYMBCLOSE[notext]);
     else                                  cloop += fprintf(outfile, "%s%s}$",  negnum(subRptr[FNTCOL][end]),  subRptr[FNTCOL][end]);

   return cloop;
}


int printipage(const char *arraysep, int begin, int end, int cloop)
{
   int pnc;

                                          cloop  = baidxloop(cloop, arraysep);
          if (pagewgt[begin] ==  MISSNUM) cloop += fprintf(outfile, "{%s}", MISSINGSYMB);
     else if (pagewgt[begin] == EMPTYNUM) cloop += fprintf(outfile, "%s",     EMPTYSYMB);
     else if (pagewgt[begin] == OVERFLOW) cloop += fprintf(outfile, "%s",  OVERFLOWSYMB);
     else if (pagewgt[begin] ==     TEXT) cloop += fprintf(outfile, "%s%s%s", TEXTSYMBOPEN[notext], subRptr[PAGECOL][begin], TEXTSYMBCLOSE[notext]);
     else                                 cloop += fprintf(outfile, "%s", subRptr[PAGECOL][begin]);

   if (end > begin)
   {
     pnc = numcmp(1, pagewgt[begin], subRptr[PAGECOL][end], subRptr[PAGECOL][begin]);
          if (pnc == 1)
     {
       cloop  = baidxloop(cloop, BANCOMMA);
       cloop += fprintf(outfile, "%s", subRptr[PAGECOL][end]);
     }
     else if (pnc  > 1) 
     {
       cloop += fprintf(outfile, "--%s%s", negnum(subRptr[PAGECOL][end]), subRptr[PAGECOL][end]);
     }
   }

   return cloop;
}


void makeipagefnt(int i, int line, int end)
{
  int begblock, lasti = -1, j, k, cloop = CLOOPASTART;
  char *asfirst  = "";
  char *assecond = BANCOMMA;
  char *arraysep = asfirst;


  while (i <= end && pagewgt[i] == UNUSED) ++i;


  if (i <= end)
  {

   while (i <= end)
   {
     if (cloop == CLOOPASTART)
     {
       fprintf(outfile, "%s", BAINDEXFIRS);  /* Erststart cloop */
       cloop = CLOOPBSTART;
     }
     else
     if (cloop == CLOOPBSTART) intern_err(16);

       begblock = i;

     if (fntwgt[i] == NOFOOTNOTE)  /* Seiten ohne FNTs */
     {
       while (i < end)
       { 
          if (      fntwgt[i]   == NOFOOTNOTE
               &&   fntwgt[i+1] == NOFOOTNOTE
               &&  pagewgt[i]   != MISSNUM
               &&  pagewgt[i]   == pagewgt[i+1]
               && (    (pagewgt[i] == TEXT && strcmp(subRptr[PAGECOL][i], subRptr[PAGECOL][i+1]) == 0)
                    || (pagewgt[i] != TEXT && ((j=numcmp(1,   pagewgt[i], subRptr[PAGECOL][i+1], subRptr[PAGECOL][i])) == 0 || j == 1))
                  )
             ) ++i;
          else break;
       }
       cloop = printipage(arraysep, begblock, i, cloop);
       lasti = -1;
     }
     else                        /* Seiten mit FNTs */
     {
       while (i < end)
       {
          k = -1;
          if (      fntwgt[i] != NOFOOTNOTE
               &&  pagewgt[i] != MISSNUM
               &&  pagewgt[i] == pagewgt[i+1]
               && (    (pagewgt[i] == TEXT && strcmp(subRptr[PAGECOL][i], subRptr[PAGECOL][i+1]) == 0)
                    || (pagewgt[i] != TEXT &&  ((k=numcmp(1,  pagewgt[i], subRptr[PAGECOL][i+1], subRptr[PAGECOL][i])) == 0 || (k == 1 && FntIndexOverPages == 1)))
                  )
               &&   fntwgt[i] != MISSNUM
               &&   fntwgt[i] ==  fntwgt[i+1]
               && (    ( fntwgt[i] == TEXT && (k == 0 || pagewgt[i] == TEXT)
                                           && strcmp(subRptr[ FNTCOL][i], subRptr[ FNTCOL][i+1]) == 0)
                    || ( fntwgt[i] != TEXT && (((j=numcmp(0,   fntwgt[i], subRptr[ FNTCOL][i+1], subRptr[ FNTCOL][i])) == 0 && k == 0) || j == 1))
                  )
             ) ++i;
          else break;
       }
       cloop = printipagefnt(arraysep, begblock, i, lasti, cloop);
       lasti = i;
     }
 
     ++i;
     arraysep = assecond;
   }
  
       fprintf(outfile, "%s", BAINDEXEND);
  }
}


void makeindex(const int nlines, const int line, const int deepsort)
{
      int i, j, begin, end;

          i = line;
   while (i > 0 && leval[i-1] != REJECTLINE)
   {
             j = 0; 
      while (j < deepsort && texcmp(subAptr[j][i], subAptr[j][i-1], dqsort[i], dqsort[i-1]) == 0) ++j;
         if (j < deepsort) break;
      --i;
   }
   begin = i;

          i = line;
   while (i < nlines-1 && leval[i+1] != REJECTLINE)
   {
             j = 0; 
      while (j < deepsort && texcmp(subAptr[j][i], subAptr[j][i+1], dqsort[i], dqsort[i+1]) == 0) ++j;
         if (j < deepsort) break;
      ++i;
   }
     end = i;

   makeipagefnt(begin, line, end);
}


void printsortline(int line, int deepsort)
{
  int l = 0;

  if (deepsort > 0)
  {
      fprintf(outfile, " %s", METALINEHEAD);
 
    while (l < deepsort)
    { 
      fprintf(outfile, "  (%s)", subBptr[l][line]);
      ++l;
    }
 
      fprintf(outfile, "\n");
  }
}


int findauthor(const char *s)
{
   int r = 0;

   while(*s != '\0')
   {
     if (*s == '\\') 
     { 
       if (*(s+1) == '\\')
       {
         ++s; ++r;
       }
       else if (texlistnum(s, NAUTHOR, authortable) > -1) break; 
     }
     ++s; ++r;
   }

   if (*s == '\0') r = -1;

   return r;
}


int fprint_inner_ktit_err(int j, int err, int dif, const char *msgA, const char *msgB)
{
  if (j >= err)
  {
    --j;
    fprintf(outfile, "%%%%  <- Warning *inner v-cmd*: %s %s (reported No. %d).\n", msgA, msgB, ktiterr+1);
    ++ktiterr;
  }

  if (j >= dif) j -= dif;

  return j;
}


#define O_KTIT_UNINIT         -2   /*  init: mostoktitarg > -2  */
#define O_KTIT_NOTUSED        -1   /*  used: mostoktitarg > -1  */
#define O_KTIT_NOPREF          0   /*  pref: mostoktitarg >  0  */
#define MOST_O_KTIT_ARGONE     1
#define MOST_O_KTIT_ARGTWO     2
#define MOST_O_KTIT_ARGTHREE   3
#define MOST_O_KTIT_ARGFOUR    4

void check_out_ktit(int line, int deepsort, int mostoktitarg)
{
  int i = 0, j = 0, numalter = 0, num = -1, numall = 0;

  if (mostoktitarg > O_KTIT_UNINIT)   /* ventry == IS_V */
  {
    while (i < deepsort)   /* assume: deep == deepsort */
    {
      num = num_of_cmds_in_str(subAptr[i][line], KTITCMD);
      if (num > 1)
      {
        fprintf(outfile, "%%%%  <- Warning *outer v-cmd*: %s used %d times in arg %d (reported No. %d).\n", KTIT, num, i+1, ktiterr+1);
        ++ktiterr;
      }
      numalter = 0;
      if (num > 0)
      {
        j = i+1;
        while (j < deepsort)
        {
          if ((numalter = num_of_cmds_in_str(subAptr[j][line], KTITCMD)) > 0) break;
          ++j;
        }
      }
      if (num > 0 && numalter > 0)
      {
        /* use int j here! */
        fprintf(outfile, "%%%%  <- Warning *outer v-cmd*: %s used in arg %d and in arg %d (reported No. %d).\n", KTIT, i+1, j+1, ktiterr+1);
        ++ktiterr;
      }
      if (num > 0 && mostoktitarg > O_KTIT_NOPREF && i+1 != mostoktitarg)
      { 
        if (numalter == 0)
        {
          if (    i+1 == MOST_O_KTIT_ARGONE
               || i+1 == MOST_O_KTIT_ARGTWO
               || i+1 == MOST_O_KTIT_ARGTHREE
               || i+1 == MOST_O_KTIT_ARGFOUR
             );
             else intern_err(42);

          fprintf(outfile, "%%%%  <- Warning *outer v-cmd*: %s defined in arg %d; most entries use arg %d (reported No. %d).\n", KTIT, i+1, mostoktitarg, ktiterr+1);
          ++ktiterr;
        }
      }
      if (num > 0 && mostoktitarg == O_KTIT_NOPREF)
      {
        if (numalter == 0)
        {
          fprintf(outfile, "%%%%  <- Warning *outer v-cmd*: %s is defined here in arg %d; sum of all entries shows no preference (reported No. %d).\n", KTIT, i+1, ktiterr+1);
          ++ktiterr;
        }
      }
             if (i == 0) { numall += num; }
        else if (i == 1) { numall += num; }
        else if (i == 2) { numall += num; }
        else if (i == 3) { numall += num; }
        else intern_err(41);
      ++i;
    }

    if (numall == 0 && mostoktitarg > O_KTIT_NOTUSED)
    {
        fprintf(outfile, "%%%%  <- Warning *outer v-cmd*: %s not used (or is masked); ", KTIT);
        if (mostoktitarg  > O_KTIT_NOPREF) fprintf(outfile, "most other entries use arg %d ", mostoktitarg);
        if (mostoktitarg == O_KTIT_NOPREF) fprintf(outfile, "other entries show no preference ");
        fprintf(outfile, "(reported No. %d).\n", ktiterr+1);
        ++ktiterr;
    }
  }
}


int writetheline(char *p, const int nlines, const int line, const int deepsort, int filec, int printsort, int kandkused, int mostoktitarg)
{
  int doprint = 0, pagenum = 0, fntnum = 0, uu = 0, tn = getthirdnum(line), j;

  if (*p != '%')
  { if (leval[line] != ACCEPTLINE)
      *p  = '%';
    intern_err(17);  /* LINE WITHOUT \'%%\' */
  }


    if (leval[line] == ACCEPTLINE)
    {
      j = line-1;
      while (j > 0 && leval[j] != ACCEPTLINE) --j;
      if (    j < 0
           || (leval[j] == ACCEPTLINE && *subCptr[0][j] != *subCptr[0][line])
           || (leval[j] != ACCEPTLINE)
         )
      {
           if (leval[j] != ACCEPTLINE && j != 0) intern_err(18);  /* misplaced PREPARESPACE/PREPAREHEAD */

        if (isalpha(*subCptr[0][line]) != 0)
        {
          fprintf(outfile, "\n\n");
          if (line > 0 && leval[j] == ACCEPTLINE)
          fprintf(outfile, "%s\n",       PREPARESPACE);
          fprintf(outfile, "%s{%c}\n\n", PREPAREHEAD,  *subCptr[0][line]);
        }
        else if (line > 0 && isdigit(*subCptr[0][line]) != 0)
        {
          fprintf(outfile, "\n\n%s\n",   PREPARESPACE);
          fprintf(outfile, "%s\n\n",     PREPAREXSPACE);
        }
      }
    }


         if (leval[line] == ACCEPTLINE) { doprint = 1;   ++p;          }
    else if (leval[line] == REJECTLINE) { doprint = 2;                 }
    else if (leval[line] == DOUBLELINE) { doprint = 0; /* *p = '!'; */ }
    else if (leval[line] == DIFFERLINE) { doprint = 4; /* *p = ':'; */ }
    else if (leval[line] == UNDEFILINE) { doprint = 5; /* *p = '*'; */ }
    else if (leval[line] == REDEF1LINE) { doprint = 6;                 }
    else if (leval[line] == REDEF2LINE) { doprint = 7;                 }
    else if (leval[line] == ABBREVLINE) { doprint = 0; /* *p = '+'; */ }
    else intern_err(19);  /* UNKNOWN WRITELINES-leval */

         if (pagewgt[line] ==   EMPTYNUM)  pagenum = 1;
    else if (pagewgt[line] ==    MISSNUM)  pagenum = 2;
    else if (pagewgt[line] ==       TEXT)  pagenum = 3;
    else if (pagewgt[line] ==   OVERFLOW)  pagenum = 4;

         if (pagewgt[line] ==     UNUSED)       uu = 1;

         if ( fntwgt[line] ==   EMPTYNUM)   fntnum = 1;
    else if ( fntwgt[line] ==    MISSNUM)   fntnum = 2;
    else if ( fntwgt[line] ==       TEXT)   fntnum = 3;
    else if ( fntwgt[line] ==   OVERFLOW)   fntnum = 4;


  if (tn > 0 && doprint == 1 && leval[line] == ACCEPTLINE) dofourtharg(line);

  if (doprint == 2)           fprintf(outfile, "%%%%  *Reject entry* (syntax error or empty):\n");

  if (doprint  > 0 || pagenum > 0 || fntnum > 0 || uu > 0)
  {
               if (filec > 1) fprintf(outfile, "%%%%  Entry from %s ...\n", subRptr[LINECOL][line]);
    if (printsort == 1 && doprint == 1) printsortline(line, deepsort);
                              fprintf(outfile, "%s\n", p);

    if (kandkused == 1 && num_of_cmds_in_str(p, KURZCMD) > 0)
                              fprintf(outfile, "%%%%  <- Warning: You have used %s here, but also %s here or elsewhere.\n", KURZ, KTIT);

    check_out_ktit(line, deepsort, mostoktitarg);

    /* get_add_ktit in 2.5 changed into: */
    j = inerr[line];
    j = fprint_inner_ktit_err(j, KTIT_OPE_ERR, KTIT_OPE_BAS, "[f{...}] or [m{...}] or [p{...}] overwrites but not replaces \'star\'", KTIT);
    j = fprint_inner_ktit_err(j, OARG_BRA_ERR, OARG_BRA_BAS, "[f/m/p TEXT]", "has TEXT not in {...}");
    j = fprint_inner_ktit_err(j, OARG_IRR_ERR, OARG_IRR_BAS, "First letter in [OptArg]", "is not f/m/p");
    j = fprint_inner_ktit_err(j, KTIT_DBL_ERR, KTIT_DBL_BAS, KTIT, KTIT);  /* double */
    j = fprint_inner_ktit_err(j, KTIT_MIS_ERR, KTIT_MIS_BAS, KTIT, "is missing");
    j = fprint_inner_ktit_err(j, KTIT_EMP_ERR, KTIT_EMP_BAS, KTIT, "has empty arg");
    j = fprint_inner_ktit_err(j, KTIT_SEV_ERR,      NOINERR, KTIT, "used several times");
    if (j != 0) intern_err(38);
  }


  if (    ex_n == EXCHANGENAMES
       && leval[line] != DOUBLELINE
       && (j=findauthor(p)) > -1
     )                        fprintf(outfile, "%%%%  <- Error: Can\'t exchange {FirstName} and {FamilyName} after cmd on pos %d (bad brackets).\n", j+1);


  if (doprint  > 3) {         fprintf(outfile, "%%%%  <- TextArg-Warning");
                 if (uu == 0) fprintf(outfile, " (NUMBERS of entry are not necessarily rejected)");
                              fprintf(outfile, ":\n");
                    }
  if (doprint == 4)           fprintf(outfile, "%%%%      HEAD is explained different now; that new explanation is ignored.\n");
  if (doprint == 5)           fprintf(outfile, "%%%%      HEAD is not (yet\?) explained.\n");
  if (doprint == 6)           fprintf(outfile, "%%%%      HEAD is explained again (short-title mismatch).\n");
  if (doprint == 7)           fprintf(outfile, "%%%%      HEAD is explained again (risky when open/close seps are different).\n");

  
                   if (pagenum > 0 || fntnum > 0)
                              fprintf(outfile, "%%%%  <- Counter-Warning (TEXT of entry is not necessarily rejected):\n");

  if (pagenum  > 0)           fprintf(outfile, "%%%%     * Page-num ");
  if (pagenum == 1)           fprintf(outfile, "empty.\n");                                  /*   EMPTYNUM */
  if (pagenum == 2)           fprintf(outfile, "missing.\n");                                /*    MISSNUM */
  if (pagenum == 3)           fprintf(outfile, "not valid and SORTED as TEXT.\n");
  if (pagenum == 4)           fprintf(outfile, "contains \"%s\" (guess Alph/alph/fnsymbol overflow).\n", OVERFLOWTXT);

  if (tn == 0 && uu == 1)     fprintf(outfile, "%%%%  <- Def is in an {unused}-environment or is a \\fill...-cmd.\n");
  if (tn  > 0 && uu == 1)     fprintf(outfile, "%%%%  <- List-internal heading (class %d).\n", tn);

  if ( fntnum  > 0)           fprintf(outfile, "%%%%     * Footnote-num ");
  if ( fntnum == 1)           fprintf(outfile, "empty.\n");                                  /*   EMPTYNUM */
  if ( fntnum == 2)           fprintf(outfile, "missing.\n");                                /*    MISSNUM */
  if ( fntnum == 3)           fprintf(outfile, "not valid and SORTED as TEXT.\n");
  if ( fntnum == 4)           fprintf(outfile, "contains \"%s\" (guess Alph/alph/fnsymbol overflow).\n", OVERFLOWTXT);


  if (doprint == 1 && nlines > 0) makeindex(nlines, line, deepsort);

  if (doprint >= 2 || pagenum > 0 || fntnum > 0 || (doprint == 1 && nlines > 0) || uu > 0)
                              fprintf(outfile, "\n");


  if (pagenum > 0 && fntnum > 0) return 2;
  if (pagenum > 0 || fntnum > 0) return 1;
                                 return 0;
}


void allerrorout(const char *s)
{
   fprintf(outfile, "%s %s\n", TeXITEM, s);
   fprintf(outfile, "%s{BibArts warning: %s}%s\n", TYPEOUT, s, RELAX);
    printf("%%%%   -> %s\n", s);
}


static void timestamp (FILE *file)
{
  time_t t;
  struct tm *tp;

  if (time (&t) == -1) return;

    tp = localtime(&t);
  if (german == 0)
  fprintf (file, "%%%%>  Begin output (month-day-year)  %.2d-%.2d-%.4d  %.2d:%.2d:%.2d\n",
    1+tp->tm_mon,  tp->tm_mday, 1900+tp->tm_year,
      tp->tm_hour,  tp->tm_min,      tp->tm_sec);
  else
  fprintf (file, "%%%%>  Begin output (day.month.year)  %.2d.%.2d.%.4d  %.2d:%.2d:%.2d\n",
    tp->tm_mday, 1+tp->tm_mon, 1900+tp->tm_year,
    tp->tm_hour,   tp->tm_min,      tp->tm_sec);
}


void outhline(void)
{
      printf("%%%% ======================================\n");
}


void fprintsquence(FILE *file, const char *utf)
{
  while (*utf != '\0')
  {
    fprintf(file, "%d", ((unsigned char)*utf));
    if (*(utf+1) != '\0') fprintf(file, "-");
    ++utf;
  }
}


void fprint_utf_msg(FILE *file, const char utf[], int mod)
{
  if (encode == IS_UTFE_ENC && utf[0] < 0)
  { 
                  fprintf(file, "%%%%\n%%%%   -utf8 :  Did not ");
    if (mod == 0) fprintf(file, "know the meaning of \'%s\' (", utf);
    else        { fprintf(file, "interpret invalid utf8 sequences (like ");  /* msg changed in 2.5 */
                  if (utf[1] == '\0')
                  fprintf(file, "the single octet ");  /* msg changed in 2.5 */
                }
    fprintsquence(file, utf);
    fprintf(file, ").\n%%%%     Only the last error is reported here; also see screen.\n%%%%\n");
  }
}


void printfilelist(FILE *file, int filec, int cc, const char *vor, const char *nach)
{
    int i = 0;

  if (cc == 1)
  {
     fprintf(file, "\n%%%%   BibArts 2.5  (C) Timo Baumann  2022b   [%s]\n", __DATE__);
     fprintf(file, "\\gdef\\@bibsortvers{2.5}\n");  /* new in 2.5 */
  }

     fprintf(file, "%%%%\n");
   timestamp(file);

     fprintf(file, "%%%%   Sort arg(s):\n");
                                 fprintf(file, "%s", vor);
     if (german == 1)            fprintf(file, " -g1");
     if (german == 2)            fprintf(file, " -g2");
     if (nogsty == 1)            fprintf(file, " -x");
     if (ignorespaces == 1)      fprintf(file, " -l");
     if (hy_as_space  == 1)      fprintf(file, " -h");
     if (registpoints == 1)      fprintf(file, " -p");
     if (encode == IS_TONE_ENC)  fprintf(file, " -t1");
     if (encode == IS_UTFE_ENC)  fprintf(file, " -utf8");
     if (kill == DOKILLAUTHORS)  fprintf(file, " -k");
     if (ex_n == EXCHANGENAMES)  fprintf(file, " -e");
     if (EinlROEMseiten == 1)    fprintf(file, " -s1");
     if (ALPHmpfootnotes == 1)   fprintf(file, " -f1");
     if (    get_R_pg == BA_TRUE
          || get_A_pg == BA_TRUE
          || get_a_pg == BA_TRUE
          || get_r_pg == BA_TRUE
        )                        fprintf(file, " -s2 %s", soptarg);
     if (    get_R_fn == BA_TRUE
          || get_A_fn == BA_TRUE
          || get_a_fn == BA_TRUE
          || get_r_fn == BA_TRUE
        )                        fprintf(file, " -f2 %s", foptarg);
     if (igleichj == 1)          fprintf(file, " -i=j");
     if (bago == 1)              fprintf(file, " -b");
     if (FntIndexOverPages == 0) fprintf(file, " -n1");
     if (notext == 1)            fprintf(file, " -c");

     if (    german == 0
          && nogsty == 0
          && ignorespaces == 0
          && hy_as_space == 0
          && registpoints == 0
          && encode == IS_BASE_ENC
          && kill != DOKILLAUTHORS
          && ex_n == NOTEXCHANGENAMES
          && EinlROEMseiten == 0
          && ALPHmpfootnotes == 0
          && (get_R_fn == BA_FALSE && get_A_fn == BA_FALSE && get_a_fn == BA_FALSE && get_r_fn == BA_FALSE)
          && (get_R_pg == BA_FALSE && get_A_pg == BA_FALSE && get_a_pg == BA_FALSE && get_r_pg == BA_FALSE)
          && igleichj == 0
          && bago == 0
          && FntIndexOverPages == 1
          && notext == 0
        )                       fprintf(file, " Default");
                                fprintf(file, "%s\n", nach);

     if (encode == IS_BASE_ENC && thisupperascii < 0) fprintf(file, "%%%%\n%%%%   *** Codepage error: ASCIIs > 127 like %d found, but you did not say -utf8 or -t1.\n%%%%\n", ((unsigned char)thisupperascii));

     fprint_utf_msg(file, this_emp_utf_eight, 0);
     fprint_utf_msg(file, this_err_utf_eight, 1);

     if (igleichj == 1 && cc == 1) fprintf(file, "%%%%   \n%%%%   Note, that you will not see i=j in %s lines here.\n", METALINEHEAD); /* "-m" + "-i=j" */

     fprintf(file, "%%%%\n%%%%>  Input file list:\n");

   if (filec == 1)
     fprintf(file, "%%%%   only  \"%s\".\n",      filelist[i]);
   else
   while (i < filec)
   {
     fprintf(file, "%%%%   %.3d  \"%s\".\n", i+1, filelist[i]);
     ++i;
   }

  if (cc == 1)
     fprintf(file, "\n\n");
  else 
  {   printf("%%%%\n");
      outhline();
  }
}


int writelines(int nlines, const int dqcol, const int lancol, const int commentANDignlines, const int deepsort, int filec, int printsort, int kandkused, int mostoktitarg)
{
   int i = 0, dqcat, lancat = 0, lancatbuf, extralines = 0, didsetorig, nerr = 0;

   printfilelist(outfile, filec, 1, "\\gdef\\bibsortargs{", " }");

   while (i < nlines && leval[i] != ACCEPTLINE)
   { 
     nerr = nerr + writetheline(lineptr[i], 0, i, 0, filec, printsort, kandkused, mostoktitarg);
     ++i;
   }

   /* Wegen 2. Arg von abk ist hier dqsort ungeeignet! */

   if (nlines-commentANDignlines > 0)
   {
     if ((dqcol > -1 && nogsty == 0) || lancol > -1) fprintf(outfile, "\n");
     
     if (dqcol > -1 && nogsty == 0)
     { dqcat = getarabicnum(subRptr[dqcol][i]);
       if      (dqcat == TeXACTIVE)
       {  fprintf(outfile, "  \\ifnum\\catcode\\grave@ba\\\"=%d%s{\\message{ [bibsort] Begin list setting \\string%s. }}\\fi\n", TeXPASSIVE, GERMANTEX, GERMANTEX);
          ++extralines;
       }
       else if (dqcat == TeXPASSIVE)
       {  fprintf(outfile, "  \\ifnum\\catcode\\grave@ba\\\"=%d%s{\\message{ [bibsort] Begin list setting \\string%s. }}\\fi\n", TeXACTIVE, ORIGINALTEX, ORIGINALTEX);
          ++extralines;
       }
     }

     if (lancol > -1)
     { lancat = getarabicnum(subRptr[lancol][i]);          
          fprintf(outfile, "  \\ifnum\\language=%d\\else%s%d\\message{ [bibsort] Begin list setting hyphenation %d. }\\fi\n", lancat, HYPHENATION, lancat, lancat);
       ++extralines;
     }

     if ((dqcol > -1 && nogsty == 0) || lancol > -1) fprintf(outfile, "\n");

     nerr = nerr + writetheline(lineptr[i], nlines, i, deepsort, filec, printsort, kandkused, mostoktitarg);

     ++i;
   }
   else if (nlines > 0) allerrorout("bibsort did reject all your entries.");


   while (i < nlines)
   { 
          didsetorig = -1;
     if (dqcol > -1 && nogsty == 0 && leval[i] == ACCEPTLINE)
     { 
       if      (dqcat == TeXACTIVE  && (dqcat = getarabicnum(subRptr[dqcol][i])) == TeXPASSIVE)
       {  fprintf(outfile, "  %s{\\message{ [bibsort] Set \\string%s \\space in line %s \\space of BibArts file. }}%s\n", ORIGINALTEX, ORIGINALTEX, TEXFILELINE, RELAX);
          ++extralines;
          didsetorig = 0;
       }
       else if (dqcat == TeXPASSIVE && (dqcat = getarabicnum(subRptr[dqcol][i])) == TeXACTIVE)
       {  fprintf(outfile, "  %s{\\message{ [bibsort] Set \\string%s \\space in line %s \\space of BibArts file. }}%s\n", GERMANTEX, GERMANTEX, TEXFILELINE, RELAX);
          ++extralines;
          didsetorig = 1;
       }
     }

     if (lancol > -1 && leval[i] == ACCEPTLINE)
     { if (    lancat != (lancatbuf = getarabicnum(subRptr[lancol][i]))
            || didsetorig != -1
          )
       {       lancat  =  lancatbuf;
          fprintf(outfile, "  %s%d\\message{ [bibsort] Reproduce hyphenation %d in line %s \\space of BibArts file. }%s\n", HYPHENATION, lancat, lancat, TEXFILELINE, RELAX);
          ++extralines;
       }
     }

     nerr = nerr + writetheline(lineptr[i], nlines, i, deepsort, filec, printsort, kandkused, mostoktitarg);

     ++i;
   }

   if (nerr > 0)
   {
     printf("%%%%  !!!\n");
     printf("%%%%    I\'ve marked %d page/footnote number(s)\n", nerr);
     printf("%%%%    as      \"%s\",      \"%s\",   \"%s\", and \"%s...%s\" for\n", OVERFLOWSYMB, EMPTYSYMB, MISSINGERMS, TEXTSYMBOPEN[notext], TEXTSYMBCLOSE[notext]);
     printf("%%%%    counter overflow / empty / missing / invalid (TEXT).\n");
     if (notext == 0 && (strcmp(TEXTSYMBOPEN[0], TEXTSYMBOPEN[1]) != 0 || strcmp(TEXTSYMBCLOSE[0], TEXTSYMBCLOSE[1]) != 0))
     {
            printf("%%%%           Use opt -c to set invalids ");
       if (*TEXTSYMBOPEN[1] != '\0' || *TEXTSYMBCLOSE[1] != '\0')
            printf("as \"%s...%s\"", TEXTSYMBOPEN[1], TEXTSYMBCLOSE[1]);
       else printf("naked");
            printf(".\n");
     }
     printf("%%%%  !!!\n");
   }

   return extralines;
}


/*--------------------------------------------------------------------------*/


      /* Autoren, die mehrfach vorkommen, werden genannt */
      /* und ab dann durch P|IKILLNAME/NKILLNAME ersetzt */


int authorcmp(const char *s, const char *p, int dqcats, int dqcatp)
{
  int mlen = strlen(MALE),   flen = strlen(FEMALE), cont = 1;
  int hlen = strlen(HASDOT), nlen = strlen(NOTDOT);

  while(1)
  {
     if (lcmdcmp(s,   MALE)   == 0) { s += mlen; while (*s == ' ') ++s; cont = 0; }
     if (lcmdcmp(s, FEMALE)   == 0) { s += flen; while (*s == ' ') ++s; cont = 0; }
     if (lcmdcmp(s, HASDOT)   == 0) { s += hlen; while (*s == ' ') ++s; cont = 0; }
     if (lcmdcmp(s, NOTDOT)   == 0) { s += nlen; while (*s == ' ') ++s; cont = 0; }
     if (strncmp(s, "\\-", 2) == 0) { s += 2;                           cont = 0; }

     if (lcmdcmp(p,   MALE)   == 0) { p += mlen; while (*p == ' ') ++p; cont = 0; }
     if (lcmdcmp(p, FEMALE)   == 0) { p += flen; while (*p == ' ') ++p; cont = 0; }
     if (lcmdcmp(p, HASDOT)   == 0) { p += hlen; while (*p == ' ') ++p; cont = 0; }
     if (lcmdcmp(p, NOTDOT)   == 0) { p += nlen; while (*p == ' ') ++p; cont = 0; }
     if (strncmp(p, "\\-", 2) == 0) { p += 2;                           cont = 0; }

     if (nogsty == 0)
     {
       if (dqcats == TeXACTIVE)
       {
         if (strncmp(s, "\"\"", 2) == 0) { s += 2; cont = 0; }
         if (strncmp(s, "\"-",  2) == 0) { s += 2; cont = 0; }
       }
       if (dqcatp == TeXACTIVE)
       {
         if (strncmp(p, "\"\"", 2) == 0) { p += 2; cont = 0; }
         if (strncmp(p, "\"-",  2) == 0) { p += 2; cont = 0; }
       }
     }

     if (cont == 1)
     {
       if (*s != '\0' && *p == *s)
       { 
         if (*s == '\\' && *(s+1) != '\0' && *(p+1) == *(s+1)) { ++p; ++s; }
         else
         if (*s == '\"' && dqcats != dqcatp) break;
         ++p; ++s; 
       }
       else break;
     }
     else if (*s == '\0' || *p == '\0') break;

     cont = 1;
  }

  if (*p == '\0' && *s == '\0') return 0;
  return 1;
}


int texcmp(const char *s, const char *p, int dqcats, int dqcatp)
{
  while(1)
  {
       if (*s != '\0' && *p == *s)
       { 
         if (*s == '\\' && *(s+1) != '\0' && *(p+1) == *(s+1)) { ++p; ++s; }
         else
         if (*s == '\"' && dqcats != dqcatp) break;
         ++p; ++s; 
       }
       else break;
  }

  if (*p == '\0' && *s == '\0') return 0;
  return 1;
}


int lcmdcmp(const char *s, const char *p)
{
  int r = 1;
  
  while (*s != '\0')
  {
    if (*s == *p) { ++s; ++p; }
    else break;
    
    if (*p == '\0' && isLaTeXcmdletter(*s) == 0)
    {
      r = 0;
      break;
    }
  }

  return r;
}


int findFEmale(const char *s)
{
   int r = 0;

   while(*s != '\0')
   {
     if (*s == '\\')
     {
       if (*(s+1) == '\\') ++s;
       else
       if (    lcmdcmp(s,   MALE) == 0
            || lcmdcmp(s, FEMALE) == 0
          )                                                     
       { r = 1; 
         break; 
       }
     }
     ++s;
   }

   return r;
}


void baaddx_error(void)
{
  printf("%%%%   baaddx-error\n");
}


int insertfalse(const char *p, char a[], int i)
{
   int n;

   while (*p == ' ') ++p;   
   n = nextbracket(p, '{', '}');     if (n < 2) baaddx_error();
   while (n-- > 0 && i < MAXLEN) a[i++] = *p++;

   while (*p == ' ') ++p;
   if    (*p == '{' && i < MAXLEN) 
   { a[i++] = *p++;

     while (*p == ' ') ++p;
     n = nextbracket(p, '{', '}');   if (n < 2) baaddx_error();
     while (n-- > 0 && i < MAXLEN) a[i++] = *p++;
   
     while (*p == ' ') ++p;
     n = nextbracket(p, '{', '}');   if (n < 2) baaddx_error();
     while (n-- > 0) p++;

     if (i + strlen(BAFALSE) < MAXLEN)
     {
       a[i] = '\0'; strcat(a, BAFALSE); i += strlen(BAFALSE);
     }
   }
   else baaddx_error();

   while (*p != '\0' && i < MAXLEN) a[i++] = *p++;

   return i;
}


int count_rejected(int nlines)
{
  int ret = 0, i = 0;

  while (i < nlines)
  {
     if (leval[i] == REJECTLINE)
       ++ret;    /* parallel zu double_or_empty_abbreviation */
    ++i;
  }

  return ret;
}


int checkifabbrisdefined(int nlines, int ret, char *Thead, const char *Thold, int Thint, int i)
{
    int found = 2, pwgt, fwgt; 
    char *pg, *fnt, *tline;

          if (          leval[i] != REJECTLINE
               && *subAptr[1][i] == '\0'
               && (    *Thold    == '\0'
                    || texcmp(subAptr[0][i], Thead, dqsort[i], Thint) != 0
                  )
             )
          {
            leval[i] = UNDEFILINE; 
              ++ret;
            found = 0;
          }


   Thead = subAptr[0][i]; 
   Thint = dqsort[i];
   pg    = subRptr[PAGECOL][i];
   fnt   = subRptr[FNTCOL][i];
   tline = subRptr[LINECOL][i];
   pwgt  = pagewgt[i];
   fwgt  = fntwgt[i];

   if (found == 0)
   {
     while (++i < nlines && texcmp(subAptr[0][i], Thead, dqsort[i], Thint) == 0)
     {
          if (          leval[i] != REJECTLINE
               && *subAptr[1][i] != '\0'
             )
             { found = 1;
               break;
             }
     }
   }

   if (found == 0)
   {  printf("%%%%\n%%%%>   Warning: Abbreviation \"%s\" is NEVER defined!\n", Thead);
            printf("%%%%     The entry (%s", tline);
                   if (Thint > -1) printf(", \":%d", Thint);
            printf(") is rejected. Use %s\?\n", ABKDEF);
   }

   if (found == 1 && 
         (    strcmp(tline, subRptr[LINECOL][i]) != 0
           || strcmp(  fnt, subRptr[FNTCOL][i] ) != 0
         )
      )
   {
                           printf("%%%%\n%%%%>   Warning: Abbreviation \"%s\" is used in\n", Thead);
                           printf("%%%%     ");
         if (fnt != nofnt) 
         {                 printf("A FNT ");
           if (fwgt != fntwgt[i] && subRptr[FNTCOL][i] != nofnt && strcmp(pg, subRptr[PAGECOL][i]) == 0)
                           printf("T%d ", fwgt);
         }
                           printf("%s and def in ", tline);
     if (subRptr[FNTCOL][i] != nofnt)
     {
                           printf("A FNT ");
           if (fwgt != fntwgt[i] &&                fnt != nofnt && strcmp(pg, subRptr[PAGECOL][i]) == 0)
                           printf("T%d ", fntwgt[i]);
     }
                           printf("%s!\n", subRptr[LINECOL][i]);
   }

   return ret;
}


int entkerne_IeC(char buf[], const char *s)
{
  return entkerne_arg(buf, s, IeC, '{', '}');
}


void printdline(int len, const char *s)
{
   printf("\"");
   maxprint(len, s);
   printf("\"");
}


void printD(const char *txt, const char *head, const char *Thold, const char *Trej, int i, int mem, int nmsg)
{
  int prnlen = 0;

  if (nmsg == 0)
  {
    printf("%%%%\n%%%%>   Warning: Different %s ", txt);
    printdline(50 - strlen(txt), head);
    printf(":\n");

    printf("%%%%     *Accept ");
    if (mem > -1) 
    { 
      printf("%s ", subRptr[LINECOL][mem]);
      prnlen = 60 - strlen(subRptr[LINECOL][mem]);
    }
    printdline(prnlen, Thold);
    printf(";\n");
  }

    printf("%%%%     *Reject %s ", subRptr[LINECOL][i]);

  if (strcmp(Thold, Trej) == 0)
       printf("(same string with other dq-catcode %d)", getarabicnum(subRptr[DQCOL][i]));
  else printdline(60 - strlen(subRptr[LINECOL][i]), Trej);

  printf(".\n");
}


int twoargsSTUFFcmp(const char *txt, int i, int mem, int messn)
{
  char a_buf[MAXLEN], b_buf[MAXLEN];

  if (mem > -1)
  {
    getfirsttwoargs(a_buf, subRptr[STUFFRCOL][i]);
    getfirsttwoargs(b_buf, subRptr[STUFFRCOL][mem]);

    if (texcmp(a_buf, b_buf, getarabicnum(subRptr[DQCOL][i]), getarabicnum(subRptr[DQCOL][mem])) != 0)
    { 
      if (messn < MAXMESSN)
      {
        printD(txt, subAptr[0][i], b_buf, a_buf, i, mem, messn);
      }
      ++messn;
    }
  }

  return messn;
}


int xstrcmp(const char *a, const char *b, int mem, int i)
{
   if ( (    nogsty == 1
          || (mem > -1 && getarabicnum(subRptr[DQCOL][i]) == getarabicnum(subRptr[DQCOL][mem]))
          || userusesdq(PCOLS, i) == 0
        )
        && strcmp(a, b) == 0
      ) return 0;
        return 1;
}


void printT(int i)
{
   printf("%%%%      ");      
   print_two(i, 0, 1);
   printf("\n");
}


void warnFEmaleT(int nlines)
{
  int i = 0;

  while (i < nlines)
  {
    if (    findFEmale(subAptr[0][i]) != 0
         || findFEmale(subAptr[1][i]) != 0
       )
    {
      printf("%%%%\n%%%%>   Warning: %s or %s in entry %s:\n", FEMALE, MALE, subRptr[LINECOL][i]);
      printT(i);
    }
    ++i;
  }
}


void checkSTUFFblock(const char *msg, int i, const int nlines, const int deepsort, const int deep)
{
     const int mem = i;
     const char *TheadA = subAptr[0][mem];
     const char *TheadB = subAptr[1][mem];
     const char *TheadC = subAptr[2][mem];
     const char *TheadD = subAptr[3][mem];
     const int   Thint  =     dqsort[mem];
     int messn = 0;


  if (    deepsort != deep 
       && deepsort != deep-1
     ) 
  { 
    intern_err(20);  /* deep/deepsort */
  }
  else
  {
     while (++i < nlines)
     {
       if (leval[i] == REJECTLINE);
       else
       if (    (deepsort > 0 && texcmp(subAptr[0][i], TheadA, dqsort[i], Thint) != 0)
            || (deepsort > 1 && texcmp(subAptr[1][i], TheadB, dqsort[i], Thint) != 0)
            || (deepsort > 2 && texcmp(subAptr[2][i], TheadC, dqsort[i], Thint) != 0)
            || (deepsort > 3 && texcmp(subAptr[3][i], TheadD, dqsort[i], Thint) != 0)
          )
       {
          break;
       }
       else if (    deepsort == deep 
                 || (*subAptr[deep-1][i] != '\0' && texcmp(subAptr[deep-1][i], subAptr[deep-1][mem], getarabicnum(subRptr[DQCOL][i]), getarabicnum(subRptr[DQCOL][mem])) == 0)
               )
       {
            messn = twoargsSTUFFcmp(msg, i, mem, messn);
       }
     }

     if (messn > MAXMESSN) printf("%%%%    *** Report the first %d different seps (of %d) for that entry. ***\n", MAXMESSN, messn);
  }
}


int double_or_empty_abbreviation(int nlines, int deepsort, int deep)
{
        int i, ret = 0, Thint = -1, mem = -1;
        char *Thead = "", *Thold = "", *Thart = NULL, *oldmsg = NULL, *oldstuffA = NULL, *oldstuffB = NULL;

    /* dqsort geht bis zur Tiefe deepsort, nicht deep */


        warnFEmaleT(nlines);


        i = 0; while (i < nlines)
        {
          if (leval[i] == REJECTLINE)
          {
            ++ret;       /* Anzahl bereits auskommentierter Zeilen */
          }
          else
          if (*subAptr[0][i] == '\0')
          {
            leval[i] = REJECTLINE;  
            printf("%%%%\n%%%%>   Reject empty abbreviation in %s!\n", subRptr[LINECOL][i]);
            ++ret;
          }
          ++i;
        }


          ret = checkifabbrisdefined(nlines, ret, Thead, Thold, Thint, 0);

        i = 1; while (i < nlines)
        { 
          if (          leval[i-1] != REJECTLINE
               && *subAptr[0][i-1] != '\0'
               && *subAptr[1][i-1] != '\0'
             )
          {
             if (    *Thold        == '\0'
                  || texcmp(subAptr[0][i-1], Thead, dqsort[i-1], Thint) != 0
                ) 
             { Thold = subAptr[1][i-1];
               mem   = i-1;
               Thart = subRptr[FNTCOL][i-1];
             }
             else
             if (Thart != nofnt && subRptr[FNTCOL][i-1] == nofnt)
             {
               Thart = subRptr[FNTCOL][i-1];
             }
               Thead = subAptr[0][i-1];
               Thint = dqsort[i-1];
          }

          if (          leval[i] != REJECTLINE
               && *subAptr[0][i] != '\0'
               && *subAptr[1][i] == '\0'
               &&          Thart != NULL
               &&          Thart != nofnt
               && subRptr[FNTCOL][i] == nofnt
               && texcmp(subAptr[0][i], Thead, dqsort[i], Thint) == 0
             )
          {
             if (oldmsg == NULL || strcmp(oldmsg, subAptr[0][i]) != 0)
             printf("%%%%\n%%%%>   Warning: Abbreviation \"%s\" is used in\n", subAptr[0][i]);
             printf("%%%%     TEXT %s, whereas all foregoing defs were in FNTs!\n", subRptr[LINECOL][i]);
             oldmsg = subAptr[0][i];
          }

          ret = checkifabbrisdefined(nlines, ret, Thead, Thold, Thint, i);

          if (          leval[i] != REJECTLINE
               && *subAptr[1][i] == '\0'
               && *Thold         != '\0'
               && texcmp(subAptr[0][i], Thead, dqsort[i], Thint) == 0
             )
          {
            leval[i] = ABBREVLINE; 
              ++ret;
          }

          if (          leval[i] != REJECTLINE
               && *subAptr[1][i] != '\0'
               && *Thead         != '\0'
               && texcmp(subAptr[0][i], Thead, dqsort[i], Thint) == 0
             )
          {
            if (xstrcmp(Thold, subAptr[1][i], mem, i) != 0)  /* leval[i] sagt nichts zu Arg2 */
            { 
              printD("defs for abbreviation", subAptr[0][i], Thold, subAptr[1][i], i, mem, 0);
              leval[i] = DIFFERLINE; 
                ++ret;
            }
            else 
            { 
              leval[i] = DOUBLELINE;
                ++ret;
              /* printf("%%%%\n%%%%>   Reject repeated def for abbreviation \"%s\" -> \"%s\"!\n", subAptr[0][i], subAptr[1][i]);
               */
              if (    (oldstuffA == NULL && oldstuffB == NULL)
                   || strcmp(oldstuffA, subAptr[0][mem]) != 0
                   || strcmp(oldstuffB, subAptr[1][mem]) != 0
                 )
              checkSTUFFblock("abkdef-open/-close seps after", mem, nlines, deepsort, deep);
                             oldstuffA = subAptr[0][mem]; 
                             oldstuffB = subAptr[1][mem]; 
            }
          }
          ++i;
        }

  return ret;
}


void printcline(int len, const char *s)
{
   printf("(");
   maxprint(len, s);
   printf(")  ");
}



#define LMAX 26
#define RMAX 36

void print_two(int i, int e, int z)
{
   char bufa[MAXLEN], bufb[MAXLEN];
   int llen, rlen;

   llen = entkerne_IeC(bufa, subAptr[e][i]);
   rlen = entkerne_IeC(bufb, subAptr[z][i]);
   
   if (llen > LMAX && rlen > RMAX)
   {
     rlen = RMAX;
     llen = LMAX;
   }
   else
   if (llen + rlen > LMAX + RMAX)
   {
     if (llen > LMAX) llen = LMAX+RMAX-rlen;
     if (rlen > RMAX) rlen = LMAX+RMAX-llen;
   }

   printcline(llen, bufa);
   printcline(rlen, bufb);
}


#define LEFTMAX  16
#define MIDMAX   18
#define RIGHTMAX 24

void print_three(int i, int e, int z, int d)
{
   char bufa[MAXLEN], bufb[MAXLEN], bufc[MAXLEN];
   int llen, mlen, rlen, vari; 

   llen = entkerne_IeC(bufa, subAptr[e][i]);
   mlen = entkerne_IeC(bufb, subAptr[z][i]);
   rlen = entkerne_IeC(bufc, subAptr[d][i]);
   
   if (llen > LEFTMAX && mlen > MIDMAX && rlen > RIGHTMAX)
   {
     rlen = RIGHTMAX;
     mlen = MIDMAX;
     llen = LEFTMAX;
   }
   else
   if (llen + mlen + rlen > LEFTMAX + MIDMAX + RIGHTMAX)
   {
     if (rlen <= RIGHTMAX && mlen <= MIDMAX)
     {
       llen =  LEFTMAX + RIGHTMAX-rlen +  MIDMAX-mlen;
     }
     else
     if (mlen <= MIDMAX && llen <= LEFTMAX)
     {
       rlen = RIGHTMAX +   MIDMAX-mlen + LEFTMAX-llen;
     }
     else
     if (rlen <= RIGHTMAX && llen <= LEFTMAX)
     {
       mlen =   MIDMAX + RIGHTMAX-rlen + LEFTMAX-llen;
     }
     else
     if (rlen <= RIGHTMAX)
     {
       vari = RIGHTMAX-rlen;
        if (llen > LEFTMAX + vari && mlen > MIDMAX)
        {
           llen =  LEFTMAX + vari;
           mlen =   MIDMAX;
        }
        else
        if (mlen + llen > MIDMAX + LEFTMAX + vari)
        {
          if (llen >  LEFTMAX+vari) llen =   MIDMAX+ LEFTMAX+vari-mlen;
          if (mlen >   MIDMAX)      mlen =   MIDMAX+ LEFTMAX+vari-llen;
        }
     }
     else
     if (llen <=  LEFTMAX)
     {
       vari =  LEFTMAX-llen;
        if (mlen >   MIDMAX + vari && rlen > RIGHTMAX)
        {
            mlen =   MIDMAX + vari;
            rlen = RIGHTMAX;
        }
        else
        if (mlen + rlen > MIDMAX + RIGHTMAX + vari)
        {
          if (mlen >   MIDMAX+vari) mlen =  MIDMAX+RIGHTMAX+vari-rlen;
          if (rlen > RIGHTMAX)      rlen =  MIDMAX+RIGHTMAX+vari-mlen;
        }
     }
     else
     if (mlen <=   MIDMAX)
     {
       vari =   MIDMAX-mlen;
        if (llen >  LEFTMAX + vari && rlen > RIGHTMAX)
        {
            llen =  LEFTMAX + vari;
            rlen = RIGHTMAX;
        }
        else
        if (rlen + llen > RIGHTMAX + LEFTMAX + vari)
        {
          if (llen >  LEFTMAX+vari) llen = RIGHTMAX+LEFTMAX+vari-rlen;
          if (rlen > RIGHTMAX)      rlen = RIGHTMAX+LEFTMAX+vari-llen;
        }
     }
   }

 if (llen + mlen + rlen > LEFTMAX + MIDMAX + RIGHTMAX)
   intern_err(21);  /* Bad screen print */

   printcline(llen, bufa);
   printcline(mlen, bufb); 
   printcline(rlen, bufc);
}


#define MAXA 12
#define MAXB  8
#define MAXC 16
#define MAXD 18
#define ALLMAX (MAXA+MAXB+MAXC+MAXD)

void print_four(int i)
{
   char bufa[MAXLEN], bufb[MAXLEN], bufc[MAXLEN], bufd[MAXLEN];
   int lena, lenb, lenc, lend;
   int over, overa = 0, overb = 0, overc = 0, overd = 0;

   lena = entkerne_IeC(bufa, subAptr[1][i]);   /*  (Vorname)  */
   lenb = entkerne_IeC(bufb, subAptr[0][i]);
   lenc = entkerne_IeC(bufc, subAptr[2][i]);
   lend = entkerne_IeC(bufd, subAptr[3][i]);

   if (lena + lenb + lenc + lend > ALLMAX)
   {
     if (lena < MAXA) overa = MAXA - lena;
     if (lenb < MAXB) overb = MAXB - lenb;
     if (lenc < MAXC) overc = MAXC - lenc;
     if (lend < MAXD) overd = MAXD - lend;

     over = overa + overb + overc + overd;
     overa = 0; overb = 0; overc = 0; overd = 0;

     while (over > 0)
     {
       if (lena-overa > MAXA && over > 0) { ++overa; --over; }
       if (lenb-overb > MAXB && over > 0) { ++overb; --over; }
       if (lenc-overc > MAXC && over > 0) { ++overc; --over; }
       if (lend-overd > MAXD && over > 0) { ++overd; --over; }
     }

     over = overa + overb + overc + overd;

     while(lena + lenb + lenc + lend + over > ALLMAX)
     {
       if (lena + lenb + lenc + lend + over > ALLMAX && lena > MAXA) --lena;
       if (lena + lenb + lenc + lend + over > ALLMAX && lenb > MAXB) --lenb;
       if (lena + lenb + lenc + lend + over > ALLMAX && lenc > MAXC) --lenc;
       if (lena + lenb + lenc + lend + over > ALLMAX && lend > MAXD) --lend;
     }
   }

   printcline(lena+overa, bufa);   /*  (Vorname)  */
   printcline(lenb+overb, bufb);
   printcline(lenc+overc, bufc);
   printcline(lend+overd, bufd);
}


void fitssingleentry(int i, int nlines, int arq, const char *suffix)
{
  int l;

  if (    leval[i] != REJECTLINE
       && *subAptr[1][i] != '\0'
       && (arq != IS_Q || getthirdnum(i) == 0)
     )
  {
        l  = i;
      while (l > 0) 
      { 
             if (leval[l-1] == REJECTLINE);
        else if (texcmp(subAptr[0][l-1], subAptr[0][i], dqsort[l-1], dqsort[i]) != 0) break;
        else if (*subAptr[1][l-1] == '\0') { --l; break; }
        --l;
      }

    if (leval[l] == REJECTLINE || *subAptr[1][l] != '\0')
    {
      while (l < nlines-1) 
      { 
             if (leval[l+1] == REJECTLINE);
        else if (texcmp(subAptr[0][l+1], subAptr[0][i], dqsort[l+1], dqsort[i]) != 0) break;
        else if (*subAptr[1][l+1] == '\0') { ++l; break; }
        ++l;
      }
    }

    if (leval[l] == REJECTLINE || *subAptr[1][l] != '\0')
    { 
      printf("%%%%\n%%%%>   Info: \\fill%s in %s fits NO single entry:\n", suffix, subRptr[LINECOL][i]);
      printT(i);
    }
  }
}


void misplaced_heading(int heada, int line)
{
  printf("%%%%\n%%%%>   Warning: Misplaced %d-heading (from %s):\n", heada, subRptr[LINECOL][line]);
  printT(line);
}


int double_or_empty_arqperregisters(int nlines, int arq, const char *suffix, int deepsort, int deep)
{
        int i, ret = 0, ThintA = -1, mem = -1, lastmem = mem, len, heada, headb;
        char *TheadA = "", *heredefault = "", *TheadB = heredefault, *oldstuffA = NULL, *oldstuffB = NULL;


        warnFEmaleT(nlines);


        i = 0; while (i < nlines)
        {
          if (leval[i] == REJECTLINE)
          {
            ++ret;       /* Anzahl bereits auskommentierter Zeilen */
          }
          else
          if (*subAptr[0][i] == '\0')
          {
            printf("%%%%\n%%%%>   Warning: Empty entry in %s", subRptr[LINECOL][i]);
            if (*subAptr[1][i] != '\0')
            { 
              if (arq == IS_Q && (heada=getthirdnum(i)) > 0)
                   printf(" (%d-heading)", heada);
              else printf(" (\\fill%s-cmd)", suffix);
            }
            printf(".\n");
            leval[i] = REJECTLINE;
            ++ret;
          }
          ++i;
        }


        fitssingleentry(0, nlines, arq, suffix);


        i = 1; while (i < nlines)
        { 
          if (          leval[i-1] != REJECTLINE
               && *subAptr[0][i-1] != '\0'
               && texcmp(subAptr[0][i-1], TheadA, dqsort[i-1], ThintA) != 0
             )
          {
               TheadA = subAptr[0][i-1];
               ThintA =     dqsort[i-1];

            if (pagewgt[i-1] == UNUSED)
            {
               mem = i-1;
               TheadB = subAptr[1][i-1];
            }
            else
            {
               mem = -1;
               TheadB = heredefault;
            }
          }

          /* da deepsort < deep wird subAptr[1][i] von dqsort nicht erfasst */
          if (    leval[i] != REJECTLINE
               && *TheadA  != '\0'
               && texcmp(subAptr[0][i], TheadA, dqsort[i], ThintA) == 0
             )
          {
            if (*subAptr[1][i] == '\0')
            {
                   leval[i] = DOUBLELINE;
            }
            else
            {
              if (xstrcmp(TheadB, subAptr[1][i], mem, i) == 0)  /* leval[i] sagt nichts zu Arg2 */
              {
                   leval[i] = REDEF2LINE;
                if (    (oldstuffA == NULL && oldstuffB == NULL)
                     || strcmp(oldstuffA, subAptr[0][mem]) != 0
                     || strcmp(oldstuffB, subAptr[1][mem]) != 0
                   )
                checkSTUFFblock("open/close seps after", mem, nlines, deepsort, deep);
                              oldstuffA = subAptr[0][mem]; 
                              oldstuffB = subAptr[1][mem]; 
              }
              else
              {
                   printD("fills for head", subAptr[0][i], TheadB, subAptr[1][i], i, mem, 0);
                     if (arq == IS_Q && getthirdnum(i) == 1)
                        printf("%%%%     ...  A list-internal heading is shadowed.\n");
                   leval[i] = DIFFERLINE;
              }
            }

                ++ret;
          }

          
          if (    arq == IS_Q 
               && (    leval[i] == ACCEPTLINE
                    || leval[i] == REDEF2LINE
                    || leval[i] == DOUBLELINE
                  )
             )
          {
            if (    mem > lastmem 
                 && (heada=getthirdnum(mem)) > 0
               )
            {
                len = strlen(TheadA);
              headb = getthirdnum(i);
              if (    strncmp(subAptr[0][i], TheadA, len) != 0
                   || len  >= strlen(subAptr[0][i])
                   || (headb < heada && headb > 0)
                 )
              { 
                misplaced_heading(heada, mem);
                      printf("%%%%     ...  That");
                if (texcmp(subAptr[0][i], TheadA, dqsort[i], ThintA) == 0)
                      printf(" melts into ");
                else
                {  
                      printf("\'s before ");
                  if (strcmp(subAptr[0][i], TheadA) == 0) printf("[\"!] ");
                }
                if (headb > 0)
                {
                     printf("%d-heading", headb);
                     printf(" (from %s):\n", subRptr[LINECOL][i]);
                     printT(i);
                }
                else printf("list item (from %s).\n", subRptr[LINECOL][i]);
              }
            }
            lastmem = mem;
          }


          fitssingleentry(i, nlines, arq, suffix);

          ++i;
        }


  return ret;
}


void printV(int i)
{
   printf("%%%%  ");
 if (dqsort[i] == TeXACTIVE)
   printf(    "!\"  ");
 else
   printf(    "    ");

 if (*subAptr[2][i] == '\0')
 {
   print_three(i, 1, 0, 3);   /*  (Vorname)  (Nachname)  (Rest)  */
 }
 else
 {
   print_four(i);
 }
   printf("\n");
}


void printK(int i)
{
   printf("%%%%  ");
 if (dqsort[i] == TeXACTIVE)
   printf(    "!\"  ");
 else
   printf(    "    ");

 if (*subAptr[1][i] == '\0')
 {
   print_two(i, 0, 2);
 }
 else
 {
   print_three(i, 0, 1, 2);
 }
   printf("\n");
}


int checkifisfullcited(int i, int nlines, int ret)
{
        char *TheadA = subAptr[0][i];
        char *TheadB = subAptr[1][i];
        char *TheadC = subAptr[2][i];
        char *TheadD = subAptr[3][i];
        int   Thint  =     dqsort[i], mem = i, j = i, cnt = 0, rest = nlines;




   if (    leval[i]   != REJECTLINE
        && isvintroduced(i) == KENTRY
      )
   {
     printf("%%%%\n%%%%>   Info: Short-%s-title %s is NOT yet introduced.\n", subAptr[3][i], subRptr[LINECOL][i]);
     printK(i);


     while (++i < nlines)
     {
       if (leval[i] == REJECTLINE);
       else
       if (    texcmp(subAptr[0][i], TheadA, dqsort[i], Thint) != 0
            || texcmp(subAptr[1][i], TheadB, dqsort[i], Thint) != 0
            || texcmp(subAptr[2][i], TheadC, dqsort[i], Thint) != 0
            || texcmp(subAptr[3][i], TheadD, dqsort[i], Thint) != 0
          )
          break;
       else
       if (isvintroduced(i) == VENTRY)
       {
         if (subRptr[FNTCOL][j] != nofnt && subRptr[FNTCOL][i] != nofnt && fntwgt[j] != fntwgt[i] && strcmp(subRptr[PAGECOL][j], subRptr[PAGECOL][i]) == 0)
         printf("%%%%       Full titel %s and short title in FNTs T%d T%d.\n", subRptr[LINECOL][i], fntwgt[i], fntwgt[j]);
         else
         printf("%%%%       ...  Exchange it with the full-title in %s.\n", subRptr[LINECOL][i]);
         mem = i;

         leval[j] = DOUBLELINE;
           ++ret;

          break;
       }
       else
       {
         leval[i] = DOUBLELINE;
           ++ret; 
       }
     }
     if (mem == j)
     {
         if (findFEmale(subAptr[0][mem]) != 0 || findFEmale(subAptr[1][mem]) != 0 || findFEmale(subAptr[2][mem]) != 0)
         printf("%%%%       ...  Delete misplaced cmd %s or %s.\n", FEMALE, MALE);
         else
         printf("%%%%       ...  Change that short-title into missing full-title (%s)\?\n", KTIT);
     }

     rest = i;
   }

   i = mem;
   if (    leval[i]   != REJECTLINE
        && isvintroduced(i) == VENTRY
      )
   {
     while (++i < nlines)
     {
       if (leval[i] == REJECTLINE);
       else
       if (    texcmp(subAptr[0][i], TheadA, dqsort[i], Thint) != 0
            || texcmp(subAptr[1][i], TheadB, dqsort[i], Thint) != 0
            || texcmp(subAptr[2][i], TheadC, dqsort[i], Thint) != 0
            || texcmp(subAptr[3][i], TheadD, dqsort[i], Thint) != 0
          )
          break;
       else
       if (isvintroduced(i) == VENTRY)
       {
         ++cnt;
         if (cnt == 1)
           printf("%%%%\n%%%%>   Info: ");
         else if (cnt < MAXINFO)
           printf("%%%%>    ");
         if (cnt < MAXINFO)
         {
              printf("Re-introduction of ");   /* intro cnt+1 */
           if (cnt > 1) printf("that ");
                printf("short-%s-title in %s.\n", subAptr[3][i], subRptr[LINECOL][i]);
         }
         if (cnt == 1)
         {
           printK(i);
           printf("%%%%    First introduction has been in %s (%s).\n", subRptr[LINECOL][mem], KTIT);
         }

         leval[i] = REDEF1LINE;
           ++ret;
       }
       else
       {
         leval[i] = DOUBLELINE;
           ++ret; 
       }
     }

     if (cnt+1 > MAXINFO)
       printf("%%%%>   This have been the first %d (of %d) intros of that short-title.\n", MAXINFO, cnt+1);

     rest = i;
   }

     i = rest;
   while (++i < nlines)
   {
     if (leval[i] == REJECTLINE);
     else
     if (    texcmp(subAptr[0][i], TheadA, dqsort[i], Thint) == 0
          && texcmp(subAptr[1][i], TheadB, dqsort[i], Thint) == 0
          && texcmp(subAptr[2][i], TheadC, dqsort[i], Thint) == 0
          && texcmp(subAptr[3][i], TheadD, dqsort[i], Thint) == 0
        )
     {
         leval[i] = DOUBLELINE;
           ++ret;
     }
     else break;          
   }

   return ret;
}


void checkmanipulationk(int i)
{
   
   if (    leval[i]   != REJECTLINE
        && isvintroduced(i) == ERRENTRY
      )
   {
     printf("%%%%\n%%%%>   Warning: Reference %s has unknown status.\n", subRptr[LINECOL][i]);
     printK(i);
   }
}


void checkshortmismatch(int i, const int nlines, const int mem)
{
  while (i < nlines)
  {
   if (    texcmp(subAptr[0][i], subAptr[0][mem], dqsort[i], dqsort[mem]) == 0
        && texcmp(subAptr[1][i], subAptr[1][mem], dqsort[i], dqsort[mem]) == 0
        && texcmp(subAptr[2][i], subAptr[2][mem], dqsort[i], dqsort[mem]) == 0
        && texcmp(subAptr[3][i], subAptr[3][mem], dqsort[i], dqsort[mem]) != 0
      )
   {
     printf("%%%%\n%%%%>   Warning: SAME short-title used for ...li and ...qu:\n");
     printK(i);
           printf("%%%%    See %s in %s and %s in %s.\n", subAptr[3][i], subRptr[LINECOL][i], subAptr[3][mem], subRptr[LINECOL][mem]);
   }
   else break;

   ++i;
  }
}


int double_k(int nlines, int deepsort)
{
        int i, ret = 0, mem = 0;
        char *TheadA = subAptr[0][0];
        char *TheadB = subAptr[1][0];
        char *TheadC = subAptr[2][0];
        char *TheadD = subAptr[3][0];
        int   Thint  =     dqsort[0];


        i = 0; while (i < nlines)
        {
          if (    findFEmale(subAptr[0][i]) != 0
               || findFEmale(subAptr[1][i]) != 0
               || findFEmale(subAptr[2][i]) != 0
             )
          {
            printf("%%%%\n%%%%>   Warning: %s or %s in short-%s-title %s:\n", FEMALE, MALE, subAptr[3][i], subRptr[LINECOL][i]);
            printK(i);
            if (isvintroduced(i) == VENTRY)
                  printf("%%%%       ...  That k-entry was generated automatically from a v-entry.\n");
          }
          ++i;
        }

        i = 0; while (i < nlines)
        {
          if (leval[i] == REJECTLINE)
          {
            ++ret;       /* Anzahl bereits auskommentierter Zeilen */
          }
          ++i;
        }

        ret = checkifisfullcited(0, nlines, ret);
              checkmanipulationk(0);

        i = 1; while (i < nlines)
        { 
          if (    leval[i-1] != REJECTLINE
               && (
                       texcmp(subAptr[0][i], TheadA, dqsort[i], Thint) != 0
                    || texcmp(subAptr[1][i], TheadB, dqsort[i], Thint) != 0
                    || texcmp(subAptr[2][i], TheadC, dqsort[i], Thint) != 0
                    || texcmp(subAptr[3][i], TheadD, dqsort[i], Thint) != 0
                  )
             )
          {
               TheadA = subAptr[0][i];
               TheadB = subAptr[1][i];
               TheadC = subAptr[2][i];
               TheadD = subAptr[3][i];
               Thint  =     dqsort[i];
                     ret = checkifisfullcited(i, nlines, ret);
                           checkshortmismatch(i, nlines, mem);
                                                         mem = i;
            checkSTUFFblock("vkc-seps after", i, nlines, deepsort, deepsort);
          }

               checkmanipulationk(i);

          ++i;
        }

  return ret;
}


int doubleventries(int nlines, int deepsort)
{
  int ret = 0, i = 1, alast = -1, ainfos = 0, aget = 0, mem = -1, difftitle = 0, shown = 0;

  /* geht davon aus, dass count_rejected nur REJECTLINE behandelt */
  /*   und doubleauthor commentANDignlines unveraendert laesst!   */

  while (i < nlines)
  {
    if (    texcmp(subAptr[0][i], subAptr[0][i-1], dqsort[i], dqsort[i-1]) == 0
         && texcmp(subAptr[1][i], subAptr[1][i-1], dqsort[i], dqsort[i-1]) == 0
         && texcmp(subAptr[2][i], subAptr[2][i-1], dqsort[i], dqsort[i-1]) == 0
         && texcmp(subAptr[3][i], subAptr[3][i-1], dqsort[i], dqsort[i-1]) == 0
       )
    {
      if (mem < 0) mem = i-1;
      
      if (alast < i-1)
      { 
        if (ainfos < MAXINFO)
        { 
          printf("%%%%\n%%%%>   Info: Identical full-titles %s, %s:\n", subRptr[LINECOL][i-1], subRptr[LINECOL][i]);
          printV(i);
            ainfos+=2;
          ++shown;
        }
        else
        ++difftitle;
            aget  +=2;
      }
      else 
      {  
        if (ainfos < MAXINFO)
        {
          printf("%%%%>      ... and the same entry again from %s.\n", subRptr[LINECOL][i]);
            ainfos+=1;
        }
            aget  +=1;
      }
      alast = i;

       
        if (    leval[i]   != REJECTLINE
             && leval[i-1] != REJECTLINE
           )
        {
          leval[i] = DOUBLELINE;
          ++ret;

          if (mem == i-1) checkSTUFFblock("v-seps after", mem, nlines, deepsort, deepsort);
        }
    }
    else mem = -1;

    ++i;
  }

  if (ainfos > 1)
  {
    printf("%%%%\n%%%%>   This have been the first %d (of %d) entries", ainfos, aget);
    if (shown > 1)
        printf(" (%d different titles)", shown);
        printf("\n%%%%      which are repeatingly cited");
    if (aget > MAXINFO)
    printf(" (further %d different titles)", difftitle);
    printf(".\n%%%%\n");
  }
    
  return ret;
}


int doubleauthor(const char *entry, int nlines)
{
        int i, num = 0, len = strlen(entry), killpos, hint, Adq;
        char a[MAXLEN], *p, xx[1], *AAp, *ABp, *ACp;

        strcpy(a, entry);

        i = 0; while (i < nlines)
        {
          if (findFEmale(subAptr[0][i]) != 0)
          { printf("%%%%\n%%%%>   Warning: %s or %s in lastname instead firstname %s:\n", FEMALE, MALE, subRptr[LINECOL][i]);
            printV(i);
          }
          if (findFEmale(subAptr[2][i]) != 0)
          { printf("%%%%\n%%%%>   Warning: %s or %s in OptArg %s:\n", FEMALE, MALE, subRptr[LINECOL][i]);
            printV(i);
          }
          ++i;
        }

        while(--nlines > 0)
        {
          if (*subAptr[0][nlines] == '\0' && *subAptr[1][nlines] == '\0') continue;
          if (leval[nlines] == REJECTLINE) continue;

          hint = nlines-1;
          while(hint >= 0 && leval[hint] == REJECTLINE) --hint;
            if (hint  < 0) break;
          AAp  = subAptr[0][hint];
          ABp  = subAptr[1][hint];
          Adq  =     dqsort[hint];
          ACp  = subAptr[2][hint];

          if (    authorcmp(subAptr[0][nlines], AAp, dqsort[nlines], Adq) == 0
               && authorcmp(subAptr[1][nlines], ABp, dqsort[nlines], Adq) == 0
             )
          {
             i = len; p = lineptr[nlines] + len;

             while (*p == ' ') { a[i++] = ' '; ++p; }
               a[i++] = '{'; a[i] = '\0'; 
             p += nextbracket(p, '{', '}');
                          killpos = i + 1;
               strcat(a, IKILLNAME); i = i + strlen(IKILLNAME);
               a[i++] = '}';

             while (*p == ' ') { a[i++] = ' '; ++p; }
               a[i++] = '{';
             p += nextbracket(p, '{', '}');
               a[i++] = '}';

             while (*p == ' ') { a[i++] = ' '; ++p; }
               a[i++] = '{';
             p += nextbracket(p, '{', '}');

             if (findFEmale(ABp) == 0 && findFEmale(subAptr[1][nlines]) == 1)
             {  printf("%%%%\n%%%%>   Warning: No %s or %s in firstname %s:\n", FEMALE, MALE, subRptr[LINECOL][hint]);
                printV(hint);
             }

               xx[0] = '0';
               i = subdoubleauthors(0, 0, a, i, i, ACp, subAptr[2][nlines], hint, nlines, findFEmale(ABp), xx);
               if (i < 0)
               { i = -i;       a[killpos] = PLURALCHAR;
                 a[i++] = '}';
                 if (xx[0] != '0') i = insertfalse(p, a, i);
                 else while (*p != '\0' && i < MAXLEN) a[i++] = *p++;
               }
               else 
               { a[i++] = '}';
                      while (*p != '\0' && i < MAXLEN) a[i++] = *p++;
               }
               

             if (i < MAXLEN)
             {  ++num;
                myfree(lineptr[nlines]);
                       lineptr[nlines] = strnmalloc(a, i+1);
             }
             else printf("%%%%\n%%%%>  Error: Can\'t replace {%s}{%s} by %s (too long)\n", subAptr[0][nlines], subAptr[1][nlines], IKILLNAME);
          }
        }
  return num;
}


void pauthorbrackwarn(int nline, const char *head)
{
  printf("%%%%\n%%%%>   Warning outfile entry %d (in arg 3): Missing brackets as %s{..}{..}\n", nline+1, head);
}


int subdoubleauthors(int found, int addtxt, char a[], int remi, int i, const char *q, const char *p, int qline, int pline, int fe, char xx[])
{
   int ql = 0, pl, len, haveto, havedone = 0, ntxt = 0, nn, j;
   char qbuf[MAXLEN] = "", pbuf[MAXLEN] = "";
   const char *pmem, *qin = q, *pin = p;
   const char *head;

   while(*q != '\0')
   {
          haveto = 0;
     if ((j=texlistnum(q, NAUTHOR, authortable)) > -1)
     {
       head =          authortable[j][1];
       haveto = 1; 
     }
     else if (*q != ' ') ntxt = 1;

     if (haveto == 1)
     {
       q += llstrlen(authortable[j][0]);
       while(*q == ' ') ++q;
       if (*q == '{' && (ql = nextbracket(q, '{', '}')) > 1)
       { 
         ql = mystrncpy(qbuf, q, ql);
         q += ql;
         while(*q == ' ') ++q;
         ql = ql + (len=nextbracket(q, '{', '}'));
         if (len > 1)
         {
           strncat(qbuf, q, len);
           q += len;
         }
       }
       break; 
     }
     else
     if (*q == '\\' && *(q+1) == '\\')
     {
       ntxt = 1;
       q++;
     }

     q++;
   }


   while(*p != '\0' && i < MAXLEN-1)
   {
          haveto = 0;
     if ((j=texlistnum(p, NAUTHOR, authortable)) > -1)
     {
       head =          authortable[j][1];
       haveto = 1;
     }
     else if (*p != ' ') ntxt = 1;

     if (haveto == 1)
     { 
       pmem = p;
         p += llstrlen(authortable[j][0]);
       while(*p == ' ') ++p;
       if (*p == '{' && (pl = nextbracket(p, '{', '}')) > 1)
       {
         pl = mystrncpy(pbuf, p, pl);
         p += pl;
         while(*p == ' ') ++p;
         pl = pl + (len=nextbracket(p, '{', '}'));
         if (len > 1)
         { strncat(pbuf, p, len);
           p += len;
           if (    authorcmp(qbuf, pbuf, dqsort[qline], dqsort[pline]) == 0
                &&    strcmp(qbuf, "{}{}")   != 0
                &&    strcmp(qbuf, "{ }{ }") != 0
                && i + strlen(head) + 4 + strlen(NKILLNAME) + 1 < MAXLEN
              )
           {
             a[i] = '\0'; 
             strcat(a, head);      i += strlen(head);
             strcat(a, " {}{");    i += 4;
             strcat(a, NKILLNAME); i += strlen(NKILLNAME);
             strcat(a, "}");       i += 1;
             havedone = 1;
           }
           else
           { p = pmem;
           }
         }
         else
         {   p = pmem;    pauthorbrackwarn(pline, head);
         }
         break;
       }
       else 
       {     p = pmem;    pauthorbrackwarn(pline, head);
       }
     }
     else
     if (*p == '\\' && *(p+1) == '\\')
     {
       ntxt = 1;
       a[i++] = *p++;
     }

     a[i++] = *p++;
   }


   if (havedone == 0)
   {
     if (found > 0 && addtxt == 0 && fe == 1 && findauthor(qin) == -1) 
     { nn = 1; i = remi;
               p = pin;
       while (*p == ' ') ++p;
                              if (ntxt == 1) xx[0] = '1';
     } else nn = 0;

       while (*p != '\0' && i < MAXLEN) a[i++] = *p++;
     if (nn == 1) i = -i;
   }
   else 
   {                 if (ntxt > 0) addtxt = ntxt;
     i = subdoubleauthors(++found, addtxt, a, remi, i, q, p, qline, pline, fe, xx);
   }

   return i;
}


/*--------------------------------------------------------------------------*/


int readlines(int maxlines, int nlines, const char *head)
{
        int len, headlen = strlen(head);
        char *p, linestr[MAXLEN];

        while ((len = ignFRONTSPACESgetline(linestr, MAXLEN)) > 0)
        {
           if (    strncmp(linestr, head, headlen) == 0
                && isLaTeXcmdletter(*(linestr+headlen))  == 0
              )
           {
              if (    nlines >= maxlines 
                   || (p = (char *) malloc(len)) == NULL
                 ) return -1;
              else
              {  linestr[len-1] = '\0';
                 strcpy(p, linestr);
                 lineptr[nlines++] = p;
              }
           }
        }
        return nlines;
}


/*--------------------------------------------------------------------------*/

      /* Kehrt mit -argc+1 zur"uck, wenn Option da ist.  */

int single_option(int argc, char *argv[], char a[], char b[])
{
  
  int l = 1, c;
  int _found = 0;

  while (l < argc)
  {
    if (strcmp(argv[l], a) == 0 || strcmp(argv[l], b) == 0)
    { _found = 1;
         /* myfree(argv[l]); */
         --argc;
         c = l;
         while (c < argc)
         { argv[c] = argv[c+1];
           ++c;
         }
    --l;
    }
    ++l;
  }
  if (_found == 1) return -argc;
  else             return  argc;
}


char *arg_option(int argc, char *argv[], char a[], char b[], char *info)
{
  
  int l = 1, c, found = 0;
  char *p = NULL;

  if (argc > 1 && (strcmp(argv[argc-1], a) == 0 || strcmp(argv[argc-1], b) == 0))
  {
    printf("%%%%\n%%%%>  Error: %s is not followed by an argument %s.\n", b, info);
    announceexit("get an arg option without arg");
    exit(1);
  }

  while (l < argc-1)
  {
    if (strcmp(argv[l], a) == 0 || strcmp(argv[l], b) == 0)
    {
         /* myfree(argv[l]); */
         p = argv[l+1];
         ++found;
         argc -= 2;
         c = l;
         while (c < argc)
         { 
           argv[c] = argv[c+2];
           ++c;
         }
    --l;
    }
    ++l;
  }

  if (found > 1)
  {
    printf("%%%%\n%%%%>  Error: %s [arg] setted %d times.\n", a, found);
    announceexit("do not accept multiple defined arg options");
    exit(1);
  }

  return p;
}


/*--------------------------------------------------------------------------*/

      /* Erkennt unbekannte Optionen.                    */

int surp_option(int argc, char *argv[])
{
  int l = 1, c;

  while (l < argc)
  {
    if (*argv[l] == '-')
    { printf("%%%%\n%%%%>  Error: Get unknown option \"%s\"!\n", argv[l]);
         /* myfree(argv[l]); */
         --argc;
         c = l;
         while (c < argc)
         { argv[c] = argv[c+1];
           ++c;
         }
    --l;
    }
    ++l;
  }
  return argc;
}


/*--------------------------------------------------------------------------*/


char *makename(const char *prefix, const char *suffix)
{
  char fname[MAXFILENAMLEN];
  int plen;

  plen = mystrncpy(fname, prefix, MAXFILENAMLEN-1);
           strncat(fname, suffix, MAXFILENAMLEN-plen-1);

  if (plen + strlen(suffix) > MAXFILENAMLEN-1)
  {
          announceexit("get a too long path/file name");
          printf("%%%%\n%%%%   \"%s%s\"\n", prefix, suffix);
          exit(1);
  }

  return strnmalloc(fname, strlen(fname)+1);
}


int dofile(int filenum, char *filenam, int nlines, const char *entry, const char *formO, int deep, int deepsort, int ventry)
{
   int alllines = nlines;

   infile = fopen(filenam, "r");

   if (infile != NULL)
   {
     printf("%%%%>  Search *\"%s\"* in file \"%s\" ...\n", entry, filenam);

     if ((alllines = readlines(MAXLINES, nlines, entry)) >= 0) 
     {
       if (alllines-nlines > 0 && MAXLINES > 0)
       { 
         printf("%%%%             ^^^^^^^^^^^  ... get %d line(s)", alllines-nlines);
         if (alllines*100/MAXLINES > 0)
         printf(" [reach %d%% of capacity]", alllines*100/MAXLINES);
         printf(".\n");
       }
        getbrackets(nlines, alllines, entry, deep, filenum, formO, ventry);
        dopgfntnumberweight(nlines, alllines);
       dodqdeepsort(nlines, alllines, deepsort);
        vorbereiten(nlines, alllines, deepsort, ventry);
     }
     else printf("%%%%\n%%%%>  Error: More than %d input entries.\n", MAXLINES);  /* exit(1) bei dofile */
   }
   else   
   {      printf("%%%%\n%%%%>  Error: No file \"%s\".\n", filenam);
          openfailed(filenam);
   }

   if (infile != NULL) fclose(infile);

   return alllines;
}


char *switchtodospath(char s[])
{
  int i = 0;

  while (s[i] != '\0')
  {
    if (s[i] == '/') s[i] = '\\';
    ++i;
  }

  return s;
}


int getfilenames(int nlines, int usedospath)
{
    int i = 0, j = 0, len;
    char *p;

    while (i < nlines)
    { 
      p = setptrtofirstbracket(lineptr[i]);
      if (*p == '{' && (len=nextbracket(p, '{', '}')) > 2)
      { filelist[j] = strnmalloc(p+1, len-1);
        p += len;
        if (usedospath == 1)
        filelist[j] = switchtodospath(filelist[j]);
        ignoreadditionaltext(p, "\\@input", filelist[j]);
        ++j;
      }
      else printf("%%%%\n%%%%>  Error after \\@input-cmd %d: Empty, or not matching brackets.\n", i+1);
      myfree(lineptr[i]);
        ++i;
    }
    if (i != j && i != 0) printf("%%%%   %d input file(s) remain(s).\n", j);
    return j;
}


int userusesdq(int deepsort, int linec)
{
   int i = 0;
   char *p;

   while (i < deepsort)
   {
             p  = subAptr[i++][linec];
     while (*p != '\0')
     { 
        if (*p == '\"') return 1;
        if (*p == '\\' && *(p+1) != '\0') ++p;
           ++p;
     }
   }

   return 0;
}


void freeall(int nlines, int deep, int deepsort)
{
   int linec = 0, i;

   if (deepsort > deep) intern_err(22);  /* deepsort > deep */

   while(linec < nlines)
   {  myfree(lineptr[linec]);
      i = 0; while (i <       deep) myfree(subAptr[i++][linec]);
      i = 0; while (i <   deepsort) myfree(subBptr[i++][linec]);
             while (i <   deep    )    if (subBptr[i++][linec] != mydefault) intern_err(23);  /* FREEALL */
      i = 0; while (i <   deepsort) myfree(subCptr[i++][linec]);
             while (i <   deep    )    if (subCptr[i++][linec] != mydefault) intern_err(24);  /* FREEALL */
      i = 0; while (i < BAALLRCOLS) myfree(subRptr[i++][linec]);
      ++linec;
   }
}


void testdrucken(int linec, int alllines, int deep, char *subptr[MAXBACOLS][MAXLINES])
{
    int i;

    while (linec < alllines)
    {
      i = 0;
      while (i < deep)
      {
        printf(" (%s)", subptr[i][linec]); 
        ++i;
      }
        printf(" [\"=%d, %d]", dqsort[linec], getarabicnum(subRptr[DQCOL][linec]));
        printf("\n");
      ++linec;
    }
}


#define WARNDOUBLEMAX  5

int fprint_out_ktit(int alllines, int deep, int ventry, const char *suffix)
{
  int linec = 0, i = 0, num = 0, numa = -1, numb = -1, numc = -1, numd = -1, kina = 0, kinb = 0, kinc = 0, kind = 0;
  int mostoktitarg = O_KTIT_UNINIT, warndouble = 0, j = 0, numalter = 0;

  if (ventry == IS_V)
  {
    while (linec < alllines)
    {
      numa = -1; numb = -1; numc = -1; numd = -1;
      i = 0;
      while (i < deep)
      {
        num = num_of_cmds_in_str(subAptr[i][linec], KTITCMD);

        if (num > 0)
        {
          j = i+1;
          while (j < deep)
          {
            if ((numalter = num_of_cmds_in_str(subAptr[j][linec], KTITCMD)) > 0) 
            { 
              num = 0;
              break;
            }
            ++j;
          }
        }

             if (i == 0) { numa = num; }
        else if (i == 1) { numb = num; }
        else if (i == 2) { numc = num; }
        else if (i == 3) { numd = num; }
        else intern_err(40);

        if (numa+numb+numc+numd > 1)
        { 
          if (warndouble < WARNDOUBLEMAX)
          {
            printf("%%%%\n%%%%>  Warning %s: *Outer* %s used %d times in:\n", subRptr[LINECOL][linec], KTIT, (numa+numb+numc+numd));
            printV(linec);
          }
          ++warndouble;
        }
        ++i;
      }

        if (numa > 0) { ++kina; }
        if (numb > 0) { ++kinb; }
        if (numc > 0) { ++kinc; }
        if (numd > 0) { ++kind; }

      ++linec;
    }

    if (warndouble > WARNDOUBLEMAX)
    printf("%%%%\n%%%%>  ... for %d futher entries with multiple %s see file.\n", warndouble-WARNDOUBLEMAX, KTIT);

         if (kina > kinb && kina > kinc && kina > kind)    { mostoktitarg = MOST_O_KTIT_ARGONE;    /* arg 1 most used */ }
    else if (kinb > kina && kinb > kinc && kinb > kind)    { mostoktitarg = MOST_O_KTIT_ARGTWO;    /* arg 2 most used */ }
    else if (kinc > kina && kinc > kinb && kinc > kind)    { mostoktitarg = MOST_O_KTIT_ARGTHREE;  /* arg 3 most used */ }
    else if (kind > kina && kind > kinb && kind > kinc)    { mostoktitarg = MOST_O_KTIT_ARGFOUR;   /* arg 4 most used */ }
    else if (kina > 0 || kinb > 0 || kinc > 0 || kind > 0) { mostoktitarg = O_KTIT_NOPREF;    /* no preference   */ }
    else { mostoktitarg = O_KTIT_NOTUSED;  /* ktit not used */ }

    printf("%%%%>  Summary *outer* v-cmds in %s: %s ", suffix, KTIT);
    if (mostoktitarg == O_KTIT_NOTUSED)
    {
      printf("never used.\n");
    }
    else
    if (mostoktitarg == O_KTIT_NOPREF)
    {
      printf("defined in different args!\n%%%%\n");
      /* kina, kinb, kinc, kind save lastused args with \ktit, 
         not all args with \ktit! */
    }
    else
    {
      if (kina == alllines || kinb == alllines || kinc == alllines || kind == alllines)
       { if (alllines > 1) printf("in all %d entries ", alllines); }
      else
       printf("NOT always ");
      printf("defined in ");
      if (alllines > 1) 
      { 
        if (kina == alllines || kinb == alllines || kinc == alllines || kind == alllines);
        else printf("mostused ");
      }
      printf("arg %d.\n", mostoktitarg);
      if (kina == alllines || kinb == alllines || kinc == alllines || kind == alllines);
      else printf("%%%%   *******\n");
    }
  }
  else
  {
    mostoktitarg = O_KTIT_UNINIT;  /* ventry != IS_V */
  }

  return mostoktitarg;
}


int check_kandk(int nlines)
{
   int i = 0;
   int outkurz = 0;
   int ktit    = 0;

   while (i < nlines)
   {
     outkurz += num_of_cmds_in_str(lineptr[i], KURZCMD);
     ktit    += num_of_cmds_in_str(lineptr[i], KTITCMD);
     ++i;
   }

   if (outkurz > 0 && ktit > 0)
   {
     printf("%%%%\n%%%%>  Warning: %d time(s) %s and %d time(s) %s found;\n", outkurz, KURZ, ktit, KTIT);
           printf("%%%%      see %s warnings in the output file.\n", KURZ);
   }

   if (outkurz > 0 && ktit > 0) return 1;
   return 0;
}


char *fill_vbuf(char *b, char *p)
{
   char *a = b;
   int arglen;

   while (*p == ' ') *b++ = *p++;

   arglen = nextbracket(p, '{', '}');
                                
   if (arglen > 1 && *p == '{')
   { 
     while (arglen-- > 0) *b++ = *p++;
   }
   else
   {
     b = a;
   }

   *b = '\0';

   return p;
}


#define MAX_EX_ERR  5

int lastname_first(int nlines)
{
   int i = 0, pos, j, k, ex_err = 0, ex_used = 0;
   char *p, bufa[MAXLEN], bufb[MAXLEN], *bega, *begp, *begc;


   while (i < nlines)
   {
     p = lineptr[i];
     begp = p;


     while ((pos=findauthor(p)) > -1)
     {
        ++ex_used;

        p += pos;
        begc = p;
        
        while (*p != '{' && *p != '}' && *p != ' ' && *p != '\0') ++p;
        bega = p;

          p = fill_vbuf (bufa, p);

          p = fill_vbuf (bufb, p);


        if (*(begc) == '\0' || *(begc+1) == '\0' || *(begc+2) == '\0')
        {
          intern_err(31);
        }
        else
        {
          if (bufb[0] != '\0' && bufa[0] != '\0')
          {
            *(begc+2) = '@';   /* \v@uthor \m@dvauthor */

            j = 0;
            k = 0;

            if (bufa[0] == ' ' && bufb[0] == '{')
            {
              *bega++ = bufa[k++];
            }

              while (bufb[j] != '\0') *bega++ = bufb[j++];          

              while (bufa[k] != '\0') *bega++ = bufa[k++];
          }
          else
          {
            if (ex_err < MAX_EX_ERR)
            {
              printf("%%%%\n%%%%>   Warning: Can\'t identify the brackets of ");
              if (bufa[0] == '\0') printf(" {arg1}");
              if (bufb[0] == '\0') printf(" {arg2}");
              printf("  after your cmd:\n");
              printf("%%%%     \"%.*s", 30, begc);
              if (strlen(begc) > 30) printf(" ...");
              printf("\"  (entry from %s).\n", subRptr[LINECOL][i]);
            }
            else
            if (ex_err == MAX_EX_ERR)
            {
              printf("%%%%\n%%%%>  -e: Do not report further error details.\n");
            }
            ++ex_err;
          }
        }
     } 

     ++i;
   }

   if (ex_used > 0)
   return ex_err;
   else
   return -1;
}


void prepareoutput(const char *entry, const char *suffix, int deep, int deepsort, int filec, char *arg_i, char *arg_o, int prep_kill, const char *formO, int ventry, int printsort)
{
    int mostoktitarg = O_KTIT_UNINIT;  /* new in 2.5 */
    int nlines = 0, i = 0, extralines = 0, commentANDignlines = 0;
    int filenum = -1, kandkused = 0, ex_err;
    char *outname = mydefault;
    
    if (arg_o == NULL) outname = makename(arg_i, suffix);
    else               outname = makename(arg_o, suffix);

    ktiterr = 0;
    thisupperascii = 0;
    mystrncpy(this_emp_utf_eight, "", UTF_EIGHT_MAXLEN);
    mystrncpy(this_err_utf_eight, "", UTF_EIGHT_MAXLEN);

    while (i < filec) 
    { 
        if (filec > 0) filenum = i+1;
        if ((nlines = dofile(filenum, filelist[i++], nlines, entry, formO, deep, deepsort, ventry)) < 0)
        { 
          announceexit("get a too big list of entries");
          exit(1);
        }
    }

    if (MAXLINES > 0 && nlines*100/MAXLINES > 90)
       printf("%%%%  *** WARNING: Have capacity for only %d further entries! ***\n%%%%\n", MAXLINES-nlines);


   /*  printf("\nC: "); testdrucken(0, nlines,       deep, subCptr); */
   /*  printf("\nB: "); testdrucken(0, nlines,       deep, subBptr); */
   /*  printf("\nA: "); testdrucken(0, nlines,       deep, subAptr); */
   /*                   testdrucken(0, nlines, BAALLRCOLS, subRptr); */


    
    if (nlines > 0) 
    {
        ++get_jobs;

        if (nlines > 1) printf("%%%%   Sort %d lines. Wait.\n", nlines);
        QQsort(0, nlines-1, deep, deepsort);


        if (prep_kill == DOKILLAUTHORS && nlines > 1)
        { printf("%%%%>  -k: Search multiply used authors ...\n");
          printf("%%%%>  -k: Authors in %d line(s) IDEMized [identical full-titles included].\n", doubleauthor(entry, nlines));
          commentANDignlines = count_rejected(nlines);
        }
        else
        if (prep_kill == DOKILLABBREVS && nlines > 0)
        { printf("%%%%   Search multiply used or empty abbreviations ...\n");
          commentANDignlines = double_or_empty_abbreviation(nlines, deepsort, deep);
        }
        else
        if (prep_kill == DOKILLREGISTS && nlines > 0)
        { printf("%%%%   Search multiply used or empty entries ...\n");
          if (*suffix == '.')
          commentANDignlines = double_or_empty_arqperregisters(nlines, ventry, suffix+1, deepsort, deep);
          else intern_err(25);  /* ERROR with \\fill... */
        }
        else
        if (prep_kill == DOKILLK && nlines > 0)
        { printf("%%%%   Search multiply used short-titles ...\n");
          commentANDignlines = double_k(nlines, deepsort);
        }
        else commentANDignlines = count_rejected(nlines);


        if (prep_kill == DOKILLAUTHORS || prep_kill == NOKILLAUTHORS)
        { 
          if (deep == VCOLS && ventry == IS_V)           
             commentANDignlines = commentANDignlines + doubleventries(nlines, deepsort);
          else intern_err(26);  /* ERROR with doubleventries */
        }

        kandkused = check_kandk(nlines);
        /* oktitused = check_ktit deleted in 2.5 */

        if (ex_n == EXCHANGENAMES)
        {
          ex_err = lastname_first(nlines);
          if (ex_err > 0)
          { 
            printf("%%%%\n%%%%>  -e: ... the exchange of {FirstName} and {FamilyName} args\n");
                  printf("%%%%       after %s or %s *failed* in %d case(s).\n%%%%\n", authortable[0][1], authortable[1][1], ex_err);
          }
          /* not used: ex_err < 0 */
        }
                        
        if ((outfile = fopen(outname, "w")) == NULL)
        {
          printf("%%%%\n%%%%>  Error: Can\'t write to file \"%s\".\n", outname);
          openfailed(outname);
        }
        else
        { 
          mostoktitarg = fprint_out_ktit(nlines, deep, ventry, suffix);
          printf("%%%%>  Write %d item(s) to file \"%s\".\n", nlines-commentANDignlines, outname);
          if ((extralines = writelines(nlines, DQCOL, LANCOL, commentANDignlines, deepsort, filec, printsort, kandkused, mostoktitarg)) > 0)
          printf("%%%%    (Have added %d line(s) containing commands for LaTeX.)\n", extralines);
          if (ktiterr > 0)
          printf("%%%%\n%%%%>   ** Wrote %d comment(s) into file concerning %s. **\n%%%%\n", ktiterr, KTIT);
        }

        if (outfile != NULL) { fclose(outfile); fflush(outfile); }
        freeall(nlines, deep, deepsort);
    }
    else
    {
              printf("%%%%    No entries. ");
        if ((outfile = fopen(outname, "r")) != NULL)
        {
          fclose(outfile); fflush(outfile);
          if ((outfile = fopen(outname, "w")) != NULL)
          {
            fclose(outfile); fflush(outfile);  
              printf("*DELETE* old");
            remove(outname);
          }
          else printf("CAN\'T delete");
        }
        else printf("Do NOT create");
             printf(" file \"%s\".\n", outname);
    }

    outhline();

    myfree(outname);
}


#define ALLWEIGHT  ENDWEIGHT-2
void setnewweight(const int pg, const char *p, const char *opt)
{
  int lauf = BEGINWEIGHT;   /* 4 ... 7+2 */
  int have_n = 0;
  int have_R = 0, typed_R = 0;
  int have_r = 0, typed_r = 0;
  int have_A = 0, typed_A = 0;
  int have_a = 0, typed_a = 0;
  int have_s = 0;
  int wrong_letter = 0;
  const char *q = p;

  while(*p != '\0' && lauf < ALLWEIGHT)
  {
         if (*p == 'n') { if (pg == 1) { pg_arabic      = lauf;                     } else { fn_arabic      = lauf;                     } ++have_n;                      }
    else if (*p == 'R') { if (pg == 1) { pg_bigroman    = lauf; get_R_pg = BA_TRUE; } else { fn_bigroman    = lauf; get_R_fn = BA_TRUE; } ++have_R; ++have_A; ++typed_R; }
    else if (*p == 'r') { if (pg == 1) { pg_smallroman  = lauf; get_r_pg = BA_TRUE; } else { fn_smallroman  = lauf; get_r_fn = BA_TRUE; } ++have_r; ++have_a; ++typed_r; }
    else if (*p == 'A') { if (pg == 1) { pg_bigletter   = lauf; get_A_pg = BA_TRUE; } else { fn_bigletter   = lauf; get_A_fn = BA_TRUE; } ++have_A; ++have_R; ++typed_A; }
    else if (*p == 'a') { if (pg == 1) { pg_smallletter = lauf; get_a_pg = BA_TRUE; } else { fn_smallletter = lauf; get_a_fn = BA_TRUE; } ++have_a; ++have_r; ++typed_a; }
    else if (*p == 's') { if (pg == 1) { pg_symbol      = lauf;                     } else { fn_symbol      = lauf;                     } ++have_s;                      }
    else { ++wrong_letter; break; }
    ++lauf; ++p;
  }


  if (    *p != '\0' 
       || lauf != ALLWEIGHT
       || have_n != 1
       || have_R != 1
       || have_r != 1
       || have_A != 1
       || have_a != 1
       || have_s != 1
     )
  {
    printf("%%%%\n%%%%>  Error: Not the %d expected letters in \"%s %.29s\".\n", ALLWEIGHT-BEGINWEIGHT, opt, q);
    if (strlen(q) <= ALLWEIGHT-BEGINWEIGHT)
    {
            printf("%%%%     Missing:");
      if ( have_n == 0)                 printf(" [n]");
      if ( have_s == 0)                 printf(" [s]");
      if (typed_R == 0 && typed_A == 0) printf(" [R *or* A]");
      if (typed_r == 0 && typed_a == 0) printf(" [r *or* a]");
    }
    else
    {
            printf("%%%%     Error: More than %d letters", ALLWEIGHT-BEGINWEIGHT);
    }
      printf(".  E.g.:  snRa, nArs, Rrns, naAs.\n");
    if (     have_n > 1 
         ||  have_s > 1 
         || typed_R > 1 
         || typed_A > 1 
         || typed_r > 1 
         || typed_a > 1
       )
    {
            printf("%%%%     Error: You have setted");
      if (typed_A > 1) printf(" [A]");
      if (typed_a > 1) printf(" [a]");
      if ( have_n > 1) printf(" [n]");
      if (typed_R > 1) printf(" [R]");
      if (typed_r > 1) printf(" [r]");
      if ( have_s > 1) printf(" [s]");
      printf(" more than once.\n");
    }
    if (    (typed_A > 0 && typed_R > 0)
         || (typed_a > 0 && typed_r > 0)
       )
    {
            printf("%%%%     Error: You have setted");
      if (typed_A > 0 && typed_R > 0) printf(" [R] *and* [A]");
      if (typed_A > 0 && typed_R > 0 && typed_a > 0 && typed_r > 0)
      printf(",  and ");
      if (typed_a > 0 && typed_r > 0) printf(" [r] *and* [a]");
      printf(".\n");
      if (    (typed_a > 0 && typed_r > 0)
           || (typed_A > 0 && typed_R > 0)
         )
      {
        printf("%%%%       **   A number like 'i' could mean 9 (alph), and 1 (roman).   In xxxx,\n");
        printf("%%%%       **   you can only choose R and a, or R and r, or A and a, or A and r.\n");
      }
    }
    if (wrong_letter > 0) printf("%%%%     Break: Unexpected character \'%c\'.\n", *p);
    announceexit("did not get 4 out of nRrAas (R or A, r or a)");
    exit(1);
  }


  if (pg == 1)
  {
         if (get_r_pg == BA_FALSE) pg_smallroman  = ++lauf;
         if (get_R_pg == BA_FALSE) pg_bigroman    = ++lauf;
         if (get_a_pg == BA_FALSE) pg_smallletter = ++lauf;
         if (get_A_pg == BA_FALSE) pg_bigletter   = ++lauf;
  }
  else
  {
         if (get_r_fn == BA_FALSE) fn_smallroman  = ++lauf;
         if (get_R_fn == BA_FALSE) fn_bigroman    = ++lauf;
         if (get_a_fn == BA_FALSE) fn_smallletter = ++lauf;
         if (get_A_fn == BA_FALSE) fn_bigletter   = ++lauf;
  }

  if ((pg == 0 || pg == 1) && lauf == ENDWEIGHT);
  else intern_err(27);  /* setnewweight */
}


void print_arg_list(char *argv[], int argc)
{
  int l = 1;
  while (l < argc) printf("%%%%      %d:  \"%s\"\n", l, argv[l++]);
}


void printhex(int n)
{
  printf(" (U+");
  if (n < 256) printf("0");
  if (n < 4096) printf("0");
  printf("%X)", n);
}


void print_utf_msg(const char err[UTF_EIGHT_MAXERR][UTF_EIGHT_MAXLEN+1], int err_c, const char *msg_a, const char *msg_b, const char *msg_c, const char *msg_d, const char *msg_e, const char *msg_f)
{
  int i, j = 0, a, b, c, d, len;

  if (err_c > 0)
  {
    printf("%%%%\n");
    fprintf(stderr, "%%%%>  -utf8 :  I did %s %s utf8 sequences.\n", msg_d, msg_b);

    while (j < err_c && j < UTF_EIGHT_MAXERR)
    {
         printf("%%%%>  %s utf8 sequence ", msg_a);
      if (err[j][1] == '\0') printf("consisting only of octet ");   /* msg changed in 2.4, 2.5 */
      i = 0; while(err[j][i] != '\0') { printf("%d", ((unsigned char)err[j][i])); if(err[j][i+1] != '\0') printf("-"); ++i; }

      a = ((unsigned char)err[j][0]);
      b = ((unsigned char)err[j][1]);
      c = ((unsigned char)err[j][2]);
      d = ((unsigned char)err[j][3]);

      len = lenUTFacht(a, b, c, d);

      /*  64 = (UTF_BLOCK_END - UTF_BLOCK_BEGIN + 1)  */

      if (len == 2) printhex((a - DOUBLETTES_BEGIN) *     64  +  b);
      if (len == 3) printhex((a - TRIPLETTES_BEGIN) *   4096  +  (b - 128) *   64  +  (c - 128));
      if (len == 4) printhex((a - QUADRUPLES_BEGIN) * 262144  +  (b - 128) * 4096  +  (c - 128) * 64  +  (d - 128));

         printf(" %s.\n", msg_c);
      ++j;
    }

    if (err_c >= UTF_EIGHT_MAXERR) printf("%%%%    (Max. %d different %s sequences are reported.)\n", UTF_EIGHT_MAXERR, msg_e);
    printf("%%%%    %s\n", msg_f);
  }
}


void no_upperascii_found_msg(const char *msg)
{
  if (get_jobs > 0)
  {
    printf("%%%%\n%%%%>  %s unnecessary yet:  No ASCIIs > 127 found!\n", msg);
  }
}


int main(int argc, char *argv[])
{
        int nlines = 0;
        int usedospath = 0;
        int filec = 0;
        int get_g = 0, get_cp = 0;
        int printsort = 0;
   char *inname = mydefault, *arg_i = NULL, *arg_o = NULL;


          printf("\n%%%%>  This is bibsort 2.5  (for help:  %s -\?)\n", getmyname(argv[0], 37));
   fprintf(stderr, "%%%%      bibsort 2.5 is part of BibArts 2.5    (C) Timo Baumann  2022b.\n");


   argc = single_option(argc, argv, "-B", "-b");
   if (argc < 0)
   {
     bago = 1;
     argc = -argc;
   }

   argc = single_option(argc, argv, "-N1", "-n1");
   if (argc < 0)
   {
     FntIndexOverPages = 0;
     argc = -argc;
   }

   argc = single_option(argc, argv, "-c", "-C");
   if (argc < 0)
   { argc = -argc;
     notext = 1;
   }

   soptarg = arg_option(argc, argv, "-S2", "-s2", "[sorting key: 4 out of nRrAas]");
   if (soptarg != NULL)
   {
     argc -= 2;
     setnewweight(1, soptarg, "-s2");
   }

   argc = single_option(argc, argv, "-S1", "-s1");
   if (argc < 0)
   { argc = -argc;
     if (    get_R_pg == BA_FALSE
          && get_A_pg == BA_FALSE
          && get_a_pg == BA_FALSE
          && get_r_pg == BA_FALSE
     ) 
     { 
       EinlROEMseiten = 1;
     }
     else
     {
       announceexit("get opt -s1 *and* -s2");
       exit(1);
     }
   }

   foptarg = arg_option(argc, argv, "-F2", "-f2", "[sorting key: 4 out of nRrAas]");
   if (foptarg != NULL)
   {
     argc -= 2;
     setnewweight(0, foptarg, "-f2");
   }

   argc = single_option(argc, argv, "-F1", "-f1");
   if (argc < 0)
   { argc = -argc;
     if (    get_R_fn == BA_FALSE
          && get_A_fn == BA_FALSE
          && get_a_fn == BA_FALSE   
          && get_r_fn == BA_FALSE
     ) 
     { 
       ALPHmpfootnotes = 1;
     }
     else
     {
       announceexit("get opt -f1 *and* -f2");
       exit(1);
     }
   }

   argc = single_option(argc, argv, "-i=j", "-I=J");
   if (argc < 0)
   { argc = -argc;
     igleichj = 1;
   }

   argc = single_option(argc, argv, "-D", "-d");
   if (argc < 0)
   { argc = -argc;
     usedospath = 1;
   }

   argc = single_option(argc, argv, "-K", "-k");
   if (argc < 0)
   { argc = -argc;
     kill = DOKILLAUTHORS;
   }

   argc = single_option(argc, argv, "-E", "-e");
   if (argc < 0)
   { argc = -argc;
     ex_n = EXCHANGENAMES;
   }

   argc = single_option(argc, argv, "-L", "-l");
   if (argc < 0)
   { argc = -argc;
     ignorespaces = 1;
   }

   argc = single_option(argc, argv, "-H", "-h");
   if (argc < 0)
   { argc = -argc;
     hy_as_space = 1;
   }

   argc = single_option(argc, argv, "-P", "-p");
   if (argc < 0)
   { argc = -argc;
     registpoints = 1;
   }


   argc = single_option(argc, argv, "-T1", "-t1");
   if (argc < 0)
   { argc = -argc;
     ++get_cp;
     encode = IS_TONE_ENC;
   }

   /* utf8 new in 2.3 (begin with utf8enc.dfu  2021/01/27 v1.2l),
       changed in 2.4 (= changes from v1.2l to 2021/06/21 v1.2n),
       changed in 2.5 (= changes from v1.2n to 2022/06/07 v1.3c)
    */
   argc = single_option(argc, argv, "-utf8", "-UTF8");
   if (argc < 0)
   { argc = -argc;
     ++get_cp;  
     encode = IS_UTFE_ENC;
   }

   if (get_cp > 1)
   {
     announceexit("get opt -t1 *and* -utf8");
     exit(1);
   }


   argc = single_option(argc, argv, "-X", "-x");
   if (argc < 0)
   { argc = -argc;
     nogsty = 1;
   }

   argc = single_option(argc, argv, "-M", "-m");
   if (argc < 0)
   { argc = -argc;
     printsort = 1;
   }


   argc = single_option(argc, argv, "-G1", "-g1");
   if (argc < 0)
   { argc = -argc;
     ++get_g;
     german = 1;
   }

   argc = single_option(argc, argv, "-G2", "-g2");
   if (argc < 0)
   { argc = -argc;
     if (get_g == 0) german = 2;
     ++get_g;
   }

   if (get_g > 1)
   { 
     announceexit("get opt -g1 *and* -g2");
     exit(1);
   }


   argc = single_option(argc, argv, "-\?", "-h1");    /* 2.2: -h => hyphen */
   if (argc < 0)
   {
     argc = -argc;
     maininfo(argv[0]);
     announceexit("get your question for help");
     exit(1);
   }

   argc = single_option(argc, argv, "-\?\?", "-h2");  /* 2.2: -hh => -h2 */
   if (argc < 0)
   {
     argc = -argc;
     moreinfo(argv[0]);
     announceexit("get your question for more help");
     exit(1);
   }

   argc = single_option(argc, argv, "-\?\?\?", "-h3");  /* new in 2.3 */
   if (argc < 0)
   {
     argc = -argc;
     moremoreinfo(argv[0]);
     announceexit("get your question for more more help");
     exit(1);
   }

   argc = single_option(argc, argv, "-R", "-r");
   if (argc < 0)
   {
     argc = -argc;
     licenseinfo();
     announceexit("get your question for license informations");
     exit(1);
   }

   if (argc < 2)
   { if (argc == 1) maininfo(argv[0]);
     announceexit("get no <FileNamePrefix> of an auxiliary file");
     exit(1);
   }

   arg_i = arg_option(argc, argv, "-I", "-i", "<InputFilenamePrefix>");
   if (arg_i != NULL)
   {
     argc -= 2;
   }
   
   arg_o = arg_option(argc, argv, "-O", "-o", "<OutputFilenamesPrefix>");
   if (arg_o != NULL)
   {
     argc -= 2;
   }
   
   if (argc != surp_option(argc, argv)) 
   { 
     announceexit("get an unknown -[option]");
     exit(1);
   }


   if (    (argc == 2 && arg_i == NULL)
        || (argc == 1 && arg_i != NULL)
      )
   {
                           if (argc == 2)  { arg_i = argv[1]; --argc; }
                           inname = makename(arg_i, AUX_SUFFIX);
            infile = fopen(inname, "r");
   }
   else
   {        printf("%%%%\n%%%%>  Error: Too many arguments:\n");
            print_arg_list(argv, argc);
            printf("%%%%      Since vers 2.1, you have to use  -o <OutputFileNamesPrefix>.\n");
            announceexit("get more than one file name in command line");
            exit(1);
   }

   if (infile == NULL)
   {
            printf("%%%%\n%%%%>  Error: No file \"%s\".\n", inname);
            openfailed(inname);
            announceexit("did not get the name of an existing file");
            exit(0);  /* damit TeXnicCenter danach trotzdem LaTeX startet */
   }

   if ((nlines = readlines(MAXFILES, 0, "\\@input")) == 0) 
   {
           printf("%%%%>  No \\@input cmds in \"%s\":\n", inname);
           filelist[0] = inname;
           filec = 1;
   }
   else
   if (nlines < 0)
   {
           printf("%%%%>  Error: To many \\@input cmds in \"%s\".\n", inname);
           announceexit("can not handle such a big list of files");
           exit(1);
   }
   else
   {
           printf("%%%%>  Try to work list of %d \\@input files in \"%s\":\n", nlines, inname);
           nlines = getfilenames(nlines, usedospath);
           if (nlines < 1) 
           { 
             announceexit("get a too big input file");
             exit(1);
           }
           filec = nlines;
   }

   if (infile != NULL) fclose(infile);

   printfilelist( stdout, filec, 0, "%%   ", "");



     /* changed in 2.2 => inner \\printonlyvli \\printonlyvqu */
     /* l = loeschen */
     /* e = entkernen */
     /* X = weder loeschen noch entkernen */
     /*  "----" KHVONUM Positionen: \\onlykurz \\onlyhere \\onlyvoll \\onlyout  [new in 2.5:]  \\ktit <= X */

   prepareoutput("%\\literentry", VLI_SUFFIX, VCOLS,     VCOLS, filec, arg_i, arg_o,          kill, "lleeX", IS_V, printsort);
   prepareoutput("%\\quellentry", VQU_SUFFIX, VCOLS,     VCOLS, filec, arg_i, arg_o,          kill, "lleeX", IS_V, printsort);
   prepareoutput("%\\vkcitentry", VKC_SUFFIX, KCOLS,     KCOLS, filec, arg_i, arg_o, DOKILLK,       "elleX", NO_V, printsort);

   prepareoutput("%\\abkrzentry", ABK_SUFFIX, ACOLS, ACOLSSORT, filec, arg_i, arg_o, DOKILLABBREVS, "lleeX", NO_V, printsort);
   prepareoutput("%\\perioentry", PER_SUFFIX, PCOLS, PCOLSSORT, filec, arg_i, arg_o, DOKILLREGISTS, "lleeX", NO_V, printsort);
   prepareoutput("%\\archqentry", ARQ_SUFFIX, PCOLS, PCOLSSORT, filec, arg_i, arg_o, DOKILLREGISTS, "lleeX", IS_Q, printsort);

   prepareoutput("%\\geogrentry", GRR_SUFFIX, PCOLS, PCOLSSORT, filec, arg_i, arg_o, DOKILLREGISTS, "lleeX", NO_V, printsort);
   prepareoutput("%\\persrentry", PRR_SUFFIX, PCOLS, PCOLSSORT, filec, arg_i, arg_o, DOKILLREGISTS, "lleeX", NO_V, printsort);
   prepareoutput("%\\subjrentry", SRR_SUFFIX, PCOLS, PCOLSSORT, filec, arg_i, arg_o, DOKILLREGISTS, "lleeX", NO_V, printsort);



   if (encode == IS_BASE_ENC && lastupperascii < 0)
   {
        printf("%%%%\n%%%%>  Default encoding:  Gave ASCIIs > 127 like %d NO sorting weight.\n", ((unsigned char)lastupperascii));
     fprintf(stderr, "%%%%   Did not evaluate your ASCIIs > 127.  Start me using -utf8 or -t1 \?\n");  /* msg changed in 2.5 */
              printf("%%%%   *******\n");
   }
   
   
   if (encode == IS_TONE_ENC && lastupperascii < 0)
   {
        printf("%%%%\n%%%%>  -t1 :  Sort ASCIIs > 127 (%d as  %s  etc.).\n", ((unsigned char)lastupperascii), getTONEenc(lastupperascii));
     fprintf(stderr, "%%%%>  -t1 :  \\UseRawInputEncoding expected.\n");
   }
     

   if (encode == IS_TONE_ENC && lastupperascii == 0)
   {
     no_upperascii_found_msg("-t1");
   }

   
   if (encode == IS_UTFE_ENC)
   {
     print_utf_msg(err_utf_eight, utf_err_c, "Invalid", "invalid", "found", "not interpret", "invalid", "Are you sure, that your text editor uses utf8 \?");  /* msg changed in 2.5 */
     print_utf_msg(emp_utf_eight, utf_emp_c, "Gave", "all your", "no sorting weight", "not know the meaning of", "unknown", "Better replace this sequence(s) by LaTeX-cmds in your text.");
     if (lastupperascii == 0) no_upperascii_found_msg("-utf8");
   }


   if (get_jobs == 0) printf("%%%%\n%%%%>  Nothing to do!\n");


   return 0;
}


/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
