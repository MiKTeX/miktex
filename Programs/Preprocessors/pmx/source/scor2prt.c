#line 1 ""
/*  -- translated by f2c (version 20160102).
   You must link the resulting object file with libf2c:
	on Microsoft Windows system, link with libf2c.lib;
	on Linux or Unix systems, link with .../path/to/libf2c.a -lm
	or, if you install libf2c.a in a standard place, with -lf2c -lm
	-- in that order, at the end of the command line, as in
		cc *.o -lf2c -lm
	Source for libf2c is in /netlib/f2c/libf2c.zip, e.g.,

		http://www.netlib.org/f2c/libf2c.zip
*/

#include "f2c.h"

#line 1 ""
/* Common Block Declarations */

struct all_1_ {
    integer noinow, iorig[24], noinst;
    logical insetup, replacing;
    integer instnum[24];
    logical botv[24];
    integer nvi[24], nsyst, nvnow;
};

#define all_1 (*(struct all_1_ *) &all_)

/* Initialized data */

struct {
    integer e_1;
    integer fill_2[24];
    integer e_3;
    integer fill_4[26];
    logical e_5[24];
    integer e_6[24];
    integer fill_7[2];
    } all_ = { 24, {0}, 24, {0}, FALSE_, FALSE_, FALSE_, FALSE_, FALSE_, 
	    FALSE_, FALSE_, FALSE_, FALSE_, FALSE_, FALSE_, FALSE_, FALSE_, 
	    FALSE_, FALSE_, FALSE_, FALSE_, FALSE_, FALSE_, FALSE_, FALSE_, 
	    FALSE_, FALSE_, FALSE_, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	    1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };


/* Table of constant values */

static integer c__9 = 9;
static integer c__1 = 1;
static integer c__27 = 27;
static integer c__2 = 2;
static integer c__128 = 128;
static integer c_n1 = -1;
static integer c__0 = 0;
static integer c__3 = 3;
static integer c__4 = 4;
static integer c__5 = 5;
static integer c__79 = 79;
static integer c__125 = 125;

/* ccccccccccccccccccccccccc */
/* c */
/* c  scor2prt 2/19/16 for PMX 2.74 */
/* ccccccccccccccccccccccccc */
/* This program, developed by Don Simons (dsimons@roadrunner.com), is */
/* part of the PMX distribution, PMX is a preprocessor for MusiXTeX. In concert */
/* with MusiXTeX and TeX, the purpose of PMX is to allow the user to create */
/* high-quality typeset musical scores by including a sequence of PMX commands */
/* in an ASCII input file. Scor2prt is an auxiliary program that creates PMX */
/* input files for individual parts from the PMX input file for the score. */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

/* 2/19/16 */
/*   Exit gracefully when last input line is comment, with mods in */
/*     subroutine chkcom, adding logical argument goto999 */
/*     which is set to .true. in that case, causing input file to */
/*     be closed after exiting. */
/* 4/9/12 */
/*   Add logical declarations to subroutine dosetup */
/* 8/8/11 */
/*   Copy & mod Ki into parts when after beginning. */
/* 7/18/11 */
/*   Start to fix up for AS..., also Ki was done a little earlier. */
/* 7/5/10 */
/*   Modify Manual to explain extended hex numbering for part-specific comments */
/* 5/16/10 */
/*   Extend part-specific comments for up to 24 parts (subroutine chkcom) */
/*     Allow files 11-34 for parts */
/*     Change file 30 to 40 */
/* 6/1/08 es2p */
/*   noimax = 24 */

/* To do */
/*  Override fracindent, musicsize? */
/*  Deal with midbar R? */
/* 2/29/04 */
/*  Check for special character being in quoted string. */
/* 10/20/02 PMX 2.407 */
/*  Interpret AN[n]"[used-defined-part-file-name]" */
/* 1/21/02 */
/*  Deals with rm in multiple parts. */
/* s2p15 */
/*  Get right non-tex index even if there's "\" inside D"..." */
/* s2p14 */
/*  10/29/00 fix transfer to parts of negative mtrdnp */
/*  4/29/00 screen for "%%" followed by "T" */
/*  Restore change from "Version 2.1a, 21 Dec" */
/* s2p13 */
/*  Allow whole-bar double-whole rests. */
/*  Bypass MIDI commands "I..." achar(10)="I" */
/* s2p12 */
/*  Let %ablabla ... %cblabla represent pmx input for parts 10-12.  But %?blabla */
/*    only taken as such if ? represents a hex digit .le. noinst, otherwise it's */
/*    a simple comment.  This lessens incompatibility. */
/*  In subroutine mbrest, properly open parts 10-12 if needed */
/* s2p11 */
/*  Ignore leading blanks */
/*  Fix undefined linelength in mbrest at very end of comments. */
/* s2p10 */
/*  Fix non-transfer of P..c" " */
/*  Allow "%%" and "%"n anywhere */
/* Version 1.43 */
/*  Fix bug with P in TeX string. */
/*  Ignore shifted whole-bar rests when consolidating whole-bar rests */
/*  Copy type 4 TeX into all parts. */
/*  Deal with XB and XP. */
/*  Permit transfer of blank line into parts */
/*  Change staves/inst in 'M' command. */
/*  Arbitrary staves/inst. */
/*  Recognize m1/2/3/4 syntax. */
/*  Enable comment and one-voice syntax in instrument names */

/* Changes since 1.1 */
/*  Deal with saved macros. */
/*  Revise setup readin, to admit comments. */
/*  Do not copy 'X' into parts */

/* ccccccccccccccccccccccccccccccccccccccccccccccc */
/* Main program */ int MAIN__(void)
{
    /* Initialized data */

    static char achar[1*10] = "P" "m" "V" "R" "A" "h" "w" "K" "M" "I";

    /* System generated locals */
    address a__1[2], a__2[3];
    integer i__1, i__2, i__3[2], i__4[3];
    char ch__1[44], ch__2[31], ch__3[4], ch__4[1], ch__5[2], ch__6[131];
    cilist ci__1;
    icilist ici__1;
    olist o__1;
    cllist cl__1;
    alist al__1;
    inlist ioin__1;

    /* Builtin functions */
    integer s_wsle(cilist *), do_lio(integer *, integer *, char *, ftnlen), 
	    e_wsle(void), s_rsfe(cilist *), do_fio(integer *, char *, ftnlen),
	     e_rsfe(void);
    /* Subroutine */ int s_stop(char *, ftnlen);
    integer i_indx(char *, char *, ftnlen, ftnlen);
    /* Subroutine */ int s_copy(char *, char *, ftnlen, ftnlen), s_cat(char *,
	     char **, integer *, integer *, ftnlen);
    integer f_inqu(inlist *), f_open(olist *), s_cmp(char *, char *, ftnlen, 
	    ftnlen), f_back(alist *), s_wsfe(cilist *), e_wsfe(void), s_rsfi(
	    icilist *), e_rsfi(void), f_clos(cllist *);

    /* Local variables */
    extern /* Subroutine */ int oddquotesbefore_(char *, integer *, logical *,
	     ftnlen);
    static logical clefpend;
    static real xmtrnum0;
    static char templine[128];
    extern /* Subroutine */ int allparts_(char *, integer *, ftnlen);
    static integer musicsize, ia, ib, ic, iv;
    static char sq[1];
    static integer nv, idxendname;
    static real fracindent;
    static integer ibb, len, ivq, iend, nchk, idxa, ljob;
    static char line[128];
    static integer isig, locp, idxq, ndxm, idxs, inst, idxq2, idxstartname;
    extern integer iargc_(void);
    static integer kvend;
    extern /* Subroutine */ int zapbl_(char *, integer *, ftnlen);
    static integer itemp, iinst, ntinx;
    static logical oneof2, goto999;
    extern doublereal readin_(char *, integer *, integer *, ftnlen);
    extern /* Subroutine */ int chkcom_(char *, logical *, ftnlen), getarg_(
	    integer *, char *, ftnlen);
    static integer npages;
    static char holdln[128];
    static integer iudpfn;
    static logical yesodd;
    static integer ludpfn[24];
    static char udpfnq[128*24];
    static integer iiinst;
    static logical fexist;
    static integer ivlast, mtrdnp;
    extern integer lenstr_(char *, integer *, ftnlen);
    static logical frstln;
    static integer insnow, ndxpmx, mtrnmp;
    static logical isachar;
    static char jobname[27];
    static integer lenhold, lenline;
    static char infileq[27];
    static logical gotname;
    static integer iccount, mtrdenl;
    extern integer ntindex_(char *, char *, ftnlen, ftnlen);
    static integer numargs;
    extern /* Subroutine */ int mbrests_(integer *, char *, integer *, 
	    integer *, char *, ftnlen, ftnlen);
    static logical termrpt;
    static integer mtrnuml;
    static char termsym[2], instrum[128*24];
    static integer kvstart;

    /* Fortran I/O blocks */
    static cilist io___6 = { 0, 6, 0, 0, 0 };
    static cilist io___8 = { 0, 6, 0, 0, 0 };
    static cilist io___9 = { 0, 6, 0, 0, 0 };
    static cilist io___10 = { 0, 5, 0, "(a)", 0 };
    static cilist io___13 = { 0, 6, 0, 0, 0 };
    static cilist io___17 = { 0, 6, 0, 0, 0 };
    static cilist io___20 = { 0, 10, 0, "(a)", 0 };
    static cilist io___23 = { 0, 10, 0, "(a)", 0 };
    static cilist io___24 = { 0, 10, 0, "(a)", 0 };
    static cilist io___39 = { 0, 6, 0, 0, 0 };
    static cilist io___40 = { 0, 6, 0, 0, 0 };
    static cilist io___41 = { 0, 6, 0, 0, 0 };
    static cilist io___42 = { 0, 10, 0, "(a)", 0 };
    static cilist io___43 = { 0, 10, 0, "(a)", 0 };
    static cilist io___45 = { 0, 10, 0, "(a)", 0 };
    static cilist io___47 = { 0, 10, 0, "(a)", 0 };
    static cilist io___49 = { 0, 6, 0, 0, 0 };
    static cilist io___50 = { 0, 0, 0, "(a)", 0 };
    static cilist io___51 = { 0, 10, 0, "(a)", 0 };
    static cilist io___52 = { 0, 10, 0, "(a)", 0 };
    static cilist io___56 = { 0, 10, 0, "(a)", 0 };
    static cilist io___58 = { 0, 0, 0, "(a2/a)", 0 };
    static cilist io___60 = { 0, 10, 1, "(a)", 0 };
    static cilist io___62 = { 0, 10, 0, "(a)", 0 };
    static cilist io___63 = { 0, 10, 0, "(a)", 0 };
    static cilist io___69 = { 0, 6, 0, 0, 0 };
    static cilist io___70 = { 0, 6, 0, 0, 0 };
    static cilist io___73 = { 0, 10, 0, "(a)", 0 };
    static cilist io___77 = { 0, 6, 0, 0, 0 };
    static cilist io___78 = { 0, 6, 0, 0, 0 };
    static cilist io___79 = { 0, 6, 0, 0, 0 };
    static cilist io___82 = { 0, 10, 0, "(a)", 0 };
    static cilist io___84 = { 0, 6, 0, 0, 0 };
    static cilist io___85 = { 0, 6, 0, 0, 0 };
    static icilist io___86 = { 0, line+2, 0, "(i1)", 1, 1 };
    static icilist io___89 = { 0, line+2, 0, "(i2)", 2, 1 };
    static cilist io___90 = { 0, 6, 0, 0, 0 };
    static cilist io___91 = { 0, 6, 0, 0, 0 };
    static cilist io___98 = { 0, 0, 0, "(a)", 0 };
    static cilist io___103 = { 0, 0, 0, "(a)", 0 };
    static cilist io___104 = { 0, 0, 0, "(a)", 0 };


#line 104 ""
    clefpend = FALSE_;
#line 105 ""
    all_1.insetup = TRUE_;
#line 106 ""
    all_1.replacing = FALSE_;
#line 107 ""
    frstln = TRUE_;
#line 108 ""
    lenhold = 0;
#line 109 ""
    *(unsigned char *)sq = '\\';
#line 110 ""
    s_wsle(&io___6);
#line 110 ""
    do_lio(&c__9, &c__1, "This is scor2prt for PMX 2.74, 19 February 2016", (
	    ftnlen)47);
#line 110 ""
    e_wsle();
#line 111 ""
    numargs = iargc_();
#line 112 ""
    if (numargs == 0) {
#line 113 ""
	s_wsle(&io___8);
#line 113 ""
	do_lio(&c__9, &c__1, "You could have entered a jobname on the comman"\
		"d line,", (ftnlen)53);
#line 113 ""
	e_wsle();
#line 114 ""
	s_wsle(&io___9);
#line 114 ""
	do_lio(&c__9, &c__1, "      but you may enter one now:", (ftnlen)32);
#line 114 ""
	e_wsle();
#line 115 ""
	s_rsfe(&io___10);
#line 115 ""
	do_fio(&c__1, jobname, (ftnlen)27);
#line 115 ""
	e_rsfe();
#line 116 ""
	numargs = 1;
#line 117 ""
    } else {
/*       call getarg(1,jobname,idum) ! May need to replace this w/ next line */
#line 119 ""
	getarg_(&c__1, jobname, (ftnlen)27);
#line 120 ""
    }
#line 121 ""
    ljob = lenstr_(jobname, &c__27, (ftnlen)27);
#line 122 ""
    if (ljob == 0) {
#line 123 ""
	s_wsle(&io___13);
#line 123 ""
	do_lio(&c__9, &c__1, "No was jobname entered. Restart and try again.",
		 (ftnlen)46);
#line 123 ""
	e_wsle();
#line 124 ""
	s_stop("", (ftnlen)0);
#line 125 ""
    }

/*  Strip ".pmx" if necessary */

/* Computing MAX */
#line 129 ""
    i__1 = i_indx(jobname, ".pmx", (ftnlen)27, (ftnlen)4), i__2 = i_indx(
	    jobname, ".PMX", (ftnlen)27, (ftnlen)4);
#line 129 ""
    ndxpmx = max(i__1,i__2);
#line 130 ""
    if (ndxpmx > 0) {
#line 131 ""
	s_copy(jobname, jobname, (ftnlen)27, ndxpmx - 1);
#line 132 ""
	ljob += -4;
#line 133 ""
    }

/*  Check for existence of input file */

/* Writing concatenation */
#line 137 ""
    i__3[0] = ljob, a__1[0] = jobname;
#line 137 ""
    i__3[1] = 4, a__1[1] = ".pmx";
#line 137 ""
    s_cat(infileq, a__1, i__3, &c__2, (ftnlen)27);
#line 138 ""
    ioin__1.inerr = 0;
#line 138 ""
    ioin__1.infilen = 27;
#line 138 ""
    ioin__1.infile = infileq;
#line 138 ""
    ioin__1.inex = &fexist;
#line 138 ""
    ioin__1.inopen = 0;
#line 138 ""
    ioin__1.innum = 0;
#line 138 ""
    ioin__1.innamed = 0;
#line 138 ""
    ioin__1.inname = 0;
#line 138 ""
    ioin__1.inacc = 0;
#line 138 ""
    ioin__1.inseq = 0;
#line 138 ""
    ioin__1.indir = 0;
#line 138 ""
    ioin__1.infmt = 0;
#line 138 ""
    ioin__1.inform = 0;
#line 138 ""
    ioin__1.inunf = 0;
#line 138 ""
    ioin__1.inrecl = 0;
#line 138 ""
    ioin__1.innrec = 0;
#line 138 ""
    ioin__1.inblank = 0;
#line 138 ""
    f_inqu(&ioin__1);
#line 139 ""
    if (! fexist) {
#line 140 ""
	s_wsle(&io___17);
/* Writing concatenation */
#line 140 ""
	i__3[0] = 17, a__1[0] = "Cannot find file ";
#line 140 ""
	i__3[1] = 27, a__1[1] = infileq;
#line 140 ""
	s_cat(ch__1, a__1, i__3, &c__2, (ftnlen)44);
#line 140 ""
	do_lio(&c__9, &c__1, ch__1, (ftnlen)44);
#line 140 ""
	e_wsle();
#line 141 ""
	s_stop("", (ftnlen)0);
#line 142 ""
    }
#line 143 ""
    o__1.oerr = 0;
#line 143 ""
    o__1.ounit = 10;
#line 143 ""
    o__1.ofnmlen = ljob + 4;
/* Writing concatenation */
#line 143 ""
    i__3[0] = ljob, a__1[0] = jobname;
#line 143 ""
    i__3[1] = 4, a__1[1] = ".pmx";
#line 143 ""
    s_cat(ch__2, a__1, i__3, &c__2, (ftnlen)31);
#line 143 ""
    o__1.ofnm = ch__2;
#line 143 ""
    o__1.orl = 0;
#line 143 ""
    o__1.osta = 0;
#line 143 ""
    o__1.oacc = 0;
#line 143 ""
    o__1.ofm = 0;
#line 143 ""
    o__1.oblnk = 0;
#line 143 ""
    f_open(&o__1);

/* Open all instrument files now for allparts stuff.  Later disgard those >nv */

#line 147 ""
    for (iv = 1; iv <= 24; ++iv) {
#line 148 ""
	all_1.iorig[iv - 1] = iv;
#line 149 ""
	o__1.oerr = 0;
#line 149 ""
	o__1.ounit = iv + 10;
#line 149 ""
	o__1.ofnm = 0;
#line 149 ""
	o__1.orl = 0;
#line 149 ""
	o__1.osta = "SCRATCH";
#line 149 ""
	o__1.oacc = 0;
#line 149 ""
	o__1.ofm = 0;
#line 149 ""
	o__1.oblnk = 0;
#line 149 ""
	f_open(&o__1);
#line 150 ""
	ludpfn[iv - 1] = 0;
#line 151 ""
/* L19: */
#line 151 ""
    }
#line 152 ""
    s_rsfe(&io___20);
#line 152 ""
    do_fio(&c__1, line, (ftnlen)128);
#line 152 ""
    e_rsfe();
#line 153 ""
    chkcom_(line, &goto999, (ftnlen)128);
#line 154 ""
    if (s_cmp(line, "---", (ftnlen)3, (ftnlen)3) == 0) {
#line 155 ""
	allparts_(line, &c__128, (ftnlen)128);
#line 156 ""
L31:
#line 156 ""
	s_rsfe(&io___23);
#line 156 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 156 ""
	e_rsfe();
#line 157 ""
	if (s_cmp(line, "---", (ftnlen)3, (ftnlen)3) != 0) {
#line 158 ""
	    allparts_(line, &c__128, (ftnlen)128);
#line 159 ""
	    goto L31;
#line 160 ""
	}
#line 161 ""
	allparts_(line, &c__128, (ftnlen)128);
#line 162 ""
	s_rsfe(&io___24);
#line 162 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 162 ""
	e_rsfe();
#line 163 ""
	chkcom_(line, &goto999, (ftnlen)128);
#line 164 ""
    }
#line 165 ""
    iccount = 0;
#line 166 ""
    nv = readin_(line, &iccount, &c__1, (ftnlen)128) + .1f;
#line 167 ""
    all_1.noinst = readin_(line, &iccount, &c__2, (ftnlen)128) + .1f;
#line 168 ""
    if (all_1.noinst > 0) {
#line 169 ""
	all_1.nvi[0] = nv - all_1.noinst + 1;
#line 170 ""
    } else {
#line 171 ""
	all_1.noinst = 1 - all_1.noinst;
#line 172 ""
	i__1 = all_1.noinst;
#line 172 ""
	for (iinst = 1; iinst <= i__1; ++iinst) {
#line 173 ""
	    all_1.nvi[iinst - 1] = readin_(line, &iccount, &c_n1, (ftnlen)128)
		     + .1f;
#line 174 ""
/* L21: */
#line 174 ""
	}
#line 175 ""
    }
#line 176 ""
    all_1.noinow = all_1.noinst;
#line 177 ""
    insnow = 1;

/*  ivlast is last iv in current inst.  instnum(iv) is iinst for current voice. */

#line 181 ""
    ivlast = all_1.nvi[0];
#line 182 ""
    i__1 = nv;
#line 182 ""
    for (iv = 1; iv <= i__1; ++iv) {
#line 183 ""
	all_1.instnum[iv - 1] = insnow;
#line 184 ""
	if (iv == ivlast) {
#line 185 ""
	    if (iv < nv) {
#line 185 ""
		all_1.botv[iv] = TRUE_;
#line 185 ""
	    }

/*  The previous stmt will set botv true only for bot voice of iinst>1.  It is */
/*  used when writing termrpts, but the one in voice one is handled differently, */
/*  so botv(1) is left .false. */

#line 191 ""
	    if (insnow < all_1.noinst) {
#line 192 ""
		++insnow;
#line 193 ""
		ivlast += all_1.nvi[insnow - 1];
#line 194 ""
	    }
#line 195 ""
	}
#line 196 ""
/* L22: */
#line 196 ""
    }
#line 197 ""
    mtrnuml = readin_(line, &iccount, &c__0, (ftnlen)128) + .1f;
#line 198 ""
    mtrdenl = readin_(line, &iccount, &c__0, (ftnlen)128) + .1f;
#line 199 ""
    mtrnmp = readin_(line, &iccount, &c__0, (ftnlen)128) + .1f;
#line 200 ""
    mtrdnp = readin_(line, &iccount, &c__0, (ftnlen)128) + .1f;
#line 201 ""
    xmtrnum0 = readin_(line, &iccount, &c__0, (ftnlen)128);
#line 202 ""
    isig = readin_(line, &iccount, &c__0, (ftnlen)128) + .1f;
#line 203 ""
    npages = readin_(line, &iccount, &c__3, (ftnlen)128) + .1f;
#line 204 ""
    all_1.nsyst = readin_(line, &iccount, &c__0, (ftnlen)128) + .1f;
#line 205 ""
    musicsize = readin_(line, &iccount, &c__4, (ftnlen)128) + .1f;
#line 206 ""
    fracindent = readin_(line, &iccount, &c__5, (ftnlen)128);
#line 207 ""
    if (npages == 0) {
#line 208 ""
	s_wsle(&io___39);
#line 208 ""
	do_lio(&c__9, &c__1, "You entered npages=0, which means nsyst is not"\
		" the total number", (ftnlen)63);
#line 208 ""
	e_wsle();
#line 210 ""
	s_wsle(&io___40);
#line 210 ""
	do_lio(&c__9, &c__1, "of systems.  Scor2prt has to know the total nu"\
		"mber of systems.", (ftnlen)62);
#line 210 ""
	e_wsle();
#line 212 ""
	s_wsle(&io___41);
#line 212 ""
	do_lio(&c__9, &c__1, "Please set npages and nsyst to their real valu"\
		"es.", (ftnlen)49);
#line 212 ""
	e_wsle();
#line 214 ""
	s_stop("", (ftnlen)0);
#line 215 ""
    }

/*  Must leave insetup=.true. else could bypass ALL instrument names. */

#line 219 ""
    s_rsfe(&io___42);
#line 219 ""
    do_fio(&c__1, line, (ftnlen)128);
#line 219 ""
    e_rsfe();
#line 220 ""
    chkcom_(line, &goto999, (ftnlen)128);
#line 221 ""
    al__1.aerr = 0;
#line 221 ""
    al__1.aunit = 10;
#line 221 ""
    f_back(&al__1);

/*  Normally this puts pointer at start of line with 1st inst name */
/*  Check if prior line was "%%" */

#line 226 ""
    al__1.aerr = 0;
#line 226 ""
    al__1.aunit = 10;
#line 226 ""
    f_back(&al__1);
#line 227 ""
    s_rsfe(&io___43);
#line 227 ""
    do_fio(&c__1, line, (ftnlen)128);
#line 227 ""
    e_rsfe();
#line 228 ""
    if (s_cmp(line, "%%", (ftnlen)2, (ftnlen)2) == 0) {
#line 228 ""
	al__1.aerr = 0;
#line 228 ""
	al__1.aunit = 10;
#line 228 ""
	f_back(&al__1);
#line 228 ""
    }
#line 229 ""
    i__1 = all_1.noinst;
#line 229 ""
    for (iv = 1; iv <= i__1; ++iv) {
#line 230 ""
	gotname = FALSE_;
#line 231 ""
L16:
#line 231 ""
	s_rsfe(&io___45);
#line 231 ""
	do_fio(&c__1, instrum + (iv - 1 << 7), (ftnlen)128);
#line 231 ""
	e_rsfe();
#line 232 ""
	if (s_cmp(instrum + (iv - 1 << 7), "%%", (ftnlen)2, (ftnlen)2) == 0) {
#line 233 ""
	    s_rsfe(&io___47);
#line 233 ""
	    do_fio(&c__1, line, (ftnlen)128);
#line 233 ""
	    e_rsfe();
#line 234 ""
	    goto L16;
#line 235 ""
	} else if (*(unsigned char *)&instrum[(iv - 1) * 128] == '%') {
#line 236 ""
	    ivq = *(unsigned char *)&instrum[(iv - 1 << 7) + 1] - 48;
#line 237 ""
	    if (ivq != iv) {

/*  It's really a comment.  Copy to parts, then get another trial name. */

#line 241 ""
		allparts_(instrum + (iv - 1 << 7), &c__128, (ftnlen)128);
#line 242 ""
		goto L16;
#line 243 ""
	    } else {
#line 244 ""
		s_copy(line, instrum + ((iv - 1 << 7) + 2), (ftnlen)128, (
			ftnlen)126);
#line 245 ""
		s_copy(instrum + (iv - 1 << 7), line, (ftnlen)128, (ftnlen)
			128);
#line 246 ""
		gotname = TRUE_;
#line 247 ""
	    }
#line 248 ""
	} else {
#line 249 ""
	    gotname = TRUE_;
#line 250 ""
	}
/* c */
/* c  The following checks for macro that write original C-clef as part of */
/* c  instrument name.  See pmx.tex */
/* c */
/*        if (index(instrum(iv),'namewpc') .eq. 0) then */
/*          write(10+iv,'(a)')' ' */
/*        else */
/*          inm1 = index(instrum(iv),'{')+1 */
/*          inm2 = index(instrum(iv),'}')-1 */
/*          read(instrum(iv)(inm2+2:inm2+8),'(i1,4x,2i1)')ilev,iy1,iy2 */
/*          write(10+iv,'(a)')sq//'namewpc{}'//char(ilev+48)//'{20}'// */
/*     *      char(iy1+49)//char(iy2+49) */
/*          instrum(iv) = instrum(iv)(inm1:inm2) */
/*        end if */
#line 265 ""
	if (! gotname) {
#line 266 ""
	    s_wsle(&io___49);
#line 266 ""
	    do_lio(&c__9, &c__1, "You must provide a replacement instrument "\
		    "name", (ftnlen)46);
#line 266 ""
	    e_wsle();
#line 267 ""
	    s_stop("", (ftnlen)0);
#line 268 ""
	}
#line 269 ""
	io___50.ciunit = iv + 10;
#line 269 ""
	s_wsfe(&io___50);
#line 269 ""
	do_fio(&c__1, " ", (ftnlen)1);
#line 269 ""
	e_wsfe();
#line 270 ""
/* L14: */
#line 270 ""
    }
#line 271 ""
    all_1.replacing = FALSE_;
#line 272 ""
    all_1.nvnow = nv;

/*  Clef string:  Note insetup is still T, so "%%" will be treated specially */

#line 276 ""
    s_rsfe(&io___51);
#line 276 ""
    do_fio(&c__1, line, (ftnlen)128);
#line 276 ""
    e_rsfe();
#line 277 ""
    chkcom_(line, &goto999, (ftnlen)128);
#line 278 ""
    if (all_1.replacing) {

/*  If here, we have next line after "%%", containing score's clef string */
/*  Assume all clefs are handled with instrument comments. */

#line 283 ""
	s_rsfe(&io___52);
#line 283 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 283 ""
	e_rsfe();
#line 284 ""
	chkcom_(line, &goto999, (ftnlen)128);
#line 285 ""
	al__1.aerr = 0;
#line 285 ""
	al__1.aunit = 10;
#line 285 ""
	f_back(&al__1);
#line 286 ""
    } else {

/*  If here, line has the clef string in it.  Handle the old way */

#line 290 ""
	kvstart = 1;
#line 291 ""
	kvend = all_1.nvi[0];
#line 292 ""
	i__1 = all_1.noinst;
#line 292 ""
	for (inst = 1; inst <= i__1; ++inst) {
#line 293 ""
	    ci__1.cierr = 0;
#line 293 ""
	    ci__1.ciunit = inst + 10;
/* Writing concatenation */
#line 293 ""
	    i__4[0] = 2, a__2[0] = "(a";
#line 293 ""
	    *(unsigned char *)&ch__4[0] = all_1.nvi[inst - 1] + 48;
#line 293 ""
	    i__4[1] = 1, a__2[1] = ch__4;
#line 293 ""
	    i__4[2] = 1, a__2[2] = ")";
#line 293 ""
	    ci__1.cifmt = (s_cat(ch__3, a__2, i__4, &c__3, (ftnlen)4), ch__3);
#line 293 ""
	    s_wsfe(&ci__1);
#line 293 ""
	    do_fio(&c__1, line + (kvstart - 1), kvend - (kvstart - 1));
#line 293 ""
	    e_wsfe();
#line 295 ""
	    if (inst < all_1.noinst) {
#line 296 ""
		kvstart = kvend + 1;
#line 297 ""
		kvend = kvstart + all_1.nvi[inst] - 1;
#line 298 ""
	    }
#line 299 ""
/* L2: */
#line 299 ""
	}
#line 300 ""
    }
#line 301 ""
    all_1.replacing = FALSE_;
#line 302 ""
    all_1.insetup = FALSE_;

/*  *****NOTE*****This comment applies to stuff done earlier! */
/*  Before starting the big loop, copy initial instnum and staffnum stuff */
/*  into working values.  Latter may change if noinst changes.  Also make */
/*  list of current inst nums relative to original ones.  In addition to those */
/*  below, must redo instnum(iv) and botv(iv) when we change noinst. */

/*  Path string:  ASSUME THIS WILL NEVER BE ALTERED IN PARTS! */

#line 312 ""
L18:
#line 312 ""
    s_rsfe(&io___56);
#line 312 ""
    do_fio(&c__1, line, (ftnlen)128);
#line 312 ""
    e_rsfe();
#line 313 ""
    if (*(unsigned char *)line == '%') {
#line 314 ""
	allparts_(line, &c__128, (ftnlen)128);
#line 315 ""
	goto L18;
#line 316 ""
    }
#line 317 ""
    allparts_(line, &c__128, (ftnlen)128);

/*  Write instrument names.  Will be blank if later part of a score. */

#line 321 ""
    if (*(unsigned char *)&instrum[0] != ' ') {
#line 322 ""
	i__1 = all_1.noinst;
#line 322 ""
	for (iv = 1; iv <= i__1; ++iv) {
#line 323 ""
	    len = lenstr_(instrum + (iv - 1 << 7), &c__79, (ftnlen)128);
#line 324 ""
	    io___58.ciunit = iv + 10;
#line 324 ""
	    s_wsfe(&io___58);
#line 324 ""
	    do_fio(&c__1, "Ti", (ftnlen)2);
#line 324 ""
	    do_fio(&c__1, instrum + (iv - 1 << 7), len);
#line 324 ""
	    e_wsfe();
#line 325 ""
/* L3: */
#line 325 ""
	}
#line 326 ""
    }

/*  The big loop.  Except for '%%', put all comment lines in all parts. */
/*  Unless preceeded by '%%', put all type 2 or 3 TeX Strings in all parts */
/*  If a line starts with %!, put the rest of it in each part. */
/*  If a line starts with %[n], put the rest of it in part [n]. */
/*  Check for Tt, Tc, Voltas, Repeats, headers, lower texts, meter changes. */
/*     Assume they only come at top of block, except terminal repeat needs */
/*     special handling. */
/*  Check for "P"; ignore in parts. */
/*  Check for consecutive full-bar rests; if found, replace with rm[nn] */

#line 338 ""
    iv = 1;
#line 339 ""
    iinst = 1;
#line 340 ""
    termrpt = FALSE_;
#line 341 ""
L4:
#line 342 ""
    i__1 = s_rsfe(&io___60);
#line 342 ""
    if (i__1 != 0) {
#line 342 ""
	goto L999;
#line 342 ""
    }
#line 342 ""
    i__1 = do_fio(&c__1, line, (ftnlen)128);
#line 342 ""
    if (i__1 != 0) {
#line 342 ""
	goto L999;
#line 342 ""
    }
#line 342 ""
    i__1 = e_rsfe();
#line 342 ""
    if (i__1 != 0) {
#line 342 ""
	goto L999;
#line 342 ""
    }
#line 343 ""
    lenline = lenstr_(line, &c__128, (ftnlen)128);
#line 344 ""
    if (lenline == 0) {
#line 344 ""
	goto L4;
#line 344 ""
    }
#line 345 ""
    zapbl_(line, &c__128, (ftnlen)128);
#line 346 ""
    chkcom_(line, &goto999, (ftnlen)128);
#line 347 ""
    if (goto999) {
#line 347 ""
	goto L999;
#line 347 ""
    }
#line 348 ""
    lenline = lenstr_(line, &c__128, (ftnlen)128);
#line 349 ""
    if (lenline == 0) {
#line 349 ""
	goto L4;
#line 349 ""
    }
#line 350 ""
    if (*(unsigned char *)line == 'T') {
#line 351 ""
	allparts_(line, &c__128, (ftnlen)128);
#line 352 ""
	s_rsfe(&io___62);
#line 352 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 352 ""
	e_rsfe();
#line 353 ""
	allparts_(line, &c__128, (ftnlen)128);
#line 354 ""
	goto L4;
#line 355 ""
    } else /* if(complicated condition) */ {
/* Writing concatenation */
#line 355 ""
	i__3[0] = 1, a__1[0] = sq;
#line 355 ""
	i__3[1] = 1, a__1[1] = sq;
#line 355 ""
	s_cat(ch__5, a__1, i__3, &c__2, (ftnlen)2);
#line 355 ""
	if (s_cmp(line, ch__5, (ftnlen)2, (ftnlen)2) == 0) {
#line 356 ""
	    allparts_(line, &c__128, (ftnlen)128);
#line 357 ""
	    goto L4;
#line 358 ""
	} else if (i_indx("hl", line, (ftnlen)2, (ftnlen)1) > 0 && i_indx(
		" +-", line + 1, (ftnlen)3, (ftnlen)1) > 0) {
#line 360 ""
	    allparts_(line, &c__128, (ftnlen)128);
#line 361 ""
	    s_rsfe(&io___63);
#line 361 ""
	    do_fio(&c__1, line, (ftnlen)128);
#line 361 ""
	    e_rsfe();
#line 362 ""
	    allparts_(line, &c__128, (ftnlen)128);
#line 363 ""
	    goto L4;
#line 364 ""
	} else if (iv == 1) {
#line 365 ""
	    for (ia = 1; ia <= 10; ++ia) {
#line 366 ""
L24:
#line 367 ""
		idxa = ntindex_(line, achar + (ia - 1), (ftnlen)128, (ftnlen)
			1);
#line 368 ""
		isachar = idxa > 0;
#line 369 ""
		if (idxa > 1) {
#line 369 ""
		    i__1 = idxa - 2;
#line 369 ""
		    isachar = s_cmp(line + i__1, " ", idxa - 1 - i__1, (
			    ftnlen)1) == 0;
#line 369 ""
		}

/*                   1   2   3   4   5   6   7   8   9   10 */
/*      data achar /'P','m','V','R','A','h','w','K','M','I'/ */

#line 374 ""
		if (ia == 9) {
#line 374 ""
		    i__1 = idxa;
#line 374 ""
		    isachar = isachar && s_cmp(line + i__1, "S", idxa + 1 - 
			    i__1, (ftnlen)1) == 0;
#line 374 ""
		}
#line 376 ""
		if (isachar) {

/*  Check whether character is inside a quoted string by counting */
/*  how many quotes precede it in the line */

#line 382 ""
		    oddquotesbefore_(line, &idxa, &yesodd, (ftnlen)128);
#line 383 ""
		    if (yesodd) {
#line 383 ""
			isachar = FALSE_;
#line 383 ""
		    }
#line 384 ""
		}
#line 385 ""
		if (isachar) {

/*  Find next blank */

#line 389 ""
		    for (ib = idxa + 1; ib <= 128; ++ib) {
#line 390 ""
			if (*(unsigned char *)&line[ib - 1] == ' ') {
#line 390 ""
			    goto L7;
#line 390 ""
			}
#line 391 ""
/* L6: */
#line 391 ""
		    }
#line 392 ""
		    s_wsle(&io___69);
#line 392 ""
		    do_lio(&c__9, &c__1, "Problem with \"V,R,m,P,A,h,MS, o"\
			    "r w\"", (ftnlen)35);
#line 392 ""
		    e_wsle();
#line 393 ""
		    s_wsle(&io___70);
#line 393 ""
		    do_lio(&c__9, &c__1, "Send files to Dr. Don at dsimons a"\
			    "t roadrunner dot com", (ftnlen)54);
#line 393 ""
		    e_wsle();
#line 395 ""
		    s_stop("1", (ftnlen)1);
#line 396 ""
L7:

/*  Next blank is at position ib.  Later, if ia=1, must check for Pc"  " ; */
/*    i.e., look for '"' between P and blank */

#line 401 ""
		    if (ia == 4) {

/*  Check for terminal repeat.  Note if there's a term rpt, there can't be any */
/*  others.  Also, must process repeats LAST, after m's and 'V's */

#line 406 ""
			for (ic = ib + 1; ic <= 128; ++ic) {

/*  If any subsequent character on this line is neither blank nor "/", get out */

#line 410 ""
			    if (i_indx(" /", line + (ic - 1), (ftnlen)2, (
				    ftnlen)1) == 0) {
#line 410 ""
				goto L9;
#line 410 ""
			    }
#line 411 ""
			    if (*(unsigned char *)&line[ic - 1] == '/') {
#line 412 ""
				termrpt = TRUE_;
#line 413 ""
				i__1 = ib - 3;
#line 413 ""
				s_copy(termsym, line + i__1, (ftnlen)2, ib - 
					1 - i__1);

/*  Process the line as if there were no "R" */

#line 417 ""
				goto L10;
#line 418 ""
			    }
#line 419 ""
/* L8: */
#line 419 ""
			}

/* +++ 060812 */
/*  If here, all chars after "R" symbol are blanks, so process the line normally, */
/*    but only IF next line is not the M-Tx line " /" */

#line 425 ""
			s_rsfe(&io___73);
#line 425 ""
			do_fio(&c__1, templine, (ftnlen)128);
#line 425 ""
			e_rsfe();
#line 426 ""
			if (s_cmp(templine, " /", (ftnlen)2, (ftnlen)2) != 0) 
				{
#line 427 ""
			    al__1.aerr = 0;
#line 427 ""
			    al__1.aunit = 10;
#line 427 ""
			    f_back(&al__1);
/* and flow out */
#line 428 ""
			} else {

/*  We have the M-Tx case where one line ends with R? and next is " /". Add / to the line, */
/*    and proceed as above */

/* Writing concatenation */
#line 433 ""
			    i__3[0] = ib, a__1[0] = line;
#line 433 ""
			    i__3[1] = 1, a__1[1] = "/";
#line 433 ""
			    s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 434 ""
			    lenline += 2;
#line 435 ""
			    termrpt = TRUE_;
#line 436 ""
			    i__1 = ib - 3;
#line 436 ""
			    s_copy(termsym, line + i__1, (ftnlen)2, ib - 1 - 
				    i__1);
#line 437 ""
			    goto L10;
#line 438 ""
			}
/* +++ 060812 */

#line 441 ""
		    } else if (ia == 1) {
#line 442 ""
			idxq = ntindex_(line, "\"", (ftnlen)128, (ftnlen)1);
#line 443 ""
			if (idxq > idxa && idxq < ib) {

/*  Quote is between P and next blank.  Find 2nd quote, starting at the blank. */

#line 447 ""
			    idxq2 = ib - 1 + ntindex_(line, "\"", (ftnlen)128,
				     (ftnlen)1);
#line 448 ""
			    i__1 = idxq2;
#line 448 ""
			    if (idxq == 0 || s_cmp(line + i__1, " ", idxq2 + 
				    1 - i__1, (ftnlen)1) != 0) {
#line 449 ""
				s_wsle(&io___77);
#line 449 ""
				e_wsle();
#line 450 ""
				s_wsle(&io___78);
#line 450 ""
				do_lio(&c__9, &c__1, "Error copying P with q"\
					"uotes, idxq2:", (ftnlen)35);
#line 450 ""
				do_lio(&c__3, &c__1, (char *)&idxq2, (ftnlen)
					sizeof(integer));
#line 450 ""
				e_wsle();
#line 451 ""
				s_wsle(&io___79);
#line 451 ""
				do_lio(&c__9, &c__1, line, (ftnlen)60);
#line 451 ""
				e_wsle();
#line 452 ""
				s_stop("1", (ftnlen)1);
#line 453 ""
			    }
#line 454 ""
			    ib = idxq2 + 1;
#line 455 ""
			}

/*  Do not transfer P into parts. */

#line 459 ""
			goto L12;
#line 460 ""
		    } else if (ia == 9) {

/*  Start Saving a macro. After leaving here, a symbol will be sent to all parts, */
/*  If all on this line, set ib to end and exit normally. */

#line 465 ""
			i__1 = ib;
#line 465 ""
			ndxm = i_indx(line + i__1, "M", 128 - i__1, (ftnlen)1)
				;
#line 466 ""
			i__1 = ib + ndxm - 2;
#line 466 ""
			if (ndxm > 0 && s_cmp(line + i__1, " ", ib + ndxm - 1 
				- i__1, (ftnlen)1) == 0) {

/*  Macro ends on this line */

#line 470 ""
			    ib = ib + ndxm + 1;
#line 471 ""
			} else {

/*  Save leading part of current line */

#line 475 ""
			    lenhold = idxa - 1;
#line 476 ""
			    if (lenhold > 0) {
#line 476 ""
				s_copy(holdln, line, (ftnlen)128, lenhold);
#line 476 ""
			    }

/*  Transfer rest of line */

#line 480 ""
			    i__1 = 129 - idxa;
#line 480 ""
			    allparts_(line + (idxa - 1), &i__1, 128 - (idxa - 
				    1));

/*  Read next line */

#line 484 ""
L20:
#line 484 ""
			    s_rsfe(&io___82);
#line 484 ""
			    do_fio(&c__1, line, (ftnlen)128);
#line 484 ""
			    e_rsfe();

/*  Check for comment, transfer and loop if so */

/*                if (line(1:1) .eq.'%') then */
#line 489 ""
L23:
#line 489 ""
			    if (*(unsigned char *)line == '%') {
/*                  call allparts(line,128) */
/*                  go to 20 */
#line 492 ""
				chkcom_(line, &goto999, (ftnlen)128);
#line 493 ""
				goto L23;
#line 494 ""
			    }

/*  Look for terminal ' M' */

#line 498 ""
			    if (*(unsigned char *)line == 'M') {
#line 499 ""
				ndxm = 1;
#line 500 ""
			    } else {
#line 501 ""
				ndxm = i_indx(line, " M", (ftnlen)128, (
					ftnlen)2);
#line 502 ""
				if (ndxm > 0) {
#line 502 ""
				    ++ndxm;
#line 502 ""
				}
#line 503 ""
			    }
#line 504 ""
			    if (ndxm > 0) {

/*  Set parameters, exit normally (but later check for leading part of 1st line */

#line 508 ""
				idxa = 1;
#line 509 ""
				ib = ndxm + 1;
#line 510 ""
			    } else {

/*  No "M", transfer entire line, loop */

#line 514 ""
				allparts_(line, &c__128, (ftnlen)128);
#line 515 ""
				goto L20;
#line 516 ""
			    }
#line 517 ""
			}
#line 518 ""
		    } else if (ia == 10) {

/*  Do not transfer MIDI command into parts */

#line 522 ""
			goto L12;
#line 523 ""
		    } else if (ia == 5) {

/*  First check for "AS", but S may come after other "A" options */

#line 527 ""
			i__1 = idxa;
#line 527 ""
			idxs = i_indx(line + i__1, "S", ib - i__1, (ftnlen)1);
#line 528 ""
			if (idxs > 0) {

/*  Get rid of the string. First check if that's all there is in A. */

#line 532 ""
			    if (ib - idxa == nv + 2) {
#line 532 ""
				goto L12;
#line 532 ""
			    }
#line 533 ""
			    i__1 = idxa + idxs + nv;
/* Writing concatenation */
#line 533 ""
			    i__3[0] = idxa + idxs - 1, a__1[0] = line;
#line 533 ""
			    i__3[1] = ib - i__1, a__1[1] = line + i__1;
#line 533 ""
			    s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 534 ""
			}

/*  Check for user-defined part file name. Must start in column 1 and have */
/*    AN[n]"userfilename". */
/*    idxa is position of "A" */
/*    ib is position of the next blank after "A" */
/*    Don't allow any blanks in user */

#line 542 ""
			i__1 = idxa;
#line 542 ""
			if (s_cmp(line + i__1, "N", idxa + 1 - i__1, (ftnlen)
				1) != 0) {
#line 542 ""
			    goto L9;
#line 542 ""
			}
/* bail out */
#line 543 ""
			if (idxa != 1) {
#line 544 ""
			    s_wsle(&io___84);
#line 544 ""
			    e_wsle();
#line 545 ""
			    s_wsle(&io___85);
#line 545 ""
			    do_lio(&c__9, &c__1, "You entered \"AN...\" some"\
				    "where beyond first column; stopping.", (
				    ftnlen)60);
#line 545 ""
			    e_wsle();
#line 547 ""
			    s_stop("1", (ftnlen)1);
#line 548 ""
			}

/*  pmxa already checked for valid one- or 2-digit number, so get it */

#line 552 ""
			if (*(unsigned char *)&line[3] == '"') {

/*  Single digit instrument number */

#line 556 ""
			    s_rsfi(&io___86);
#line 556 ""
			    do_fio(&c__1, (char *)&iudpfn, (ftnlen)sizeof(
				    integer));
#line 556 ""
			    e_rsfi();
#line 557 ""
			    idxstartname = 5;
#line 558 ""
			} else {
#line 559 ""
			    s_rsfi(&io___89);
#line 559 ""
			    do_fio(&c__1, (char *)&iudpfn, (ftnlen)sizeof(
				    integer));
#line 559 ""
			    e_rsfi();
#line 560 ""
			    idxstartname = 6;
#line 561 ""
			}
#line 562 ""
			ludpfn[iudpfn - 1] = i_indx(line + (idxstartname - 1),
				 "\"", 128 - (idxstartname - 1), (ftnlen)1) - 
				1;
#line 563 ""
			if (ludpfn[iudpfn - 1] < 0) {
#line 564 ""
			    s_wsle(&io___90);
#line 564 ""
			    e_wsle();
#line 565 ""
			    s_wsle(&io___91);
#line 565 ""
			    do_lio(&c__9, &c__1, "User-defined part file nam"\
				    "e must be in quotes", (ftnlen)45);
#line 565 ""
			    e_wsle();
#line 566 ""
			    s_stop("1", (ftnlen)1);
#line 567 ""
			}
#line 568 ""
			idxendname = idxstartname + ludpfn[iudpfn - 1] - 1;
#line 569 ""
			s_copy(udpfnq + (iudpfn - 1 << 7), line + (
				idxstartname - 1), (ftnlen)128, idxendname - (
				idxstartname - 1));

/*  Get a new line! */

#line 573 ""
			goto L4;
#line 574 ""
		    } else if (ia == 8) {

/* Key change/transposition. */
/* If not instrument specific, copy to all parts */

#line 579 ""
			i__1 = idxa;
#line 579 ""
			if (s_cmp(line + i__1, "i", idxa + 1 - i__1, (ftnlen)
				1) != 0) {
#line 579 ""
			    goto L9;
#line 579 ""
			}

/* Instrument-wise key/transposition(s): Ki[nn][+/-][dd}[+/-][dd]... */

#line 583 ""
			ibb = idxa + 2;
/* Starts on digit after 'i' */
#line 584 ""
L40:
#line 585 ""
			ici__1.icierr = 0;
#line 585 ""
			ici__1.iciend = 0;
#line 585 ""
			ici__1.icirnum = 1;
#line 585 ""
			ici__1.icirlen = 1;
#line 585 ""
			ici__1.iciunit = line + (ibb - 1);
#line 585 ""
			ici__1.icifmt = "(i1)";
#line 585 ""
			s_rsfi(&ici__1);
#line 585 ""
			do_fio(&c__1, (char *)&iiinst, (ftnlen)sizeof(integer)
				);
#line 585 ""
			e_rsfi();
/* 1st digit of iinst */
#line 586 ""
			i__1 = ibb;
#line 586 ""
			itemp = i_indx("01234567890", line + i__1, (ftnlen)11,
				 ibb + 1 - i__1);
#line 587 ""
			if (itemp > 0) {
#line 588 ""
			    ++ibb;
#line 589 ""
			    iiinst = iiinst * 10 + itemp - 1;
#line 590 ""
			}
#line 591 ""
			++ibb;
/* now at first +/-. Need end of 2nd number */
#line 592 ""
			i__1 = ibb;
#line 592 ""
			itemp = i_indx(line + i__1, "i", ib - i__1, (ftnlen)1)
				;
/* Rel pos'n of next 'i' */
#line 593 ""
			if (itemp > 0) {
#line 594 ""
			    iend = ibb + itemp - 1;
#line 595 ""
			} else {
#line 596 ""
			    iend = ib - 1;
#line 597 ""
			}
#line 598 ""
			io___98.ciunit = all_1.iorig[iiinst - 1] + 10;
#line 598 ""
			s_wsfe(&io___98);
/* Writing concatenation */
#line 598 ""
			i__3[0] = 3, a__1[0] = "Ki1";
#line 598 ""
			i__3[1] = iend - (ibb - 1), a__1[1] = line + (ibb - 1)
				;
#line 598 ""
			s_cat(ch__6, a__1, i__3, &c__2, (ftnlen)131);
#line 598 ""
			do_fio(&c__1, ch__6, iend - (ibb - 1) + 3);
#line 598 ""
			e_wsfe();
#line 599 ""
			if (itemp > 0) {
#line 600 ""
			    ibb = iend + 2;
#line 601 ""
			    goto L40;
#line 602 ""
			}
#line 603 ""
			goto L12;
/* Remove K command from string, go to next ia */
#line 604 ""
		    }
#line 605 ""
L9:
#line 606 ""
		    i__1 = ib - idxa;
#line 606 ""
		    allparts_(line + (idxa - 1), &i__1, ib - 1 - (idxa - 1));
#line 607 ""
L12:

/*  Remove the string from line */

#line 611 ""
		    if (idxa == 1) {
#line 612 ""
			s_copy(line, line + (ib - 1), (ftnlen)128, 128 - (ib 
				- 1));
#line 613 ""
		    } else {
/* Writing concatenation */
#line 614 ""
			i__3[0] = idxa - 1, a__1[0] = line;
#line 614 ""
			i__3[1] = 128 - (ib - 1), a__1[1] = line + (ib - 1);
#line 614 ""
			s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 615 ""
		    }
#line 616 ""
		    lenline = lenstr_(line, &c__128, (ftnlen)128);

/*  Loop if only blanks are left */

#line 620 ""
		    if (lenline == 0) {
#line 620 ""
			goto L4;
#line 620 ""
		    }

/*  Must check for multiple "I" commands, so go to just after start of ia loop */

#line 624 ""
		    if (ia == 10) {
#line 624 ""
			goto L24;
#line 624 ""
		    }

/*  Tack on front part from 1st line of saved macro */

#line 628 ""
		    if (lenhold > 0) {
/* Writing concatenation */
#line 629 ""
			i__4[0] = lenhold, a__2[0] = holdln;
#line 629 ""
			i__4[1] = 1, a__2[1] = " ";
#line 629 ""
			i__4[2] = lenline, a__2[2] = line;
#line 629 ""
			s_cat(line, a__2, i__4, &c__3, (ftnlen)128);
#line 630 ""
			lenhold = 0;
#line 631 ""
		    }
#line 632 ""
		}
#line 633 ""
/* L5: */
#line 633 ""
	    }
#line 634 ""
	}
#line 634 ""
    }

/*  Now a special loop to deal with 'X'.  If it was %[n]X..., will have been */
/*  copied into part [n] already.  If no "B" or "P", remove.  If "P", just */
/*  remove the "P" so pmxa/b will process.  If "B". do nothing. */

#line 640 ""
L10:
#line 641 ""
    nchk = 1;
#line 642 ""
L13:
#line 642 ""
    ntinx = nchk - 1 + ntindex_(line + (nchk - 1), "X", 128 - (nchk - 1), (
	    ftnlen)1);
#line 643 ""
    if (ntinx > nchk - 1) {

/*  There is a non-TeX 'X' at ntinx.  Loop if neither 1st nor after a blank. */

#line 647 ""
	if (ntinx > 1) {
#line 648 ""
	    i__1 = ntinx - 2;
#line 648 ""
	    if (s_cmp(line + i__1, " ", ntinx - 1 - i__1, (ftnlen)1) != 0) {

/*  The X is not 1st char of PMX command.  Advance starting point, loop. */

#line 652 ""
		nchk = ntinx + 1;
#line 653 ""
		goto L13;
#line 654 ""
	    }
#line 655 ""
	}

/*  We now know the X at ntinx starts a PMX command.  Find next blank */

#line 659 ""
	i__1 = ntinx;
#line 659 ""
	ib = ntinx + i_indx(line + i__1, " ", 128 - i__1, (ftnlen)1);

/*  There must be a blank to right of "X", so ib>ntinx */

/*        locp = nchk-1+index(line(nchk:ib),'P') */
#line 664 ""
	i__1 = ntinx;
#line 664 ""
	locp = ntinx + i_indx(line + i__1, "P", ib - i__1, (ftnlen)1);

/*  Did not need to use ntindex because we already know bounds of PMX command. */

/*        if (locp .gt. nchk-1) then */
#line 669 ""
	if (locp > ntinx) {

/*  Strip out the 'P' */

#line 673 ""
	    s_copy(templine, line, (ftnlen)128, locp - 1);
#line 674 ""
	    i__1 = locp;
/* Writing concatenation */
#line 674 ""
	    i__3[0] = locp - 1, a__1[0] = templine;
#line 674 ""
	    i__3[1] = lenline - i__1, a__1[1] = line + i__1;
#line 674 ""
	    s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 675 ""
	    --lenline;
#line 676 ""
	    --ib;
#line 677 ""
	}
#line 678 ""
	if (i_indx(line + (ntinx - 1), ":", ib - (ntinx - 1), (ftnlen)1) > 0 
		|| i_indx(line + (ntinx - 1), "S", ib - (ntinx - 1), (ftnlen)
		1) > 0 || i_indx(line + (ntinx - 1), "B", ib - (ntinx - 1), (
		ftnlen)1) > 0 || locp > ntinx) {

/*  The X command is a shift, "Both", or "Part".  Do not remove. */

#line 684 ""
	    nchk = ib + 1;
#line 685 ""
	    goto L13;
#line 686 ""
	}

/*  Remove the X command. */

#line 690 ""
	if (ntinx == 1) {
#line 691 ""
	    if (ib < lenline) {
#line 692 ""
		i__1 = ib;
#line 692 ""
		s_copy(line, line + i__1, (ftnlen)128, lenline - i__1);
#line 693 ""
	    } else {

/*  line contains ONLY the "X" command, so get a new line */

#line 697 ""
		goto L4;
#line 698 ""
	    }
#line 699 ""
	} else {
#line 700 ""
	    s_copy(templine, line, (ftnlen)128, ntinx - 1);
#line 701 ""
	    if (ib < lenline) {
#line 702 ""
		i__1 = ib;
/* Writing concatenation */
#line 702 ""
		i__3[0] = ntinx - 1, a__1[0] = templine;
#line 702 ""
		i__3[1] = lenline - i__1, a__1[1] = line + i__1;
#line 702 ""
		s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 703 ""
	    } else {
#line 704 ""
		s_copy(line, templine, (ftnlen)128, ntinx - 1);
#line 705 ""
	    }
#line 706 ""
	}

/*  Recompute lenline */

#line 710 ""
	lenline = lenstr_(line, &c__128, (ftnlen)128);

/*  Resume checking after location of removed command. */

#line 714 ""
	nchk = ntinx;
#line 715 ""
	goto L13;
#line 716 ""
    }

/*  End of loop for X-checks */

#line 720 ""
    oneof2 = ntindex_(line, "//", (ftnlen)128, (ftnlen)2) > 0;
#line 721 ""
    if (termrpt && all_1.botv[iv - 1] && frstln && *(unsigned char *)&line[
	    lenline - 1] == '/') {

/*  Must add a terminal repeat before the slash */

#line 726 ""
	if (oneof2) {
#line 726 ""
	    --lenline;
#line 726 ""
	}
#line 727 ""
	if (lenline > 1) {
#line 727 ""
	    io___103.ciunit = all_1.iorig[iinst - 1] + 10;
#line 727 ""
	    s_wsfe(&io___103);
#line 727 ""
	    do_fio(&c__1, line, lenline - 1);
#line 727 ""
	    e_wsfe();
#line 727 ""
	}
#line 728 ""
	if (! oneof2) {
/* Writing concatenation */
#line 729 ""
	    i__3[0] = 2, a__1[0] = termsym;
#line 729 ""
	    i__3[1] = 2, a__1[1] = " /";
#line 729 ""
	    s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 730 ""
	    lenline = 4;
#line 731 ""
	} else {
/* Writing concatenation */
#line 732 ""
	    i__3[0] = 2, a__1[0] = termsym;
#line 732 ""
	    i__3[1] = 3, a__1[1] = " //";
#line 732 ""
	    s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 733 ""
	    lenline = 5;
#line 734 ""
	}
#line 735 ""
    }
#line 736 ""
    if (termrpt && frstln && *(unsigned char *)&line[lenline - 1] == '/' && 
	    iv == all_1.nvnow) {
#line 736 ""
	termrpt = FALSE_;
#line 736 ""
    }
#line 738 ""
    io___104.ciunit = all_1.iorig[iinst - 1] + 10;
#line 738 ""
    s_wsfe(&io___104);
#line 738 ""
    do_fio(&c__1, line, lenline);
#line 738 ""
    e_wsfe();
#line 739 ""
    if (oneof2) {
#line 740 ""
	frstln = FALSE_;
#line 741 ""
    } else if (! frstln) {
#line 742 ""
	frstln = TRUE_;
#line 743 ""
    }
/*      if (ntindex(line,'/').gt.0 .and. index(line,'//').eq.0) then */
#line 745 ""
    if (ntindex_(line, "/", (ftnlen)128, (ftnlen)1) > 0 && ntindex_(line, 
	    "//", (ftnlen)128, (ftnlen)2) == 0) {
#line 746 ""
	iv = iv % all_1.nvnow + 1;
#line 747 ""
	iinst = all_1.instnum[iv - 1];
#line 748 ""
    }
#line 749 ""
    goto L4;
#line 750 ""
L999:
#line 751 ""
    cl__1.cerr = 0;
#line 751 ""
    cl__1.cunit = 10;
#line 751 ""
    cl__1.csta = 0;
#line 751 ""
    f_clos(&cl__1);

/*  In the mbrest checks, must run through ALL noinst files (not just noinow) */

#line 755 ""
    i__1 = all_1.noinst;
#line 755 ""
    for (iinst = 1; iinst <= i__1; ++iinst) {
/* cc+++ */
/* c */
/* c  Temporarily transfer entire scratch file to real file */
/* c */
/*        rewind(10+iinst) */
/*        open(40,file='s2pout'//char(48+iinst)//'.pmx') */
/*        do 50 m = 1 , 10000 */
/*          read(10+iinst,'(a)',end=51)line */
/*          lenline = lenstr(line,128) */
/*          if (lenline .ge. 1) then */
/*            write(40,'(a)')line(1:lenline) */
/*          else */
/*            write(40,'(a)')' ' */
/*          end if */
/* 50      continue */
/* 51      continue */
/*        close(40) */
/* cc+++ */
#line 774 ""
	if (all_1.nvi[iinst - 1] == 1) {
#line 775 ""
	    mbrests_(&iinst, jobname, &ljob, &ludpfn[iinst - 1], udpfnq + (
		    iinst - 1 << 7), (ftnlen)27, (ftnlen)128);
#line 776 ""
	} else {

/*  Send a signal with ljob to bypass most mbrest processing */

#line 780 ""
	    i__2 = -ljob;
#line 780 ""
	    mbrests_(&iinst, jobname, &i__2, &ludpfn[iinst - 1], udpfnq + (
		    iinst - 1 << 7), (ftnlen)27, (ftnlen)128);
#line 781 ""
	}
#line 782 ""
/* L11: */
#line 782 ""
    }
#line 783 ""
    return 0;
} /* MAIN__ */

integer lenstr_(char *string, integer *n, ftnlen string_len)
{
    /* System generated locals */
    integer ret_val;

#line 786 ""
    for (ret_val = *n; ret_val >= 1; --ret_val) {
#line 787 ""
	if (*(unsigned char *)&string[ret_val - 1] != ' ') {
#line 787 ""
	    return ret_val;
#line 787 ""
	}
#line 788 ""
/* L1: */
#line 788 ""
    }
#line 789 ""
    ret_val = 0;
#line 790 ""
    return ret_val;
} /* lenstr_ */

/* Subroutine */ int allparts_(char *string, integer *n, ftnlen string_len)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    /* Subroutine */ int s_copy(char *, char *, ftnlen, ftnlen);
    integer s_wsfe(cilist *), do_fio(integer *, char *, ftnlen), e_wsfe(void);

    /* Local variables */
    static integer len, iinst;
    extern integer lenstr_(char *, integer *, ftnlen);

    /* Fortran I/O blocks */
    static cilist io___107 = { 0, 0, 0, "(a)", 0 };


#line 798 ""
    len = lenstr_(string, n, string_len);
#line 799 ""
    if (len == 0) {
#line 800 ""
	len = 1;
#line 801 ""
	s_copy(string, " ", string_len, (ftnlen)1);
#line 802 ""
    }
#line 803 ""
    i__1 = all_1.noinow;
#line 803 ""
    for (iinst = 1; iinst <= i__1; ++iinst) {
#line 804 ""
	io___107.ciunit = all_1.iorig[iinst - 1] + 10;
#line 804 ""
	s_wsfe(&io___107);
#line 804 ""
	do_fio(&c__1, string, len);
#line 804 ""
	e_wsfe();
#line 805 ""
/* L1: */
#line 805 ""
    }
#line 806 ""
    return 0;
} /* allparts_ */

/* Subroutine */ int mbrests_(integer *iv, char *jobname, integer *ljob, 
	integer *ludpfn, char *udpfnq, ftnlen jobname_len, ftnlen udpfnq_len)
{
    /* System generated locals */
    address a__1[2], a__2[3];
    integer i__1[2], i__2[3], i__3, i__4;
    real r__1;
    char ch__1[1], ch__2[18], ch__3[132], ch__4[4], ch__5[2], ch__6[10];
    cilist ci__1;
    icilist ici__1;
    olist o__1;
    cllist cl__1;
    alist al__1;

    /* Builtin functions */
    integer f_rew(alist *);
    /* Subroutine */ int s_cat(char *, char **, integer *, integer *, ftnlen);
    integer f_open(olist *), s_rsfe(cilist *), do_fio(integer *, char *, 
	    ftnlen), e_rsfe(void), s_cmp(char *, char *, ftnlen, ftnlen), 
	    s_wsfe(cilist *), e_wsfe(void), s_wsle(cilist *), do_lio(integer *
	    , integer *, char *, ftnlen), e_wsle(void);
    /* Subroutine */ int s_stop(char *, ftnlen);
    integer i_indx(char *, char *, ftnlen, ftnlen), s_rsfi(icilist *), e_rsfi(
	    void);
    /* Subroutine */ int s_copy(char *, char *, ftnlen, ftnlen);
    double r_lg10(real *);
    integer f_clos(cllist *);

    /* Local variables */
    static integer i__, il, iw;
    static char sq[1];
    static integer ip1, ipc, ipe, len, idx;
    static char sym[80];
    static integer ndig;
    static char line[128*50];
    static integer idxb, nmbr, idxs, lsym;
    static char line1[128];
    static logical type4;
    static integer icden, iline, lpart;
    static char partq[2];
    static integer lwbrs, nwbrs, lenbar, ipenew, mtrden;
    static logical wbrest;
    extern integer lenstr_(char *, integer *, ftnlen);
    static logical newmtr;
    static integer lwbrsx, mtrnum;
    static char wbrsym[3*2];
    static integer lenbeat;
    static logical alldone;
    extern integer ifnodur_(integer *, char *, ftnlen), ntindex_(char *, char 
	    *, ftnlen, ftnlen);
    static integer lenmult;
    extern /* Subroutine */ int dosetup_(integer *, char *, integer *, 
	    integer *, ftnlen);
    static logical rpfirst;
    extern /* Subroutine */ int fwbrsym_(integer *, integer *, char *, 
	    integer *, ftnlen), nextsym_(char *, integer *, integer *, 
	    integer *, char *, integer *, ftnlen, ftnlen);

    /* Fortran I/O blocks */
    static cilist io___114 = { 0, 0, 0, "(a)", 0 };
    static cilist io___117 = { 0, 40, 0, "(a)", 0 };
    static cilist io___118 = { 0, 40, 0, "(a)", 0 };
    static cilist io___119 = { 0, 6, 0, 0, 0 };
    static cilist io___122 = { 0, 0, 1, "(a)", 0 };
    static cilist io___123 = { 0, 40, 0, "(a)", 0 };
    static cilist io___124 = { 0, 0, 0, "(a)", 0 };
    static cilist io___125 = { 0, 40, 0, "(a)", 0 };
    static cilist io___145 = { 0, 0, 1, "(a)", 0 };
    static cilist io___146 = { 0, 40, 0, "(a)", 0 };
    static cilist io___147 = { 0, 40, 0, "(a)", 0 };
    static cilist io___148 = { 0, 6, 0, 0, 0 };
    static cilist io___153 = { 0, 40, 0, "(a)", 0 };
    static cilist io___155 = { 0, 6, 0, 0, 0 };
    static cilist io___157 = { 0, 40, 0, "(a)", 0 };
    static cilist io___159 = { 0, 40, 0, "(a)", 0 };
    static cilist io___160 = { 0, 40, 0, "(a)", 0 };
    static cilist io___161 = { 0, 40, 0, "(a)", 0 };
    static cilist io___162 = { 0, 40, 0, "(a)", 0 };


#line 816 ""
    type4 = FALSE_;
#line 817 ""
    *(unsigned char *)sq = '\\';
#line 818 ""
    alldone = FALSE_;
#line 819 ""
    al__1.aerr = 0;
#line 819 ""
    al__1.aunit = *iv + 10;
#line 819 ""
    f_rew(&al__1);
#line 820 ""
    if (*iv < 10) {
#line 821 ""
	*(unsigned char *)partq = (char) (*iv + 48);
#line 822 ""
	lpart = 1;
#line 823 ""
    } else {
#line 824 ""
	lpart = 2;
#line 825 ""
	if (*iv < 20) {
/* Writing concatenation */
#line 826 ""
	    i__1[0] = 1, a__1[0] = "1";
#line 826 ""
	    *(unsigned char *)&ch__1[0] = *iv + 38;
#line 826 ""
	    i__1[1] = 1, a__1[1] = ch__1;
#line 826 ""
	    s_cat(partq, a__1, i__1, &c__2, (ftnlen)2);
#line 827 ""
	} else {
/* Writing concatenation */
#line 828 ""
	    i__1[0] = 1, a__1[0] = "2";
#line 828 ""
	    *(unsigned char *)&ch__1[0] = *iv + 28;
#line 828 ""
	    i__1[1] = 1, a__1[1] = ch__1;
#line 828 ""
	    s_cat(partq, a__1, i__1, &c__2, (ftnlen)2);
#line 829 ""
	}
#line 830 ""
    }
/* 130327 */
/*      open(40,file=jobname(1:abs(ljob))//partq(1:lpart)//'.pmx') */
#line 833 ""
    if (*ludpfn == 0) {
#line 834 ""
	o__1.oerr = 0;
#line 834 ""
	o__1.ounit = 40;
#line 834 ""
	o__1.ofnmlen = abs(*ljob) + lpart + 4;
/* Writing concatenation */
#line 834 ""
	i__2[0] = abs(*ljob), a__2[0] = jobname;
#line 834 ""
	i__2[1] = lpart, a__2[1] = partq;
#line 834 ""
	i__2[2] = 4, a__2[2] = ".pmx";
#line 834 ""
	s_cat(ch__2, a__2, i__2, &c__3, (ftnlen)18);
#line 834 ""
	o__1.ofnm = ch__2;
#line 834 ""
	o__1.orl = 0;
#line 834 ""
	o__1.osta = 0;
#line 834 ""
	o__1.oacc = 0;
#line 834 ""
	o__1.ofm = 0;
#line 834 ""
	o__1.oblnk = 0;
#line 834 ""
	f_open(&o__1);
#line 835 ""
    } else {
#line 836 ""
	o__1.oerr = 0;
#line 836 ""
	o__1.ounit = 40;
#line 836 ""
	o__1.ofnmlen = *ludpfn + 4;
/* Writing concatenation */
#line 836 ""
	i__1[0] = *ludpfn, a__1[0] = udpfnq;
#line 836 ""
	i__1[1] = 4, a__1[1] = ".pmx";
#line 836 ""
	s_cat(ch__3, a__1, i__1, &c__2, (ftnlen)132);
#line 836 ""
	o__1.ofnm = ch__3;
#line 836 ""
	o__1.orl = 0;
#line 836 ""
	o__1.osta = 0;
#line 836 ""
	o__1.oacc = 0;
#line 836 ""
	o__1.ofm = 0;
#line 836 ""
	o__1.oblnk = 0;
#line 836 ""
	f_open(&o__1);
#line 837 ""
    }
#line 838 ""
    for (i__ = 1; i__ <= 10000; ++i__) {
#line 839 ""
	io___114.ciunit = *iv + 10;
#line 839 ""
	s_rsfe(&io___114);
#line 839 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 839 ""
	e_rsfe();
#line 840 ""
	if (*(unsigned char *)&line[0] == '%' || s_cmp(line, "---", (ftnlen)3,
		 (ftnlen)3) == 0 || type4) {
#line 842 ""
	    len = lenstr_(line, &c__128, (ftnlen)128);
#line 843 ""
	    if (len > 0) {
#line 844 ""
		s_wsfe(&io___117);
#line 844 ""
		do_fio(&c__1, line, len);
#line 844 ""
		e_wsfe();
#line 845 ""
	    } else {
#line 846 ""
		s_wsfe(&io___118);
#line 846 ""
		do_fio(&c__1, " ", (ftnlen)1);
#line 846 ""
		e_wsfe();
#line 847 ""
	    }
#line 848 ""
	    if (s_cmp(line, "---", (ftnlen)3, (ftnlen)3) == 0) {
#line 848 ""
		type4 = ! type4;
#line 848 ""
	    }
#line 849 ""
	} else {
#line 850 ""
	    goto L11;
#line 851 ""
	}
#line 852 ""
/* L10: */
#line 852 ""
    }
#line 853 ""
    s_wsle(&io___119);
#line 853 ""
    do_lio(&c__9, &c__1, "You should not be here in scor2prt.  Call Dr. Don", 
	    (ftnlen)49);
#line 853 ""
    e_wsle();
#line 854 ""
    s_stop("", (ftnlen)0);
#line 855 ""
L11:

/*  Finished reading opening type4 TeX and comments.  Next line to be read */
/*  will contain the first of the input numbers */

#line 860 ""
    dosetup_(iv, line, &mtrnum, &mtrden, (ftnlen)128);
#line 861 ""
    for (i__ = 1; i__ <= 10000; ++i__) {
#line 862 ""
L13:
#line 862 ""
	io___122.ciunit = *iv + 10;
#line 862 ""
	i__3 = s_rsfe(&io___122);
#line 862 ""
	if (i__3 != 0) {
#line 862 ""
	    goto L999;
#line 862 ""
	}
#line 862 ""
	i__3 = do_fio(&c__1, line, (ftnlen)128);
#line 862 ""
	if (i__3 != 0) {
#line 862 ""
	    goto L999;
#line 862 ""
	}
#line 862 ""
	i__3 = e_rsfe();
#line 862 ""
	if (i__3 != 0) {
#line 862 ""
	    goto L999;
#line 862 ""
	}
#line 863 ""
L7:
#line 863 ""
	len = lenstr_(line, &c__128, (ftnlen)128);

/*  Pass-through (and copy into part file) if instrumnet has >1 voice. */

#line 867 ""
	if (*ljob < 0) {
#line 867 ""
	    goto L2;
#line 867 ""
	}
#line 868 ""
	if (i_indx("TtTiTch+h-h l ", line, (ftnlen)14, (ftnlen)2) > 0) {

/*  Traps titles, instruments, composers, headers, lower strings.  Read 2 lines. */

#line 872 ""
	    s_wsfe(&io___123);
#line 872 ""
	    do_fio(&c__1, line, len);
#line 872 ""
	    e_wsfe();
#line 873 ""
	    io___124.ciunit = *iv + 10;
#line 873 ""
	    s_rsfe(&io___124);
#line 873 ""
	    do_fio(&c__1, line, (ftnlen)128);
#line 873 ""
	    e_rsfe();
#line 874 ""
	    len = lenstr_(line, &c__128, (ftnlen)128);
#line 875 ""
	    goto L2;
#line 876 ""
	}
#line 877 ""
	if (i__ == 1 || i__ > 5 && *(unsigned char *)&line[0] == 'm') {

/*  Either just starting, or a new meter is defined. */
/*  NOTE! The above test may be bogus. */

#line 882 ""
	    if (*(unsigned char *)&line[0] == '%') {
#line 883 ""
		s_wsfe(&io___125);
#line 883 ""
		do_fio(&c__1, line, len);
#line 883 ""
		e_wsfe();
#line 884 ""
		goto L13;
#line 885 ""
	    }
#line 886 ""
	    if (i__ != 1) {

/*  New meter. Check for slashes (new meter change syntax) */

#line 890 ""
		idxs = i_indx(line, "/", (ftnlen)128, (ftnlen)1);
#line 891 ""
		idxb = i_indx(line, " ", (ftnlen)128, (ftnlen)1);
#line 892 ""
		newmtr = idxs > 0 && (idxb == 0 || idxs < idxb);
#line 893 ""
		if (! newmtr) {

/*  Old way, no slashes, uses 'o' for lonesome '1' */

#line 897 ""
		    icden = 3;
#line 898 ""
		    if (*(unsigned char *)&line[1] == 'o') {
#line 899 ""
			mtrnum = 1;
#line 900 ""
		    } else {
#line 901 ""
			mtrnum = *(unsigned char *)&line[1] - 48;
#line 902 ""
			if (mtrnum == 1) {
#line 903 ""
			    icden = 4;
#line 904 ""
			    mtrnum = *(unsigned char *)&line[2] - 38;
#line 905 ""
			}
#line 906 ""
		    }
#line 907 ""
		    mtrden = *(unsigned char *)&line[icden - 1] - 48;
#line 908 ""
		} else {

/*  New way with slashes: idxs is index of 1st slash! */

#line 912 ""
		    ici__1.icierr = 0;
#line 912 ""
		    ici__1.iciend = 0;
#line 912 ""
		    ici__1.icirnum = 1;
#line 912 ""
		    ici__1.icirlen = idxs - 2;
#line 912 ""
		    ici__1.iciunit = line + 1;
/* Writing concatenation */
#line 912 ""
		    i__2[0] = 2, a__2[0] = "(i";
#line 912 ""
		    *(unsigned char *)&ch__1[0] = idxs + 46;
#line 912 ""
		    i__2[1] = 1, a__2[1] = ch__1;
#line 912 ""
		    i__2[2] = 1, a__2[2] = ")";
#line 912 ""
		    ici__1.icifmt = (s_cat(ch__4, a__2, i__2, &c__3, (ftnlen)
			    4), ch__4);
#line 912 ""
		    s_rsfi(&ici__1);
#line 912 ""
		    do_fio(&c__1, (char *)&mtrnum, (ftnlen)sizeof(integer));
#line 912 ""
		    e_rsfi();
#line 913 ""
		    i__3 = idxs;
#line 913 ""
		    idxb = i_indx(line + i__3, "/", 128 - i__3, (ftnlen)1);
#line 914 ""
		    i__3 = idxs;
#line 914 ""
		    ici__1.icierr = 0;
#line 914 ""
		    ici__1.iciend = 0;
#line 914 ""
		    ici__1.icirnum = 1;
#line 914 ""
		    ici__1.icirlen = idxs + idxb - 1 - i__3;
#line 914 ""
		    ici__1.iciunit = line + i__3;
/* Writing concatenation */
#line 914 ""
		    i__2[0] = 2, a__2[0] = "(i";
#line 914 ""
		    *(unsigned char *)&ch__1[0] = idxb + 47;
#line 914 ""
		    i__2[1] = 1, a__2[1] = ch__1;
#line 914 ""
		    i__2[2] = 1, a__2[2] = ")";
#line 914 ""
		    ici__1.icifmt = (s_cat(ch__4, a__2, i__2, &c__3, (ftnlen)
			    4), ch__4);
#line 914 ""
		    s_rsfi(&ici__1);
#line 914 ""
		    do_fio(&c__1, (char *)&mtrden, (ftnlen)sizeof(integer));
#line 914 ""
		    e_rsfi();
#line 916 ""
		}
#line 917 ""
	    }
#line 918 ""
	    lenbeat = ifnodur_(&mtrden, "x", (ftnlen)1);
#line 919 ""
	    lenmult = 1;
#line 920 ""
	    if (mtrden == 2) {
#line 921 ""
		lenbeat = 16;
#line 922 ""
		lenmult = 2;
#line 923 ""
	    }
#line 924 ""
	    lenbar = lenmult * mtrnum * lenbeat;
#line 925 ""
	    fwbrsym_(&lenbar, &nwbrs, wbrsym, &lwbrs, (ftnlen)3);
#line 926 ""
	}

/* Finished setting up meter stuff and defining whole-bar rest symbols */

#line 930 ""
	ip1 = 0;
#line 931 ""
	s_copy(line1, line, (ftnlen)128, (ftnlen)128);
#line 932 ""
	i__3 = nwbrs;
#line 932 ""
	for (iw = 0; iw <= i__3; ++iw) {
#line 933 ""
	    if (iw > 0) {
#line 934 ""
		idx = ntindex_(line1, wbrsym + (iw - 1) * 3, (ftnlen)128, 
			lwbrs);
#line 935 ""
		if (idx > 0) {

/*  Check for blank or shifted rest, discount it if it's there */

#line 939 ""
		    i__4 = idx + lwbrs - 1;
#line 939 ""
		    if (s_cmp(line1 + i__4, " ", idx + lwbrs - i__4, (ftnlen)
			    1) != 0) {
#line 939 ""
			idx = 0;
#line 939 ""
		    }
#line 940 ""
		}
#line 941 ""
	    } else {
#line 942 ""
		idx = ntindex_(line1, "rp", (ftnlen)128, (ftnlen)2);

/*  Check for raised rest */

#line 946 ""
		if (idx > 0) {
#line 947 ""
		    i__4 = idx + 1;
#line 947 ""
		    if (s_cmp(line1 + i__4, " ", idx + 2 - i__4, (ftnlen)1) !=
			     0) {
#line 947 ""
			idx = 0;
#line 947 ""
		    }
#line 948 ""
		}
#line 949 ""
	    }
#line 950 ""
	    if (idx > 0) {
#line 951 ""
		if (ip1 == 0) {
#line 952 ""
		    ip1 = idx;
#line 953 ""
		} else {
#line 954 ""
		    ip1 = min(ip1,idx);
/* Maybe allows e.g. r0 rp ... */
#line 955 ""
		}
#line 956 ""
	    }
#line 957 ""
/* L3: */
#line 957 ""
	}
/* Writing concatenation */
#line 958 ""
	i__1[0] = 1, a__1[0] = sq;
#line 958 ""
	i__1[1] = 1, a__1[1] = sq;
#line 958 ""
	s_cat(ch__5, a__1, i__1, &c__2, (ftnlen)2);
#line 958 ""
	if (i__ < 5 || *(unsigned char *)&line[0] == '%' || s_cmp(line, ch__5,
		 (ftnlen)2, (ftnlen)2) == 0 || ip1 == 0) {
#line 958 ""
	    goto L2;
#line 958 ""
	}

/*  Switch to multibar rest search mode!!!  Start forward in line(1) */

#line 963 ""
	rpfirst = s_cmp(line1 + (ip1 - 1), "rp", (ftnlen)2, (ftnlen)2) == 0;
#line 964 ""
	iline = 1;
#line 965 ""
	nmbr = 1;
#line 966 ""
	if (rpfirst) {
#line 967 ""
	    lwbrsx = 2;
#line 968 ""
	} else {
#line 969 ""
	    lwbrsx = lwbrs;
#line 970 ""
	}
#line 971 ""
	ipe = ip1 + lwbrsx - 1;
/* ip at end of 1st wbrsym */
#line 972 ""
L4:
#line 972 ""
	if (ipe == len) {

/*  Need a new line */

#line 976 ""
	    ++iline;
#line 977 ""
L6:
#line 977 ""
	    io___145.ciunit = *iv + 10;
#line 977 ""
	    i__3 = s_rsfe(&io___145);
#line 977 ""
	    if (i__3 != 0) {
#line 977 ""
		goto L998;
#line 977 ""
	    }
#line 977 ""
	    i__3 = do_fio(&c__1, line + (iline - 1 << 7), (ftnlen)128);
#line 977 ""
	    if (i__3 != 0) {
#line 977 ""
		goto L998;
#line 977 ""
	    }
#line 977 ""
	    i__3 = e_rsfe();
#line 977 ""
	    if (i__3 != 0) {
#line 977 ""
		goto L998;
#line 977 ""
	    }
#line 978 ""
	    len = lenstr_(line + (iline - 1 << 7), &c__128, (ftnlen)128);
#line 979 ""
	    if (*(unsigned char *)&line[(iline - 1) * 128] == '%') {
#line 980 ""
		s_wsfe(&io___146);
#line 980 ""
		do_fio(&c__1, "% Following comment has been moved forward", (
			ftnlen)42);
#line 980 ""
		e_wsfe();
#line 981 ""
		s_wsfe(&io___147);
#line 981 ""
		do_fio(&c__1, line + (iline - 1 << 7), len);
#line 981 ""
		e_wsfe();
#line 982 ""
		goto L6;
#line 983 ""
	    }
#line 984 ""
	    ipe = 0;
#line 985 ""
	    goto L4;
#line 986 ""
L998:

/*  No more input left */

#line 990 ""
	    s_wsle(&io___148);
#line 990 ""
	    do_lio(&c__9, &c__1, "All done!", (ftnlen)9);
#line 990 ""
	    e_wsle();
#line 991 ""
	    alldone = TRUE_;
#line 992 ""
	    ipe = 0;
#line 993 ""
	    --iline;
#line 994 ""
	    len = lenstr_(line + (iline - 1 << 7), &c__128, (ftnlen)128);
#line 995 ""
	    goto L4;
#line 996 ""
	} else {
#line 997 ""
	    if (alldone) {
#line 998 ""
		*(unsigned char *)sym = ' ';
#line 999 ""
	    } else {

/*  ipe<len here, so it's ok to get a symbol */

#line 1003 ""
		nextsym_(line + (iline - 1 << 7), &len, &ipe, &ipenew, sym, &
			lsym, (ftnlen)128, (ftnlen)80);
#line 1004 ""
	    }

/*  Check for end of block or bar line symbol */

#line 1008 ""
	    if (i_indx("/|", sym, (ftnlen)2, (ftnlen)1) > 0) {
#line 1009 ""
		ipe = ipenew;
#line 1010 ""
		goto L4;
#line 1011 ""
	    } else {
#line 1012 ""
		wbrest = FALSE_;
#line 1013 ""
		if (alldone) {
#line 1013 ""
		    goto L12;
#line 1013 ""
		}
#line 1014 ""
		i__3 = nwbrs;
#line 1014 ""
		for (iw = 1; iw <= i__3; ++iw) {
#line 1015 ""
		    wbrest = wbrest || s_cmp(sym, wbrsym + (iw - 1) * 3, lsym,
			     lwbrs) == 0;
#line 1016 ""
/* L5: */
#line 1016 ""
		}
#line 1017 ""
		wbrest = wbrest || s_cmp(sym, "r", lsym, (ftnlen)1) == 0 && 
			lwbrs == 2 || s_cmp(sym, "rd", lsym, (ftnlen)2) == 0 
			&& lwbrs == 3 || s_cmp(sym, "rp", lsym, (ftnlen)2) == 
			0 || s_cmp(sym, "r", lsym, (ftnlen)1) == 0 && rpfirst;
#line 1021 ""
L12:
#line 1021 ""
		if (wbrest) {
#line 1022 ""
		    ipe = ipenew;
#line 1023 ""
		    ++nmbr;
#line 1024 ""
		    goto L4;
#line 1025 ""
		} else {

/*  AHA! Failed prev. test, so last symbol was *not* mbr. */
/*  It must be saved, and its starting position is ipenew-lsym+1 */

#line 1030 ""
		    if (nmbr > 1) {

/*  Write stuff up to start of mbr */

#line 1034 ""
			if (ip1 > 1) {
#line 1034 ""
			    s_wsfe(&io___153);
#line 1034 ""
			    do_fio(&c__1, line, ip1 - 1);
#line 1034 ""
			    e_wsfe();
#line 1034 ""
			}

/*  Insert mbr symbol.  Always end with a slash just in case next sym must be */
/*  at start of block.  May think this causes undefined octaves, but */
/*  probably not since it's a single voice. */

#line 1040 ""
			r__1 = nmbr + .01f;
#line 1040 ""
			ndig = (integer) r_lg10(&r__1) + 1;
#line 1041 ""
			s_wsle(&io___155);
#line 1041 ""
			do_lio(&c__9, &c__1, "Inserting rm, iv,nmbr:", (
				ftnlen)22);
#line 1041 ""
			do_lio(&c__3, &c__1, (char *)&(*iv), (ftnlen)sizeof(
				integer));
#line 1041 ""
			do_lio(&c__3, &c__1, (char *)&nmbr, (ftnlen)sizeof(
				integer));
#line 1041 ""
			e_wsle();
#line 1042 ""
			ci__1.cierr = 0;
#line 1042 ""
			ci__1.ciunit = 40;
/* Writing concatenation */
#line 1042 ""
			i__2[0] = 5, a__2[0] = "(a2,i";
#line 1042 ""
			*(unsigned char *)&ch__1[0] = ndig + 48;
#line 1042 ""
			i__2[1] = 1, a__2[1] = ch__1;
#line 1042 ""
			i__2[2] = 4, a__2[2] = ",a2)";
#line 1042 ""
			ci__1.cifmt = (s_cat(ch__6, a__2, i__2, &c__3, (
				ftnlen)10), ch__6);
#line 1042 ""
			s_wsfe(&ci__1);
#line 1042 ""
			do_fio(&c__1, "rm", (ftnlen)2);
#line 1042 ""
			do_fio(&c__1, (char *)&nmbr, (ftnlen)sizeof(integer));
#line 1042 ""
			do_fio(&c__1, " /", (ftnlen)2);
#line 1042 ""
			e_wsfe();
#line 1043 ""
			if (alldone) {
#line 1043 ""
			    goto L999;
#line 1043 ""
			}
#line 1044 ""
			ipc = ipenew - lsym + 1;
#line 1045 ""
			s_copy(line, line + ((iline - 1 << 7) + (ipc - 1)), (
				ftnlen)128, len - (ipc - 1));
#line 1046 ""
		    } else {

/*  Write old stuff up to end of original lonesome wbr, save the rest. */
/*  4 cases:  (wbr /) , (wbr line-end) , (wbr followed by other non-/ symbols) , */
/*      alldone. */
/*  In 1st 2 will have gotten some other lines, so write all up to one b4 last */
/*  non-comment line; then revert to normal mode on that.  In 3rd case must */
/*  split line. */

#line 1055 ""
			if (alldone) {
#line 1056 ""
			    s_wsfe(&io___157);
#line 1056 ""
			    do_fio(&c__1, line, len);
#line 1056 ""
			    e_wsfe();
#line 1057 ""
			    goto L999;
#line 1058 ""
			} else if (iline > 1) {
#line 1059 ""
			    i__3 = iline - 1;
#line 1059 ""
			    for (il = 1; il <= i__3; ++il) {
#line 1060 ""
				len = lenstr_(line + (il - 1 << 7), &c__128, (
					ftnlen)128);
#line 1061 ""
				s_wsfe(&io___159);
#line 1061 ""
				do_fio(&c__1, line + (il - 1 << 7), len);
#line 1061 ""
				e_wsfe();
#line 1062 ""
/* L9: */
#line 1062 ""
			    }
#line 1063 ""
			    s_copy(line, line + (iline - 1 << 7), (ftnlen)128,
				     (ftnlen)128);
#line 1064 ""
			} else {

/*  Since iline = 1 the wbr is not the last sym, so must split */

#line 1068 ""
			    s_wsfe(&io___160);
#line 1068 ""
			    do_fio(&c__1, line, ip1 + lwbrsx - 1);
#line 1068 ""
			    e_wsfe();
#line 1069 ""
			    i__3 = ip1 + lwbrsx;
#line 1069 ""
			    s_copy(line, line + i__3, (ftnlen)128, len - i__3)
				    ;
#line 1070 ""
			}
#line 1071 ""
		    }

/*  Exit multibar mode */

#line 1075 ""
		    goto L7;
#line 1076 ""
		}
#line 1077 ""
	    }
#line 1078 ""
	}
#line 1079 ""
L2:
#line 1080 ""
	if (len > 0) {
#line 1081 ""
	    s_wsfe(&io___161);
#line 1081 ""
	    do_fio(&c__1, line, len);
#line 1081 ""
	    e_wsfe();
#line 1082 ""
	} else {
#line 1083 ""
	    s_wsfe(&io___162);
#line 1083 ""
	    do_fio(&c__1, " ", (ftnlen)1);
#line 1083 ""
	    e_wsfe();
#line 1084 ""
	}
#line 1085 ""
/* L1: */
#line 1085 ""
    }
#line 1086 ""
L999:
#line 1087 ""
    cl__1.cerr = 0;
#line 1087 ""
    cl__1.cunit = *iv + 10;
#line 1087 ""
    cl__1.csta = 0;
#line 1087 ""
    f_clos(&cl__1);
#line 1088 ""
    cl__1.cerr = 0;
#line 1088 ""
    cl__1.cunit = 40;
#line 1088 ""
    cl__1.csta = 0;
#line 1088 ""
    f_clos(&cl__1);
#line 1089 ""
    return 0;
} /* mbrests_ */

integer ifnodur_(integer *idur, char *dotq, ftnlen dotq_len)
{
    /* System generated locals */
    integer ret_val;

    /* Builtin functions */
    integer s_wsle(cilist *), do_lio(integer *, integer *, char *, ftnlen), 
	    e_wsle(void);
    /* Subroutine */ int s_stop(char *, ftnlen);

    /* Fortran I/O blocks */
    static cilist io___163 = { 0, 6, 0, 0, 0 };


#line 1093 ""
    if (*idur == 6) {
#line 1094 ""
	ret_val = 1;
#line 1095 ""
    } else if (*idur == 3) {
#line 1096 ""
	ret_val = 2;
#line 1097 ""
    } else if (*idur == 1 || *idur == 16) {
#line 1098 ""
	ret_val = 4;
#line 1099 ""
    } else if (*idur == 8) {
#line 1100 ""
	ret_val = 8;
#line 1101 ""
    } else if (*idur == 4) {
#line 1102 ""
	ret_val = 16;
#line 1103 ""
    } else if (*idur == 2) {
#line 1104 ""
	ret_val = 32;
#line 1105 ""
    } else if (*idur == 0) {
#line 1106 ""
	ret_val = 64;
#line 1107 ""
    } else {
#line 1108 ""
	s_wsle(&io___163);
#line 1108 ""
	do_lio(&c__9, &c__1, "You entered an invalid note-length value", (
		ftnlen)40);
#line 1108 ""
	e_wsle();
#line 1109 ""
	s_stop("", (ftnlen)0);
#line 1110 ""
    }
#line 1111 ""
    if (*(unsigned char *)dotq == 'd') {
#line 1111 ""
	ret_val = ret_val * 3 / 2;
#line 1111 ""
    }
#line 1112 ""
    return ret_val;
} /* ifnodur_ */

/* Subroutine */ int fwbrsym_(integer *lenbar, integer *nwbrs, char *wbrsym, 
	integer *lwbrs, ftnlen wbrsym_len)
{
    /* Builtin functions */
    /* Subroutine */ int s_copy(char *, char *, ftnlen, ftnlen);
    integer s_wsfe(cilist *), do_fio(integer *, char *, ftnlen), e_wsfe(void);

    /* Fortran I/O blocks */
    static cilist io___164 = { 0, 6, 0, "(33H Any whole-bar rests of duratio"
	    "n ,i3,\r                 26H/64 will not be recognized)", 0 };


#line 1116 ""
    /* Parameter adjustments */
#line 1116 ""
    wbrsym -= 3;
#line 1116 ""

#line 1116 ""
    /* Function Body */
#line 1116 ""
    *nwbrs = 1;
#line 1117 ""
    *lwbrs = 2;
#line 1118 ""
    if (*lenbar == 16) {
#line 1119 ""
	s_copy(wbrsym + 3, "r4", (ftnlen)3, (ftnlen)2);
#line 1120 ""
    } else if (*lenbar == 32) {
#line 1121 ""
	s_copy(wbrsym + 3, "r2", (ftnlen)3, (ftnlen)2);
#line 1122 ""
    } else if (*lenbar == 64) {
#line 1123 ""
	s_copy(wbrsym + 3, "r0", (ftnlen)3, (ftnlen)2);
#line 1124 ""
    } else if (*lenbar == 8) {
#line 1125 ""
	s_copy(wbrsym + 3, "r8", (ftnlen)3, (ftnlen)2);
#line 1126 ""
    } else if (*lenbar == 128) {
#line 1127 ""
	s_copy(wbrsym + 3, "r9", (ftnlen)3, (ftnlen)2);
#line 1128 ""
    } else {
#line 1129 ""
	*nwbrs = 2;
#line 1130 ""
	*lwbrs = 3;
#line 1131 ""
	if (*lenbar == 24) {
#line 1132 ""
	    s_copy(wbrsym + 3, "rd4", (ftnlen)3, (ftnlen)3);
#line 1133 ""
	    s_copy(wbrsym + 6, "r4d", (ftnlen)3, (ftnlen)3);
#line 1134 ""
	} else if (*lenbar == 48) {
#line 1135 ""
	    s_copy(wbrsym + 3, "rd2", (ftnlen)3, (ftnlen)3);
#line 1136 ""
	    s_copy(wbrsym + 6, "r2d", (ftnlen)3, (ftnlen)3);
#line 1137 ""
	} else if (*lenbar == 96) {
#line 1138 ""
	    s_copy(wbrsym + 3, "rd0", (ftnlen)3, (ftnlen)3);
#line 1139 ""
	    s_copy(wbrsym + 6, "r0d", (ftnlen)3, (ftnlen)3);
#line 1140 ""
	} else {
#line 1141 ""
	    s_wsfe(&io___164);
#line 1141 ""
	    do_fio(&c__1, (char *)&(*lenbar), (ftnlen)sizeof(integer));
#line 1141 ""
	    e_wsfe();
#line 1143 ""
	}
#line 1144 ""
    }
#line 1145 ""
    return 0;
} /* fwbrsym_ */

/* Subroutine */ int nextsym_(char *line, integer *len, integer *ipeold, 
	integer *ipenew, char *sym, integer *lsym, ftnlen line_len, ftnlen 
	sym_len)
{
    /* System generated locals */
    integer i__1, i__2;

    /* Builtin functions */
    integer s_wsle(cilist *), do_lio(integer *, integer *, char *, ftnlen), 
	    e_wsle(void);
    /* Subroutine */ int s_stop(char *, ftnlen), s_copy(char *, char *, 
	    ftnlen, ftnlen);

    /* Local variables */
    static integer ip, iip;

    /* Fortran I/O blocks */
    static cilist io___165 = { 0, 6, 0, 0, 0 };
    static cilist io___166 = { 0, 6, 0, 0, 0 };
    static cilist io___169 = { 0, 6, 0, 0, 0 };



/*  Know its the last symbol if on return ipenew = len!.  So should never */
/*    be called when ipstart=len. */

#line 1154 ""
    if (*ipeold >= *len) {
#line 1155 ""
	s_wsle(&io___165);
#line 1155 ""
	do_lio(&c__9, &c__1, "Called nextsym with ipstart>=len ", (ftnlen)33);
#line 1155 ""
	e_wsle();
#line 1156 ""
	s_wsle(&io___166);
#line 1156 ""
	do_lio(&c__9, &c__1, "Send files to Dr. Don at dsimons@logicon.com", (
		ftnlen)44);
#line 1156 ""
	e_wsle();
#line 1157 ""
	s_stop("", (ftnlen)0);
#line 1158 ""
    }
#line 1159 ""
    i__1 = *len;
#line 1159 ""
    for (ip = *ipeold + 1; ip <= i__1; ++ip) {
#line 1160 ""
	if (*(unsigned char *)&line[ip - 1] != ' ') {

/*  symbol starts here (ip).  We're committed to exit the loop. */

#line 1164 ""
	    if (ip < *len) {
#line 1165 ""
		i__2 = *len;
#line 1165 ""
		for (iip = ip + 1; iip <= i__2; ++iip) {
#line 1166 ""
		    if (*(unsigned char *)&line[iip - 1] != ' ') {
#line 1166 ""
			goto L2;
#line 1166 ""
		    }

/*  iip is the space after the symbol */

#line 1170 ""
		    *ipenew = iip - 1;
#line 1171 ""
		    *lsym = *ipenew - ip + 1;
#line 1172 ""
		    s_copy(sym, line + (ip - 1), (ftnlen)80, *ipenew - (ip - 
			    1));
#line 1173 ""
		    return 0;
#line 1174 ""
L2:
#line 1174 ""
		    ;
#line 1174 ""
		}

/*  Have len>=2 and ends on len */

#line 1178 ""
		*ipenew = *len;
#line 1179 ""
		*lsym = *ipenew - ip + 1;
#line 1180 ""
		s_copy(sym, line + (ip - 1), (ftnlen)80, *ipenew - (ip - 1));
#line 1181 ""
		return 0;
#line 1182 ""
	    } else {

/*  ip = len */

#line 1186 ""
		*ipenew = *len;
#line 1187 ""
		*lsym = 1;
#line 1188 ""
		s_copy(sym, line + (ip - 1), (ftnlen)80, (ftnlen)1);
#line 1189 ""
		return 0;
#line 1190 ""
	    }
#line 1191 ""
	}
#line 1192 ""
/* L1: */
#line 1192 ""
    }
#line 1193 ""
    s_wsle(&io___169);
#line 1193 ""
    do_lio(&c__9, &c__1, "Error #3.  Send files to Dr. Don at dsimons@logico"\
	    "n.com", (ftnlen)55);
#line 1193 ""
    e_wsle();
#line 1194 ""
    return 0;
} /* nextsym_ */

integer ntindex_(char *line, char *s2q, ftnlen line_len, ftnlen s2q_len)
{
    /* System generated locals */
    address a__1[2];
    integer ret_val, i__1, i__2[2], i__3;

    /* Builtin functions */
    /* Subroutine */ int s_copy(char *, char *, ftnlen, ftnlen);
    integer i_indx(char *, char *, ftnlen, ftnlen), s_wsle(cilist *), do_lio(
	    integer *, integer *, char *, ftnlen), e_wsle(void);
    /* Subroutine */ int s_stop(char *, ftnlen), s_cat(char *, char **, 
	    integer *, integer *, ftnlen);
    integer s_cmp(char *, char *, ftnlen, ftnlen);

    /* Local variables */
    static integer ic, len, ndxs2;
    static char tline[128];
    static integer ndxbs;
    static logical intex;
    static integer ndxdq1, ndxdq2;
    extern integer lenstr_(char *, integer *, ftnlen);

    /* Fortran I/O blocks */
    static cilist io___175 = { 0, 6, 0, 0, 0 };



/*  Returns index(line,s2q) if NOT in TeX string, 0 otherwise */


/*     print*,'Starting ntindex.  s2q:',s2q,', line(1:79) is below' */
/*     print*,line(1:79) */


/*  Use a temporary string to store the input and test, so can zap D"..." */

#line 1209 ""
    s_copy(tline, line, (ftnlen)128, (ftnlen)128);
#line 1210 ""
    ndxs2 = i_indx(tline, s2q, (ftnlen)128, s2q_len);

/*  Return point below for rechecks after zapping D"  " */

#line 1214 ""
L2:
#line 1215 ""
    ndxbs = i_indx(tline, "\\", (ftnlen)128, (ftnlen)1);
#line 1216 ""
    if (ndxbs > 0) {

/* Special check in case \ is inside D"..." */

#line 1220 ""
	ndxdq1 = i_indx(tline, "D\"", (ftnlen)128, (ftnlen)2);

/* If the following test fails, flow out of if block; else loop up to 2. */

#line 1224 ""
	if (ndxdq1 > 0) {

/* Find end of D"..." */

#line 1228 ""
	    i__1 = ndxdq1 + 1;
#line 1228 ""
	    ndxdq2 = ndxdq1 + 1 + i_indx(tline + i__1, "\"", 128 - i__1, (
		    ftnlen)1);
#line 1229 ""
	    if (ndxdq2 == ndxdq1 + 1) {
#line 1230 ""
		s_wsle(&io___175);
#line 1230 ""
		do_lio(&c__9, &c__1, "Something is really wierd here", (
			ftnlen)30);
#line 1230 ""
		e_wsle();
#line 1231 ""
		s_stop("", (ftnlen)0);
#line 1232 ""
	    }
#line 1233 ""
	    s_copy(tline, tline, (ftnlen)128, ndxdq1 - 1);
#line 1234 ""
	    i__1 = ndxdq2;
#line 1234 ""
	    for (ic = ndxdq1; ic <= i__1; ++ic) {
/* Writing concatenation */
#line 1235 ""
		i__2[0] = ic - 1, a__1[0] = tline;
#line 1235 ""
		i__2[1] = 1, a__1[1] = " ";
#line 1235 ""
		s_cat(tline, a__1, i__2, &c__2, (ftnlen)128);
#line 1236 ""
/* L3: */
#line 1236 ""
	    }
#line 1237 ""
	    i__1 = ndxdq2;
/* Writing concatenation */
#line 1237 ""
	    i__2[0] = ndxdq2, a__1[0] = tline;
#line 1237 ""
	    i__2[1] = 128 - i__1, a__1[1] = line + i__1;
#line 1237 ""
	    s_cat(tline, a__1, i__2, &c__2, (ftnlen)128);
#line 1238 ""
	    goto L2;
#line 1239 ""
	}
#line 1240 ""
    }
#line 1241 ""
    if (ndxbs == 0 || ndxs2 < ndxbs) {
#line 1242 ""
	ret_val = ndxs2;
/*     print*,'No bs, or char is left of 1st bs, ntindex:',ntindex */
#line 1244 ""
    } else {

/*  There are both bs and s2q, and bs is to the left of sq2. So check bs's to */
/*  right of first: End is '\ ', start is ' \' */

#line 1249 ""
	len = lenstr_(tline, &c__128, (ftnlen)128);
#line 1250 ""
	intex = TRUE_;
/*     print*,'intex+>',intex */
#line 1252 ""
	i__1 = len;
#line 1252 ""
	for (ic = ndxbs + 1; ic <= i__1; ++ic) {
#line 1253 ""
	    if (ic == ndxs2) {
#line 1254 ""
		if (intex) {
#line 1255 ""
		    ret_val = 0;
#line 1256 ""
		    i__3 = ic;
#line 1256 ""
		    ndxs2 = i_indx(tline + i__3, s2q, len - i__3, s2q_len) + 
			    ic;
/*     print*,'ndxs2 =>',ndxs2 */
#line 1258 ""
		} else {
#line 1259 ""
		    ret_val = ndxs2;
#line 1260 ""
		    return ret_val;
#line 1261 ""
		}
/*     print*,'Internal exit, intex, ntindex:',intex,ntindex */
#line 1263 ""
	    } else /* if(complicated condition) */ {
#line 1263 ""
		i__3 = ic;
#line 1263 ""
		if (intex && s_cmp(tline + i__3, "\\ ", ic + 2 - i__3, (
			ftnlen)2) == 0) {
#line 1264 ""
		    intex = FALSE_;
/*     print*,'intex+>',intex */
#line 1266 ""
		} else /* if(complicated condition) */ {
#line 1266 ""
		    i__3 = ic;
#line 1266 ""
		    if (! intex && s_cmp(tline + i__3, " \\", ic + 2 - i__3, (
			    ftnlen)2) == 0) {
#line 1268 ""
			intex = TRUE_;
/*     print*,'intex+>',intex */
#line 1270 ""
		    }
#line 1270 ""
		}
#line 1270 ""
	    }
#line 1271 ""
/* L1: */
#line 1271 ""
	}
/*     print*,'Out end of loop 1' */
#line 1273 ""
    }
/*     print*,'Exiting ntindex at the end???' */
#line 1275 ""
    return ret_val;
} /* ntindex_ */

/* Subroutine */ int getchar_(char *line, integer *iccount, char *charq, 
	ftnlen line_len, ftnlen charq_len)
{
    /* Builtin functions */
    integer s_rsfe(cilist *), do_fio(integer *, char *, ftnlen), e_rsfe(void);

    /* Fortran I/O blocks */
    static cilist io___179 = { 0, 10, 0, "(a)", 0 };



/*  Gets the next character out of line*128.  If pointer iccount=128 on entry, */
/*  then reads in a new line.  Resets iccount to position of the new character. */

#line 1284 ""
    if (*iccount == 128) {
#line 1285 ""
	s_rsfe(&io___179);
#line 1285 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 1285 ""
	e_rsfe();
#line 1286 ""
	*iccount = 0;
#line 1287 ""
    }
#line 1288 ""
    ++(*iccount);
#line 1289 ""
    *(unsigned char *)charq = *(unsigned char *)&line[*iccount - 1];
#line 1290 ""
    return 0;
} /* getchar_ */

doublereal readin_(char *line, integer *iccount, integer *iread, ftnlen 
	line_len)
{
    /* System generated locals */
    address a__1[3];
    integer i__1[3], i__2;
    real ret_val;
    char ch__1[27], ch__2[6], ch__3[1];
    icilist ici__1;

    /* Builtin functions */
    integer s_rsfe(cilist *), do_fio(integer *, char *, ftnlen), e_rsfe(void),
	     i_indx(char *, char *, ftnlen, ftnlen), s_wsle(cilist *);
    /* Subroutine */ int s_cat(char *, char **, integer *, integer *, ftnlen);
    integer do_lio(integer *, integer *, char *, ftnlen), e_wsle(void);
    /* Subroutine */ int s_stop(char *, ftnlen);
    integer s_rsfi(icilist *), e_rsfi(void);

    /* Local variables */
    extern /* Subroutine */ int allparts_(char *, integer *, ftnlen);
    static integer i1, i2, icf;
    static char durq[1];
    static logical goto999;
    extern /* Subroutine */ int chkcom_(char *, logical *, ftnlen), getchar_(
	    char *, integer *, char *, ftnlen, ftnlen);

    /* Fortran I/O blocks */
    static cilist io___180 = { 0, 10, 0, "(a)", 0 };
    static cilist io___185 = { 0, 6, 0, 0, 0 };
    static cilist io___187 = { 0, 6, 0, 0, 0 };



/*  Reads a piece of setup data from line, gets a new line from */
/*  file 10 (jobname.pmx) if needed, Transfers comment lines into all parts. */

/*  iread controls copying of values into scratch files for parts, but only */
/*  if not replacing. */

/*  iread input  value written */
/*   -1   nvi      nothing (only used when noinst<0 initially) */
/*   0   various   value read */
/*   1    nv       -1 , replace later with nvi(i) */
/*   2    noinst    1 */
/*   3    np       -2 , replace later with (nsyst-1)/12+1 */
/*   4  musicsize   20 */
/*   5  fracondent  0.05 */

#line 1315 ""
L4:
#line 1315 ""
    if (*iccount == 128) {
#line 1316 ""
	s_rsfe(&io___180);
#line 1316 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 1316 ""
	e_rsfe();
#line 1317 ""
	if (all_1.replacing) {
#line 1317 ""
	    all_1.replacing = FALSE_;
#line 1317 ""
	}
#line 1318 ""
	chkcom_(line, &goto999, (ftnlen)128);
#line 1319 ""
	*iccount = 0;
#line 1320 ""
    }
#line 1321 ""
    ++(*iccount);

/*  Find next non-blank or end of line */

#line 1325 ""
    for (*iccount = *iccount; *iccount <= 127; ++(*iccount)) {
#line 1326 ""
	if (*(unsigned char *)&line[*iccount - 1] != ' ') {
#line 1326 ""
	    goto L3;
#line 1326 ""
	}
#line 1327 ""
/* L2: */
#line 1327 ""
    }

/*  If here, need to get a new line */

#line 1331 ""
    *iccount = 128;
#line 1332 ""
    goto L4;
#line 1333 ""
L3:

/*  iccount now points to start of number to read */

#line 1337 ""
    i1 = *iccount;
#line 1338 ""
L5:
#line 1338 ""
    getchar_(line, iccount, durq, (ftnlen)128, (ftnlen)1);

/*  Remember that getchar first increments iccount, *then* reads a character. */

#line 1342 ""
    if (i_indx("0123456789.-", durq, (ftnlen)12, (ftnlen)1) > 0) {
#line 1342 ""
	goto L5;
#line 1342 ""
    }
#line 1343 ""
    i2 = *iccount - 1;
#line 1344 ""
    if (i2 < i1) {
#line 1345 ""
	s_wsle(&io___185);
/* Writing concatenation */
#line 1345 ""
	i__1[0] = 7, a__1[0] = "Found \"";
#line 1345 ""
	i__1[1] = 1, a__1[1] = durq;
#line 1345 ""
	i__1[2] = 19, a__1[2] = "\" instead of number";
#line 1345 ""
	s_cat(ch__1, a__1, i__1, &c__3, (ftnlen)27);
#line 1345 ""
	do_lio(&c__9, &c__1, ch__1, (ftnlen)27);
#line 1345 ""
	e_wsle();
#line 1346 ""
	s_stop("1", (ftnlen)1);
#line 1347 ""
    }
#line 1348 ""
    icf = i2 - i1 + 49;
#line 1349 ""
    ici__1.icierr = 0;
#line 1349 ""
    ici__1.iciend = 0;
#line 1349 ""
    ici__1.icirnum = 1;
#line 1349 ""
    ici__1.icirlen = i2 - (i1 - 1);
#line 1349 ""
    ici__1.iciunit = line + (i1 - 1);
/* Writing concatenation */
#line 1349 ""
    i__1[0] = 2, a__1[0] = "(f";
#line 1349 ""
    *(unsigned char *)&ch__3[0] = icf;
#line 1349 ""
    i__1[1] = 1, a__1[1] = ch__3;
#line 1349 ""
    i__1[2] = 3, a__1[2] = ".0)";
#line 1349 ""
    ici__1.icifmt = (s_cat(ch__2, a__1, i__1, &c__3, (ftnlen)6), ch__2);
#line 1349 ""
    s_rsfi(&ici__1);
#line 1349 ""
    do_fio(&c__1, (char *)&ret_val, (ftnlen)sizeof(real));
#line 1349 ""
    e_rsfi();
#line 1350 ""
    if (! all_1.replacing) {
#line 1351 ""
	if (*iread == 0) {
#line 1352 ""
	    i__2 = i2 - i1 + 1;
#line 1352 ""
	    allparts_(line + (i1 - 1), &i__2, i2 - (i1 - 1));
#line 1353 ""
	} else if (*iread == 1) {
#line 1354 ""
	    allparts_("-999", &c__4, (ftnlen)4);
#line 1355 ""
	} else if (*iread == 2) {
#line 1356 ""
	    allparts_("1", &c__1, (ftnlen)1);
#line 1357 ""
	} else if (*iread == 3) {
#line 1358 ""
	    allparts_("-998", &c__4, (ftnlen)4);
#line 1359 ""
	} else if (*iread == 4) {
#line 1360 ""
	    allparts_("20", &c__2, (ftnlen)2);
#line 1361 ""
	} else if (*iread == 5) {
#line 1362 ""
	    allparts_(".05", &c__3, (ftnlen)3);
#line 1363 ""
	} else if (*iread != -1) {
#line 1364 ""
	    s_wsle(&io___187);
#line 1364 ""
	    do_lio(&c__9, &c__1, "Error with iread in readin", (ftnlen)26);
#line 1364 ""
	    e_wsle();
#line 1365 ""
	    s_stop("", (ftnlen)0);
#line 1366 ""
	}
#line 1367 ""
    }
#line 1368 ""
    return ret_val;
} /* readin_ */

/* Subroutine */ int chkcom_(char *line, logical *goto999, ftnlen line_len)
{
    /* System generated locals */
    integer i__1, i__2;

    /* Builtin functions */
    integer s_rsfe(cilist *), do_fio(integer *, char *, ftnlen), e_rsfe(void),
	     i_indx(char *, char *, ftnlen, ftnlen), s_cmp(char *, char *, 
	    ftnlen, ftnlen), s_wsfe(cilist *), e_wsfe(void);

    /* Local variables */
    static logical clefpend;
    extern /* Subroutine */ int allparts_(char *, integer *, ftnlen);
    static integer j, k, ivq, idxb, idxl, idxm, idxn;
    extern /* Subroutine */ int zapbl_(char *, integer *, ftnlen);
    static integer iposc0;
    extern integer lenstr_(char *, integer *, ftnlen);
    static integer lenline;

    /* Fortran I/O blocks */
    static cilist io___188 = { 0, 10, 0, "(a)", 0 };
    static cilist io___197 = { 0, 10, 0, "(a)", 0 };
    static cilist io___200 = { 0, 0, 0, "(a)", 0 };
    static cilist io___201 = { 0, 0, 0, "(a)", 0 };
    static cilist io___202 = { 0, 10, 1, "(a)", 0 };



/*  Assume that line has just been read. No need to change iccount since we only */
/*  process full lines. */

#line 1380 ""
    *goto999 = FALSE_;
#line 1381 ""
L1:
#line 1381 ""
    if (*(unsigned char *)line != '%') {
#line 1381 ""
	return 0;
#line 1381 ""
    }

/*  If here, line has some sort of comment */

#line 1385 ""
    if (*(unsigned char *)&line[1] == '%') {
#line 1386 ""
	if (! all_1.insetup) {

/*  Suck up a line, then flow out of "if" block to get another and loop */

#line 1390 ""
	    s_rsfe(&io___188);
#line 1390 ""
	    do_fio(&c__1, line, (ftnlen)128);
#line 1390 ""
	    e_rsfe();
/* ++VV */

/*  UNLESS (a) it has a score-only "M" and changes # of inst's. */

#line 1396 ""
	    if (i_indx(line, "M", (ftnlen)128, (ftnlen)1) > 0) {
#line 1397 ""
		idxl = i_indx(line, "L", (ftnlen)128, (ftnlen)1);
#line 1398 ""
		idxm = i_indx(line, "M", (ftnlen)128, (ftnlen)1);
#line 1399 ""
		idxn = i_indx(line, "n", (ftnlen)128, (ftnlen)1);
#line 1400 ""
		idxb = i_indx(line, " ", (ftnlen)128, (ftnlen)1);
#line 1401 ""
		if (idxl < idxm && idxm < idxn && (idxb == 0 || idxn < idxb)) 
			{
#line 1403 ""
		    i__1 = idxn;
#line 1403 ""
		    all_1.noinow = *(unsigned char *)&line[i__1] - 48;
#line 1404 ""
		    clefpend = TRUE_;

/*  Next noinow digits are original inst. #'s of new inst. set.  Next noinow */
/*  char's after that are clefs */

#line 1409 ""
		    all_1.nvnow = 0;
#line 1410 ""
		    i__1 = all_1.noinow;
#line 1410 ""
		    for (j = 1; j <= i__1; ++j) {
#line 1411 ""
			i__2 = idxn + 1 + j - 1;
#line 1411 ""
			all_1.iorig[j - 1] = *(unsigned char *)&line[i__2] - 
				48;
#line 1412 ""
			iposc0 = idxn + 1 + all_1.noinow;
#line 1413 ""
			i__2 = all_1.nvi[all_1.iorig[j - 1] - 1];
#line 1413 ""
			for (k = 1; k <= i__2; ++k) {
#line 1414 ""
			    ++all_1.nvnow;
/*                  clefq(nvnow) = line(iposc0+nvnow:iposc0+nvnow) */
#line 1416 ""
			    all_1.instnum[all_1.nvnow - 1] = j;
#line 1417 ""
			    all_1.botv[all_1.nvnow - 1] = k == 1 && j != 1;
#line 1418 ""
/* L25: */
#line 1418 ""
			}
#line 1419 ""
/* L24: */
#line 1419 ""
		    }
#line 1420 ""
		}
#line 1421 ""
	    }

/*  or if it's "h" or "l", need to suck up one more line */

#line 1425 ""
	    if (*(unsigned char *)line == 'h' && i_indx("+- ", line + 1, (
		    ftnlen)3, (ftnlen)1) > 0 || *(unsigned char *)line == 'T' 
		    || s_cmp(line, "l ", (ftnlen)2, (ftnlen)2) == 0) {
#line 1425 ""
		s_rsfe(&io___197);
#line 1425 ""
		do_fio(&c__1, line, (ftnlen)128);
#line 1425 ""
		e_rsfe();
#line 1425 ""
	    }

/*  4/29/00 check for T string also */

#line 1431 ""
	} else {

/*  In setup mode. Set flag, flow out and do use following line */

#line 1435 ""
	    all_1.replacing = TRUE_;
#line 1436 ""
	}
#line 1437 ""
    } else if (*(unsigned char *)&line[1] == '!') {

/*  Copy to all parts */

#line 1441 ""
	allparts_(line + 2, &c__125, (ftnlen)126);
#line 1442 ""
    } else {

/*  Get value of hex integer 1,2,...,9,a,b,c in 2nd position, zero otherwise */
/* c  Get value of extended hex integer 1,2,...,9,a,b,c,...,o in 2nd position, zero otherwise */

#line 1447 ""
	ivq = i_indx("123456789abcdefghijklmno", line + 1, (ftnlen)24, (
		ftnlen)1);

/*  Only treat as part-specific pmx line if number .le. noinst */

#line 1451 ""
	if (ivq < 1 || ivq > all_1.noinst) {

/*  Simple comment. */

#line 1455 ""
	    allparts_(line, &c__128, (ftnlen)128);
#line 1456 ""
	} else {

/*  Instrument comment, copy only to part */

#line 1460 ""
	    lenline = lenstr_(line, &c__128, (ftnlen)128);
#line 1461 ""
	    if (lenline > 2) {
#line 1462 ""
		io___200.ciunit = ivq + 10;
#line 1462 ""
		s_wsfe(&io___200);
#line 1462 ""
		do_fio(&c__1, line + 2, lenline - 2);
#line 1462 ""
		e_wsfe();
#line 1463 ""
	    } else {

/*  Transferring blank line */

#line 1467 ""
		io___201.ciunit = ivq + 10;
#line 1467 ""
		s_wsfe(&io___201);
#line 1467 ""
		do_fio(&c__1, " ", (ftnlen)1);
#line 1467 ""
		e_wsfe();
#line 1468 ""
	    }
#line 1469 ""
	}
#line 1470 ""
    }
#line 1471 ""
    i__1 = s_rsfe(&io___202);
#line 1471 ""
    if (i__1 != 0) {
#line 1471 ""
	goto L2;
#line 1471 ""
    }
#line 1471 ""
    i__1 = do_fio(&c__1, line, (ftnlen)128);
#line 1471 ""
    if (i__1 != 0) {
#line 1471 ""
	goto L2;
#line 1471 ""
    }
#line 1471 ""
    i__1 = e_rsfe();
#line 1471 ""
    if (i__1 != 0) {
#line 1471 ""
	goto L2;
#line 1471 ""
    }
#line 1472 ""
    zapbl_(line, &c__128, (ftnlen)128);
#line 1473 ""
    goto L1;
#line 1474 ""
L2:
#line 1475 ""
    *goto999 = TRUE_;
#line 1476 ""
    return 0;
} /* chkcom_ */

/* Subroutine */ int dosetup_(integer *iv, char *line, integer *mtrnum, 
	integer *mtrden, ftnlen line_len)
{
    /* System generated locals */
    integer i__1, i__2;

    /* Builtin functions */
    integer s_wsfe(cilist *), do_fio(integer *, char *, ftnlen), e_wsfe(void),
	     i_nint(real *);

    /* Local variables */
    static integer ioi, noi, iset;
    static real xdata;
    static integer iccount;
    extern /* Subroutine */ int partnum_(integer *, integer *, char *, real *,
	     ftnlen);

    /* Fortran I/O blocks */
    static cilist io___206 = { 0, 40, 0, "(i5)", 0 };
    static cilist io___208 = { 0, 40, 0, "(i5)", 0 };
    static cilist io___210 = { 0, 40, 0, "(i5)", 0 };
    static cilist io___211 = { 0, 40, 0, "(i5)", 0 };
    static cilist io___212 = { 0, 40, 0, "(i5)", 0 };
    static cilist io___213 = { 0, 40, 0, "(i5)", 0 };
    static cilist io___214 = { 0, 40, 0, "(f5.2)", 0 };



/*  Transfers setup data from scratch file to real one for one instrument */
/*  Data may be mixed with comments, but on entry 1st item is a number. */
/*  Write a comment when encountered, as it comes. */
/*  Write numbers one per line. */
/*  Three input data require special handling: */
/*    If not already replaced, i.e., if negative, then */
/*      iset(1) (nv) will be replaced with nvi(i) */
/*      iset(9) (npages) will be replaced with (nsyst-1)/12+1 */
/*    iset(2), if negative, will be followed by extra numbers to be transf. */

#line 1495 ""
    iccount = 0;
#line 1496 ""
    for (iset = 1; iset <= 12; ++iset) {
#line 1497 ""
	partnum_(iv, &iccount, line, &xdata, (ftnlen)128);
#line 1498 ""
	if (iset == 2) {
#line 1499 ""
	    if (xdata > 0.f) {
#line 1500 ""
		s_wsfe(&io___206);
#line 1500 ""
		i__1 = (integer) (xdata + .1f);
#line 1500 ""
		do_fio(&c__1, (char *)&i__1, (ftnlen)sizeof(integer));
#line 1500 ""
		e_wsfe();
#line 1501 ""
	    } else {
#line 1502 ""
		noi = -xdata + .1f;
#line 1503 ""
		s_wsfe(&io___208);
#line 1503 ""
		do_fio(&c__1, (char *)&noi, (ftnlen)sizeof(integer));
#line 1503 ""
		e_wsfe();
#line 1504 ""
		i__1 = noi;
#line 1504 ""
		for (ioi = 1; ioi <= i__1; ++ioi) {
#line 1505 ""
		    partnum_(iv, &iccount, line, &xdata, (ftnlen)128);
#line 1506 ""
		    s_wsfe(&io___210);
#line 1506 ""
		    i__2 = (integer) (xdata + .1f);
#line 1506 ""
		    do_fio(&c__1, (char *)&i__2, (ftnlen)sizeof(integer));
#line 1506 ""
		    e_wsfe();
#line 1507 ""
/* L2: */
#line 1507 ""
		}
#line 1508 ""
	    }
/*        else if (iset.ne.8 .and. xdata.lt.0) then */
#line 1510 ""
	} else if (iset != 8 && iset != 5 && xdata < 0.f) {

/*  Must be either nv or npages */

#line 1514 ""
	    if ((integer) (-xdata + .1f) == 999) {

/*  It's nv */

#line 1518 ""
		s_wsfe(&io___211);
#line 1518 ""
		do_fio(&c__1, (char *)&all_1.nvi[*iv - 1], (ftnlen)sizeof(
			integer));
#line 1518 ""
		e_wsfe();
#line 1519 ""
	    } else {

/*  npages must be computed */

#line 1523 ""
		s_wsfe(&io___212);
#line 1523 ""
		i__1 = (all_1.nsyst - 1) / 12 + 1;
#line 1523 ""
		do_fio(&c__1, (char *)&i__1, (ftnlen)sizeof(integer));
#line 1523 ""
		e_wsfe();
#line 1524 ""
	    }
#line 1525 ""
	} else if (iset != 7 && iset != 12) {

/*  write integer */

#line 1529 ""
	    s_wsfe(&io___213);
#line 1529 ""
	    i__1 = i_nint(&xdata);
#line 1529 ""
	    do_fio(&c__1, (char *)&i__1, (ftnlen)sizeof(integer));
#line 1529 ""
	    e_wsfe();
#line 1530 ""
	} else {

/*  write floating number */

#line 1534 ""
	    s_wsfe(&io___214);
#line 1534 ""
	    do_fio(&c__1, (char *)&xdata, (ftnlen)sizeof(real));
#line 1534 ""
	    e_wsfe();
#line 1535 ""
	}
#line 1536 ""
	if (iset == 3) {
#line 1537 ""
	    *mtrnum = i_nint(&xdata);
#line 1538 ""
	} else if (iset == 4) {
#line 1539 ""
	    *mtrden = i_nint(&xdata);
#line 1540 ""
	}
#line 1541 ""
/* L1: */
#line 1541 ""
    }
#line 1542 ""
    return 0;
} /* dosetup_ */

/* Subroutine */ int partnum_(integer *iv, integer *iccount, char *line, real 
	*xdata, ftnlen line_len)
{
    /* System generated locals */
    address a__1[3];
    integer i__1[3];
    char ch__1[27], ch__2[6], ch__3[1];
    icilist ici__1;

    /* Builtin functions */
    integer s_rsfe(cilist *), do_fio(integer *, char *, ftnlen), e_rsfe(void),
	     s_wsfe(cilist *), e_wsfe(void), i_indx(char *, char *, ftnlen, 
	    ftnlen), s_wsle(cilist *);
    /* Subroutine */ int s_cat(char *, char **, integer *, integer *, ftnlen);
    integer do_lio(integer *, integer *, char *, ftnlen), e_wsle(void);
    /* Subroutine */ int s_stop(char *, ftnlen);
    integer s_rsfi(icilist *), e_rsfi(void);

    /* Local variables */
    static integer i1, i2, icf, len;
    static char durq[1];
    extern integer lenstr_(char *, integer *, ftnlen);
    extern /* Subroutine */ int getchar_(char *, integer *, char *, ftnlen, 
	    ftnlen);

    /* Fortran I/O blocks */
    static cilist io___215 = { 0, 0, 0, "(a)", 0 };
    static cilist io___217 = { 0, 40, 0, "(a)", 0 };
    static cilist io___221 = { 0, 6, 0, 0, 0 };



/*  Simplified number parsing.  Only looks for comment lines and numbers. */

#line 1550 ""
L2:
#line 1550 ""
    if (*iccount == 128) {
#line 1551 ""
	io___215.ciunit = *iv + 10;
#line 1551 ""
	s_rsfe(&io___215);
#line 1551 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 1551 ""
	e_rsfe();
#line 1552 ""
	if (*(unsigned char *)line == '%') {
#line 1553 ""
	    len = lenstr_(line, &c__128, (ftnlen)128);
#line 1554 ""
	    s_wsfe(&io___217);
#line 1554 ""
	    do_fio(&c__1, line, len);
#line 1554 ""
	    e_wsfe();
#line 1555 ""
	    goto L2;
#line 1556 ""
	}
#line 1557 ""
	*iccount = 0;
#line 1558 ""
    }
#line 1559 ""
    ++(*iccount);

/*  Find next non-blank or end of line */

#line 1563 ""
    for (*iccount = *iccount; *iccount <= 127; ++(*iccount)) {
#line 1564 ""
	if (*(unsigned char *)&line[*iccount - 1] != ' ') {
#line 1564 ""
	    goto L3;
#line 1564 ""
	}
#line 1565 ""
/* L4: */
#line 1565 ""
    }

/*  If here, iccount=128 and need to get a new line */

#line 1569 ""
    goto L2;
#line 1570 ""
L3:

/*  iccount now points to start of number to read */

#line 1574 ""
    i1 = *iccount;
#line 1575 ""
L5:
#line 1575 ""
    getchar_(line, iccount, durq, (ftnlen)128, (ftnlen)1);

/*  Remember that getchar first increments iccount, *then* reads a character. */

#line 1579 ""
    if (i_indx("0123456789.-", durq, (ftnlen)12, (ftnlen)1) > 0) {
#line 1579 ""
	goto L5;
#line 1579 ""
    }
#line 1580 ""
    i2 = *iccount - 1;
#line 1581 ""
    if (i2 < i1) {
#line 1582 ""
	s_wsle(&io___221);
/* Writing concatenation */
#line 1582 ""
	i__1[0] = 7, a__1[0] = "Found \"";
#line 1582 ""
	i__1[1] = 1, a__1[1] = durq;
#line 1582 ""
	i__1[2] = 19, a__1[2] = "\" instead of number";
#line 1582 ""
	s_cat(ch__1, a__1, i__1, &c__3, (ftnlen)27);
#line 1582 ""
	do_lio(&c__9, &c__1, ch__1, (ftnlen)27);
#line 1582 ""
	e_wsle();
#line 1583 ""
	s_stop("1", (ftnlen)1);
#line 1584 ""
    }
#line 1585 ""
    icf = i2 - i1 + 49;
#line 1586 ""
    ici__1.icierr = 0;
#line 1586 ""
    ici__1.iciend = 0;
#line 1586 ""
    ici__1.icirnum = 1;
#line 1586 ""
    ici__1.icirlen = i2 - (i1 - 1);
#line 1586 ""
    ici__1.iciunit = line + (i1 - 1);
/* Writing concatenation */
#line 1586 ""
    i__1[0] = 2, a__1[0] = "(f";
#line 1586 ""
    *(unsigned char *)&ch__3[0] = icf;
#line 1586 ""
    i__1[1] = 1, a__1[1] = ch__3;
#line 1586 ""
    i__1[2] = 3, a__1[2] = ".0)";
#line 1586 ""
    ici__1.icifmt = (s_cat(ch__2, a__1, i__1, &c__3, (ftnlen)6), ch__2);
#line 1586 ""
    s_rsfi(&ici__1);
#line 1586 ""
    do_fio(&c__1, (char *)&(*xdata), (ftnlen)sizeof(real));
#line 1586 ""
    e_rsfi();
#line 1587 ""
    return 0;
} /* partnum_ */


/* Subroutine */ int zapbl_(char *string, integer *len, ftnlen string_len)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    /* Subroutine */ int s_copy(char *, char *, ftnlen, ftnlen);

    /* Local variables */
    static integer i__;

#line 1599 ""
    i__1 = *len;
#line 1599 ""
    for (i__ = 1; i__ <= i__1; ++i__) {
#line 1600 ""
	if (*(unsigned char *)&string[i__ - 1] == ' ') {
#line 1600 ""
	    goto L1;
#line 1600 ""
	}
#line 1601 ""
	if (i__ == 1) {
#line 1601 ""
	    return 0;
#line 1601 ""
	}
#line 1602 ""
	goto L2;
#line 1603 ""
L1:
#line 1603 ""
	;
#line 1603 ""
    }

/*  If line is all blank, leave it alone */

#line 1607 ""
    return 0;
#line 1608 ""
L2:
#line 1609 ""
    s_copy(string, string + (i__ - 1), string_len, *len - (i__ - 1));
#line 1610 ""
    return 0;
} /* zapbl_ */

/* Subroutine */ int oddquotesbefore_(char *lineq, integer *indx, logical *
	yesodd, ftnlen lineq_len)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i__, numdqs;


/*  This counts number of double quotes in lineq up to position indx-1, then */
/*    sets yesodd according to whether number is odd or even */

#line 1619 ""
    numdqs = 0;
#line 1620 ""
    i__1 = *indx - 1;
#line 1620 ""
    for (i__ = 1; i__ <= i__1; ++i__) {
#line 1621 ""
	if (*(unsigned char *)&lineq[i__ - 1] == '"') {
#line 1621 ""
	    ++numdqs;
#line 1621 ""
	}
#line 1622 ""
/* L1: */
#line 1622 ""
    }
#line 1623 ""
    *yesodd = numdqs % 2 == 1;
#line 1624 ""
    return 0;
} /* oddquotesbefore_ */

