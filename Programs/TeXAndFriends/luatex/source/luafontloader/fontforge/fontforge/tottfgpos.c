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
#include <utype.h>
#include <ustring.h>

int coverageformatsallowed=3;
int use_second_indic_scripts = false;

#include "ttf.h"

/* This file contains routines to create the otf gpos and gsub tables and their */
/*  attendant subtables */

/* Undocumented fact: ATM (which does kerning for otf fonts in Word) can't handle features with multiple lookups */

/* Undocumented fact: Only one feature with a given tag allowed per script/lang */
/*  So if we have multiple lookups with the same tag they must be merged into */
/*  one feature with many lookups */

/* scripts (for opentype) that I understand */
    /* see also list in lookups.c mapping script tags to friendly names */

static uint32 scripts[][15] = {
/* Arabic */	{ CHR('a','r','a','b'), 0x0600, 0x06ff, 0xfb50, 0xfdff, 0xfe70, 0xfefe },
/* Aramaic */	{ CHR('a','r','a','m'), 0x820, 0x83f },
/* Armenian */	{ CHR('a','r','m','n'), 0x0530, 0x058f, 0xfb13, 0xfb17 },
/* Balinese */	{ CHR('b','a','l','i'), 0x1b00, 0x1b7f },
/* Bengali */	{ CHR('b','e','n','g'), 0x0980, 0x09ff },
/* Bliss symb */{ CHR('b','l','i','s'), 0x12200, 0x124ff },
/* Bopomofo */	{ CHR('b','o','p','o'), 0x3100, 0x312f, 0x31a0, 0x31bf },
/* Braille */	{ CHR('b','r','a','i'), 0x2800, 0x28ff },
/* Buginese */	{ CHR('b','u','g','i'), 0x1a00, 0x1a1f },
/* Buhid */	{ CHR('b','u','h','d'), 0x1740, 0x1753 },
/* Byzantine M*/{ CHR('b','y','z','m'), 0x1d000, 0x1d0ff },
/* Canadian Syl*/{CHR('c','a','n','s'), 0x1400, 0x167f },
/* Carian */     {CHR('c','a','r','i'), 0x0, 0x0 },
/* Cham */       {CHR('c','h','a','m'), 0x0, 0x0 },
/* Cherokee */	{ CHR('c','h','e','r'), 0x13a0, 0x13ff },
/* Cirth */	{ CHR('c','i','r','t'), 0x12080, 0x120ff },
/* CJKIdeogra */{ CHR('h','a','n','i'), 0x3300, 0x9fff, 0xf900, 0xfaff, 0x020000, 0x02ffff },
/* Coptic */	{ CHR('c','o','p','t'), 0x2c80, 0x2cff },
/* Cypriot */	{ CHR('c','p','m','n'), 0x10800, 0x1083f },
/* Cyrillic */	{ CHR('c','y','r','l'), 0x0400, 0x052f, 0x1d2b, 0x1d2b, 0x1d78, 0x1d78,
	0x2de0, 0x2dff, 0xa640, 0xa6ff },
/* Deseret */	{ CHR('d','s','r','t'), 0x10400, 0x1044f },
/* Devanagari */{ CHR('d','e','v','a'), 0x0900, 0x097f },
/* Ethiopic */	{ CHR('e','t','h','i'), 0x1200, 0x139f },
/* Georgian */	{ CHR('g','e','o','r'), 0x1080, 0x10ff },
/* Glagolitic */{ CHR('g','l','a','g'), 0x1080, 0x10ff },
/* Gothic */	{ CHR('g','o','t','h'), 0x10330, 0x1034a },
/* Greek */	{ CHR('g','r','e','k'), 0x0370, 0x03ff, 0x1f00, 0x1fff },
/* Gujarati */	{ CHR('g','u','j','r'), 0x0a80, 0x0aff },
/* Gurmukhi */	{ CHR('g','u','r','u'), 0x0a00, 0x0a7f },
/* Hangul */	{ CHR('h','a','n','g'), 0xac00, 0xd7af, 0x3130, 0x319f, 0xffa0, 0xff9f },
/* Hanunoo */	{ CHR('h','a','n','o'), 0x1720, 0x1734 },
 /* I'm not sure what the difference is between the 'hang' tag and the 'jamo' */
 /*  tag. 'Jamo' is said to be the precomposed forms, but what's 'hang'? */
/* Hebrew */	{ CHR('h','e','b','r'), 0x0590, 0x05ff, 0xfb1e, 0xfb4f },
#if 0	/* Hiragana used to have its own tag, but has since been merged with katakana */
/* Hiragana */	{ CHR('h','i','r','a'), 0x3040, 0x309f },
#endif
/* Hangul Jamo*/{ CHR('j','a','m','o'), 0x1100, 0x11ff, 0x3130, 0x319f, 0xffa0, 0xffdf },
/* Javanese */	{ CHR('j','a','v','a'), 0 },	/* MS has a tag, but there is no unicode range */
/* Katakana */	{ CHR('k','a','n','a'), 0x3040, 0x30ff, 0xff60, 0xff9f },
/* Kayah Li */	{ CHR('k','a','l','i'), 0 },
/* Kannada */	{ CHR('k','n','d','a'), 0x0c80, 0x0cff },
/* Kharosthi */	{ CHR('k','h','a','r'), 0x10a00, 0x10a5f },
/* Khmer */	{ CHR('k','h','m','r'), 0x1780, 0x17ff },
/* Latin */	{ CHR('l','a','t','n'), 0x0041, 0x005a, 0x0061, 0x007a,
	0x00c0, 0x02af, 0x1d00, 0x1eff, 0xfb00, 0xfb0f, 0xff00, 0xff5f, 0xa770, 0xa7ff },
/* Lao */	{ CHR('l','a','o',' '), 0x0e80, 0x0eff },
/* Lepcha */    { CHR('l','e','p','c'), 0 },
/* Limbu */	{ CHR('l','i','m','b'), 0x1900, 0x194f },
/* Linear A */	/*{ CHR('l','i','n','a'), 0x10180, 0x102cf },*/ /* What happened to linear A? */
/* Linear B */	{ CHR('l','i','n','b'), 0x10000, 0x100fa },
/* Lycian */    { CHR('l','y','c','i'), 0 },
/* Lydian */    { CHR('l','y','d','i'), 0 },
/* Malayalam */	{ CHR('m','l','y','m'), 0x0d00, 0x0d7f },
/* Mathematical Alphanumeric Symbols */
		{ CHR('m','a','t','h'), 0x1d400, 0x1d7ff },
/* Mongolian */	{ CHR('m','o','n','g'), 0x1800, 0x18af },
/* Musical */	{ CHR('m','u','s','i'), 0x1d100, 0x1d1ff },
/* Myanmar */	{ CHR('m','y','m','r'), 0x1000, 0x107f },
/* New Tai Lue*/{ CHR('t','a','l','u'), 0 },
/* N'Ko */	{ CHR('n','k','o',' '), 0x07c0, 0x07fa },
/* Ogham */	{ CHR('o','g','a','m'), 0x1680, 0x169f },
/* Ol Chiki */  { CHR('o','l','c','k'), 0 },
/* Old Italic */{ CHR('i','t','a','l'), 0x10300, 0x1031e },
/* Old Permic */{ CHR('p','e','r','m'), 0x10350, 0x1037f },
/* Old Persian cuneiform */
		{ CHR('x','p','e','o'), 0x103a0, 0x103df },
/* Oriya */	{ CHR('o','r','y','a'), 0x0b00, 0x0b7f },
/* Osmanya */	{ CHR('o','s','m','a'), 0x10480, 0x104a9 },
/* Phags-pa */	{ CHR('p','h','a','g'), 0xa840, 0xa87f },
/* Phoenician */{ CHR('p','h','n','x'), 0x10900, 0x1091f },
/* Pollard */	{ CHR('p','l','r','d'), 0x104b0, 0x104d9 },
/* Rejang */    { CHR('r','j','n','g'), 0 },
/* Rongorongo */{ CHR('r','o','r','o'), 0 },
/* Runic */	{ CHR('r','u','n','r'), 0x16a0, 0x16ff },
/* Saurashtra*/ { CHR('s','a','u','r'), 0 },
/* Shavian */	{ CHR('s','h','a','w'), 0x10450, 0x1047f },
/* Sinhala */	{ CHR('s','i','n','h'), 0x0d80, 0x0dff },
/* Sumero-Akkadian Cuneiform */
		{ CHR('x','s','u','x'), 0x12000, 0x1236e },
/* Sundanese */ { CHR('s','u','n','d'), 0 },
/* Syloti Nagri */
		{ CHR('s','y','l','o'), 0xa800, 0xa82f },
/* Syriac */	{ CHR('s','y','r','c'), 0x0700, 0x074f },
/* Tagalog */	{ CHR('t','a','g','l'), 0x1700, 0x1714 },
/* Tagbanwa */	{ CHR('t','a','g','b'), 0x1760, 0x1773 },
/* Tai Le */	{ CHR('t','a','l','e'), 0x1950, 0x1974 },
/* Tai Lu */	{ CHR('t','a','l','u'), 0x1980, 0x19df },
/* Tamil */	{ CHR('t','a','m','l'), 0x0b80, 0x0bff },
/* Telugu */	{ CHR('t','e','l','u'), 0x0c00, 0x0c7f },
/* Tengwar */	{ CHR('t','e','n','g'), 0x12000, 0x1207f },
/* Thaana */	{ CHR('t','h','a','a'), 0x0780, 0x07bf },
/* Thai */	{ CHR('t','h','a','i'), 0x0e00, 0x0e7f },
/* Tibetan */	{ CHR('t','i','b','t'), 0x0f00, 0x0fff },
/* Tifinagh */	{ CHR('t','f','n','g'), 0x2d30, 0x2d7f },
/* Ugaritic */	{ CHR('u','g','a','r'), 0x10380, 0x1039d },
/* Yi */	{ CHR('y','i',' ',' '), 0xa000, 0xa4c6 },
		{ 0 }
};

int ScriptIsRightToLeft(uint32 script) {
    if ( script==CHR('a','r','a','b') || script==CHR('h','e','b','r') ||
	    script==CHR('c','p','m','n') || script==CHR('k','h','a','r') ||
	    script==CHR('s','y','r','c') || script==CHR('t','h','a','a') ||
	    script==CHR('n','k','o',' '))
return( true );

return( false );
}

uint32 ScriptFromUnicode(int u,SplineFont *sf) {
    int s, k;

    if ( u!=-1 ) {
	for ( s=0; scripts[s][0]!=0; ++s ) {
	    for ( k=1; scripts[s][k+1]!=0; k += 2 )
		if ( u>=scripts[s][k] && u<=scripts[s][k+1] )
	    break;
	    if ( scripts[s][k+1]!=0 )
	break;
	}
	if ( scripts[s][0]!=0 ) {
	    uint32 script = scripts[s][0];
	    if ( use_second_indic_scripts ) {
		/* MS has a parallel set of script tags for their new */
		/*  Indic font shaper */
		if ( script == CHR('b','e','n','g' )) script = CHR('b','n','g','2');
		else if ( script == CHR('d','e','v','a' )) script = CHR('d','e','v','2');
		else if ( script == CHR('g','u','j','r' )) script = CHR('g','j','r','2');
		else if ( script == CHR('g','u','r','u' )) script = CHR('g','u','r','2');
		else if ( script == CHR('k','n','d','a' )) script = CHR('k','n','d','2');
		else if ( script == CHR('m','l','y','m' )) script = CHR('m','l','y','2');
		else if ( script == CHR('o','r','y','a' )) script = CHR('o','r','y','2');
		else if ( script == CHR('t','a','m','l' )) script = CHR('t','m','l','2');
		else if ( script == CHR('t','e','l','u' )) script = CHR('t','e','l','2');
	    }
return( script );
	}
    } else if ( sf!=NULL ) {
	if ( sf->cidmaster!=NULL || sf->subfontcnt!=0 ) {
	    if ( sf->cidmaster!=NULL ) sf = sf->cidmaster;
	    if ( strmatch(sf->ordering,"Identity")==0 )
return( DEFAULT_SCRIPT );
	    else if ( strmatch(sf->ordering,"Korean")==0 )
return( CHR('h','a','n','g'));
	    else
return( CHR('h','a','n','i') );
	}
    }

return( DEFAULT_SCRIPT );
}

uint32 SCScriptFromUnicode(SplineChar *sc) {
    char *pt;
    PST *pst;
    SplineFont *sf;
    int i; unsigned uni;
    FeatureScriptLangList *features;

    if ( sc==NULL )
return( DEFAULT_SCRIPT );

    sf = sc->parent;
    if ( sc->unicodeenc!=-1 &&
	    !(sc->unicodeenc>=0xe000 && sc->unicodeenc<0xf8ff) &&
	    !(sc->unicodeenc>=0xf0000 && sc->unicodeenc<0x10ffff))
return( ScriptFromUnicode( sc->unicodeenc,sf ));

    pt = sc->name;
    if ( *pt ) for ( ++pt; *pt!='\0' && *pt!='_' && *pt!='.'; ++pt );
    if ( *pt!='\0' ) {
	char *str = copyn(sc->name,pt-sc->name);
	int uni = sf==NULL || sf->fv==NULL ? UniFromName(str,ui_none,&custom) :
			    UniFromName(str,sf->uni_interp,sf->fv->map->enc);
	free(str);
	if ( uni!=-1 )
return( ScriptFromUnicode( uni,sf ));
    }
    /* Adobe ligature uniXXXXXXXX */
    if ( strncmp(sc->name,"uni",3)==0 && sscanf(sc->name+3,"%4x", &uni)==1 )
return( ScriptFromUnicode( uni,sf ));

    if ( sf==NULL )
return( DEFAULT_SCRIPT );

    if ( sf->cidmaster ) sf=sf->cidmaster;
    else if ( sf->mm!=NULL ) sf=sf->mm->normal;
    for ( i=0; i<2; ++i ) {
	for ( pst=sc->possub; pst!=NULL; pst=pst->next ) {
	    if ( pst->type == pst_lcaret )
	continue;
	    for ( features = pst->subtable->lookup->features; features!=NULL; features=features->next ) {
		if ( features->scripts!=NULL )
return( features->scripts->script );
	    }
	}
    }
return( ScriptFromUnicode( sc->unicodeenc,sf ));
}


SplineChar **SFGlyphsFromNames(SplineFont *sf,char *names) {
    int cnt, ch;
    char *pt, *end;
    SplineChar *sc, **glyphs;

    cnt = 0;
    for ( pt = names; *pt; pt = end+1 ) {
	++cnt;
	end = strchr(pt,' ');
	if ( end==NULL )
    break;
    }

    glyphs = galloc((cnt+1)*sizeof(SplineChar *));
    cnt = 0;
    for ( pt = names; *pt; pt = end+1 ) {
	end = strchr(pt,' ');
	if ( end==NULL )
	    end = pt+strlen(pt);
	ch = *end;
	*end = '\0';
	sc = SFGetChar(sf,-1,pt);
	if ( sc!=NULL && sc->ttf_glyph!=-1 )
	    glyphs[cnt++] = sc;
	*end = ch;
	if ( ch=='\0' )
    break;
    }
    glyphs[cnt] = NULL;
return( glyphs );
}


static int glyphnameinlist(char *haystack,char *name) {
    char *start, *pt;
    int ch, match, slen = strlen(name);

    for ( pt=haystack ; ; ) {
	while ( *pt==' ' ) ++pt;
	if ( *pt=='\0' )
return( false );
	start=pt;
	while ( *pt!=' ' && *pt!='\0' ) ++pt;
	if ( pt-start==slen ) {
	    ch = *pt; *pt='\0';
	    match = strcmp(start,name);
	    *pt = ch;
	    if ( match==0 )
return( true );
	}
    }
}

static int ReferencedByGSUB(SplineChar *sc) {
    PST *pst;
    SplineFont *sf = sc->parent;
    int gid;
    SplineChar *testsc;
    char *name = sc->name;

    /* If it is itself a ligature it will be referenced by GSUB */
    /* (because we store ligatures on the glyph generated) */
    for ( pst=sc->possub; pst!=NULL; pst=pst->next )
	if ( pst->type == pst_ligature )
return( true );

    for ( gid=0; gid<sf->glyphcnt; ++gid ) if ( (testsc=sf->glyphs[gid])!=NULL ) {
	for ( pst=testsc->possub; pst!=NULL; pst=pst->next ) {
	    if ( pst->type==pst_substitution || pst->type==pst_alternate ||
		    pst->type==pst_multiple ) {
		if ( glyphnameinlist(pst->u.mult.components,name) )
return( true );
	    }
	}
    }
return( false );
}

int gdefclass(SplineChar *sc) {
    PST *pst;
    AnchorPoint *ap;

    if ( sc->glyph_class!=0 )
return( sc->glyph_class-1 );

    if ( strcmp(sc->name,".notdef")==0 )
return( 0 );

    /* It isn't clear to me what should be done if a glyph is both a ligature */
    /*  and a mark (There are some greek accent ligatures, it is probably more*/
    /*  important that they be indicated as marks). Here I chose mark rather  */
    /*  than ligature as the mark class is far more likely to be used */
    ap=sc->anchor;
    while ( ap!=NULL && (ap->type==at_centry || ap->type==at_cexit) )
	ap = ap->next;
    if ( ap!=NULL && (ap->type==at_mark || ap->type==at_basemark) )
return( 3 );

    for ( pst=sc->possub; pst!=NULL; pst=pst->next ) {
	if ( pst->type == pst_ligature )
return( 2 );			/* Ligature */
    }

	/* I not quite sure what a componant glyph is. Probably something */
	/*  that is not in the cmap table and is referenced in other glyphs */
	/* (I've never seen it used by others) */
	/* (Note: No glyph in a CID font can be components as all CIDs mean */
	/*  something) (I think) */
    if ( sc->unicodeenc==-1 && sc->dependents!=NULL &&
	    sc->parent->cidmaster!=NULL && !ReferencedByGSUB(sc))
return( 4 );
    else
return( 1 );
}

