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
#include "namehash.h"

static int _SFFindExistingSlot(SplineFont *sf, int unienc, const char *name );

void __GlyphHashFree(struct glyphnamehash *hash) {
    struct glyphnamebucket *test, *next;
    int i;

    if ( hash==NULL )
return;
    for ( i=0; i<GN_HSIZE; ++i ) {
	for ( test = hash->table[i]; test!=NULL; test = next ) {
	    next = test->next;
	    chunkfree(test,sizeof(struct glyphnamebucket));
	}
    }
}

static void _GlyphHashFree(SplineFont *sf) {

    if ( sf->glyphnames==NULL )
return;
    __GlyphHashFree(sf->glyphnames);
    free(sf->glyphnames);
    sf->glyphnames = NULL;
}

void GlyphHashFree(SplineFont *sf) {
    _GlyphHashFree(sf);
    if ( sf->cidmaster )
	_GlyphHashFree(sf->cidmaster);
}

static void GlyphHashCreate(SplineFont *sf) {
    int i, k, hash;
    SplineFont *_sf;
    struct glyphnamehash *gnh;
    struct glyphnamebucket *new;

    if ( sf->glyphnames!=NULL )
return;
    sf->glyphnames = gnh = calloc(1,sizeof(*gnh));
    k = 0;
    do {
	_sf = k<sf->subfontcnt ? sf->subfonts[k] : sf;
	/* I walk backwards because there are some ttf files where multiple */
	/*  glyphs get the same name. In the cases I've seen only one of these */
	/*  has an encoding. That's the one we want. It will be earlier in the */
	/*  font than the others. If we build the list backwards then it will */
	/*  be the top name in the bucket, and will be the one we return */
	for ( i=_sf->glyphcnt-1; i>=0; --i ) if ( _sf->glyphs[i]!=NULL ) {
	    new = chunkalloc(sizeof(struct glyphnamebucket));
	    new->sc = _sf->glyphs[i];
	    hash = hashname(new->sc->name);
	    new->next = gnh->table[hash];
	    gnh->table[hash] = new;
	}
	++k;
    } while ( k<sf->subfontcnt );
}

SplineChar *SFHashName(SplineFont *sf,const char *name) {
    struct glyphnamebucket *test;

    if ( sf->glyphnames==NULL )
	GlyphHashCreate(sf);

    for ( test=sf->glyphnames->table[hashname(name)]; test!=NULL; test = test->next )
	if ( strcmp(test->sc->name,name)==0 )
return( test->sc );

return( NULL );
}

static int SCUniMatch(SplineChar *sc,int unienc) {
    struct altuni *alt;

    if ( sc->unicodeenc==unienc )
return( true );
    for ( alt=sc->altuni; alt!=NULL; alt=alt->next )
	if ( alt->unienc==unienc )
return( true );

return( false );
}

/* Find the position in the glyph list where this code point/name is found. */
/*  Returns -1 else on error */
int SFFindGID(SplineFont *sf, int unienc, const char *name ) {
    int gid;
    SplineChar *sc;

    if ( unienc!=-1 ) {
	for ( gid=0; gid<sf->glyphcnt; ++gid ) if ( sf->glyphs[gid]!=NULL ) {
	    if ( SCUniMatch(sf->glyphs[gid],unienc) )
return( gid );
	}
    }
    if ( name!=NULL ) {
	sc = SFHashName(sf,name);
	if ( sc!=NULL )
return( sc->orig_pos );
    }

return ( -1 );
}

int SFCIDFindCID(SplineFont *sf, int unienc, const char *name ) {
	// For a given SplineFont *sf, find the index of the SplineChar with code unienc or name *name.
    int j,ret;
	
	// If there is a cidmap or if there are multiple subfonts, do complicated things.
    if ( sf->cidmaster!=NULL || sf->subfontcnt!=0 ) {
		if ( sf->cidmaster!=NULL )
			sf=sf->cidmaster;
    }

	// If things are simple, perform a flat map.
    if ( sf->subfonts==NULL && sf->cidmaster==NULL )
	return( SFFindGID(sf,unienc,name));

	// If the cid lookup from before failed, look through subfonts.
    if ( sf->cidmaster!=NULL )
		sf=sf->cidmaster;
    for ( j=0; j<sf->subfontcnt; ++j )
		if (( ret = SFFindGID(sf,unienc,name))!=-1 )
			return( ret );

	return( -1 );
}

int SFHasCID(SplineFont *sf,int cid) {
    int i;
    /* What subfont (if any) contains this cid? */
    if ( sf->cidmaster!=NULL )
	sf=sf->cidmaster;
    for ( i=0; i<sf->subfontcnt; ++i )
	if ( cid<sf->subfonts[i]->glyphcnt &&
		SCWorthOutputting(sf->subfonts[i]->glyphs[cid]) )
return( i );
    for ( i=0; i<sf->subfontcnt; ++i )
	if ( cid<sf->subfonts[i]->glyphcnt && sf->subfonts[i]->glyphs[cid]!=NULL )
return( i );

return( -1 );
}

SplineChar *SFGetChar(SplineFont *sf, int unienc, const char *name ) {
	// This function presumably finds a glyph matching the code or name supplied. Undefined code is unienc = -1. Undefined name is name = NULL.
    int ind = -1;
    int j;
    char *pt, *start; int ch;

    if ( name==NULL )
		ind = SFCIDFindCID(sf,unienc,NULL);
    else {
		for ( start=(char *) name; *start==' '; ++start );
		for ( pt=start; *pt!='\0' && *pt!='('; ++pt );
		ch = *pt;
		// We truncate any glyph name before parentheses.
		if ( ch=='\0' )
			ind = SFCIDFindCID(sf,unienc,start);
		else {
			char *tmp;
			if ( (tmp = copy(name)) ) {
				tmp[pt-name] = '\0';
				ind = SFCIDFindCID(sf,unienc,tmp+(start-name));
				tmp[pt-name] = ch;
				free(tmp);
			}
		}
    }
    if ( ind==-1 )
		return( NULL );

	// If the typeface is simple, return the result from the flat glyph collection.
    if ( sf->subfonts==NULL && sf->cidmaster==NULL )
		return( sf->glyphs[ind]);

    if ( sf->cidmaster!=NULL )
		sf=sf->cidmaster;

	// Find a subfont that contains the glyph in question.
    j = SFHasCID(sf,ind);
    if ( j==-1 )
		return( NULL );

	return( sf->subfonts[j]->glyphs[ind] );
}

static int _SFFindExistingSlot(SplineFont *sf, int unienc, const char *name ) {
    int gid = -1;
    struct altuni *altuni;

    if ( unienc!=-1 ) {
	for ( gid=sf->glyphcnt-1; gid>=0; --gid ) if ( sf->glyphs[gid]!=NULL ) {
	    if ( sf->glyphs[gid]->unicodeenc==unienc )
	break;
	    for ( altuni=sf->glyphs[gid]->altuni ; altuni!=NULL &&
		    (altuni->unienc!=unienc || altuni->vs!=-1 || altuni->fid!=0);
		    altuni=altuni->next );
	    if ( altuni!=NULL )
	break;
	}
    }
    if ( gid==-1 && name!=NULL ) {
	SplineChar *sc = SFHashName(sf,name);
	if ( sc==NULL )
return( -1 );
	gid = sc->orig_pos;
	if ( gid<0 || gid>=sf->glyphcnt ) {
	    IError("Invalid glyph location when searching for %s", name );
return( -1 );
	}
    }
return( gid );
}

int SFFindExistingSlot(SplineFont *sf, int unienc, const char *name ) {
    int gid = _SFFindExistingSlot(sf,unienc,name);

    if ( gid==-1 || !SCWorthOutputting(sf->glyphs[gid]) )
return( -1 );

return( gid );
}
