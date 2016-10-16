/* Copyright (C) 2010-2012 by George Williams */
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

/* Support for woff files */
/* Which are defined here: http://people.mozilla.com/~jkew/woff/woff-2009-09-16.html */
/* Basically sfnts with compressed tables and some more metadata */

#include "fontforge.h"
#include <math.h>
#include <ctype.h>

# include <zlib.h>

static void copydata(FILE *to,int off_to,FILE *from,int off_from, int len) {
    int ch, i;

    fseek(to  ,off_to  ,SEEK_SET);
    fseek(from,off_from,SEEK_SET);
    for ( i=0; i<len; ++i ) {
	ch = getc(from);
	putc(ch,to);
    }
}

#define CHUNK	(128*1024)
/* Copied with few mods from the zlib usage examples */

static int compressOrNot(FILE *to,int off_to, FILE *from,int off_from,
	int len, int forcecompress ) {
    char in[CHUNK];
    char out[CHUNK];
    z_stream strm;
    int ret, err=0;
    int amount;
    FILE *tmp;
    int uncompLen = len;

    /* Empty table, nothing to do */
    if ( len==0 )
return(0);

    fseek(from,off_from,SEEK_SET);
    memset(&strm,0,sizeof(strm));
    ret = deflateInit(&strm,Z_DEFAULT_COMPRESSION);
    if ( ret!=Z_OK ) {
	fprintf( stderr,"Compression initialization failed.\n" );
return(0);
    }
    tmp = tmpfile2();

    do {
	if ( len<=0 ) {
            (void)deflateEnd(&strm);
    break;
        }
	amount = len;
	if ( amount>CHUNK )
	    amount = CHUNK;
        strm.avail_in = fread(in, 1, amount, from);
	len -= strm.avail_in;
        if (ferror(from)) {
            (void)deflateEnd(&strm);
	    fprintf( stderr, "IO error.\n" );
    break;
        }
        if (strm.avail_in == 0)
    break;
        strm.next_in = (unsigned char*)in;
        do {
            strm.avail_out = CHUNK;
            strm.next_out = (unsigned char*)out;
            ret = deflate(&strm, len==0 ? Z_FINISH : Z_NO_FLUSH);
	    if ( ret==Z_STREAM_ERROR ) {
		(void)deflateEnd(&strm);
		fprintf( stderr, "Compression failed somehow.\n");
		err = 1;
	break;
	    }
	    amount = CHUNK - strm.avail_out;
	    if ( fwrite(out,1,amount,tmp)!= amount || ferror(tmp) ) {
		(void)deflateEnd(&strm);
		fprintf( stderr, "IO Error.\n");
		err=1;
	break;
	    }
	} while ( strm.avail_out==0 );
	if ( err )
    break;
    } while ( ret!=Z_STREAM_END );
    (void)deflateEnd(&strm);

    if ( strm.total_out>=uncompLen ) {
	/* Didn't actually make the data smaller, so store uncompressed */
	fclose(tmp);
	copydata(to,off_to,from,off_from,uncompLen);
return( uncompLen );
    } else {
	copydata(to,off_to,tmp,0,strm.total_out);
	fclose(tmp);
return( strm.total_out );
    }
}

typedef struct {
    int index;
    int offset;
} tableOrderRec;

static int
compareOffsets(const void * lhs, const void * rhs)
{
    const tableOrderRec * a = (const tableOrderRec *) lhs;
    const tableOrderRec * b = (const tableOrderRec *) rhs;
    /* don't simply return a->offset - b->offset because these are unsigned
       offset values; could convert to int, but possible integer overflow */
    return a->offset > b->offset ? 1 :
           a->offset < b->offset ? -1 :
           0;
}

int _WriteWOFFFont(FILE *woff,SplineFont *sf, enum fontformat format,
	int32 *bsizes, enum bitmapformat bf,int flags,EncMap *enc,int layer) {
    int ret;
    FILE *sfnt;
    int major=sf->woffMajor, minor=sf->woffMinor;
    int flavour, num_tabs;
    int filelen, len;
    int i;
    int compLen, uncompLen, newoffset;
    int tag, checksum, offset;
    int tab_start;
    tableOrderRec *tableOrder = NULL;

    if ( major==woffUnset ) {
	struct ttflangname *useng;
	major = 1; minor = 0;
	for ( useng=sf->names; useng!=NULL; useng=useng->next )
	    if ( useng->lang==0x409 )
	break;
	if ( useng!=NULL && useng->names[ttf_version]!=NULL &&
		sscanf(useng->names[ttf_version], "Version %d.%d", &major, &minor)>=1 ) {
	    /* All done */
	} else if ( sf->subfontcnt!=0 ) {
	    major = floor(sf->cidversion);
	    minor = floor(1000.*(sf->cidversion-major));
	} else if ( sf->version!=NULL ) {
	    char *pt=sf->version;
	    char *end;
	    while ( *pt && !isdigit(*pt) && *pt!='.' ) ++pt;
	    if ( *pt ) {
		major = strtol(pt,&end,10);
		if ( *end=='.' )
		    minor = strtol(end+1,NULL,10);
	    }
	}
    }

    format = sf->subfonts!=NULL ? ff_otfcid :
		sf->layers[layer].order2 ? ff_ttf : ff_otf;
    sfnt = tmpfile2();
    ret = _WriteTTFFont(sfnt,sf,format,bsizes,bf,flags,enc,layer);
    if ( !ret ) {
	fclose(sfnt);
return( ret );
    }

    fseek(sfnt,0,SEEK_END);
    filelen = ftell(sfnt);
    rewind(sfnt);

    flavour = getlong(sfnt);
    /* The woff standard says we should accept all flavours of sfnt, so can't */
    /*  test flavour to make sure we've got a valid sfnt */
    /* But we can test the rest of the header for consistancy */
    num_tabs = getushort(sfnt);
    (void) getushort(sfnt);
    (void) getushort(sfnt);
    (void) getushort(sfnt);

    /*
     * At this point _WriteTTFFont should have generated an sfnt file with
     * valid checksums, correct padding and no extra gaps. However, the order
     * of the font tables in the original sfnt font must also be preserved so
     * that WOFF consumers can recover the original offsets as well as the
     * original font. Hence we will compress and write the font tables into
     * the WOFF file using the original offset order. Note that the order of
     * tables may not be the same as the one of table directory entries.
     * See https://github.com/fontforge/fontforge/issues/926
     */
    tableOrder = (tableOrderRec *) malloc(num_tabs * sizeof(tableOrderRec));
    if (!tableOrder) {
        fclose(sfnt);
        return false;
    }
    for ( i=0; i<num_tabs; ++i ) {
        fseek(sfnt,(3 + 4*i + 2)*sizeof(int32),SEEK_SET);
        tableOrder[i].index = i;
        tableOrder[i].offset = getlong(sfnt);
    }
    qsort(tableOrder, num_tabs, sizeof(tableOrderRec), compareOffsets);

    /* Now generate the WOFF file */
    rewind(woff);
    putlong(woff,CHR('w','O','F','F'));
    putlong(woff,flavour);
    putlong(woff,0);		/* Off: 8. total length of file, fill in later */
    putshort(woff,num_tabs);
    putshort(woff,0);		/* Must be zero */
    putlong(woff,filelen);
    putshort(woff,major);	/* Major and minor version numbers of font */
    putshort(woff,minor);
    putlong(woff,0);		/* Off: 24. Offset to metadata table */
    putlong(woff,0);		/* Off: 28. Length (compressed) of metadata */
    putlong(woff,0);		/* Off: 32. Length (uncompressed) */
    putlong(woff,0);		/* Off: 36. Offset to private data */
    putlong(woff,0);		/* Off: 40. Length of private data */

    tab_start = ftell(woff);
    for ( i=0; i<5*num_tabs; ++i )
	putlong(woff,0);

    for ( i=0; i<num_tabs; ++i ) {
	fseek(sfnt,(3 + 4*tableOrder[i].index)*sizeof(int32),SEEK_SET);
	tag = getlong(sfnt);
	checksum = getlong(sfnt);
	offset = getlong(sfnt);
	uncompLen = getlong(sfnt);
	newoffset = ftell(woff);
	compLen = compressOrNot(woff,newoffset,sfnt,offset,uncompLen,false);
	if ( (ftell(woff)&3)!=0 ) {
	    /* Pad to a 4 byte boundary */
	    if ( ftell(woff)&1 )
		putc('\0',woff);
	    if ( ftell(woff)&2 )
		putshort(woff,0);
	}
	fseek(woff,tab_start+(5*tableOrder[i].index)*sizeof(int32),SEEK_SET);
	putlong(woff,tag);
	putlong(woff,newoffset);
	putlong(woff,compLen);
	putlong(woff,uncompLen);
	putlong(woff,checksum);
	fseek(woff,0,SEEK_END);
    }
    fclose(sfnt);

    if ( sf->woffMetadata!= NULL ) {
	int uncomplen = strlen(sf->woffMetadata);
	uLongf complen = 2*uncomplen;
	char *temp=malloc(complen+1);
	newoffset = ftell(woff);
	compress((unsigned char*)temp,&complen,(unsigned char*)sf->woffMetadata,uncomplen);
	fwrite(temp,1,complen,woff);
	free(temp);
	if ( (ftell(woff)&3)!=0 ) {
	    /* Pad to a 4 byte boundary */
	    if ( ftell(woff)&1 )
		putc('\0',woff);
	    if ( ftell(woff)&2 )
		putshort(woff,0);
	}
	fseek(woff,24,SEEK_SET);
	putlong(woff,newoffset);
	putlong(woff,complen);
	putlong(woff,uncomplen);
	fseek(woff,0,SEEK_END);
    }

    fseek(woff,0,SEEK_END);
    len = ftell(woff);
    fseek(woff,8,SEEK_SET);
    putlong(woff,len);

    free(tableOrder);
return( true );		/* No errors */
}

int WriteWOFFFont(char *fontname,SplineFont *sf, enum fontformat format,
	int32 *bsizes, enum bitmapformat bf,int flags,EncMap *enc,int layer) {
    FILE *woff;
    int ret;

    if ( strstr(fontname,"://")!=NULL ) {
	if (( woff = tmpfile2())==NULL )
return( 0 );
    } else {
	if (( woff=fopen(fontname,"wb+"))==NULL )
return( 0 );
    }
    ret = _WriteWOFFFont(woff,sf,format,bsizes,bf,flags,enc,layer);
    if ( fclose(woff)==-1 )
return( 0 );
return( ret );
}
