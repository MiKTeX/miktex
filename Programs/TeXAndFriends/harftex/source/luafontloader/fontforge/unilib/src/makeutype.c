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

/*
 * This program generates 2 different files based on the latest UnicodeData.txt
 * obtained from http://unicode.org which is then used to build FontForge
 *
 * To generate the latest files, you will first need to go and get these 4 files
 * and put them in the Unicode subdirectory:
 *	http://unicode.org/Public/UNIDATA/NamesList.txt
 *	http://unicode.org/Public/UNIDATA/PropList.txt
 *	http://unicode.org/Public/UNIDATA/UnicodeData.txt
 *
 * Next, you will need to build ./makeutype before you can use it:
 * Run "make makeutype"
 * or
 * Run "gcc -s -I../inc -o makeutype makeutype.c"
 *
 * Then run the executable binary "/makeutype".
 * This will create 2 files in the same directory:
 *	utype.c, utype.h
 * (please move utype.h into Fontforge's "../inc" subdirectory)
 *
 * When done building the updated files, you can clean-up by removing
 * NamesList.txt, PropList.txt,UnicodeData.txt, and the
 * binary executable file makeutype as they are no longer needed now.
 */


/* Build a ctype array out of the UnicodeData.txt and PropList.txt files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unibasics.h"
#define true 1
#define false 0

/*#define MAXC		0x600		/* Last upper/lower case dicodomy is Armenian 0x580, er, nope. 1fff (greek and latin extended) then full-width 0xff00 */
#define MAXC	65536
#define MAXA	18

/* These values get stored within flags[unicodechar={0..MAXC}] */
#define _LOWER		1
#define _UPPER		2
#define _TITLE		4
#define _DIGIT		8
#define _SPACE		0x10
#define _HEX		0x20
#define _COMBINING	0x40
#define _ALPHABETIC	0x80

unsigned short mytolower[MAXC];
unsigned short flags[MAXC];			/* 8 binary flags for each unicode.org character */

const char GeneratedFileMessage[] = "\n/* This file was generated using the program 'makeutype' */\n\n";
const char CantReadFile[] = "Can't find or read file %s\n";		/* exit(1) */
const char CantSaveFile[] = "Can't open or write to output file %s\n";	/* exit(2) */
const char NoMoreMemory[] = "Can't access more memory.\n";		/* exit(3) */
const char LineLengthBg[] = "Error with %s. Found line too long: %s\n";	/* exit(4) */

static void readin(void) {
    char buffer[512+1], buf2[300+1], oldname[301], *pt, *end, *pt1;
    long index, lc, uc, tc, flg, val, indexend, wasfirst;
    int  cc;
    FILE *fp;
    int i,j;

    buffer[512]='\0'; buf2[0] = buf2[300]='\0'; oldname[0]='\0';
    if ((fp = fopen("UnicodeData.txt","r"))==NULL ) {
	fprintf( stderr, CantReadFile,"UnicodeData.txt" );
	exit(1);
    }
    while ( fgets(buffer,sizeof(buffer)-1,fp)!=NULL ) {
	if (strlen(buffer)>=299) {	/* previous version was linelength of 300 chars, jul2012 */
	    fprintf( stderr, LineLengthBg,"UnicodeData.txt",buffer );
	    fprintf( stderr, "\n%s\n",buffer );

	    fclose(fp);
	    exit(4);
	}
	if ( *buffer=='#' )
    continue;
	flg = 0;
	/* Unicode character value */
	index = strtol(buffer,&end,16);
	if ( index>=MAXC )		/* For now can only deal with BMP !!!! */
    continue;
	pt = end;
	if ( *pt==';' ) {
	    ++pt;
	    /* buf2 = character name */
	    for ( pt1=pt; *pt1!=';' && *pt1!='\0'; ++pt1 );
	    strncpy(buf2,pt,pt1-pt); buf2[pt1-pt] = '\0'; pt = pt1;
	    if ( *pt==';' ) ++pt;
	    /* general category */
	    for ( pt1=pt; *pt1!=';' && *pt1!='\0'; ++pt1 );
	    if ( strncmp(pt,"Lu",pt1-pt)==0 )
		flg |= _UPPER|_ALPHABETIC;
	    else if ( strncmp(pt,"Ll",pt1-pt)==0 )
		flg |= _LOWER|_ALPHABETIC;
	    else if ( strncmp(pt,"Lt",pt1-pt)==0 )
		flg |= _TITLE|_ALPHABETIC;
	    else if ( strncmp(pt,"Lo",pt1-pt)==0 )
		flg |= _ALPHABETIC;
	    else if ( strncmp(pt,"Nd",pt1-pt)==0 )
		flg |= _DIGIT;
	    pt = pt1;
	    if ( *pt==';' ) ++pt;
	    /* Unicode combining classes, I do my own version later */
	    cc = strtol(pt,&end,16);
	    pt = end;
	    if ( *pt==';' ) ++pt;
	    /* Bidirectional Category */
	    for ( pt1=pt; *pt1!=';' && *pt1!='\0'; ++pt1 );
	    pt = pt1;
	    if ( *pt==';' ) ++pt;
	    while ( *pt!=';' && *pt!='\0' ) ++pt;
	    if ( *pt==';' ) ++pt;
	    /* Don't care about decimal digit value */
	    while ( *pt!=';' && *pt!='\0' ) ++pt;
	    if ( *pt==';' ) ++pt;
	    /* Don't care about digit value */
	    while ( *pt!=';' && *pt!='\0' ) ++pt;
	    if ( *pt==';' ) ++pt;
	    /* numeric value */
	    val = strtol(pt,&end,10);
	    if ( pt==end ) val = -1;
	    pt = end;
	    if ( *pt==';' ) ++pt;
	    /* Don't care about mirrored value */
	    while ( *pt!=';' && *pt!='\0' ) ++pt;
	    if ( *pt==';' ) ++pt;
	    /* Only care about old name (unicode 1.0) for control characters */
	    for ( pt1=pt; *pt1!=';' && *pt1!='\0'; ++pt1 );
	    strncpy(oldname,pt,pt1-pt); oldname[pt1-pt] = '\0';
	    if ( pt1-pt>100 ) oldname[100] = '\0'; pt = pt1;
	    if ( *pt==';' ) ++pt;
	    /* Don't care about 10646 comment field */
	    while ( *pt!=';' && *pt!='\0' ) ++pt;
	    if ( *pt==';' ) ++pt;
	    /* upper-case value */
	    uc = strtol(pt,&end,16);
	    if ( end==pt )
		uc = index;
	    pt = end;
	    if ( *pt==';' ) ++pt;
	    /* lower-case value */
	    lc = strtol(pt,&end,16);
	    if ( end==pt )
		lc = index;
	    pt = end;
	    if ( *pt==';' ) ++pt;
	    /* title-case value */
	    tc = strtol(pt,&end,16);
	    if ( end==pt )
		tc = index;
	    pt = end;
	    if ( *pt==';' ) ++pt;
	    if ( index>=MAXC )
    break;
	    mytolower[index]= lc;
	    flags[index] |= flg;
	    if ( strstr(buf2," First>")!=NULL )
		wasfirst = index;
	    else if ( strstr(buf2," Last>")!=NULL ) {
		for ( ; wasfirst<index; ++wasfirst ) {
		    mytolower[wasfirst]= wasfirst;
		    flags[wasfirst]  = flg;
		}
	    } else {
		if ( strcmp(buf2,"<control>")==0 ) {
		    strcat(buf2, " ");
		    strcat(buf2, oldname);
		}
	    }
	}
    }
    fclose(fp);

    if ((fp = fopen("PropList.txt","r"))==NULL ) {
	fprintf( stderr, CantReadFile, "PropList.txt" );
	exit(1);
    }
    while ( fgets(buffer,sizeof(buffer)-1,fp)!=NULL ) {
	flg = 0;
	if (strlen(buffer)>=299) {	/* previous version was linelength of 300 chars, jul2012 */
	    fprintf( stderr, LineLengthBg,"PropList.txt",buffer );
	    fclose(fp);
	    exit(4);
	}
	if ( true || strncmp(buffer,"Property dump for:", strlen("Property dump for:"))==0 ) {
	    if ( strstr(buffer, "(White space)")!=NULL || strstr(buffer, "White_Space")!=NULL )
		flg = _SPACE;
	    else if ( strstr(buffer, "(Alphabetic)")!=NULL || strstr(buffer, "Alphabetic")!=NULL )
		flg = _ALPHABETIC;
	    else if ( strstr(buffer, "(Hex Digit)")!=NULL || strstr(buffer, "Hex_Digit")!=NULL )
		flg = _HEX;
	    else if ( strstr(buffer, "(Combining)")!=NULL || strstr(buffer, "COMBINING")!=NULL )
		flg = _COMBINING;
	    if ( flg!=0 ) {
		if (( buffer[0]>='0' && buffer[0]<='9') || (buffer[0]>='A' && buffer[0]<='F')) {
		    index = wasfirst = strtol(buffer,NULL,16);
		    if ( buffer[4]=='.' && buffer[5]=='.' )
			index = strtol(buffer+6,NULL,16);
		    for ( ; wasfirst<=index && wasfirst<=0xffff; ++wasfirst )		/* BMP !!!!! */
			flags[wasfirst] |= flg;
		}
	    }
	}
    }
    fclose(fp);

    if ((fp = fopen("NamesList.txt","r"))==NULL ) {
	fprintf( stderr, CantReadFile, "NamesList.txt" );
	exit(1);
    }
    while ( fgets(buffer,sizeof(buffer)-1,fp)!=NULL ) {
	flg = 0;
	if (strlen(buffer)>=511) {
	    fprintf( stderr, LineLengthBg,"NamesList.txt",buffer );
	    fclose(fp);
	    exit(4);
	}
	if ( (index = strtol(buffer,NULL,16))!=0 ) {
	    if ( strstr(buffer, "COMBINING")!=NULL )
		flg = _COMBINING;

	    if ( index<0xffff )		/* !!!!! BMP */
		flags[wasfirst] |= flg;
	}
    }
    fclose(fp);
}

static void readcorpfile(char *prefix, char *corp) {
    char buffer[300+1], buf2[300+1], *pt, *end, *pt1;
    long index;
    FILE *fp;

    buffer[300]='\0'; buf2[0] = buf2[300]='\0';
    if ((fp = fopen(corp,"r"))==NULL ) {
	fprintf( stderr, CantReadFile, corp );		/* Not essential */
return;
    }
    while ( fgets(buffer,sizeof(buffer)-1,fp)!=NULL ) {
	if (strlen(buffer)>=299) {
	    fprintf( stderr, LineLengthBg,corp,buffer );
	    fclose(fp);
	    exit(4);
	}
	if ( *buffer=='#' )
    continue;
	/* code */
	index = strtol(buffer,&end,16);
	pt = end;
	if ( *pt==';' ) {
	    ++pt;
	    while ( *pt!=';' && *pt!='\0' ) ++pt;
	    if ( *pt==';' ) ++pt;
	    /* character name */
	    for ( pt1=pt; *pt1!=';' && *pt1!='\0' && *pt1!='\n' && *pt1!='\r'; ++pt1 );
	    strncpy(buf2,pt,pt1-pt); buf2[pt1-pt] = '\0'; pt = pt1;
	    if ( *pt==';' ) ++pt;
	    /* character decomposition */
	    if ( index>=MAXC )
    break;
	}
    }
    fclose(fp);
}

static void dump() {
    FILE *header, *data;
    int i,j;

    header=fopen("utype.h","w");
    data = fopen("utype.c","w");

    if ( header==NULL || data==NULL ) {
	fprintf( stderr, CantSaveFile, "(utype.[ch])" );
	if ( header ) fclose( header );
	if ( data   ) fclose( data );
	exit(2);
    }

    fprintf( header, "#ifndef _UTYPE_H\n" );
    fprintf( header, "#define _UTYPE_H\n" );

    fprintf( header, GeneratedFileMessage );

    fprintf( header, "#include <ctype.h>\t\t/* Include here so we can control it. If a system header includes it later bad things happen */\n" );
    fprintf( header, "#include <unibasics.h>\t\t/* Include here so we can use pre-defined int types to correctly size constant data arrays. */\n" );
    fprintf( header, "#ifdef tolower\n" );
    fprintf( header, "# undef tolower\n" );
    fprintf( header, "#endif\n" );
    fprintf( header, "#ifdef isupper\n" );
    fprintf( header, "# undef isupper\n" );
    fprintf( header, "#endif\n" );
    fprintf( header, "#ifdef isalpha\n" );
    fprintf( header, "# undef isalpha\n" );
    fprintf( header, "#endif\n" );
    fprintf( header, "#ifdef isdigit\n" );
    fprintf( header, "# undef isdigit\n" );
    fprintf( header, "#endif\n" );
    fprintf( header, "#ifdef isalnum\n" );
    fprintf( header, "# undef isalnum\n" );
    fprintf( header, "#endif\n" );
    fprintf( header, "#ifdef isspace\n" );
    fprintf( header, "# undef isspace\n" );
    fprintf( header, "#endif\n" );
    fprintf( header, "#ifdef ishexdigit\n" );
    fprintf( header, "# undef ishexdigit\n" );
    fprintf( header, "#endif\n\n" );

    fprintf( header, "#define ____L	0x%0x\n", _LOWER );
    fprintf( header, "#define ____U	0x%0x\n", _UPPER );
    fprintf( header, "#define ____TITLE	0x%0x\n", _TITLE );
    fprintf( header, "#define ____D	0x%0x\n", _DIGIT );
    fprintf( header, "#define ____S	0x%0x\n", _SPACE );
    fprintf( header, "#define ____X	0x%0x\n", _HEX );
    fprintf( header, "#define ____COMBINE	0x%0x\n", _COMBINING );
    fprintf( header, "#define ____AL	0x%0x\n", _ALPHABETIC );
    fprintf( header, "\n" );

    fprintf( header, "extern const unsigned short ____tolower[];\n" );
    fprintf( header, "extern const unsigned short ____utype[];\n\n" );


    fprintf( header, "#define tolower(ch) (____tolower[(ch)+1])\n" );
    fprintf( header, "#define isupper(ch) (____utype[(ch)+1]&____U)\n" );
    fprintf( header, "#define isalpha(ch) (____utype[(ch)+1]&(____L|____U|____TITLE|____AL))\n" );
    fprintf( header, "#define isdigit(ch) (____utype[(ch)+1]&____D)\n" );
    fprintf( header, "#define isalnum(ch) (____utype[(ch)+1]&(____L|____U|____TITLE|____AL|____D))\n" );
    fprintf( header, "#define isspace(ch) (____utype[(ch)+1]&____S)\n" );
    fprintf( header, "#define ishexdigit(ch) (____utype[(ch)+1]&____X)\n" );
    fprintf( header, "#define iscombining(ch) (____utype[(ch)+1]&____COMBINE)\n" );

    fprintf( header, "\n#endif\n" );
    fclose( header );

    fprintf( data, "#include \"utype.h\"\n" );
    fprintf( data, GeneratedFileMessage );
    fprintf( data, "const unsigned short ____tolower[]= { 0,\n" );
    for ( i=0; i<MAXC; i+=j ) {
	fprintf( data, " " );
	for ( j=0; j<8 && i+j<MAXC-1; ++j )
	    fprintf(data, " 0x%04x,", mytolower[i+j]);
	if ( i+j==MAXC-1 ) {
	    fprintf(data, " 0x%04x\n};\n\n", mytolower[i+j]);
    break;
	} else
	    if ( (i & 63)==0 )
		fprintf( data, "\t/* 0x%04x */\n",i);
	    else
		fprintf( data, "\n");
    }
    fprintf( data, "const unsigned short ____utype[] = { 0,\n" );
    for ( i=0; i<MAXC; i+=j ) {
	fprintf( data, " " );
	for ( j=0; j<8 && i+j<MAXC-1; ++j )
	    fprintf(data, " 0x%02x,", flags[i+j]);
	if ( i+j==MAXC-1 ) {
	    fprintf(data, " 0x%02x\n};\n\n", flags[i+j]);
    break;
	} else
	    if ( (i & 63)==0 )
		fprintf( data, "\t/* 0x%04x */\n",i);
	    else
		fprintf( data, "\n");
    }

    fclose( data );
}

int main() {
    readin();				/* load the "official" Unicode data from unicode.org */
    /* Apple's file contains no interesting information that I can see */
    /* Adobe's file is interesting, but should only be used conditionally */
    /*  so apply at a different level */
    /* readcorpfile("ADOBE ", "AdobeCorporateuse.txt"); */
    dump();				/* create utype.h and utype.c */
return( 0 );
}
