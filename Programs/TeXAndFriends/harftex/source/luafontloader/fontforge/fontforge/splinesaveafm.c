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
#include "fontforgevw.h"		/* For Error */
#include <stdio.h>
#include "splinefont.h"
#include <utype.h>
#include <ustring.h>
#include <time.h>
#include <math.h>

#include <sys/types.h>		/* For stat */
#include <sys/stat.h>
#include <unistd.h>

#ifdef __CygWin
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
#endif

static void *mygets(FILE *file,char *buffer,int size) {
    char *end = buffer+size-1;
    char *pt = buffer;
    int ch;

    while ( (ch=getc(file))!=EOF && ch!='\r' && ch!='\n' && pt<end )
	*pt++ = ch;
    *pt = '\0';
    if ( ch==EOF && pt==buffer )
return( NULL );
    if ( ch=='\r' ) {
	ch = getc(file);
	if ( ch!='\n' )
	    ungetc(ch,file);
    }
return( buffer );
}

/* ************************************************************************** */
/* **************************** Reading AFM files *************************** */
/* ************************************************************************** */
static void KPInsert( SplineChar *sc1, SplineChar *sc2, int off, int isv ) {
    KernPair *kp;
    int32 script;

    if ( sc1!=NULL && sc2!=NULL ) {
	for ( kp=sc1->kerns; kp!=NULL && kp->sc!=sc2; kp = kp->next );
	if ( kp!=NULL )
	    kp->off = off;
	else if ( off!=0 ) {
	    kp = chunkalloc(sizeof(KernPair));
	    kp->sc = sc2;
	    kp->off = off;
	    script = SCScriptFromUnicode(sc1);
	    if ( script==DEFAULT_SCRIPT )
		script = SCScriptFromUnicode(sc2);
	    kp->subtable = SFSubTableFindOrMake(sc1->parent,
		    isv?CHR('v','k','r','n'):CHR('k','e','r','n'),
		    script, gpos_pair);
	    if ( isv ) {
		kp->next = sc1->vkerns;
		sc1->vkerns = kp;
	    } else {
		kp->next = sc1->kerns;
		sc1->kerns = kp;
	    }
	}
    }
}

int LoadKerningDataFromAfm(SplineFont *sf, char *filename,EncMap *map) {
    FILE *file = fopen(filename,"r");
    char buffer[200], *pt, *ept, ch;
    SplineChar *sc1, *sc2;
    int off;
    char name[44], second[44], lig[44], buf2[100];
    PST *liga;
    double scale = (sf->ascent+sf->descent)/1000.0;

    if ( file==NULL )
return( 0 );
    ff_progress_change_line2(_("Reading AFM file"));
    while ( mygets(file,buffer,sizeof(buffer))!=NULL ) {
	if ( strncmp(buffer,"KPX",3)==0 || strncmp(buffer,"KPY",3)==0 ) {
	    int isv = strncmp(buffer,"KPY",3)==0;
	    for ( pt=buffer+3; isspace(*pt); ++pt);
	    for ( ept = pt; *ept!='\0' && !isspace(*ept); ++ept );
	    ch = *ept; *ept = '\0';
	    sc1 = SFGetChar(sf,-1,pt);
	    *ept = ch;
	    for ( pt=ept; isspace(*pt); ++pt);
	    for ( ept = pt; *ept!='\0' && !isspace(*ept); ++ept );
	    ch = *ept; *ept = '\0';
	    sc2 = SFGetChar(sf,-1,pt);
	    *ept = ch;
	    off = strtol(ept,NULL,10);
	    KPInsert(sc1,sc2,rint(off*scale),isv);
	} else if ( buffer[0]=='C' && isspace(buffer[1])) {
	    char *pt;
	    sc2 = NULL;
	    for ( pt= strchr(buffer,';'); pt!=NULL; pt=strchr(pt+1,';') ) {
		if ( sscanf( pt, "; N %40s", name )==1 )
		    sc2 = SFGetChar(sf,-1,name);
		else if ( sc2!=NULL &&
			sscanf( pt, "; L %40s %40s", second, lig)==2 ) {
		    sc1 = SFGetChar(sf,-1,lig);
		    if ( sc1!=NULL ) {
			sprintf( buf2, "%s %s", name, second);
			for ( liga=sc1->possub; liga!=NULL; liga=liga->next ) {
			    if ( liga->type == pst_ligature && strcmp(liga->u.lig.components,buf2)==0 )
			break;
			}
			if ( liga==NULL ) {
			    liga = chunkalloc(sizeof(PST));
			    liga->subtable = SFSubTableFindOrMake(sf,
				    CHR('l','i','g','a'),SCScriptFromUnicode(sc2),
				    gsub_ligature);
			    liga->subtable->lookup->store_in_afm = true;
			    liga->type = pst_ligature;
			    liga->next = sc1->possub;
			    sc1->possub = liga;
			    liga->u.lig.lig = sc1;
			    liga->u.lig.components = copy( buf2 );
			}
		    }
		}
	    }
	}
    }
    fclose(file);
return( 1 );
}

static void CheckMMAfmFile(SplineFont *sf,char *amfm_filename,char *fontname,EncMap *map) {
    /* the afm file should be in the same directory as the amfm file */
    /*  with the fontname as the filename */
    char *temp, *pt;

    free(sf->fontname);
    sf->fontname = copy(fontname);

    temp = galloc(strlen(amfm_filename)+strlen(fontname)+strlen(".afm")+1);
    strcpy(temp, amfm_filename);
    pt = strrchr(temp,'/');
    if ( pt==NULL ) pt = temp;
    else ++pt;
    strcpy(pt,fontname);
    pt += strlen(pt);
    strcpy(pt,".afm");
    if ( !LoadKerningDataFromAfm(sf,temp,map) ) {
	strcpy(pt,".AFM");
	LoadKerningDataFromAfm(sf,temp,map);
    }
    free(temp);
}

int LoadKerningDataFromAmfm(SplineFont *sf, char *filename,EncMap *map) {
    FILE *file=NULL;
    char buffer[280], *pt, lastname[257];
    int index, i;
    MMSet *mm = sf->mm;

    if ( mm!=NULL )
	file = fopen(filename,"r");
    pt = strstrmatch(filename,".amfm");
    if ( pt!=NULL ) {
	char *afmname = copy(filename);
	strcpy(afmname+(pt-filename),isupper(pt[1])?".AFM":".afm");
	LoadKerningDataFromAfm(mm->normal,afmname,map);
	free(afmname);
    }
    if ( file==NULL )
return( 0 );

    ff_progress_change_line2(_("Reading AFM file"));
    while ( fgets(buffer,sizeof(buffer),file)!=NULL ) {
	if ( strstrmatch(buffer,"StartMaster")!=NULL )
    break;
    }
    index = -1; lastname[0] = '\0';
    while ( fgets(buffer,sizeof(buffer),file)!=NULL ) {
	if ( strstrmatch(buffer,"EndMaster")!=NULL ) {
	    if ( lastname[0]!='\0' && index!=-1 && index<mm->instance_count )
		CheckMMAfmFile(mm->instances[index],filename,lastname,map);
	    index = -1; lastname[0] = '\0';
	} else if ( sscanf(buffer,"FontName %256s", lastname )== 1 ) {
	    /* Do Nothing, all done */
	} else if ( (pt = strstr(buffer,"WeightVector"))!=NULL ) {
	    pt += strlen("WeightVector");
	    while ( *pt==' ' || *pt=='[' ) ++pt;
	    i = 0;
	    while ( *pt!=']' && *pt!='\0' ) {
		if ( *pt=='0' )
		    ++i;
		else if ( *pt=='1' ) {
		    index = i;
	    break;
		}
		++pt;
	    }
	}
    }
    fclose(file);
return( true );
}

int CheckAfmOfPostscript(SplineFont *sf, const char *psname, EncMap *map) {
    char *new, *pt;
    int ret;
    int wasuc=false;

    new = galloc(strlen(psname)+6);
    strcpy(new,psname);
    pt = strrchr(new,'.');
    if ( pt==NULL ) pt = new+strlen(new);
    else wasuc = isupper(pt[1]);

    if ( sf->mm!=NULL ) {
	strcpy(pt,wasuc?".AMFM":".amfm");
	if ( !LoadKerningDataFromAmfm(sf,new,map)) {
	    strcpy(pt,wasuc?".amfm":".AMFM");
	    ret = LoadKerningDataFromAmfm(sf,new,map);
	} else
	    ret = true;
	/* The above routine reads from the afm file if one exist */
    } else {
	strcpy(pt,wasuc?".AFM":".afm");
	if ( !LoadKerningDataFromAfm(sf,new,map)) {
	    strcpy(pt,wasuc?".afm":".AFM");
	    ret = LoadKerningDataFromAfm(sf,new,map);
	} else
	    ret = true;
    }
    free(new);
return( ret );
}

