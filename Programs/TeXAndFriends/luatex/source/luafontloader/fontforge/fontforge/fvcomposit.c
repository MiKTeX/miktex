/* Copyright (C) 2000-2008 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "fontforgevw.h"
#include <chardata.h>
#include <math.h>
#include <utype.h>
#include <ustring.h>

int accent_offset = 6;
int GraveAcuteCenterBottom = 1;
int PreferSpacingAccents = true;
int CharCenterHighest = 1;

#define BottomAccent	0x300
#define TopAccent	0x345

/* for accents between 0x300 and 345 these are some synonyms */
/* type1 wants accented chars built with accents in the 0x2c? range */
/*  except for grave and acute which live in iso8859-1 range */
/*  this table is ordered on a best try basis */
static const unichar_t accents[][4] = {
    { 0x2cb, 0x300, 0x60 },	/* grave */
    { 0x2ca, 0x301, 0xb4 },	/* acute */
    { 0x2c6, 0x302, 0x5e },	/* circumflex */
    { 0x2dc, 0x303, 0x7e },	/* tilde */
    { 0x2c9, 0x304, 0xaf },	/* macron */
    { 0x305, 0xaf },		/* overline, (macron is suggested as a syn, but it's not quite right) */
    { 0x2d8, 0x306 },		/* breve */
    { 0x2d9, 0x307, '.' },	/* dot above */
    { 0xa8,  0x308 },		/* diaeresis */
    { 0x2c0 },			/* hook above */
    { 0x2da, 0xb0 },		/* ring above */
    { 0x2dd },			/* real acute */
    { 0x2c7 },			/* caron */
    { 0x2c8, 0x384, 0x30d, '\''  },	/* vertical line, tonos */
    { 0x30e, '"' },		/* real vertical line */
    { 0 },			/* real grave */
    { 0 },			/* cand... */		/* 310 */
    { 0 },			/* inverted breve */
    { 0x2bb },			/* turned comma */
    { 0x2bc, 0x313, ',' },	/* comma above */
    { 0x2bd },			/* reversed comma */
    { 0x2bc, 0x315, ',' },	/* comma above right */
    { 0x316, 0x60, 0x2cb },	/* grave below */
    { 0x317, 0xb4, 0x2ca },	/* acute below */
    { 0 },			/* left tack */
    { 0 },			/* right tack */
    { 0 },			/* left angle */
    { 0 },			/* horn, sometimes comma but only if nothing better */
    { 0 },			/* half ring */
    { 0x2d4 },			/* up tack */
    { 0x2d5 },			/* down tack */
    { 0x2d6, 0x31f, '+' },	/* plus below */
    { 0x2d7, 0x320, '-' },	/* minus below */	/* 320 */
    { 0x2b2 },			/* hook */
    { 0 },			/* back hook */
    { 0x323, 0x2d9, '.' },	/* dot below */
    { 0x324, 0xa8 },		/* diaeresis below */
    { 0x325, 0x2da, 0xb0 },	/* ring below */
    { 0x326, 0x2bc, ',' },	/* comma below */
    { 0xb8 },			/* cedilla */
    { 0x2db },			/* ogonek */		/* 0x328 */
    { 0x329, 0x2c8, 0x384, '\''  },	/* vertical line below */
    { 0 },			/* bridge below */
    { 0 },			/* real arch below */
    { 0x32c, 0x2c7 },		/* caron below */
    { 0x32d, 0x2c6, 0x52 },	/* circumflex below */
    { 0x32e, 0x2d8 },		/* breve below */
    { 0 },			/* inverted breve below */
    { 0x330, 0x2dc, 0x7e },	/* tilde below */	/* 0x330 */
    { 0x331, 0xaf, 0x2c9 },	/* macron below */
    { 0x332, '_' },		/* low line */
    { 0 },			/* real low line */
    { 0x334, 0x2dc, 0x7e },	/* tilde overstrike */
    { 0x335, '-' },		/* line overstrike */
    { 0x336, '_' },		/* long line overstrike */
    { 0x337, '/' },		/* short solidus overstrike */
    { 0x338, '/' },		/* long solidus overstrike */	/* 0x338 */
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0x340, 0x60, 0x2cb },	/* tone mark, left of circumflex */ /* 0x340 */
    { 0x341, 0xb4, 0x2ca },	/* tone mark, right of circumflex */
    { 0x342, 0x2dc, 0x7e },	/* perispomeni (tilde) */
    { 0x343, 0x2bc, ',' },	/* koronis */
    { 0 },			/* dialytika tonos (two accents) */
    { 0x37a },			/* ypogegrammeni */
    { 0xffff }
};

int CanonicalCombiner(int uni) {
    /* Translate spacing accents to combiners */
    int j,k;

    /* The above table will use these occasionally, but we don't want to */
    /*  translate them. They aren't accents */
    if ( uni==',' || uni=='\'' || uni=='"' || uni=='~' || uni=='^' || uni=='-' ||
	    uni=='+' || uni=='.' )
return( uni );

    for ( j=0; accents[j][0]!=0xffff; ++j ) {
	for ( k=0; k<4 && accents[j][k]!=0; ++k ) {
	    if ( uni==(int)accents[j][k] ) {
		uni = 0x300+j;
	break;
	    }
	}
	if ( uni>=0x300 && uni<0x370 )
    break;
    }
return( uni );
}



int isaccent(int uni) {

    if ( uni<0x10000 && iscombining(uni) )
return( true );
    if ( uni>=0x2b0 && uni<0x2ff )
return( true );
    if ( uni=='.' || uni==',' || uni==0x60 || uni==0x5e || uni==0x7e ||
	    uni==0xa8 || uni==0xaf || uni==0xb8 || uni==0x384 || uni==0x385 ||
	    (uni>=0x1fbd && uni<=0x1fc1) ||
	    (uni>=0x1fcd && uni<=0x1fcf) ||
	    (uni>=0x1fed && uni<=0x1fef) ||
	    (uni>=0x1ffd && uni<=0x1fff) )
return( true );

return( false );
}

