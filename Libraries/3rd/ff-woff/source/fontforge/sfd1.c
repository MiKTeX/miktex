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

#include "fontforge.h"
#include "sfd1.h"

/* This file contains the routines needed to process an old style sfd file and*/
/*  convert it into the new format */

static void SFGuessScriptList(SplineFont1 *sf) {
    uint32 scripts[32], script;
    int i, scnt=0, j;

    for ( i=0; i<sf->sf.glyphcnt; ++i ) if ( sf->sf.glyphs[i]!=NULL ) {
	script = SCScriptFromUnicode(sf->sf.glyphs[i]);
	if ( script!=0 && script!=DEFAULT_SCRIPT ) {
	    for ( j=scnt-1; j>=0 ; --j )
		if ( scripts[j]==script )
	    break;
	    if ( j<0 ) {
		scripts[scnt++] = script;
		if ( scnt>=32 )
    break;
	    }
	}
    }
    if ( scnt==0 )
	scripts[scnt++] = CHR('l','a','t','n');

    /* order scripts */
    for ( i=0; i<scnt-1; ++i ) for ( j=i+1; j<scnt; ++j ) {
	if ( scripts[i]>scripts[j] ) {
	    script = scripts[i];
	    scripts[i] = scripts[j];
	    scripts[j] = script;
	}
    }

    if ( sf->sf.cidmaster ) sf = (SplineFont1 *) sf->sf.cidmaster;
    else if ( sf->sf.mm!=NULL ) sf=(SplineFont1 *) sf->sf.mm->normal;
    if ( sf->script_lang!=NULL )
return;
    sf->script_lang = calloc(2,sizeof(struct script_record *));
    sf->script_lang[0] = calloc(scnt+1,sizeof(struct script_record));
    sf->sli_cnt = 1;
    for ( j=0; j<scnt; ++j ) {
	sf->script_lang[0][j].script = scripts[j];
	sf->script_lang[0][j].langs = malloc(2*sizeof(uint32));
	sf->script_lang[0][j].langs[0] = DEFAULT_LANG;
	sf->script_lang[0][j].langs[1] = 0;
    }
    sf->script_lang[1] = NULL;
}

static int SLContains(struct script_record *sr, uint32 script, uint32 lang) {
    int i, j;

    if ( script==DEFAULT_SCRIPT || script == 0 )
return( true );
    for ( i=0; sr[i].script!=0; ++i ) {
	if ( sr[i].script==script ) {
	    if ( lang==0 )
return( true );
	    for ( j=0; sr[i].langs[j]!=0; ++j )
		if ( sr[i].langs[j]==lang )
return( true );

return( false );	/* this script entry didn't contain the language. won't be any other scripts to check */
	}
    }
return( false );	/* Never found script */
}

static int SFAddScriptLangIndex(SplineFont *_sf,uint32 script,uint32 lang) {
    int i;
    SplineFont1 *sf;

    if ( _sf->cidmaster ) _sf = _sf->cidmaster;
    else if ( _sf->mm!=NULL ) _sf=_sf->mm->normal;

    if ( _sf->sfd_version>=2 )
	IError( "SFFindBiggestScriptLangIndex called with bad version number.\n" );

    sf = (SplineFont1 *) _sf;

    if ( script==0 ) script=DEFAULT_SCRIPT;
    if ( lang==0 ) lang=DEFAULT_LANG;
    if ( sf->script_lang==NULL )
	sf->script_lang = calloc(2,sizeof(struct script_record *));
    for ( i=0; sf->script_lang[i]!=NULL; ++i ) {
	if ( sf->script_lang[i][0].script==script && sf->script_lang[i][1].script==0 &&
		sf->script_lang[i][0].langs[0]==lang &&
		sf->script_lang[i][0].langs[1]==0 )
return( i );
    }
    sf->script_lang = realloc(sf->script_lang,(i+2)*sizeof(struct script_record *));
    sf->script_lang[i] = calloc(2,sizeof(struct script_record));
    sf->script_lang[i][0].script = script;
    sf->script_lang[i][0].langs = malloc(2*sizeof(uint32));
    sf->script_lang[i][0].langs[0] = lang;
    sf->script_lang[i][0].langs[1] = 0;
    sf->script_lang[i+1] = NULL;
    sf->sli_cnt = i+1;
return( i );
}

static int SLCount(struct script_record *sr) {
    int sl_cnt = 0;
    int i,j;

    for ( i=0; sr[i].script!=0; ++i ) {
	for ( j=0; sr[i].langs[j]!=0; ++j )
	    ++sl_cnt;
    }
return( sl_cnt );
}

int SFFindBiggestScriptLangIndex(SplineFont *_sf,uint32 script,uint32 lang) {
    int i, best_sli= -1, best_cnt= -1, cnt;
    SplineFont1 *sf = (SplineFont1 *) _sf;

    if ( _sf->sfd_version>=2 )
	IError( "SFFindBiggestScriptLangIndex called with bad version number.\n" );

    if ( sf->script_lang==NULL )
	SFGuessScriptList(sf);
    for ( i=0; sf->script_lang[i]!=NULL; ++i ) {
	if ( SLContains(sf->script_lang[i],script,lang)) {
	    cnt = SLCount(sf->script_lang[i]);
	    if ( cnt>best_cnt ) {
		best_sli = i;
		best_cnt = cnt;
	    }
	}
    }
    if ( best_sli==-1 )
return( SFAddScriptLangIndex(_sf,script,lang) );

return( best_sli );
}
