% pdftoepdf.w
%
% Copyright 1996-2006 Han The Thanh <thanh@@pdftex.org>
% Copyright 2006-2015 Taco Hoekwater <taco@@luatex.org>
%
% This file is part of LuaTeX.
%
% LuaTeX is free software; you can redistribute it and/or modify it under
% the terms of the GNU General Public License as published by the Free
% Software Foundation; either version 2 of the License, or (at your
% option) any later version.
%
% LuaTeX is distributed in the hope that it will be useful, but WITHOUT
% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
% FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
% License for more details.
%
% You should have received a copy of the GNU General Public License along
% with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

@ @c

#define __STDC_FORMAT_MACROS /* for PRId64 etc.  */

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#  include <inttypes.h>
#endif
#include "image/epdf.h"

/*
    This file is mostly C and not very much C++; it's just used to interface
    the functions of poppler, which happens to be written in C++.
*/

extern void md5(Guchar *msg, int msgLen, Guchar *digest);

static GBool isInit = gFalse;

/* Maintain AVL tree of all PDF files for embedding */

static avl_table *PdfDocumentTree = NULL;

/* AVL sort PdfDocument into PdfDocumentTree by file_path */

static int CompPdfDocument(const void *pa, const void *pb, void * /*p */ )
{
    return strcmp(((const PdfDocument *) pa)->file_path, ((const PdfDocument *) pb)->file_path);
}

/* Returns pointer to PdfDocument structure for PDF file. */

static PdfDocument *findPdfDocument(char *file_path)
{
    PdfDocument *pdf_doc, tmp;
    if (file_path == NULL) {
        normal_error("pdf backend","empty filename when loading pdf file");
    } else if (PdfDocumentTree == NULL) {
        return NULL;
    }
    tmp.file_path = file_path;
    pdf_doc = (PdfDocument *) avl_find(PdfDocumentTree, &tmp);
    return pdf_doc;
}

#define PDF_CHECKSUM_SIZE 32

static char *get_file_checksum(const char *a, file_error_mode fe)
{
    struct stat finfo;
    char *ck = NULL;
    if (stat(a, &finfo) == 0) {
        off_t size = finfo.st_size;
        time_t mtime = finfo.st_mtime;
        ck = (char *) malloc(PDF_CHECKSUM_SIZE);
        if (ck == NULL)
            formatted_error("pdf inclusion","out of memory while processing '%s'", a);
#if defined(MIKTEX)
        snprintf(ck, PDF_CHECKSUM_SIZE, "%"
		         PRIu64 "_%"
				 PRIu64, (uint64_t) size,
                 (uint64_t) mtime);
#else
        snprintf(ck, PDF_CHECKSUM_SIZE, "%" PRIu64 "_%" PRIu64, (uint64_t) size,(uint64_t) mtime);
#endif
   } else {
        switch (fe) {
            case FE_FAIL:
                formatted_error("pdf inclusion","could not stat() file '%s'", a);
                break;
            case FE_RETURN_NULL:
                if (ck != NULL)
                    free(ck);
                ck = NULL;
                break;
            default:
                assert(0);
        }
    }
    return ck;
}


static char *get_stream_checksum (const char *str, unsigned long long str_size){
    /* http://www.cse.yorku.ca/~oz/hash.html */
    /* djb2                                  */
    unsigned long hash ;
    char *ck = NULL;
    unsigned int i;
    hash = 5381;
    ck = (char *) malloc(STRSTREAM_CHECKSUM_SIZE+1);
    if (ck == NULL)
        normal_error("pdf inclusion","out of memory while processing a memstream");
    for(i=0; i<(unsigned int)(str_size); i++) {
        hash = ((hash << 5) + hash) + str[i]; /* hash * 33 + str[i] */
    }
    snprintf(ck,STRSTREAM_CHECKSUM_SIZE+1,"%lx",hash);
    ck[STRSTREAM_CHECKSUM_SIZE]='\0';
    return ck;
}

/*
    Returns pointer to PdfDocument structure for PDF file.
    Creates a new PdfDocument structure if it doesn't exist yet.
    When fe = FE_RETURN_NULL, the function returns NULL in error case.
*/

PdfDocument *refPdfDocument(const char *file_path, file_error_mode fe)
{
    char *checksum, *path_copy;
    PdfDocument *pdf_doc;
    PDFDoc *doc = NULL;
    GooString *docName = NULL;
    int new_flag = 0;
    if ((checksum = get_file_checksum(file_path, fe)) == NULL) {
        return (PdfDocument *) NULL;
    }
    path_copy = xstrdup(file_path);
    if ((pdf_doc = findPdfDocument(path_copy)) == NULL) {
        new_flag = 1;
        pdf_doc = new PdfDocument;
        pdf_doc->file_path = path_copy;
        pdf_doc->checksum = checksum;
        pdf_doc->doc = NULL;
        pdf_doc->inObjList = NULL;
        pdf_doc->ObjMapTree = NULL;
        pdf_doc->occurences = 0; /* 0 = unreferenced */
        pdf_doc->pc = 0;
    } else {
        if (strncmp(pdf_doc->checksum, checksum, PDF_CHECKSUM_SIZE) != 0) {
            formatted_error("pdf inclusion","file has changed '%s'", file_path);
        }
        free(checksum);
        free(path_copy);
    }
    if (pdf_doc->doc == NULL) {
        docName = new GooString(file_path);
        doc = new PDFDoc(docName); /* takes ownership of docName */
        pdf_doc->pc++;

        if (!doc->isOk() || !doc->okToPrint()) {
            switch (fe) {
            case FE_FAIL:
                normal_error("pdf inclusion","reading image failed");
                break;
            case FE_RETURN_NULL:
                delete doc;
                /* delete docName */
                if (new_flag == 1) {
                    if (pdf_doc->file_path != NULL)
                        free(pdf_doc->file_path);
                    if (pdf_doc->checksum != NULL)
                        free(pdf_doc->checksum);
                    delete pdf_doc;
                }
                return (PdfDocument *) NULL;
                break;
            default:
                assert(0);
            }
        }
        pdf_doc->doc = doc;
    }
    /* PDF file could be opened without problems, checksum ok. */
    if (PdfDocumentTree == NULL)
        PdfDocumentTree = avl_create(CompPdfDocument, NULL, &avl_xallocator);
    if ((PdfDocument *) avl_find(PdfDocumentTree, pdf_doc) == NULL) {
        avl_probe(PdfDocumentTree, pdf_doc);
    }
    pdf_doc->occurences++;
    return pdf_doc;
}

/*
    Returns pointer to PdfDocument structure for a PDF stream in memory of streamsize
    dimension. As before, creates a new PdfDocument structure if it doesn't exist yet
    with file_path = file_id
*/

PdfDocument *refMemStreamPdfDocument(char *docstream, unsigned long long streamsize,const char *file_id)
{
    char *checksum;
    char *file_path;
    PdfDocument *pdf_doc;
    PDFDoc *doc = NULL;
    Object obj;
    MemStream *docmemstream = NULL;
    /*int new_flag = 0;*/
    size_t  cnt = 0;
    checksum = get_stream_checksum(docstream, streamsize);
    cnt = strlen(file_id);
    assert(cnt>0 && cnt <STREAM_FILE_ID_LEN);
    file_path = (char *) malloc(cnt+STREAM_URI_LEN+STRSTREAM_CHECKSUM_SIZE+1); /* 1 for \0 */
    assert(file_path != NULL);
    strcpy(file_path,STREAM_URI);
    strcat(file_path,file_id);
    strcat(file_path,checksum);
    file_path[cnt+STREAM_URI_LEN+STRSTREAM_CHECKSUM_SIZE]='\0';
    if ((pdf_doc = findPdfDocument(file_path)) == NULL) {
        /*new_flag = 1;*/
        pdf_doc = new PdfDocument;
        pdf_doc->file_path = file_path;
        pdf_doc->checksum = checksum;
        pdf_doc->doc = NULL;
        pdf_doc->inObjList = NULL;
        pdf_doc->ObjMapTree = NULL;
        pdf_doc->occurences = 0; /* 0 = unreferenced */
        pdf_doc->pc = 0;
    } else {
        /* As is now, checksum is in file_path, so this check should be useless. */
        if (strncmp(pdf_doc->checksum, checksum, STRSTREAM_CHECKSUM_SIZE) != 0) {
            formatted_error("pdf inclusion","stream has changed '%s'", file_path);
        }
        free(file_path);
        free(checksum);
    }
    if (pdf_doc->doc == NULL) {
#ifndef MIKTEX_POPPLER_59
        docmemstream = new MemStream( docstream,0,streamsize, obj.initNull() );
#else
        docmemstream = new MemStream( docstream,0,streamsize, Object(objNull) );
#endif
        doc = new PDFDoc(docmemstream); /* takes ownership of docmemstream */
        pdf_doc->pc++;
        if (!doc->isOk() || !doc->okToPrint()) {
            normal_error("pdf inclusion","reading pdf Stream failed");
    }
        pdf_doc->doc = doc;
    }
    /* PDF file could be opened without problems, checksum ok. */
    if (PdfDocumentTree == NULL)
        PdfDocumentTree = avl_create(CompPdfDocument, NULL, &avl_xallocator);
    if ((PdfDocument *) avl_find(PdfDocumentTree, pdf_doc) == NULL) {
        avl_probe(PdfDocumentTree, pdf_doc);
    }
    pdf_doc->occurences++;
    return pdf_doc;
}

/*
    AVL sort ObjMap into ObjMapTree by object number and generation keep the ObjMap
    struct small, as these are accumulated until the end
*/

struct ObjMap {
    Ref in;
    int out_num;
};

static int CompObjMap(const void *pa, const void *pb, void * /*p */ )
{
    const Ref *a = &(((const ObjMap *) pa)->in);
    const Ref *b = &(((const ObjMap *) pb)->in);
    if (a->num > b->num)
        return 1;
    else if (a->num < b->num)
        return -1;
    else if (a->gen == b->gen)
        return 0;
    else if (a->gen < b->gen)
        return -1;
    return 1;
}

static ObjMap *findObjMap(PdfDocument * pdf_doc, Ref in)
{
    ObjMap *obj_map, tmp;
    if (pdf_doc->ObjMapTree == NULL)
        return NULL;
    tmp.in = in;
    obj_map = (ObjMap *) avl_find(pdf_doc->ObjMapTree, &tmp);
    return obj_map;
}

static void addObjMap(PdfDocument * pdf_doc, Ref in, int out_num)
{
    ObjMap *obj_map = NULL;
    if (pdf_doc->ObjMapTree == NULL)
        pdf_doc->ObjMapTree = avl_create(CompObjMap, NULL, &avl_xallocator);
    obj_map = new ObjMap;
    obj_map->in = in;
    obj_map->out_num = out_num;
    avl_probe(pdf_doc->ObjMapTree, obj_map);
}

/*
    When copying the Resources of the selected page, all objects are
    copied recursively top-down.  The findObjMap() function checks if an
    object has already been copied; if so, instead of copying just the
    new object number will be referenced.  The ObjMapTree guarantees,
    that during the entire LuaTeX run any object from any embedded PDF
    file will end up max. once in the output PDF file.  Indirect objects
    are not fetched during copying, but get a new object number from
    LuaTeX and then will be appended into a linked list.
*/

static int addInObj(PDF pdf, PdfDocument * pdf_doc, Ref ref)
{
    ObjMap *obj_map;
    InObj *p, *q, *n;
    if (ref.num == 0) {
        normal_error("pdf inclusion","reference to invalid object (broken pdf)");
    }
    if ((obj_map = findObjMap(pdf_doc, ref)) != NULL)
        return obj_map->out_num;
    n = new InObj;
    n->ref = ref;
    n->next = NULL;
    n->num = pdf_create_obj(pdf, obj_type_others, 0);
    addObjMap(pdf_doc, ref, n->num);
    if (pdf_doc->inObjList == NULL) {
        pdf_doc->inObjList = n;
    } else {
        /*
            It is important to add new objects at the end of the list,
            because new objects are being added while the list is being
            written out by writeRefs().
        */
        for (p = pdf_doc->inObjList; p != NULL; p = p->next)
            q = p;
        q->next = n;
    }
    return n->num;
}

/*
    Function converts double to pdffloat; very small and very large numbers
    are NOT converted to scientific notation. Here n must be a number or real
    conforming to the implementation limits of PDF as specified in appendix C.1
    of the PDF Ref. These are:

    maximum value of ints is +2^32
    maximum value of reals is +2^15
    smalles values of reals is 1/(2^16)
*/

static pdffloat conv_double_to_pdffloat(double n)
{
    pdffloat a;
    a.e = 6;
    a.m = i64round(n * ten_pow[a.e]);
    return a;
}

static void copyObject(PDF, PdfDocument *, Object *);

void copyReal(PDF pdf, double d)
{
    if (pdf->cave)
        pdf_out(pdf, ' ');
    print_pdffloat(pdf, conv_double_to_pdffloat(d));
    pdf->cave = true;
}

static void copyString(PDF pdf, GooString * string)
{
    char *p;
    unsigned char c;
    size_t i, l;
    p = string->getCString();
    l = (size_t) string->getLength();
    if (pdf->cave)
        pdf_out(pdf, ' ');
    if (strlen(p) == l) {
        pdf_out(pdf, '(');
        for (; *p != 0; p++) {
            c = (unsigned char) *p;
            if (c == '(' || c == ')' || c == '\\')
                pdf_printf(pdf, "\\%c", c);
            else if (c < 0x20 || c > 0x7F)
                pdf_printf(pdf, "\\%03o", (int) c);
            else
                pdf_out(pdf, c);
        }
        pdf_out(pdf, ')');
    } else {
        pdf_out(pdf, '<');
        for (i = 0; i < l; i++) {
            c = (unsigned char) string->getChar(i);
            pdf_printf(pdf, "%.2x", (int) c);
        }
        pdf_out(pdf, '>');
    }
    pdf->cave = true;
}

static void copyName(PDF pdf, char *s)
{
    pdf_out(pdf, '/');
    for (; *s != 0; s++) {
        if (isdigit(*s) || isupper(*s) || islower(*s) || *s == '_' ||
            *s == '.' || *s == '-' || *s == '+')
            pdf_out(pdf, *s);
        else
            pdf_printf(pdf, "#%.2X", *s & 0xFF);
    }
    pdf->cave = true;
}

static void copyArray(PDF pdf, PdfDocument * pdf_doc, Array * array)
{
    int i, l;
    Object obj1;
    pdf_begin_array(pdf);
    for (i = 0, l = array->getLength(); i < l; ++i) {
#ifndef MIKTEX_POPPLER_59
        array->getNF(i, &obj1);
#else
        obj1 = array->getNF(i);
#endif
        copyObject(pdf, pdf_doc, &obj1);
#ifndef MIKTEX_POPPLER_59
        obj1.free();
#endif
    }
    pdf_end_array(pdf);
}

static void copyDict(PDF pdf, PdfDocument * pdf_doc, Dict * dict)
{
    int i, l;
    Object obj1;
    pdf_begin_dict(pdf);
    for (i = 0, l = dict->getLength(); i < l; ++i) {
        copyName(pdf, dict->getKey(i));
#ifndef MIKTEX_POPPLER_59
        dict->getValNF(i, &obj1);
#else
        obj1 = dict->getValNF(i);
#endif
        copyObject(pdf, pdf_doc, &obj1);
#ifndef MIKTEX_POPPLER_59
        obj1.free();
#endif
    }
    pdf_end_dict(pdf);
}

static void copyStreamStream(PDF pdf, Stream * str)
{
    int c, i, len = 1024;
    str->reset();
    i = len;
    while ((c = str->getChar()) != EOF) {
        if (i == len) {
            pdf_room(pdf, len);
            i = 0;
        }
        pdf_quick_out(pdf, c);
        i++;
    }
}

static void copyStream(PDF pdf, PdfDocument * pdf_doc, Stream * stream)
{
    copyDict(pdf, pdf_doc, stream->getDict());
    pdf_begin_stream(pdf);
    copyStreamStream(pdf, stream->getUndecodedStream());
    pdf_end_stream(pdf);
}

static void copyObject(PDF pdf, PdfDocument * pdf_doc, Object * obj)
{
    switch (obj->getType()) {
    case objBool:
        pdf_add_bool(pdf, (int) obj->getBool());
        break;
    case objInt:
        pdf_add_int(pdf, obj->getInt());
        break;
    case objReal:
        copyReal(pdf, obj->getReal());
        break;
    /*
    case objNum:
        GBool isNum() { return type == objInt || type == objReal; }
        break;
    */
    case objString:
        copyString(pdf, obj->getString());
        break;
    case objName:
        copyName(pdf, obj->getName());
        break;
    case objNull:
        pdf_add_null(pdf);
        break;
    case objArray:
        copyArray(pdf, pdf_doc, obj->getArray());
        break;
    case objDict:
        copyDict(pdf, pdf_doc, obj->getDict());
        break;
    case objStream:
        copyStream(pdf, pdf_doc, obj->getStream());
        break;
    case objRef:
        pdf_add_ref(pdf, addInObj(pdf, pdf_doc, obj->getRef()));
        break;
    case objCmd:
    case objError:
    case objEOF:
    case objNone:
        formatted_error("pdf inclusion","type '%s' cannot be copied", obj->getTypeName());
        break;
    default:
        /* poppler doesn't have any other types */
        assert(0);
    }
}

static void writeRefs(PDF pdf, PdfDocument * pdf_doc)
{
    InObj *r, *n;
    Object obj1;
    XRef *xref;
    PDFDoc *doc = pdf_doc->doc;
    xref = doc->getXRef();
    for (r = pdf_doc->inObjList; r != NULL;) {
#ifndef MIKTEX_POPPLER_59
        xref->fetch(r->ref.num, r->ref.gen, &obj1);
#else
        obj1 = xref->fetch(r->ref.num, r->ref.gen);
#endif
        if (obj1.isStream())
            pdf_begin_obj(pdf, r->num, OBJSTM_NEVER);
        else
            pdf_begin_obj(pdf, r->num, 2);
        copyObject(pdf, pdf_doc, &obj1);
#ifndef MIKTEX_POPPLER_59
        obj1.free();
#endif
        pdf_end_obj(pdf);
        n = r->next;
        delete r;
        pdf_doc->inObjList = r = n;
    }
}

/* get the pagebox coordinates according to the pagebox_spec */

static PDFRectangle *get_pagebox(Page * page, int pagebox_spec)
{
    switch (pagebox_spec) {
        case PDF_BOX_SPEC_MEDIA:
            return page->getMediaBox();
            break;
        case PDF_BOX_SPEC_CROP:
            return page->getCropBox();
            break;
        case PDF_BOX_SPEC_BLEED:
            return page->getBleedBox();
            break;
        case PDF_BOX_SPEC_TRIM:
            return page->getTrimBox();
            break;
        case PDF_BOX_SPEC_ART:
            return page->getArtBox();
            break;
        default:
            return page->getMediaBox();
            break;
    }
}

/*
    Reads various information about the PDF and sets it up for later inclusion.
    This will fail if the PDF version of the PDF is higher than minor_pdf_version_wanted
    or page_name is given and can not be found. It makes no sense to give page_name and
    page_num. Returns the page number.
*/

void flush_pdf_info(image_dict * idict)
{
    if (img_keepopen(idict)) {
        unrefPdfDocument(img_filepath(idict));
    }
}

/*
    void flush_pdfstream_info(image_dict * idict)
    {
        if (img_pdfstream_ptr(idict) != NULL) {
            xfree(img_pdfstream_stream(idict));
            xfree(img_pdfstream_ptr(idict));
            img_pdfstream_stream(idict) = NULL;
            img_pdfstream_ptr(idict) = NULL;
        }
    }
*/

void read_pdf_info(image_dict * idict)
{
    PdfDocument *pdf_doc = NULL;
    PDFDoc *doc = NULL;
    Catalog *catalog;
    Page *page;
    int rotate;
    PDFRectangle *pagebox;
    int pdf_major_version_found, pdf_minor_version_found;
    float xsize, ysize, xorig, yorig;
    if (isInit == gFalse) {
        if (!(globalParams))
            globalParams = new GlobalParams();
        globalParams->setErrQuiet(gFalse);
        isInit = gTrue;
    }
    if (img_type(idict) == IMG_TYPE_PDF)
        pdf_doc = refPdfDocument(img_filepath(idict), FE_FAIL);
    else if (img_type(idict) == IMG_TYPE_PDFMEMSTREAM) {
        pdf_doc = findPdfDocument(img_filepath(idict)) ;
        if (pdf_doc == NULL ) 
           normal_error("pdf inclusion", "memstream not initialized");
        if (pdf_doc->doc == NULL)
           normal_error("pdf inclusion", "memstream document is empty");
        pdf_doc->occurences++;
    } else {
        normal_error("pdf inclusion","unknown document");
    }
    doc = pdf_doc->doc;
    catalog = doc->getCatalog();
    /*
        Check PDF version. This works only for PDF 1.x but since any versions of
        PDF newer than 1.x will not be backwards compatible to PDF 1.x, we will
        then have to changed drastically anyway.
    */
    pdf_major_version_found = doc->getPDFMajorVersion();
    pdf_minor_version_found = doc->getPDFMinorVersion();
    if ((pdf_major_version_found > 1) || (pdf_minor_version_found > img_pdfminorversion(idict))) {
        const char *msg = "PDF inclusion: found PDF version '%d.%d', but at most version '1.%d' allowed";
        if (img_errorlevel(idict) > 0) {
            formatted_error("pdf inclusion",msg, pdf_major_version_found, pdf_minor_version_found, img_pdfminorversion(idict));
        } else {
            formatted_warning("pdf inclusion",msg, pdf_major_version_found, pdf_minor_version_found, img_pdfminorversion(idict));
        }
    }
    img_totalpages(idict) = catalog->getNumPages();
    if (img_pagename(idict)) {
        /* get page by name */
        GooString name(img_pagename(idict));
        LinkDest *link = doc->findDest(&name);
        if (link == NULL || !link->isOk())
            formatted_error("pdf inclusion","invalid destination '%s'",img_pagename(idict));
        Ref ref = link->getPageRef();
        img_pagenum(idict) = catalog->findPage(ref.num, ref.gen);
        if (img_pagenum(idict) == 0)
            formatted_error("pdf inclusion","destination is not a page '%s'",img_pagename(idict));
        delete link;
    } else {
        /* get page by number */
        if (img_pagenum(idict) <= 0
            || img_pagenum(idict) > img_totalpages(idict))
            formatted_error("pdf inclusion","required page '%i' does not exist",(int) img_pagenum(idict));
    }
    /* get the required page */
    page = catalog->getPage(img_pagenum(idict));
    /* get the pagebox coordinates (media, crop,...) to use. */
    pagebox = get_pagebox(page, img_pagebox(idict));
    if (pagebox->x2 > pagebox->x1) {
        xorig = pagebox->x1;
        xsize = pagebox->x2 - pagebox->x1;
    } else {
        xorig = pagebox->x2;
        xsize = pagebox->x1 - pagebox->x2;
    }
    if (pagebox->y2 > pagebox->y1) {
        yorig = pagebox->y1;
        ysize = pagebox->y2 - pagebox->y1;
    } else {
        yorig = pagebox->y2;
        ysize = pagebox->y1 - pagebox->y2;
    }
    /* The following 4 parameters are raw. Do _not_ modify by /Rotate! */
    img_xsize(idict) = bp2sp(xsize);
    img_ysize(idict) = bp2sp(ysize);
    img_xorig(idict) = bp2sp(xorig);
    img_yorig(idict) = bp2sp(yorig);
    /*
        Handle /Rotate parameter. Only multiples of 90 deg. are allowed (PDF Ref. v1.3,
        p. 78). We also accept negative angles. Beware: PDF counts clockwise! */
    rotate = page->getRotate();
    switch (((rotate % 360) + 360) % 360) {
        case 0:
            img_rotation(idict) = 0;
            break;
        case 90:
            img_rotation(idict) = 3;
            break;
        case 180:
            img_rotation(idict) = 2;
            break;
        case 270:
            img_rotation(idict) = 1;
            break;
        default:
            formatted_warning("pdf inclusion","/Rotate parameter in PDF file not multiple of 90 degrees");
    }
    /* currently unused info whether PDF contains a /Group */
    if (page->getGroup() != NULL)
        img_set_group(idict);
    /*
        LuaTeX pre 0.85 versions did this:

        if (readtype == IMG_CLOSEINBETWEEN) {
            unrefPdfDocument(img_filepath(idict));
        }

        and also unref'd in the finalizer zo we got an extra unrefs when garbage was
        collected. However it is more efficient to keep the file open so we do that
        now. The (slower) alternative is to unref here (which in most cases forcing a
        close of the file) but then we must not call flush_pdf_info.

        A close (unref) can be forced by nilling the dict object at the lua end and
        forcing a collectgarbage("collect") after that.

    */
    if (! img_keepopen(idict)) {
        unrefPdfDocument(img_filepath(idict));
    }
}

/*
    Write the current epf_doc. Here the included PDF is copied, so most errors
    that can happen during PDF inclusion will arise here.
*/

void write_epdf(PDF pdf, image_dict * idict, int suppress_optional_info)
{
    PdfDocument *pdf_doc = NULL;
    PDFDoc *doc = NULL;
    Catalog *catalog;
    Page *page;
    Ref *pageref;
    Dict *pageDict;
    Object obj1, contents, pageobj, pagesobj1, pagesobj2, *op1, *op2, *optmp;
    PDFRectangle *pagebox;
    int i, l;
    double bbox[4];
    /* char s[256]; */
    const char *pagedictkeys[] = {
        "Group", "LastModified", "Metadata", "PieceInfo", "Resources", "SeparationInfo", NULL
    };
    /* open PDF file */
    if (img_type(idict) == IMG_TYPE_PDF) {
        pdf_doc = refPdfDocument(img_filepath(idict), FE_FAIL);
    } else if (img_type(idict) == IMG_TYPE_PDFMEMSTREAM) {
        pdf_doc = findPdfDocument(img_filepath(idict)) ;
        pdf_doc->occurences++;
    } else {
        normal_error("pdf inclusion","unknown document");
    }
    doc = pdf_doc->doc;
    catalog = doc->getCatalog();
    page = catalog->getPage(img_pagenum(idict));
    pageref = catalog->getPageRef(img_pagenum(idict));
#ifndef MIKTEX_POPPLER_59
    doc->getXRef()->fetch(pageref->num, pageref->gen, &pageobj);
#else
    pageobj = doc->getXRef()->fetch(pageref->num, pageref->gen);
#endif
    pageDict = pageobj.getDict();
    /* write the Page header */
    pdf_begin_obj(pdf, img_objnum(idict), OBJSTM_NEVER);
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "XObject");
    pdf_dict_add_name(pdf, "Subtype", "Form");
    if (img_attr(idict) != NULL && strlen(img_attr(idict)) > 0) {
        pdf_printf(pdf, "\n%s\n", img_attr(idict));
    }
    pdf_dict_add_int(pdf, "FormType", 1);
    /* write additional information */
    pdf_dict_add_img_filename(pdf, idict);
    if ((suppress_optional_info & 4) == 0) {
        pdf_dict_add_int(pdf, "PTEX.PageNumber", (int) img_pagenum(idict));
    }
    if ((suppress_optional_info & 8) == 0) {
#ifndef MIKTEX_POPPLER_59
        doc->getDocInfoNF(&obj1);
#else
        obj1 = doc->getDocInfoNF();
#endif
        if (obj1.isRef()) {
            /* the info dict must be indirect (PDF Ref p. 61) */
            pdf_dict_add_ref(pdf, "PTEX.InfoDict", addInObj(pdf, pdf_doc, obj1.getRef()));
        }
#ifndef MIKTEX_POPPLER_59
        obj1.free();
#endif
    }
    if (img_is_bbox(idict)) {
        bbox[0] = sp2bp(img_bbox(idict)[0]);
        bbox[1] = sp2bp(img_bbox(idict)[1]);
        bbox[2] = sp2bp(img_bbox(idict)[2]);
        bbox[3] = sp2bp(img_bbox(idict)[3]);
    } else {
        /* get the pagebox coordinates (media, crop,...) to use. */
        pagebox = get_pagebox(page, img_pagebox(idict));
        bbox[0] = pagebox->x1;
        bbox[1] = pagebox->y1;
        bbox[2] = pagebox->x2;
        bbox[3] = pagebox->y2;
    }
    pdf_add_name(pdf, "BBox");
    pdf_begin_array(pdf);
    copyReal(pdf, bbox[0]);
    copyReal(pdf, bbox[1]);
    copyReal(pdf, bbox[2]);
    copyReal(pdf, bbox[3]);
    pdf_end_array(pdf);
    /*
        Now all relevant parts of the Page dictionary are copied. Metadata validity
        check is needed(as a stream it must be indirect).
    */
#ifndef MIKTEX_POPPLER_59
    pageDict->lookupNF("Metadata", &obj1);
#else
    obj1 = pageDict->lookupNF("Metadata");
#endif
    if (!obj1.isNull() && !obj1.isRef())
        formatted_warning("pdf inclusion","/Metadata must be indirect object");
#ifndef MIKTEX_POPPLER_59
    obj1.free();
#endif
    /* copy selected items in Page dictionary */
    for (i = 0; pagedictkeys[i] != NULL; i++) {
#ifndef MIKTEX_POPPLER_59
        pageDict->lookupNF(pagedictkeys[i], &obj1);
#else
        obj1 = pageDict->lookupNF(pagedictkeys[i]);
#endif
        if (!obj1.isNull()) {
            pdf_add_name(pdf, pagedictkeys[i]);
            /* preserves indirection */
            copyObject(pdf, pdf_doc, &obj1);
        }
#ifndef MIKTEX_POPPLER_59
        obj1.free();
#endif
    }
    /*
        If there are no Resources in the Page dict of the embedded page,
        try to inherit the Resources from the Pages tree of the embedded
        PDF file, climbing up the tree until the Resources are found.
        (This fixes a problem with Scribus 1.3.3.14.)
    */
#ifndef MIKTEX_POPPLER_59
    pageDict->lookupNF("Resources", &obj1);
#else
    obj1 = pageDict->lookupNF("Resources");
#endif
    if (obj1.isNull()) {
        op1 = &pagesobj1;
        op2 = &pagesobj2;
#ifndef MIKTEX_POPPLER_59
        pageDict->lookup("Parent", op1);
#else
        *op1 = pageDict->lookup("Parent");
#endif
        while (op1->isDict()) {
#ifndef MIKTEX_POPPLER_59
            obj1.free();
            op1->dictLookupNF("Resources", &obj1);
#else
            obj1 = op1->dictLookupNF("Resources");
#endif
            if (!obj1.isNull()) {
                pdf_add_name(pdf, "Resources");
                copyObject(pdf, pdf_doc, &obj1);
                break;
            }
#ifndef MIKTEX_POPPLER_59
            op1->dictLookup("Parent", op2);
#else
            *op2 = op1->dictLookup("Parent");
#endif
            optmp = op1;
            op1 = op2;
            op2 = optmp;
#ifndef MIKTEX_POPPLER_59
            op2->free();
#endif
        };
        if (!op1->isDict())
            formatted_warning("pdf inclusion","Page /Resources missing");
#ifndef MIKTEX_POPPLER_59
        op1->free();
#endif
    }
#ifndef MIKTEX_POPPLER_59
    obj1.free();
#endif
    /* Write the Page contents. */
#ifndef MIKTEX_POPPLER_59
    page->getContents(&contents);
#else
    contents = page->getContents();
#endif
    if (contents.isStream()) {
        /*
            Variant A: get stream and recompress under control of \pdfcompresslevel

            pdf_begin_stream();
            copyStreamStream(contents->getStream());
            pdf_end_stream();

            Variant B: copy stream without recompressing
        */
#ifndef MIKTEX_POPPLER_59
        contents.streamGetDict()->lookup("F", &obj1);
#else
        obj1 = contents.streamGetDict()->lookup("F");
#endif
        if (!obj1.isNull()) {
            normal_error("pdf inclusion","unsupported external stream");
        }
#ifndef MIKTEX_POPPLER_59
        obj1.free();
        contents.streamGetDict()->lookup("Length", &obj1);
#else
        obj1 = contents.streamGetDict()->lookup("Length");
#endif
        pdf_add_name(pdf, "Length");
        copyObject(pdf, pdf_doc, &obj1);
#ifndef MIKTEX_POPPLER_59
        obj1.free();
        contents.streamGetDict()->lookup("Filter", &obj1);
#else
        obj1 = contents.streamGetDict()->lookup("Filter");
#endif
        if (!obj1.isNull()) {
            pdf_add_name(pdf, "Filter");
            copyObject(pdf, pdf_doc, &obj1);
#ifndef MIKTEX_POPPLER_59
            obj1.free();
            contents.streamGetDict()->lookup("DecodeParms", &obj1);
#else
            obj1 = contents.streamGetDict()->lookup("DecodeParms");
#endif
            if (!obj1.isNull()) {
                pdf_add_name(pdf, "DecodeParms");
                copyObject(pdf, pdf_doc, &obj1);
            }
        }
#ifndef MIKTEX_POPPLER_59
        obj1.free();
#endif
        pdf_end_dict(pdf);
        pdf_begin_stream(pdf);
        copyStreamStream(pdf, contents.getStream()->getUndecodedStream());
        pdf_end_stream(pdf);
        pdf_end_obj(pdf);
    } else if (contents.isArray()) {
        pdf_dict_add_streaminfo(pdf);
        pdf_end_dict(pdf);
        pdf_begin_stream(pdf);
        for (i = 0, l = contents.arrayGetLength(); i < l; ++i) {
#ifndef MIKTEX_POPPLER_59
            copyStreamStream(pdf, (contents.arrayGet(i, &obj1))->getStream());
            obj1.free();
#else
            obj1 = contents.arrayGet(i);
            copyStreamStream(pdf, obj1.getStream());
#endif
            if (i < (l - 1)) {
                /*
                    Put a space between streams to be on the safe side (streams
                    should have a trailing space here, but one never knows)
                */
                pdf_out(pdf, ' ');
            }
        }
        pdf_end_stream(pdf);
        pdf_end_obj(pdf);
    } else {
        /* the contents are optional, but we need to include an empty stream */
        pdf_dict_add_streaminfo(pdf);
        pdf_end_dict(pdf);
        pdf_begin_stream(pdf);
        pdf_end_stream(pdf);
        pdf_end_obj(pdf);
    }
    /* write out all indirect objects */
    writeRefs(pdf, pdf_doc);
#ifndef MIKTEX_POPPLER_59
    contents.free();
    pageobj.free();
#endif
    /*
        unrefPdfDocument() must come after contents.free() and pageobj.free()!
        TH: The next line makes repeated pdf inclusion unacceptably slow

        unrefPdfDocument(img_filepath(idict));
    */
}

/* Deallocate a PdfDocument with all its resources. */

static void deletePdfDocumentPdfDoc(PdfDocument * pdf_doc)
{
    InObj *r, *n;
    /* this may be probably needed for an emergency destroyPdfDocument() */
    for (r = pdf_doc->inObjList; r != NULL; r = n) {
        n = r->next;
        delete r;
    }
    delete pdf_doc->doc;
    pdf_doc->doc = NULL;
    pdf_doc->pc++;
}

static void destroyPdfDocument(void *pa, void * /*pb */ )
{
    PdfDocument *pdf_doc = (PdfDocument *) pa;
    deletePdfDocumentPdfDoc(pdf_doc);
    /* TODO: delete rest of pdf_doc */
}

/*
    Called when an image has been written and its resources in image_tab are
    freed and it's not referenced anymore.
*/

void unrefPdfDocument(char *file_path)
{
    PdfDocument *pdf_doc = findPdfDocument(file_path);
    if (pdf_doc->occurences > 0) {
        pdf_doc->occurences--;
        if (pdf_doc->occurences == 0) {
            deletePdfDocumentPdfDoc(pdf_doc);
        }
    } else {
        /*
            We either have a mismatch in ref and unref or we're somehow out of sync
            which can happen when we mess with the same file in lua and tex.
        */
        formatted_warning("pdf inclusion","there can be a mismatch in opening and closing file '%s'",file_path);
    }
}

/*
    For completeness, but it isn't currently used (unreferencing is done by mean
    of file_path.
*/

void unrefMemStreamPdfDocument(char *file_id)
{
  (void) unrefPdfDocument(file_id);

}

/*
    Called when PDF embedding system is finalized.  We now deallocate all remaining
    PdfDocuments.
*/

void epdf_free()
{
    if (PdfDocumentTree != NULL)
        avl_destroy(PdfDocumentTree, destroyPdfDocument);
    PdfDocumentTree = NULL;
    if (isInit == gTrue)
        delete globalParams;
    isInit = gFalse;
}
