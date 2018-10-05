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
#include "ustring.h"
#include "utype.h"
#include "gfile.h"
#include "chardata.h"

RefChar *RefCharsCopy(RefChar *ref) {
    RefChar *rhead=NULL, *last=NULL, *cur;

    while ( ref!=NULL ) {
	cur = RefCharCreate();
#ifdef FONTFORGE_CONFIG_TYPE3
	{ struct reflayer *layers = cur->layers; int layer;
	layers = grealloc(layers,ref->layer_cnt*sizeof(struct reflayer));
	memcpy(layers,ref->layers,ref->layer_cnt*sizeof(struct reflayer));
	*cur = *ref;
	cur->layers = layers;
	for ( layer=0; layer<cur->layer_cnt; ++layer ) {
	    cur->layers[layer].splines = NULL;
	    cur->layers[layer].images = NULL;
	}
	}
#else
	{struct reflayer *rl = cur->layers;
	*cur = *ref;
	cur->layers = rl;
	}
#endif
	if ( cur->sc!=NULL )
	    cur->orig_pos = cur->sc->orig_pos;
	cur->next = NULL;
	if ( rhead==NULL )
	    rhead = cur;
	else
	    last->next = cur;
	last = cur;
	ref = ref->next;
    }
return( rhead );
}

static OTLookup *MCConvertLookup(struct sfmergecontext *mc,OTLookup *otl) {
    int l;
    OTLookup *newotl;

    if ( mc==NULL || mc->sf_from==mc->sf_to )
return( otl );		/* No translation needed */

    for ( l=0; l<mc->lcnt; ++l ) {
	if ( mc->lks[l].from == otl )
    break;
    }
    if ( l==mc->lcnt )
return( NULL );
    if ( mc->lks[l].to!=NULL )
return( mc->lks[l].to );

    mc->lks[l].to = newotl = chunkalloc(sizeof(struct lookup_subtable));
    newotl->lookup_name = strconcat(mc->prefix,otl->lookup_name);
    newotl->lookup_type = otl->lookup_type;
    newotl->lookup_flags = otl->lookup_flags;
    newotl->features = FeatureListCopy(otl->features);
    newotl->store_in_afm = otl->store_in_afm;
return( newotl );
}

struct lookup_subtable *MCConvertSubtable(struct sfmergecontext *mc,struct lookup_subtable *sub) {
    int s;
    struct lookup_subtable *newsub;

    if ( mc==NULL || mc->sf_from==mc->sf_to )
return( sub );		/* No translation needed */
    if ( mc->prefix==NULL ) {
	int lcnt, scnt;
	OTLookup *otl;
	struct lookup_subtable *subs;
	int isgpos, doit;
	char *temp;

	/* Not initialized */
	if ( mc->sf_from->cidmaster ) mc->sf_from = mc->sf_from->cidmaster;
	else if ( mc->sf_from->mm!=NULL ) mc->sf_from = mc->sf_from->mm->normal;
	if ( mc->sf_to->cidmaster ) mc->sf_to = mc->sf_to->cidmaster;
	else if ( mc->sf_to->mm!=NULL ) mc->sf_to = mc->sf_to->mm->normal;
	if ( mc->sf_from == mc->sf_to )
return( sub );
	mc->prefix = strconcat(mc->sf_from->fontname,"-");
	for ( doit = 0; doit<2; ++doit ) {
	    lcnt = scnt = 0;
	    for ( isgpos=0; isgpos<2; ++isgpos ) {
		for ( otl = isgpos ? mc->sf_from->gpos_lookups : mc->sf_from->gsub_lookups; otl!=NULL; otl=otl->next ) {
		    if ( doit ) {
			mc->lks[lcnt].from = otl;
			temp = strconcat(mc->prefix,otl->lookup_name);
			mc->lks[lcnt].to = SFFindLookup(mc->sf_to,temp);
			free(temp);
			mc->lks[lcnt].old = mc->lks[lcnt].to!=NULL;
		    }
		    ++lcnt;
		    for ( subs=otl->subtables; subs!=NULL; subs=subs->next ) {
			if ( doit ) {
			    mc->subs[scnt].from = subs;
			    temp = strconcat(mc->prefix,subs->subtable_name);
			    mc->subs[scnt].to = SFFindLookupSubtable(mc->sf_to,temp);
			    free(temp);
			    mc->subs[scnt].old = mc->subs[scnt].to!=NULL;
			}
			++scnt;
		    }
		}
	    }
	    if ( !doit ) {
		mc->lcnt = lcnt; mc->scnt = scnt;
		mc->lks = gcalloc(lcnt,sizeof(struct lookup_cvt));
		mc->subs = gcalloc(scnt,sizeof(struct sub_cvt));
	    }
	}
    }

    for ( s=0; s<mc->scnt; ++s ) {
	if ( mc->subs[s].from == sub )
    break;
    }
    if ( s==mc->scnt )
return( NULL );
    if ( mc->subs[s].to!=NULL )
return( mc->subs[s].to );

    mc->subs[s].to = newsub = chunkalloc(sizeof(struct lookup_subtable));
    newsub->subtable_name = strconcat(mc->prefix,sub->subtable_name);
    newsub->lookup = MCConvertLookup(mc,sub->lookup);
    newsub->anchor_classes = sub->anchor_classes;
    newsub->per_glyph_pst_or_kern = sub->per_glyph_pst_or_kern;
return( newsub );
}

PST *PSTCopy(PST *base,SplineChar *sc,struct sfmergecontext *mc) {
    PST *head=NULL, *last=NULL, *cur;

    for ( ; base!=NULL; base = base->next ) {
	cur = chunkalloc(sizeof(PST));
	*cur = *base;
	cur->subtable = MCConvertSubtable(mc,base->subtable);
	if ( cur->type==pst_ligature ) {
	    cur->u.lig.components = copy(cur->u.lig.components);
	    cur->u.lig.lig = sc;
	} else if ( cur->type==pst_pair ) {
	    cur->u.pair.paired = copy(cur->u.pair.paired);
	    cur->u.pair.vr = chunkalloc(sizeof( struct vr [2]));
	    memcpy(cur->u.pair.vr,base->u.pair.vr,sizeof(struct vr [2]));
	} else if ( cur->type==pst_lcaret ) {
	    cur->u.lcaret.carets = galloc(cur->u.lcaret.cnt*sizeof(uint16));
	    memcpy(cur->u.lcaret.carets,base->u.lcaret.carets,cur->u.lcaret.cnt*sizeof(uint16));
	} else if ( cur->type==pst_substitution || cur->type==pst_multiple || cur->type==pst_alternate )
	    cur->u.subs.variant = copy(cur->u.subs.variant);
	if ( head==NULL )
	    head = cur;
	else
	    last->next = cur;
	last = cur;
    }
return( head );
}

static AnchorPoint *AnchorPointsDuplicate(AnchorPoint *base,SplineChar *sc) {
    AnchorPoint *head=NULL, *last=NULL, *cur;
    AnchorClass *ac;

    for ( ; base!=NULL; base = base->next ) {
	cur = chunkalloc(sizeof(AnchorPoint));
	*cur = *base;
	cur->next = NULL;
	for ( ac=sc->parent->anchor; ac!=NULL; ac=ac->next )
	    if ( strcmp(ac->name,base->anchor->name)==0 )
	break;
	cur->anchor = ac;
	if ( ac==NULL ) {
	    LogError( "No matching AnchorClass for %s", base->anchor->name);
	    chunkfree(cur,sizeof(AnchorPoint));
	} else {
	    if ( head==NULL )
		head = cur;
	    else
		last->next = cur;
	    last = cur;
	}
    }
return( head );
}

static struct altuni *AltUniCopy(struct altuni *altuni,SplineFont *noconflicts) {
    struct altuni *head=NULL, *last=NULL, *cur;

    while ( altuni!=NULL ) {
	if ( noconflicts==NULL || SFGetChar(noconflicts,altuni->unienc,NULL)==NULL ) {
	    cur = chunkalloc(sizeof(struct altuni));
	    cur->unienc = altuni->unienc;
	    cur->vs = altuni->vs;
	    cur->fid = altuni->fid;
	    if ( head==NULL )
		head = cur;
	    else
		last->next = cur;
	    last = cur;
	}
	altuni = altuni->next;
    }
return( head );
}

SplineChar *SplineCharCopy(SplineChar *sc,SplineFont *into,struct sfmergecontext *mc) {
    SplineChar *nsc;
    Layer *layers;
    int layer;
    if (into==NULL) {
      nsc = SplineCharCreate(2);
    } else {
      nsc = SFSplineCharCreate(into);
    }
    layers = nsc->layers;
    *nsc = *sc;		/* We copy the layers just below */
    if (into==NULL || sc->layer_cnt!=into->layer_cnt )
	  layers = grealloc(layers,sc->layer_cnt*sizeof(Layer));
    memcpy(layers,sc->layers,sc->layer_cnt*sizeof(Layer));
    nsc->layers = layers;
    for ( layer = ly_back; layer<sc->layer_cnt; ++layer ) {
	layers[layer].splines = SplinePointListCopy(layers[layer].splines);
	layers[layer].refs = RefCharsCopy(layers[layer].refs);
	layers[layer].images = ImageListCopy(layers[layer].images);
	layers[layer].undoes = NULL;
	layers[layer].redoes = NULL;
    }
    nsc->parent = into;
    nsc->orig_pos = -2;
    nsc->name = copy(sc->name);
    nsc->hstem = StemInfoCopy(nsc->hstem);
    nsc->vstem = StemInfoCopy(nsc->vstem);
    nsc->anchor = AnchorPointsDuplicate(nsc->anchor,nsc);
    nsc->views = NULL;
    nsc->changed = true;
    nsc->dependents = NULL;		/* Fix up later when we know more */
    nsc->layers[ly_fore].undoes = nsc->layers[ly_back].undoes = NULL;
    nsc->layers[ly_fore].redoes = nsc->layers[ly_back].redoes = NULL;
    if ( nsc->ttf_instrs_len!=0 ) {
	nsc->ttf_instrs = galloc(nsc->ttf_instrs_len);
	memcpy(nsc->ttf_instrs,sc->ttf_instrs,nsc->ttf_instrs_len);
    }
    nsc->kerns = NULL;
    nsc->possub = PSTCopy(nsc->possub,nsc,mc);
    nsc->altuni = AltUniCopy(nsc->altuni,into);
return(nsc);
}

static int _SFFindExistingSlot(SplineFont *sf, int unienc, const char *name );


#define GN_HSIZE	257

struct glyphnamebucket {
    SplineChar *sc;
    struct glyphnamebucket *next;
};

struct glyphnamehash {
    struct glyphnamebucket *table[GN_HSIZE];
};

#ifndef __GNUC__
# define __inline__
#endif

static __inline__ int hashname(const char *pt) {
    int val = 0;

    while ( *pt ) {
	val = (val<<3)|((val>>29)&0x7);
	val ^= (unsigned char)(*pt-'!');
	pt++;
    }
    val ^= (val>>16);
    val &= 0xffff;
    val %= GN_HSIZE;
return( val );
}

static void _GlyphHashFree(SplineFont *sf) {
    struct glyphnamebucket *test, *next;
    int i;

    if ( sf->glyphnames==NULL )
return;
    for ( i=0; i<GN_HSIZE; ++i ) {
	for ( test = sf->glyphnames->table[i]; test!=NULL; test = next ) {
	    next = test->next;
	    chunkfree(test,sizeof(struct glyphnamebucket));
	}
    }
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
    sf->glyphnames = gnh = gcalloc(1,sizeof(*gnh));
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

void SFHashGlyph(SplineFont *sf,SplineChar *sc) {
    /* sc just got added to the font. Put it in the lookup */
    int hash;
    struct glyphnamebucket *new;

    if ( sf->glyphnames==NULL )
return;		/* No hash table, nothing to update */

    new = chunkalloc(sizeof(struct glyphnamebucket));
    new->sc = sc;
    hash = hashname(sc->name);
    new->next = sf->glyphnames->table[hash];
    sf->glyphnames->table[hash] = new;
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

/* Find the position in the glyph list where this code point/name is found. */
/*  Returns -1 else on error */
int SFFindGID(SplineFont *sf, int unienc, const char *name ) {
    struct altuni *altuni;
    int gid;
    SplineChar *sc;

    if ( unienc!=-1 ) {
	for ( gid=0; gid<sf->glyphcnt; ++gid ) if ( sf->glyphs[gid]!=NULL ) {
	    if ( sf->glyphs[gid]->unicodeenc == unienc )
return( gid );
	    for ( altuni = sf->glyphs[gid]->altuni; altuni!=NULL; altuni=altuni->next ) {
		if ( altuni->unienc == unienc && altuni->vs==-1 && altuni->fid==0 )
return( gid );
	    }
	}
    }
    if ( name!=NULL ) {
	sc = SFHashName(sf,name);
	if ( sc!=NULL )
return( sc->orig_pos );
    }

return ( -1 );
}

/* Find the position in the current encoding where this code point/name should*/
/*  be found. (or for unencoded glyphs where it is found). Returns -1 else */
int SFFindSlot(SplineFont *sf, EncMap *map, int unienc, const char *name ) {
    int index=-1, pos;
    struct cidmap *cidmap;

    if ( sf->cidmaster!=NULL && !map->enc->is_compact &&
		(cidmap = FindCidMap(sf->cidmaster->cidregistry,
				    sf->cidmaster->ordering,
				    sf->cidmaster->supplement,
				    sf->cidmaster))!=NULL )
	index = NameUni2CID(cidmap,unienc,name);
    if ( index!=-1 )
	/* All done */;
    else if ( (map->enc->is_custom || map->enc->is_compact ||
	    map->enc->is_original) && unienc!=-1 ) {
	if ( unienc<map->enccount && map->map[unienc]!=-1 &&
		sf->glyphs[map->map[unienc]]!=NULL &&
		sf->glyphs[map->map[unienc]]->unicodeenc==unienc )
	    index = unienc;
	else for ( index = map->enccount-1; index>=0; --index ) {
	    if ( (pos = map->map[index])!=-1 && sf->glyphs[pos]!=NULL &&
		    sf->glyphs[pos]->unicodeenc==unienc )
	break;
	}
    } else if ( unienc!=-1 &&
	    ((unienc<0x10000 && map->enc->is_unicodebmp) ||
	     (unienc<0x110000 && map->enc->is_unicodefull))) {
	 index = unienc;
    } else if ( unienc!=-1 ) {
	index = EncFromUni(unienc,map->enc);
	if ( index<0 || index>=map->enccount ) {
	    for ( index=map->enc->char_cnt; index<map->enccount; ++index )
		if ( (pos = map->map[index])!=-1 && sf->glyphs[pos]!=NULL &&
			sf->glyphs[pos]->unicodeenc==unienc )
	    break;
	    if ( index>=map->enccount )
		index = -1;
	}
    }
    if ( index==-1 && name!=NULL ) {
	SplineChar *sc = SFHashName(sf,name);
	if ( sc!=NULL ) index = map->backmap[sc->orig_pos];
	if ( index==-1 ) {
	    unienc = UniFromName(name,sf->uni_interp,map->enc);
	    if ( unienc!=-1 )
return( SFFindSlot(sf,map,unienc,NULL));
	    if ( map->enc->psnames!=NULL ) {
		for ( index = map->enc->char_cnt-1; index>=0; --index )
		    if ( map->enc->psnames[index]!=NULL &&
			    strcmp(map->enc->psnames[index],name)==0 )
return( index );
	    }
	}
    }

return( index );
}

int SFCIDFindCID(SplineFont *sf, int unienc, const char *name ) {
    int j,ret;
    struct cidmap *cidmap;

    if ( sf->cidmaster!=NULL || sf->subfontcnt!=0 ) {
	if ( sf->cidmaster!=NULL )
	    sf=sf->cidmaster;
	cidmap = FindCidMap(sf->cidregistry,sf->ordering,sf->supplement,sf);
	ret = NameUni2CID(cidmap,unienc,name);
	if ( ret!=-1 )
return( ret );
    }

    if ( sf->subfonts==NULL && sf->cidmaster==NULL )
return( SFFindGID(sf,unienc,name));

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
    int ind;
    int j;

    ind = SFCIDFindCID(sf,unienc,name);
    if ( ind==-1 )
return( NULL );

    if ( sf->subfonts==NULL && sf->cidmaster==NULL )
return( sf->glyphs[ind]);

    if ( sf->cidmaster!=NULL )
	sf=sf->cidmaster;

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
