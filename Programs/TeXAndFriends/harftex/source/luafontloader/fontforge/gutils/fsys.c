/* Copyright (C) 2000-2004 by George Williams */
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

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif
#include "basics.h"
#include "ustring.h"
#include "gfile.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>		/* for mkdir */
#include <unistd.h>

#ifdef _WIN32
#define MKDIR(A,B) mkdir(A)
#else
#define MKDIR(A,B) mkdir(A,B)
#endif

static char dirname_[1024];

char *GFileGetAbsoluteName(char *name, char *result, int rsiz) {
    /* result may be the same as name */
    char buffer[1000];

    if ( *name!='/' ) {
	char *pt, *spt, *rpt, *bpt;

	if ( dirname_[0]=='\0' ) {
	  if (getcwd(dirname_,sizeof(dirname_)) == NULL)
return(result);
	}
	strcpy(buffer,dirname_);
	if ( buffer[strlen(buffer)-1]!='/' )
	    strcat(buffer,"/");
	strcat(buffer,name);

	/* Normalize out any .. */
	spt = rpt = buffer;
	while ( *spt!='\0' ) {
	    if ( *spt=='/' ) ++spt;
	    for ( pt = spt; *pt!='\0' && *pt!='/'; ++pt );
	    if ( pt==spt )	/* Found // in a path spec, reduce to / (we've*/
		strcpy(spt,pt); /*  skipped past the :// of the machine name) */
	    else if ( pt==spt+1 && spt[0]=='.' )	/* Noop */
		strcpy(spt,pt);
	    else if ( pt==spt+2 && spt[0]=='.' && spt[1]=='.' ) {
		for ( bpt=spt-2 ; bpt>rpt && *bpt!='/'; --bpt );
		if ( bpt>=rpt && *bpt=='/' ) {
		    strcpy(bpt,pt);
		    spt = bpt;
		} else {
		    rpt = pt;
		    spt = pt;
		}
	    } else
		spt = pt;
	}
	name = buffer;
	if ( rsiz>sizeof(buffer)) rsiz = sizeof(buffer);	/* Else valgrind gets unhappy */
    }
    if (result!=name) {
	strncpy(result,name,rsiz);
	result[rsiz-1]='\0';
    }
return(result);
}

char *GFileNameTail(const char *oldname) {
    char *pt;

    pt = strrchr(oldname,'/');
    if ( pt !=NULL )
return( pt+1);
    else
return( (char *)oldname );
}

char *GFileAppendFile(char *dir,char *name,int isdir) {
    char *ret, *pt;

    ret = xmalloc((strlen(dir)+strlen(name)+3));
    strcpy(ret,dir);
    pt = ret+strlen(ret);
    if ( pt>ret && pt[-1]!='/' )
	*pt++ = '/';
    strcpy(pt,name);
    if ( isdir ) {
	pt += strlen(pt);
	if ( pt>ret && pt[-1]!='/' ) {
	    *pt++ = '/';
	    *pt = '\0';
	}
    }
return(ret);
}

int GFileExists(const char *file) {
return( access(file,0)==0 );
}

int GFileModifyable(const char *file) {
return( access(file,02)==0 );
}

int GFileReadable(char *file) {
return( access(file,04)==0 );
}
