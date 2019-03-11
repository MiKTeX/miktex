/*  BibArts 2.2 assists you to write LaTeX texts in arts and humanities. */
/*  Copyright (C) 2019  Timo Baumann   bibarts[at]gmx.de   (2019/03/03)  */

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
    bibarts.sty   LaTeX style file
    ba-short.pdf  Short documentation (English)
    ba-short.tex  Source of ba-short.pdf
    bibarts.pdf   Full documentation (German)
    bibarts.tex   Source of bibarts.pdf
    bibsort.exe   Binary to create the lists
    bibsort.c     This file; source of bibsort.exe
    COPYING       The license of the package
 */



/*
  COMPATIBILITY
  I have tested bibsort.c with several C compilers.  I had problems with
  the devcpp.exe EDITOR (Dev-Cpp_5.4.0_TDM-GCC_x64_4.7.1).  Sucessful was:
  "C:\Program Files (x86)\Dev-Cpp\MinGW64\bin\gcc.exe" -O2 -c bibsort.c -o bibsort.o
  "C:\Program Files (x86)\Dev-Cpp\MinGW64\bin\gcc.exe" bibsort.o -o bibsort.exe

  Do NOT change this file when you do not have a qualified text
  editor - it contains control chars: '', '', '', '',  |  Are the
  '', '', '', '', '', '', '', '', '', '', '',   |  |'s in
  '', '', '', '', '', '', '', '', '', and ''.    |  a line?

  The C-compiled binary of this file is reading LaTeX .aux files.
  It can NOT read the .bar files, which BibArts did use before 2.0.
  BibArts 2.2 ONLY has an EMULATION for 1.3 texts:  Keep copies of 1.x?
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

#define OPENB   "\'{\'"
#define NUMLEN  16

int ignorespaces = 0;
int hy_as_space = 0;
        int kill = NOKILLAUTHORS;
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
#define ANNOUNCEKTIT "\\announcektit"
#define KURZ         "\\kurz"
#define KURZCMD      "\\outkurz\t"
#define KTITINTRO    "\t"
#define KTITINTROCH  '\t'
#define KTITERRORCH  '\?'    /* Insert ?? for bad inner \ktit */
#define KTITSHA      "\t<"
#define KTITMISS     "\t|"
#define KTITEMPTY    "\t>"
#define KTITBEG      "{"
#define KTITEND      "}"
#define KTITOP       '{'
#define KTITCL       '}'
int otherktiterr   = -1;

#define ACCEPTLINE   0
#define REJECTLINE   1
#define DOUBLELINE   2   /*  \                  */
#define DIFFERLINE   3   /*    Erst ***nach***  */
#define UNDEFILINE   4   /*    QQsort vergeben  */
#define ABBREVLINE   5   /*  /                  */
#define REDEF1LINE   6
#define REDEF2LINE   7
int leval[MAXLINES];

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

int upperascii = 0;
int german = 0;
int nogsty = 0;
int encode = 0;
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

void print_two(int line, int left, int right);
int isLaTeXcmdletter(char a);
int entkerne_IeC(char buf[], const char *s);

char *mydefault = "";
char *nofnt = "(nofnt)";   /* nur "" wuerde Fehler machen */
char *unused = "(unused)";
void myfree(char *p) { if (p != mydefault && p != nofnt && p != unused && p != NULL) free(p); }


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


char *getmyname(char *p, int max)
{
  char *m = p;
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


void moreinfo(void)
{
  printf("%%%%    -t1  You use [T1]{fontenc} and NO [...]{inputenc}: I sort ASCIIs > 127.\n");
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


void maininfo(char *myname)
{
  printf("%%%%   I read a LaTeX %s file (follow \\@input), and I write my output in files\n", AUX_SUFFIX);
  printf("%%%%     %s  Literature     %s  Published sources     %s  Geographic index\n", VLI_SUFFIX, VQU_SUFFIX, GRR_SUFFIX);
  printf("%%%%     %s  Short titles   %s  Unpublished sources   %s  Person index\n",     VKC_SUFFIX, ARQ_SUFFIX, PRR_SUFFIX);
  printf("%%%%     %s  Periodicals    %s  Abbreviations         %s  Subject index\n",    PER_SUFFIX, ABK_SUFFIX, SRR_SUFFIX);
  printf("%%%%\n");
  printf("%%%%  %s <LaTeXFile> [-o <OutFile>] [-g1 [OR] -g2] [<OtherOptions>]\n", getmyname(myname, 13));
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
  printf("%%%%     -r  Typeout license (read).    -\?\?  Further options.\n");
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


int unshielded_cmds_in_str(const char *p, const char *s)
{
  int r = 0;

  while (*p != '\0')
  {
    if (*p == '{') p += nextbracket(p, '{', '}') - 1;
    else
    if (*p == '\\')
    {
      if (*(p+1) == '\\') ++p;
      else
      if (*(p+1) ==  '{') ++p;
      else
      if (isexactlcmd(p, s) == 0) ++r;
    }
    ++p;
  }

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
          if ((cmd == 0 || cmd == 6) && cmdtyp  < 2 && strncmp(p, "\\delimiter", (len=strlen("\\delimiter"))) == 0)
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


      /* Neugewichtung der Ascii-Zeichen 33 und 48 bis 126.  */

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

  void swap (int, int);
  if (left >= right) return;
  swap(left, (left + right)/2);
  last = left;

  for (i = left + 1; i <= right; i++)
  {  
     j = 0; l = 0;
     while (l < deepsort)
     { 
       if ((j=vergleich(subCptr[l][i], subCptr[l][left])) != 0) break;
       if ((j=Vergleich(subBptr[l][i], subBptr[l][left])) != 0) break;
       if ((j=vergleich(subBptr[l][i], subBptr[l][left])) != 0) break;
       ++l;
     }

     l = 0;
     if (j == 0)
       while (l < deepsort && (j=vergleich(subAptr[l][i], subAptr[l][left])) == 0) ++l;


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

     l = deepsort;
     if (j == 0)
       while (l < deep && (j=vergleich(subAptr[l][i], subAptr[l][left])) == 0) ++l;


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

    while (len > 30) { --len; ++p; ++sw; }

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


void printktiterr(const char *s)
{
  printf("%%%%    ");
  maxprint(72, s);
  printf("\n");
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


void get_ktit(char s[], const char *p, const char *com, const char op, const char cl, const int line)
{
   int l, reallen, found = 0, r = 0, ll, llen, b = 0, shielded = 0;
   char buf[MAXLEN], fub[MAXLEN];

   while (*p != '\0')
   {
     if (*p == '{' && b > 0)
     {
       l = nextbracket(p, '{', '}');
       ll = 0;
       while (l-- > 0)
       { 
         fub[ll++] = *p;
         buf[b++] = *p++; 
       }
         fub[ll] = '\0';
         shielded += num_of_cmds_in_str(fub, com);
     }
     else
     if (*p == '\\')
     {
       if (*(p+1) == '\\' || *(p+1) == '{')
       {
         buf[b++] = *p++; buf[b++] = *p++;
       }
       else
       if ((ll=innerentry(p, innerVcmd, AVNUM)) > -1)
       {
         /* inneres v-cmd ueberspringen: dort darf weiteres \\ktit stehen */
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
              s[r++] = op;
              s[r++] = *p; buf[b++] = *p++;
              s[r++] = cl;
            }
            else
            if (l == 2 && *p == '\\' && *(p+1) != '\0')
            {
              if (isLaTeXcmdletter(*(p+1)) == 0);
              else
              while (isLaTeXcmdletter(*(p+l)) != 0) ++l;
              s[r++] = op;
              while (l-- > 0) { s[r++] = *p; buf[b++] = *p++; }
              s[r++] = cl;
            }
            else
            {
              while (l-- > 0) 
              { 
                s[r++] = *p; buf[b++] = *p++;
              }
            }
         }
       }
       else buf[b++] = *p++;
     }
     else   buf[b++] = *p++;
   }

   s[r] = '\0';
   buf[b] = '\0';


   if (r == 0 || (s[0] == op && s[1] == cl) || (s[0] == op && s[1] == ' ' && s[2] == cl))
   {
     if (   strlen(KTITBEG)
          + strlen(KTITSHA)
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
         strcpy(s, KTITBEG);
       printf("%%%%\n%%%%>  ");

       if (shielded > 0) 
       { 
         printf("Error: %d shadowed { %s } cmd(s), and ...\n%%%%   ", shielded, KTIT);
         strcat(s, KTITSHA);
       }

       if (found == 0)
       {
         printf("Error: Missing");
         strcat(s, KTITMISS);
       }
       else
       { 
         printf("Warning: Empty");
         strcat(s, KTITEMPTY);
       }

       printf(" %s in last arg of *inner v-cmd* (%s-entry %d):\n", KTIT, AUX_SUFFIX, line);
       printktiterr(buf);
         strcat(s, KTITEND);
     }
   }

   if (found > 1) 
   {
     printf("%%%%\n%%%%>  Warning: %s used %d times in *inner v-cmd* (%s-entry %d):\n", KTIT, found, AUX_SUFFIX, line);
     printktiterr(buf);
   }

   if   (r > 2 && s[r-1] == cl && s[r-2] == ' ')
   {
     l = 3;
     while (r-l > 1 && s[r-l] == ' ') ++l;
     while (r-l > 1 && isLaTeXcmdletter(s[r-l]) != 0) ++l;
     if (s[r-l] != '\\') print_not_tightly_err(s, cl);
   }
}


int get_add_ktit(const char *p, const int maxktit, const int argno, const int line, const int screen)
{
   int reallen, found = 0, ll, llen, b = 0;
   char buf[MAXLEN];

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
         /* inneres v-cmd ueberspringen: dort darf weiteres \\ktit stehen */
         llen = LaTeXcmp_endlen(p, innerVcmd[ll][0]);
         p += llen;
         llen =     endofbracks(p, innerVcmd[ll][1]);
         p += llen;
         buf[b++] = '<'; buf[b++] = '#'; buf[b++] = '>';
       }
       else
       if ((reallen = LaTeXcmp_endlen(p, KTITCMD)) > 0)
       {
         while (reallen-- > 0) buf[b++] = *p++;
         ++found;
       }
       else buf[b++] = *p++;
     }
     else   buf[b++] = *p++;
   }

   buf[b] = '\0';

   if (screen == 1 && found > maxktit)
   { 
     printf("%%%%\n%%%%>  Warning: ");
     if (maxktit == 1)
          printf("%d %s too much",  found-1, KTIT);
     else printf("%d misplaced %s", found,   KTIT);
     printf(" in %s-file (entry %d, arg %d):\n", AUX_SUFFIX, line, argno);
     printktiterr(buf);
   }

   return found;
}


void do_innerVstarArg(char ret[], const char *p)
{
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


int dobracks(char buf[], const char *p, const char fmt[], const int line)
{
  int l = 0, len = 0, pos;
  char ibuf[MAXLEN], obuf[MAXLEN];

  buf[0] = '\0';

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
      if (fmt[l] == '!')
      {
                     mystrncpy(ibuf, p, pos);
                get_ktit(obuf, ibuf, KTITCMD, KTITOP, KTITCL, line);
             strcat(buf, obuf);
      }
      else
      if (fmt[l] == '*' || fmt[l] == '+') /*  +: \\vautkor => \\kauthor  */
      {
                     mystrncpy(ibuf, p, pos);
        do_innerVstarArg(obuf, ibuf);
            if (fmt[l] == '*')     
             strcat(buf,  "*");           /*  keep fmt[l]  */
             strcat(buf, obuf);
      }
      else
      if (fmt[l] == DELNORMARG || fmt[l] == DELSTARARG)
             ; 
      else
            strncat(buf,             p, pos);
      
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
                            NULL
                          };
#define KHVONUM 4


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


  const char *innerK_PER_ARQ_cmd[][2] = {
  {  "\\printonlykli\t",    ":[]:{}*{}:{}"     },
  {  "\\printonlykqu\t",    ":[]:{}*{}:{}"     },
  { "\\xprintonlykli\t",    ":[]:{}*{}:{}"     },
  { "\\xprintonlykqu\t",    ":[]:{}*{}:{}"     },
  {  "\\printonlyper\t",    ":{}"              },
  {  "\\printonlyarq\t",    ":[]:{}:{}"        },
  {  "\\printonlydefabk\t", ":{}:{}"           },
  {  "\\printonlyabkdef\t", ":{}:{}"           },
  {  "\\printonlyabk\t",    ":{}"              },
  { NULL, NULL }};
#define INNERTITLES 6
#define XKPANUM 9


int do_inneronly(char ret[], const char in[], const char *vf, const char *kpf)
{
  char buf[MAXLEN];
  int len;

  len = do_innercmd(buf,  in, innerVcmd,          XVNUM,    vf);  /* AVNUM unnecessary */
  len = do_innercmd(ret, buf, innerK_PER_ARQ_cmd, XKPANUM, kpf);

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
                               alen = mystrncpy(cbuf, p, len);
         }
         else
         {
                                      mystrncpy(cbuf, p, len);
              alen =         do_inneronly(ibuf, cbuf, "eell", "leel");
              alen = do_only(cbuf, formO, ibuf);

           if (ventry == IS_V && i == VCOLS-1)
           {
               alen = do_innerVcmd(ibuf, cbuf, innerVcmd, AVNUM, linec+1);
                      strcpy(cbuf, ibuf);
                get_add_ktit(cbuf, 1, i+cols, linec+1, 1);      /* Rest v-cmd: Ein \\ktit erlaubt */
           }
           else
           { 
             if(get_add_ktit(cbuf, 0, i+cols, linec+1, 1) > 0)  /* sonst (auch andere Args v-cmd) */
             {                                                  /* get_add_ktit schreibt Fehler!! */
               if (otherktiterr < i+1) otherktiterr = i+1;
             }
           }
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

           /* ohne Aequivalent in MathMode sind \\b \\c \\k \\d \\H \\t */
const char *weighttable[][2] = {
  {  "\\.",                 ""       },  /* CTL+A */  /* MARK_POINT */
  {  "\\dot\t",             ""       },
  {  "\\\'",                ""       },  /* CTL+B */
  {  "\\acute\t",           ""       },
  {  "\\`",                 ""       },  /* CTL+C */
  {  "\\grave\t",           ""       },
  {  "\\^",                 ""       },  /* CTL+D */
  {  "\\hat\t",             ""       },
  {  "\\ddot\t",            ""       },  /* CTL+E umlaut */
  {  "\\ij\t",              "i"      },  /* Niederlaendische */
  {  "\\IJ\t",              "I"      },  /* Sondersortierung */
  {  "\\=",                 ""       },  /* CTL+F */
  {  "\\bar\t",             ""       },
  {  "\\vec\t",             ""       },  /* Ctl+G */
  {  "\\~",                 ""       },  /* CTL+K */  /* MARK_TILDE */
  {  "\\tilde\t",           ""       },
  {  "\\aa\t",              "a"      },  /* CTL+L */  /* \\r a \\r A */
  {  "\\AA\t",              "A"      },
  {  "\\r\t",               ""       },               /*  \\AA \\aa */
  {  "\\mathring\t",        ""       },
  {  "\\b\t",               ""       },  /* CTL+N */
  {  "\\c\t",               ""       },  /* CTL+O */
  {  "\\k\t",               ""       },  /* CTL+P */  /* rechte Tilde */
  {  "\\d\t",               ""       },  /* CTL+Q */
  {  "\\H\t",               ""       },  /* CTL+R */
  {  "\\t\t",               ""       },  /* CTL+S */
  {  "\\u\t",               ""       },  /* CTL+T */
  {  "\\breve\t",           ""       },
  {  "\\v\t",               ""       },  /* CTL+U */
  {  "\\check\t",           ""       },
  {  "\\st\t",              ""       },  /* CTL+V */  /* NEW in 2.2 */
  {  "\\textst\t",          ""       },               /* NEW in 2.2 */
  {  "\\underline\t",       ""       },  /* CTL+W */  /* CHANGED in 2.2 */
  {  "\\underbar\t",        ""       },
  {  "\\ul\t",              ""       },
  {  "\\textul\t",          ""       },               /* NEW in 2.2 */
  {  "\\,",                 " "       },
  {  "\\space\t",           " "       },
  {  "\\pteskip\t",         " "       },  /* 1\te 000     NEW in 2.2 */
  {  "\\te\t",             ". "       },  /*  != 1000     NEW in 2.2 */
  {  "\\i\t",               "i"       },
  {  "\\imath\t",           "i"       },
  {  "\\j\t",               "j"       },
  {  "\\jmath\t",           "j"       },
  {  "\\ae\t",              "ae"     },  /* \     */  /* zweiteilig */
  {  "\\AE\t",              "Ae"     },  /*  |    */  /* <=> Oe    */
  {  "\\BibArts\t",         "BibArts"},  /*  |    */
  {  "\\pBibArts\t",        "BibArts"},  /*  |    */
  {  "\\textcent\t",        "c"      },  /*  |    */
  {  "\\pcopyright\t",      "c"      },  /*  |    */
  {  "\\copyright\t",       "c"      },  /*  |    */
  {  "\\textcopyright\t",   "c"      },  /*  |    */
  {  "\\textcelsius\t",     "C"      },  /*  |    */
  {  "\\dh\t",              "d"      },  /*  |    */
  {  "\\dj\t",              "d"      },  /*  |    */
  {  "\\DH\t",              "D"      },  /*  |    */
  {  "\\DJ\t",              "D"      },  /*  |    */  /* =\\DH      */
  {  "\\texteuro\t",        "E"      },  /*  |    */
  {  "\\textflorin\t",      "f"      },  /*  |    */
  {  "\\l\t",               "l"      },  /*  |    */
  {  "\\ell\t",             "l"      },  /*  |    */
  {  "\\Im\t",              "I"      },  /*  |    */
  {  "\\L\t",               "L"      },  /*  |    */
  {  "\\LaTeX\t",           "LaTeX"  },  /*  |    */
  {  "\\ng\t",              "n"      },  /*  |    */
  {  "\\NG\t",              "N"      },  /*CTL_X/Y*/
  {  "\\o\t",               "o"      },  /*  |    */
  {  "\\O\t",               "O"      },  /*  |    */
  {  "\\oe\t",              "oe"     },  /*  |    */
  {  "\\OE\t",              "Oe"     },  /*  |    */
  {  "\\textsterling\t",    "P"      },  /*  |    */
  {  "\\ppounds\t",         "P"      },  /*  |    */
  {  "\\pounds\t",          "P"      },  /*  |    */
  {  "\\Re\t",              "R"      },  /*  |    */
  {  "\\textregistered\t",  "R"      },  /*  |    */
  {  "\\th\t",              "th"     },  /*  |    */
  {  "\\TH\t",              "Th"     },  /*  |    */
  {  "\\TeX\t",             "TeX"    },  /*  |    */
  {  "\\textwon\t",         "W"      },  /*  |    */
  {  "\\textyen\t",         "Y"      },  /* /     */
  {  "\\alpha\t",           "a"      },   /* 28 */
  {  "\\Alpha\t",           "A"      },  /* bibarts.sty; new in 2.1 */
  {  "\\beta\t",            "b"      },
  {  "\\Beta\t",            "B"      },  /* bibarts.sty; new in 2.1 */
  {  "\\chi\t",             "ch"     },
  {  "\\Chi\t",             "Ch"     },  /* bibarts.sty; new in 2.1 */
  {  "\\delta\t",           "d"      },
  {  "\\Delta\t",           "D"      },
  {  "\\epsilon\t",         "e"      },
  {  "\\Epsilon\t",         "E"      },  /* bibarts.sty; new in 2.1 */
  {  "\\varepsilon\t",      "e"      },   /* 29 */
  {  "\\eta\t",             "e"      },   /* 30 */
  {  "\\Eta\t",             "E"      },  /* bibarts.sty; new in 2.1 */
  {  "\\gamma\t",           "g"      },
  {  "\\Gamma\t",           "G"      },
  {  "\\iota\t",            "i"      },
  {  "\\Iota\t",            "I"      },  /* bibarts.sty; new in 2.1 */
  {  "\\kappa\t",           "k"      },
  {  "\\Kappa\t",           "K"      },  /* bibarts.sty; new in 2.1 */
  {  "\\varkappa\t",        "k"      },
  {  "\\lambda\t",          "l"      },
  {  "\\Lambda\t",          "L"      },
  {  "\\mu\t",              "m"      },
  {  "\\Mu\t",              "M"      },  /* bibarts.sty; new in 2.1 */
  {  "\\nu\t",              "n"      },
  {  "\\Nu\t",              "N"      },  /* bibarts.sty; new in 2.1 */
  {  "\\omicron\t",         "o"      },  /* bibarts.sty */
  {  "\\Omicron\t",         "O"      },  /* bibarts.sty; new in 2.1 */
  {  "\\omega\t",           "o"      },
  {  "\\Omega\t",           "O"      },
  {  "\\pi\t",              "p"      },
  {  "\\Pi\t",              "P"      },
  {  "\\varpi\t",           "p"      },
  {  "\\phi\t",             "ph"     },
  {  "\\Phi\t",             "Ph"     },
  {  "\\varphi\t",          "ph"     },
  {  "\\psi\t",             "ps"     },
  {  "\\Psi\t",             "Ps"     },
  {  "\\rho\t",             "r"      },
  {  "\\Rho\t",             "R"      },  /* bibarts.sty; new in 2.1 */
  {  "\\varrho\t",          "r"      },
  {  "\\sigma\t",           "s"      },
  {  "\\Sigma\t",           "S"      },
  {  "\\varsigma\t",        "s"      },
  {  "\\tau\t",             "t"      },
  {  "\\Tau\t",             "T"      },  /* bibarts.sty; new in 2.1 */
  {  "\\theta\t",           "th"     },
  {  "\\Theta\t",           "Th"     },
  {  "\\vartheta\t",        "th"     },
  {  "\\xi\t",              "x"      },
  {  "\\Xi\t",              "X"      },
  {  "\\upsilon\t",         "y"      },
  {  "\\Upsilon\t",         "Y"      },
  {  "\\zeta\t",            "z"      },
  {  "\\Zeta\t",            "Z"      },  /* bibarts.sty; new in 2.1 */
  { NULL, NULL }};
#define NLATEX  135


const char *udtable[][2] = { 
  {  "\\\"a",               "a"      },  /* \     \"a */
  {  "\\\"o",               "o"      },  /*  |    \"o */
  {  "\\\"u",               "u"      },  /* CTL+E \"u */
  {  "\\\"A",               "A"      },  /*  |    \"A */
  {  "\\\"O",               "O"      },  /*  |    \"O */
  {  "\\\"U",               "U"      },  /*  |    \"U */
  {  "\\ss\t",              "s"      },  /*       \ss */
  {  "\\SS\t",              "SS"      },  /*       \SS */
  {  "\\sz\t",              "s"      },  /*       \sz */   /* new in 2.1 */
  {  "\\SZ\t",              "S"      },  /*       \SZ */   /* new in 2.1 */
  {  "\\flqq\t",             ""       },  /*     \flqq */   /* new in 2.2 */
  {  "\\frqq\t",             ""       },  /*     \frqq */   /* new in 2.2 */
  {  "\\glqq\t",             ""       },  /*     \glqq */   /* new in 2.2 */
  {  "\\grqq\t",             ""       },  /*     \grqq */   /* new in 2.2 */
  {  "\\/",                  ""       },  /*       \/  */   /* new in 2.2 */
  {  "\\-",                  ""       },  /*       \-  */   /* new in 2.2 */
  {  "\\oldhyc\t",          "c"       },  /*     [c-]  */   /* \          */
  {  "\\OLDHYC\t",          "C"       },  /*     [C-]  */   /*  |         */
  {  "\\oldhyl\t",          "l"       },  /*     [l-]  */   /*  |         */
  {  "\\OLDHYL\t",          "L"       },  /*     [L-]  */   /*  |         */
  {  "\\oldhym\t",          "m"       },  /*     [m-]  */   /*  |         */
  {  "\\OLDHYM\t",          "M"       },  /*     [M-]  */   /*  |         */
  {  "\\oldhyn\t",          "n"       },  /*     [n-]  */   /*  | new     */
  {  "\\OLDHYN\t",          "N"       },  /*     [N-]  */   /*  |  in     */
  {  "\\oldhyp\t",          "p"       },  /*     [p-]  */   /*  | 2.2     */
  {  "\\OLDHYP\t",          "P"       },  /*     [P-]  */   /*  |         */
  {  "\\oldhyr\t",          "r"       },  /*     [r-]  */   /*  |         */
  {  "\\OLDHYR\t",          "R"       },  /*     [R-]  */   /*  |         */
  {  "\\oldhyt\t",          "t"       },  /*     [t-]  */   /*  |         */
  {  "\\OLDHYT\t",          "T"       },  /*     [T-]  */   /*  |         */
  {  "\\oldhyf\t",          "f"       },  /*     [f-]  */   /*  |         */
  {  "\\OLDHYF\t",          "F"       },  /*     [F-]  */   /* /          */
  {  "\\hy\t",              "-"       },  /* MINUS \hy */   /* new in 2.2 */
  {  "\\fhy\t",             "-"       },  /* MINUS \fhy*/   /* new in 2.2 */
  {  "\\\"",                ""       },  /* /     \"  */
  {  "\\3",                 "s"      },  /*       \3  */
  {  "\\ck\t",              "ck"      },  /*    [c-k]  */   /* new in 2.2 */
  {  "\\CK\t",              "CK"      },  /*    [C-K]  */   /* new in 2.2 */
  {  "\\oldhyss\t",        "s"       },  /*    [s-s]  */   /* \          */
  {  "\\OLDHYSS\t",        "SS"       },  /*    [S-S]  */   /*  | new     */
  {  "\\newhyss\t",        "s"       },  /*    [-ss]  */   /*  |  in     */
  {  "\\NEWHYSS\t",        "SS"       },  /*    [-SS]  */   /*  | 2.2     */
  {  "\\hyss\t",           "s"       },  /* [s-s/-ss] */   /*  |         */
  {  "\\HYSS\t",           "SS"       },  /* [S-S/-SS] */   /* /          */
  { NULL, NULL }};
#define NUMTEX  44


const char *gstylist[][2] = { 
  {  "\"a",                  NULL     },  /* \      "a */
  {  "\"o",                  NULL     },  /*  |     "o */
  {  "\"u",                  NULL     },  /* CTL+E  "u */
  {  "\"A",                  NULL     },  /*  |     "A */
  {  "\"O",                  NULL     },  /*  |     "O */
  {  "\"U",                  NULL     },  /*  |     "U */
  {  "\"s",                  NULL     },  /*        "s */
  {  "\"S",                  NULL     },  /*        "S */
  {  "\"z",                  NULL     },  /*        "z */   /* new in 2.1 */
  {  "\"Z",                  NULL     },  /*        "Z */   /* new in 2.1 */
  {  "\"<",                  NULL     },  /*        "` */   /* new in 2.2 */
  {  "\">",                  NULL     },  /*        "' */   /* new in 2.2 */
  {  "\"`",                  NULL     },  /*        "` */   /* new in 2.2 */
  {  "\"\'",                 NULL     },  /*        "' */   /* new in 2.2 */
  {  "\"|",                  NULL     },  /*        "| */   /* new in 2.2 */
  {  "\"-",                  NULL     },  /*        "- */   /* new in 2.2 */
  {  "\"c",                  NULL     },  /*     [c-]  */   /* \          */
  {  "\"C",                  NULL     },  /*     [C-]  */   /*  |         */
  {  "\"l",                  NULL     },  /*     [l-]  */   /*  |         */
  {  "\"L",                  NULL     },  /*     [L-]  */   /*  |         */
  {  "\"m",                  NULL     },  /*     [m-]  */   /*  |         */
  {  "\"M",                  NULL     },  /*     [M-]  */   /*  |         */
  {  "\"n",                  NULL     },  /*     [n-]  */   /*  | new     */
  {  "\"N",                  NULL     },  /*     [N-]  */   /*  |  in     */
  {  "\"p",                  NULL     },  /*     [p-]  */   /*  | 2.2     */
  {  "\"P",                  NULL     },  /*     [P-]  */   /*  |         */
  {  "\"r",                  NULL     },  /*     [r-]  */   /*  |         */
  {  "\"R",                  NULL     },  /*     [R-]  */   /*  |         */
  {  "\"t",                  NULL     },  /*     [t-]  */   /*  |         */
  {  "\"T",                  NULL     },  /*     [T-]  */   /*  |         */
  {  "\"f",                  NULL     },  /*     [f-]  */   /*  |         */
  {  "\"F",                  NULL     },  /*     [F-]  */   /*  |         */
  {  "\"=",                  NULL     },  /* MINUS  "= */   /* new in 2.2 */
  {  "\"~",                  NULL     },  /* MINUS  "~ */   /* new in 2.2 */
  {  "\"",                   NULL     },  /* /      "  */
  { NULL, NULL }};
#define NUMGER  35


const char *gweight[][2] = { 
  {  NULL,                  "ae"     },  /* \      \"a "a */
  {  NULL,                  "oe"     },  /*  |     \"o "o */
  {  NULL,                  "ue"     },  /* CTL+E  \"u "u */
  {  NULL,                  "Ae"     },  /*  |     \"A "A */
  {  NULL,                  "Oe"     },  /*  |     \"O "O */
  {  NULL,                  "Ue"     },  /*  |     \"U "U */
  {  NULL,                  "ss"     },  /*        \ss "s */
  {  NULL,                   "SS"     },  /*        \SS "S */
  {  NULL,                  "ss"     },  /*        \sz "z */   /* new in 2.1 */
  {  NULL,                  "SS"     },  /*        \SZ "Z */   /* new in 2.1 */
  {  NULL,                     ""     },  /*     \flqq  "< */   /* new in 2.2 */
  {  NULL,                     ""     },  /*     \frqq  "> */   /* new in 2.2 */
  {  NULL,                     ""     },  /*     \glqq  "` */   /* new in 2.2 */
  {  NULL,                     ""     },  /*     \grqq  "' */   /* new in 2.2 */
  {  NULL,                     ""     },  /*        \/  "| */   /* new in 2.2 */
  {  NULL,                     ""     },  /*        \-  "- */   /* new in 2.2 */
  {  NULL,                    "c"     },  /*      [c-]  "c */   /* \          */
  {  NULL,                    "C"     },  /*      [C-]  "C */   /*  |         */
  {  NULL,                    "l"     },  /*      [l-]  "l */   /*  |         */
  {  NULL,                    "L"     },  /*      [L-]  "L */   /*  |         */
  {  NULL,                    "m"     },  /*      [m-]  "m */   /*  |         */
  {  NULL,                    "M"     },  /*      [M-]  "M */   /*  |         */
  {  NULL,                    "n"     },  /*      [n-]  "n */   /*  | new     */
  {  NULL,                    "N"     },  /*      [N-]  "N */   /*  |  in     */
  {  NULL,                    "p"     },  /*      [p-]  "p */   /*  | 2.2     */
  {  NULL,                    "P"     },  /*      [P-]  "P */   /*  |         */
  {  NULL,                    "r"     },  /*      [r-]  "r */   /*  |         */
  {  NULL,                    "R"     },  /*      [R-]  "R */   /*  |         */
  {  NULL,                    "t"     },  /*      [t-]  "t */   /*  |         */
  {  NULL,                    "T"     },  /*      [T-]  "T */   /*  |         */
  {  NULL,                    "f"     },  /*      [f-]  "f */   /*  |         */
  {  NULL,                    "F"     },  /*      [F-]  "F */   /* /          */
  {  NULL,                    "-"     },  /* MINUS \hy  "= */   /* new in 2.2 */
  {  NULL,                    "-"     },  /* MINUS \fhy "~ */   /* new in 2.2 */
  {  NULL,                    ""     },  /* /      \"  "  */
  {  NULL,                  "ss"     },  /*        \3  [] */
  {  NULL,                   "ck"     },  /*       \ck     */   /* new in 2.2 */
  {  NULL,                   "CK"     },  /*       \CK     */   /* new in 2.2 */
  {  NULL,                  "ss"     },  /*    [s-s]  */   /* \          */
  {  NULL,                   "SS"     },  /*    [S-S]  */   /*  | new     */
  {  NULL,                  "ss"     },  /*    [-ss]  */   /*  |  in     */
  {  NULL,                   "SS"     },  /*    [-SS]  */   /*  | 2.2     */
  {  NULL,                  "ss"     },  /* [s-s/-ss] */   /*  |         */
  {  NULL,                   "SS"     },  /* [S-S/-SS] */   /* /          */
  { NULL, NULL }};


const char *mweight[][2] = { 
  {  NULL,                   "a"     },  /* \     \"a "a */
  {  NULL,                   "o"     },  /*  |    \"o "o */
  {  NULL,                   "u"     },  /* CTL+E \"u "u */
  {  NULL,                   "A"     },  /*  |    \"A "A */
  {  NULL,                   "O"     },  /*  |    \"O "O */
  {  NULL,                   "U"     },  /*  |    \"U "U */
  {  NULL,                  "ss"     },  /*       \ss "s */
  {  NULL,                   "SS"     },  /*       \SS "S */
  {  NULL,                  "ss"     },  /*       \sz "z */   /* new in 2.1 */
  {  NULL,                  "SS"     },  /*       \SZ "Z */   /* new in 2.1 */
  {  NULL,                     ""     },  /*     \flqq "< */   /* new in 2.2 */
  {  NULL,                     ""     },  /*     \frqq "> */   /* new in 2.2 */
  {  NULL,                     ""     },  /*     \glqq "` */   /* new in 2.2 */
  {  NULL,                     ""     },  /*     \grqq "' */   /* new in 2.2 */
  {  NULL,                     ""     },  /*       \/  "| */   /* new in 2.2 */
  {  NULL,                     ""     },  /*       \-  "- */   /* new in 2.2 */
  {  NULL,                    "c"     },  /*      [c-]  "c */   /* \          */
  {  NULL,                    "C"     },  /*      [C-]  "C */   /*  |         */
  {  NULL,                    "l"     },  /*      [l-]  "l */   /*  |         */
  {  NULL,                    "L"     },  /*      [L-]  "L */   /*  |         */
  {  NULL,                    "m"     },  /*      [m-]  "m */   /*  |         */
  {  NULL,                    "M"     },  /*      [M-]  "M */   /*  |         */
  {  NULL,                    "n"     },  /*      [n-]  "n */   /*  | new     */
  {  NULL,                    "N"     },  /*      [N-]  "N */   /*  |  in     */
  {  NULL,                    "p"     },  /*      [p-]  "p */   /*  | 2.2     */
  {  NULL,                    "P"     },  /*      [P-]  "P */   /*  |         */
  {  NULL,                    "r"     },  /*      [r-]  "r */   /*  |         */
  {  NULL,                    "R"     },  /*      [R-]  "R */   /*  |         */
  {  NULL,                    "t"     },  /*      [t-]  "t */   /*  |         */
  {  NULL,                    "T"     },  /*      [T-]  "T */   /*  |         */
  {  NULL,                    "f"     },  /*      [f-]  "f */   /*  |         */
  {  NULL,                    "F"     },  /*      [F-]  "F */   /* /          */
  {  NULL,                    "-"     },  /*      \hy  "= */   /* new in 2.2 */
  {  NULL,                    "-"     },  /*      \fhy "~ */   /* new in 2.2 */
  {  NULL,                    ""     },  /* /     \"  "  */
  {  NULL,                  "ss"     },  /*       \3  [] */
  {  NULL,                   "ck"     },  /*      \ck     */   /* new in 2.2 */
  {  NULL,                   "CK"     },  /*      \CK     */   /* new in 2.2 */
  {  NULL,                  "ss"     },  /*    [s-s]  */   /* \          */
  {  NULL,                   "SS"     },  /*    [S-S]  */   /*  | new     */
  {  NULL,                  "ss"     },  /*    [-ss]  */   /*  |  in     */
  {  NULL,                   "SS"     },  /*    [-SS]  */   /*  | 2.2     */
  {  NULL,                  "ss"     },  /* [s-s/-ss] */   /*  |         */
  {  NULL,                   "SS"     },  /* [S-S/-SS] */   /* /          */
  { NULL, NULL }};



int texlistnum(const char *p, int n, const char *list[][2])
{
   const char *entry, *line;
   int i = 0;

   while (i < n)
   { 
       entry = list[i][0];
       line  = p;
       while (*line == *entry && *entry != '\0' && *entry != '\t')
       { ++line; ++entry;
       }
       if (*entry == '\0' || (*entry == '\t' && isLaTeXcmdletter(*line) == 0)) break;
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

   while (*s != '\0' && *s != '\t') { ++s; ++len; }

   return len;
}


char *overspacesafterlettercmd(int len, const char *q, char *p)
{
  if (len > 0 && isLaTeXcmdletter(*(q+1)) != 0) while (*p == ' ') ++p;

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


void nzvorziehen(char in[], char *p)
{
  int pos = 0, l;
  char mem;
  
  while (*p != '\0')
  {
    if (nichtvorziehen(*p) == 1)
    {
            l = pos;
      while (nichtvorziehen(*p) == 1) in[l++] = *p++;
          mem = in[l-1];
      in[l-1] = in[pos];
      in[pos] = mem;
          pos = l;
    }
    else in[pos++] = *p++;
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
      if ((i=texlistnum(p, NUMGER,  gstylist)) > -1)
      {              p += llstrlen( gstylist[i][0]);
             pos = insertweight(in,          i,      pos, MAXLEN-1);
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


void erweitert(char in[], const char *p)
{
  int pos = 0;
  
  if (encode == 1)
  {
    while (*p != '\0' && pos < MAXLEN-1)
    {
      if (*p < 0) pos = insposmaxstr(in, getTONEenc(upperascii = *p), pos, MAXLEN-1);
      else in[pos++] = *p;
      ++p;
    }
  }
  else
  {
    while (*p != '\0' && pos < MAXLEN-1)
    {
      if (*p > 0) in[pos++] = *p;
      else       upperascii = *p;
      ++p;
    }
  }
  in[pos] = '\0';
}


const char *transformtable[][2] = {
  {              "\\\\",  "[]"                   },
  {        "\\vspace\t",  "{}"                   },
  {        "\\hspace\t",  "{}"                   },
  {        "\\nosort\t",  "{}"                   },
  {         "\\index\t",  "{}"                   },
  {      "\\glossary\t",  "{}"                   },
  {         "\\label\t",  "{}"                   },
  {      "\\pbalabel\t",  "{}"                   },
  {       "\\balabel\t",  "{}"                   },
  {       "\\pageref\t",  "{}"                   },
  {        "\\pbaref\t",  "[]{}"                },
  {         "\\baref\t",  "[]{}"                },
  {           "\\ref\t",  "{}"                   },
  {     "\\linebreak\t",  "[]"                   },
  {   "\\nolinebreak\t",  "[]"                   },
  {     "\\pagebreak\t",  "[]"                   },
  {   "\\nopagebreak\t",  "[]"                   },
  {      "\\footnote\t",  "[]{}"                },
  {  "\\footnotetext\t",  "[]{}"                },
  {  "\\footnotemark\t",  "[]"                   },
  {      "\\framebox\t",  "[][]"                },
  {       "\\makebox\t",  "[][]"                },
  {        "\\parbox\t",  "[]{}"                },
  {      "\\raisebox\t",  "{}[][]"             },
  {          "\\rule\t",  "[]{}{}"             },
  {  "\\printonlyvqu\t",  "[]{}{}*{}{}"       },   /* innerVcmd */
  {           "\\vqu\t",  "[]{}{}*{}{}"       },
  {  "\\printonlyvli\t",  "[]{}{}*{}{}"       },   /* olny      */
  {           "\\vli\t",  "[]{}{}*{}{}"       },
  { "\\xprintonlyvqu\t",  "[]{}{}*{}{}"       },   /* changes   */
  {          "\\xvqu\t",  "[]{}{}*{}{}"       },
  { "\\xprintonlyvli\t",  "[]{}{}*{}{}"       },   /* IS_V      */
  {          "\\xvli\t",  "[]{}{}*{}{}"       },
  {     "\\@shadowba\t",  "{}"             },   /* new. in 2.2 */
  {       "\\shadowv\t",  "[]{}{}{}{}" },
  {      "\\addtovli\t",  "[]{}{}{}{}" },
  {      "\\addtovqu\t",  "[]{}{}{}{}" },
  {     "\\xaddtovli\t",  "[]{}{}{}{}" },
  {     "\\xaddtovqu\t",  "[]{}{}{}{}" },
  {  "\\printonlykqu\t",  "[]{}*{}{}"          },
  {  "\\printonlykli\t",  "[]{}*{}{}"          },
  { "\\xprintonlykqu\t",  "[]{}*{}{}"          },
  { "\\xprintonlykli\t",  "[]{}*{}{}"          },
  {       "\\shadowk\t",  "[]{}{}{}"    },
  {      "\\addtokli\t",  "[]{}{}{}"    },
  {      "\\addtokqu\t",  "[]{}{}{}"    },
  {     "\\xaddtokli\t",  "[]{}{}{}"    },
  {     "\\xaddtokqu\t",  "[]{}{}{}"    },
  {          "\\frac\t",  "{}{}"                },
  {       "\\vauthor\t",  "{}{}"                },
  {    "\\midvauthor\t",  "{}{}"                },
  {       "\\kauthor\t",  "{}"                   },
  {    "\\midkauthor\t",  "{}"                   },
  {         "\\ersch\t",  "{}[]{}{}"       },
  {        "\\persch\t",  "{}[]{}{}"       },
  {  "\\printonlyper\t",  "{}"                   },
  {  "\\printonlyarq\t",  "[]{}{}"             },  /* new in 2.2 */
  {  "\\printonlydefabk\t",  "{}{}"             },  /* new in 2.2 */
  {  "\\printonlyabkdef\t",  "{}{}"             },  /* new in 2.2 */
  {       "\\shadowo\t",  "{}"             },
  {      "\\addtoper\t",  "{}"             },
  {       "\\shadowt\t",  "{}{}"          },
  {      "\\addtoarq\t",  "{}{}"          },
  {     "\\shadowone\t",  "{}"                   },
  {      "\\addtoabk\t",  "{}"                   },
  {      "\\addtogrr\t",  "{}"                   },
  {      "\\addtoprr\t",  "{}"                   },
  {      "\\addtosrr\t",  "{}"                   },
  {     "\\shadowtwo\t",  "{}{}"                },
  {       "\\fillper\t",  "{}{}"                },
  {       "\\fillarq\t",  "{}{}"                },
  {       "\\fillgrr\t",  "{}{}"                },
  {       "\\fillprr\t",  "{}{}"                },
  {       "\\fillsrr\t",  "{}{}"                },
  {   "\\addtodefabk\t",  "{}{}"                },
  {   "\\addtoabkdef\t",  "{}{}"                },
  {    "\\errmessage\t",  "{}"                   },
  {  "\\GenericError\t",  "{}{}{}{}"          },
  {       "\\message\t",  "{}"                   },
  {       "\\typeout\t",  "{}"                   },
  {    "\\mathhexbox\t",  "{}{}{}"             },
  {       "\\phantom\t",  "{}"                   },
  {      "\\vphantom\t",  "{}"                   },
  {      "\\hphantom\t",  "{}"                   },
  {   "\\showhyphens\t",  "{}"                   },
  {"\\sethyphenation\t",  "{}"                   },
  {"\\selectlanguage\t",  "{}"                   },
  { "\\discretionary\t",  "{}{}"                },
  {    "\\pstressing\t",  "{}"                   },
  {     "\\stressing\t",  "{}"                   },
  {     "\\pxbibmark\t",  "{}{}"                },  /* Nur \\p...  */
  {      "\\pxbibref\t",  "{}"                   },  /* bearbeiten! */
  {      "\\pbibmark\t",  "{}"                   },
  {       "\\bibmark\t",  "{}"                   },
  {             KURZCMD,  "{}"                   },
  {       "\\delkurz\t",  "{}"                   },
  {         "\\begin\t",  "{}"                   },
  {           "\\end\t",  "{}"                   },
  {     "\\pfordinal\t",  "{}"                   },  /* new in 2.2 */
  { NULL, NULL }};
#define NUMTRANSV  99


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


void exchange_v(char *p, const char *bra, int num)
{
  int l = 0, la = 0, i = 0, lb = 0, j = 0, k;
  char buf[MAXLEN], fub[MAXLEN], *lp = p, *anf = p;
  
  while (*p == ' ') { ++p; ++la; }
  if ((l=nextbracket(p, *(bra+(3*num+1)), *(bra+(3*num+2)))) > 0)
  {
    la += l; while (i < la) buf[i++] = *lp++;
     p += l;
    if (*(bra+(3*(num+1))) == '')
    {
      while (*p == ' ') { ++p; ++lb; }
      if ((l=nextbracket(p, *(bra+(3*(num+1)+1)), *(bra+(3*(num+1)+2)))) > 0)
      {
        lb += l; while (j < lb) fub[j++] = *lp++;
        k = 0; while (k < j) *anf++ = fub[k++];
        k = 0; while (k < i) *anf++ = buf[k++];
      }
    }
    else intern_err(10);  /* No formatter Strg+Q after Strg+P */
  }
}


char *handletransformarg(char *p, const char *bra)
{
  int l = 0, num = 0;
  char *q = p;

  while (*(bra+(3*num)) != '\0' && *q != '\0')
  {
    if ('' == *(bra+(3*num)) || '' == *(bra+(3*num)))
    {
      while (*q == ' ') ++q;
      if ((l=nextbracket(q, *(bra+(3*num+1)), *(bra+(3*num+2)))) < 2) l = 0;
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
    if ('' == *(bra+(3*num)) || '' == *(bra+(3*num)) || '' == *(bra+(3*num)))
    {
      if ('' == *(bra+(3*num))) exchange_v(q, bra, num);
      while (*q == ' ') ++q;
      if ((l=nextbracket(q, *(bra+(3*num+1)), *(bra+(3*num+2)))) < 2) l = 0;
      if (l > 0)
      { 
        q += l-1;
        q = bracketsformward(q, *(bra+(3*num+2)));
        ++q;
      }
    }
    else
    if ('*' == *(bra+(3*num)))
    {
      while (*q == ' ') ++q;
      if (*q == '*')
      { *q = OPTSTAR;
        ++q;
        while (*q == ' ') ++q;
        if ((l=nextbracket(q, *(bra+(3*num+1)), *(bra+(3*num+2)))) < 2) l = 0;
        if (l > 0)
        { 
          q += l-1;
          q = bracketsformward(q, *(bra+(3*num+2)));
          ++q;
        }
      }
    }
    ++num;
  }

  return p;
}


void transformline(char buf[], char *p)
{
  int pos = 0, i, l;

  while (*p != '\0' && pos < MAXLEN-2)
  {
    if (*p == '\\')
    {
      if ((i=texlistnum(p, NUMTRANSV, transformtable)) > -1)
      {
          if (*(p+1) == '\\') buf[pos++] = ' ';
          p += llstrlen(transformtable[i][0]);
          p = handletransformarg(p, transformtable[i][1]);
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
          transformline(buf,                in);  /* veraendert zweites Arg! */
            erweitert  ( in,               buf);
        p = austauschen(buf,                in, dqsort[linec]);


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


void del_ktitintro(char *s)
{
  while(*s != '\0')
  {
    if (*s == KTITINTROCH)
    {
      if (*(s+1) != '\0')
      {
        *s++ = KTITERRORCH;     /* "????" is possible here,    */
        *s++ = KTITERRORCH;     /* whereas BibArts prints "??" */
      }
      else 
      { 
        intern_err(30);
        break;
      }
    }
    else ++s;
  }
}


int writetheline(char *p, const int nlines, const int line, const int deepsort, int filec, int printsort, int kandkused, int oktitused)
{
  int doprint = 0, pagenum = 0, fntnum = 0, uu = 0, tn = getthirdnum(line), j, ok, ie, sha, tha, uha, oerr;

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
                              fprintf(outfile, "%%%%  <- Warning: You have used %s here, but %s elsewhere.\n", KURZ, KTIT);

    if (otherktiterr > 0)
    {
      j = 0;
      while (j < otherktiterr && subAptr[j][line] != mydefault)
      {
        if ((oerr = get_add_ktit(subAptr[j][line], 0, -1, -1, 0)) > 0)
                              fprintf(outfile, "%%%%  <- Warning: %d misplaced %s in arg %d.\n", oerr, KTIT, j+1);
        ++j;
      }
    }

    if (oktitused > 0 && subAptr[VCOLS-1][line] != mydefault)
    {
      ok = num_of_cmds_in_str(subAptr[VCOLS-1][line], KTITCMD);
      ie =      hasinnerentry(subAptr[VCOLS-1][line], innerK_PER_ARQ_cmd, INNERTITLES);

      if (    (oktitused == 3 || oktitused == 4)
           &&  ok == 0
           &&  ie > -1
         )
                              fprintf(outfile, "%%%%  <- Info: No outer %s used in **item with inner title**, but elsewhere you have %s in that case.\n", KTIT, KTIT);
      else if (ok == 0 && oktitused < 5)
      {
                              fprintf(outfile, "%%%%  <- Info: No");
          if (ie > -1)        fprintf(outfile, " OUTER");
                              fprintf(outfile, " %s in arg %d (other entries have %s).\n", KTIT, VCOLS, KTIT);
      }
      else if (ok > 1)
      {
                              fprintf(outfile, "%%%%  <- Warning: %d", ok);
                              fprintf(outfile, " %s in last arg outside inner v-cmd found.  Also check %s file.\n", KTIT, VKC_SUFFIX);  /* VCOLS */
      }

      if      (oktitused == 2 || oktitused == 4)
      {
        if (ok > (j=unshielded_cmds_in_str(subAptr[VCOLS-1][line], KTITCMD)))
        {
                              fprintf(outfile, "%%%%  <- Warning: %d", ok-j);
                              fprintf(outfile, " { %s } shadowed by brackets (in last arg outside inner v-cmd).\n", KTIT);  /* VCOLS */
        }
      }

      if   (     isinstr(subAptr[VCOLS-1][line], KTITINTRO) == 1)
      {
                              fprintf(outfile, "%%%%  -> Info: An inner v-cmd should be printed as shortened reference (created by the arg of your %s cmd).\n", KTIT);
        if ((sha=isinstr(subAptr[VCOLS-1][line], KTITSHA  )) == 1)
                              fprintf(outfile, "%%%%  <- Error: INNER { %s } is shadowed by brackets, and ...\n", KTIT);
        if ((tha=isinstr(subAptr[VCOLS-1][line], KTITMISS )) == 1)
                              fprintf(outfile, "%%%%  <- Error: INNER %s is missing.\n", KTIT);
        if ((uha=isinstr(subAptr[VCOLS-1][line], KTITEMPTY)) == 1)
        {
                              fprintf(outfile, "%%%%  <- Warning: Arg of");
          if (sha == 1)       fprintf(outfile, " (last non-shadowed)");
                              fprintf(outfile, " INNER %s is empty.\n", KTIT);
        }
        if (sha != 1 && tha != 1 && uha != 1) intern_err(28);  /* extra KTITINTRO found */
      
        del_ktitintro(subAptr[VCOLS-1][line]);  /* aus Prinzip: "\tX" => "??" */
      }
    }

  }


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


void printfilelist(FILE *file, int filec, int cc, const char *vor, const char *nach)
{
    int i = 0;

  if (cc == 1)
     fprintf(file, "\n%%%%   BibArts 2.2  (C) Timo Baumann  2019   [%s]\n", __DATE__);

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
     if (encode == 1)            fprintf(file, " -t1");
     if (kill == DOKILLAUTHORS)  fprintf(file, " -k");
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
          && encode == 0
          && kill != DOKILLAUTHORS
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


int writelines(int nlines, const int dqcol, const int lancol, const int commentANDignlines, const int deepsort, int filec, int printsort, int kandkused, int oktitused)
{
   int i = 0, dqcat, lancat = 0, lancatbuf, extralines = 0, didsetorig, nerr = 0;

   printfilelist(outfile, filec, 1, "\\gdef\\bibsortargs{", " }");

   while (i < nlines && leval[i] != ACCEPTLINE)
   { 
     nerr = nerr + writetheline(lineptr[i], 0, i, 0, filec, printsort, kandkused, oktitused);
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

     nerr = nerr + writetheline(lineptr[i], nlines, i, deepsort, filec, printsort, kandkused, oktitused);

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

     nerr = nerr + writetheline(lineptr[i], nlines, i, deepsort, filec, printsort, kandkused, oktitused);

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
  int r = entkerne_arg(buf, s, IeC, '{', '}');
         del_ktitintro(buf);
  return r;
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


const char *authortable[][2] = {
  {  "\\vauthor\t",     "\\vauthor"     },
  {  "\\midvauthor\t",  "\\midvauthor"  },
  {NULL, NULL}};
#define  NAUTHOR 2


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


int hasinnerentry(const char *p, const char *icmdlist[][2], const int num)
{
   int r = -1;

   while (*p != '\0')
   {
     if (*p == '\\')
     {
       if (*(p+1) == '\\') ++p;
       else
       if ((r=innerentry(p, icmdlist, num)) > -1) break;
     }
     ++p;
   }

   return r;
}


int check_ktit(int nlines, int ventry)
{
   int i = 0, ohask = 0, ohasnok = 0, elsehask = 0, shadowedlines = 0, unshadnum = 0, l, ll, badlines = 0;
   int ohk[MAXINFO];

   if (ventry != IS_V) return 0;

   while (i < nlines)
   {
     if (subAptr[VCOLS-1][i] == mydefault)
     { 
       ++badlines;
     }
     else
     {
                     l =    num_of_cmds_in_str(subAptr[VCOLS-1][i], KTITCMD);
       unshadnum += (ll=unshielded_cmds_in_str(subAptr[VCOLS-1][i], KTITCMD));

       if (l > ll)
       {
         if (shadowedlines == 0)
         printf("%%%%\n%%%%>  Warning: { %s } is shadowed by brackets ...\n", KTIT);
         printf("%%%%     ... in entry from %s:\n", subRptr[LINECOL][i]);
         printV(i);
         ++shadowedlines;
       }

       if (hasinnerentry(subAptr[VCOLS-1][i], innerK_PER_ARQ_cmd, INNERTITLES) > -1)
       {
         if (l > 0)
         {
              ++ohask;
         }
         else
         {
            if (ohasnok < MAXINFO) ohk[ohasnok] = i;
              ++ohasnok;
         }
       }
       else
       if   (l > 0)
              ++elsehask;
     }
     ++i;
   }

   if (shadowedlines > 0) printf("%%%%>  A shadowed outer %s can not create %s in your text;\n%%%%   a shadowed inner %s can not create a shortened reference in the list.\n%%%%\n", KTIT, ANNOUNCEKTIT, KTIT);

   if (badlines > 0)
       printf("%%%%>  %s: Can\'t check all entries (%d are corrupt).\n", KTIT, badlines);


   if (badlines < nlines)
   {

     if (ohask > 0 && ohasnok > 0)
     {
       printf("%%%%>  Info: Items with inner title: %d with outer %s found, %d without.\n", ohask, KTIT, ohasnok);
     }


     if (ohasnok > 0)
     {
       printf("%%%%\n%%%%>  Info: Entries with inner reference and no outer %s ...\n", KTIT);
       i = 0;
       while (i < MAXINFO && i < ohasnok)
       {
         l = ohk[i];
         printf("%%%%     ... from %s:\n", subRptr[LINECOL][l]);
         printV(l);
         ++i;
       }
       if (ohasnok > 0)
         printf("%%%%>  This have been the first %d (of %d) entries without outer %s.\n%%%%\n", i, ohasnok, KTIT);
     }


     if (    (shadowedlines == 0 && badlines == 0)
          || (elsehask + ohask == 0 && badlines == 0)
        )
       printf("%%%%>  Info: ");


     if (shadowedlines == 0 && badlines == 0 && (ohask > 0 || elsehask > 0))
     {
       if (elsehask+ohask == nlines) printf("All");
       else                          printf("NOT all");
       printf(" v-entries have (outer) %s cmds in their last arg", KTIT);   /* VCOLS */
       if (elsehask+ohask == nlines) printf(".\n");
       else                          printf(":\n%%%%     In case of %s see messages in the output file.\n", ANNOUNCEKTIT);

       if (nlines < unshadnum)
       printf("%%%%     I count %d unshadowed %s in %d entries (%d too much).\n", unshadnum, KTIT, nlines, unshadnum-nlines);
     }
     else if (elsehask + ohask == 0 && badlines == 0)
     {
       printf("You\'ve never used %s in last v-args:\n", KTIT);
       printf("%%%%     No info on outer %s in output file; no %s done.\n", KTIT, ANNOUNCEKTIT);
     }
   }


   l = 1;
   if (elsehask + ohask == 0)    l += 4;
   if (ohask > 0 && ohasnok > 0) l += 2;
   if (shadowedlines > 0) ++l;
   return l;
}


void prepareoutput(const char *entry, const char *suffix, int deep, int deepsort, int filec, char *arg_i, char *arg_o, int prep_kill, const char *formO, int ventry, int printsort)
{
    int nlines = 0, i = 0, extralines = 0, commentANDignlines = 0;
    int filenum = -1, kandkused = 0, oktitused = 0;
    char *outname = mydefault;
    
    if (arg_o == NULL) outname = makename(arg_i, suffix);
    else               outname = makename(arg_o, suffix);

    otherktiterr = 0;

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
        oktitused = check_ktit(nlines, ventry);

        if ((outfile = fopen(outname, "w")) == NULL)
        {
          printf("%%%%\n%%%%>  Error: Can\'t write to file \"%s\".\n", outname);
          openfailed(outname);
        }
        else
        { printf("%%%%>  Write %d item(s) to file \"%s\".\n", nlines-commentANDignlines, outname);
          if ((extralines = writelines(nlines, DQCOL, LANCOL, commentANDignlines, deepsort, filec, printsort, kandkused, oktitused)) > 0)
          printf("%%%%    (Have added %d line(s) containing commands for LaTeX.)\n", extralines);
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


int main(int argc, char *argv[])
{
        int nlines = 0;
        int usedospath = 0;
        int filec = 0;
        int get_g = 0;
        int printsort = 0;
   char *inname = mydefault, *arg_i = NULL, *arg_o = NULL;


          printf("\n%%%%>  This is bibsort 2.2  (for help:  %s -\?)\n", getmyname(argv[0], 37));
   fprintf(stderr, "%%%%      bibsort 2.2 is part of BibArts 2.2    (C) Timo Baumann  2019.\n");


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
     encode = 1;
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
     moreinfo();
     announceexit("get your question for more help");
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
     /*  "----" KHVONUM Positionen: \\onlykurz \\onlyhere \\onlyvoll \\onlyout  */

   prepareoutput("%\\literentry", VLI_SUFFIX, VCOLS,     VCOLS, filec, arg_i, arg_o,          kill, "llee", IS_V, printsort);
   prepareoutput("%\\quellentry", VQU_SUFFIX, VCOLS,     VCOLS, filec, arg_i, arg_o,          kill, "llee", IS_V, printsort);
   prepareoutput("%\\vkcitentry", VKC_SUFFIX, KCOLS,     KCOLS, filec, arg_i, arg_o, DOKILLK,       "elle", NO_V, printsort);

   prepareoutput("%\\abkrzentry", ABK_SUFFIX, ACOLS, ACOLSSORT, filec, arg_i, arg_o, DOKILLABBREVS, "llee", NO_V, printsort);
   prepareoutput("%\\perioentry", PER_SUFFIX, PCOLS, PCOLSSORT, filec, arg_i, arg_o, DOKILLREGISTS, "llee", NO_V, printsort);
   prepareoutput("%\\archqentry", ARQ_SUFFIX, PCOLS, PCOLSSORT, filec, arg_i, arg_o, DOKILLREGISTS, "llee", IS_Q, printsort);

   prepareoutput("%\\geogrentry", GRR_SUFFIX, PCOLS, PCOLSSORT, filec, arg_i, arg_o, DOKILLREGISTS, "llee", NO_V, printsort);
   prepareoutput("%\\persrentry", PRR_SUFFIX, PCOLS, PCOLSSORT, filec, arg_i, arg_o, DOKILLREGISTS, "llee", NO_V, printsort);
   prepareoutput("%\\subjrentry", SRR_SUFFIX, PCOLS, PCOLSSORT, filec, arg_i, arg_o, DOKILLREGISTS, "llee", NO_V, printsort);



   if (upperascii < 0)
   {    
     if (encode == 1)
     {
          printf("%%%%\n%%%%>  -t1: Gave 8-bit character %d sorting weight \"%s\" ....\n", ((unsigned char)upperascii), getTONEenc(upperascii));
       fprintf(stderr, "%%%%   Did NOT ignore upper ascii characters ({T1} and no {inputenc} assumed).\n");
     }
     else
     {
          printf("%%%%\n%%%%>  Info: Gave 8-bit character %d NO sorting weight. Use -t1\?\n", ((unsigned char)upperascii));
       fprintf(stderr, "%%%%   Did ignore all upper ascii characters ({OT1} and no {inputenc} assumed).\n");
     }
   }
   else if (encode == 1)
          printf("%%%%\n%%%%>  -t1 is unnecessary: No upper ascii characters found.\n");

   return 0;
}


/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
