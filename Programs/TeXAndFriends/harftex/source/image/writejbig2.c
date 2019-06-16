/*

writejbig2.c

Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
Copyright 2006-2013 Taco Hoekwater <taco@luatex.org>
Copyright 2003-2013 Hartmut Henkel <hartmut@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU General Public License along with
LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

/*tex

This is experimental JBIG2 image support to pdfTeX. JBIG2 image decoding is part
of Adobe PDF-1.4, and requires Acroread 5.0 or later.

References
==========

* 14492 FCD: Information technology -- coded representation of picture
and audio information -- lossy/lossless coding of bi-level images /
JBIG committee, 1999 July 16. This JBIG2 Working Draft is available from
http://www.jpeg.org/public/fcd14492.pdf. The references in the C-code
correspond to the sections of this document.

* PDF Reference, 5th edition, version 1.6, 1985--2005 Adobe Systems
Incorporated. Available online:

http://partners.adobe.com/public/developer/en/pdf/PDFReference16.pdf

News
====

31 May 2006: no need to wait for |endoffileflag| in sequential access
organization.

10 May 2006: |ygetc()| for some catching of broken JBIG2 files; modify to
accept Example 3.4 from PDFRef 5th ed. with short end-of-file segment.

09 May 2006: |pages_maketree()| and |segments_maketree()| by AVL tree,
some cleaning up.

06 May 2006: File list replaced by AVL tree; |new_fileinfo()|,
|new_pageinfo()|.

04 May 2006: Updated for pdftex-1.40-beta-20060213.

08 Jan. 2003: Added |flushjbig2page0objects()| function. Now at the end
of the pdfTeX run all pending page0 objects are written out.

08 Jan. 2003: Release on private webpage.

04 Jan. 2003: Completely rewritten. Now with some data structures.
Rudimentary local file and image bookkeeping. Multiple image inclusion
from one JBIG2 file. Only required page0 segments are marked for
inclusion.

13 Nov. 2002: pdfcrypting removed.

08 Dec. 2002: bug in page 0 stream writing repaired.
Strategy for multiple page inclusion from same JBIG2 file: When writing
1st image, create fresh PDF object for page 0, and include any page
0 segments from complete file (even if these segments are not needed
for image). When writing next image, check by filename comparison if
PDF object for page 0 of this JBIG2 file has already been written. This
can only remember the file name for the direct predecessor JBIG2 image
(but images of other types might come inbetween). If such page 0 PDF
object exists, reference it. Else create fresh one.

09 Dec. 2002: JBIG2 seg. page numbers > 0 are now set to 1, see PDF Ref.

*/

#undef DEBUG

#include "ptexlib.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "image/image.h"

/*tex Table 7.3: Segment types */

#define M_SymbolDictionary 0
#define M_IntermediateTextRegion 4
#define M_ImmediateTextRegion 6
#define M_ImmediateLosslessTextRegion 7
#define M_PatternDictionary 16
#define M_IntermediateHalftoneRegion 20
#define M_ImmediateHalftoneRegion 22
#define M_ImmediateLosslessHalftoneRegion 23
#define M_IntermediateGenericRegion 36
#define M_ImmediateGenericRegion 38
#define M_ImmediateLosslessGenericRegion 39
#define M_IntermediateGenericRefinementRegion 40
#define M_ImmediateGenericRefinementRegion 42
#define M_ImmediateLosslessGenericRefinementRegion 43
#define M_PageInformation 48
#define M_EndOfPage 49
#define M_EndOfStripe 50
#define M_EndOfFile 51
#define M_Profiles 52
#define M_Tables 53
#define M_Extension 62

typedef enum { INITIAL, HAVEINFO, WRITEPDF } PHASE;

typedef struct _LITEM {
    struct _LITEM *prev;
    struct _LITEM *next;
    void *d;
} LITEM;

typedef struct _LIST {
    LITEM *first;
    LITEM *last;
    struct avl_table *tree;
} LIST;

typedef struct _SEGINFO {
    unsigned long segnum;
    boolean isrefered;
    boolean refers;
    /*tex Set by |readseghdr|: */
    unsigned int seghdrflags;
    boolean pageassocsizeflag;
    unsigned int reftosegcount;
    unsigned int countofrefered;
    unsigned int fieldlen;
    unsigned int segnumwidth;
    long segpage;
    unsigned long segdatalen;
    unsigned long hdrstart;
    unsigned long hdrend;
    unsigned long datastart;
    unsigned long dataend;
    /*tex Set by |checkseghdrflags|: */
    boolean endofstripeflag;
    boolean endofpageflag;
    boolean pageinfoflag;
    boolean endoffileflag;
} SEGINFO;

typedef struct _PAGEINFO {
    LIST segments;
    unsigned long pagenum;
    unsigned int width;
    unsigned int height;
    unsigned int xres;
    unsigned int yres;
    unsigned int pagesegmentflags;
    unsigned int stripinginfo;
    unsigned int stripedheight;
} PAGEINFO;

typedef struct _FILEINFO {
    FILE *file;
    char *filepath;
    long filesize;
    /*tex Not including |page0|: */
    LIST pages;
    LIST page0;
    /*tex Set by |readfilehdr| */
    unsigned int filehdrflags;
    boolean sequentialaccess;
    unsigned long numofpages;
    /*tex Set by |get_jbig2_info| */
    unsigned long streamstart;
    unsigned long pdfpage0objnum;
    PHASE phase;
} FILEINFO;

static struct avl_table *file_tree = NULL;

static int comp_file_entry(const void *pa, const void *pb, void *p)
{
    (void) p;
    return strcmp(((const FILEINFO *) pa)->filepath,((const FILEINFO *) pb)->filepath);
}

static int comp_page_entry(const void *pa, const void *pb, void *p)
{
    (void) p;
    return (int) (((const PAGEINFO *) pa)->pagenum - ((const PAGEINFO *) pb)->pagenum);
}

static int comp_segment_entry(const void *pa, const void *pb, void *p)
{
    (void) p;
    return (int) (((const SEGINFO *) pa)->segnum - ((const SEGINFO *) pb)->segnum);
}

static int ygetc(FILE * stream)
{
    int c = getc(stream);
    if (c < 0) {
        if (c == EOF)
            normal_error("readjbig2","premature end file");
        else
            normal_error("readjbig2","can't happen");
    }
    return c;
}

static void initlinkedlist(LIST * lp)
{
    lp->first = NULL;
    lp->last = NULL;
    lp->tree = NULL;
}

static LIST *litem_append(LIST * lp)
{
    LITEM *ip;
    ip = xtalloc(1, LITEM);
    if (lp->first == NULL) {
        lp->first = ip;
        ip->prev = NULL;
    } else {
        lp->last->next = ip;
        ip->prev = lp->last;
    }
    lp->last = ip;
    ip->next = NULL;
    ip->d = NULL;
    return lp;
}

static FILEINFO *new_fileinfo(void)
{
    FILEINFO *fip;
    fip = xtalloc(1, FILEINFO);
    fip->file = NULL;
    fip->filepath = NULL;
    fip->filesize = 0;
    initlinkedlist(&(fip->pages));
    initlinkedlist(&(fip->page0));
    fip->filehdrflags = 0;
    fip->sequentialaccess = false;
    fip->numofpages = 0;
    fip->streamstart = 0;
    fip->pdfpage0objnum = 0;
    fip->phase = INITIAL;
    return fip;
}

static PAGEINFO *new_pageinfo(void)
{
    PAGEINFO *pip;
    pip = xtalloc(1, PAGEINFO);
    initlinkedlist(&(pip->segments));
    pip->pagenum = 0;
    pip->width = 0;
    pip->height = 0;
    pip->xres = 0;
    pip->yres = 0;
    pip->pagesegmentflags = 0;
    pip->stripinginfo = 0;
    pip->stripedheight = 0;
    return pip;
}

static void init_seginfo(SEGINFO * sip)
{
    sip->segnum = 0;
    sip->isrefered = false;
    sip->refers = false;
    sip->seghdrflags = 0;
    sip->pageassocsizeflag = false;
    sip->reftosegcount = 0;
    sip->countofrefered = 0;
    sip->fieldlen = 0;
    sip->segnumwidth = 0;
    sip->segpage = 0;
    sip->segdatalen = 0;
    sip->hdrstart = 0;
    sip->hdrend = 0;
    sip->datastart = 0;
    sip->dataend = 0;
    sip->endofstripeflag = false;
    sip->endofpageflag = false;
    sip->pageinfoflag = false;
    sip->endoffileflag = false;
}

static void pages_maketree(LIST * plp)
{
    LITEM *ip;
    void **aa;
    assert(plp->tree == NULL);
    plp->tree = avl_create(comp_page_entry, NULL, &avl_xallocator);
    assert(plp->tree != NULL);
    for (ip = plp->first; ip != NULL; ip = ip->next) {
        aa = avl_probe(plp->tree, (PAGEINFO *) ip->d);
        assert(aa != NULL);
    }
}

static void segments_maketree(LIST * slp)
{
    LITEM *ip;
    void **aa;
    assert(slp->tree == NULL);
    slp->tree = avl_create(comp_segment_entry, NULL, &avl_xallocator);
    assert(slp->tree != NULL);
    for (ip = slp->first; ip != NULL; ip = ip->next) {
        aa = avl_probe(slp->tree, (SEGINFO *) ip->d);
        assert(aa != NULL);
    }
}

static PAGEINFO *find_pageinfo(LIST * plp, unsigned long pagenum)
{
    PAGEINFO tmp;
    tmp.pagenum = pagenum;
    assert(plp->tree != NULL);
    return (PAGEINFO *) avl_find(plp->tree, &tmp);
}

static SEGINFO *find_seginfo(LIST * slp, unsigned long segnum)
{
    SEGINFO tmp;
    tmp.segnum = segnum;
    assert(slp->tree != NULL);
    return (SEGINFO *) avl_find(slp->tree, &tmp);
}

unsigned int read2bytes(FILE * f)
{
    unsigned int c = (unsigned int) ygetc(f);
    return (c << 8) + (unsigned int) ygetc(f);
}

unsigned int read4bytes(FILE * f)
{
    unsigned int l = read2bytes(f);
    return (l << 16) + read2bytes(f);
}

static unsigned long getstreamlen(LITEM * slip, boolean refer)
{
    SEGINFO *sip;
    unsigned long len = 0;
    for (; slip != NULL; slip = slip->next) {
        sip = slip->d;
        if (refer || sip->isrefered)
            len += sip->hdrend - sip->hdrstart + sip->dataend - sip->datastart;
    }
    return len;
}

static void readfilehdr(FILEINFO * fip)
{
    unsigned int i;
    /*tex Annex D.4: File header syntax */
    /*tex Annex D.4.1: ID string */
    unsigned char jbig2_id[] = { 0x97, 'J', 'B', '2', 0x0d, 0x0a, 0x1a, 0x0a };
    xfseek(fip->file, 0, SEEK_SET, fip->filepath);
    for (i = 0; i < 8; i++)
        if (ygetc(fip->file) != jbig2_id[i])
            normal_error("readjbig2","ID string missing");
    /*tex Annex D.4.2: File header flags */
    fip->filehdrflags = (unsigned int) ygetc(fip->file);
    fip->sequentialaccess = (fip->filehdrflags & 0x01) ? true : false;
    if (fip->sequentialaccess) {
        /*tex Annex D.1 vs. Annex D.2 */
        xfseek(fip->file, 0, SEEK_END, fip->filepath);
        fip->filesize = (long) xftello(fip->file, fip->filepath);
        xfseek(fip->file, 9, SEEK_SET, fip->filepath);
    }
    /*tex Annex D.4.3: Number of pages */
    if (( !(fip->filehdrflags >> 1)) & 0x01) {
        /*tex The known number of pages: */
        fip->numofpages = read4bytes(fip->file);
    }
    /*tex End of file header */
}

static void checkseghdrflags(SEGINFO * sip)
{
    sip->endofstripeflag = false;
    sip->endofpageflag = false;
    sip->pageinfoflag = false;
    sip->endoffileflag = false;
    /*tex Table 7.3: Segment types */
    switch (sip->seghdrflags & 0x3f) {
        case M_SymbolDictionary:
        case M_IntermediateTextRegion:
        case M_ImmediateTextRegion:
        case M_ImmediateLosslessTextRegion:
        case M_PatternDictionary:
        case M_IntermediateHalftoneRegion:
        case M_ImmediateHalftoneRegion:
        case M_ImmediateLosslessHalftoneRegion:
        case M_IntermediateGenericRegion:
        case M_ImmediateGenericRegion:
        case M_ImmediateLosslessGenericRegion:
        case M_IntermediateGenericRefinementRegion:
        case M_ImmediateGenericRefinementRegion:
        case M_ImmediateLosslessGenericRefinementRegion:
            break;
        case M_PageInformation:
            sip->pageinfoflag = true;
            break;
        case M_EndOfPage:
            sip->endofpageflag = true;
            break;
        case M_EndOfStripe:
            sip->endofstripeflag = true;
            break;
        case M_EndOfFile:
            sip->endoffileflag = true;
            break;
        case M_Profiles:
        case M_Tables:
        case M_Extension:
            break;
        default:
            normal_error("readjbig2","unknown segment type file");
            break;
    }
}

/*tex

    For first reading of file; return value tells if header been read.

*/

static boolean readseghdr(FILEINFO * fip, SEGINFO * sip)
{
    unsigned int i;
    sip->hdrstart = xftell(fip->file, fip->filepath);
    if (fip->sequentialaccess && sip->hdrstart == (unsigned) fip->filesize) {
        /*tex No endoffileflag is ok for sequential access. */
        return false;
    }
    /*tex Table 7.2.2: Segment number */
    sip->segnum = read4bytes(fip->file);
    /*tex Table 7.2.3: Segment header flags */
    sip->seghdrflags = (unsigned int) ygetc(fip->file);
    checkseghdrflags(sip);
    if (fip->sequentialaccess && sip->endoffileflag) {
        /*
            Accept shorter segment, makes it compliant with Example 3.4 of
            PDFRef. 5th ed.
        */
        return true;
    }
    sip->pageassocsizeflag = ((sip->seghdrflags >> 6) & 0x01) ? true : false;
    /*tex Table 7.2.4: Referred-to segment count and retention flags */
    sip->reftosegcount = (unsigned int) ygetc(fip->file);
    sip->countofrefered = sip->reftosegcount >> 5;
    if (sip->countofrefered < 5)
        sip->fieldlen = 1;
    else {
        sip->fieldlen = 5 + sip->countofrefered / 8;
        xfseek(fip->file, sip->fieldlen - 1, SEEK_CUR, fip->filepath);
    }
    /*tex Table 7.2.5: Referred-to segment numbers */
    if (sip->segnum <= 256)
        sip->segnumwidth = 1;
    else if (sip->segnum <= 65536)
        sip->segnumwidth = 2;
    else
        sip->segnumwidth = 4;
    for (i = 0; i < sip->countofrefered; i++) {
        switch (sip->segnumwidth) {
            case 1:
                (void) ygetc(fip->file);
                break;
            case 2:
                (void) read2bytes(fip->file);
                break;
            case 4:
                (void) read4bytes(fip->file);
                break;
        }
    }
    /*tex Table 7.2.6: Segment page association */
    if (sip->pageassocsizeflag)
        sip->segpage = read4bytes(fip->file);
    else
        sip->segpage = ygetc(fip->file);
    /*tex Table 7.2.7: Segment data length */
    sip->segdatalen = read4bytes(fip->file);
    sip->hdrend = (unsigned long) xftello(fip->file, fip->filepath);
    /*tex End of segment header. */
    return true;
}

static void checkseghdr(FILEINFO * fip, SEGINFO * sip);

static void markpage0seg(FILEINFO * fip, unsigned long referedseg)
{
    PAGEINFO *pip;
    SEGINFO *sip;
    pip = fip->page0.first->d;
    sip = find_seginfo(&(pip->segments), referedseg);
    if (sip != NULL) {
        if (!sip->refers && sip->countofrefered > 0)
            checkseghdr(fip, sip);
        sip->isrefered = true;
    }
}

/*tex

    For writing, marks refered page0 segments, sets segpage larger than
    zero to one.

*/

static void writeseghdr(PDF pdf, FILEINFO * fip, SEGINFO * sip)
{
    unsigned int i;
    unsigned long referedseg = 0;
    /*tex Table 7.2.2: Segment number */
    /*tex Table 7.2.3: Segment header flags */
    /*tex Table 7.2.4: Referred-to segment count and retention flags */
    for (i = 0; i < 5 + sip->fieldlen; i++)
        pdf_out(pdf, ygetc(fip->file));
    /*tex Table 7.2.5: Referred-to segment numbers */
    for (i = 0; i < sip->countofrefered; i++) {
        switch (sip->segnumwidth) {
            case 1:
                referedseg = (unsigned long) ygetc(fip->file);
                pdf_out(pdf, referedseg);
                break;
            case 2:
                referedseg = read2bytes(fip->file);
                pdf_out(pdf, (referedseg >> 8) & 0xff);
                pdf_out(pdf, referedseg & 0xff);
                break;
            case 4:
                referedseg = read4bytes(fip->file);
                pdf_out(pdf, (referedseg >> 24) & 0xff);
                pdf_out(pdf, (referedseg >> 16) & 0xff);
                pdf_out(pdf, (referedseg >> 8) & 0xff);
                pdf_out(pdf, referedseg & 0xff);
                break;
        }
        if (fip->page0.last != NULL && !sip->refers)
            markpage0seg(fip, referedseg);
    }
    if (sip->countofrefered > 0)
        sip->refers = true;
    /*tex Table 7.2.6: Segment page association */
    if (sip->pageassocsizeflag)
        for (i = 0; i < 3; i++) {
            (void) ygetc(fip->file);
            pdf_out(pdf, 0);
        }
    (void) ygetc(fip->file);
    pdf_out(pdf, (unsigned char) ((sip->segpage > 0) ? 1 : 0));
    /*tex Table 7.2.7: Segment data length */
    for (i = 0; i < 4; i++)
        pdf_out(pdf, ygetc(fip->file));
    /* End of segment header. */
}

/*tex

    For recursive marking of refered page0 segments:

*/

static void checkseghdr(FILEINFO * fip, SEGINFO * sip)
{
    unsigned int i;
    unsigned long referedseg = 0;
    /*tex Table 7.2.2: Segment number */
    /*tex Table 7.2.3: Segment header flags */
    /*tex Table 7.2.4: Referred-to segment count and retention flags */
    xfseek(fip->file, 5 + sip->fieldlen, SEEK_CUR, fip->filepath);
    /*tex Table 7.2.5: Referred-to segment numbers */
    for (i = 0; i < sip->countofrefered; i++) {
        switch (sip->segnumwidth) {
            case 1:
                referedseg = (unsigned long) ygetc(fip->file);
                break;
            case 2:
                referedseg = read2bytes(fip->file);
                break;
            case 4:
                referedseg = read4bytes(fip->file);
                break;
        }
        if (!sip->refers)
            markpage0seg(fip, referedseg);
    }
    if (sip->countofrefered > 0)
        sip->refers = true;
    /*tex Table 7.2.6: Segment page association */
    /*tex Table 7.2.7: Segment data length */
    if (sip->pageassocsizeflag)
        xfseek(fip->file, 8, SEEK_CUR, fip->filepath);
    else
        xfseek(fip->file, 5, SEEK_CUR, fip->filepath);
    /*tex End of segment header. */
}

static unsigned long findstreamstart(FILEINFO * fip)
{
    SEGINFO tmp;
    /*tex Table D.2: Random-access organisation */
    do {
        /*tex Find random-access stream start. */
        (void) readseghdr(fip, &tmp);
    } while (!tmp.endoffileflag);
    fip->streamstart = tmp.hdrend;
    readfilehdr(fip);
    return fip->streamstart;
}

static void rd_jbig2_info(FILEINFO * fip)
{
    /*tex For sequential-access only: */
    unsigned long seekdist = 0;
    /*tex For random-access only: */
    unsigned long streampos = 0;
    unsigned long currentpage = 0;
    boolean sipavail = false;
    PAGEINFO *pip;
    SEGINFO *sip = NULL;
    LIST *plp, *slp;
    fip->file = xfopen(fip->filepath, FOPEN_RBIN_MODE);
    readfilehdr(fip);
    if (!fip->sequentialaccess) {
        /*tex Table D.2: Random-access organisation */
        streampos = findstreamstart(fip);
    }
    while (true) {
        /*tex Loop over segments: */
        if (!sipavail) {
            sip = xtalloc(1, SEGINFO);
            sipavail = true;
        }
        init_seginfo(sip);
        if (!readseghdr(fip, sip) || sip->endoffileflag)
            break;
        if (sip->segpage > 0) {
            if (sip->segpage > (int) currentpage) {
                plp = litem_append(&(fip->pages));
                plp->last->d = new_pageinfo();
                currentpage = (unsigned long) sip->segpage;
            }
            pip = fip->pages.last->d;
        } else {
            if (fip->page0.last == NULL) {
                plp = litem_append(&(fip->page0));
                plp->last->d = new_pageinfo();
            }
            pip = fip->page0.last->d;
        }
        if (!sip->endofpageflag) {
            slp = litem_append(&(pip->segments));
            slp->last->d = sip;
            sipavail = false;
        }
        if (!fip->sequentialaccess)
            sip->datastart = streampos;
        else
            sip->datastart = sip->hdrend;
        sip->dataend = sip->datastart + sip->segdatalen;
        if (!fip->sequentialaccess && (sip->pageinfoflag || sip->endofstripeflag))
            xfseeko(fip->file, (off_t) sip->datastart, SEEK_SET, fip->filepath);
        seekdist = sip->segdatalen;
        /*tex Table 7.4.8: Page information segment syntax */
        if (sip->pageinfoflag) {
            pip->pagenum = (unsigned long) sip->segpage;
            pip->width = read4bytes(fip->file);
            pip->height = read4bytes(fip->file);
            pip->xres = read4bytes(fip->file);
            pip->yres = read4bytes(fip->file);
            pip->pagesegmentflags = (unsigned) ygetc(fip->file);
            /*tex Table 7.4.8.6: Page striping information */
            pip->stripinginfo = read2bytes(fip->file);
            seekdist -= 19;
        }
        if (sip->endofstripeflag) {
            pip->stripedheight = read4bytes(fip->file);
            seekdist -= 4;
        }
        if (!fip->sequentialaccess
            && (sip->pageinfoflag || sip->endofstripeflag))
            xfseeko(fip->file, (off_t) sip->hdrend, SEEK_SET, fip->filepath);
        if (!fip->sequentialaccess)
            streampos += sip->segdatalen;
        if (fip->sequentialaccess)
            xfseeko(fip->file, (off_t) seekdist, SEEK_CUR, fip->filepath);
        if (sip->endofpageflag && currentpage && (pip->stripinginfo >> 15))
            pip->height = pip->stripedheight;
    }
    fip->phase = HAVEINFO;
    if (sipavail)
        xfree(sip);
    xfclose(fip->file, fip->filepath);
}

static void wr_jbig2(PDF pdf, image_dict * idict, FILEINFO * fip, unsigned long page)
{
    LITEM *slip;
    PAGEINFO *pip;
    SEGINFO *sip;
    unsigned long i;
    if (page > 0) {
        assert(idict != NULL);
        pip = find_pageinfo(&(fip->pages), page);
        assert(pip != NULL);
        pdf_begin_obj(pdf, img_objnum(idict), OBJSTM_NEVER);
        pdf_begin_dict(pdf);
        pdf_dict_add_name(pdf, "Type", "XObject");
        pdf_dict_add_name(pdf, "Subtype", "Image");
        pdf_dict_add_img_filename(pdf, idict);
        pdf_dict_add_int(pdf, "Width", pip->width);
        pdf_dict_add_int(pdf, "Height", pip->height);
        pdf_dict_add_name(pdf, "ColorSpace", "DeviceGray");
        pdf_dict_add_int(pdf, "BitsPerComponent", 1);
        pdf_dict_add_int(pdf, "Length", getstreamlen(pip->segments.first, true));
        pdf_add_name(pdf, "Filter");
        pdf_begin_array(pdf);
        pdf_add_name(pdf, "JBIG2Decode");
        pdf_end_array(pdf);
        if (fip->page0.last != NULL) {
            if (fip->pdfpage0objnum == 0) {
                fip->pdfpage0objnum = (unsigned long) pdf_create_obj(pdf, obj_type_others, 0);
            }
            pdf_add_name(pdf, "DecodeParms");
            pdf_begin_array(pdf);
            pdf_begin_dict(pdf);
            pdf_dict_add_ref(pdf, "JBIG2Globals", fip->pdfpage0objnum);
            pdf_end_dict(pdf);
            pdf_end_array(pdf);
        }
        pdf_end_dict(pdf);
    } else {
        assert(idict == NULL);
        pip = find_pageinfo(&(fip->page0), page);
        assert(pip != NULL);
        pdf_begin_obj(pdf, (int) fip->pdfpage0objnum, OBJSTM_NEVER);
        pdf_begin_dict(pdf);
        pdf_dict_add_int(pdf, "Length", getstreamlen(pip->segments.first, false));
        pdf_end_dict(pdf);
    }
    pdf_begin_stream(pdf);
    fip->file = xfopen(fip->filepath, FOPEN_RBIN_MODE);
    for (slip = pip->segments.first; slip != NULL; slip = slip->next) {
        /*tex Loop over page segments. */
        sip = slip->d;
        if (sip->isrefered || page > 0) {
            xfseeko(fip->file, (off_t) sip->hdrstart, SEEK_SET, fip->filepath);
            /*tex Mark refered-to page 0 segments, change segpages > 1 to 1. */
            writeseghdr(pdf, fip, sip);
            xfseeko(fip->file, (off_t) sip->datastart, SEEK_SET, fip->filepath);
            for (i = sip->datastart; i < sip->dataend; i++)
                pdf_out(pdf, ygetc(fip->file));
        }
    }
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
    xfclose(fip->file, fip->filepath);
}

boolean supported_jbig2(image_dict * idict)
{
    if (img_pdfmajorversion(idict) < 2 && img_pdfminorversion(idict) < 4) {
        normal_error("readjbig2","you need to generate at least PDF 1.4");
        return false;
    } else {
        return true;
    }
}

void flush_jbig2_info(image_dict * idict)
{
    /*tex Todo (or not). */
}

void read_jbig2_info(image_dict * idict)
{
    FILEINFO *fip, tmp;
    PAGEINFO *pip;
    /*tex Already set probably, see other |read_|. */
    img_type(idict) = IMG_TYPE_JBIG2;
    if (! supported_jbig2(idict)) {
        /*tex Already an error seen? */
    }
    if (img_pagenum(idict) < 1) {
        normal_error("readjbig2","page must be > 0");
    }
    if (file_tree == NULL) {
        file_tree = avl_create(comp_file_entry, NULL, &avl_xallocator);
    }
    tmp.filepath = img_filepath(idict);
    fip = (FILEINFO *) avl_find(file_tree, &tmp);
    if (fip == NULL) {
        fip = new_fileinfo();
        fip->filepath = xstrdup(img_filepath(idict));
        avl_probe(file_tree, fip);
    }
    if (fip->phase == INITIAL) {
        rd_jbig2_info(fip);
        pages_maketree(&(fip->pages));
        if (fip->page0.last != NULL) {
            pages_maketree(&(fip->page0));
            pip = fip->page0.first->d;
            segments_maketree(&(pip->segments));
        }
    }
    pip = find_pageinfo(&(fip->pages), (unsigned long) img_pagenum(idict));
    if (pip == NULL) {
        formatted_error("readjbig2","page %d not found in image file",(int) img_pagenum(idict));
    }
    img_totalpages(idict) = (int) fip->numofpages;
    img_xsize(idict) = (int) pip->width;
    img_ysize(idict) = (int) pip->height;
    img_xres(idict) = (int) (pip->xres * 0.0254 + 0.5);
    img_yres(idict) = (int) (pip->yres * 0.0254 + 0.5);
    img_colordepth(idict) = 1;
}

void write_jbig2(PDF pdf, image_dict * idict)
{
    FILEINFO *fip, tmp;
    PAGEINFO *pip;
    tmp.filepath = img_filepath(idict);
    fip = (FILEINFO *) avl_find(file_tree, &tmp);
    /*tex Don't write before |rd_jbig2_info()| call. */
    pip = find_pageinfo(&(fip->pages), (unsigned long) img_pagenum(idict));
    assert(pip != NULL);
    wr_jbig2(pdf, idict, fip, pip->pagenum);
    img_file(idict) = NULL;
}

void flush_jbig2_page0_objects(PDF pdf)
{
    FILEINFO *fip;
    struct avl_traverser t;
    if (file_tree != NULL) {
        avl_t_init(&t, file_tree);
        for (fip = avl_t_first(&t, file_tree); fip != NULL;
             fip = avl_t_next(&t)) {
            if (fip->page0.last != NULL)
                /*tex |NULL|: page0 */
                wr_jbig2(pdf, NULL, fip, 0);
        }
    }
}
