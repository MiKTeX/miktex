/* -*- coding: utf-8 -*- */
/* Copyright (C) 2007-2012 by George Williams */
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

static int uint32_cmp(const void *_ui1, const void *_ui2) {
    if ( *(uint32 *) _ui1 > *(uint32 *)_ui2 )
return( 1 );
    if ( *(uint32 *) _ui1 < *(uint32 *)_ui2 )
return( -1 );

return( 0 );
}

static int lang_cmp(const void *_ui1, const void *_ui2) {
    /* The default language is magic, and should come first in the list even */
    /*  if that is not true alphabetical order */
    if ( *(uint32 *) _ui1 == DEFAULT_LANG )
return( -1 );
    if ( *(uint32 *) _ui2 == DEFAULT_LANG )
return( 1 );

    if ( *(uint32 *) _ui1 > *(uint32 *)_ui2 )
return( 1 );
    if ( *(uint32 *) _ui1 < *(uint32 *)_ui2 )
return( -1 );

return( 0 );
}

uint32 *SFScriptsInLookups(SplineFont *sf,int gpos) {
    /* Presumes that either SFFindUnusedLookups or SFFindClearUnusedLookupBits */
    /*  has been called first */
    /* Since MS will sometimes ignore a script if it isn't found in both */
    /*  GPOS and GSUB we want to return the same script list no matter */
    /*  what the setting of gpos ... so we totally ignore that argument */
    /*  and always look at both sets of lookups */

/* Sergey Malkin from MicroSoft tells me:
    Each shaping engine in Uniscribe can decide on its requirements for
    layout tables - some of them require both GSUB and GPOS, in some cases
    any table present is enough, or it can work without any table.

    Sometimes, purpose of the check is to determine if font is supporting
    particular script - if required tables are not there font is just
    rejected by this shaping engine. Sometimes, shaping engine can not just
    reject the font because there are fonts using older shaping technologies
    we still have to support, so it uses some logic when to fallback to
    legacy layout code.

    In your case this is Hebrew, where both tables are required to use
    OpenType processing. Arabic requires both tables too, Latin requires
    GSUB to execute GPOS. But in general, if you have both tables you should
    be safe with any script to get fully featured OpenType shaping.

In other words, if we have a Hebrew font with just GPOS features they won't work,
and MS will not use the font at all. We must add a GSUB table. In the unlikely
event that we had a hebrew font with only GSUB it would not work either.

So if we want our lookups to have a chance of executing under Uniscribe we
better make sure that both tables have the same script set.

(Sergey says we could optimize a little: A Latin GSUB table will run without
a GPOS, but he says the GPOS won't work without a GSUB.)
*/
    int cnt=0, tot=0, i;
    uint32 *scripts = NULL;
    OTLookup *test;
    FeatureScriptLangList *fl;
    struct scriptlanglist *sl;

    /* So here always give scripts for both (see comment above) no */
    /*  matter what they asked for */
    for ( gpos=0; gpos<2; ++gpos ) {
	for ( test = gpos ? sf->gpos_lookups : sf->gsub_lookups; test!=NULL; test = test->next ) {
	    if ( test->unused )
	continue;
	    for ( fl=test->features; fl!=NULL; fl=fl->next ) {
		if ( fl->ismac )
	    continue;
		for ( sl=fl->scripts ; sl!=NULL; sl=sl->next ) {
		    for ( i=0; i<cnt; ++i ) {
			if ( sl->script==scripts[i] )
		    break;
		    }
		    if ( i==cnt ) {
			if ( cnt>=tot )
			    scripts = realloc(scripts,(tot+=10)*sizeof(uint32));
			scripts[cnt++] = sl->script;
		    }
		}
	    }
	}
    }

    if ( cnt==0 )
return( NULL );

    /* We want our scripts in alphabetic order */
    qsort(scripts,cnt,sizeof(uint32),uint32_cmp);
    /* add a 0 entry to mark the end of the list */
    if ( cnt>=tot )
	scripts = realloc(scripts,(tot+1)*sizeof(uint32));
    scripts[cnt] = 0;
return( scripts );
}

uint32 *SFLangsInScript(SplineFont *sf,int gpos,uint32 script) {
    /* However, the language lists (I think) are distinct */
    /* But giving a value of -1 for gpos will give us the set of languages in */
    /*  both tables (for this script) */
    int cnt=0, tot=0, i, g, l;
    uint32 *langs = NULL;
    OTLookup *test;
    FeatureScriptLangList *fl;
    struct scriptlanglist *sl;

    for ( g=0; g<2; ++g ) {
	if (( gpos==0 && g==1 ) || ( gpos==1 && g==0 ))
    continue;
	for ( test = g ? sf->gpos_lookups : sf->gsub_lookups; test!=NULL; test = test->next ) {
	    if ( test->unused )
	continue;
	    for ( fl=test->features; fl!=NULL; fl=fl->next ) {
		for ( sl=fl->scripts ; sl!=NULL; sl=sl->next ) {
		    if ( sl->script==script ) {
			for ( l=0; l<sl->lang_cnt; ++l ) {
			    uint32 lang;
			    if ( l<MAX_LANG )
				lang = sl->langs[l];
			    else
				lang = sl->morelangs[l-MAX_LANG];
			    for ( i=0; i<cnt; ++i ) {
				if ( lang==langs[i] )
			    break;
			    }
			    if ( i==cnt ) {
				if ( cnt>=tot )
				    langs = realloc(langs,(tot+=10)*sizeof(uint32));
				langs[cnt++] = lang;
			    }
			}
		    }
		}
	    }
	}
    }

    if ( cnt==0 ) {
	/* We add dummy script entries. Because Uniscribe will refuse to */
	/*  process some scripts if they don't have an entry in both GPOS */
	/*  an GSUB. So if a script appears in either table, force it to */
	/*  appear in both. That means we can get scripts with no lookups */
	/*  and hence no languages. It seems that Uniscribe doesn't like */
	/*  that either. So give each such script a dummy default language */
	/*  entry. This is what VOLT does */
	langs = calloc(2,sizeof(uint32));
	langs[0] = DEFAULT_LANG;
return( langs );
    }

    /* We want our languages in alphabetic order */
    qsort(langs,cnt,sizeof(uint32),lang_cmp);
    /* add a 0 entry to mark the end of the list */
    if ( cnt>=tot )
	langs = realloc(langs,(tot+1)*sizeof(uint32));
    langs[cnt] = 0;
return( langs );
}

uint32 *SFFeaturesInScriptLang(SplineFont *sf,int gpos,uint32 script,uint32 lang) {
    int cnt=0, tot=0, i, l, isg;
    uint32 *features = NULL;
    OTLookup *test;
    FeatureScriptLangList *fl;
    struct scriptlanglist *sl;
    /* gpos==0 => GSUB, gpos==1 => GPOS, gpos==-1 => both, gpos==-2 => Both & morx & kern */

    if ( sf->cidmaster ) sf=sf->cidmaster;
    for ( isg = 0; isg<2; ++isg ) {
	if ( gpos>=0 && isg!=gpos )
    continue;
	for ( test = isg ? sf->gpos_lookups : sf->gsub_lookups; test!=NULL; test = test->next ) {
	    if ( test->unused )
	continue;
	    for ( fl=test->features; fl!=NULL; fl=fl->next ) {
		if ( fl->ismac && gpos!=-2 )
	    continue;
		if ( script==0xffffffff ) {
		    for ( i=0; i<cnt; ++i ) {
			if ( fl->featuretag==features[i] )
		    break;
		    }
		    if ( i==cnt ) {
			if ( cnt>=tot )
			    features = realloc(features,(tot+=10)*sizeof(uint32));
			features[cnt++] = fl->featuretag;
		    }
		} else for ( sl=fl->scripts ; sl!=NULL; sl=sl->next ) {
		    if ( sl->script==script ) {
			int matched = false;
			if ( fl->ismac && gpos==-2 )
			    matched = true;
			else for ( l=0; l<sl->lang_cnt; ++l ) {
			    uint32 testlang;
			    if ( l<MAX_LANG )
				testlang = sl->langs[l];
			    else
				testlang = sl->morelangs[l-MAX_LANG];
			    if ( testlang==lang ) {
				matched = true;
			break;
			    }
			}
			if ( matched ) {
			    for ( i=0; i<cnt; ++i ) {
				if ( fl->featuretag==features[i] )
			    break;
			    }
			    if ( i==cnt ) {
				if ( cnt>=tot )
				    features = realloc(features,(tot+=10)*sizeof(uint32));
				features[cnt++] = fl->featuretag;
			    }
			}
		    }
		}
	    }
	}
    }

    if ( sf->design_size!=0 && gpos ) {
	/* The 'size' feature is like no other. It has no lookups and so */
	/*  we will never find it in the normal course of events. If the */
	/*  user has specified a design size, then every script/lang combo */
	/*  gets a 'size' feature which contains no lookups but feature */
	/*  params */
	if ( cnt>=tot )
	    features = realloc(features,(tot+=2)*sizeof(uint32));
	features[cnt++] = CHR('s','i','z','e');
    }

    if ( cnt==0 )
return( calloc(1,sizeof(uint32)) );

    /* We don't care if our features are in alphabetical order here */
    /*  all that matters is whether the complete list of features is */
    /*  ordering here would be irrelevant */
    /* qsort(features,cnt,sizeof(uint32),uint32_cmp); */

    /* add a 0 entry to mark the end of the list */
    if ( cnt>=tot )
	features = realloc(features,(tot+1)*sizeof(uint32));
    features[cnt] = 0;
return( features );
}

OTLookup **SFLookupsInScriptLangFeature(SplineFont *sf,int gpos,uint32 script,uint32 lang, uint32 feature) {
    int cnt=0, tot=0, l;
    OTLookup **lookups = NULL;
    OTLookup *test;
    FeatureScriptLangList *fl;
    struct scriptlanglist *sl;

    for ( test = gpos ? sf->gpos_lookups : sf->gsub_lookups; test!=NULL; test = test->next ) {
	if ( test->unused )
    continue;
	for ( fl=test->features; fl!=NULL; fl=fl->next ) {
	    if ( fl->featuretag==feature ) {
		for ( sl=fl->scripts ; sl!=NULL; sl=sl->next ) {
		    if ( sl->script==script ) {
			for ( l=0; l<sl->lang_cnt; ++l ) {
			    uint32 testlang;
			    if ( l<MAX_LANG )
				testlang = sl->langs[l];
			    else
				testlang = sl->morelangs[l-MAX_LANG];
			    if ( testlang==lang ) {
				if ( cnt>=tot )
				    lookups = realloc(lookups,(tot+=10)*sizeof(OTLookup *));
				lookups[cnt++] = test;
	goto found;
			    }
			}
		    }
		}
	    }
	}
	found:;
    }

    if ( cnt==0 )
return( NULL );

    /* lookup order is irrelevant here. might as well leave it in invocation order */
    /* add a 0 entry to mark the end of the list */
    if ( cnt>=tot )
	lookups = realloc(lookups,(tot+1)*sizeof(OTLookup *));
    lookups[cnt] = 0;
return( lookups );
}

static int LigaturesFirstComponentGID(SplineFont *sf,char *components) {
    int gid, ch;
    char *pt;

    for ( pt = components; *pt!='\0' && *pt!=' '; ++pt );
    ch = *pt; *pt = '\0';
    gid = SFFindExistingSlot(sf,-1,components);
    *pt = ch;
return( gid );
}

static int PSTValid(SplineFont *sf,PST *pst) {
    char *start, *pt, ch;
    int ret;

    switch ( pst->type ) {
      case pst_position:
return( true );
      case pst_pair:
return( SCWorthOutputting(SFGetChar(sf,-1,pst->u.pair.paired)) );
      case pst_substitution: case pst_alternate: case pst_multiple:
      case pst_ligature:
	for ( start = pst->u.mult.components; *start ; ) {
	    for ( pt=start; *pt && *pt!=' '; ++pt );
	    ch = *pt; *pt = '\0';
	    ret = SCWorthOutputting(SFGetChar(sf,-1,start));
	    if ( !ret ) {
		LogError(_("Lookup subtable contains unused glyph %s making the whole subtable invalid"), start);
		*pt = ch;
return( false );
	    }
	    *pt = ch;
	    if ( ch==0 )
		start = pt;
	    else
		start = pt+1;
	}
      default:
      break;
    }
return( true );
}

SplineChar **SFGlyphsWithPSTinSubtable(SplineFont *sf,struct lookup_subtable *subtable) {
    uint8 *used = calloc(sf->glyphcnt,sizeof(uint8));
    SplineChar **glyphs, *sc;
    int i, k, gid, cnt;
    KernPair *kp;
    PST *pst;
    int ispair = subtable->lookup->lookup_type == gpos_pair;
    int isliga = subtable->lookup->lookup_type == gsub_ligature;

    for ( i=0; i<sf->glyphcnt; ++i ) if ( SCWorthOutputting(sc = sf->glyphs[i]) ) {
	if ( ispair ) {
	    for ( k=0; k<2; ++k ) {
		for ( kp= k ? sc->kerns : sc->vkerns; kp!=NULL ; kp=kp->next ) {
		    if ( !SCWorthOutputting(kp->sc))
		continue;
		    if ( kp->subtable == subtable ) {
			used[i] = true;
    goto continue_;
		    }
		}
	    }
	}
	for ( pst=sc->possub; pst!=NULL; pst=pst->next ) {
	    if ( pst->subtable == subtable && PSTValid(sf,pst)) {
		if ( !isliga ) {
		    used[i] = true;
    goto continue_;
		} else {
		    gid = LigaturesFirstComponentGID(sf,pst->u.lig.components);
		    pst->u.lig.lig = sc;
		    if ( gid!=-1 )
			used[gid] = true;
		    /* can't continue here. ffi might be "f+f+i" and "ff+i" */
		    /*  and we need to mark both "f" and "ff" as used */
		}
	    }
	}
    continue_: ;
    }

    for ( i=cnt=0 ; i<sf->glyphcnt; ++i )
	if ( used[i] )
	    ++cnt;

    if ( cnt==0 ) {
	free(used);
return( NULL );
    }
    glyphs = malloc((cnt+1)*sizeof(SplineChar *));
    for ( i=cnt=0 ; i<sf->glyphcnt; ++i ) {
	if ( used[i] )
	    glyphs[cnt++] = sf->glyphs[i];
    }
    glyphs[cnt] = NULL;
    free(used);
return( glyphs );
}

static void TickLookupKids(OTLookup *otl) {
    struct lookup_subtable *sub;
    int i,j;

    for ( sub=otl->subtables; sub!=NULL; sub=sub->next ) {
	if ( sub->fpst!=NULL ) {
	    for ( i=0; i<sub->fpst->rule_cnt; ++i ) {
		struct fpst_rule *rule = &sub->fpst->rules[i];
		for ( j=0; j<rule->lookup_cnt; ++j ) {
		    if ( rule->lookups[j].lookup!=NULL )
			rule->lookups[j].lookup->in_gpos = true;
		}
	    }
	}
    }
}

void SFFindUnusedLookups(SplineFont *sf) {
    OTLookup *test;
    struct lookup_subtable *sub;
    int gpos;
    AnchorClass *ac;
    AnchorPoint *ap;
    SplineChar *sc;
    KernPair *kp;
    PST *pst;
    int i,k,gid,isv;
    SplineFont *_sf = sf;
    Justify *jscripts;
    struct jstf_lang *jlangs;

    if ( _sf->cidmaster ) _sf = _sf->cidmaster;

    /* Some things are obvious. If a subtable consists of a kernclass or some */
    /*  such, then obviously it is used. But more distributed info takes more */
    /*  work. So mark anything easy as used, and anything difficult as unused */
    /* We'll work on the difficult things later */
    for ( gpos=0; gpos<2; ++gpos ) {
	for ( test = gpos ? _sf->gpos_lookups : _sf->gsub_lookups; test!=NULL; test = test->next ) {
	    for ( sub = test->subtables; sub!=NULL; sub=sub->next ) {
		if ( sub->kc!=NULL || sub->fpst!=NULL || sub->sm!=NULL ) {
		    sub->unused = false;
	    continue;
		}
		sub->unused = true;
		/* We'll turn the following bit back on if there turns out */
		/*  to be an anchor class attached to it -- that is subtly */
		/*  different than being unused -- unused will be set if all */
		/*  acs are unused, this bit will be on if there are unused */
		/*  classes that still refer to us. */
		sub->anchor_classes = false;
	    }
	}
    }

    /* To be useful an anchor class must have both at least one base and one mark */
    /*  (for cursive anchors that means at least one entry and at least one exit) */
    /* Start by assuming the worst */
    for ( ac = _sf->anchor; ac!=NULL; ac=ac->next )
	ac->has_mark = ac->has_base = false;

    /* Ok, for each glyph, look at all lookups (or anchor classes) it affects */
    /*  and mark the appropriate parts of them as used */
    k = 0;
    do {
	sf = _sf->subfontcnt==0 ? _sf : _sf->subfonts[k];
	for ( gid=0; gid<sf->glyphcnt; ++gid ) if ( SCWorthOutputting(sc = sf->glyphs[gid]) ) {
	    for ( ap=sc->anchor; ap!=NULL; ap=ap->next ) {
		switch ( ap->type ) {
		  case at_mark: case at_centry:
		    ap->anchor->has_mark = true;
		  break;
		  case at_basechar: case at_baselig: case at_basemark:
		  case at_cexit:
		    ap->anchor->has_base = true;
		  break;
		  default:
		  break;
		}
	    }
	    for ( isv=0; isv<2; ++isv ) {
		for ( kp= isv ? sc->kerns : sc->vkerns ; kp!=NULL; kp=kp->next ) {
		    if ( SCWorthOutputting(kp->sc))
			kp->subtable->unused = false;
		}
	    }
	    for ( pst=sc->possub; pst!=NULL; pst=pst->next ) {
		if ( pst->subtable==NULL )
	    continue;
		if ( !PSTValid(sf,pst))
	    continue;
		pst->subtable->unused = false;
	    }
	}
	++k;
    } while ( k<_sf->subfontcnt );

    /* Finally for any anchor class that has both a mark and a base then it is */
    /*  used, and its lookup is also used */
    /* Also, even if unused, as long as the anchor class exists we must keep */
    /*  the subtable around */
    for ( ac = _sf->anchor; ac!=NULL; ac=ac->next ) {
        if ( ac->subtable==NULL )
    continue;
	ac->subtable->anchor_classes = true;
	if ( ac->has_mark && ac->has_base )
	    ac->subtable->unused = false;
    }

    /* Now for each lookup, a lookup is unused if ALL subtables are unused */
    for ( gpos=0; gpos<2; ++gpos ) {
	for ( test = gpos ? _sf->gpos_lookups : _sf->gsub_lookups; test!=NULL; test = test->next ) {
	    test->unused = test->empty = true;
	    for ( sub=test->subtables; sub!=NULL; sub=sub->next ) {
		if ( !sub->unused )
		    test->unused = false;
		if ( !sub->unused && !sub->anchor_classes ) {
		    test->empty = false;
	    break;
		}
	    }
	}
    }

    /* I store JSTF max lookups in the gpos list because they have the same */
    /*  format. But now I need to tease them out and learn which lookups are */
    /*  used in GPOS and which in JSTF (and conceivably which get duplicated */
    /*  and placed in both) */
    for ( test = sf->gpos_lookups; test!=NULL; test = test->next ) {
	test->only_jstf = test->in_jstf = test->in_gpos = false;
	if ( test->features!=NULL )
	    test->in_gpos = true;
    }
    for ( jscripts = sf->justify; jscripts!=NULL; jscripts=jscripts->next ) {
	for ( jlangs=jscripts->langs; jlangs!=NULL; jlangs=jlangs->next ) {
	    for ( i=0; i<jlangs->cnt; ++i ) {
		struct jstf_prio *prio = &jlangs->prios[i];
		if ( prio->enableShrink!=NULL )
		    for ( k=0; prio->enableShrink[k]!=NULL; ++k )
			prio->enableShrink[k]->in_gpos = true;
		if ( prio->disableShrink!=NULL )
		    for ( k=0; prio->disableShrink[k]!=NULL; ++k )
			prio->disableShrink[k]->in_gpos = true;
		if ( prio->enableExtend!=NULL )
		    for ( k=0; prio->enableExtend[k]!=NULL; ++k )
			prio->enableExtend[k]->in_gpos = true;
		if ( prio->disableExtend!=NULL )
		    for ( k=0; prio->disableExtend[k]!=NULL; ++k )
			prio->disableExtend[k]->in_gpos = true;
		if ( prio->maxShrink!=NULL )
		    for ( k=0; prio->maxShrink[k]!=NULL; ++k )
			prio->maxShrink[k]->in_jstf = true;
		if ( prio->maxExtend!=NULL )
		    for ( k=0; prio->maxExtend[k]!=NULL; ++k )
			prio->maxExtend[k]->in_jstf = true;
	    }
	}
    }
    for ( test = sf->gpos_lookups; test!=NULL; test = test->next ) {
	if ( test->in_gpos && (test->lookup_type==gpos_context || test->lookup_type==gpos_contextchain))
	    TickLookupKids(test);
    }
    for ( test = sf->gpos_lookups; test!=NULL; test = test->next )
	test->only_jstf = test->in_jstf && !test->in_gpos;
}

struct lookup_subtable *SFFindLookupSubtable(SplineFont *sf,char *name) {
    int isgpos;
    OTLookup *otl;
    struct lookup_subtable *sub;

    if ( sf->cidmaster ) sf = sf->cidmaster;

    if ( name==NULL )
return( NULL );

    for ( isgpos=0; isgpos<2; ++isgpos ) {
	for ( otl = isgpos ? sf->gpos_lookups : sf->gsub_lookups ; otl!=NULL; otl=otl->next ) {
	    for ( sub = otl->subtables; sub!=NULL; sub=sub->next ) {
		if ( strcmp(name,sub->subtable_name)==0 )
return( sub );
	    }
	}
    }
return( NULL );
}

struct lookup_subtable *SFFindLookupSubtableAndFreeName(SplineFont *sf,char *name) {
    struct lookup_subtable *sub = SFFindLookupSubtable(sf,name);
    free(name);
return( sub );
}

OTLookup *SFFindLookup(SplineFont *sf,char *name) {
    int isgpos;
    OTLookup *otl;

    if ( sf->cidmaster ) sf = sf->cidmaster;

    if ( name==NULL )
return( NULL );

    for ( isgpos=0; isgpos<2; ++isgpos ) {
	for ( otl = isgpos ? sf->gpos_lookups : sf->gsub_lookups ; otl!=NULL; otl=otl->next ) {
	    if ( strcmp(name,otl->lookup_name)==0 )
return( otl );
	}
    }
return( NULL );
}

struct scriptlanglist *SLCopy(struct scriptlanglist *sl) {
    struct scriptlanglist *newsl;

    newsl = chunkalloc(sizeof(struct scriptlanglist));
    *newsl = *sl;
    newsl->next = NULL;

    if ( sl->lang_cnt>MAX_LANG ) {
	newsl->morelangs = malloc((newsl->lang_cnt-MAX_LANG)*sizeof(uint32));
	memcpy(newsl->morelangs,sl->morelangs,(newsl->lang_cnt-MAX_LANG)*sizeof(uint32));
    }
return( newsl );
}

struct scriptlanglist *SListCopy(struct scriptlanglist *sl) {
    struct scriptlanglist *head=NULL, *last=NULL, *cur;

    for ( ; sl!=NULL; sl=sl->next ) {
	cur = SLCopy(sl);
	if ( head==NULL )
	    head = cur;
	else
	    last->next = cur;
	last = cur;
    }
return( head );
}

FeatureScriptLangList *FeatureListCopy(FeatureScriptLangList *fl) {
    FeatureScriptLangList *newfl;

    if ( fl==NULL )
return( NULL );

    newfl = chunkalloc(sizeof(FeatureScriptLangList));
    *newfl = *fl;
    newfl->next = NULL;

    newfl->scripts = SListCopy(fl->scripts);
return( newfl );
}
