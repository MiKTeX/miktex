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
#include <math.h>

#ifdef __CygWin
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
#endif

char *PSDictHasEntry(struct psdict *dict, const char *key) {
    int i;

    if ( dict==NULL )
return( NULL );

    for ( i=0; i<dict->next; ++i )
	if ( strcmp(dict->keys[i],key)==0 )
return( dict->values[i] );

return( NULL );
}

int PSDictSame(struct psdict *dict1, struct psdict *dict2) {
    int i;

    if ( (dict1==NULL || dict1->cnt==0) && (dict2==NULL || dict2->cnt==0))
return( true );
    if ( dict1==NULL || dict2==NULL || dict1->cnt!=dict2->cnt )
return( false );

    for ( i=0; i<dict1->cnt; ++i ) {
	char *val = PSDictHasEntry(dict2,dict1->keys[i]);
	if ( val==NULL || strcmp(val,dict1->values[i])!=0 )
return( false );
    }
return( true );
}

/* Inside a font, I can't use a <stdin> as a data source. Probably because */
/*  the parser doesn't know what to do with those data when building the char */
/*  proc (as opposed to executing) */
/* So I can't use run length filters or other compression technique */

static double FindMaxDiffOfBlues(char *pt, double max_diff) {
    char *end;
    double p1, p2;

    while ( *pt==' ' || *pt=='[' ) ++pt;
    for (;;) {
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

double BlueScaleFigureForced(struct psdict *private_,real bluevalues[], real otherblues[]) {
    double max_diff=0;
    char *pt;
    int i;

    pt = PSDictHasEntry(private_,"BlueValues");
    if ( pt!=NULL ) {
	max_diff = FindMaxDiffOfBlues(pt,max_diff);
    } else if ( bluevalues!=NULL ) {
	for ( i=0; i<14 && (bluevalues[i]!=0 || bluevalues[i+1])!=0; i+=2 ) {
	    if ( bluevalues[i+1] - bluevalues[i]>=max_diff )
		max_diff = bluevalues[i+1] - bluevalues[i];
	}
    }
    pt = PSDictHasEntry(private_,"FamilyBlues");
    if ( pt!=NULL )
	max_diff = FindMaxDiffOfBlues(pt,max_diff);

    pt = PSDictHasEntry(private_,"OtherBlues");
    if ( pt!=NULL )
	max_diff = FindMaxDiffOfBlues(pt,max_diff);
    else if ( otherblues!=NULL ) {
	for ( i=0; i<10 && (otherblues[i]!=0 || otherblues[i+1]!=0); i+=2 ) {
	    if ( otherblues[i+1] - otherblues[i]>=max_diff )
		max_diff = otherblues[i+1] - otherblues[i];
	}
    }
    pt = PSDictHasEntry(private_,"FamilyOtherBlues");
    if ( pt!=NULL )
	max_diff = FindMaxDiffOfBlues(pt,max_diff);
    if ( max_diff<=0 )
return( -1 );
    if ( 1/max_diff > .039625 )
return( -1 );

    return rint(240.0*0.99/max_diff)/240.0;
}

double BlueScaleFigure(struct psdict *private_,real bluevalues[], real otherblues[]) {
    if ( PSDictHasEntry(private_,"BlueScale")!=NULL )
return( -1 );
    return BlueScaleFigureForced(private_, bluevalues, otherblues);
}
