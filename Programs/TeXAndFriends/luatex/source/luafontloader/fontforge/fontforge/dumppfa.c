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
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ustring.h>
#include <utype.h>
#include <unistd.h>
#include <locale.h>
#ifndef _WIN32
# include <pwd.h>
#endif
#include <stdarg.h>
#include <time.h>
#include "psfont.h"
#include "splinefont.h"

#ifdef __CygWin
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
#endif

extern int autohint_before_generate;
char *xuid=NULL;

typedef void (*DumpChar)(int ch,void *data);
struct fileencryptdata {
    DumpChar olddump;
    void *olddata;
    unsigned short r;
    int hexline;
};
#define c1	52845
#define c2	22719

struct psdict *PSDictCopy(struct psdict *dict) {
    struct psdict *ret;
    int i;

    if ( dict==NULL )
return( NULL );

    ret = gcalloc(1,sizeof(struct psdict));
    ret->cnt = dict->cnt; ret->next = dict->next;
    ret->keys = gcalloc(ret->cnt,sizeof(char *));
    ret->values = gcalloc(ret->cnt,sizeof(char *));
    for ( i=0; i<dict->next; ++i ) {
	ret->keys[i] = copy(dict->keys[i]);
	ret->values[i] = copy(dict->values[i]);
    }

return( ret );
}

int PSDictFindEntry(struct psdict *dict, char *key) {
    int i;

    if ( dict==NULL )
return( -1 );

    for ( i=0; i<dict->next; ++i )
	if ( strcmp(dict->keys[i],key)==0 )
return( i );

return( -1 );
}

char *PSDictHasEntry(struct psdict *dict, char *key) {
    int i;

    if ( dict==NULL )
return( NULL );

    for ( i=0; i<dict->next; ++i )
	if ( strcmp(dict->keys[i],key)==0 )
return( dict->values[i] );

return( NULL );
}

int PSDictRemoveEntry(struct psdict *dict, char *key) {
    int i;

    if ( dict==NULL )
return( false );

    for ( i=0; i<dict->next; ++i )
	if ( strcmp(dict->keys[i],key)==0 )
    break;
    if ( i==dict->next )
return( false );
    free( dict->keys[i]);
    free( dict->values[i] );
    --dict->next;
    while ( i<dict->next ) {
	dict->keys[i] = dict->keys[i+1];
	dict->values[i] = dict->values[i+1];
	++i;
    }

return( true );
}

int PSDictChangeEntry(struct psdict *dict, char *key, char *newval) {
    int i;

    if ( dict==NULL )
return( -1 );

    for ( i=0; i<dict->next; ++i )
	if ( strcmp(dict->keys[i],key)==0 )
    break;
    if ( i==dict->next ) {
	if ( dict->next>=dict->cnt ) {
	    dict->cnt += 10;
	    dict->keys = grealloc(dict->keys,dict->cnt*sizeof(char *));
	    dict->values = grealloc(dict->values,dict->cnt*sizeof(char *));
	}
	dict->keys[dict->next] = copy(key);
	dict->values[dict->next] = NULL;
	++dict->next;
    }
    free(dict->values[i]);
    dict->values[i] = copy(newval);
return( i );
}


static double FindMaxDiffOfBlues(char *pt, double max_diff) {
    char *end;
    double p1, p2;

    while ( *pt==' ' || *pt=='[' ) ++pt;
    forever {
	p1 = strtod(pt,&end);
	if ( end==pt )
    break;
	pt = end;
	p2 = strtod(pt,&end);
	if ( end==pt )
    break;
	if ( p2-p1 >max_diff ) max_diff = p2-p1;
	pt = end;
    }
return( max_diff );
}

double BlueScaleFigureForced(struct psdict *private,real bluevalues[], real otherblues[]) {
    double max_diff=0;
    char *pt;
    int i;

    pt = PSDictHasEntry(private,"BlueValues");
    if ( pt!=NULL ) {
	max_diff = FindMaxDiffOfBlues(pt,max_diff);
    } else if ( bluevalues!=NULL ) {
	for ( i=0; i<14 && (bluevalues[i]!=0 || bluevalues[i+1])!=0; i+=2 ) {
	    if ( bluevalues[i+1] - bluevalues[i]>=max_diff )
		max_diff = bluevalues[i+1] - bluevalues[i];
	}
    }
    pt = PSDictHasEntry(private,"FamilyBlues");
    if ( pt!=NULL )
	max_diff = FindMaxDiffOfBlues(pt,max_diff);

    pt = PSDictHasEntry(private,"OtherBlues");
    if ( pt!=NULL )
	max_diff = FindMaxDiffOfBlues(pt,max_diff);
    else if ( otherblues!=NULL ) {
	for ( i=0; i<10 && (otherblues[i]!=0 || otherblues[i+1]!=0); i+=2 ) {
	    if ( otherblues[i+1] - otherblues[i]>=max_diff )
		max_diff = otherblues[i+1] - otherblues[i];
	}
    }
    pt = PSDictHasEntry(private,"FamilyOtherBlues");
    if ( pt!=NULL )
	max_diff = FindMaxDiffOfBlues(pt,max_diff);
    if ( max_diff<=0 )
return( -1 );
    if ( 1/max_diff > .039625 )
return( -1 );

return( .99/max_diff );
}

double BlueScaleFigure(struct psdict *private,real bluevalues[], real otherblues[]) {
    if ( PSDictHasEntry(private,"BlueScale")!=NULL )
return( -1 );
    return BlueScaleFigureForced(private, bluevalues, otherblues);
}

