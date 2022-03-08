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
/* c  scor2prt 1/29/21 for PMX 2.94 */
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

/* 1/29/21 */
/*   Reformat output statement for warning about full bar rests to */
/*     eliminate use of H format specifier */
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


#line 107 ""
    clefpend = FALSE_;
#line 108 ""
    all_1.insetup = TRUE_;
#line 109 ""
    all_1.replacing = FALSE_;
#line 110 ""
    frstln = TRUE_;
#line 111 ""
    lenhold = 0;
#line 112 ""
    *(unsigned char *)sq = '\\';
#line 113 ""
    s_wsle(&io___6);
#line 113 ""
    do_lio(&c__9, &c__1, "This is scor2prt for PMX 2.94, 29 January 2021", (
	    ftnlen)46);
#line 113 ""
    e_wsle();
#line 114 ""
    numargs = iargc_();
#line 115 ""
    if (numargs == 0) {
#line 116 ""
	s_wsle(&io___8);
#line 116 ""
	do_lio(&c__9, &c__1, "You could have entered a jobname on the comman"\
		"d line,", (ftnlen)53);
#line 116 ""
	e_wsle();
#line 117 ""
	s_wsle(&io___9);
#line 117 ""
	do_lio(&c__9, &c__1, "      but you may enter one now:", (ftnlen)32);
#line 117 ""
	e_wsle();
#line 118 ""
	s_rsfe(&io___10);
#line 118 ""
	do_fio(&c__1, jobname, (ftnlen)27);
#line 118 ""
	e_rsfe();
#line 119 ""
	numargs = 1;
#line 120 ""
    } else {
/*       call getarg(1,jobname,idum) ! May need to replace this w/ next line */
#line 122 ""
	getarg_(&c__1, jobname, (ftnlen)27);
#line 123 ""
    }
#line 124 ""
    ljob = lenstr_(jobname, &c__27, (ftnlen)27);
#line 125 ""
    if (ljob == 0) {
#line 126 ""
	s_wsle(&io___13);
#line 126 ""
	do_lio(&c__9, &c__1, "No was jobname entered. Restart and try again.",
		 (ftnlen)46);
#line 126 ""
	e_wsle();
#line 127 ""
	s_stop("", (ftnlen)0);
#line 128 ""
    }

/*  Strip ".pmx" if necessary */

/* Computing MAX */
#line 132 ""
    i__1 = i_indx(jobname, ".pmx", (ftnlen)27, (ftnlen)4), i__2 = i_indx(
	    jobname, ".PMX", (ftnlen)27, (ftnlen)4);
#line 132 ""
    ndxpmx = max(i__1,i__2);
#line 133 ""
    if (ndxpmx > 0) {
#line 134 ""
	s_copy(jobname, jobname, (ftnlen)27, ndxpmx - 1);
#line 135 ""
	ljob += -4;
#line 136 ""
    }

/*  Check for existence of input file */

/* Writing concatenation */
#line 140 ""
    i__3[0] = ljob, a__1[0] = jobname;
#line 140 ""
    i__3[1] = 4, a__1[1] = ".pmx";
#line 140 ""
    s_cat(infileq, a__1, i__3, &c__2, (ftnlen)27);
#line 141 ""
    ioin__1.inerr = 0;
#line 141 ""
    ioin__1.infilen = 27;
#line 141 ""
    ioin__1.infile = infileq;
#line 141 ""
    ioin__1.inex = &fexist;
#line 141 ""
    ioin__1.inopen = 0;
#line 141 ""
    ioin__1.innum = 0;
#line 141 ""
    ioin__1.innamed = 0;
#line 141 ""
    ioin__1.inname = 0;
#line 141 ""
    ioin__1.inacc = 0;
#line 141 ""
    ioin__1.inseq = 0;
#line 141 ""
    ioin__1.indir = 0;
#line 141 ""
    ioin__1.infmt = 0;
#line 141 ""
    ioin__1.inform = 0;
#line 141 ""
    ioin__1.inunf = 0;
#line 141 ""
    ioin__1.inrecl = 0;
#line 141 ""
    ioin__1.innrec = 0;
#line 141 ""
    ioin__1.inblank = 0;
#line 141 ""
    f_inqu(&ioin__1);
#line 142 ""
    if (! fexist) {
#line 143 ""
	s_wsle(&io___17);
/* Writing concatenation */
#line 143 ""
	i__3[0] = 17, a__1[0] = "Cannot find file ";
#line 143 ""
	i__3[1] = 27, a__1[1] = infileq;
#line 143 ""
	s_cat(ch__1, a__1, i__3, &c__2, (ftnlen)44);
#line 143 ""
	do_lio(&c__9, &c__1, ch__1, (ftnlen)44);
#line 143 ""
	e_wsle();
#line 144 ""
	s_stop("", (ftnlen)0);
#line 145 ""
    }
#line 146 ""
    o__1.oerr = 0;
#line 146 ""
    o__1.ounit = 10;
#line 146 ""
    o__1.ofnmlen = ljob + 4;
/* Writing concatenation */
#line 146 ""
    i__3[0] = ljob, a__1[0] = jobname;
#line 146 ""
    i__3[1] = 4, a__1[1] = ".pmx";
#line 146 ""
    s_cat(ch__2, a__1, i__3, &c__2, (ftnlen)31);
#line 146 ""
    o__1.ofnm = ch__2;
#line 146 ""
    o__1.orl = 0;
#line 146 ""
    o__1.osta = 0;
#line 146 ""
    o__1.oacc = 0;
#line 146 ""
    o__1.ofm = 0;
#line 146 ""
    o__1.oblnk = 0;
#line 146 ""
    f_open(&o__1);

/* Open all instrument files now for allparts stuff.  Later disgard those >nv */

#line 150 ""
    for (iv = 1; iv <= 24; ++iv) {
#line 151 ""
	all_1.iorig[iv - 1] = iv;
#line 152 ""
	o__1.oerr = 0;
#line 152 ""
	o__1.ounit = iv + 10;
#line 152 ""
	o__1.ofnm = 0;
#line 152 ""
	o__1.orl = 0;
#line 152 ""
	o__1.osta = "SCRATCH";
#line 152 ""
	o__1.oacc = 0;
#line 152 ""
	o__1.ofm = 0;
#line 152 ""
	o__1.oblnk = 0;
#line 152 ""
	f_open(&o__1);
#line 153 ""
	ludpfn[iv - 1] = 0;
#line 154 ""
/* L19: */
#line 154 ""
    }
#line 155 ""
    s_rsfe(&io___20);
#line 155 ""
    do_fio(&c__1, line, (ftnlen)128);
#line 155 ""
    e_rsfe();
#line 156 ""
    chkcom_(line, &goto999, (ftnlen)128);
#line 157 ""
    if (s_cmp(line, "---", (ftnlen)3, (ftnlen)3) == 0) {
#line 158 ""
	allparts_(line, &c__128, (ftnlen)128);
#line 159 ""
L31:
#line 159 ""
	s_rsfe(&io___23);
#line 159 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 159 ""
	e_rsfe();
#line 160 ""
	if (s_cmp(line, "---", (ftnlen)3, (ftnlen)3) != 0) {
#line 161 ""
	    allparts_(line, &c__128, (ftnlen)128);
#line 162 ""
	    goto L31;
#line 163 ""
	}
#line 164 ""
	allparts_(line, &c__128, (ftnlen)128);
#line 165 ""
	s_rsfe(&io___24);
#line 165 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 165 ""
	e_rsfe();
#line 166 ""
	chkcom_(line, &goto999, (ftnlen)128);
#line 167 ""
    }
#line 168 ""
    iccount = 0;
#line 169 ""
    nv = readin_(line, &iccount, &c__1, (ftnlen)128) + .1f;
#line 170 ""
    all_1.noinst = readin_(line, &iccount, &c__2, (ftnlen)128) + .1f;
#line 171 ""
    if (all_1.noinst > 0) {
#line 172 ""
	all_1.nvi[0] = nv - all_1.noinst + 1;
#line 173 ""
    } else {
#line 174 ""
	all_1.noinst = 1 - all_1.noinst;
#line 175 ""
	i__1 = all_1.noinst;
#line 175 ""
	for (iinst = 1; iinst <= i__1; ++iinst) {
#line 176 ""
	    all_1.nvi[iinst - 1] = readin_(line, &iccount, &c_n1, (ftnlen)128)
		     + .1f;
#line 177 ""
/* L21: */
#line 177 ""
	}
#line 178 ""
    }
#line 179 ""
    all_1.noinow = all_1.noinst;
#line 180 ""
    insnow = 1;

/*  ivlast is last iv in current inst.  instnum(iv) is iinst for current voice. */

#line 184 ""
    ivlast = all_1.nvi[0];
#line 185 ""
    i__1 = nv;
#line 185 ""
    for (iv = 1; iv <= i__1; ++iv) {
#line 186 ""
	all_1.instnum[iv - 1] = insnow;
#line 187 ""
	if (iv == ivlast) {
#line 188 ""
	    if (iv < nv) {
#line 188 ""
		all_1.botv[iv] = TRUE_;
#line 188 ""
	    }

/*  The previous stmt will set botv true only for bot voice of iinst>1.  It is */
/*  used when writing termrpts, but the one in voice one is handled differently, */
/*  so botv(1) is left .false. */

#line 194 ""
	    if (insnow < all_1.noinst) {
#line 195 ""
		++insnow;
#line 196 ""
		ivlast += all_1.nvi[insnow - 1];
#line 197 ""
	    }
#line 198 ""
	}
#line 199 ""
/* L22: */
#line 199 ""
    }
#line 200 ""
    mtrnuml = readin_(line, &iccount, &c__0, (ftnlen)128) + .1f;
#line 201 ""
    mtrdenl = readin_(line, &iccount, &c__0, (ftnlen)128) + .1f;
#line 202 ""
    mtrnmp = readin_(line, &iccount, &c__0, (ftnlen)128) + .1f;
#line 203 ""
    mtrdnp = readin_(line, &iccount, &c__0, (ftnlen)128) + .1f;
#line 204 ""
    xmtrnum0 = readin_(line, &iccount, &c__0, (ftnlen)128);
#line 205 ""
    isig = readin_(line, &iccount, &c__0, (ftnlen)128) + .1f;
#line 206 ""
    npages = readin_(line, &iccount, &c__3, (ftnlen)128) + .1f;
#line 207 ""
    all_1.nsyst = readin_(line, &iccount, &c__0, (ftnlen)128) + .1f;
#line 208 ""
    musicsize = readin_(line, &iccount, &c__4, (ftnlen)128) + .1f;
#line 209 ""
    fracindent = readin_(line, &iccount, &c__5, (ftnlen)128);
#line 210 ""
    if (npages == 0) {
#line 211 ""
	s_wsle(&io___39);
#line 211 ""
	do_lio(&c__9, &c__1, "You entered npages=0, which means nsyst is not"\
		" the total number", (ftnlen)63);
#line 211 ""
	e_wsle();
#line 213 ""
	s_wsle(&io___40);
#line 213 ""
	do_lio(&c__9, &c__1, "of systems.  Scor2prt has to know the total nu"\
		"mber of systems.", (ftnlen)62);
#line 213 ""
	e_wsle();
#line 215 ""
	s_wsle(&io___41);
#line 215 ""
	do_lio(&c__9, &c__1, "Please set npages and nsyst to their real valu"\
		"es.", (ftnlen)49);
#line 215 ""
	e_wsle();
#line 217 ""
	s_stop("", (ftnlen)0);
#line 218 ""
    }

/*  Must leave insetup=.true. else could bypass ALL instrument names. */

#line 222 ""
    s_rsfe(&io___42);
#line 222 ""
    do_fio(&c__1, line, (ftnlen)128);
#line 222 ""
    e_rsfe();
#line 223 ""
    chkcom_(line, &goto999, (ftnlen)128);
#line 224 ""
    al__1.aerr = 0;
#line 224 ""
    al__1.aunit = 10;
#line 224 ""
    f_back(&al__1);

/*  Normally this puts pointer at start of line with 1st inst name */
/*  Check if prior line was "%%" */

#line 229 ""
    al__1.aerr = 0;
#line 229 ""
    al__1.aunit = 10;
#line 229 ""
    f_back(&al__1);
#line 230 ""
    s_rsfe(&io___43);
#line 230 ""
    do_fio(&c__1, line, (ftnlen)128);
#line 230 ""
    e_rsfe();
#line 231 ""
    if (s_cmp(line, "%%", (ftnlen)2, (ftnlen)2) == 0) {
#line 231 ""
	al__1.aerr = 0;
#line 231 ""
	al__1.aunit = 10;
#line 231 ""
	f_back(&al__1);
#line 231 ""
    }
#line 232 ""
    i__1 = all_1.noinst;
#line 232 ""
    for (iv = 1; iv <= i__1; ++iv) {
#line 233 ""
	gotname = FALSE_;
#line 234 ""
L16:
#line 234 ""
	s_rsfe(&io___45);
#line 234 ""
	do_fio(&c__1, instrum + (iv - 1 << 7), (ftnlen)128);
#line 234 ""
	e_rsfe();
#line 235 ""
	if (s_cmp(instrum + (iv - 1 << 7), "%%", (ftnlen)2, (ftnlen)2) == 0) {
#line 236 ""
	    s_rsfe(&io___47);
#line 236 ""
	    do_fio(&c__1, line, (ftnlen)128);
#line 236 ""
	    e_rsfe();
#line 237 ""
	    goto L16;
#line 238 ""
	} else if (*(unsigned char *)&instrum[(iv - 1) * 128] == '%') {
#line 239 ""
	    ivq = *(unsigned char *)&instrum[(iv - 1 << 7) + 1] - 48;
#line 240 ""
	    if (ivq != iv) {

/*  It's really a comment.  Copy to parts, then get another trial name. */

#line 244 ""
		allparts_(instrum + (iv - 1 << 7), &c__128, (ftnlen)128);
#line 245 ""
		goto L16;
#line 246 ""
	    } else {
#line 247 ""
		s_copy(line, instrum + ((iv - 1 << 7) + 2), (ftnlen)128, (
			ftnlen)126);
#line 248 ""
		s_copy(instrum + (iv - 1 << 7), line, (ftnlen)128, (ftnlen)
			128);
#line 249 ""
		gotname = TRUE_;
#line 250 ""
	    }
#line 251 ""
	} else {
#line 252 ""
	    gotname = TRUE_;
#line 253 ""
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
#line 268 ""
	if (! gotname) {
#line 269 ""
	    s_wsle(&io___49);
#line 269 ""
	    do_lio(&c__9, &c__1, "You must provide a replacement instrument "\
		    "name", (ftnlen)46);
#line 269 ""
	    e_wsle();
#line 270 ""
	    s_stop("", (ftnlen)0);
#line 271 ""
	}
#line 272 ""
	io___50.ciunit = iv + 10;
#line 272 ""
	s_wsfe(&io___50);
#line 272 ""
	do_fio(&c__1, " ", (ftnlen)1);
#line 272 ""
	e_wsfe();
#line 273 ""
/* L14: */
#line 273 ""
    }
#line 274 ""
    all_1.replacing = FALSE_;
#line 275 ""
    all_1.nvnow = nv;

/*  Clef string:  Note insetup is still T, so "%%" will be treated specially */

#line 279 ""
    s_rsfe(&io___51);
#line 279 ""
    do_fio(&c__1, line, (ftnlen)128);
#line 279 ""
    e_rsfe();
#line 280 ""
    chkcom_(line, &goto999, (ftnlen)128);
#line 281 ""
    if (all_1.replacing) {

/*  If here, we have next line after "%%", containing score's clef string */
/*  Assume all clefs are handled with instrument comments. */

#line 286 ""
	s_rsfe(&io___52);
#line 286 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 286 ""
	e_rsfe();
#line 287 ""
	chkcom_(line, &goto999, (ftnlen)128);
#line 288 ""
	al__1.aerr = 0;
#line 288 ""
	al__1.aunit = 10;
#line 288 ""
	f_back(&al__1);
#line 289 ""
    } else {

/*  If here, line has the clef string in it.  Handle the old way */

#line 293 ""
	kvstart = 1;
#line 294 ""
	kvend = all_1.nvi[0];
#line 295 ""
	i__1 = all_1.noinst;
#line 295 ""
	for (inst = 1; inst <= i__1; ++inst) {
#line 296 ""
	    ci__1.cierr = 0;
#line 296 ""
	    ci__1.ciunit = inst + 10;
/* Writing concatenation */
#line 296 ""
	    i__4[0] = 2, a__2[0] = "(a";
#line 296 ""
	    *(unsigned char *)&ch__4[0] = all_1.nvi[inst - 1] + 48;
#line 296 ""
	    i__4[1] = 1, a__2[1] = ch__4;
#line 296 ""
	    i__4[2] = 1, a__2[2] = ")";
#line 296 ""
	    ci__1.cifmt = (s_cat(ch__3, a__2, i__4, &c__3, (ftnlen)4), ch__3);
#line 296 ""
	    s_wsfe(&ci__1);
#line 296 ""
	    do_fio(&c__1, line + (kvstart - 1), kvend - (kvstart - 1));
#line 296 ""
	    e_wsfe();
#line 298 ""
	    if (inst < all_1.noinst) {
#line 299 ""
		kvstart = kvend + 1;
#line 300 ""
		kvend = kvstart + all_1.nvi[inst] - 1;
#line 301 ""
	    }
#line 302 ""
/* L2: */
#line 302 ""
	}
#line 303 ""
    }
#line 304 ""
    all_1.replacing = FALSE_;
#line 305 ""
    all_1.insetup = FALSE_;

/*  *****NOTE*****This comment applies to stuff done earlier! */
/*  Before starting the big loop, copy initial instnum and staffnum stuff */
/*  into working values.  Latter may change if noinst changes.  Also make */
/*  list of current inst nums relative to original ones.  In addition to those */
/*  below, must redo instnum(iv) and botv(iv) when we change noinst. */

/*  Path string:  ASSUME THIS WILL NEVER BE ALTERED IN PARTS! */

#line 315 ""
L18:
#line 315 ""
    s_rsfe(&io___56);
#line 315 ""
    do_fio(&c__1, line, (ftnlen)128);
#line 315 ""
    e_rsfe();
#line 316 ""
    if (*(unsigned char *)line == '%') {
#line 317 ""
	allparts_(line, &c__128, (ftnlen)128);
#line 318 ""
	goto L18;
#line 319 ""
    }
#line 320 ""
    allparts_(line, &c__128, (ftnlen)128);

/*  Write instrument names.  Will be blank if later part of a score. */

#line 324 ""
    if (*(unsigned char *)&instrum[0] != ' ') {
#line 325 ""
	i__1 = all_1.noinst;
#line 325 ""
	for (iv = 1; iv <= i__1; ++iv) {
#line 326 ""
	    len = lenstr_(instrum + (iv - 1 << 7), &c__79, (ftnlen)128);
#line 327 ""
	    io___58.ciunit = iv + 10;
#line 327 ""
	    s_wsfe(&io___58);
#line 327 ""
	    do_fio(&c__1, "Ti", (ftnlen)2);
#line 327 ""
	    do_fio(&c__1, instrum + (iv - 1 << 7), len);
#line 327 ""
	    e_wsfe();
#line 328 ""
/* L3: */
#line 328 ""
	}
#line 329 ""
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

#line 341 ""
    iv = 1;
#line 342 ""
    iinst = 1;
#line 343 ""
    termrpt = FALSE_;
#line 344 ""
L4:
#line 345 ""
    i__1 = s_rsfe(&io___60);
#line 345 ""
    if (i__1 != 0) {
#line 345 ""
	goto L999;
#line 345 ""
    }
#line 345 ""
    i__1 = do_fio(&c__1, line, (ftnlen)128);
#line 345 ""
    if (i__1 != 0) {
#line 345 ""
	goto L999;
#line 345 ""
    }
#line 345 ""
    i__1 = e_rsfe();
#line 345 ""
    if (i__1 != 0) {
#line 345 ""
	goto L999;
#line 345 ""
    }
#line 346 ""
    lenline = lenstr_(line, &c__128, (ftnlen)128);
#line 347 ""
    if (lenline == 0) {
#line 347 ""
	goto L4;
#line 347 ""
    }
#line 348 ""
    zapbl_(line, &c__128, (ftnlen)128);
#line 349 ""
    chkcom_(line, &goto999, (ftnlen)128);
#line 350 ""
    if (goto999) {
#line 350 ""
	goto L999;
#line 350 ""
    }
#line 351 ""
    lenline = lenstr_(line, &c__128, (ftnlen)128);
#line 352 ""
    if (lenline == 0) {
#line 352 ""
	goto L4;
#line 352 ""
    }
#line 353 ""
    if (*(unsigned char *)line == 'T') {
#line 354 ""
	allparts_(line, &c__128, (ftnlen)128);
#line 355 ""
	s_rsfe(&io___62);
#line 355 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 355 ""
	e_rsfe();
#line 356 ""
	allparts_(line, &c__128, (ftnlen)128);
#line 357 ""
	goto L4;
#line 358 ""
    } else /* if(complicated condition) */ {
/* Writing concatenation */
#line 358 ""
	i__3[0] = 1, a__1[0] = sq;
#line 358 ""
	i__3[1] = 1, a__1[1] = sq;
#line 358 ""
	s_cat(ch__5, a__1, i__3, &c__2, (ftnlen)2);
#line 358 ""
	if (s_cmp(line, ch__5, (ftnlen)2, (ftnlen)2) == 0) {
#line 359 ""
	    allparts_(line, &c__128, (ftnlen)128);
#line 360 ""
	    goto L4;
#line 361 ""
	} else if (i_indx("hl", line, (ftnlen)2, (ftnlen)1) > 0 && i_indx(
		" +-", line + 1, (ftnlen)3, (ftnlen)1) > 0) {
#line 363 ""
	    allparts_(line, &c__128, (ftnlen)128);
#line 364 ""
	    s_rsfe(&io___63);
#line 364 ""
	    do_fio(&c__1, line, (ftnlen)128);
#line 364 ""
	    e_rsfe();
#line 365 ""
	    allparts_(line, &c__128, (ftnlen)128);
#line 366 ""
	    goto L4;
#line 367 ""
	} else if (iv == 1) {
#line 368 ""
	    for (ia = 1; ia <= 10; ++ia) {
#line 369 ""
L24:
#line 370 ""
		idxa = ntindex_(line, achar + (ia - 1), (ftnlen)128, (ftnlen)
			1);
#line 371 ""
		isachar = idxa > 0;
#line 372 ""
		if (idxa > 1) {
#line 372 ""
		    i__1 = idxa - 2;
#line 372 ""
		    isachar = s_cmp(line + i__1, " ", idxa - 1 - i__1, (
			    ftnlen)1) == 0;
#line 372 ""
		}

/*                   1   2   3   4   5   6   7   8   9   10 */
/*      data achar /'P','m','V','R','A','h','w','K','M','I'/ */

#line 377 ""
		if (ia == 9) {
#line 377 ""
		    i__1 = idxa;
#line 377 ""
		    isachar = isachar && s_cmp(line + i__1, "S", idxa + 1 - 
			    i__1, (ftnlen)1) == 0;
#line 377 ""
		}
#line 379 ""
		if (isachar) {

/*  Check whether character is inside a quoted string by counting */
/*  how many quotes precede it in the line */

#line 385 ""
		    oddquotesbefore_(line, &idxa, &yesodd, (ftnlen)128);
#line 386 ""
		    if (yesodd) {
#line 386 ""
			isachar = FALSE_;
#line 386 ""
		    }
#line 387 ""
		}
#line 388 ""
		if (isachar) {

/*  Find next blank */

#line 392 ""
		    for (ib = idxa + 1; ib <= 128; ++ib) {
#line 393 ""
			if (*(unsigned char *)&line[ib - 1] == ' ') {
#line 393 ""
			    goto L7;
#line 393 ""
			}
#line 394 ""
/* L6: */
#line 394 ""
		    }
#line 395 ""
		    s_wsle(&io___69);
#line 395 ""
		    do_lio(&c__9, &c__1, "Problem with \"V,R,m,P,A,h,MS, o"\
			    "r w\"", (ftnlen)35);
#line 395 ""
		    e_wsle();
#line 396 ""
		    s_wsle(&io___70);
#line 396 ""
		    do_lio(&c__9, &c__1, "Send files to Dr. Don at dsimons a"\
			    "t roadrunner dot com", (ftnlen)54);
#line 396 ""
		    e_wsle();
#line 398 ""
		    s_stop("1", (ftnlen)1);
#line 399 ""
L7:

/*  Next blank is at position ib.  Later, if ia=1, must check for Pc"  " ; */
/*    i.e., look for '"' between P and blank */

#line 404 ""
		    if (ia == 4) {

/*  Check for terminal repeat.  Note if there's a term rpt, there can't be any */
/*  others.  Also, must process repeats LAST, after m's and 'V's */

#line 409 ""
			for (ic = ib + 1; ic <= 128; ++ic) {

/*  If any subsequent character on this line is neither blank nor "/", get out */

#line 413 ""
			    if (i_indx(" /", line + (ic - 1), (ftnlen)2, (
				    ftnlen)1) == 0) {
#line 413 ""
				goto L9;
#line 413 ""
			    }
#line 414 ""
			    if (*(unsigned char *)&line[ic - 1] == '/') {
#line 415 ""
				termrpt = TRUE_;
#line 416 ""
				i__1 = ib - 3;
#line 416 ""
				s_copy(termsym, line + i__1, (ftnlen)2, ib - 
					1 - i__1);

/*  Process the line as if there were no "R" */

#line 420 ""
				goto L10;
#line 421 ""
			    }
#line 422 ""
/* L8: */
#line 422 ""
			}

/* +++ 060812 */
/*  If here, all chars after "R" symbol are blanks, so process the line normally, */
/*    but only IF next line is not the M-Tx line " /" */

#line 428 ""
			s_rsfe(&io___73);
#line 428 ""
			do_fio(&c__1, templine, (ftnlen)128);
#line 428 ""
			e_rsfe();
#line 429 ""
			if (s_cmp(templine, " /", (ftnlen)2, (ftnlen)2) != 0) 
				{
#line 430 ""
			    al__1.aerr = 0;
#line 430 ""
			    al__1.aunit = 10;
#line 430 ""
			    f_back(&al__1);
/* and flow out */
#line 431 ""
			} else {

/*  We have the M-Tx case where one line ends with R? and next is " /". Add / to the line, */
/*    and proceed as above */

/* Writing concatenation */
#line 436 ""
			    i__3[0] = ib, a__1[0] = line;
#line 436 ""
			    i__3[1] = 1, a__1[1] = "/";
#line 436 ""
			    s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 437 ""
			    lenline += 2;
#line 438 ""
			    termrpt = TRUE_;
#line 439 ""
			    i__1 = ib - 3;
#line 439 ""
			    s_copy(termsym, line + i__1, (ftnlen)2, ib - 1 - 
				    i__1);
#line 440 ""
			    goto L10;
#line 441 ""
			}
/* +++ 060812 */

#line 444 ""
		    } else if (ia == 1) {
#line 445 ""
			idxq = ntindex_(line, "\"", (ftnlen)128, (ftnlen)1);
#line 446 ""
			if (idxq > idxa && idxq < ib) {

/*  Quote is between P and next blank.  Find 2nd quote, starting at the blank. */

#line 450 ""
			    idxq2 = ib - 1 + ntindex_(line, "\"", (ftnlen)128,
				     (ftnlen)1);
#line 451 ""
			    i__1 = idxq2;
#line 451 ""
			    if (idxq == 0 || s_cmp(line + i__1, " ", idxq2 + 
				    1 - i__1, (ftnlen)1) != 0) {
#line 452 ""
				s_wsle(&io___77);
#line 452 ""
				e_wsle();
#line 453 ""
				s_wsle(&io___78);
#line 453 ""
				do_lio(&c__9, &c__1, "Error copying P with q"\
					"uotes, idxq2:", (ftnlen)35);
#line 453 ""
				do_lio(&c__3, &c__1, (char *)&idxq2, (ftnlen)
					sizeof(integer));
#line 453 ""
				e_wsle();
#line 454 ""
				s_wsle(&io___79);
#line 454 ""
				do_lio(&c__9, &c__1, line, (ftnlen)60);
#line 454 ""
				e_wsle();
#line 455 ""
				s_stop("1", (ftnlen)1);
#line 456 ""
			    }
#line 457 ""
			    ib = idxq2 + 1;
#line 458 ""
			}

/*  Do not transfer P into parts. */

#line 462 ""
			goto L12;
#line 463 ""
		    } else if (ia == 9) {

/*  Start Saving a macro. After leaving here, a symbol will be sent to all parts, */
/*  If all on this line, set ib to end and exit normally. */

#line 468 ""
			i__1 = ib;
#line 468 ""
			ndxm = i_indx(line + i__1, "M", 128 - i__1, (ftnlen)1)
				;
#line 469 ""
			i__1 = ib + ndxm - 2;
#line 469 ""
			if (ndxm > 0 && s_cmp(line + i__1, " ", ib + ndxm - 1 
				- i__1, (ftnlen)1) == 0) {

/*  Macro ends on this line */

#line 473 ""
			    ib = ib + ndxm + 1;
#line 474 ""
			} else {

/*  Save leading part of current line */

#line 478 ""
			    lenhold = idxa - 1;
#line 479 ""
			    if (lenhold > 0) {
#line 479 ""
				s_copy(holdln, line, (ftnlen)128, lenhold);
#line 479 ""
			    }

/*  Transfer rest of line */

#line 483 ""
			    i__1 = 129 - idxa;
#line 483 ""
			    allparts_(line + (idxa - 1), &i__1, 128 - (idxa - 
				    1));

/*  Read next line */

#line 487 ""
L20:
#line 487 ""
			    s_rsfe(&io___82);
#line 487 ""
			    do_fio(&c__1, line, (ftnlen)128);
#line 487 ""
			    e_rsfe();

/*  Check for comment, transfer and loop if so */

/*                if (line(1:1) .eq.'%') then */
#line 492 ""
L23:
#line 492 ""
			    if (*(unsigned char *)line == '%') {
/*                  call allparts(line,128) */
/*                  go to 20 */
#line 495 ""
				chkcom_(line, &goto999, (ftnlen)128);
#line 496 ""
				goto L23;
#line 497 ""
			    }

/*  Look for terminal ' M' */

#line 501 ""
			    if (*(unsigned char *)line == 'M') {
#line 502 ""
				ndxm = 1;
#line 503 ""
			    } else {
#line 504 ""
				ndxm = i_indx(line, " M", (ftnlen)128, (
					ftnlen)2);
#line 505 ""
				if (ndxm > 0) {
#line 505 ""
				    ++ndxm;
#line 505 ""
				}
#line 506 ""
			    }
#line 507 ""
			    if (ndxm > 0) {

/*  Set parameters, exit normally (but later check for leading part of 1st line */

#line 511 ""
				idxa = 1;
#line 512 ""
				ib = ndxm + 1;
#line 513 ""
			    } else {

/*  No "M", transfer entire line, loop */

#line 517 ""
				allparts_(line, &c__128, (ftnlen)128);
#line 518 ""
				goto L20;
#line 519 ""
			    }
#line 520 ""
			}
#line 521 ""
		    } else if (ia == 10) {

/*  Do not transfer MIDI command into parts */

#line 525 ""
			goto L12;
#line 526 ""
		    } else if (ia == 5) {

/*  First check for "AS", but S may come after other "A" options */

#line 530 ""
			i__1 = idxa;
#line 530 ""
			idxs = i_indx(line + i__1, "S", ib - i__1, (ftnlen)1);
#line 531 ""
			if (idxs > 0) {

/*  Get rid of the string. First check if that's all there is in A. */

#line 535 ""
			    if (ib - idxa == nv + 2) {
#line 535 ""
				goto L12;
#line 535 ""
			    }
#line 536 ""
			    i__1 = idxa + idxs + nv;
/* Writing concatenation */
#line 536 ""
			    i__3[0] = idxa + idxs - 1, a__1[0] = line;
#line 536 ""
			    i__3[1] = ib - i__1, a__1[1] = line + i__1;
#line 536 ""
			    s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 537 ""
			}

/*  Check for user-defined part file name. Must start in column 1 and have */
/*    AN[n]"userfilename". */
/*    idxa is position of "A" */
/*    ib is position of the next blank after "A" */
/*    Don't allow any blanks in user */

#line 545 ""
			i__1 = idxa;
#line 545 ""
			if (s_cmp(line + i__1, "N", idxa + 1 - i__1, (ftnlen)
				1) != 0) {
#line 545 ""
			    goto L9;
#line 545 ""
			}
/* bail out */
#line 546 ""
			if (idxa != 1) {
#line 547 ""
			    s_wsle(&io___84);
#line 547 ""
			    e_wsle();
#line 548 ""
			    s_wsle(&io___85);
#line 548 ""
			    do_lio(&c__9, &c__1, "You entered \"AN...\" some"\
				    "where beyond first column; stopping.", (
				    ftnlen)60);
#line 548 ""
			    e_wsle();
#line 550 ""
			    s_stop("1", (ftnlen)1);
#line 551 ""
			}

/*  pmxa already checked for valid one- or 2-digit number, so get it */

#line 555 ""
			if (*(unsigned char *)&line[3] == '"') {

/*  Single digit instrument number */

#line 559 ""
			    s_rsfi(&io___86);
#line 559 ""
			    do_fio(&c__1, (char *)&iudpfn, (ftnlen)sizeof(
				    integer));
#line 559 ""
			    e_rsfi();
#line 560 ""
			    idxstartname = 5;
#line 561 ""
			} else {
#line 562 ""
			    s_rsfi(&io___89);
#line 562 ""
			    do_fio(&c__1, (char *)&iudpfn, (ftnlen)sizeof(
				    integer));
#line 562 ""
			    e_rsfi();
#line 563 ""
			    idxstartname = 6;
#line 564 ""
			}
#line 565 ""
			ludpfn[iudpfn - 1] = i_indx(line + (idxstartname - 1),
				 "\"", 128 - (idxstartname - 1), (ftnlen)1) - 
				1;
#line 566 ""
			if (ludpfn[iudpfn - 1] < 0) {
#line 567 ""
			    s_wsle(&io___90);
#line 567 ""
			    e_wsle();
#line 568 ""
			    s_wsle(&io___91);
#line 568 ""
			    do_lio(&c__9, &c__1, "User-defined part file nam"\
				    "e must be in quotes", (ftnlen)45);
#line 568 ""
			    e_wsle();
#line 569 ""
			    s_stop("1", (ftnlen)1);
#line 570 ""
			}
#line 571 ""
			idxendname = idxstartname + ludpfn[iudpfn - 1] - 1;
#line 572 ""
			s_copy(udpfnq + (iudpfn - 1 << 7), line + (
				idxstartname - 1), (ftnlen)128, idxendname - (
				idxstartname - 1));

/*  Get a new line! */

#line 576 ""
			goto L4;
#line 577 ""
		    } else if (ia == 8) {

/* Key change/transposition. */
/* If not instrument specific, copy to all parts */

#line 582 ""
			i__1 = idxa;
#line 582 ""
			if (s_cmp(line + i__1, "i", idxa + 1 - i__1, (ftnlen)
				1) != 0) {
#line 582 ""
			    goto L9;
#line 582 ""
			}

/* Instrument-wise key/transposition(s): Ki[nn][+/-][dd}[+/-][dd]... */

#line 586 ""
			ibb = idxa + 2;
/* Starts on digit after 'i' */
#line 587 ""
L40:
#line 588 ""
			ici__1.icierr = 0;
#line 588 ""
			ici__1.iciend = 0;
#line 588 ""
			ici__1.icirnum = 1;
#line 588 ""
			ici__1.icirlen = 1;
#line 588 ""
			ici__1.iciunit = line + (ibb - 1);
#line 588 ""
			ici__1.icifmt = "(i1)";
#line 588 ""
			s_rsfi(&ici__1);
#line 588 ""
			do_fio(&c__1, (char *)&iiinst, (ftnlen)sizeof(integer)
				);
#line 588 ""
			e_rsfi();
/* 1st digit of iinst */
#line 589 ""
			i__1 = ibb;
#line 589 ""
			itemp = i_indx("01234567890", line + i__1, (ftnlen)11,
				 ibb + 1 - i__1);
#line 590 ""
			if (itemp > 0) {
#line 591 ""
			    ++ibb;
#line 592 ""
			    iiinst = iiinst * 10 + itemp - 1;
#line 593 ""
			}
#line 594 ""
			++ibb;
/* now at first +/-. Need end of 2nd number */
#line 595 ""
			i__1 = ibb;
#line 595 ""
			itemp = i_indx(line + i__1, "i", ib - i__1, (ftnlen)1)
				;
/* Rel pos'n of next 'i' */
#line 596 ""
			if (itemp > 0) {
#line 597 ""
			    iend = ibb + itemp - 1;
#line 598 ""
			} else {
#line 599 ""
			    iend = ib - 1;
#line 600 ""
			}
#line 601 ""
			io___98.ciunit = all_1.iorig[iiinst - 1] + 10;
#line 601 ""
			s_wsfe(&io___98);
/* Writing concatenation */
#line 601 ""
			i__3[0] = 3, a__1[0] = "Ki1";
#line 601 ""
			i__3[1] = iend - (ibb - 1), a__1[1] = line + (ibb - 1)
				;
#line 601 ""
			s_cat(ch__6, a__1, i__3, &c__2, (ftnlen)131);
#line 601 ""
			do_fio(&c__1, ch__6, iend - (ibb - 1) + 3);
#line 601 ""
			e_wsfe();
#line 602 ""
			if (itemp > 0) {
#line 603 ""
			    ibb = iend + 2;
#line 604 ""
			    goto L40;
#line 605 ""
			}
#line 606 ""
			goto L12;
/* Remove K command from string, go to next ia */
#line 607 ""
		    }
#line 608 ""
L9:
#line 609 ""
		    i__1 = ib - idxa;
#line 609 ""
		    allparts_(line + (idxa - 1), &i__1, ib - 1 - (idxa - 1));
#line 610 ""
L12:

/*  Remove the string from line */

#line 614 ""
		    if (idxa == 1) {
#line 615 ""
			s_copy(line, line + (ib - 1), (ftnlen)128, 128 - (ib 
				- 1));
#line 616 ""
		    } else {
/* Writing concatenation */
#line 617 ""
			i__3[0] = idxa - 1, a__1[0] = line;
#line 617 ""
			i__3[1] = 128 - (ib - 1), a__1[1] = line + (ib - 1);
#line 617 ""
			s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 618 ""
		    }
#line 619 ""
		    lenline = lenstr_(line, &c__128, (ftnlen)128);

/*  Loop if only blanks are left */

#line 623 ""
		    if (lenline == 0) {
#line 623 ""
			goto L4;
#line 623 ""
		    }

/*  Must check for multiple "I" commands, so go to just after start of ia loop */

#line 627 ""
		    if (ia == 10) {
#line 627 ""
			goto L24;
#line 627 ""
		    }

/*  Tack on front part from 1st line of saved macro */

#line 631 ""
		    if (lenhold > 0) {
/* Writing concatenation */
#line 632 ""
			i__4[0] = lenhold, a__2[0] = holdln;
#line 632 ""
			i__4[1] = 1, a__2[1] = " ";
#line 632 ""
			i__4[2] = lenline, a__2[2] = line;
#line 632 ""
			s_cat(line, a__2, i__4, &c__3, (ftnlen)128);
#line 633 ""
			lenhold = 0;
#line 634 ""
		    }
#line 635 ""
		}
#line 636 ""
/* L5: */
#line 636 ""
	    }
#line 637 ""
	}
#line 637 ""
    }

/*  Now a special loop to deal with 'X'.  If it was %[n]X..., will have been */
/*  copied into part [n] already.  If no "B" or "P", remove.  If "P", just */
/*  remove the "P" so pmxa/b will process.  If "B". do nothing. */

#line 643 ""
L10:
#line 644 ""
    nchk = 1;
#line 645 ""
L13:
#line 645 ""
    ntinx = nchk - 1 + ntindex_(line + (nchk - 1), "X", 128 - (nchk - 1), (
	    ftnlen)1);
#line 646 ""
    if (ntinx > nchk - 1) {

/*  There is a non-TeX 'X' at ntinx.  Loop if neither 1st nor after a blank. */

#line 650 ""
	if (ntinx > 1) {
#line 651 ""
	    i__1 = ntinx - 2;
#line 651 ""
	    if (s_cmp(line + i__1, " ", ntinx - 1 - i__1, (ftnlen)1) != 0) {

/*  The X is not 1st char of PMX command.  Advance starting point, loop. */

#line 655 ""
		nchk = ntinx + 1;
#line 656 ""
		goto L13;
#line 657 ""
	    }
#line 658 ""
	}

/*  We now know the X at ntinx starts a PMX command.  Find next blank */

#line 662 ""
	i__1 = ntinx;
#line 662 ""
	ib = ntinx + i_indx(line + i__1, " ", 128 - i__1, (ftnlen)1);

/*  There must be a blank to right of "X", so ib>ntinx */

/*        locp = nchk-1+index(line(nchk:ib),'P') */
#line 667 ""
	i__1 = ntinx;
#line 667 ""
	locp = ntinx + i_indx(line + i__1, "P", ib - i__1, (ftnlen)1);

/*  Did not need to use ntindex because we already know bounds of PMX command. */

/*        if (locp .gt. nchk-1) then */
#line 672 ""
	if (locp > ntinx) {

/*  Strip out the 'P' */

#line 676 ""
	    s_copy(templine, line, (ftnlen)128, locp - 1);
#line 677 ""
	    i__1 = locp;
/* Writing concatenation */
#line 677 ""
	    i__3[0] = locp - 1, a__1[0] = templine;
#line 677 ""
	    i__3[1] = lenline - i__1, a__1[1] = line + i__1;
#line 677 ""
	    s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 678 ""
	    --lenline;
#line 679 ""
	    --ib;
#line 680 ""
	}
#line 681 ""
	if (i_indx(line + (ntinx - 1), ":", ib - (ntinx - 1), (ftnlen)1) > 0 
		|| i_indx(line + (ntinx - 1), "S", ib - (ntinx - 1), (ftnlen)
		1) > 0 || i_indx(line + (ntinx - 1), "B", ib - (ntinx - 1), (
		ftnlen)1) > 0 || locp > ntinx) {

/*  The X command is a shift, "Both", or "Part".  Do not remove. */

#line 687 ""
	    nchk = ib + 1;
#line 688 ""
	    goto L13;
#line 689 ""
	}

/*  Remove the X command. */

#line 693 ""
	if (ntinx == 1) {
#line 694 ""
	    if (ib < lenline) {
#line 695 ""
		i__1 = ib;
#line 695 ""
		s_copy(line, line + i__1, (ftnlen)128, lenline - i__1);
#line 696 ""
	    } else {

/*  line contains ONLY the "X" command, so get a new line */

#line 700 ""
		goto L4;
#line 701 ""
	    }
#line 702 ""
	} else {
#line 703 ""
	    s_copy(templine, line, (ftnlen)128, ntinx - 1);
#line 704 ""
	    if (ib < lenline) {
#line 705 ""
		i__1 = ib;
/* Writing concatenation */
#line 705 ""
		i__3[0] = ntinx - 1, a__1[0] = templine;
#line 705 ""
		i__3[1] = lenline - i__1, a__1[1] = line + i__1;
#line 705 ""
		s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 706 ""
	    } else {
#line 707 ""
		s_copy(line, templine, (ftnlen)128, ntinx - 1);
#line 708 ""
	    }
#line 709 ""
	}

/*  Recompute lenline */

#line 713 ""
	lenline = lenstr_(line, &c__128, (ftnlen)128);

/*  Resume checking after location of removed command. */

#line 717 ""
	nchk = ntinx;
#line 718 ""
	goto L13;
#line 719 ""
    }

/*  End of loop for X-checks */

#line 723 ""
    oneof2 = ntindex_(line, "//", (ftnlen)128, (ftnlen)2) > 0;
#line 724 ""
    if (termrpt && all_1.botv[iv - 1] && frstln && *(unsigned char *)&line[
	    lenline - 1] == '/') {

/*  Must add a terminal repeat before the slash */

#line 729 ""
	if (oneof2) {
#line 729 ""
	    --lenline;
#line 729 ""
	}
#line 730 ""
	if (lenline > 1) {
#line 730 ""
	    io___103.ciunit = all_1.iorig[iinst - 1] + 10;
#line 730 ""
	    s_wsfe(&io___103);
#line 730 ""
	    do_fio(&c__1, line, lenline - 1);
#line 730 ""
	    e_wsfe();
#line 730 ""
	}
#line 731 ""
	if (! oneof2) {
/* Writing concatenation */
#line 732 ""
	    i__3[0] = 2, a__1[0] = termsym;
#line 732 ""
	    i__3[1] = 2, a__1[1] = " /";
#line 732 ""
	    s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 733 ""
	    lenline = 4;
#line 734 ""
	} else {
/* Writing concatenation */
#line 735 ""
	    i__3[0] = 2, a__1[0] = termsym;
#line 735 ""
	    i__3[1] = 3, a__1[1] = " //";
#line 735 ""
	    s_cat(line, a__1, i__3, &c__2, (ftnlen)128);
#line 736 ""
	    lenline = 5;
#line 737 ""
	}
#line 738 ""
    }
#line 739 ""
    if (termrpt && frstln && *(unsigned char *)&line[lenline - 1] == '/' && 
	    iv == all_1.nvnow) {
#line 739 ""
	termrpt = FALSE_;
#line 739 ""
    }
#line 741 ""
    io___104.ciunit = all_1.iorig[iinst - 1] + 10;
#line 741 ""
    s_wsfe(&io___104);
#line 741 ""
    do_fio(&c__1, line, lenline);
#line 741 ""
    e_wsfe();
#line 742 ""
    if (oneof2) {
#line 743 ""
	frstln = FALSE_;
#line 744 ""
    } else if (! frstln) {
#line 745 ""
	frstln = TRUE_;
#line 746 ""
    }
/*      if (ntindex(line,'/').gt.0 .and. index(line,'//').eq.0) then */
#line 748 ""
    if (ntindex_(line, "/", (ftnlen)128, (ftnlen)1) > 0 && ntindex_(line, 
	    "//", (ftnlen)128, (ftnlen)2) == 0) {
#line 749 ""
	iv = iv % all_1.nvnow + 1;
#line 750 ""
	iinst = all_1.instnum[iv - 1];
#line 751 ""
    }
#line 752 ""
    goto L4;
#line 753 ""
L999:
#line 754 ""
    cl__1.cerr = 0;
#line 754 ""
    cl__1.cunit = 10;
#line 754 ""
    cl__1.csta = 0;
#line 754 ""
    f_clos(&cl__1);

/*  In the mbrest checks, must run through ALL noinst files (not just noinow) */

#line 758 ""
    i__1 = all_1.noinst;
#line 758 ""
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
#line 777 ""
	if (all_1.nvi[iinst - 1] == 1) {
#line 778 ""
	    mbrests_(&iinst, jobname, &ljob, &ludpfn[iinst - 1], udpfnq + (
		    iinst - 1 << 7), (ftnlen)27, (ftnlen)128);
#line 779 ""
	} else {

/*  Send a signal with ljob to bypass most mbrest processing */

#line 783 ""
	    i__2 = -ljob;
#line 783 ""
	    mbrests_(&iinst, jobname, &i__2, &ludpfn[iinst - 1], udpfnq + (
		    iinst - 1 << 7), (ftnlen)27, (ftnlen)128);
#line 784 ""
	}
#line 785 ""
/* L11: */
#line 785 ""
    }
#line 786 ""
    return 0;
} /* MAIN__ */

integer lenstr_(char *string, integer *n, ftnlen string_len)
{
    /* System generated locals */
    integer ret_val;

#line 789 ""
    for (ret_val = *n; ret_val >= 1; --ret_val) {
#line 790 ""
	if (*(unsigned char *)&string[ret_val - 1] != ' ') {
#line 790 ""
	    return ret_val;
#line 790 ""
	}
#line 791 ""
/* L1: */
#line 791 ""
    }
#line 792 ""
    ret_val = 0;
#line 793 ""
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


#line 801 ""
    len = lenstr_(string, n, string_len);
#line 802 ""
    if (len == 0) {
#line 803 ""
	len = 1;
#line 804 ""
	s_copy(string, " ", string_len, (ftnlen)1);
#line 805 ""
    }
#line 806 ""
    i__1 = all_1.noinow;
#line 806 ""
    for (iinst = 1; iinst <= i__1; ++iinst) {
#line 807 ""
	io___107.ciunit = all_1.iorig[iinst - 1] + 10;
#line 807 ""
	s_wsfe(&io___107);
#line 807 ""
	do_fio(&c__1, string, len);
#line 807 ""
	e_wsfe();
#line 808 ""
/* L1: */
#line 808 ""
    }
#line 809 ""
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


#line 819 ""
    type4 = FALSE_;
#line 820 ""
    *(unsigned char *)sq = '\\';
#line 821 ""
    alldone = FALSE_;
#line 822 ""
    al__1.aerr = 0;
#line 822 ""
    al__1.aunit = *iv + 10;
#line 822 ""
    f_rew(&al__1);
#line 823 ""
    if (*iv < 10) {
#line 824 ""
	*(unsigned char *)partq = (char) (*iv + 48);
#line 825 ""
	lpart = 1;
#line 826 ""
    } else {
#line 827 ""
	lpart = 2;
#line 828 ""
	if (*iv < 20) {
/* Writing concatenation */
#line 829 ""
	    i__1[0] = 1, a__1[0] = "1";
#line 829 ""
	    *(unsigned char *)&ch__1[0] = *iv + 38;
#line 829 ""
	    i__1[1] = 1, a__1[1] = ch__1;
#line 829 ""
	    s_cat(partq, a__1, i__1, &c__2, (ftnlen)2);
#line 830 ""
	} else {
/* Writing concatenation */
#line 831 ""
	    i__1[0] = 1, a__1[0] = "2";
#line 831 ""
	    *(unsigned char *)&ch__1[0] = *iv + 28;
#line 831 ""
	    i__1[1] = 1, a__1[1] = ch__1;
#line 831 ""
	    s_cat(partq, a__1, i__1, &c__2, (ftnlen)2);
#line 832 ""
	}
#line 833 ""
    }
/* 130327 */
/*      open(40,file=jobname(1:abs(ljob))//partq(1:lpart)//'.pmx') */
#line 836 ""
    if (*ludpfn == 0) {
#line 837 ""
	o__1.oerr = 0;
#line 837 ""
	o__1.ounit = 40;
#line 837 ""
	o__1.ofnmlen = abs(*ljob) + lpart + 4;
/* Writing concatenation */
#line 837 ""
	i__2[0] = abs(*ljob), a__2[0] = jobname;
#line 837 ""
	i__2[1] = lpart, a__2[1] = partq;
#line 837 ""
	i__2[2] = 4, a__2[2] = ".pmx";
#line 837 ""
	s_cat(ch__2, a__2, i__2, &c__3, (ftnlen)18);
#line 837 ""
	o__1.ofnm = ch__2;
#line 837 ""
	o__1.orl = 0;
#line 837 ""
	o__1.osta = 0;
#line 837 ""
	o__1.oacc = 0;
#line 837 ""
	o__1.ofm = 0;
#line 837 ""
	o__1.oblnk = 0;
#line 837 ""
	f_open(&o__1);
#line 838 ""
    } else {
#line 839 ""
	o__1.oerr = 0;
#line 839 ""
	o__1.ounit = 40;
#line 839 ""
	o__1.ofnmlen = *ludpfn + 4;
/* Writing concatenation */
#line 839 ""
	i__1[0] = *ludpfn, a__1[0] = udpfnq;
#line 839 ""
	i__1[1] = 4, a__1[1] = ".pmx";
#line 839 ""
	s_cat(ch__3, a__1, i__1, &c__2, (ftnlen)132);
#line 839 ""
	o__1.ofnm = ch__3;
#line 839 ""
	o__1.orl = 0;
#line 839 ""
	o__1.osta = 0;
#line 839 ""
	o__1.oacc = 0;
#line 839 ""
	o__1.ofm = 0;
#line 839 ""
	o__1.oblnk = 0;
#line 839 ""
	f_open(&o__1);
#line 840 ""
    }
#line 841 ""
    for (i__ = 1; i__ <= 10000; ++i__) {
#line 842 ""
	io___114.ciunit = *iv + 10;
#line 842 ""
	s_rsfe(&io___114);
#line 842 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 842 ""
	e_rsfe();
#line 843 ""
	if (*(unsigned char *)&line[0] == '%' || s_cmp(line, "---", (ftnlen)3,
		 (ftnlen)3) == 0 || type4) {
#line 845 ""
	    len = lenstr_(line, &c__128, (ftnlen)128);
#line 846 ""
	    if (len > 0) {
#line 847 ""
		s_wsfe(&io___117);
#line 847 ""
		do_fio(&c__1, line, len);
#line 847 ""
		e_wsfe();
#line 848 ""
	    } else {
#line 849 ""
		s_wsfe(&io___118);
#line 849 ""
		do_fio(&c__1, " ", (ftnlen)1);
#line 849 ""
		e_wsfe();
#line 850 ""
	    }
#line 851 ""
	    if (s_cmp(line, "---", (ftnlen)3, (ftnlen)3) == 0) {
#line 851 ""
		type4 = ! type4;
#line 851 ""
	    }
#line 852 ""
	} else {
#line 853 ""
	    goto L11;
#line 854 ""
	}
#line 855 ""
/* L10: */
#line 855 ""
    }
#line 856 ""
    s_wsle(&io___119);
#line 856 ""
    do_lio(&c__9, &c__1, "You should not be here in scor2prt.  Call Dr. Don", 
	    (ftnlen)49);
#line 856 ""
    e_wsle();
#line 857 ""
    s_stop("", (ftnlen)0);
#line 858 ""
L11:

/*  Finished reading opening type4 TeX and comments.  Next line to be read */
/*  will contain the first of the input numbers */

#line 863 ""
    dosetup_(iv, line, &mtrnum, &mtrden, (ftnlen)128);
#line 864 ""
    for (i__ = 1; i__ <= 10000; ++i__) {
#line 865 ""
L13:
#line 865 ""
	io___122.ciunit = *iv + 10;
#line 865 ""
	i__3 = s_rsfe(&io___122);
#line 865 ""
	if (i__3 != 0) {
#line 865 ""
	    goto L999;
#line 865 ""
	}
#line 865 ""
	i__3 = do_fio(&c__1, line, (ftnlen)128);
#line 865 ""
	if (i__3 != 0) {
#line 865 ""
	    goto L999;
#line 865 ""
	}
#line 865 ""
	i__3 = e_rsfe();
#line 865 ""
	if (i__3 != 0) {
#line 865 ""
	    goto L999;
#line 865 ""
	}
#line 866 ""
L7:
#line 866 ""
	len = lenstr_(line, &c__128, (ftnlen)128);

/*  Pass-through (and copy into part file) if instrumnet has >1 voice. */

#line 870 ""
	if (*ljob < 0) {
#line 870 ""
	    goto L2;
#line 870 ""
	}
#line 871 ""
	if (i_indx("TtTiTch+h-h l ", line, (ftnlen)14, (ftnlen)2) > 0) {

/*  Traps titles, instruments, composers, headers, lower strings.  Read 2 lines. */

#line 875 ""
	    s_wsfe(&io___123);
#line 875 ""
	    do_fio(&c__1, line, len);
#line 875 ""
	    e_wsfe();
#line 876 ""
	    io___124.ciunit = *iv + 10;
#line 876 ""
	    s_rsfe(&io___124);
#line 876 ""
	    do_fio(&c__1, line, (ftnlen)128);
#line 876 ""
	    e_rsfe();
#line 877 ""
	    len = lenstr_(line, &c__128, (ftnlen)128);
#line 878 ""
	    goto L2;
#line 879 ""
	}
#line 880 ""
	if (i__ == 1 || i__ > 5 && *(unsigned char *)&line[0] == 'm') {

/*  Either just starting, or a new meter is defined. */
/*  NOTE! The above test may be bogus. */

#line 885 ""
	    if (*(unsigned char *)&line[0] == '%') {
#line 886 ""
		s_wsfe(&io___125);
#line 886 ""
		do_fio(&c__1, line, len);
#line 886 ""
		e_wsfe();
#line 887 ""
		goto L13;
#line 888 ""
	    }
#line 889 ""
	    if (i__ != 1) {

/*  New meter. Check for slashes (new meter change syntax) */

#line 893 ""
		idxs = i_indx(line, "/", (ftnlen)128, (ftnlen)1);
#line 894 ""
		idxb = i_indx(line, " ", (ftnlen)128, (ftnlen)1);
#line 895 ""
		newmtr = idxs > 0 && (idxb == 0 || idxs < idxb);
#line 896 ""
		if (! newmtr) {

/*  Old way, no slashes, uses 'o' for lonesome '1' */

#line 900 ""
		    icden = 3;
#line 901 ""
		    if (*(unsigned char *)&line[1] == 'o') {
#line 902 ""
			mtrnum = 1;
#line 903 ""
		    } else {
#line 904 ""
			mtrnum = *(unsigned char *)&line[1] - 48;
#line 905 ""
			if (mtrnum == 1) {
#line 906 ""
			    icden = 4;
#line 907 ""
			    mtrnum = *(unsigned char *)&line[2] - 38;
#line 908 ""
			}
#line 909 ""
		    }
#line 910 ""
		    mtrden = *(unsigned char *)&line[icden - 1] - 48;
#line 911 ""
		} else {

/*  New way with slashes: idxs is index of 1st slash! */

#line 915 ""
		    ici__1.icierr = 0;
#line 915 ""
		    ici__1.iciend = 0;
#line 915 ""
		    ici__1.icirnum = 1;
#line 915 ""
		    ici__1.icirlen = idxs - 2;
#line 915 ""
		    ici__1.iciunit = line + 1;
/* Writing concatenation */
#line 915 ""
		    i__2[0] = 2, a__2[0] = "(i";
#line 915 ""
		    *(unsigned char *)&ch__1[0] = idxs + 46;
#line 915 ""
		    i__2[1] = 1, a__2[1] = ch__1;
#line 915 ""
		    i__2[2] = 1, a__2[2] = ")";
#line 915 ""
		    ici__1.icifmt = (s_cat(ch__4, a__2, i__2, &c__3, (ftnlen)
			    4), ch__4);
#line 915 ""
		    s_rsfi(&ici__1);
#line 915 ""
		    do_fio(&c__1, (char *)&mtrnum, (ftnlen)sizeof(integer));
#line 915 ""
		    e_rsfi();
#line 916 ""
		    i__3 = idxs;
#line 916 ""
		    idxb = i_indx(line + i__3, "/", 128 - i__3, (ftnlen)1);
#line 917 ""
		    i__3 = idxs;
#line 917 ""
		    ici__1.icierr = 0;
#line 917 ""
		    ici__1.iciend = 0;
#line 917 ""
		    ici__1.icirnum = 1;
#line 917 ""
		    ici__1.icirlen = idxs + idxb - 1 - i__3;
#line 917 ""
		    ici__1.iciunit = line + i__3;
/* Writing concatenation */
#line 917 ""
		    i__2[0] = 2, a__2[0] = "(i";
#line 917 ""
		    *(unsigned char *)&ch__1[0] = idxb + 47;
#line 917 ""
		    i__2[1] = 1, a__2[1] = ch__1;
#line 917 ""
		    i__2[2] = 1, a__2[2] = ")";
#line 917 ""
		    ici__1.icifmt = (s_cat(ch__4, a__2, i__2, &c__3, (ftnlen)
			    4), ch__4);
#line 917 ""
		    s_rsfi(&ici__1);
#line 917 ""
		    do_fio(&c__1, (char *)&mtrden, (ftnlen)sizeof(integer));
#line 917 ""
		    e_rsfi();
#line 919 ""
		}
#line 920 ""
	    }
#line 921 ""
	    lenbeat = ifnodur_(&mtrden, "x", (ftnlen)1);
#line 922 ""
	    lenmult = 1;
#line 923 ""
	    if (mtrden == 2) {
#line 924 ""
		lenbeat = 16;
#line 925 ""
		lenmult = 2;
#line 926 ""
	    }
#line 927 ""
	    lenbar = lenmult * mtrnum * lenbeat;
#line 928 ""
	    fwbrsym_(&lenbar, &nwbrs, wbrsym, &lwbrs, (ftnlen)3);
#line 929 ""
	}

/* Finished setting up meter stuff and defining whole-bar rest symbols */

#line 933 ""
	ip1 = 0;
#line 934 ""
	s_copy(line1, line, (ftnlen)128, (ftnlen)128);
#line 935 ""
	i__3 = nwbrs;
#line 935 ""
	for (iw = 0; iw <= i__3; ++iw) {
#line 936 ""
	    if (iw > 0) {
#line 937 ""
		idx = ntindex_(line1, wbrsym + (iw - 1) * 3, (ftnlen)128, 
			lwbrs);
#line 938 ""
		if (idx > 0) {

/*  Check for blank or shifted rest, discount it if it's there */

#line 942 ""
		    i__4 = idx + lwbrs - 1;
#line 942 ""
		    if (s_cmp(line1 + i__4, " ", idx + lwbrs - i__4, (ftnlen)
			    1) != 0) {
#line 942 ""
			idx = 0;
#line 942 ""
		    }
#line 943 ""
		}
#line 944 ""
	    } else {
#line 945 ""
		idx = ntindex_(line1, "rp", (ftnlen)128, (ftnlen)2);

/*  Check for raised rest */

#line 949 ""
		if (idx > 0) {
#line 950 ""
		    i__4 = idx + 1;
#line 950 ""
		    if (s_cmp(line1 + i__4, " ", idx + 2 - i__4, (ftnlen)1) !=
			     0) {
#line 950 ""
			idx = 0;
#line 950 ""
		    }
#line 951 ""
		}
#line 952 ""
	    }
#line 953 ""
	    if (idx > 0) {
#line 954 ""
		if (ip1 == 0) {
#line 955 ""
		    ip1 = idx;
#line 956 ""
		} else {
#line 957 ""
		    ip1 = min(ip1,idx);
/* Maybe allows e.g. r0 rp ... */
#line 958 ""
		}
#line 959 ""
	    }
#line 960 ""
/* L3: */
#line 960 ""
	}
/* Writing concatenation */
#line 961 ""
	i__1[0] = 1, a__1[0] = sq;
#line 961 ""
	i__1[1] = 1, a__1[1] = sq;
#line 961 ""
	s_cat(ch__5, a__1, i__1, &c__2, (ftnlen)2);
#line 961 ""
	if (i__ < 5 || *(unsigned char *)&line[0] == '%' || s_cmp(line, ch__5,
		 (ftnlen)2, (ftnlen)2) == 0 || ip1 == 0) {
#line 961 ""
	    goto L2;
#line 961 ""
	}

/*  Switch to multibar rest search mode!!!  Start forward in line(1) */

#line 966 ""
	rpfirst = s_cmp(line1 + (ip1 - 1), "rp", (ftnlen)2, (ftnlen)2) == 0;
#line 967 ""
	iline = 1;
#line 968 ""
	nmbr = 1;
#line 969 ""
	if (rpfirst) {
#line 970 ""
	    lwbrsx = 2;
#line 971 ""
	} else {
#line 972 ""
	    lwbrsx = lwbrs;
#line 973 ""
	}
#line 974 ""
	ipe = ip1 + lwbrsx - 1;
/* ip at end of 1st wbrsym */
#line 975 ""
L4:
#line 975 ""
	if (ipe == len) {

/*  Need a new line */

#line 979 ""
	    ++iline;
#line 980 ""
L6:
#line 980 ""
	    io___145.ciunit = *iv + 10;
#line 980 ""
	    i__3 = s_rsfe(&io___145);
#line 980 ""
	    if (i__3 != 0) {
#line 980 ""
		goto L998;
#line 980 ""
	    }
#line 980 ""
	    i__3 = do_fio(&c__1, line + (iline - 1 << 7), (ftnlen)128);
#line 980 ""
	    if (i__3 != 0) {
#line 980 ""
		goto L998;
#line 980 ""
	    }
#line 980 ""
	    i__3 = e_rsfe();
#line 980 ""
	    if (i__3 != 0) {
#line 980 ""
		goto L998;
#line 980 ""
	    }
#line 981 ""
	    len = lenstr_(line + (iline - 1 << 7), &c__128, (ftnlen)128);
#line 982 ""
	    if (*(unsigned char *)&line[(iline - 1) * 128] == '%') {
#line 983 ""
		s_wsfe(&io___146);
#line 983 ""
		do_fio(&c__1, "% Following comment has been moved forward", (
			ftnlen)42);
#line 983 ""
		e_wsfe();
#line 984 ""
		s_wsfe(&io___147);
#line 984 ""
		do_fio(&c__1, line + (iline - 1 << 7), len);
#line 984 ""
		e_wsfe();
#line 985 ""
		goto L6;
#line 986 ""
	    }
#line 987 ""
	    ipe = 0;
#line 988 ""
	    goto L4;
#line 989 ""
L998:

/*  No more input left */

#line 993 ""
	    s_wsle(&io___148);
#line 993 ""
	    do_lio(&c__9, &c__1, "All done!", (ftnlen)9);
#line 993 ""
	    e_wsle();
#line 994 ""
	    alldone = TRUE_;
#line 995 ""
	    ipe = 0;
#line 996 ""
	    --iline;
#line 997 ""
	    len = lenstr_(line + (iline - 1 << 7), &c__128, (ftnlen)128);
#line 998 ""
	    goto L4;
#line 999 ""
	} else {
#line 1000 ""
	    if (alldone) {
#line 1001 ""
		*(unsigned char *)sym = ' ';
#line 1002 ""
	    } else {

/*  ipe<len here, so it's ok to get a symbol */

#line 1006 ""
		nextsym_(line + (iline - 1 << 7), &len, &ipe, &ipenew, sym, &
			lsym, (ftnlen)128, (ftnlen)80);
#line 1007 ""
	    }

/*  Check for end of block or bar line symbol */

#line 1011 ""
	    if (i_indx("/|", sym, (ftnlen)2, (ftnlen)1) > 0) {
#line 1012 ""
		ipe = ipenew;
#line 1013 ""
		goto L4;
#line 1014 ""
	    } else {
#line 1015 ""
		wbrest = FALSE_;
#line 1016 ""
		if (alldone) {
#line 1016 ""
		    goto L12;
#line 1016 ""
		}
#line 1017 ""
		i__3 = nwbrs;
#line 1017 ""
		for (iw = 1; iw <= i__3; ++iw) {
#line 1018 ""
		    wbrest = wbrest || s_cmp(sym, wbrsym + (iw - 1) * 3, lsym,
			     lwbrs) == 0;
#line 1019 ""
/* L5: */
#line 1019 ""
		}
#line 1020 ""
		wbrest = wbrest || s_cmp(sym, "r", lsym, (ftnlen)1) == 0 && 
			lwbrs == 2 || s_cmp(sym, "rd", lsym, (ftnlen)2) == 0 
			&& lwbrs == 3 || s_cmp(sym, "rp", lsym, (ftnlen)2) == 
			0 || s_cmp(sym, "r", lsym, (ftnlen)1) == 0 && rpfirst;
#line 1024 ""
L12:
#line 1024 ""
		if (wbrest) {
#line 1025 ""
		    ipe = ipenew;
#line 1026 ""
		    ++nmbr;
#line 1027 ""
		    goto L4;
#line 1028 ""
		} else {

/*  AHA! Failed prev. test, so last symbol was *not* mbr. */
/*  It must be saved, and its starting position is ipenew-lsym+1 */

#line 1033 ""
		    if (nmbr > 1) {

/*  Write stuff up to start of mbr */

#line 1037 ""
			if (ip1 > 1) {
#line 1037 ""
			    s_wsfe(&io___153);
#line 1037 ""
			    do_fio(&c__1, line, ip1 - 1);
#line 1037 ""
			    e_wsfe();
#line 1037 ""
			}

/*  Insert mbr symbol.  Always end with a slash just in case next sym must be */
/*  at start of block.  May think this causes undefined octaves, but */
/*  probably not since it's a single voice. */

#line 1043 ""
			r__1 = nmbr + .01f;
#line 1043 ""
			ndig = (integer) r_lg10(&r__1) + 1;
#line 1044 ""
			s_wsle(&io___155);
#line 1044 ""
			do_lio(&c__9, &c__1, "Inserting rm, iv,nmbr:", (
				ftnlen)22);
#line 1044 ""
			do_lio(&c__3, &c__1, (char *)&(*iv), (ftnlen)sizeof(
				integer));
#line 1044 ""
			do_lio(&c__3, &c__1, (char *)&nmbr, (ftnlen)sizeof(
				integer));
#line 1044 ""
			e_wsle();
#line 1045 ""
			ci__1.cierr = 0;
#line 1045 ""
			ci__1.ciunit = 40;
/* Writing concatenation */
#line 1045 ""
			i__2[0] = 5, a__2[0] = "(a2,i";
#line 1045 ""
			*(unsigned char *)&ch__1[0] = ndig + 48;
#line 1045 ""
			i__2[1] = 1, a__2[1] = ch__1;
#line 1045 ""
			i__2[2] = 4, a__2[2] = ",a2)";
#line 1045 ""
			ci__1.cifmt = (s_cat(ch__6, a__2, i__2, &c__3, (
				ftnlen)10), ch__6);
#line 1045 ""
			s_wsfe(&ci__1);
#line 1045 ""
			do_fio(&c__1, "rm", (ftnlen)2);
#line 1045 ""
			do_fio(&c__1, (char *)&nmbr, (ftnlen)sizeof(integer));
#line 1045 ""
			do_fio(&c__1, " /", (ftnlen)2);
#line 1045 ""
			e_wsfe();
#line 1046 ""
			if (alldone) {
#line 1046 ""
			    goto L999;
#line 1046 ""
			}
#line 1047 ""
			ipc = ipenew - lsym + 1;
#line 1048 ""
			s_copy(line, line + ((iline - 1 << 7) + (ipc - 1)), (
				ftnlen)128, len - (ipc - 1));
#line 1049 ""
		    } else {

/*  Write old stuff up to end of original lonesome wbr, save the rest. */
/*  4 cases:  (wbr /) , (wbr line-end) , (wbr followed by other non-/ symbols) , */
/*      alldone. */
/*  In 1st 2 will have gotten some other lines, so write all up to one b4 last */
/*  non-comment line; then revert to normal mode on that.  In 3rd case must */
/*  split line. */

#line 1058 ""
			if (alldone) {
#line 1059 ""
			    s_wsfe(&io___157);
#line 1059 ""
			    do_fio(&c__1, line, len);
#line 1059 ""
			    e_wsfe();
#line 1060 ""
			    goto L999;
#line 1061 ""
			} else if (iline > 1) {
#line 1062 ""
			    i__3 = iline - 1;
#line 1062 ""
			    for (il = 1; il <= i__3; ++il) {
#line 1063 ""
				len = lenstr_(line + (il - 1 << 7), &c__128, (
					ftnlen)128);
#line 1064 ""
				s_wsfe(&io___159);
#line 1064 ""
				do_fio(&c__1, line + (il - 1 << 7), len);
#line 1064 ""
				e_wsfe();
#line 1065 ""
/* L9: */
#line 1065 ""
			    }
#line 1066 ""
			    s_copy(line, line + (iline - 1 << 7), (ftnlen)128,
				     (ftnlen)128);
#line 1067 ""
			} else {

/*  Since iline = 1 the wbr is not the last sym, so must split */

#line 1071 ""
			    s_wsfe(&io___160);
#line 1071 ""
			    do_fio(&c__1, line, ip1 + lwbrsx - 1);
#line 1071 ""
			    e_wsfe();
#line 1072 ""
			    i__3 = ip1 + lwbrsx;
#line 1072 ""
			    s_copy(line, line + i__3, (ftnlen)128, len - i__3)
				    ;
#line 1073 ""
			}
#line 1074 ""
		    }

/*  Exit multibar mode */

#line 1078 ""
		    goto L7;
#line 1079 ""
		}
#line 1080 ""
	    }
#line 1081 ""
	}
#line 1082 ""
L2:
#line 1083 ""
	if (len > 0) {
#line 1084 ""
	    s_wsfe(&io___161);
#line 1084 ""
	    do_fio(&c__1, line, len);
#line 1084 ""
	    e_wsfe();
#line 1085 ""
	} else {
#line 1086 ""
	    s_wsfe(&io___162);
#line 1086 ""
	    do_fio(&c__1, " ", (ftnlen)1);
#line 1086 ""
	    e_wsfe();
#line 1087 ""
	}
#line 1088 ""
/* L1: */
#line 1088 ""
    }
#line 1089 ""
L999:
#line 1090 ""
    cl__1.cerr = 0;
#line 1090 ""
    cl__1.cunit = *iv + 10;
#line 1090 ""
    cl__1.csta = 0;
#line 1090 ""
    f_clos(&cl__1);
#line 1091 ""
    cl__1.cerr = 0;
#line 1091 ""
    cl__1.cunit = 40;
#line 1091 ""
    cl__1.csta = 0;
#line 1091 ""
    f_clos(&cl__1);
#line 1092 ""
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


#line 1096 ""
    if (*idur == 6) {
#line 1097 ""
	ret_val = 1;
#line 1098 ""
    } else if (*idur == 3) {
#line 1099 ""
	ret_val = 2;
#line 1100 ""
    } else if (*idur == 1 || *idur == 16) {
#line 1101 ""
	ret_val = 4;
#line 1102 ""
    } else if (*idur == 8) {
#line 1103 ""
	ret_val = 8;
#line 1104 ""
    } else if (*idur == 4) {
#line 1105 ""
	ret_val = 16;
#line 1106 ""
    } else if (*idur == 2) {
#line 1107 ""
	ret_val = 32;
#line 1108 ""
    } else if (*idur == 0) {
#line 1109 ""
	ret_val = 64;
#line 1110 ""
    } else {
#line 1111 ""
	s_wsle(&io___163);
#line 1111 ""
	do_lio(&c__9, &c__1, "You entered an invalid note-length value", (
		ftnlen)40);
#line 1111 ""
	e_wsle();
#line 1112 ""
	s_stop("", (ftnlen)0);
#line 1113 ""
    }
#line 1114 ""
    if (*(unsigned char *)dotq == 'd') {
#line 1114 ""
	ret_val = ret_val * 3 / 2;
#line 1114 ""
    }
#line 1115 ""
    return ret_val;
} /* ifnodur_ */

/* Subroutine */ int fwbrsym_(integer *lenbar, integer *nwbrs, char *wbrsym, 
	integer *lwbrs, ftnlen wbrsym_len)
{
    /* Builtin functions */
    /* Subroutine */ int s_copy(char *, char *, ftnlen, ftnlen);
    integer s_wsfe(cilist *), do_fio(integer *, char *, ftnlen), e_wsfe(void);

    /* Fortran I/O blocks */
    static cilist io___164 = { 0, 6, 0, "(a33,i3,a26)", 0 };


#line 1119 ""
    /* Parameter adjustments */
#line 1119 ""
    wbrsym -= 3;
#line 1119 ""

#line 1119 ""
    /* Function Body */
#line 1119 ""
    *nwbrs = 1;
#line 1120 ""
    *lwbrs = 2;
#line 1121 ""
    if (*lenbar == 16) {
#line 1122 ""
	s_copy(wbrsym + 3, "r4", (ftnlen)3, (ftnlen)2);
#line 1123 ""
    } else if (*lenbar == 32) {
#line 1124 ""
	s_copy(wbrsym + 3, "r2", (ftnlen)3, (ftnlen)2);
#line 1125 ""
    } else if (*lenbar == 64) {
#line 1126 ""
	s_copy(wbrsym + 3, "r0", (ftnlen)3, (ftnlen)2);
#line 1127 ""
    } else if (*lenbar == 8) {
#line 1128 ""
	s_copy(wbrsym + 3, "r8", (ftnlen)3, (ftnlen)2);
#line 1129 ""
    } else if (*lenbar == 128) {
#line 1130 ""
	s_copy(wbrsym + 3, "r9", (ftnlen)3, (ftnlen)2);
#line 1131 ""
    } else {
#line 1132 ""
	*nwbrs = 2;
#line 1133 ""
	*lwbrs = 3;
#line 1134 ""
	if (*lenbar == 24) {
#line 1135 ""
	    s_copy(wbrsym + 3, "rd4", (ftnlen)3, (ftnlen)3);
#line 1136 ""
	    s_copy(wbrsym + 6, "r4d", (ftnlen)3, (ftnlen)3);
#line 1137 ""
	} else if (*lenbar == 48) {
#line 1138 ""
	    s_copy(wbrsym + 3, "rd2", (ftnlen)3, (ftnlen)3);
#line 1139 ""
	    s_copy(wbrsym + 6, "r2d", (ftnlen)3, (ftnlen)3);
#line 1140 ""
	} else if (*lenbar == 96) {
#line 1141 ""
	    s_copy(wbrsym + 3, "rd0", (ftnlen)3, (ftnlen)3);
#line 1142 ""
	    s_copy(wbrsym + 6, "r0d", (ftnlen)3, (ftnlen)3);
#line 1143 ""
	} else {
#line 1144 ""
	    s_wsfe(&io___164);
#line 1144 ""
	    do_fio(&c__1, " Any whole-bar rests of duration", (ftnlen)32);
#line 1144 ""
	    do_fio(&c__1, (char *)&(*lenbar), (ftnlen)sizeof(integer));
#line 1144 ""
	    do_fio(&c__1, "/64 will not be recognized", (ftnlen)26);
#line 1144 ""
	    e_wsfe();
#line 1146 ""
	}
#line 1147 ""
    }
#line 1148 ""
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

#line 1157 ""
    if (*ipeold >= *len) {
#line 1158 ""
	s_wsle(&io___165);
#line 1158 ""
	do_lio(&c__9, &c__1, "Called nextsym with ipstart>=len ", (ftnlen)33);
#line 1158 ""
	e_wsle();
#line 1159 ""
	s_wsle(&io___166);
#line 1159 ""
	do_lio(&c__9, &c__1, "Send files to Dr. Don at dsimons@logicon.com", (
		ftnlen)44);
#line 1159 ""
	e_wsle();
#line 1160 ""
	s_stop("", (ftnlen)0);
#line 1161 ""
    }
#line 1162 ""
    i__1 = *len;
#line 1162 ""
    for (ip = *ipeold + 1; ip <= i__1; ++ip) {
#line 1163 ""
	if (*(unsigned char *)&line[ip - 1] != ' ') {

/*  symbol starts here (ip).  We're committed to exit the loop. */

#line 1167 ""
	    if (ip < *len) {
#line 1168 ""
		i__2 = *len;
#line 1168 ""
		for (iip = ip + 1; iip <= i__2; ++iip) {
#line 1169 ""
		    if (*(unsigned char *)&line[iip - 1] != ' ') {
#line 1169 ""
			goto L2;
#line 1169 ""
		    }

/*  iip is the space after the symbol */

#line 1173 ""
		    *ipenew = iip - 1;
#line 1174 ""
		    *lsym = *ipenew - ip + 1;
#line 1175 ""
		    s_copy(sym, line + (ip - 1), (ftnlen)80, *ipenew - (ip - 
			    1));
#line 1176 ""
		    return 0;
#line 1177 ""
L2:
#line 1177 ""
		    ;
#line 1177 ""
		}

/*  Have len>=2 and ends on len */

#line 1181 ""
		*ipenew = *len;
#line 1182 ""
		*lsym = *ipenew - ip + 1;
#line 1183 ""
		s_copy(sym, line + (ip - 1), (ftnlen)80, *ipenew - (ip - 1));
#line 1184 ""
		return 0;
#line 1185 ""
	    } else {

/*  ip = len */

#line 1189 ""
		*ipenew = *len;
#line 1190 ""
		*lsym = 1;
#line 1191 ""
		s_copy(sym, line + (ip - 1), (ftnlen)80, (ftnlen)1);
#line 1192 ""
		return 0;
#line 1193 ""
	    }
#line 1194 ""
	}
#line 1195 ""
/* L1: */
#line 1195 ""
    }
#line 1196 ""
    s_wsle(&io___169);
#line 1196 ""
    do_lio(&c__9, &c__1, "Error #3.  Send files to Dr. Don at dsimons@logico"\
	    "n.com", (ftnlen)55);
#line 1196 ""
    e_wsle();
#line 1197 ""
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

#line 1212 ""
    s_copy(tline, line, (ftnlen)128, (ftnlen)128);
#line 1213 ""
    ndxs2 = i_indx(tline, s2q, (ftnlen)128, s2q_len);

/*  Return point below for rechecks after zapping D"  " */

#line 1217 ""
L2:
#line 1218 ""
    ndxbs = i_indx(tline, "\\", (ftnlen)128, (ftnlen)1);
#line 1219 ""
    if (ndxbs > 0) {

/* Special check in case \ is inside D"..." */

#line 1223 ""
	ndxdq1 = i_indx(tline, "D\"", (ftnlen)128, (ftnlen)2);

/* If the following test fails, flow out of if block; else loop up to 2. */

#line 1227 ""
	if (ndxdq1 > 0) {

/* Find end of D"..." */

#line 1231 ""
	    i__1 = ndxdq1 + 1;
#line 1231 ""
	    ndxdq2 = ndxdq1 + 1 + i_indx(tline + i__1, "\"", 128 - i__1, (
		    ftnlen)1);
#line 1232 ""
	    if (ndxdq2 == ndxdq1 + 1) {
#line 1233 ""
		s_wsle(&io___175);
#line 1233 ""
		do_lio(&c__9, &c__1, "Something is really wierd here", (
			ftnlen)30);
#line 1233 ""
		e_wsle();
#line 1234 ""
		s_stop("", (ftnlen)0);
#line 1235 ""
	    }
#line 1236 ""
	    s_copy(tline, tline, (ftnlen)128, ndxdq1 - 1);
#line 1237 ""
	    i__1 = ndxdq2;
#line 1237 ""
	    for (ic = ndxdq1; ic <= i__1; ++ic) {
/* Writing concatenation */
#line 1238 ""
		i__2[0] = ic - 1, a__1[0] = tline;
#line 1238 ""
		i__2[1] = 1, a__1[1] = " ";
#line 1238 ""
		s_cat(tline, a__1, i__2, &c__2, (ftnlen)128);
#line 1239 ""
/* L3: */
#line 1239 ""
	    }
#line 1240 ""
	    i__1 = ndxdq2;
/* Writing concatenation */
#line 1240 ""
	    i__2[0] = ndxdq2, a__1[0] = tline;
#line 1240 ""
	    i__2[1] = 128 - i__1, a__1[1] = line + i__1;
#line 1240 ""
	    s_cat(tline, a__1, i__2, &c__2, (ftnlen)128);
#line 1241 ""
	    goto L2;
#line 1242 ""
	}
#line 1243 ""
    }
#line 1244 ""
    if (ndxbs == 0 || ndxs2 < ndxbs) {
#line 1245 ""
	ret_val = ndxs2;
/*     print*,'No bs, or char is left of 1st bs, ntindex:',ntindex */
#line 1247 ""
    } else {

/*  There are both bs and s2q, and bs is to the left of sq2. So check bs's to */
/*  right of first: End is '\ ', start is ' \' */

#line 1252 ""
	len = lenstr_(tline, &c__128, (ftnlen)128);
#line 1253 ""
	intex = TRUE_;
/*     print*,'intex+>',intex */
#line 1255 ""
	i__1 = len;
#line 1255 ""
	for (ic = ndxbs + 1; ic <= i__1; ++ic) {
#line 1256 ""
	    if (ic == ndxs2) {
#line 1257 ""
		if (intex) {
#line 1258 ""
		    ret_val = 0;
#line 1259 ""
		    i__3 = ic;
#line 1259 ""
		    ndxs2 = i_indx(tline + i__3, s2q, len - i__3, s2q_len) + 
			    ic;
/*     print*,'ndxs2 =>',ndxs2 */
#line 1261 ""
		} else {
#line 1262 ""
		    ret_val = ndxs2;
#line 1263 ""
		    return ret_val;
#line 1264 ""
		}
/*     print*,'Internal exit, intex, ntindex:',intex,ntindex */
#line 1266 ""
	    } else /* if(complicated condition) */ {
#line 1266 ""
		i__3 = ic;
#line 1266 ""
		if (intex && s_cmp(tline + i__3, "\\ ", ic + 2 - i__3, (
			ftnlen)2) == 0) {
#line 1267 ""
		    intex = FALSE_;
/*     print*,'intex+>',intex */
#line 1269 ""
		} else /* if(complicated condition) */ {
#line 1269 ""
		    i__3 = ic;
#line 1269 ""
		    if (! intex && s_cmp(tline + i__3, " \\", ic + 2 - i__3, (
			    ftnlen)2) == 0) {
#line 1271 ""
			intex = TRUE_;
/*     print*,'intex+>',intex */
#line 1273 ""
		    }
#line 1273 ""
		}
#line 1273 ""
	    }
#line 1274 ""
/* L1: */
#line 1274 ""
	}
/*     print*,'Out end of loop 1' */
#line 1276 ""
    }
/*     print*,'Exiting ntindex at the end???' */
#line 1278 ""
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

#line 1287 ""
    if (*iccount == 128) {
#line 1288 ""
	s_rsfe(&io___179);
#line 1288 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 1288 ""
	e_rsfe();
#line 1289 ""
	*iccount = 0;
#line 1290 ""
    }
#line 1291 ""
    ++(*iccount);
#line 1292 ""
    *(unsigned char *)charq = *(unsigned char *)&line[*iccount - 1];
#line 1293 ""
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

#line 1318 ""
L4:
#line 1318 ""
    if (*iccount == 128) {
#line 1319 ""
	s_rsfe(&io___180);
#line 1319 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 1319 ""
	e_rsfe();
#line 1320 ""
	if (all_1.replacing) {
#line 1320 ""
	    all_1.replacing = FALSE_;
#line 1320 ""
	}
#line 1321 ""
	chkcom_(line, &goto999, (ftnlen)128);
#line 1322 ""
	*iccount = 0;
#line 1323 ""
    }
#line 1324 ""
    ++(*iccount);

/*  Find next non-blank or end of line */

#line 1328 ""
    for (*iccount = *iccount; *iccount <= 127; ++(*iccount)) {
#line 1329 ""
	if (*(unsigned char *)&line[*iccount - 1] != ' ') {
#line 1329 ""
	    goto L3;
#line 1329 ""
	}
#line 1330 ""
/* L2: */
#line 1330 ""
    }

/*  If here, need to get a new line */

#line 1334 ""
    *iccount = 128;
#line 1335 ""
    goto L4;
#line 1336 ""
L3:

/*  iccount now points to start of number to read */

#line 1340 ""
    i1 = *iccount;
#line 1341 ""
L5:
#line 1341 ""
    getchar_(line, iccount, durq, (ftnlen)128, (ftnlen)1);

/*  Remember that getchar first increments iccount, *then* reads a character. */

#line 1345 ""
    if (i_indx("0123456789.-", durq, (ftnlen)12, (ftnlen)1) > 0) {
#line 1345 ""
	goto L5;
#line 1345 ""
    }
#line 1346 ""
    i2 = *iccount - 1;
#line 1347 ""
    if (i2 < i1) {
#line 1348 ""
	s_wsle(&io___185);
/* Writing concatenation */
#line 1348 ""
	i__1[0] = 7, a__1[0] = "Found \"";
#line 1348 ""
	i__1[1] = 1, a__1[1] = durq;
#line 1348 ""
	i__1[2] = 19, a__1[2] = "\" instead of number";
#line 1348 ""
	s_cat(ch__1, a__1, i__1, &c__3, (ftnlen)27);
#line 1348 ""
	do_lio(&c__9, &c__1, ch__1, (ftnlen)27);
#line 1348 ""
	e_wsle();
#line 1349 ""
	s_stop("1", (ftnlen)1);
#line 1350 ""
    }
#line 1351 ""
    icf = i2 - i1 + 49;
#line 1352 ""
    ici__1.icierr = 0;
#line 1352 ""
    ici__1.iciend = 0;
#line 1352 ""
    ici__1.icirnum = 1;
#line 1352 ""
    ici__1.icirlen = i2 - (i1 - 1);
#line 1352 ""
    ici__1.iciunit = line + (i1 - 1);
/* Writing concatenation */
#line 1352 ""
    i__1[0] = 2, a__1[0] = "(f";
#line 1352 ""
    *(unsigned char *)&ch__3[0] = icf;
#line 1352 ""
    i__1[1] = 1, a__1[1] = ch__3;
#line 1352 ""
    i__1[2] = 3, a__1[2] = ".0)";
#line 1352 ""
    ici__1.icifmt = (s_cat(ch__2, a__1, i__1, &c__3, (ftnlen)6), ch__2);
#line 1352 ""
    s_rsfi(&ici__1);
#line 1352 ""
    do_fio(&c__1, (char *)&ret_val, (ftnlen)sizeof(real));
#line 1352 ""
    e_rsfi();
#line 1353 ""
    if (! all_1.replacing) {
#line 1354 ""
	if (*iread == 0) {
#line 1355 ""
	    i__2 = i2 - i1 + 1;
#line 1355 ""
	    allparts_(line + (i1 - 1), &i__2, i2 - (i1 - 1));
#line 1356 ""
	} else if (*iread == 1) {
#line 1357 ""
	    allparts_("-999", &c__4, (ftnlen)4);
#line 1358 ""
	} else if (*iread == 2) {
#line 1359 ""
	    allparts_("1", &c__1, (ftnlen)1);
#line 1360 ""
	} else if (*iread == 3) {
#line 1361 ""
	    allparts_("-998", &c__4, (ftnlen)4);
#line 1362 ""
	} else if (*iread == 4) {
#line 1363 ""
	    allparts_("20", &c__2, (ftnlen)2);
#line 1364 ""
	} else if (*iread == 5) {
#line 1365 ""
	    allparts_(".05", &c__3, (ftnlen)3);
#line 1366 ""
	} else if (*iread != -1) {
#line 1367 ""
	    s_wsle(&io___187);
#line 1367 ""
	    do_lio(&c__9, &c__1, "Error with iread in readin", (ftnlen)26);
#line 1367 ""
	    e_wsle();
#line 1368 ""
	    s_stop("", (ftnlen)0);
#line 1369 ""
	}
#line 1370 ""
    }
#line 1371 ""
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

#line 1383 ""
    *goto999 = FALSE_;
#line 1384 ""
L1:
#line 1384 ""
    if (*(unsigned char *)line != '%') {
#line 1384 ""
	return 0;
#line 1384 ""
    }

/*  If here, line has some sort of comment */

#line 1388 ""
    if (*(unsigned char *)&line[1] == '%') {
#line 1389 ""
	if (! all_1.insetup) {

/*  Suck up a line, then flow out of "if" block to get another and loop */

#line 1393 ""
	    s_rsfe(&io___188);
#line 1393 ""
	    do_fio(&c__1, line, (ftnlen)128);
#line 1393 ""
	    e_rsfe();
/* ++VV */

/*  UNLESS (a) it has a score-only "M" and changes # of inst's. */

#line 1399 ""
	    if (i_indx(line, "M", (ftnlen)128, (ftnlen)1) > 0) {
#line 1400 ""
		idxl = i_indx(line, "L", (ftnlen)128, (ftnlen)1);
#line 1401 ""
		idxm = i_indx(line, "M", (ftnlen)128, (ftnlen)1);
#line 1402 ""
		idxn = i_indx(line, "n", (ftnlen)128, (ftnlen)1);
#line 1403 ""
		idxb = i_indx(line, " ", (ftnlen)128, (ftnlen)1);
#line 1404 ""
		if (idxl < idxm && idxm < idxn && (idxb == 0 || idxn < idxb)) 
			{
#line 1406 ""
		    i__1 = idxn;
#line 1406 ""
		    all_1.noinow = *(unsigned char *)&line[i__1] - 48;
#line 1407 ""
		    clefpend = TRUE_;

/*  Next noinow digits are original inst. #'s of new inst. set.  Next noinow */
/*  char's after that are clefs */

#line 1412 ""
		    all_1.nvnow = 0;
#line 1413 ""
		    i__1 = all_1.noinow;
#line 1413 ""
		    for (j = 1; j <= i__1; ++j) {
#line 1414 ""
			i__2 = idxn + 1 + j - 1;
#line 1414 ""
			all_1.iorig[j - 1] = *(unsigned char *)&line[i__2] - 
				48;
#line 1415 ""
			iposc0 = idxn + 1 + all_1.noinow;
#line 1416 ""
			i__2 = all_1.nvi[all_1.iorig[j - 1] - 1];
#line 1416 ""
			for (k = 1; k <= i__2; ++k) {
#line 1417 ""
			    ++all_1.nvnow;
/*                  clefq(nvnow) = line(iposc0+nvnow:iposc0+nvnow) */
#line 1419 ""
			    all_1.instnum[all_1.nvnow - 1] = j;
#line 1420 ""
			    all_1.botv[all_1.nvnow - 1] = k == 1 && j != 1;
#line 1421 ""
/* L25: */
#line 1421 ""
			}
#line 1422 ""
/* L24: */
#line 1422 ""
		    }
#line 1423 ""
		}
#line 1424 ""
	    }

/*  or if it's "h" or "l", need to suck up one more line */

#line 1428 ""
	    if (*(unsigned char *)line == 'h' && i_indx("+- ", line + 1, (
		    ftnlen)3, (ftnlen)1) > 0 || *(unsigned char *)line == 'T' 
		    || s_cmp(line, "l ", (ftnlen)2, (ftnlen)2) == 0) {
#line 1428 ""
		s_rsfe(&io___197);
#line 1428 ""
		do_fio(&c__1, line, (ftnlen)128);
#line 1428 ""
		e_rsfe();
#line 1428 ""
	    }

/*  4/29/00 check for T string also */

#line 1434 ""
	} else {

/*  In setup mode. Set flag, flow out and do use following line */

#line 1438 ""
	    all_1.replacing = TRUE_;
#line 1439 ""
	}
#line 1440 ""
    } else if (*(unsigned char *)&line[1] == '!') {

/*  Copy to all parts */

#line 1444 ""
	allparts_(line + 2, &c__125, (ftnlen)126);
#line 1445 ""
    } else {

/*  Get value of hex integer 1,2,...,9,a,b,c in 2nd position, zero otherwise */
/* c  Get value of extended hex integer 1,2,...,9,a,b,c,...,o in 2nd position, zero otherwise */

#line 1450 ""
	ivq = i_indx("123456789abcdefghijklmno", line + 1, (ftnlen)24, (
		ftnlen)1);

/*  Only treat as part-specific pmx line if number .le. noinst */

#line 1454 ""
	if (ivq < 1 || ivq > all_1.noinst) {

/*  Simple comment. */

#line 1458 ""
	    allparts_(line, &c__128, (ftnlen)128);
#line 1459 ""
	} else {

/*  Instrument comment, copy only to part */

#line 1463 ""
	    lenline = lenstr_(line, &c__128, (ftnlen)128);
#line 1464 ""
	    if (lenline > 2) {
#line 1465 ""
		io___200.ciunit = ivq + 10;
#line 1465 ""
		s_wsfe(&io___200);
#line 1465 ""
		do_fio(&c__1, line + 2, lenline - 2);
#line 1465 ""
		e_wsfe();
#line 1466 ""
	    } else {

/*  Transferring blank line */

#line 1470 ""
		io___201.ciunit = ivq + 10;
#line 1470 ""
		s_wsfe(&io___201);
#line 1470 ""
		do_fio(&c__1, " ", (ftnlen)1);
#line 1470 ""
		e_wsfe();
#line 1471 ""
	    }
#line 1472 ""
	}
#line 1473 ""
    }
#line 1474 ""
    i__1 = s_rsfe(&io___202);
#line 1474 ""
    if (i__1 != 0) {
#line 1474 ""
	goto L2;
#line 1474 ""
    }
#line 1474 ""
    i__1 = do_fio(&c__1, line, (ftnlen)128);
#line 1474 ""
    if (i__1 != 0) {
#line 1474 ""
	goto L2;
#line 1474 ""
    }
#line 1474 ""
    i__1 = e_rsfe();
#line 1474 ""
    if (i__1 != 0) {
#line 1474 ""
	goto L2;
#line 1474 ""
    }
#line 1475 ""
    zapbl_(line, &c__128, (ftnlen)128);
#line 1476 ""
    goto L1;
#line 1477 ""
L2:
#line 1478 ""
    *goto999 = TRUE_;
#line 1479 ""
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

#line 1498 ""
    iccount = 0;
#line 1499 ""
    for (iset = 1; iset <= 12; ++iset) {
#line 1500 ""
	partnum_(iv, &iccount, line, &xdata, (ftnlen)128);
#line 1501 ""
	if (iset == 2) {
#line 1502 ""
	    if (xdata > 0.f) {
#line 1503 ""
		s_wsfe(&io___206);
#line 1503 ""
		i__1 = (integer) (xdata + .1f);
#line 1503 ""
		do_fio(&c__1, (char *)&i__1, (ftnlen)sizeof(integer));
#line 1503 ""
		e_wsfe();
#line 1504 ""
	    } else {
#line 1505 ""
		noi = -xdata + .1f;
#line 1506 ""
		s_wsfe(&io___208);
#line 1506 ""
		do_fio(&c__1, (char *)&noi, (ftnlen)sizeof(integer));
#line 1506 ""
		e_wsfe();
#line 1507 ""
		i__1 = noi;
#line 1507 ""
		for (ioi = 1; ioi <= i__1; ++ioi) {
#line 1508 ""
		    partnum_(iv, &iccount, line, &xdata, (ftnlen)128);
#line 1509 ""
		    s_wsfe(&io___210);
#line 1509 ""
		    i__2 = (integer) (xdata + .1f);
#line 1509 ""
		    do_fio(&c__1, (char *)&i__2, (ftnlen)sizeof(integer));
#line 1509 ""
		    e_wsfe();
#line 1510 ""
/* L2: */
#line 1510 ""
		}
#line 1511 ""
	    }
/*        else if (iset.ne.8 .and. xdata.lt.0) then */
#line 1513 ""
	} else if (iset != 8 && iset != 5 && xdata < 0.f) {

/*  Must be either nv or npages */

#line 1517 ""
	    if ((integer) (-xdata + .1f) == 999) {

/*  It's nv */

#line 1521 ""
		s_wsfe(&io___211);
#line 1521 ""
		do_fio(&c__1, (char *)&all_1.nvi[*iv - 1], (ftnlen)sizeof(
			integer));
#line 1521 ""
		e_wsfe();
#line 1522 ""
	    } else {

/*  npages must be computed */

#line 1526 ""
		s_wsfe(&io___212);
#line 1526 ""
		i__1 = (all_1.nsyst - 1) / 12 + 1;
#line 1526 ""
		do_fio(&c__1, (char *)&i__1, (ftnlen)sizeof(integer));
#line 1526 ""
		e_wsfe();
#line 1527 ""
	    }
#line 1528 ""
	} else if (iset != 7 && iset != 12) {

/*  write integer */

#line 1532 ""
	    s_wsfe(&io___213);
#line 1532 ""
	    i__1 = i_nint(&xdata);
#line 1532 ""
	    do_fio(&c__1, (char *)&i__1, (ftnlen)sizeof(integer));
#line 1532 ""
	    e_wsfe();
#line 1533 ""
	} else {

/*  write floating number */

#line 1537 ""
	    s_wsfe(&io___214);
#line 1537 ""
	    do_fio(&c__1, (char *)&xdata, (ftnlen)sizeof(real));
#line 1537 ""
	    e_wsfe();
#line 1538 ""
	}
#line 1539 ""
	if (iset == 3) {
#line 1540 ""
	    *mtrnum = i_nint(&xdata);
#line 1541 ""
	} else if (iset == 4) {
#line 1542 ""
	    *mtrden = i_nint(&xdata);
#line 1543 ""
	}
#line 1544 ""
/* L1: */
#line 1544 ""
    }
#line 1545 ""
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

#line 1553 ""
L2:
#line 1553 ""
    if (*iccount == 128) {
#line 1554 ""
	io___215.ciunit = *iv + 10;
#line 1554 ""
	s_rsfe(&io___215);
#line 1554 ""
	do_fio(&c__1, line, (ftnlen)128);
#line 1554 ""
	e_rsfe();
#line 1555 ""
	if (*(unsigned char *)line == '%') {
#line 1556 ""
	    len = lenstr_(line, &c__128, (ftnlen)128);
#line 1557 ""
	    s_wsfe(&io___217);
#line 1557 ""
	    do_fio(&c__1, line, len);
#line 1557 ""
	    e_wsfe();
#line 1558 ""
	    goto L2;
#line 1559 ""
	}
#line 1560 ""
	*iccount = 0;
#line 1561 ""
    }
#line 1562 ""
    ++(*iccount);

/*  Find next non-blank or end of line */

#line 1566 ""
    for (*iccount = *iccount; *iccount <= 127; ++(*iccount)) {
#line 1567 ""
	if (*(unsigned char *)&line[*iccount - 1] != ' ') {
#line 1567 ""
	    goto L3;
#line 1567 ""
	}
#line 1568 ""
/* L4: */
#line 1568 ""
    }

/*  If here, iccount=128 and need to get a new line */

#line 1572 ""
    goto L2;
#line 1573 ""
L3:

/*  iccount now points to start of number to read */

#line 1577 ""
    i1 = *iccount;
#line 1578 ""
L5:
#line 1578 ""
    getchar_(line, iccount, durq, (ftnlen)128, (ftnlen)1);

/*  Remember that getchar first increments iccount, *then* reads a character. */

#line 1582 ""
    if (i_indx("0123456789.-", durq, (ftnlen)12, (ftnlen)1) > 0) {
#line 1582 ""
	goto L5;
#line 1582 ""
    }
#line 1583 ""
    i2 = *iccount - 1;
#line 1584 ""
    if (i2 < i1) {
#line 1585 ""
	s_wsle(&io___221);
/* Writing concatenation */
#line 1585 ""
	i__1[0] = 7, a__1[0] = "Found \"";
#line 1585 ""
	i__1[1] = 1, a__1[1] = durq;
#line 1585 ""
	i__1[2] = 19, a__1[2] = "\" instead of number";
#line 1585 ""
	s_cat(ch__1, a__1, i__1, &c__3, (ftnlen)27);
#line 1585 ""
	do_lio(&c__9, &c__1, ch__1, (ftnlen)27);
#line 1585 ""
	e_wsle();
#line 1586 ""
	s_stop("1", (ftnlen)1);
#line 1587 ""
    }
#line 1588 ""
    icf = i2 - i1 + 49;
#line 1589 ""
    ici__1.icierr = 0;
#line 1589 ""
    ici__1.iciend = 0;
#line 1589 ""
    ici__1.icirnum = 1;
#line 1589 ""
    ici__1.icirlen = i2 - (i1 - 1);
#line 1589 ""
    ici__1.iciunit = line + (i1 - 1);
/* Writing concatenation */
#line 1589 ""
    i__1[0] = 2, a__1[0] = "(f";
#line 1589 ""
    *(unsigned char *)&ch__3[0] = icf;
#line 1589 ""
    i__1[1] = 1, a__1[1] = ch__3;
#line 1589 ""
    i__1[2] = 3, a__1[2] = ".0)";
#line 1589 ""
    ici__1.icifmt = (s_cat(ch__2, a__1, i__1, &c__3, (ftnlen)6), ch__2);
#line 1589 ""
    s_rsfi(&ici__1);
#line 1589 ""
    do_fio(&c__1, (char *)&(*xdata), (ftnlen)sizeof(real));
#line 1589 ""
    e_rsfi();
#line 1590 ""
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

#line 1602 ""
    i__1 = *len;
#line 1602 ""
    for (i__ = 1; i__ <= i__1; ++i__) {
#line 1603 ""
	if (*(unsigned char *)&string[i__ - 1] == ' ') {
#line 1603 ""
	    goto L1;
#line 1603 ""
	}
#line 1604 ""
	if (i__ == 1) {
#line 1604 ""
	    return 0;
#line 1604 ""
	}
#line 1605 ""
	goto L2;
#line 1606 ""
L1:
#line 1606 ""
	;
#line 1606 ""
    }

/*  If line is all blank, leave it alone */

#line 1610 ""
    return 0;
#line 1611 ""
L2:
#line 1612 ""
    s_copy(string, string + (i__ - 1), string_len, *len - (i__ - 1));
#line 1613 ""
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

#line 1622 ""
    numdqs = 0;
#line 1623 ""
    i__1 = *indx - 1;
#line 1623 ""
    for (i__ = 1; i__ <= i__1; ++i__) {
#line 1624 ""
	if (*(unsigned char *)&lineq[i__ - 1] == '"') {
#line 1624 ""
	    ++numdqs;
#line 1624 ""
	}
#line 1625 ""
/* L1: */
#line 1625 ""
    }
#line 1626 ""
    *yesodd = numdqs % 2 == 1;
#line 1627 ""
    return 0;
} /* oddquotesbefore_ */

