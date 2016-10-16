/* Copyright (C) 2000-2012 by George Williams */
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

#if __Mac
# include <ctype.h>
# include "carbon.h"
#else
#undef __Mac
#define __Mac 0
#endif

/* I had always assumed that the mac still believed in 72dpi screens, but I */
/*  see that in geneva under OS/9, the pointsize does not match the pixel */
/*  size of the font. But the dpi is not constant (and the differences */
/*  excede those supplied by rounding errors) varying between 96 and 84dpi */

/* A Mac Resource fork */
/*  http://developer.apple.com/techpubs/mac/MoreToolbox/MoreToolbox-9.html */
/*    begins with a 16 byte header containing: */
/*	resource start offset */
/*	map start offset */
/*	resource length */
/*	map length */
/*    then 256-16 bytes of zeros */
/*    the resource section consists of (many) */
/*	4 byte length count */
/*	resource data	*/
/*    the map section contains */
/*	A copy of the 16 byte header */
/*	a 4 byte mac internal value (I hope) */
/*	another 4 bytes of mac internal values (I hope) */
/*	a 2 byte offset from the start of the map section to the list of resource types */
/*	a 2 byte offset from the start of the map section to the list of resource names */
/*	The resource type list consists of */
/*	    a 2 byte count of the number of resource types (-1) */
/*	    (many copies of) */
/*		a 4 byte resource type ('FOND' for example) */
/*		a 2 byte count of the number of resources of this type (-1) */
/*		a 2 byte offset from the type list start to the resource table */
/*	    a resource table looks like */
/*		a 2 byte offset from the resource name table to a pascal */
/*			string containing this resource's name (or 0xffff for none) */
/*		1 byte of resource flags */
/*		3 bytes of offset from the resource section to the length & */
/*			data of this instance of the resource type */
/*		4 bytes of 0 */
/*	The resource name section consists of */
/*	    a bunch of pascal strings (ie. preceded by a length byte) */

/* The POST resource isn't noticeably documented, it's pretty much a */
/*  straight copy of the pfb file cut up into 0x800 byte chunks. */
/*  (each section of the pfb file has it's own set of chunks, the last may be smaller than 0x800) */
/* The NFNT resource http://developer.apple.com/techpubs/mac/Text/Text-250.html */
/* The FOND resource http://developer.apple.com/techpubs/mac/Text/Text-269.html */
/* The sfnt resource is basically a copy of the ttf file */

/* A MacBinary file */
/*  http://www.lazerware.com/formats/macbinary.html */
/*    begins with a 128 byte header */
/*	(which specifies lengths for data/resource forks) */
/*	(and contains mac type/creator data) */
/*	(and other stuff) */
/*	(and finally a crc checksum) */
/*    is followed by the data section (padded to a mult of 128 bytes) */
/*    is followed by the resource section (padded to a mult of 128 bytes) */

/* Crc code taken from: */
/* http://www.ctan.org/tex-archive/tools/macutils/crc/ */
/* MacBinary files use the same CRC that binhex does (in the MacBinary header) */

/* ******************************** Creation ******************************** */
struct resource {
    uint32 pos;
    uint8 flags;
    uint16 id;
    char *name;
    uint32 nameloc;
    uint32 nameptloc;
};

enum psstyle_flags { psf_bold = 1, psf_italic = 2, psf_outline = 4,
	psf_shadow = 0x8, psf_condense = 0x10, psf_extend = 0x20 };

uint16 _MacStyleCode( const char *styles, SplineFont *sf, uint16 *psstylecode ) {
    unsigned short stylecode= 0, psstyle=0;

    if ( strstrmatch( styles, "Bold" ) || strstrmatch(styles,"Demi") ||
	    strstrmatch( styles,"Heav") || strstrmatch(styles,"Blac") ||
/* A few fonts have German/French styles in their names */
	    strstrmatch( styles,"Fett") || strstrmatch(styles,"Gras") ) {
	stylecode = sf_bold;
	psstyle = psf_bold;
    } else if ( sf!=NULL && sf->weight!=NULL &&
	    (strstrmatch( sf->weight, "Bold" ) || strstrmatch(sf->weight,"Demi") ||
	     strstrmatch( sf->weight,"Heav") || strstrmatch(sf->weight,"Blac") ||
	     strstrmatch( sf->weight,"Fett") || strstrmatch(sf->weight,"Gras")) ) {
	stylecode = sf_bold;
	psstyle = psf_bold;
    }
    /* URW uses four leter abbreviations of Italic and Oblique */
    /* Somebody else uses two letter abbrevs */
    if ( (sf!=NULL && sf->italicangle!=0) ||
	    strstrmatch( styles, "Ital" ) ||
	    strstrmatch( styles, "Obli" ) ||
	    strstrmatch(styles, "Slanted") ||
	    strstrmatch(styles, "Kurs") ||
	    strstr( styles,"It" ) ) {
	stylecode |= sf_italic;
	psstyle |= psf_italic;
    }
    if ( strstrmatch( styles, "Underline" ) ) {
	stylecode |= sf_underline;
    }
    if ( strstrmatch( styles, "Outl" ) ) {
	stylecode |= sf_outline;
	psstyle |= psf_outline;
    }
    if ( strstr(styles,"Shadow")!=NULL ) {
	stylecode |= sf_shadow;
	psstyle |= psf_shadow;
    }
    if ( strstrmatch( styles, "Cond" ) || strstr( styles,"Cn") ||
	    strstrmatch( styles, "Narrow") ) {
	stylecode |= sf_condense;
	psstyle |= psf_condense;
    }
    if ( strstrmatch( styles, "Exte" ) || strstr( styles,"Ex") ) {
	stylecode |= sf_extend;
	psstyle |= psf_extend;
    }
    if ( (psstyle&psf_extend) && (psstyle&psf_condense) ) {
	if ( sf!=NULL )
	    LogError( _("Warning: %s(%s) is both extended and condensed. That's impossible.\n"),
		    sf->fontname, sf->origname );
	else
	    LogError( _("Warning: Both extended and condensed. That's impossible.\n") );
	psstyle &= ~psf_extend;
	stylecode &= ~sf_extend;
    }
    if ( psstylecode!=NULL )
	*psstylecode = psstyle;
return( stylecode );
}

uint16 MacStyleCode( SplineFont *sf, uint16 *psstylecode ) {
    const char *styles;

    if ( sf->cidmaster!=NULL )
	sf = sf->cidmaster;

    if ( sf->macstyle!=-1 ) {
	if ( psstylecode!=NULL )
	    *psstylecode = (sf->macstyle&0x3)|((sf->macstyle&0x6c)>>1);
return( sf->macstyle );
    }

    styles = SFGetModifiers(sf);
return( _MacStyleCode(styles,sf,psstylecode));
}

/* ******************************** Reading ********************************* */


struct kerns {
    unsigned char ch1, ch2;
    short offset;		/* 4.12 */
};


#if __Mac
static SplineFont *HasResourceFork(char *filename,int flags,enum openflags openflags,
	SplineFont *into,EncMap *map) {
    /* If we're on a mac, we can try to see if we've got a real resource fork */
    /* (if we do, copy it into a temporary data file and then manipulate that)*/
    SplineFont *ret;
    FILE *resfork;
    char *tempfn=filename, *pt, *lparen, *respath;

    if (( pt=strrchr(filename,'/'))==NULL ) pt = filename;
    if ( (lparen = strchr(pt,'('))!=NULL && strchr(lparen,')')!=NULL ) {
	tempfn = copy(filename);
	tempfn[lparen-filename] = '\0';
    }
    respath = malloc(strlen(tempfn)+strlen("/..namedfork/rsrc")+1);
    strcpy(respath,tempfn);
    strcat(respath,"/..namedfork/rsrc");
    resfork = fopen(respath,"r");
    if ( resfork==NULL ) {
	strcpy(respath,tempfn);
	strcat(respath,"/rsrc");
	resfork = fopen(respath,"r");
    }
    free(respath);
    if ( tempfn!=filename )
	free(tempfn);
    if ( resfork==NULL )
return( NULL );
    ret = IsResourceFork(resfork,0,filename,flags,openflags,into,map);
    fclose(resfork);
return( ret );
}
#endif
