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
#include "pfaedit.h"
#include <math.h>
#include <time.h>
#include <utype.h>
#include <ustring.h>
#include <gimage.h>		/* For COLOR_DEFAULT */

#include "ttf.h"

/* This file contains routines to generate non-standard true/opentype tables */
/* The first is the 'PfEd' table containing PfaEdit specific information */
/*	glyph comments & colours ... perhaps other info later */

/* ************************************************************************** */
/* *************************    The 'PfEd' table    ************************* */
/* *************************         Output         ************************* */
/* ************************************************************************** */

#include "PfEd.h"	/* This describes the format of the 'PfEd' table */
			/*  and its many subtables. */

#define MAX_SUBTABLE_TYPES	20

/* *************************    The 'PfEd' table    ************************* */
/* *************************          Input         ************************* */

static void pfed_readfontcomment(FILE *ttf,struct ttfinfo *info,uint32 base,
	uint32 tag) {
    int len;
    char *start, *pt, *end;
    int use_utf8;

    fseek(ttf,base,SEEK_SET);
    use_utf8 = getushort(ttf);
    if ( use_utf8!=0 && use_utf8!=1 )
return;			/* Bad version number */
    len = getushort(ttf);
    start = pt = galloc(len+1);

    end = pt+len;
    if ( use_utf8 ) {
	while ( pt<end )
	    *pt++ = getc(ttf);
    } else {
	while ( pt<end )
	    *pt++ = getushort(ttf);
    }
    *pt = '\0';
    if ( !use_utf8 ) {
	pt = latin1_2_utf8_copy(info->fontcomments);
	free(start);
	start = pt;
    }
    if ( tag==flog_TAG )
	info->fontlog = start;
    else
	info->fontcomments = start;
}

static char *pfed_read_utf8(FILE *ttf, uint32 start) {
    int ch, len;
    char *str, *pt;

    fseek( ttf, start, SEEK_SET);
    len = 0;
    while ( (ch=getc(ttf))!='\0' && ch!=EOF )
	++len;
    fseek( ttf, start, SEEK_SET);
    str = pt = galloc(len+1);
    while ( (ch=getc(ttf))!='\0' && ch!=EOF )
	*pt++ = ch;
    *pt = '\0';
return( str );
}

static char *pfed_read_ucs2_len(FILE *ttf,uint32 offset,int len) {
    char *pt, *str;
    uint32 uch, uch2;
    int i;

    if ( len<0 )
return( NULL );

    len>>=1;
    pt = str = galloc(3*len);
    fseek(ttf,offset,SEEK_SET);
    for ( i=0; i<len; ++i ) {
	uch = getushort(ttf);
	if ( uch>=0xd800 && uch<0xdc00 ) {
	    uch2 = getushort(ttf);
	    if ( uch2>=0xdc00 && uch2<0xe000 )
		uch = ((uch-0xd800)<<10) | (uch2&0x3ff);
	    else {
		pt = utf8_idpb(pt,uch);
		uch = uch2;
	    }
	}
	pt = utf8_idpb(pt,uch);
    }
    *pt++ = 0;
return( grealloc(str,pt-str) );
}

static char *pfed_read_utf8_len(FILE *ttf,uint32 offset,int len) {
    char *pt, *str;
    int i;

    if ( len<0 )
return( NULL );

    pt = str = galloc(len+1);
    fseek(ttf,offset,SEEK_SET);
    for ( i=0; i<len; ++i )
	*pt++ = getc(ttf);
    *pt = '\0';
return( str );
}

static void pfed_readcvtcomments(FILE *ttf,struct ttfinfo *info,uint32 base ) {
    int count, i;
    uint16 *offsets;

    fseek(ttf,base,SEEK_SET);
    if ( getushort(ttf)!=0 )
return;			/* Bad version number */
    count = getushort(ttf);
    
    offsets = galloc(count*sizeof(uint16));
    info->cvt_names = galloc((count+1)*sizeof(char *));
    for ( i=0; i<count; ++i )
	offsets[i] = getushort(ttf);
    for ( i=0; i<count; ++i ) {
	if ( offsets[i]==0 )
	    info->cvt_names[i] = NULL;
	else
	    info->cvt_names[i] = pfed_read_utf8(ttf,base+offsets[i]);
    }
    free(offsets);
}

static void pfed_readglyphcomments(FILE *ttf,struct ttfinfo *info,uint32 base) {
    int n, i, j;
    struct grange { int start, end; uint32 offset; } *grange;
    uint32 offset, next;
    int use_utf8;

    fseek(ttf,base,SEEK_SET);
    use_utf8 = getushort(ttf);
    if ( use_utf8!=0 && use_utf8!=1 )
return;			/* Bad version number */
    n = getushort(ttf);
    grange = galloc(n*sizeof(struct grange));
    for ( i=0; i<n; ++i ) {
	grange[i].start = getushort(ttf);
	grange[i].end = getushort(ttf);
	grange[i].offset = getlong(ttf);
	if ( grange[i].start>grange[i].end || grange[i].end>info->glyph_cnt ) {
	    LogError( _("Bad glyph range specified in glyph comment subtable of PfEd table\n") );
	    grange[i].start = 1; grange[i].end = 0;
	}
    }
    for ( i=0; i<n; ++i ) {
	for ( j=grange[i].start; j<=grange[i].end; ++j ) {
	    fseek( ttf,base+grange[i].offset+(j-grange[i].start)*sizeof(uint32),SEEK_SET);
	    offset = getlong(ttf);
	    next = getlong(ttf);
	    if ( use_utf8 )
		info->chars[j]->comment = pfed_read_utf8_len(ttf,base+offset,next-offset);
	    else
		info->chars[j]->comment = pfed_read_ucs2_len(ttf,base+offset,next-offset);
	    if ( info->chars[j]->comment == NULL )
		LogError("Invalid comment string (negative length?) in 'PfEd' table for glyph %s.",
			info->chars[j]->name );
	}
    }
    free(grange);
}

static void pfed_readcolours(FILE *ttf,struct ttfinfo *info,uint32 base) {
    int n, i, j, start, end;
    uint32 col;

    fseek(ttf,base,SEEK_SET);
    if ( getushort(ttf)!=0 )
return;			/* Bad version number */
    n = getushort(ttf);
    for ( i=0; i<n; ++i ) {
	start = getushort(ttf);
	end = getushort(ttf);
	col = getlong(ttf);
	if ( start>end || end>info->glyph_cnt )
	    LogError( _("Bad glyph range specified in colour subtable of PfEd table\n") );
	else {
	    for ( j=start; j<=end; ++j )
		info->chars[j]->color = col;
	}
    }
}

static void pfed_readlookupnames(FILE *ttf,struct ttfinfo *info,uint32 base,
	OTLookup *lookups) {
    OTLookup *otl;
    struct lookup_subtable *sub;
    AnchorClass *ac;
    int i, j, k, n, s, a;
    struct lstruct { int name_off, subs_off; } *ls, *ss, *as;

    fseek(ttf,base,SEEK_SET);
    if ( getushort(ttf)!=0 )
return;			/* Bad version number */
    n = getushort(ttf);
    ls = galloc(n*sizeof(struct lstruct));
    for ( i=0; i<n; ++i ) {
	ls[i].name_off = getushort(ttf);
	ls[i].subs_off = getushort(ttf);
    }
    for ( i=0, otl=lookups; i<n && otl!=NULL; ++i, otl=otl->next ) {
	if ( ls[i].name_off!=0 ) {
	    free( otl->lookup_name );
	    otl->lookup_name = pfed_read_utf8(ttf,base+ls[i].name_off);
	}
	if ( ls[i].subs_off!=0 ) {
	    fseek(ttf,base+ls[i].subs_off,SEEK_SET);
	    s = getushort(ttf);
	    ss = galloc(s*sizeof(struct lstruct));
	    for ( j=0; j<s; ++j ) {
		ss[j].name_off = getushort(ttf);
		ss[j].subs_off = getushort(ttf);
	    }
	    for ( j=0, sub=otl->subtables; j<s && sub!=NULL; ++j, sub=sub->next ) {
		if ( ss[j].name_off!=0 ) {
		    free( sub->subtable_name );
		    sub->subtable_name = pfed_read_utf8(ttf,base+ss[j].name_off);
		}
		if ( ss[j].subs_off!=0 ) {
		    if ( !sub->anchor_classes )
			LogError("Whoops, attempt to name anchors in a subtable which doesn't contain any\n");
		    else {
			fseek(ttf,base+ss[j].subs_off,SEEK_SET);
			a = getushort(ttf);
			as = galloc(a*sizeof(struct lstruct));
			for ( k=0; k<a; ++k ) {
			    as[k].name_off = getushort(ttf);
			}
			k=0;
			for ( ac=info->ahead; ac!=NULL; ac=ac->next ) {
			    if ( ac->subtable==sub ) {
				if ( as[k].name_off!=0 ) {
				    free( ac->name );
				    ac->name = pfed_read_utf8(ttf,base+as[k].name_off);
				}
			        ++k;
			    }
			}
			free(as);
		    }
		}
	    }
	    /* I guess it's ok for some subtables to be unnamed, so no check for sub!=NULL */
	    if ( j<s )
		LogError("Whoops, more names than subtables of lookup %s\n", otl->lookup_name );
	    free(ss);
	}
    }
    /* I guess it's ok for some lookups to be unnamed, so no check for otf!=NULL */
    if ( i<n )
	LogError("Whoops, more names than lookups\n" );
    free(ls);
}

static float pfed_get_coord(FILE *ttf,int mod) {
    if ( mod==V_B )
return( (float) (signed char) getc(ttf) );
    else if ( mod==V_S )
return( (float) (short) getushort(ttf));
    else if ( mod==V_F )
return( getlong(ttf)/256.0 );
    else {
	LogError( "Bad data type in contour verb in 'PfEd'\n");
return( 0 );
    }
}

static void pfed_read_normal_contour(FILE *ttf,SplineSet *ss,
	uint32 base, int type) {
    SplinePoint *sp, *current;
    int verb, v, m;
    float offx, offy, offx1 = 0, offy1 = 0, offx2, offy2;
    int was_implicit=false;

    fseek(ttf,base,SEEK_SET);

    verb = getc(ttf);
    if ( COM_VERB(verb)!=V_MoveTo ) {
	LogError("Whoops, contours must begin with a move to\n" );
	ss->first = ss->last = SplinePointCreate(0,0);
return;
    }
    offx = pfed_get_coord(ttf,COM_MOD(verb));
    offy = pfed_get_coord(ttf,COM_MOD(verb));
    ss->first = current = SplinePointCreate(offx,offy);
    forever {
	verb = getc(ttf);
	v = COM_VERB(verb); m = COM_MOD(verb);
	if ( m==3 ) {
	    LogError("Bad data modifier in contour command in 'PfEd'\n" );
    break;
	}
	if ( verb==V_Close || verb==V_End )
    break;
	else if ( v>=V_LineTo && v<=V_VLineTo ) {
	    offx = offy = 0;
	    if ( v==V_LineTo ) {
		offx = pfed_get_coord(ttf,m);
		offy = pfed_get_coord(ttf,m);
	    } else if ( v==V_HLineTo )
		offx = pfed_get_coord(ttf,m);
	    else if ( v==V_VLineTo )
		offy = pfed_get_coord(ttf,m);
	    sp = SplinePointCreate(current->me.x+offx,current->me.y+offy);
	} else if ( v>=V_QCurveTo && v<=V_QVImplicit ) {
	    int will_be_implicit = true;
	    offx = offy = 0; offx1 = offy1 = 1;	/* else implicit points become straight lines too soon */
	    if ( v==V_QCurveTo ) {
		offx = pfed_get_coord(ttf,m);
		offy = pfed_get_coord(ttf,m);
		offx1 = pfed_get_coord(ttf,m);
		offy1 = pfed_get_coord(ttf,m);
		will_be_implicit = false;
	    } else if ( v==V_QImplicit ) {
		offx = pfed_get_coord(ttf,m);
		offy = pfed_get_coord(ttf,m);
	    } else if ( v==V_QHImplicit ) {
		offx = pfed_get_coord(ttf,m);
	    } else if ( v==V_QVImplicit ) {
		offy = pfed_get_coord(ttf,m);
	    }
	    
	    current->nextcp.x = current->me.x+offx;
	    current->nextcp.y = current->me.y+offy;
	    current->nonextcp = false;
	    sp = SplinePointCreate(current->nextcp.x+offx1,current->nextcp.y+offy1);
	    sp->prevcp = current->nextcp;
	    sp->noprevcp = false;
	    if ( was_implicit ) {
		current->me.x = (current->prevcp.x + current->nextcp.x)/2;
		current->me.y = (current->prevcp.y + current->nextcp.y)/2;
		SplineRefigure(current->prev);
	    }
	    was_implicit = will_be_implicit;
	} else if ( v>=V_CurveTo && v<=V_HVCurveTo ) {
	    offx=offy=offx2=offy2=0;
	    if ( v==V_CurveTo ) {
		offx = pfed_get_coord(ttf,m);
		offy = pfed_get_coord(ttf,m);
		offx1 = pfed_get_coord(ttf,m);
		offy1 = pfed_get_coord(ttf,m);
		offx2 = pfed_get_coord(ttf,m);
		offy2 = pfed_get_coord(ttf,m);
	    } else if ( v==V_VHCurveTo ) {
		offy = pfed_get_coord(ttf,m);
		offx1 = pfed_get_coord(ttf,m);
		offy1 = pfed_get_coord(ttf,m);
		offx2 = pfed_get_coord(ttf,m);
	    } else if ( v==V_HVCurveTo ) {
		offx = pfed_get_coord(ttf,m);
		offx1 = pfed_get_coord(ttf,m);
		offy1 = pfed_get_coord(ttf,m);
		offy2 = pfed_get_coord(ttf,m);
	    }
	    current->nextcp.x = current->me.x+offx;
	    current->nextcp.y = current->me.y+offy;
	    current->nonextcp = false;
	    sp = SplinePointCreate(current->nextcp.x+offx1+offx2,current->nextcp.y+offy1+offy2);
	    sp->prevcp.x = current->nextcp.x+offx1;
	    sp->prevcp.y = current->nextcp.y+offy1;
	    sp->noprevcp = false;
	} else {
	    LogError("Whoops, unexpected verb in contour %d.%d\n", v, m );
    break;
	}
	SplineMake(current,sp,type==2);
	current = sp;
    }
    if ( verb==V_Close ) {
	if ( was_implicit ) {
	    current->me.x = (current->prevcp.x + ss->first->nextcp.x)/2;
	    current->me.y = (current->prevcp.y + ss->first->nextcp.y)/2;
	}
	if ( current->me.x==ss->first->me.x && current->me.y==ss->first->me.y ) {
	    current->prev->to = ss->first;
	    ss->first->prev = current->prev;
	    ss->first->prevcp = current->prevcp;
	    ss->first->noprevcp = current->noprevcp;
	    SplinePointFree(current);
	} else
	    SplineMake(current,ss->first,type==2);
	ss->last = ss->first;
    } else {
	ss->last = current;
    }
    SPLCatagorizePoints(ss);
}


static void pfed_read_glyph_layer(FILE *ttf,struct ttfinfo *info,Layer *ly,
	uint32 base, int type, int version) {
    int cc, ic, rc, i, j;
    SplineSet *ss;
    struct contours { int data_off, name_off; SplineSet *ss; } *contours;
    int gid;
    RefChar *last, *cur;

    fseek(ttf,base,SEEK_SET);
    cc = getushort(ttf);
    rc = 0;
    if ( version==1 )
	rc = getushort(ttf);
    ic = getushort(ttf);
    contours = galloc(cc*sizeof(struct contours));
    for ( i=0; i<cc; ++i ) {
	contours[i].data_off = getushort(ttf);
	contours[i].name_off = getushort(ttf);
    }
    last = NULL;
    for ( i=0; i<rc; ++i ) {
	cur = RefCharCreate();
	for ( j=0; j<6; ++j )
	    cur->transform[j] = getlong(ttf)/32768.0;
	gid = getushort(ttf);
	cur->sc = info->chars[gid];
	cur->orig_pos = gid;
	cur->unicode_enc = cur->sc->unicodeenc;
	if ( last==NULL )
	    ly->refs = cur;
	else
	    last->next = cur;
	last = cur;
    }
    
    ss = ly->splines;			/* Only relevant for spiros where they live in someone else's layer */
    for ( i=0; i<cc; ++i ) {
	if ( type!=1 ) {		/* Not spiros */
	    contours[i].ss = chunkalloc(sizeof(SplineSet));
	    if ( i==0 )
		ly->splines = contours[i].ss;
	    else
		contours[i-1].ss->next = contours[i].ss;
	    if ( contours[i].name_off!=0 )
		contours[i].ss->contour_name = pfed_read_utf8(ttf,base+contours[i].name_off);
	    pfed_read_normal_contour(ttf,contours[i].ss,base+contours[i].data_off,type);
	}
    }
    free(contours);
}

static void pfed_readguidelines(FILE *ttf,struct ttfinfo *info,uint32 base) {
    int i,v,h,off;
    int version;
    SplinePoint *sp, *nsp;
    SplineSet *ss;

    fseek(ttf,base,SEEK_SET);
    version = getushort(ttf);
    if ( version>1 )
return;			/* Bad version number */
    v = getushort(ttf);
    h = getushort(ttf);
    (void) getushort(ttf);
    off = getushort(ttf);

    if ( off!=0 ) {
	pfed_read_glyph_layer(ttf,info,&info->guidelines,base+off,info->to_order2?2:3,version);
    } else {
	struct npos { int pos; int offset; } *vs, *hs;
	vs = galloc(v*sizeof(struct npos));
	hs = galloc(h*sizeof(struct npos));
	for ( i=0; i<v; ++i ) {
	    vs[i].pos = (short) getushort(ttf);
	    vs[i].offset = getushort(ttf);
	}
	for ( i=0; i<h; ++i ) {
	    hs[i].pos = (short) getushort(ttf);
	    hs[i].offset = getushort(ttf);
	}
	for ( i=0; i<v; ++i ) {
	    sp = SplinePointCreate(vs[i].pos,-info->emsize);
	    nsp = SplinePointCreate(vs[i].pos,2*info->emsize);
	    SplineMake(sp,nsp,info->to_order2);
	    ss = chunkalloc(sizeof(SplineSet));
	    ss->first = sp; ss->last = nsp;
	    if ( vs[i].offset!=0 )
		ss->contour_name = pfed_read_utf8(ttf,base+vs[i].offset);
	    ss->next = info->guidelines.splines;
	    info->guidelines.splines = ss;
	}
	for ( i=0; i<h; ++i ) {
	    sp = SplinePointCreate(-info->emsize,hs[i].pos);
	    nsp = SplinePointCreate(2*info->emsize,hs[i].pos);
	    SplineMake(sp,nsp,info->to_order2);
	    ss = chunkalloc(sizeof(SplineSet));
	    ss->first = sp; ss->last = nsp;
	    if ( hs[i].offset!=0 )
		ss->contour_name = pfed_read_utf8(ttf,base+hs[i].offset);
	    ss->next = info->guidelines.splines;
	    info->guidelines.splines = ss;
	}
	SPLCatagorizePoints(info->guidelines.splines);
	free(vs); free(hs);
    }
}

static void pfed_redo_refs(SplineChar *sc,int layer) {
    RefChar *refs;

    sc->ticked = true;
    for ( refs=sc->layers[layer].refs; refs!=NULL; refs=refs->next ) {
	if ( !refs->sc->ticked )
	    pfed_redo_refs(refs->sc,layer);
	SCReinstanciateRefChar(sc,refs,layer);
    }
}

static void pfed_read_layer(FILE *ttf,struct ttfinfo *info,int layer,int type, uint32 base,
	uint32 start,int version) {
    uint32 *loca = gcalloc(info->glyph_cnt,sizeof(uint32));
    int i,j;
    SplineChar *sc;
    int rcnt;
    struct range { int start, last; uint32 offset; } *ranges;

    fseek(ttf,start,SEEK_SET);
    rcnt = getushort(ttf);
    ranges = galloc(rcnt*sizeof(struct range));
    for ( i=0; i<rcnt; ++i ) {
	ranges[i].start  = getushort(ttf);
	ranges[i].last   = getushort(ttf);
	ranges[i].offset = getlong(ttf);
    }
    for ( i=0; i<rcnt; ++i ) {
	fseek(ttf,base+ranges[i].offset,SEEK_SET);
	for ( j=ranges[i].start; j<=ranges[i].last; ++j )
	    loca[j] = getlong(ttf);
	for ( j=ranges[i].start; j<=ranges[i].last; ++j ) {
	    Layer *ly;
	    sc = info->chars[j];
	    ly = &sc->layers[layer];
	    if ( loca[j]!=0 )
		pfed_read_glyph_layer(ttf,info,ly,base+loca[j],type,version);
	}
    }
    free(ranges); free(loca);

    for ( i=0; i<info->glyph_cnt; ++i ) if ( info->chars[i]!=NULL )
	info->chars[i]->ticked = false;
    for ( i=0; i<info->glyph_cnt; ++i ) if ( info->chars[i]!=NULL )
	pfed_redo_refs(info->chars[i],layer);
}

static void pfed_readotherlayers(FILE *ttf,struct ttfinfo *info,uint32 base) {
    int i, l, lcnt, spiro_index, gid;
    int version;
    struct layer_info { int type, name_off, data_off, sf_layer; char *name; } *layers;
    int non_spiro_cnt=0;
    SplineChar *sc;

    fseek(ttf,base,SEEK_SET);
    version = getushort(ttf);
    if ( version>1 )
return;			/* Bad version number */
    lcnt = getushort(ttf);
    layers = galloc(lcnt*sizeof(struct layer_info));
    for ( i=0; i<lcnt; ++i ) {
	layers[i].type     = getushort(ttf);
	layers[i].name_off = getushort(ttf);
	layers[i].data_off = getlong(ttf);
	layers[i].sf_layer = -1;
    }
    spiro_index = -1;
    non_spiro_cnt = 0;
    for ( i=0; i<lcnt; ++i ) {
	if ( layers[i].name_off==0 )
	    layers[i].name = copy("Unnamed");
	else {
	    layers[i].name = pfed_read_utf8(ttf,base+layers[i].name_off);
	    if ( layers[i].type==1 && strcmp(layers[i].name,"Spiro")==0 )
		spiro_index = i;
	}
	if ( layers[i].type==2 || layers[i].type==3 || layers[i].type==0x102 || layers[i].type==0x103 )
	    ++non_spiro_cnt;
    }
    if ( spiro_index==-1 ) {
	for ( i=0; i<lcnt; ++i )
	    if ( layers[i].type==1 ) {
		spiro_index=i;
	break;
	    }
    }

    if ( non_spiro_cnt!=0 ) {
	info->layer_cnt = non_spiro_cnt+1;
	info->layers = gcalloc(info->layer_cnt+1,sizeof(LayerInfo));
	info->layers[ly_back].background = true;
	info->layers[ly_fore].order2 = info->to_order2;
	info->layers[ly_fore].background = false;
	l = i = 0;
	if ( (layers[i].type&0xff)==1 )
	    ++i;
	if ( layers[i].type&0x100 ) {
	    /* first layer output is foreground, so it can't replace the background layer */
	    ++info->layer_cnt;
	    l = 2;
	    info->layers[ly_back].order2 = info->to_order2;
	}
	for ( ; i<lcnt; ++i ) if ( (layers[i].type&0xff)==2 || (layers[i].type&0xff)==3 ) {
	    info->layers[l].name   = layers[i].name;
	    layers[i].name = NULL;
	    layers[i].sf_layer = l;
	    info->layers[l].order2 = (layers[i].type&0xff)==2;
	    info->layers[l].background = (layers[i].type&0x100)?0:1;
	    if ( l==0 ) l=2; else ++l;
	}
	if ( info->layer_cnt!=2 ) {
	    for ( gid = 0; gid<info->glyph_cnt; ++gid ) if ((sc=info->chars[gid])!=NULL ) {
		sc->layers = grealloc(sc->layers,info->layer_cnt*sizeof(Layer));
		memset(sc->layers+2,0,(info->layer_cnt-2)*sizeof(Layer));
		sc->layer_cnt = info->layer_cnt;
	    }
	}
    }
    if ( spiro_index!=-1 )
	pfed_read_layer(ttf,info,ly_fore,layers[spiro_index].type,base,base+layers[spiro_index].data_off,version);
    for ( i=0; i<lcnt; ++i ) if ( layers[i].sf_layer!=-1 ) {
	pfed_read_layer(ttf,info,layers[i].sf_layer,layers[i].type&0xff,
		base,base+layers[i].data_off,version);
    }
    for ( i=0; i<lcnt; ++i )
	free( layers[i].name );
    free( layers );
}

void pfed_read(FILE *ttf,struct ttfinfo *info) {
    int n,i;
    struct tagoff { uint32 tag, offset; } tagoff[MAX_SUBTABLE_TYPES+30];

    fseek(ttf,info->pfed_start,SEEK_SET);

    if ( getlong(ttf)!=0x00010000 )
return;
    n = getlong(ttf);
    if ( n>=MAX_SUBTABLE_TYPES+30 )
	n = MAX_SUBTABLE_TYPES+30;
    for ( i=0; i<n; ++i ) {
	tagoff[i].tag = getlong(ttf);
	tagoff[i].offset = getlong(ttf);
    }
    for ( i=0; i<n; ++i ) switch ( tagoff[i].tag ) {
      case fcmt_TAG: case flog_TAG:
	pfed_readfontcomment(ttf,info,info->pfed_start+tagoff[i].offset, tagoff[i].tag);
      break;
      case cvtc_TAG:
	pfed_readcvtcomments(ttf,info,info->pfed_start+tagoff[i].offset);
      break;
      case cmnt_TAG:
	pfed_readglyphcomments(ttf,info,info->pfed_start+tagoff[i].offset);
      break;
      case colr_TAG:
	pfed_readcolours(ttf,info,info->pfed_start+tagoff[i].offset);
      break;
      case GPOS_TAG:
	pfed_readlookupnames(ttf,info,info->pfed_start+tagoff[i].offset,info->gpos_lookups);
      break;
      case GSUB_TAG:
	pfed_readlookupnames(ttf,info,info->pfed_start+tagoff[i].offset,info->gsub_lookups);
      break;
      case layr_TAG:
	pfed_readotherlayers(ttf,info,info->pfed_start+tagoff[i].offset);
      break;
      case guid_TAG:
	pfed_readguidelines(ttf,info,info->pfed_start+tagoff[i].offset);
      break;
      default:
	LogError( _("Unknown subtable '%c%c%c%c' in 'PfEd' table, ignored\n"),
		tagoff[i].tag>>24, (tagoff[i].tag>>16)&0xff, (tagoff[i].tag>>8)&0xff, tagoff[i].tag&0xff );
      break;
    }
}

/* 'TeX ' table format is as follows...				 */
/* uint32  version number 0x00010000				 */
/* uint32  subtable count					 */
/* struct { uint32 tab, offset } tag/offset for first subtable	 */
/* struct { uint32 tab, offset } tag/offset for second subtable	 */
/* ...								 */

/* 'TeX ' 'ftpm' font parameter subtable format			 */
/*  short version number 0					 */
/*  parameter count						 */
/*  array of { 4chr tag, value }				 */

/* 'TeX ' 'htdp' per-glyph height/depth subtable format		 */
/*  short  version number 0					 */
/*  short  glyph-count						 */
/*  array[glyph-count] of { int16 height,depth }		 */

/* 'TeX ' 'itlc' per-glyph italic correction subtable		 */
/*  short  version number 0					 */
/*  short  glyph-count						 */
/*  array[glyph-count] of int16 italic_correction		 */

/* !!!!!!!!!!! OBSOLETE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* 'TeX ' 'sbsp' per-glyph sub/super script positioning subtable */
/*  short  version number 0					 */
/*  short  glyph-count						 */
/*  array[glyph-count] of { int16 sub,super }			 */

#undef MAX_SUBTABLE_TYPES
#define MAX_SUBTABLE_TYPES	4

struct TeX_subtabs {
    int next;
    struct {
	FILE *data;
	uint32 tag;
	uint32 offset;
    } subtabs[MAX_SUBTABLE_TYPES];
};

static uint32 tex_text_params[] = {
    TeX_Slant,
    TeX_Space,
    TeX_Stretch,
    TeX_Shrink,
    TeX_XHeight,
    TeX_Quad,
    TeX_ExtraSp,
    0
};
static uint32 tex_math_params[] = {
    TeX_Slant,
    TeX_Space,
    TeX_Stretch,
    TeX_Shrink,
    TeX_XHeight,
    TeX_Quad,
    TeX_MathSp,
    TeX_Num1,
    TeX_Num2,
    TeX_Num3,
    TeX_Denom1,
    TeX_Denom2,
    TeX_Sup1,
    TeX_Sup2,
    TeX_Sup3,
    TeX_Sub1,
    TeX_Sub2,
    TeX_SupDrop,
    TeX_SubDrop,
    TeX_Delim1,
    TeX_Delim2,
    TeX_AxisHeight,
    0};
static uint32 tex_mathext_params[] = {
    TeX_Slant,
    TeX_Space,
    TeX_Stretch,
    TeX_Shrink,
    TeX_XHeight,
    TeX_Quad,
    TeX_MathSp,
    TeX_DefRuleThick,
    TeX_BigOpSpace1,
    TeX_BigOpSpace2,
    TeX_BigOpSpace3,
    TeX_BigOpSpace4,
    TeX_BigOpSpace5,
    0};


/* *************************    The 'TeX ' table    ************************* */
/* *************************          Input         ************************* */

static void TeX_readFontParams(FILE *ttf,struct ttfinfo *info,uint32 base) {
    int i,pcnt;
    static uint32 *alltags[] = { tex_text_params, tex_math_params, tex_mathext_params };
    int j,k;
    uint32 tag;
    int32 val;

    fseek(ttf,base,SEEK_SET);
    if ( getushort(ttf)!=0 )	/* Don't know how to read this version of the subtable */
return;
    pcnt = getushort(ttf);
    if ( pcnt==22 ) info->texdata.type = tex_math;
    else if ( pcnt==13 ) info->texdata.type = tex_mathext;
    else if ( pcnt>=7 ) info->texdata.type = tex_text;
    for ( i=0; i<pcnt; ++i ) {
	tag = getlong(ttf);
	val = getlong(ttf);
	for ( j=0; j<3; ++j ) {
	    for ( k=0; alltags[j][k]!=0; ++k )
		if ( alltags[j][k]==tag )
	    break;
	    if ( alltags[j][k]==tag )
	break;
	}
	if ( j<3 )
	    info->texdata.params[k] = val;
    }
}

static void TeX_readHeightDepth(FILE *ttf,struct ttfinfo *info,uint32 base) {
    int i,gcnt;

    fseek(ttf,base,SEEK_SET);
    if ( getushort(ttf)!=0 )	/* Don't know how to read this version of the subtable */
return;
    gcnt = getushort(ttf);
    for ( i=0; i<gcnt && i<info->glyph_cnt; ++i ) {
	int h, d;
	h = getushort(ttf);
	d = getushort(ttf);
	if ( info->chars[i]!=NULL ) {
	    info->chars[i]->tex_height = h;
	    info->chars[i]->tex_depth = d;
	}
    }
}

static void TeX_readItalicCorr(FILE *ttf,struct ttfinfo *info,uint32 base) {
    int i,gcnt;

    fseek(ttf,base,SEEK_SET);
    if ( getushort(ttf)!=0 )	/* Don't know how to read this version of the subtable */
return;
    gcnt = getushort(ttf);
    for ( i=0; i<gcnt && i<info->glyph_cnt; ++i ) {
	int ital;
	ital = getushort(ttf);
	if ( info->chars[i]!=NULL ) {
	    info->chars[i]->italic_correction = ital;
	}
    }
}

void tex_read(FILE *ttf,struct ttfinfo *info) {
    int n,i;
    struct tagoff { uint32 tag, offset; } tagoff[MAX_SUBTABLE_TYPES+30];

    fseek(ttf,info->tex_start,SEEK_SET);

    if ( getlong(ttf)!=0x00010000 )
return;
    n = getlong(ttf);
    if ( n>=MAX_SUBTABLE_TYPES+30 )
	n = MAX_SUBTABLE_TYPES+30;
    for ( i=0; i<n; ++i ) {
	tagoff[i].tag = getlong(ttf);
	tagoff[i].offset = getlong(ttf);
    }
    for ( i=0; i<n; ++i ) switch ( tagoff[i].tag ) {
      case CHR('f','t','p','m'):
	TeX_readFontParams(ttf,info,info->tex_start+tagoff[i].offset);
      break;
      case CHR('h','t','d','p'):
	TeX_readHeightDepth(ttf,info,info->tex_start+tagoff[i].offset);
      break;
      case CHR('i','t','l','c'):
	TeX_readItalicCorr(ttf,info,info->tex_start+tagoff[i].offset);
      break;
      default:
	LogError( _("Unknown subtable '%c%c%c%c' in 'TeX ' table, ignored\n"),
		tagoff[i].tag>>24, (tagoff[i].tag>>16)&0xff, (tagoff[i].tag>>8)&0xff, tagoff[i].tag&0xff );
      break;
    }
}

